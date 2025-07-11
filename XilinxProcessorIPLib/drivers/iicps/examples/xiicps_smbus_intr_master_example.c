/******************************************************************************
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xiicps_smbus_intr_master_example.c
 *
 * This example can run on zynqmp / versal platform evaluation board and
 * IIC controller configured as master in interrupt-driven mode and Aardvark
 * Analyzer used as slave.
 * It sends and receives the data using IIC master device for SMBus transfers.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00  gm  06/30/22 First release
 * 3.18  gm  07/14/23 Added SDT support.
 * 3.22  bkv 07/09/25 Fixed GCC Warnings.
 *
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xiicps.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define IIC_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define IIC_INT_VEC_ID		XPAR_XIICPS_0_INTR
#else
#define XIICPS_BASEADDRESS	XPAR_XIICPS_0_BASEADDR
#endif

/*
 * The slave address to send to and receive from.
 */
#define IIC_SLAVE_ADDR		0x45
#define IIC_SCLK_RATE		100000

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the IIC.
 */
#define TEST_BUFFER_SIZE   	250

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

#ifndef SDT
int XIicPsSmbusMasterIntrExample(u16 DeviceId);
static int SetupInterruptSystem(XIicPs *IicPsPtr);
#else
int XIicPsSmbusMasterIntrExample(UINTPTR BaseAddress);
#endif
int XIicPsSmbusMasterWriteBlockData(XIicPs *InstancePtr, u8 Command, u8 ByteCount, u8 *SendBufferPtr);
int XIicPsSmbusMasterReadBlockData(XIicPs *InstancePtr, u8 Command, u8 ByteCount, u8 *RecvBufferPtr);
void Handler(void *CallBackRef, u32 Event);

/************************** Variable Definitions ******************************/

XIicPs Iic;			/* Instance of the IIC Device */
#ifndef SDT
XScuGic InterruptController;	/* Instance of the Interrupt Controller */
#endif

/*
 * The following buffers are used in this example to send and receive data
 * with the IIC. They are defined as global so that they are not on the stack.
 */
u8 SendBuffer[TEST_BUFFER_SIZE];    /* Buffer for Transmitting Data */
u8 RecvBuffer[TEST_BUFFER_SIZE];    /* Buffer for Receiving Data */

