/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_freertos_ch9_storage.c
 *
 * This file contains the implementation of the Mass Storage specific chapter 9
 * code for the example.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   22/03/18 First release
 * 1.5   vak  03/25/19 Fixed incorrect data_alignment pragma directive for IAR
 *
 *</pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "xparameters.h"		/* XPAR parameters */
#include "xusb_freertos_ch9_storage.h"
#include "xusb_freertos_class_storage.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/* Device Descriptors */
#ifdef __ICCARM__
#pragma data_alignment = 16
#endif

#ifdef __ICCARM__
USB_STD_DEV_DESC deviceDesc[] = {
#else
USB_STD_DEV_DESC __attribute__ ((aligned(16))) deviceDesc[] = {
#endif
	{
		/* USB 2.0 */
		sizeof(USB_STD_DEV_DESC),	/* bLength */
		USB_TYPE_DEVICE_DESC,		/* bDescriptorType */
		0x0200,				/* bcdUSB 2.0 */
		0x00,				/* bDeviceClass */
		0x00,				/* bDeviceSubClass */
		0x00,				/* bDeviceProtocol */
		0x40,				/* bMaxPackedSize0 */
		0x03Fd,				/* idVendor */
		0x0500,				/* idProduct */
		0x0100,				/* bcdDevice */
		0x01,				/* iManufacturer */
		0x02,				/* iProduct */
		0x03,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	},
	{
		/* USB 3.0 */
		sizeof(USB_STD_DEV_DESC),	/* bLength */
		USB_TYPE_DEVICE_DESC,		/* bDescriptorType */
		0x0300,				/* bcdUSB 3.0 */
		0x00,				/* bDeviceClass */
		0x00,				/* bDeviceSubClass */
		0x00,				/* bDeviceProtocol */
		0x09,				/* bMaxPackedSize0 */
		0x0525,				/* idVendor */
		0xA4A5,				/* idProduct */
		0x0404,				/* bcdDevice */
		0x03,				/* iManufacturer */
		0x04,				/* iProduct */
		0x05,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	}
};


/* Configuration Descriptors */
#ifdef __ICCARM__
USB30_CONFIG config3 = {
#else
USB30_CONFIG __attribute__ ((aligned(16))) config3 = {
#endif
	{
		/* Std Config */
		sizeof(USB_STD_CFG_DESC),	/* bLength */
		USB_TYPE_CONFIG_DESC,		/* bDescriptorType */
		sizeof(USB30_CONFIG),		/* wTotalLength */
		0x01,				/* bNumInterfaces */
		0x01,				/* bConfigurationValue */
		0x00,				/* iConfiguration */
		0xc0,				/* bmAttribute */
		0x00				/* bMaxPower  */
	},
	{
		/* Mass Storage Standard Interface Descriptor */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x00,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x02,				/* bNumEndPoints */
		USB_CLASS_STORAGE,		/* bInterfaceClass */
		0x06,				/* bInterfaceSubClass */
		0x50,				/* bInterfaceProtocol */
		0x01				/* iInterface */
	},
	{
		/* Bulk In Endpoint Config */
		sizeof(USB_STD_EP_DESC),	/* bLength */
		USB_TYPE_ENDPOINT_CFG_DESC,	/* bDescriptorType */
		USB_EP1_IN,			/* bEndpointAddress */
		0x02,				/* bmAttribute  */
		0x00,				/* wMaxPacketSize - LSB */
		0x04,				/* wMaxPacketSize - MSB */
		0x00				/* bInterval */
	},
	{
		/* SS Endpoint companion */
		sizeof(USB_STD_EP_SS_COMP_DESC),/* bLength */
		0x30,				/* bDescriptorType */
		0x0F,				/* bMaxBurst */
		0x00,				/* bmAttributes */
		0x00				/* wBytesPerInterval */
	},
	{
		/* Bulk Out Endpoint Config */
		sizeof(USB_STD_EP_DESC),	/* bLength */
		USB_TYPE_ENDPOINT_CFG_DESC,	/* bDescriptorType */
		USB_EP1_OUT,			/* bEndpointAddress */
		0x02,				/* bmAttribute */
		0x00,				/* wMaxPacketSize - LSB */
		0x04,				/* wMaxPacketSize - MSB */
		0x00				/* bInterval */
	},
	{
		/* SS Endpoint companion */
		sizeof(USB_STD_EP_SS_COMP_DESC),/* bLength */
		0x30,				/* bDescriptorType */
		0x0F,				/* bMaxBurst */
		0x00,				/* bmAttributes */
		0x00				/* wBytesPerInterval */
	}
};

#ifdef __ICCARM__
USB_CONFIG config2 = {
#else
USB_CONFIG __attribute__ ((aligned(16))) config2 = {
#endif
	{
		/* Std Config */
		sizeof(USB_STD_CFG_DESC),	/* bLength */
		USB_TYPE_CONFIG_DESC,		/* bDescriptorType */
		sizeof(USB_CONFIG),		/* wTotalLength */
		0x01,				/* bNumInterfaces */
		0x01,				/* bConfigurationValue */
		0x00,				/* iConfiguration */
		0xc0,				/* bmAttribute */
		0x00				/* bMaxPower  */
	},
	{
		/* Mass Storage Standard Interface Descriptor */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x00,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x02,				/* bNumEndPoints */
		USB_CLASS_STORAGE,		/* bInterfaceClass */
		0x06,				/* bInterfaceSubClass */
		0x50,				/* bInterfaceProtocol */
		0x05				/* iInterface */
	},
	{
		/* Bulk In Endpoint Config */
		sizeof(USB_STD_EP_DESC),	/* bLength */
		USB_TYPE_ENDPOINT_CFG_DESC,	/* bDescriptorType */
		USB_EP1_IN,			/* bEndpointAddress */
		0x02,				/* bmAttribute  */
		0x00,				/* wMaxPacketSize - LSB */
		0x02,				/* wMaxPacketSize - MSB */
		0x00				/* bInterval */
	},
	{
		/* Bulk Out Endpoint Config */
		sizeof(USB_STD_EP_DESC),	/* bLength */
		USB_TYPE_ENDPOINT_CFG_DESC,	/* bDescriptorType */
		USB_EP1_OUT,			/* bEndpointAddress */
		0x02,				/* bmAttribute  */
		0x00,				/* wMaxPacketSize - LSB */
		0x02,				/* wMaxPacketSize - MSB */
		0x00				/* bInterval */
	}
};

/* String Descriptors */
static u8 StringList[2][6][128] = {
	{
		"Xilinx standalone",
		"Mass Storage",
		"USB 2.0 Flash Drive Disk Emulation",
		"2A49876D9CC1AA4",
		"Mass Storage Gadget",
		"Default Interface"
	},
	{
		"Xilinx standalone",
		"Mass Storage",
		"2A49876D9CC1AA4",
		"USB 3.0 Flash Drive Disk Emulation",
		"Mass Storage Gadget",
		"7ABC7ABC7ABC7ABC7ABC7ABC"
	}
};

/*****************************************************************************/
/**
*
* This function returns the device descriptor for the device.
*
* @param	InstancePtr is a pointer to the Usb_DevData instance.
* @param	BufPtr is pointer to the buffer that is to be filled
*		with the descriptor.
* @param	BufLen is the size of the provided buffer.
*
* @return	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 Usb_Ch9SetupDevDescReply(struct Usb_DevData *InstancePtr,
		u8 *BufPtr, u32 BufLen)
{
	u8 Index;
	s32 Status;

	Status = IsSuperSpeed(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* USB 2.0 */
		Index = 0;
	} else {
		/* USB 3.0 */
		Index = 1;
	}

	if (!BufPtr || BufLen < sizeof(USB_STD_DEV_DESC))
		return 0;

	memcpy(BufPtr, &deviceDesc[Index], sizeof(USB_STD_DEV_DESC));

	return sizeof(USB_STD_DEV_DESC);
}


/*****************************************************************************/
/**
*
* This function returns the configuration descriptor for the device.
*
* @param	InstancePtr is a pointer to the Usb_DevData instance.
* @param	BufPtr is the pointer to the buffer that is to be filled with
*		the descriptor.
* @param	BufLen is the size of the provided buffer.
*
* @return	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 Usb_Ch9SetupCfgDescReply(struct Usb_DevData *InstancePtr,
		u8 *BufPtr, u32 BufLen)
{
	s32 Status;
	u8 *config;
	u32 CfgDescLen;

	Status = IsSuperSpeed(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* USB 2.0 */
		config = (u8 *)&config2;
		CfgDescLen  = sizeof(USB_CONFIG);
	} else {
		/* USB 3.0 */
		config = (u8 *)&config3;
		CfgDescLen  = sizeof(USB30_CONFIG);
	}

	if (!BufPtr || BufLen < sizeof(USB_STD_CFG_DESC))
		return 0;

	memcpy(BufPtr, config, CfgDescLen);

	return CfgDescLen;
}


