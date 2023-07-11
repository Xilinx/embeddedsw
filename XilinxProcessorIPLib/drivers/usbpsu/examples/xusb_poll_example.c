/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/****************************************************************************/
/**
 *
 * @file xusb_poll_example.c
 *
 * This file implements the mass storage class poll example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.5   vak  06/02/19 First release
 * 1.5   vak  03/25/19 Fixed incorrect data_alignment pragma directive for IAR
 * 1.8   pm  15/09/20 Fixed C++ Compilation error.
 * 1.14  pm   21/06/23 Added support for system device-tree flow.
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/
#include "xil_printf.h"
#include "sleep.h"
#include <stdio.h>
#include "xusb_ch9_storage.h"
#include "xusb_class_storage.h"
#include "xusb_wrapper.h"

#include "xparameters.h"

#ifndef SDT
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions ****************************/
#define MEMORY_SIZE (64U * 1024U)
#ifdef __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64U
#else
#pragma data_alignment = 32U
#endif
u8 Buffer[MEMORY_SIZE];
#else
u8 Buffer[MEMORY_SIZE] ALIGNMENT_CACHELINE;
#endif

#ifdef SDT
#define XUSBPSU_BASEADDRESS	XPAR_XUSBPSU_0_BASEADDR /* USB base address */
#endif
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void BulkOutHandler(void *CallBackRef, u32 RequestedBytes,
		    u32 BytesTxed);
void BulkInHandler(void *CallBackRef, u32 RequestedBytes,
		   u32 BytesTxed);

/************************** Variable Definitions *****************************/
struct Usb_DevData UsbInstance;

Usb_Config *UsbConfigPtr;

/* Buffer for virtual flash disk space. */
#ifdef __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64
u8 VirtFlash[VFLASH_SIZE];
#pragma data_alignment = 64
USB_CBW CBW;
#pragma data_alignment = 64
USB_CSW CSW;
#else
#pragma data_alignment = 32
u8 VirtFlash[VFLASH_SIZE];
#pragma data_alignment = 32
USB_CBW CBW;
#pragma data_alignment = 32
USB_CSW CSW;
#endif
#else
u8 VirtFlash[VFLASH_SIZE] ALIGNMENT_CACHELINE;
USB_CBW CBW ALIGNMENT_CACHELINE;
USB_CSW CSW ALIGNMENT_CACHELINE;
#endif

u8	Phase;
u32	rxBytesLeft;
u8	*VirtFlashWritePointer = VirtFlash;

/* Initialize a DFU data structure */
static USBCH9_DATA storage_data = {
	.ch9_func = {
		/* Set the chapter9 hooks */
		.Usb_Ch9SetupDevDescReply =
		Usb_Ch9SetupDevDescReply,
		.Usb_Ch9SetupCfgDescReply =
		Usb_Ch9SetupCfgDescReply,
		.Usb_Ch9SetupBosDescReply =
		Usb_Ch9SetupBosDescReply,
		.Usb_Ch9SetupStrDescReply =
		Usb_Ch9SetupStrDescReply,
		.Usb_SetConfiguration =
		Usb_SetConfiguration,
		.Usb_SetConfigurationApp =
		Usb_SetConfigurationApp,
		/* hook the set interface handler */
		.Usb_SetInterfaceHandler = NULL,
		/* hook up storage class handler */
		.Usb_ClassReq = ClassReq,
		.Usb_GetDescReply = NULL,
	},
	.data_ptr = (void *)NULL,
};

/****************************************************************************/
/**
* This function is the main function of the USB mass storage example.
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

	xil_printf("Mass Storage Gadget Poll Example Start...\r\n");

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

	CacheInit();

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

	/* Assign the data to usb driver */
	Set_DrvData(UsbInstance.PrivateData, &storage_data);

	EpConfigure(UsbInstance.PrivateData, 1U, USB_EP_DIR_OUT,
		    USB_EP_TYPE_BULK);
	EpConfigure(UsbInstance.PrivateData, 1U, USB_EP_DIR_IN,
		    USB_EP_TYPE_BULK);

	Status = ConfigureDevice(UsbInstance.PrivateData,
				 &Buffer[0U], MEMORY_SIZE);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
	 * set endpoint handlers
	 * BulkOutHandler - to be called when data is received
	 * BulkInHandler -  to be called when data is sent
	 */
	SetEpHandler(UsbInstance.PrivateData, 1U, USB_EP_DIR_OUT,
		     BulkOutHandler);
	SetEpHandler(UsbInstance.PrivateData, 1U, USB_EP_DIR_IN,
		     BulkInHandler);

	/*
	 * Enable events for Reset, Disconnect, ConnectionDone, Link State
	 * Wakeup and Overflow events.
	 */
	UsbEnableEvent((struct XUsbPsu *)UsbInstance.PrivateData,
		       XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
		       XUSBPSU_DEVTEN_WKUPEVTEN |
		       XUSBPSU_DEVTEN_ULSTCNGEN |
		       XUSBPSU_DEVTEN_CONNECTDONEEN |
		       XUSBPSU_DEVTEN_USBRSTEN |
		       XUSBPSU_DEVTEN_DISCONNEVTEN);

	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstance.PrivateData);

	while (1U) {
		/* Call Poll Handler for any valid events */
		UsbPollHandler((struct XUsbPsu *)UsbInstance.PrivateData);
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This function is Bulk Out Endpoint handler/Callback called by driver when
* data is received.
*
* @param	CallBackRef is pointer to Usb_DevData instance.
* @param	RequestedBytes is number of bytes requested for reception.
* @param	BytesTxed is actual number of bytes received from Host.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void BulkOutHandler(void *CallBackRef, u32 RequestedBytes,
		    u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = (struct Usb_DevData *)CallBackRef;

	if (Phase == USB_EP_STATE_COMMAND) {
		ParseCBW(InstancePtr);
	} else if (Phase == USB_EP_STATE_DATA_OUT) {
		/* WRITE command */
		switch (CBW.CBWCB[0U]) {
			case USB_RBC_WRITE:
				VirtFlashWritePointer += BytesTxed;
				rxBytesLeft -= BytesTxed;
				break;
			default:
				break;
		}
		SendCSW(InstancePtr, 0U);
	}
}

/****************************************************************************/
/**
* This function is Bulk In Endpoint handler/Callback called by driver when
* data is sent.
*
* @param	CallBackRef is pointer to Usb_DevData instance.
* @param	RequestedBytes is number of bytes requested to send.
* @param	BytesTxed is actual number of bytes sent to Host.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void BulkInHandler(void *CallBackRef, u32 RequestedBytes,
		   u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = (struct Usb_DevData *)CallBackRef;

	if (Phase == USB_EP_STATE_DATA_IN) {
		/* Send the status */
		SendCSW(InstancePtr, 0U);
	} else if (Phase == USB_EP_STATE_STATUS) {
		Phase = USB_EP_STATE_COMMAND;
		/* Receive next CBW */
		EpBufferRecv(InstancePtr->PrivateData, 1U,
			     (u8 *)&CBW, sizeof(CBW));
	}
}
