/******************************************************************************
* Copyright (C) 2022 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiicps_smbus_intr_slave_example.c
*
* This example can run on zynqmp / versal platform evaluation board and
* IIC controller configured slave in interrupt-driven mode and loopback setup
* used for master.
* It sends and receives the data using IIC device as slave for SMBus transfers.
*
* @note	None.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.00  gm  05/10/22 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xiicps.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_DEVICE_ID		XPAR_XIICPS_1_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define IIC_INT_VEC_ID		XPAR_XIICPS_1_INTR

/*
 * The slave address to send to and receive from.
 */
#define IIC_SLAVE_ADDR		0x45
#define IIC_SCLK_RATE		400000

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the IIC.
 */

#define BUFFER_SIZE	250

/**************************** Type Definitions ********************************/


/************************** Function Prototypes *******************************/

int IicPsSmbusSlaveIntrExample(u16 DeviceId);
static int SetupInterruptSystem(XIicPs *IicPsPtr);
int XIicPsSmbusWriteBlockData(XIicPs *InstancePtr, u8 *Command, u8 ByteCount, u8 *SendBufferPtr);
int XIicPsSmbusReadBlockData(XIicPs *InstancePtr, u8 *Command, u8 *ByteCount, u8 *RecvBufferPtr);
void Handler(void *CallBackRef, u32 Event);

/************************** Variable Definitions ******************************/

XIicPs Iic;			/* Instance of the IIC Device */
XScuGic InterruptController; 	/* Instance of the Interrupt Controller */

/*
 * The following buffers are used in this example to send and receive data
 * with the IIC. The buffers are defined as global so that they are not on the
 * stack.
 */
u8 SendBuffer[BUFFER_SIZE];	/* Buffer for Transmitting Data */
u8 RecvBuffer[BUFFER_SIZE];	/* Buffer for Receiving Data */

u8 RecvCmd;			/* Received command */
u8 Cmd;				/* Command received during Send operation */
u8 RecvByteCount=0;

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
volatile u32 SendComplete;
volatile u32 RecvComplete;
volatile u32 TotalErrorCount;