/*****************************************************************************/
/**
*
* This function returns a string descriptor for the given index.
*
* @param	InstancePtr is a pointer to the Usb_DevData instance.
* @param	BufPtr is a  pointer to the buffer that is to be filled with
*		the descriptor.
* @param	BufLen is the size of the provided buffer.
* @param	Index is the index of the string for which the descriptor
*		is requested.
*
* @return	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 Usb_Ch9SetupStrDescReply(struct Usb_DevData *InstancePtr,
		u8 *BufPtr, u32 BufLen, u8 Index)
{
	u32 i;
	char *String;
	u32 StringLen;
	u32 DescLen;
	u8 TmpBuf[128];
	s32 Status;
	u8 StrArray;

	USB_STD_STRING_DESC *StringDesc;

	Status = IsSuperSpeed(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* USB 2.0 */
		StrArray = 0;
	} else {
		/* USB 3.0 */
		StrArray = 1;
	}

	if (!BufPtr)
		return 0;

	String = (char *)&StringList[StrArray][Index];

	if (Index >= sizeof(StringList) / sizeof(u8 *))
		return 0;

	StringLen = strlen(String);

	StringDesc = (USB_STD_STRING_DESC *) TmpBuf;

	/* Index 0 is special as we can not represent the string required in
	 * the table above. Therefore we handle index 0 as a special case.
	 */
	if (0 == Index) {
		StringDesc->bLength = 4;
		StringDesc->bDescriptorType = 0x03;
		StringDesc->wLANGID[0] = 0x0409;
	}
	/* All other strings can be pulled from the table above. */
	else {
		StringDesc->bLength = StringLen * 2 + 2;
		StringDesc->bDescriptorType = 0x03;

		for (i = 0; i < StringLen; i++)
			StringDesc->wLANGID[i] = (u16) String[i];
	}
	DescLen = StringDesc->bLength;

	/* Check if the provided buffer is big enough to hold the descriptor. */
	if (DescLen > BufLen)
		return 0;

	memcpy(BufPtr, StringDesc, DescLen);

	return DescLen;
}

