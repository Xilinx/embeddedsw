/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
 * 1.5	 vak  13/02/19 Added support for versal
 * 1.8   pm   15/09/20 Fixed C++ Compilation error.
 * 1.14  pm   21/06/23 Added support for system device-tree flow.
 * 1.17  ka   20/01/25 Fixed C++ warnings and errors
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/

#include "xusb_ch9_dfu.h"
#include "xusb_class_dfu.h"
#include "xusb_wrapper.h"
#include "xil_exception.h"

#include "xparameters.h"

#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

#ifndef SDT
#ifdef __MICROBLAZE__
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#endif /* XPAR_INTC_0_DEVICE_ID */
#elif defined (PLATFORM_ZYNQMP) || defined (versal)
#include "xscugic.h"
#endif
#else
#define INTRNAME_DWC3USB3	0 /* Interrupt-name - USB */
#define INTRNAME_HIBER		2 /* Interrupt-name - Hiber */
#define XUSBPSU_BASEADDRESS	XPAR_XUSBPSU_0_BASEADDR /* USB base address */
#endif

/************************** Constant Definitions ****************************/

/************************** Function Prototypes ******************************/
#ifndef SDT
static s32 SetupInterruptSystem(struct XUsbPsu *InstancePtr, u16 IntcDeviceID,
				u16 USB_INTR_ID, void *IntcPtr);
#endif

/************************** Variable Definitions *****************************/
struct Usb_DevData UsbInstance;

Usb_Config *UsbConfigPtr;

#ifndef SDT
#ifdef __MICROBLAZE__
#ifdef XPAR_INTC_0_DEVICE_ID
XIntc	InterruptController;	/*XIntc interrupt controller instance */
#endif /* XPAR_INTC_0_DEVICE_ID */
#else
XScuGic	InterruptController;	/* Interrupt controller instance */
#endif
#endif

#ifndef SDT
#ifdef __MICROBLAZE__		/* MICROBLAZE */
#ifdef	XPAR_INTC_0_DEVICE_ID
#define	INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define	USB_INT_ID		XPAR_AXI_INTC_0_ZYNQ_ULTRA_PS_E_0_PS_PL_IRQ_USB3_0_ENDPOINT_0_INTR
#endif /* MICROBLAZE */
#elif	defined	(PLATFORM_ZYNQMP) || defined (versal)
#define	INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define	USB_INT_ID		XPAR_XUSBPS_0_INTR
#define	USB_WAKEUP_INTR_ID	XPAR_XUSBPS_0_WAKE_INTR
#else	/* OTHERS */
#define	INTC_DEVICE_ID		0
#define	USB_INT_ID		0
#endif
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
		.Usb_GetDescReply = 0,
	},
	.data_ptr = (void *) &DFU,
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

#ifdef SDT
	struct XUsbPsu *InstancePtr = (struct XUsbPsu *)UsbInstance.PrivateData;
#endif
	/* Initialize the USB driver so that it's ready to use,
	 * specify the controller ID that is generated in xparameters.h
	 */
#ifndef SDT
	UsbConfigPtr = LookupConfig(USB_DEVICE_ID);
#else
	UsbConfigPtr = LookupConfig(XUSBPSU_BASEADDRESS);
#endif
	if (NULL == UsbConfigPtr) {
		return XST_FAILURE;
	}

	/* We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example.  For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = CfgInitialize(&UsbInstance, UsbConfigPtr,
			       UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

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
#ifndef SDT
	Status = SetupInterruptSystem((struct XUsbPsu *)UsbInstance.PrivateData,
				      INTC_DEVICE_ID,
				      USB_INT_ID,
				      (void *)&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstance.PrivateData);
#else
	Status = XSetupInterruptSystem(UsbInstance.PrivateData,
				      (void *) &XUsbPsu_IntrHandler,
				       UsbConfigPtr->IntrId[INTRNAME_DWC3USB3],
				       UsbConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifdef XUSBPSU_HIBERNATION_ENABLE
	Status = XSetupInterruptSystem(UsbInstance.PrivateData,
				       &XUsbPsu_WakeUpIntrHandler,
				       UsbConfigPtr->IntrId[INTRNAME_HIBER],
				       UsbConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif
	/*
	 * Enable interrupts for Reset, Disconnect, ConnectionDone, Link State
	 * Wakeup and Overflow events.
	 */
	XUsbPsu_EnableIntr((struct  XUsbPsu*)UsbInstance.PrivateData,
			   XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
			   XUSBPSU_DEVTEN_WKUPEVTEN |
			   XUSBPSU_DEVTEN_ULSTCNGEN |
			   XUSBPSU_DEVTEN_CONNECTDONEEN |
			   XUSBPSU_DEVTEN_USBRSTEN |
			   XUSBPSU_DEVTEN_DISCONNEVTEN);

#ifdef XUSBPSU_HIBERNATION_ENABLE
	if (InstancePtr->HasHibernation)
		XUsbPsu_EnableIntr(UsbInstance.PrivateData,
				   XUSBPSU_DEVTEN_HIBERNATIONREQEVTEN);
#else
	(void)InstancePtr;
#endif
	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstance.PrivateData);

#endif
	while (1) {
		/* Rest is taken care by interrupts */
	}
}

#ifndef SDT
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
#elif defined (PLATFORM_ZYNQMP) || defined (versal)
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
#endif /* PLATFORM_ZYNQMP or versal */

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif
