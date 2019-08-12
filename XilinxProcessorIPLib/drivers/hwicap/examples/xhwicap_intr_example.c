/******************************************************************************
*
* Copyright (C) 2007 - 2019 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xhwicap_intr_example.c
*
*
* This file contains a design example using the HwIcap driver (XHwIcap) and
* the HwIcap device using the interrupt mode.
*
* @note		None.
*
*
* *****WARNING ************
*
* This is not an example that can be used directly. This example gives the
* sequence of steps that need to be done for programming the FPGA with a given
* configuration(partial) file. This configuration file differs from each system
* and each version of the FPGA device.
*
*<pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 2.00a sv   09/29/07 First release
* 4.00a hvm  12/1/09  Updated with HAL phase 1 changes
* 5.00a hvm  2/25/10  Updated with S6 support
* 10.0  bss  6/24/14  Removed support for families older than 7 series
* 11.0  ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 11.2 Nava 30/01/19  Replaced #error with #pragma message since the information
*                     related to #error is not an error info. It provides an
*		      information about the example behaviour.
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* XPAR parameters */
#include "xhwicap.h"		/* HwIcap device driver */
#include "xintc.h"		/* Interrupt controller device driver */
#include "xil_exception.h"      /* Exceptions */
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define HWICAP_DEVICE_ID		XPAR_HWICAP_0_DEVICE_ID
#define INTC_DEVICE_ID			XPAR_INTC_0_DEVICE_ID
#define HWICAP_IRPT_INTR		XPAR_INTC_0_HWICAP_0_VEC_ID

/*
 *  This is the size of the buffer in words which is written in this example.
 */
#define TEST_WRITE_BUFFER_SIZE		4096


#pragma message ("This is an example to show the usage of the device/driver in Interrupt \
	mode. The user has to give a valid partial bitstream as an input to \
	the example otherwise it may overwrite the existing configuration \
	data in the device and result in unpredictable behaviour")


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int HwIcapIntrExample(XIntc *IntcInstancePtr,
			XHwIcap *HwIcapInstancePtr,
			u16 HwIcapDeviceId,
			u16 HwIcapIntrId);

void HwIcapIntrHandler(void *CallBackRef, u32 StatusEvent, u32 WordCount);

static int HwIcapSetupInterruptSystem(XIntc* IntcInstancePtr,
				      XHwIcap *HwIcapPtr,
				      u16 IntrId );

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that the
 * are initialized to zero each time the program runs.
 */
static XIntc IntcInstance;	/* The instance of the Interrupt Controller */
static XHwIcap  HwIcapInstance;	/* The instance of the HwIcap device */

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int TransferInProgress;

/*
 * The following variable tracks any errors that occur during interrupt
 * processing
 */
int Error;

/*
 * The following variables are used to read and write to the  HwIcap device, they
 * are global to avoid having large buffers on the stack.
 */
u32 WriteBuffer[TEST_WRITE_BUFFER_SIZE];