u8 Cmd = 0x11;						/* Send Command */
u8 RecvByteCount = 0;
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
* Main function to call the example.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
*******************************************************************************/
int main(void)
{
	int Status;
	int Index;

	xil_printf("SMBus Master Interrupt Example Test \r\n");

	/*
	 * Run the Iic Master Interrupt example , specify the Device ID that is
	 * generated in xparameters.h
	 */
#ifndef SDT
	Status = XIicPsSmbusMasterIntrExample(IIC_DEVICE_ID);
#else
	Status = XIicPsSmbusMasterIntrExample(XIICPS_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("SMBus Master Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Print receive operation data
	 */

	xil_printf("SMBus Master: Byte count = 0x%x \r\n", RecvByteCount);

	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		xil_printf("SMBus Master : Data: RecvBuffer[%d] = 0x%x \r\n", Index, RecvBuffer[Index]);
	}

	xil_printf("Successfully ran SMBus Master Interrupt Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the Iic device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XIicPs driver.
*
* This function sends and receives data as a smbus master in interrupt driver mode of the IIC.
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
#ifndef SDT
int XIicPsSmbusMasterIntrExample(u16 DeviceId)
#else
int XIicPsSmbusMasterIntrExample(UINTPTR BaseAddress)
#endif
{
	int Index;
	int Status;
	XIicPs_Config *Config;

	/*
	 * Initialize the IIC driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
#ifndef SDT
	Config = XIicPs_LookupConfig(DeviceId);
#else
	Config = XIicPs_LookupConfig(BaseAddress);
#endif
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
#ifndef SDT
	Status = SetupInterruptSystem(&Iic);
#else
	Status = XSetupInterruptSystem(&Iic, XIicPs_MasterInterruptHandler,
				       Config->IntrId,
				       Config->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
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

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);

	/*
	 * SMBus Slave Send operation
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = Index;
	}

	Status = XIicPsSmbusMasterWriteBlockData(&Iic, Cmd, TEST_BUFFER_SIZE, SendBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	/*
	 * SMBus Slave Receive operation
	 */
	Status = XIicPsSmbusMasterReadBlockData(&Iic, Cmd, TEST_BUFFER_SIZE, RecvBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes a buffer of data to the smbus slave.
*
* @param	InstancePtr	contains the address of XIicPs instance.
* @param	Command contains send command.
* @param	ByteCount contains the number of bytes to write.
* @param	SendBufferPtr contains the address of send buffer.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note	None
*
******************************************************************************/
int XIicPsSmbusMasterWriteBlockData(XIicPs *InstancePtr, u8 Command, u8 ByteCount, u8 *SendBufferPtr)
{
	u32 Index;
	u32 BufferIndex;
	static u8 SmbusSendBuffer[TEST_BUFFER_SIZE + 2];

	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	/*
	 * Fill and Send the Data.
	 */

	SmbusSendBuffer[0] = Command;
	SmbusSendBuffer[1] = ByteCount;

	for (Index = 2, BufferIndex = 0; Index < ((u32)ByteCount + 2); Index++, BufferIndex++) {
		SmbusSendBuffer[Index] = SendBufferPtr[BufferIndex];
	}

	TotalErrorCount = 0;
	SendComplete = FALSE;

	/*
	 * Send the buffer, errors are reported by TotalErrorCount.
	 */
	XIicPs_MasterSend(InstancePtr, SmbusSendBuffer, ByteCount + 2,
			  IIC_SLAVE_ADDR);

	/*
	 * Wait for the entire buffer to be sent, letting the interrupt
	 * processing work in the background, this function may get
	 * locked up in this loop if the interrupts are not working
	 * correctly.
	 */
	while (!SendComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	/*
	 * Wait bus activities to finish.
	 */
	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function sends a command and reads a buffer of data from the smbus slave.
*
* @param	InstancePtr	contains the address of XIicPs instance.
* @param	Command contains send command.
* @param	ByteCount contains the number of bytes to read.
* @param	RecvBufferPtr contains the address of receive buffer.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note	None
*
******************************************************************************/
int XIicPsSmbusMasterReadBlockData(XIicPs *InstancePtr, u8 Command, u8 ByteCount, u8 *RecvBufferPtr)
{
	u32 Index;
	u32 BufferIndex;
	static u8 SmbusRecvBuffer[TEST_BUFFER_SIZE + 1];

	/*
	 * Command Part
	 */

	/*
	 * Enable repeated start option.
	 * This call will give an indication to the driver.
	 * The hold bit is actually set before beginning the following transfer
	 */
	XIicPs_SetOptions(InstancePtr, XIICPS_REP_START_OPTION);

	SendBuffer[0] = Command;

	TotalErrorCount = 0;
	SendComplete = FALSE;

	/*
	 * Send the buffer, errors are reported by TotalErrorCount.
	 */
	XIicPs_MasterSend(InstancePtr, SendBuffer, 1, IIC_SLAVE_ADDR);

	/*
	 * Wait for the entire buffer to be sent, letting the interrupt
	 * processing work in the background, this function may get
	 * locked up in this loop if the interrupts are not working
	 * correctly.
	 */
	while (!SendComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	XIicPs_ClearOptions(InstancePtr, XIICPS_REP_START_OPTION);

	XIicPs_DisableAllInterrupts(InstancePtr->Config.BaseAddress);
	(void)XIicPs_ReadReg(InstancePtr->Config.BaseAddress, (u32)XIICPS_ISR_OFFSET);

	/*
	 * Receive Part
	 */

	/*
	 * Receive data from slave, errors are reported through
	 * TotalErrorCount.
	 */
	RecvByteCount = 0;

	for (Index = 0; Index < ByteCount; Index++) {
		SmbusRecvBuffer[Index] = 0;
		RecvBufferPtr[Index] = 0;
	}
	SmbusRecvBuffer[Index] = 0;

	TotalErrorCount = 0;
	RecvComplete = FALSE;

	XIicPs_MasterRecv(InstancePtr, SmbusRecvBuffer, ByteCount + 1,
			  IIC_SLAVE_ADDR);

	while (!RecvComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(InstancePtr));

	RecvByteCount = SmbusRecvBuffer[0];

	for (BufferIndex = 0, Index = 1; Index < ((u32)ByteCount + 1); BufferIndex++, Index++) {
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
* @param	CallBackRef contains a callback reference from the driver, in
*		this case it is the instance pointer for the IIC driver.
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
	 * All of the data transfer has been finished.
	 */
	if (0 != (Event & XIICPS_EVENT_COMPLETE_RECV)) {
		RecvComplete = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_COMPLETE_SEND)) {
		SendComplete = TRUE;
	} else if (0 == (Event & XIICPS_EVENT_SLAVE_RDY)) {
		/*
		 * If it is other interrupt but not slave ready interrupt, it is
		 * an error.
		 * Data was received with an error.
		 */
		TotalErrorCount++;
	}
}
#ifndef SDT
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
*		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
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
				 (Xil_InterruptHandler)XIicPs_MasterInterruptHandler,
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
#endif
