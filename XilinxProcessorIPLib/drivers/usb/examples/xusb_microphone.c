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
 * @file xusb_microphone.c
 *
 * This file contains usb Microphone application related functions.
 * This example only provides a reference as to how an isochronous transfer
 * related application can be written. This example emulates a microphone and
 * when connected to a Windows PC will be detected as a Microphone and
 * if we open sound recorder and start recording data, this example sends
 * data on isochronous endpoint and the PC can store this data.
 * Noise will be heard when this data is played on the PC.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -----------------------------------------------------------------
 * 1.00a hvm  12/20/10 First release
 * 4.01a bss  11/01/11 Modified UsbIfIntrHandler function to unconditionally
 *			reset when USB reset is asserted (CR 627574).
 * 4.02a bss  02/20/12 Modified main function to call Xil_DCacheFlushRange
 * 			when DMA is enabled.(CR 640005)
 *
 * </pre>
 *****************************************************************************/
/***************************** Include Files *********************************/

#include "xusb.h"
#include "xintc.h"
#include "xusb_microphone.h"
#include "stdio.h"
#include "data.h"
#include "xil_exception.h"
#include "xil_cache.h"

/************************** Constant Definitions *****************************/

#define USB_DEVICE_ID		XPAR_USB_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define USB_INTR		XPAR_INTC_0_USB_0_VEC_ID

static int SetupInterruptSystem(XUsb * InstancePtr);

/************************** Variable Definitions *****************************/

static XUsb UsbInstance;		/* The instance of the USB device */

XUsb_Config *UsbConfigPtr;	/* Pointer to the USB config structure */

XIntc InterruptController;	/* Instance of the Interrupt Controller */