/*****************************************************************************/
/**
*
* Main function to call the HwIcap interrupt example.
*
* @param    None
*
* @return   XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note     None
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the HwIcap Interrupt example.
	 */
	Status = HwIcapIntrExample(&IntcInstance, &HwIcapInstance,
			    HWICAP_DEVICE_ID,
			    HWICAP_IRPT_INTR);
	if (Status != XST_SUCCESS) {
		xil_printf("Hwicap interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Hwicap interrupt Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the HwIcap device and driver as a
* design example. The purpose of this function is to illustrate how to use
* the XHwIcap component using the interrupt mode.
*
* This function sends data and expects to receive the same data.
*
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC component.
* @param	HwIcapInstancePtr is a pointer to the instance of HwIcap component.
* @param	HwIcapDeviceId is the Device ID of the HwIcap Device and is the
*		XPAR_<HWICAP_instance>_DEVICE_ID value from xparameters.h.
* @param	HwIcapIntrId is the interrupt Id and is typically
*		XPAR_<INTC_instance>_<HWICAP_instance>_IP2INTC_IRPT_INTR
*		value from xparameters.h .
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
* This function contains an infinite loop such that if interrupts are not
* working it may never return.
*
******************************************************************************/
int HwIcapIntrExample(XIntc *IntcInstancePtr, XHwIcap *HwIcapInstancePtr,
			u16 HwIcapDeviceId, u16 HwIcapIntrId)
{
	int Status;
	u32 Count;
	u8 Test;
	XHwIcap_Config *ConfigPtr;

	/*
	 * Initialize the HwIcap driver.
	 */
	ConfigPtr = XHwIcap_LookupConfig(HwIcapDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XHwIcap_CfgInitialize(HwIcapInstancePtr,
					ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XHwIcap_SelfTest(HwIcapInstancePtr);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the HwIcap device to the interrupt subsystem such that
	 * interrupts can occur. This function is application specific.
	 */
	Status = HwIcapSetupInterruptSystem(IntcInstancePtr,
					HwIcapInstancePtr,
					HwIcapIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the HwIcap that will be called from the
	 * interrupt context when an HwIcap status occurs, specify a pointer
	 * to the HwIcap driver instance as the callback reference so the
	 * handler is able to access the instance data.
	 */
	XHwIcap_SetInterruptHandler(HwIcapInstancePtr, HwIcapInstancePtr,
			 (XHwIcap_StatusHandler)HwIcapIntrHandler);

	/*
	 * Initialize the write buffer with pattern to write.
	 */
	for (Count = 0; Count < TEST_WRITE_BUFFER_SIZE;) {

		WriteBuffer[Count++] = XHI_DUMMY_PACKET;
		WriteBuffer[Count++] = XHI_SYNC_PACKET;
		WriteBuffer[Count++] = XHwIcap_Type1Read(XHI_IDCODE) | 1;
		WriteBuffer[Count++] = XHI_NOOP_PACKET;
		WriteBuffer[Count++] = XHI_NOOP_PACKET;
		WriteBuffer[Count++] = XHI_DUMMY_PACKET;
		WriteBuffer[Count++] = XHI_SYNC_PACKET;
		WriteBuffer[Count++] = XHwIcap_Type1Read(XHI_COR) | 1;
		WriteBuffer[Count++] = XHI_NOOP_PACKET;
		WriteBuffer[Count++] = XHI_NOOP_PACKET;
	}

	/*
	 * Enable the Write FIFO Half Full Interrupt.
	 */
	XHwIcap_IntrEnable(HwIcapInstancePtr, XHI_IPIXR_WRP_MASK);

	/*
	 * Write the the data to the device.
	 */
	TransferInProgress = TRUE;
	Status = XHwIcap_DeviceWrite(HwIcapInstancePtr,
					(u32 *) &WriteBuffer[0],
					TEST_WRITE_BUFFER_SIZE);
	if (Status != XST_SUCCESS)  {
		return XST_FAILURE;
	}

	/*
	 * Wait for the data to be written to the device.
	 */
	while (TransferInProgress);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the handler which performs processing for the HwIcap driver.
* It is called from an interrupt context such that the amount of processing
* performed should be minimized. It is called when a transfer of HwIcap data
* completes or an error occurs.
*
* This handler provides an example of how to handle HwIcap interrupts and
* is application specific.
*
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
* @param	StatusEvent is the event that just occurred.
* @param	WordCount is the number of words (32 bit) transferred up until
*		the event occurred.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void HwIcapIntrHandler(void *CallBackRef, u32 StatusEvent, u32 ByteCount)
{
	/*
	 * Indicate the transfer between the HwIcap to the Icap device is done
	 * regardless of the status event.
	 */
	TransferInProgress = FALSE;

	/*
	 * If the event was not transfer done, then track it as an error.
	 */
	if (StatusEvent != XST_HWICAP_WRITE_DONE) {
		Error++;
	}
}

/****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* HwIcap device.  The function is application-specific since the actual
* system may or may not have an interrupt controller. The HwIcap device
* could be directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance
* @param	HwIcapPtr is a pointer to the driver instance for the System
* 		Monitor device which is going to be connected to the interrupt
*		controller
* @param	DeviceId is the XPAR_<HWICAP_INSTANCE>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS if successful, or XST_FAILURE.
*
* @note		None.
*
*
****************************************************************************/
static int HwIcapSetupInterruptSystem(XIntc* IntcInstancePtr,
				      XHwIcap *HwIcapPtr,
				      u16 IntrId )
{
	int Status;

	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the handler that will be called when an interrupt
	 * for the device occurs, the handler defined above performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstancePtr,
		 		IntrId,
				(XInterruptHandler) XHwIcap_IntrHandler,
				HwIcapPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts. Specify real mode so that the
	 * HwIcap device can cause interrupts through the interrupt
	 * controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the HwIcap device.
	 */
	XIntc_Enable(IntcInstancePtr, IntrId);

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) XIntc_InterruptHandler,
				IntcInstancePtr);
	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

