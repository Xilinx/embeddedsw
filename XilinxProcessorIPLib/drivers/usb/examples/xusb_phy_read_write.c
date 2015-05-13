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
 * @file xusb_phy_read_write.c
 *
 * This file contains PHY register access related example.
 *
 * @note	This example only shows reading and writing to the ULPI PHY
 *		SCRATCH register.The user can access any other register as per
 *		their requirement.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -----------------------------------------------------------------
 * 1.00a hvm  12/3/10 First release
 *
 * </pre>
 *****************************************************************************/
/***************************** Include Files *********************************/

#include "xusb.h"
#include "xintc.h"
#include "stdio.h"
#include "xenv_standalone.h"
#include "xil_exception.h"
#include "xil_cache.h"

/************************** Constant Definitions *****************************/

#define USB_DEVICE_ID		XPAR_USB_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define USB_INTR		XPAR_INTC_0_USB_0_VEC_ID

#define ULPI_SCRATCH_REGISTER	0x16
#define WRITE_REG_DATA		0xAA

/************************** Variable Definitions *****************************/

XUsb UsbInstance;		/* The instance of the USB device */
XUsb_Config *UsbConfigPtr;	/* Instance of the USB config structure */

XIntc InterruptController;	/* Instance of the Interrupt Controller */

volatile u8 PhyAccessDone = 0;

void UsbIfPhyIntrHandler(void *CallBackRef, u32 IntrStatus);
static int SetupInterruptSystem(XUsb * InstancePtr);

/*****************************************************************************/
/**
 * This main function starts the USB application.
 *
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if test fails.
 * @note	None.
 *
 *****************************************************************************/
int main()
{
	int Status;
	u32 ReadRegData = 0;
	/*
	 * Initialize the USB driver.
	 */
	UsbConfigPtr = XUsb_LookupConfig(USB_DEVICE_ID);
	if (NULL == UsbConfigPtr) {
		return XST_FAILURE;
	}
#ifdef __PPC__

	Xil_ICacheEnableRegion (0x80000001);
	Xil_DCacheEnableRegion (0x80000001);
#endif
#ifdef __MICROBLAZE__
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();


	Xil_DCacheInvalidate();
	Xil_DCacheEnable();
#endif

	/*
	 * We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example. For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = XUsb_CfgInitialize(&UsbInstance,
				    UsbConfigPtr, UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	XUsb_UlpiIntrSetHandler (&UsbInstance, (void *) UsbIfPhyIntrHandler,
			    &UsbInstance);
	/*
	 * Setup the interrupt system.
	 */
	Status = SetupInterruptSystem(&UsbInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupts.
	 */
	XUsb_IntrEnable(&UsbInstance, XUSB_STATUS_GLOBAL_INTR_MASK |
			XUSB_STATUS_PHY_ACCESS_MASK);

	XUsb_Start(&UsbInstance);


	/*
	 * Initiate a ULPI register write transaction.
	 */
	XUsb_UlpiPhyWriteRegister(&UsbInstance, ULPI_SCRATCH_REGISTER,
					WRITE_REG_DATA);


	/* Wait until the write transaction is done */
	while (!PhyAccessDone);

	/*
	 * Read the PHY read register.  We do not wait for transaction
	 * complete interrupt in this case. The API internally polls for the
	 * completion and then returns the register value read.
 	 */
	ReadRegData = XUsb_UlpiPhyReadRegister(&UsbInstance,
					ULPI_SCRATCH_REGISTER);


	/* Compare the Written data and read data*/
	if (ReadRegData != WRITE_REG_DATA) {

		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
 * This function is the ULPI PHY interrupt handler
 *
 *
 * @param    	CallBackRef is the callback reference passed from the interrupt
 *           	handler, which in our case is a pointer to the driver instance.
 * @param    	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return   	None.
 *
 * @note        None.
 *
 ******************************************************************************/
void UsbIfPhyIntrHandler(void *CallBackRef, u32 IntrStatus)
{

	XUsb *InstancePtr;

	InstancePtr = (XUsb *) CallBackRef;


	if (IntrStatus & XUSB_STATUS_PHY_ACCESS_MASK) {

		PhyAccessDone = 1;
	}

}

/******************************************************************************/
/**
*
* This function sets up the interrupt system such that interrupts can occur
* for the USB. This function is application specific since the actual
* system may or may not have an interrupt controller. The USB could be
* directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	InstancePtr contains a pointer to the instance of the USB
*		component, which is going to be connected to the interrupt
*		controller.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE. if it fails.
*
* @note		None
*
*******************************************************************************/
static int SetupInterruptSystem(XUsb * InstancePtr)
{
	int Status;

	/*
	 * Initialize the interrupt controller driver.
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the USB device occurs.
	 */
	Status = XIntc_Connect(&InterruptController, USB_INTR,
			       (XInterruptHandler) XUsb_IntrHandler,
			       (void *) InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the USB can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the USB.
	 */
	XIntc_Enable(&InterruptController, USB_INTR);

	/*
	 * Initialize the exception table
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				&InterruptController);

	/*
	 * Enable non-critical exceptions
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}