/*****************************************************************************/
/**
*
* This function returns the BOS descriptor for the device.
*
* @param	BufPtr is the pointer to the buffer that is to be filled with
*		the descriptor.
* @param	BufLen is the size of the provided buffer.
*
* @return	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 Usb_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufLen)
{
#ifdef __ICCARM__
#pragma data_alignment = 16
	static USB_BOS_DESC bosDesc = {
#else
	static USB_BOS_DESC __attribute__ ((aligned(16))) bosDesc = {
#endif
		/* BOS descriptor */
		{sizeof(USB_STD_BOS_DESC),	/* bLength */
			USB_TYPE_BOS_DESC,	/* DescriptorType */
			sizeof(USB_BOS_DESC),	/* wTotalLength */
			0x02},			/* bNumDeviceCaps */

		{sizeof(USB_STD_DEVICE_CAP_7BYTE), /* bLength */
			0x10,			/* bDescriptorType */
			0x02,			/* bDevCapabiltyType */
			0x06},			/* bmAttributes */

		{sizeof(USB_STD_DEVICE_CAP_10BYTE), /* bLength */
			0x10,			/* bDescriptorType */
			0x03,			/* bDevCapabiltyType */
			0x00,			/* bmAttributes */
			(0x000F),		/* wSpeedsSupported */
			0x01,			/* bFunctionalitySupport */
			0x01,			/* bU1DevExitLat */
			(0x01F4)}		/* wU2DevExitLat */
	};

	/* Check buffer pointer is OK and buffer is big enough. */
	if (!BufPtr || BufLen < sizeof(USB_STD_BOS_DESC))
		return 0;

	memcpy(BufPtr, &bosDesc, sizeof(USB_BOS_DESC));

	return sizeof(USB_BOS_DESC);
}

/****************************************************************************/
/**
* Changes State of Core to USB configured State.
*
* @param	InstancePtr is a pointer to the Usb_DevData instance.
* @param	Ctrl is a pointer to the Setup packet data.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
s32 Usb_SetConfiguration(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl)
{
	u8 State;
	s32 Ret = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Ctrl != NULL);

	State = InstancePtr->State;
	SetConfigDone(InstancePtr->PrivateData, 0U);

	switch (State) {
		case USB_STATE_DEFAULT:
			Ret = XST_FAILURE;
			break;

		case USB_STATE_ADDRESS:
			InstancePtr->State = USB_STATE_CONFIGURED;
			break;

		case USB_STATE_CONFIGURED:
			break;

		default:
			Ret = XST_FAILURE;
			break;
	}

	return Ret;
}

/****************************************************************************/
/**
* This function is called by Chapter9 handler when SET_CONFIGURATION command
* is received from Host.
*
* @param	InstancePtr is pointer to Usb_DevData instance.
* @param	SetupData is the setup packet received from Host.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE if unsuccessful.
*
* @note
*		Non control endpoints must be enabled after SET_CONFIGURATION
*		command since hardware clears all previously enabled endpoints
*		except control endpoints when this command is received.
*
*****************************************************************************/
s32 Usb_SetConfigurationApp(struct Usb_DevData *InstancePtr,
		SetupPacket *SetupData)
{
	BaseType_t xHigherPriorityTaskWoken;

	/* When we run CV test suite application in Windows, need to
	 * add SET_CONFIGURATION command with value 0/1 to pass test suite
	 */
	if ((SetupData->wValue && 0xff) ==  1) {

		SetConfigDone(InstancePtr->PrivateData, 1U);

		xTaskNotifyFromISR(xMainTask, MSG_CONFIG,
				eSetValueWithOverwrite,
				&xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	} else {

		SetConfigDone(InstancePtr->PrivateData, 0U);
		xTaskNotifyFromISR(xMainTask, MSG_UNCONFIG,
				eSetValueWithOverwrite,
				&xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

	return XST_SUCCESS;
}
