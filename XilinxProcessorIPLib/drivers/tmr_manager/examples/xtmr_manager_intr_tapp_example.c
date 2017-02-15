/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
/******************************************************************************/
/**
*
* @file xtmr_manager_intr_tapp_example.c
*
* This file contains a design example using the TMRManager driver and
* hardware device using the interrupt mode for transmission of data.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xtmr_manager.h"
#include "xil_exception.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else
#include "xscugic.h"
#include "xil_printf.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define TMRMANAGER_DEVICE_ID	  XPAR_TMRMANAGER_0_DEVICE_ID
#define TMRMANAGER_IRPT_INTR	  XPAR_INTC_0_TMRMANAGER_0_VEC_ID

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif /* XPAR_INTC_0_DEVICE_ID */
#endif /* TESTAPP_GEN */

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the TMRManager device.
 */
#define TEST_BUFFER_SIZE		100


/**************************** Type Definitions *******************************/

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TMRManagerIntrExample(INTC *IntcInstancePtr,
			XTMRManager *TMRManagerInstancePtr,
			u16 TMRManagerDeviceId,
			u16 TMRManagerIntrId);

static void TMRManagerSendHandler(void *CallBackRef, unsigned int EventData);

static void TMRManagerRecvHandler(void *CallBackRef, unsigned int EventData);

static int TMRManagerSetupIntrSystem(INTC *IntcInstancePtr,
				XTMRManager *TMRManagerInstancePtr,
				u16 TMRManagerIntrId);

static void TMRManagerDisableIntrSystem(INTC *IntrInstancePtr,
				u16 TMRManagerIntrId);


/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs.
 */
#ifndef TESTAPP_GEN
static INTC IntcInstance;	/* The instance of the Interrupt Controller */
static XTMRManager TMRManagerInst;  /* The instance of the TMRManager Device */
#endif

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */

/*
 * The following buffer is used in this example to send data  with the TMRManager.
 */
u8 SendBuffer[TEST_BUFFER_SIZE];

/*
 * The following counter is used to determine when the entire buffer has
 * been sent.
 */
static volatile int TotalSentCount;


/******************************************************************************/
/**
*
* Main function to call the TMRManager interrupt example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Run the TMRManager Interrupt example , specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = TMRManagerIntrExample(&IntcInstance,
				 &TMRManagerInst,
				 TMRMANAGER_DEVICE_ID,
				 TMRMANAGER_IRPT_INTR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
*
* This function does a minimal test on the TMRManager device and driver as a
* design example. The purpose of this function is to illustrate how to use
* the XTMRManager component.
*
* This function sends data through the TMRManager.
*
* This function uses the interrupt driver mode of the TMRManager.  The calls to
* the  TMRManager driver in the interrupt handlers, should only use the
* non-blocking calls.
*
* @param	IntcInstancePtr is a pointer to the instance of INTC driver.
* @param	TMRManagerInstPtr is a pointer to the instance of TMRManager driver.
* @param	TMRManagerDeviceId is the Device ID of the TMRManager Device and
*		is the XPAR_<TMRMANAGER_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	TMRManagerIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<TMRMANAGER_instance>_VEC_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
* This function contains an infinite loop such that if interrupts are not
* working it may never return.
*
****************************************************************************/
int TMRManagerIntrExample(INTC *IntcInstancePtr,
			XTMRManager *TMRManagerInstPtr,
			u16 TMRManagerDeviceId,
			u16 TMRManagerIntrId)

