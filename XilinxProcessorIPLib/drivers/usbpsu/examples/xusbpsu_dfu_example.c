/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xusbpsu_dfu_example.c
*
* This file implements the DFU class example.
* Please refer to DFU 1.1 specification for details.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   vak  30/11/16 First release
*
* </pre>
*
*****************************************************************************/
/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xscugic.h"
#include "xusbpsu.h"
#include "xusbpsu_ch9.h"
#include "xusbpsu_dfu.h"
#include "xusbpsu_ch9_dfu.h"

/************************** Constant Definitions ****************************/

#define USB_DEVICE_ID		XPAR_XUSBPSU_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define	USB_INTR_ID			XPAR_XUSBPS_0_INTR

/************************** Function Prototypes ******************************/

static int SetupInterruptSystem(u16 IntcDeviceID, XScuGic *IntcInstancePtr);

/************************** Variable Definitions *****************************/

struct XUsbPsu UsbInstance;

XUsbPsu_Config *UsbConfigPtr;

XScuGic InterruptController;  /* Interrupt controller instance */

u8 Phase;

/****************************************************************************/
/**
* This function is the main function of the DFU example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
*
*****************************************************************************/
int main(void)
{
	int Status;

	xil_printf("DFU Start...\r\n");

	UsbConfigPtr = XUsbPsu_LookupConfig(USB_DEVICE_ID);
	if (NULL == UsbConfigPtr)
		return XST_FAILURE;

	Status = XUsbPsu_CfgInitialize(&UsbInstance, UsbConfigPtr,
					UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status)
		return XST_FAILURE;

	/* Initialize DFU here */
	dfu_if_init(&UsbInstance);

	/* setup interrupts */
	Status = SetupInterruptSystem(INTC_DEVICE_ID, &InterruptController);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Start the controller so that Host can see our device */
	XUsbPsu_Start(&UsbInstance);

	while (1) {
		/* Rest is taken care by interrupts */
		;
	}
}

/****************************************************************************/
/**
* This function setups the interrupt system such that interrupts can occur.
* This function is application specific since the actual system may or may not
* have an interrupt controller.  The USB controller could be
* directly connected to aprocessor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param	IntcDeviceID is the unique ID of the interrupt controller
* @param	IntcInstacePtr is a pointer to the interrupt controller
*			instance.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
static int SetupInterruptSystem(u16 IntcDeviceID, XScuGic *IntcInstancePtr)
{
	int Status;
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver
	 */
	IntcConfig = XScuGic_LookupConfig(IntcDeviceID);
	if (NULL == IntcConfig)
		return XST_FAILURE;

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
						IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Connect to the interrupt controller
	 */
	Status = XScuGic_Connect(&InterruptController, USB_INTR_ID,
				(Xil_ExceptionHandler)XUsbPsu_IntrHandler,
				(void *)&UsbInstance);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Enable the interrupt for the USB
	 */
	XScuGic_Enable(&InterruptController, USB_INTR_ID);

	/*
	 * Enable interrupts for Reset, Disconnect, ConnectionDone, Link State
	 * Wakeup and Overflow events.
	 */
	XUsbPsu_EnableIntr(&UsbInstance, XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
			XUSBPSU_DEVTEN_WKUPEVTEN |
			XUSBPSU_DEVTEN_ULSTCNGEN |
			XUSBPSU_DEVTEN_CONNECTDONEEN |
			XUSBPSU_DEVTEN_USBRSTEN |
			XUSBPSU_DEVTEN_DISCONNEVTEN);
	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				IntcInstancePtr);

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