/******************************************************************************/
/**
*
* Main function to call the interrupt example in the slave mode.
*
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note		None
*
*******************************************************************************/
int main(void)
{
	int Status;
	int Index=0;

	xil_printf("SMBus Slave Interrupt Example Test \r\n");

	/*
	 * Run the Iic Slave Interrupt example , specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = IicPsSmbusSlaveIntrExample(IIC_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("SMBus Slave Interrupt Example Test Failed  \r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SMBus Slave Interrupt Example Test \r\n");

	/*
	 * Print receive operation data
	 */
	xil_printf("SMBus Slave : receive operation : command = 0x%x \r\n", RecvCmd);
	xil_printf("SMBus Slave : Byte count = 0x%x \r\n", RecvByteCount);

	for(Index=0; Index<BUFFER_SIZE; Index++){
		xil_printf("SMBus Slave : Data: RecvBuffer[%d] = 0x%x \r\n", Index, RecvBuffer[Index] );
	}

	/*
	 * Print send operation data
	 */
	xil_printf("SMBus Slave : send operation : command = 0x%x \r\n", Cmd);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the Iic device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XIicPs component.
*
* This function sends and receives data as a smbus slave in interrupt driver mode of the IIC.
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
* This function contains an infinite loop such that if interrupts are not
* working it may never return.
*
*******************************************************************************/
int IicPsSmbusSlaveIntrExample(u16 DeviceId)
{
	int Status;
	int Index;
	XIicPs_Config *Config;

	/*
	 * Initialize the IIC driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XIicPs_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */

	Status = XIicPs_SelfTest(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the IIC to the interrupt subsystem such that interrupts can
	 * occur. This function is application specific.
	 */
	Status = SetupInterruptSystem(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the IIC that will be called from the
	 * interrupt context when data has been sent and received, specify a
	 * pointer to the IIC driver instance as the callback reference so
	 * the handlers are able to access the instance data.
	 */
	XIicPs_SetStatusHandler(&Iic, (void *) &Iic, Handler);
	XIicPs_SetupSlave(&Iic, IIC_SLAVE_ADDR);

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);

	/*
	 * SMBus Slave Receive operation
	 */
	XIicPsSmbusReadBlockData(&Iic, &RecvCmd ,&RecvByteCount, RecvBuffer);

	/*
	 * SMBus Slave Send operation
	 */

	for(Index=0;Index<BUFFER_SIZE; Index++){
		SendBuffer[Index]=Index;
	}

	XIicPsSmbusWriteBlockData(&Iic, &Cmd, BUFFER_SIZE, SendBuffer);

	return XST_SUCCESS;
}

int XIicPsSmbusWriteBlockData(XIicPs *InstancePtr, u8 *Command, u8 ByteCount, u8 *SendBufferPtr)
{
	u8 Cmmd=0;
	u32 Index;
	u32 BufferIndex;
	static u8 SmbusSendBuffer[BUFFER_SIZE+1];

	InstancePtr->RecvBufferPtr = &Cmmd;

	/*
	 * Command Receive part
	 */

	while ((XIicPs_RxDataValidStatus(InstancePtr)) != 0x20U){
		/* NOP */
	}

	XIicPs_RecvByte(InstancePtr);

	*Command = Cmmd;

	XIicPs_DisableAllInterrupts(InstancePtr->Config.BaseAddress);
	(void)XIicPs_ReadReg(InstancePtr->Config.BaseAddress, (u32)XIICPS_ISR_OFFSET);

	/*
	 * SMBus Slave send part
	 */
	SmbusSendBuffer[0] = ByteCount;

	for (Index = 1, BufferIndex=0; Index < (BUFFER_SIZE+1); Index++, BufferIndex++){
		SmbusSendBuffer[Index] = SendBufferPtr[BufferIndex];
	}

	TotalErrorCount = 0;
	SendComplete = FALSE;

	/*
	 * Send the buffer using the IIC and ignore the number of bytes sent.
	 * In case of error, the interrupt handler will inform us through event
	 * flag.
	 */
	XIicPs_SlaveSend(&Iic, SmbusSendBuffer, BUFFER_SIZE+1);

	/*
	 * Wait for the entire buffer to be sent, let the interrupt
	 * processing work in the background, this function may get locked
	 * up in this loop if the interrupts are not working correctly.
	 */
	while (!SendComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	/*
	 * Wait until the bus transfer finishes.
	 */
	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	return XST_SUCCESS;
}

int XIicPsSmbusReadBlockData(XIicPs *InstancePtr, u8 *Command, u8 *ByteCount, u8 *RecvBufferPtr)
{
	u32 Index;
	u32 BufferIndex;
	static u8 SmbusRecvBuffer[BUFFER_SIZE+2];

	/*
	 * Receive data from master.
	 * Receive errors will be signaled through event flag.
	 */

	for(Index=0;Index<BUFFER_SIZE;Index++){
		SmbusRecvBuffer[Index]=0;
		RecvBufferPtr[Index]=0;
	}

	*Command = 0;
	*ByteCount = 0;

	RecvComplete = FALSE;
	XIicPs_SlaveRecv(&Iic, SmbusRecvBuffer,0);

	while (!RecvComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	*Command = SmbusRecvBuffer[0];
	*ByteCount = SmbusRecvBuffer[1];

	for(BufferIndex=0, Index = 2; Index < (BUFFER_SIZE+2); BufferIndex++, Index ++) {
		RecvBufferPtr[BufferIndex] = SmbusRecvBuffer[Index];
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the handler which performs processing to handle data events
* from the IIC.  It is called from an interrupt context such that the amount
* of processing performed should be minimized.
*
* This handler provides an example of how to handle data for the IIC and
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver,
*		in this case it is the instance pointer for the IIC driver.
* @param	Event contains the specific kind of event that has occurred.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void Handler(void *CallBackRef, u32 Event)
{
	/*
	 * Data transfer finishes.
	 */
	if (0 != (Event & XIICPS_EVENT_COMPLETE_RECV)){
		RecvComplete = TRUE;
	}
	else if (0 != (Event & XIICPS_EVENT_COMPLETE_SEND)) {
		SendComplete = TRUE;
	}
	else {

		/*
		 * Data was received with an error.
		 */
		TotalErrorCount++;
	}
}

/******************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the IIC.  This function is application specific since the actual
* system may or may not have an interrupt controller.  The IIC could be
* directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	IicPsPtr contains a pointer to the instance of the Iic
*		component which is going to be connected to the interrupt
*		controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
*******************************************************************************/
static int SetupInterruptSystem(XIicPs *IicPsPtr)
{
	int Status;
	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				&InterruptController);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(&InterruptController, IIC_INT_VEC_ID,
			(Xil_InterruptHandler)XIicPs_SlaveInterruptHandler,
			(void *)IicPsPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Iic device.
	 */
	XScuGic_Enable(&InterruptController, IIC_INT_VEC_ID);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