{
	int Status;
	u32 Index;

	/*
	 * Initialize the TMRManager driver so that it's ready to use.
	 */
	Status = XTMRManager_Initialize(TMRManagerInstPtr, TMRManagerDeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XTMRManager_SelfTest(TMRManagerInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the TMRManager to the interrupt subsystem such that interrupts
	 * can occur. This function is application specific.
	 */
	Status = TMRManagerSetupIntrSystem(IntcInstancePtr,
					 TMRManagerInstPtr,
					 TMRManagerIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the TMRManager that will be called from the
	 * interrupt context when data has been sent and received,
	 * specify a pointer to the TMRManager driver instance as the callback
	 * reference so the handlers are able to access the instance data.
	 */
	XTMRManager_SetSendHandler(TMRManagerInstPtr, TMRManagerSendHandler,
							 TMRManagerInstPtr);
	XTMRManager_SetRecvHandler(TMRManagerInstPtr, TMRManagerRecvHandler,
							 TMRManagerInstPtr);

	/*
	 * Enable the interrupt of the TMRManager so that the interrupts
	 * will occur.
	 */
	XTMRManager_EnableInterrupt(TMRManagerInstPtr);

	/*
	 * Initialize the send buffer bytes with a pattern to send.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = Index;
	}

	/*
	 * Send the buffer using the TMRManager.
	 */
	XTMRManager_Send(TMRManagerInstPtr, SendBuffer, TEST_BUFFER_SIZE);

	/*
	 * Wait for the entire buffer to be transmitted,  the function may get
	 * locked up in this loop if the interrupts are not working correctly.
	 */
	while ((TotalSentCount != TEST_BUFFER_SIZE)) {
	}

	TMRManagerDisableIntrSystem(IntcInstancePtr, TMRManagerIntrId);

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
* This handler provides an example of how to handle data for the TMRManager, but
* is application specific.
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
static void TMRManagerSendHandler(void *CallBackRef, unsigned int EventData)
{
	TotalSentCount = EventData;
}

/****************************************************************************/
/**
*
* This function is the handler which performs processing to receive data from
* the TMRManager. It is called from an interrupt context such that the amount of
* processing performed should be minimized. It is called when any data is
* present in the receive FIFO of the TMRManager such that the data can be
* retrieved from the TMRManager. The amount of data present in the FIFO is not
* known when this function is called.
*
* This handler provides an example of how to handle data for the TMRManager, but
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver,
*		in this case it is the instance pointer for the TMRManager driver.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void TMRManagerRecvHandler(void *CallBackRef, unsigned int EventData)
{

}

/****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the TMRManager. This function is application specific since the actual
* system may or may not have an interrupt controller. The TMRManager could be
* directly connected to a processor without an interrupt controller. The
* user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of INTC driver.
* @param	TMRManagerInstPtr is a pointer to the instance of TMRManager driver.
*		XPAR_<TMRMANAGER_instance>_DEVICE_ID value from xparameters.h.
* @param	TMRManagerIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<TMRMANAGER_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int TMRManagerSetupIntrSystem(INTC *IntcInstancePtr,
				XTMRManager *TMRManagerInstPtr,
				u16 TMRManagerIntrId)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID

#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that it is ready
	 * to use.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the specific
	 * interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstancePtr, TMRManagerIntrId,
			(XInterruptHandler)XTMRManager_InterruptHandler,
			(void *)TMRManagerInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the core can cause interrupts thru the interrupt controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Enable the interrupt for the TMRManager.
	 */
	XIntc_Enable(IntcInstancePtr, TMRManagerIntrId);
#else

#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif /* TESTAPP_GEN */

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, TMRManagerIntrId,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, TMRManagerIntrId,
				 (Xil_ExceptionHandler)XTMRManager_InterruptHandler,
				 TMRManagerInstPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Timer device.
	 */
	XScuGic_Enable(IntcInstancePtr, TMRManagerIntrId);
#endif /* XPAR_INTC_0_DEVICE_ID */



#ifndef TESTAPP_GEN

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)INTC_HANDLER,
			IntcInstancePtr);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

#endif /* TESTAPP_GEN */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the TMRManager.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC driver.
* @param	TMRManagerIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<TMRMANAGER_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TMRManagerDisableIntrSystem(INTC *IntcInstancePtr,
					  u16 TMRManagerIntrId)
{

	/*
	 * Disconnect and disable the interrupt for the TMRManager.
	 */
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disconnect(IntcInstancePtr, TMRManagerIntrId);
#else
	XScuGic_Disable(IntcInstancePtr, TMRManagerIntrId);
	XScuGic_Disconnect(IntcInstancePtr, TMRManagerIntrId);

#endif

}
