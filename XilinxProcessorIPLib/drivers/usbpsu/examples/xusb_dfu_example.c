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
/****************************************************************************/
/**
 *
 * @file xusb_intr_example.c
 *
 * This file implements DFU class example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   vak  30/11/16 First release
 * 1.4	 BK   12/01/18 Renamed the file to be in sync with usb common code
 *		       changes for all USB IPs
 *	 vak  22/01/18 Added changes for supporting microblaze platform
 *	 vak  13/03/18 Moved the setup interrupt system calls from driver to
 *		       example.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xusb_ch9_dfu.h"
#include "xusb_class_dfu.h"
#include "xusb_wrapper.h"
#include "xil_exception.h"

#ifdef __MICROBLAZE__
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#endif /* XPAR_INTC_0_DEVICE_ID */
#elif defined PLATFORM_ZYNQMP
#include "xscugic.h"
#endif

/************************** Constant Definitions ****************************/

/************************** Function Prototypes ******************************/
static s32 SetupInterruptSystem(struct XUsbPsu *InstancePtr, u16 IntcDeviceID,
		u16 USB_INTR_ID, void *IntcPtr);

/************************** Variable Definitions *****************************/
struct Usb_DevData UsbInstance;

Usb_Config *UsbConfigPtr;

#ifdef __MICROBLAZE__
#ifdef XPAR_INTC_0_DEVICE_ID
XIntc	InterruptController;	/*XIntc interrupt controller instance */
#endif /* XPAR_INTC_0_DEVICE_ID */
#else
XScuGic	InterruptController;	/* Interrupt controller instance */
#endif

#ifdef __MICROBLAZE__		/* MICROBLAZE */
#ifdef	XPAR_INTC_0_DEVICE_ID
#define	INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define	USB_INT_ID		XPAR_AXI_INTC_0_ZYNQ_ULTRA_PS_E_0_PS_PL_IRQ_USB3_0_ENDPOINT_0_INTR
#endif /* MICROBLAZE */
#elif	defined	PLATFORM_ZYNQMP	/* ZYNQMP */
#define	INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define	USB_INT_ID		XPAR_XUSBPS_0_INTR
#define	USB_WAKEUP_INTR_ID	XPAR_XUSBPS_0_WAKE_INTR
#else	/* OTHERS */
#define	INTC_DEVICE_ID		0
#define	USB_INT_ID		0
#endif

u8 VirtFlash[0x10000000];

struct dfu_if DFU;		/* DFU instance structure*/

/* Initialize a DFU data structure */
static USBCH9_DATA dfu_data = {
	.ch9_func = {
		.Usb_Ch9SetupDevDescReply = Usb_Ch9SetupDevDescReply,
		.Usb_Ch9SetupCfgDescReply = Usb_Ch9SetupCfgDescReply,
		.Usb_Ch9SetupBosDescReply = Usb_Ch9SetupBosDescReply,
		.Usb_Ch9SetupStrDescReply = Usb_Ch9SetupStrDescReply,
		.Usb_SetConfiguration = Usb_SetConfiguration,
		.Usb_SetConfigurationApp = Usb_SetConfigurationApp,
		/* hook the set interface handler */
		.Usb_SetInterfaceHandler = Usb_DfuSetIntf,
		/* hook up storage class handler */
		.Usb_ClassReq = Usb_DfuClassReq,
		/* Set the DFU address for call back */
	},
	.data_ptr = (void *)&DFU,
};

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
	s32 Status;

	xil_printf("DFU Start...\r\n");

	/* Initialize the USB driver so that it's ready to use,
	 * specify the controller ID that is generated in xparameters.h
	 */
	UsbConfigPtr = LookupConfig(USB_DEVICE_ID);
	if (NULL == UsbConfigPtr)
		return XST_FAILURE;

	/* We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example.  For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = CfgInitialize(&UsbInstance, UsbConfigPtr,
					UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status)
		return XST_FAILURE;

	/* hook up chapter9 handler */
	Set_Ch9Handler(UsbInstance.PrivateData, Ch9Handler);

	/* Set the disconnect event handler */
	Set_Disconnect(UsbInstance.PrivateData, Usb_DfuDisconnect);

	/* Set the reset event handler */
	Set_RstHandler(UsbInstance.PrivateData, Usb_DfuReset);

	/* Assign the data to usb driver */
	Set_DrvData(UsbInstance.PrivateData, &dfu_data);

	/* Initialize the DFU instance structure */
	DFU.InstancePtr = &UsbInstance;
	/* Set DFU state to APP_IDLE */
	Usb_DfuSetState(&DFU, STATE_APP_IDLE);
	/* Set the DFU descriptor pointers, so we can use it when in DFU mode */
	DFU.total_transfers = 0;
	DFU.total_bytes_dnloaded = 0;
	DFU.total_bytes_uploaded = 0;

	/* setup interrupts */
	Status = SetupInterruptSystem(UsbInstance.PrivateData, INTC_DEVICE_ID,
					USB_INT_ID, (void *)&InterruptController);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstance.PrivateData);

	while (1) {
		/* Rest is taken care by interrupts */
	}
}