/*****************************************************************************/
/**
 * This main function starts the USB Intrerrupt example.
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
	int Index = 0;
	int Cnt = 0;
	/*
	 * Initialize the USB driver.
	 */
	UsbConfigPtr = XUsb_LookupConfig(USB_DEVICE_ID);
	if (UsbConfigPtr == NULL) {
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
	Status = XUsb_CfgInitialize(&UsbInstance,
					UsbConfigPtr,
					UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the USB instance as required for the
	 * application.
	 */
	InitUsbInterface(&UsbInstance);

	/*
	 * Set our function address to 0 which is the unenumerated state.
	 */
	Status = XUsb_SetDeviceAddress(&UsbInstance, 0);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
	 * Setup the interrupt handlers.
	 */
	XUsb_IntrSetHandler(&UsbInstance, (void *) UsbIfIntrHandler,
			    &UsbInstance);

	XUsb_EpSetHandler(&UsbInstance, 0,
			  (XUsb_EpHandlerFunc *) Ep0IntrHandler, &UsbInstance);

	XUsb_EpSetHandler(&UsbInstance, 1,
			  (XUsb_EpHandlerFunc *) Ep1IntrHandler, &UsbInstance);

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
			XUSB_STATUS_RESET_MASK |
			XUSB_STATUS_SUSPEND_MASK |
			XUSB_STATUS_DISCONNECT_MASK |
			XUSB_STATUS_FIFO_BUFF_RDY_MASK |
			XUSB_STATUS_FIFO_BUFF_FREE_MASK |
			XUSB_STATUS_EP0_BUFF1_COMP_MASK |
			XUSB_STATUS_EP1_BUFF1_COMP_MASK |
			XUSB_STATUS_EP2_BUFF1_COMP_MASK |
			XUSB_STATUS_EP1_BUFF2_COMP_MASK |
			XUSB_STATUS_EP2_BUFF2_COMP_MASK);

	XUsb_Start(&UsbInstance);

	/*
	 * Set the device configuration to unenumerated state.
	 */
	UsbInstance.DeviceConfig.CurrentConfiguration = 0;

	/*
	 * Wait until the USB device is enumerated.
	 */
	while (!UsbInstance.DeviceConfig.CurrentConfiguration);


	/*
	 * Stop the test if the Stop key is pressed or
	 * device lost enumeration.
	 */
	while (1) {

		if (UsbInstance.Config.DmaEnabled) {
			/* Flush the cache before DMA transfer */
			Xil_DCacheFlushRange((u32)&Hello_wav[Index],(u32)1024);
		}

		if (XUsb_EpDataSend(&UsbInstance, 1, &Hello_wav[Index],
					1024) == XST_SUCCESS){
				Index += 1024;
				Cnt ++;
				if (Cnt >= 9000){
					Cnt =0;
					Index = 0;
				}
			}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This is the USB initialization function. This example initializes the device
 * for Microphone Application. The following configuration is done.
 *	- EP0 : CONTROL end point, Bidirectional, Packet size 64 bytes.
 *	- EP1 : ISOCHRONOUS, BULK_IN, packet size 1024 bytes.
 *
 *
 * @param	InstancePtr is a pointer to the XUsb instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void InitUsbInterface(XUsb * InstancePtr)
{

	XUsb_DeviceConfig DeviceConfig;

	/*
	 * Setup Endpoint 0.
	 */
	DeviceConfig.Ep[0].RamBase = 0x22;
	DeviceConfig.Ep[0].Size = 0x40;
	DeviceConfig.Ep[0].EpType = 0;
	DeviceConfig.Ep[0].OutIn = XUSB_EP_DIRECTION_OUT;


	/*
	 * Setup EP 1  1024 byte packets, ISOCHRONOUS IN.
	 */
	DeviceConfig.Ep[1].RamBase = 0x1000;
	DeviceConfig.Ep[1].Size = 0x400;
	DeviceConfig.Ep[1].EpType = 1;
	DeviceConfig.Ep[1].OutIn = XUSB_EP_DIRECTION_IN;


	DeviceConfig.NumEndpoints = 2;

	/*
	 * Initialize the device configuration.
	 */
	XUsb_ConfigureDevice(InstancePtr, &DeviceConfig);

	XUsb_EpEnable(InstancePtr, 0);
	XUsb_EpEnable(InstancePtr, 1);

	MaxControlSize = 64;

	/*
	 * Store the actual RAM address offset in the device structure, so as to
	 * avoid the multiplication during processing.
	 */
	InstancePtr->DeviceConfig.Ep[1].RamBase <<= 2;

}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB microphone device
 * application.
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
void UsbIfIntrHandler(void *CallBackRef, u32 IntrStatus)
{

	XUsb *InstancePtr;
	u8 Index;

	InstancePtr = (XUsb *) CallBackRef;

	if (IntrStatus & XUSB_STATUS_RESET_MASK) {

			XUsb_Stop(InstancePtr);
			InstancePtr->DeviceConfig.CurrentConfiguration = 0;
			InstancePtr->DeviceConfig.Status = XUSB_RESET;
			for (Index = 0; Index < 3; Index++) {
				XUsb_WriteReg(InstancePtr->Config.BaseAddress,
					       InstancePtr->
					       EndPointOffset[Index], 0);
			}
			/*
			 * Re-initialize the device and set the device address
			 * to 0 and re-start the device.
			 */
			InitUsbInterface(InstancePtr);
			XUsb_SetDeviceAddress(InstancePtr, 0);
			XUsb_Start(InstancePtr);

		XUsb_IntrDisable(InstancePtr, XUSB_STATUS_RESET_MASK);
		XUsb_IntrEnable(InstancePtr, (XUSB_STATUS_DISCONNECT_MASK |
					      XUSB_STATUS_SUSPEND_MASK));
	}
	if (IntrStatus & XUSB_STATUS_SUSPEND_MASK) {
		/*
		 * Process the suspend event.
		 */
		XUsb_IntrDisable(InstancePtr, XUSB_STATUS_SUSPEND_MASK);
		XUsb_IntrEnable(InstancePtr, (XUSB_STATUS_RESET_MASK |
					      XUSB_STATUS_DISCONNECT_MASK));
	}

}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB End point Zero events.
 *
 *
 * @param	CallBackRef is the callback reference passed from the interrupt.
 *		handler, which in our case is a pointer to the driver instance.
 * @param	EpNum is the end point number.
 * @param	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return	None.
 *
 * @note	EpNum is not used in this function as the handler is attached
 *		specific to end point zero. This parameter is useful when a
 *		single handler is used for processing all end point interrupts.
 *
 ******************************************************************************/
void Ep0IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus)
{

	XUsb *InstancePtr;
	int SetupRequest;
	u32 EpCfgReg;

	InstancePtr = (XUsb *) CallBackRef;

	/*
	 * Process the end point zero buffer interrupt.
	 */
	if (IntrStatus & XUSB_BUFFREADY_EP0_BUFF_MASK) {
		if (IntrStatus & XUSB_STATUS_SETUP_PACKET_MASK) {
			/*
			 * Received a setup packet. Execute the chapter 9
			 * command.
			 */
			XUsb_IntrEnable(InstancePtr,
					(XUSB_STATUS_DISCONNECT_MASK |
					 XUSB_STATUS_SUSPEND_MASK |
					 XUSB_STATUS_RESET_MASK));
			SetupRequest = Chapter9(InstancePtr);
			if (SetupRequest != XST_SUCCESS) {
				/*
				 * Unsupported command. Simple Acknowledge.
				 */
				xil_printf("unsuppported %d\r\n", SetupRequest);

				EpCfgReg = XUsb_ReadReg(
					InstancePtr->Config.BaseAddress,
					 InstancePtr->EndPointOffset[0]);
				EpCfgReg |= XUSB_EP_CFG_DATA_TOGGLE_MASK;

				XUsb_WriteReg(InstancePtr->Config.BaseAddress,
					InstancePtr->EndPointOffset[0],
					EpCfgReg);
				XUsb_WriteReg(InstancePtr->Config.BaseAddress,
					XUSB_BUFFREADY_OFFSET, 1);

			}
		}
		else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_RDY_MASK) {
			EP0ProcessOutToken(InstancePtr);
		}
		else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_FREE_MASK) {
			EP0ProcessInToken(InstancePtr);
		}
	}
}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB End point one events.
 *
 * @param	CallBackRef is the callback reference passed from the interrupt
 *		handler, which in our case is a pointer to the driver instance.
 * @param	EpNum is the end point number.
 * @param	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return	None.
 *
 * @note	EpNum is not used in this function as the handler is attached
 *		specific to end point one. This parameter is useful when a
 *		single handler is used for processing all end point interrupts.
 *
 ******************************************************************************/
void Ep1IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus)
{

	XUsb *InstancePtr;

	InstancePtr = (XUsb *) CallBackRef;

	/*
	 * Process the End point 1 interrupts.
	 */
	if (IntrStatus & XUSB_BUFFREADY_EP1_BUFF1_MASK) {
			InstancePtr->DeviceConfig.Ep[1].Buffer0Ready = 0;
	}

	if (IntrStatus & XUSB_BUFFREADY_EP1_BUFF2_MASK) {
			InstancePtr->DeviceConfig.Ep[1].Buffer1Ready = 0;
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

