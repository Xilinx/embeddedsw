/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file xtmr_manager_intr_example.c
*
* This file contains a design example using the TMRManager driver (XTMRManager)
* and hardware device using the interrupt mode.
*
* @note
*
* The user must provide a physical loopback such that data which is
* transmitted will be received.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xtmr_manager.h"
#include "xintc.h"
#include "xil_exception.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMRMANAGER_DEVICE_ID      XPAR_TMRMANAGER_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define TMRMANAGER_INT_IRQ_ID     XPAR_INTC_0_TMRMANAGER_0_VEC_ID

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the TMRManager device.
 */
#define TEST_BUFFER_SIZE        100


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TMRManagerIntrExample(u16 DeviceId);

int SetupInterruptSystem(XTMRManager *TMRManagerPtr);

void SendHandler(void *CallBackRef, unsigned int EventData);

void RecvHandler(void *CallBackRef, unsigned int EventData);

/************************** Variable Definitions *****************************/

 XTMRManager TMRManager;            /* The instance of the TMRManager Device */

 XIntc InterruptController;     /* The instance of the Interrupt Controller */

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */

/*
 * The following buffers are used in this example to send and receive data
 * with the TMRManager.
 */
u8 SendBuffer[TEST_BUFFER_SIZE];
u8 ReceiveBuffer[TEST_BUFFER_SIZE];

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
static volatile int TotalReceivedCount;
static volatile int TotalSentCount;


/******************************************************************************/
/**
*
* Main function to call the TMRManager interrupt example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
*******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the TMRManager Interrupt example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = TMRManagerIntrExample(TMRMANAGER_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function does a minimal test on the TMRManager device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XTMRManager component.
*
* This function sends data and expects to receive the same data through the
* TMRManager. The user must provide a physical loopback such that data which is
* transmitted will be received.
*
* This function uses interrupt driver mode of the TMRManager device. The calls
* to the TMRManager driver in the handlers should only use the non-blocking
* calls.
*
* @param	DeviceId is the Device ID of the TMRManager Device and is the
*		XPAR_<tmr_manager_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
* This function contains an infinite loop such that if interrupts are not
* working it may never return.
*
****************************************************************************/
int TMRManagerIntrExample(u16 DeviceId)
{
	int Status;
	int Index;

	/*
	 * Initialize the TMRManager driver so that it's ready to use.
	 */
	Status = XTMRManager_Initialize(&TMRManager, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XTMRManager_SelfTest(&TMRManager);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the TMRManager to the interrupt subsystem such that interrupts can
	 * occur. This function is application specific.
	 */
	Status = SetupInterruptSystem(&TMRManager);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the TMRManager that will be called from the
	 * interrupt context when data has been sent and received, specify a
	 * pointer to the TMRManager driver instance as the callback reference so
	 * that the handlers are able to access the instance data.
	 */
	XTMRManager_SetSendHandler(&TMRManager, SendHandler, &TMRManager);
	XTMRManager_SetRecvHandler(&TMRManager, RecvHandler, &TMRManager);

	/*
	 * Enable the interrupt of the TMRManager so that interrupts will occur.
	 */
	XTMRManager_EnableInterrupt(&TMRManager);

	/*
	 * Initialize the send buffer bytes with a pattern to send and the
	 * the receive buffer bytes to zero to allow the receive data to be
	 * verified.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = Index;
		ReceiveBuffer[Index] = 0;
	}

	/*
	 * Start receiving data before sending it since there is a loopback.
	 */
	XTMRManager_Recv(&TMRManager, ReceiveBuffer, TEST_BUFFER_SIZE);

	/*
	 * Send the buffer using the TMRManager.
	 */
	XTMRManager_Send(&TMRManager, SendBuffer, TEST_BUFFER_SIZE);

	/*
	 * Wait for the entire buffer to be received, letting the interrupt
	 * processing work in the background, this function may get locked
	 * up in this loop if the interrupts are not working correctly.
	 */
	while ((TotalReceivedCount != TEST_BUFFER_SIZE) ||
		(TotalSentCount != TEST_BUFFER_SIZE)) {
	}

	/*
	 * Verify the entire receive buffer was successfully received.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		if (ReceiveBuffer[Index] != SendBuffer[Index]) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the handler which performs processing to send data to the
* TMRManager. It is called from an interrupt context such that the amount of
* processing performed should be minimized. It is called when the transmit
* FIFO of the TMRManager is empty and more data can be sent through the TMRManager.
*
* This handler provides an example of how to handle data for the TMRManager,
* but is application specific.
*
* @param	CallBackRef contains a callback reference from the driver.
*		In this case it is the instance pointer for the TMRManager driver.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void SendHandler(void *CallBackRef, unsigned int EventData)
{
	TotalSentCount = EventData;
}

/****************************************************************************/
/**
*
* This function is the handler which performs processing to receive data from
* the TMRManager. It is called from an interrupt context such that the amount of
* processing performed should be minimized.  It is called data is present in
* the receive FIFO of the TMRManager such that the data can be retrieved from
* the TMRManager. The size of the data present in the FIFO is not known when
* this function is called.
*
* This handler provides an example of how to handle data for the TMRManager,
* but is application specific.
*
* @param	CallBackRef contains a callback reference from the driver, in
*		this case it is the instance pointer for the TMRManager driver.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void RecvHandler(void *CallBackRef, unsigned int EventData)
{
	TotalReceivedCount = EventData;
}

/****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the TMRManager device. This function is application specific since the
* actual system may or may not have an interrupt controller. The TMRManager
* could be directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param    TMRManagerPtr contains a pointer to the instance of the TMRManager
*           component which is going to be connected to the interrupt
*           controller.
*
* @return   XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note     None.
*
****************************************************************************/
int SetupInterruptSystem(XTMRManager *TMRManagerPtr)
{

	int Status;


	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&InterruptController, TMRMANAGER_INT_IRQ_ID,
			   (XInterruptHandler)XTMRManager_InterruptHandler,
			   (void *)TMRManagerPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the TMRManager can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the TMRManager device.
	 */
	XIntc_Enable(&InterruptController, TMRMANAGER_INT_IRQ_ID);

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)XIntc_InterruptHandler,
			 &InterruptController);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
