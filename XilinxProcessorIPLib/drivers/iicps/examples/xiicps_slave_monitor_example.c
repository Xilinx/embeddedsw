/******************************************************************************
*
* Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
 * @file xiicps_slave_monitor_example.c
 *
 * A design example of using the device as master to check slave's
 * availability.
 *
 * @note
 * Please set the slave address to 0x3FB, which tests the device's ability
 *	to handle 10-bit address.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00a jz  01/30/10 First release
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

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#define INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID

#define IIC_SCLK_RATE		100000
#define SLV_MON_LOOP_COUNT 0x000FFFFF	/**< Slave Monitor Loop Count*/

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

int IicPsSlaveMonitorExample();

static void Handler(void *CallBackRef, u32 Event);
static int IicPsSlaveMonitor(u16 Address, u16 DeviceId, u32 Int_Id);
static int SetupInterruptSystem(XIicPs *IicPsPtr, u32 Int_Id);
static int IicPsConfig(u16 DeviceId, u32 Int_Id);
static int IicPsFindDevice(u16 Addr);

/************************** Variable Definitions ******************************/

XIicPs	Iic;			/* Instance of the IIC Device */
XScuGic InterruptController;	/* Instance of the Interrupt Controller */

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
volatile u8 TransmitComplete;	/**< Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/**< Flag to check completion of Reception */
volatile u32 TotalErrorCount;	/**< Total Error Count Flag */
volatile u32 SlaveResponse;		/**< Slave Response Flag */

/**Searching for the required Slave Address and user can also add
 * their own slave Address in the below array list**/
u16 SlvAddr[] = {0x54,0x55,0x74,0};
XIicPs IicInstance;		/* The instance of the IIC device. */
/******************************************************************************/
/**
*
* Main function to call the Slave Monitor example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
*******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("IIC Slave Monitor Example Test \r\n");

	/*
	 * Run the Iic Slave Monitor example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = IicPsSlaveMonitorExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC Slave Monitor Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC Slave Monitor Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function perform the initial configuration for the IICPS Device.
*
* @param	DeviceId instance and Interrupt ID mapped to the device.
*
* @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
static int IicPsConfig(u16 DeviceId, u32 Int_Id)
{
	int Status;
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIicPs_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&IicInstance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the Interrupt System.
	 */
	Status = SetupInterruptSystem(&IicInstance, Int_Id);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the IIC that will be called from the
	 * interrupt context when data has been sent and received, specify a
	 * pointer to the IIC driver instance as the callback reference so
	 * the handlers are able to access the instance data.
	 */
	XIicPs_SetStatusHandler(&IicInstance, (void *) &IicInstance, Handler);

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&IicInstance, IIC_SCLK_RATE);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function checks the availability of a slave using slave monitor mode.
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note 	None.
*
*******************************************************************************/
int IicPsSlaveMonitorExample()
{
	int Status;
	int Index;

	for(Index = 0;SlvAddr[Index] != 0;Index++) {
		Status = IicPsFindDevice(SlvAddr[Index]);
		if (Status == XST_SUCCESS) {
			return XST_SUCCESS;
		}
	}
	return XST_FAILURE;
}
/*****************************************************************************/
/**
*
* This function checks the availability of a slave using slave monitor mode.
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note 	None.
*
*******************************************************************************/
static int IicPsSlaveMonitor(u16 Address, u16 DeviceId, u32 Int_Id)
{
	u32 Index;
	int Status;
	XIicPs *IicPtr;

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	Status = IicPsConfig(DeviceId,Int_Id);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	IicPtr = &IicInstance;
	XIicPs_DisableAllInterrupts(IicPtr->Config.BaseAddress);
	XIicPs_EnableSlaveMonitor(&IicInstance, Address);

	TotalErrorCount = 0;
	SlaveResponse = FALSE;

	Index = 0;

	/*
	 * Wait for the Slave Monitor Interrupt, the interrupt processing
	 * works in the background, this function may get locked up in this
	 * loop if the interrupts are not working correctly or the slave
	 * never responds.
	 */
	while ((!SlaveResponse) && (Index < SLV_MON_LOOP_COUNT)) {
		Index++;

		/*
		 * Ignore any errors. The hardware generates NACK interrupts
		 * if the slave is not present.
		 */
		if (0 != TotalErrorCount) {
			xil_printf("Test error unexpected NACK\n");
			return XST_FAILURE;
		}
	}

	if (Index >= SLV_MON_LOOP_COUNT) {
		return XST_FAILURE;

	}

	XIicPs_DisableSlaveMonitor(&IicInstance);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function checks whether the slave in alive or not.
*
* @param	Addr : Address of the slave device
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note 	None.
*
*******************************************************************************/
static int IicPsFindDevice(u16 Addr)
{
	int Status;

	Status = IicPsSlaveMonitor(Addr,0,XPAR_XIICPS_0_INTR);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	Status = IicPsSlaveMonitor(Addr,1,XPAR_XIICPS_1_INTR);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	Status = IicPsSlaveMonitor(Addr,0,XPAR_XIICPS_1_INTR);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	Status = IicPsSlaveMonitor(Addr,1,XPAR_XIICPS_0_INTR);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	return XST_FAILURE;
}
/******************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the IIC.
*
* @param	IicPsPtr contains a pointer to the instance of the Iic
*		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
static int SetupInterruptSystem(XIicPs *IicPsPtr, u32 Int_Id)
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
	Status = XScuGic_Connect(&InterruptController, Int_Id,
			(Xil_InterruptHandler)XIicPs_MasterInterruptHandler,
			(void *)IicPsPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Iic device.
	 */
	XScuGic_Enable(&InterruptController, Int_Id);


	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

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
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
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

	if (0 != (Event & XIICPS_EVENT_COMPLETE_SEND)) {
		TransmitComplete = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_COMPLETE_RECV)){
		ReceiveComplete = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_SLAVE_RDY)) {
		SlaveResponse = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_ERROR)){
		TotalErrorCount++;
	}
}