/****************************************************************************/
/**
* This function setups the interrupt system such that interrupts can occur.
* This function is application specific since the actual system may or may not
* have an interrupt controller.  The USB controller could be
* directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	IntcDeviceID is the unique ID of the interrupt controller
* @param	USB_INTR_ID is the interrupt ID of the usb controller
* @param	IntcPtr is a pointer to the interrupt controller
*			instance.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
static s32 SetupInterruptSystem(struct XUsbPsu *InstancePtr, u16 IntcDeviceID,
		u16 USB_INTR_ID, void *IntcPtr)
{
	/*
	 * This below is done to remove warnings which occur when usbpsu
	 * driver is compiled for platforms other than MICROBLAZE or ZYNQMP
	 */
	(void)InstancePtr;
	(void)IntcDeviceID;
	(void)IntcPtr;

#ifdef __MICROBLAZE__
#ifdef XPAR_INTC_0_DEVICE_ID
	s32 Status;

	XIntc *IntcInstancePtr = (XIntc *)IntcPtr;

	/*
	 * Initialize the interrupt controller driver.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, IntcDeviceID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the USB device occurs.
	 */
	Status = XIntc_Connect(IntcInstancePtr, USB_INTR_ID,
			       (Xil_ExceptionHandler)XUsbPsu_IntrHandler,
			       (void *) InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the USB can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the USB.
	 */
	XIntc_Enable(IntcInstancePtr, USB_INTR_ID);

	/*
	 * Initialize the exception table
	 */
	Xil_ExceptionInit();

	/*
	 * Enable interrupts for Reset, Disconnect, ConnectionDone, Link State
	 * Wakeup and Overflow events.
	 */
	XUsbPsu_EnableIntr(InstancePtr, XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
                        XUSBPSU_DEVTEN_WKUPEVTEN |
                        XUSBPSU_DEVTEN_ULSTCNGEN |
                        XUSBPSU_DEVTEN_CONNECTDONEEN |
                        XUSBPSU_DEVTEN_USBRSTEN |
                        XUSBPSU_DEVTEN_DISCONNEVTEN);

	/*
	 * Register the interrupt controller handler with the exception table
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				IntcInstancePtr);
#endif /* XPAR_INTC_0_DEVICE_ID */
#elif defined PLATFORM_ZYNQMP
	s32 Status;

	XScuGic_Config *IntcConfig; /* The configuration parameters of the
					interrupt controller */

	XScuGic *IntcInstancePtr = (XScuGic *)IntcPtr;

	/*
	 * Initialize the interrupt controller driver
	 */
	IntcConfig = XScuGic_LookupConfig(IntcDeviceID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
								   IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect to the interrupt controller
	 */
	Status = XScuGic_Connect(IntcInstancePtr, USB_INTR_ID,
							(Xil_ExceptionHandler)XUsbPsu_IntrHandler,
							(void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#ifdef XUSBPSU_HIBERNATION_ENABLE
	Status = XScuGic_Connect(IntcInstancePtr, USB_WAKEUP_INTR_ID,
							(Xil_ExceptionHandler)XUsbPsu_WakeUpIntrHandler,
							(void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Enable the interrupt for the USB
	 */
	XScuGic_Enable(IntcInstancePtr, USB_INTR_ID);
#ifdef XUSBPSU_HIBERNATION_ENABLE
	XScuGic_Enable(IntcInstancePtr, USB_WAKEUP_INTR_ID);
#endif

	/*
	 * Enable interrupts for Reset, Disconnect, ConnectionDone, Link State
	 * Wakeup and Overflow events.
	 */
	XUsbPsu_EnableIntr(InstancePtr, XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
                        XUSBPSU_DEVTEN_WKUPEVTEN |
                        XUSBPSU_DEVTEN_ULSTCNGEN |
                        XUSBPSU_DEVTEN_CONNECTDONEEN |
                        XUSBPSU_DEVTEN_USBRSTEN |
                        XUSBPSU_DEVTEN_DISCONNEVTEN);

#ifdef XUSBPSU_HIBERNATION_ENABLE
	if (InstancePtr->HasHibernation)
		XUsbPsu_EnableIntr(InstancePtr,
				XUSBPSU_DEVTEN_HIBERNATIONREQEVTEN);
#endif

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
								(Xil_ExceptionHandler)XScuGic_InterruptHandler,
								IntcInstancePtr);
#endif /* PLATFORM_ZYNQMP */

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
