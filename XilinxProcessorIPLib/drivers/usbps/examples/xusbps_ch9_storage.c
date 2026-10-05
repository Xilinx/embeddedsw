/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xusbps_ch9_storage.c
 *
 * This file contains the implementation of the storage specific chapter 9 code
 * for the example.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.00a wgr  10/10/10 First release
 * 2.5	 pm   02/20/20 Added SetConfigurationApp and SetInterfaceHandler API to
 *			make ch9 common framework to all example.
 * 2.10  ka   08/21/25 Fixed GCC warnings
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include <string.h>

#include "xparameters.h"	/* XPAR parameters */
#include "xusbps.h"		/* USB controller driver */

#include "xusbps_ch9.h"
#include "xusbps_ch9_storage.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8  bLength; /**< Descriptor length in bytes. */
	u8  bDescriptorType; /**< Descriptor type identifier. */
	u16 bcdUSB; /**< USB specification version (BCD). */
	u8  bDeviceClass; /**< Device class code. */
	u8  bDeviceSubClass; /**< Device subclass code. */
	u8  bDeviceProtocol; /**< Device protocol code. */
	u8  bMaxPacketSize0; /**< Maximum packet size for endpoint 0. */
	u16 idVendor; /**< Vendor ID. */
	u16 idProduct; /**< Product ID. */
	u16 bcdDevice; /**< Device release number (BCD). */
	u8  iManufacturer; /**< Index of manufacturer string descriptor. */
	u8  iProduct; /**< Index of product string descriptor. */
	u8  iSerialNumber; /**< Index of serial number string descriptor. */
	u8  bNumConfigurations; /**< Number of possible configurations. */
#ifdef __ICCARM__
} USB_STD_DEV_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB_STD_DEV_DESC;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8  bLength; /**< Descriptor length in bytes. */
	u8  bDescriptorType; /**< Descriptor type identifier. */
	u16 wTotalLength; /**< Total length of configuration descriptor. */
	u8  bNumInterfaces; /**< Number of interfaces in this configuration. */
	u8  bConfigurationValue; /**< Value for SET_CONFIGURATION request. */
	u8  iConfiguration; /**< Index of string descriptor for this configuration. */
	u8  bmAttributes; /**< Configuration attributes (power and remote-wakeup flags). */
	u8  bMaxPower; /**< Maximum power consumption in 2mA units. */
#ifdef __ICCARM__
} USB_STD_CFG_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB_STD_CFG_DESC;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8  bLength; /**< Descriptor length in bytes. */
	u8  bDescriptorType; /**< Descriptor type identifier. */
	u8  bInterfaceNumber; /**< Interface number. */
	u8  bAlternateSetting; /**< Alternate setting number. */
	u8  bNumEndPoints; /**< Number of endpoints. */
	u8  bInterfaceClass; /**< Interface class code. */
	u8  bInterfaceSubClass; /**< Interface subclass code. */
	u8  bInterfaceProtocol; /**< Interface protocol code. */
	u8  iInterface; /**< Index of string descriptor for this interface. */
#ifdef __ICCARM__
} USB_STD_IF_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB_STD_IF_DESC;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8  bLength; /**< Descriptor length in bytes. */
	u8  bDescriptorType; /**< Descriptor type identifier. */
	u8  bEndpointAddress; /**< Endpoint address (direction + number). */
	u8  bmAttributes; /**< Endpoint/capability attributes. */
	u16 wMaxPacketSize; /**< Maximum packet size for this endpoint. */
	u8  bInterval; /**< Polling interval in frames. */
#ifdef __ICCARM__
} USB_STD_EP_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB_STD_EP_DESC;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8  bLength; /**< Descriptor length in bytes. */
	u8  bDescriptorType; /**< Descriptor type identifier. */
	u16 wLANGID[1]; /**< Supported language IDs. */
#ifdef __ICCARM__
} USB_STD_STRING_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB_STD_STRING_DESC;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	USB_STD_CFG_DESC stdCfg; /**< Standard configuration descriptor. */
	USB_STD_IF_DESC ifCfg; /**< Standard mass-storage interface descriptor. */
	USB_STD_EP_DESC epCfg1; /**< First bulk endpoint configuration. */
	USB_STD_EP_DESC epCfg2; /**< Second bulk endpoint configuration. */
#ifdef __ICCARM__
} USB_CONFIG;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB_CONFIG;
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/** @name USB descriptor type and endpoint constants.
 * @{
 */
#define USB_ENDPOINT0_MAXP		0x40 /**< Maximum packet size for endpoint 0. */

#define USB_BULKIN_EP			1 /**< Bulk IN endpoint number. */
#define USB_BULKOUT_EP			1 /**< Bulk OUT endpoint number. */

#define USB_DEVICE_DESC			0x01 /**< Device descriptor type. */
#define USB_CONFIG_DESC			0x02 /**< Configuration descriptor type. */
#define USB_STRING_DESC			0x03 /**< String descriptor type. */
#define USB_INTERFACE_CFG_DESC		0x04 /**< Interface descriptor type. */
#define USB_ENDPOINT_CFG_DESC		0x05 /**< Endpoint descriptor type. */
/** @} */


/*****************************************************************************/
/**
*
* This function returns the device descriptor for the device.
*
* @param[out] BufPtr is pointer to the buffer that is to be filled
*		with the descriptor.
* @param[in] BufLen is the size of the provided buffer.
*
* @return 	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 XUsbPs_Ch9SetupDevDescReply(u8 *BufPtr, u32 BufLen)
{
	USB_STD_DEV_DESC deviceDesc = {
		sizeof(USB_STD_DEV_DESC),	/* bLength */
		USB_DEVICE_DESC,		/* bDescriptorType */
		be2les(0x0200),			/* bcdUSB 2.0 */
		0x00,				/* bDeviceClass */
		0x00,				/* bDeviceSubClass */
		0x00,				/* bDeviceProtocol */
		USB_ENDPOINT0_MAXP,		/* bMaxPackedSize0 */
		be2les(0x0d7d),			/* idVendor */
		be2les(0x0100),			/* idProduct */
		be2les(0x0100),			/* bcdDevice */
		0x01,				/* iManufacturer */
		0x02,				/* iProduct */
		0x03,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	};

	/* Check buffer pointer is there and buffer is big enough. */
	if (!BufPtr) {
		return 0;
	}

	if (BufLen < sizeof(USB_STD_DEV_DESC)) {
		return 0;
	}

	memcpy(BufPtr, &deviceDesc, sizeof(USB_STD_DEV_DESC));

	return sizeof(USB_STD_DEV_DESC);
}


/*****************************************************************************/
/**
*
* This function returns the configuration descriptor for the device.
*
* @param[out] BufPtr is the pointer to the buffer that is to be filled with
*		the descriptor.
* @param[in] BufLen is the size of the provided buffer.
*
* @return 	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 XUsbPs_Ch9SetupCfgDescReply(u8 *BufPtr, u32 BufLen)
{
	USB_CONFIG config = {
		/* Std Config */
		{
			sizeof(USB_STD_CFG_DESC),	/* bLength */
			USB_CONFIG_DESC,		/* bDescriptorType */
			be2les(sizeof(USB_CONFIG)),	/* wTotalLength */
			0x01,				/* bNumInterfaces */
			0x01,				/* bConfigurationValue */
			0x04,				/* iConfiguration */
			0xc0,				/* bmAttribute */
			0x00
		},				/* bMaxPower  */

		/* Interface Config */
		{
			sizeof(USB_STD_IF_DESC),	/* bLength */
			USB_INTERFACE_CFG_DESC,	/* bDescriptorType */
			0x00,				/* bInterfaceNumber */
			0x00,				/* bAlternateSetting */
			0x02,				/* bNumEndPoints */
			0x08,				/* bInterfaceClass */
			0x06,				/* bInterfaceSubClass */
			0x50,				/* bInterfaceProtocol */
			0x05
		},				/* iInterface */

		/* Bulk Out Endpoint Config */
		{
			sizeof(USB_STD_EP_DESC),	/* bLength */
			USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
			0x00 | USB_BULKOUT_EP,		/* bEndpointAddress */
			0x02,				/* bmAttribute  */
			be2les(0x200),			/* wMaxPacketSize */
			0x00
		},				/* bInterval */

		/* Bulk In Endpoint Config */
		{
			sizeof(USB_STD_EP_DESC),	/* bLength */
			USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
			0x80 | USB_BULKIN_EP,		/* bEndpointAddress */
			0x02,				/* bmAttribute  */
			be2les(0x200),			/* wMaxPacketSize */
			0x00
		}				/* bInterval */
	};

	/* Check buffer pointer is OK and buffer is big enough. */
	if (!BufPtr) {
		return 0;
	}

	if (BufLen < sizeof(USB_STD_DEV_DESC)) {
		return 0;
	}

	memcpy(BufPtr, &config, sizeof(USB_CONFIG));

	return sizeof(USB_CONFIG);
}


/*****************************************************************************/
/**
*
* This function returns a string descriptor for the given index.
*
* @param[out] BufPtr is a  pointer to the buffer that is to be filled with
*		the descriptor.
* @param[in] BufLen is the size of the provided buffer.
* @param[in] Index is the index of the string for which the descriptor
*		is requested.
*
* @return 	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 XUsbPs_Ch9SetupStrDescReply(u8 *BufPtr, u32 BufLen, u8 Index)
{
	u32 i;

	static char *StringList[] = {
		"UNUSED",
		"Xilinx",
		"EPB USB Flash Drive Disk Emulation",
		"2A49876D9CC1AA4",
		"Default Configuration",
		"Default Interface",
	};
	char *String;
	u32 StringLen;
	u32 DescLen;
	u8 TmpBuf[128];

	USB_STD_STRING_DESC *StringDesc;

	if (!BufPtr) {
		return 0;
	}

	if (Index >= sizeof(StringList) / sizeof(char *)) {
		return 0;
	}

	String = StringList[Index];
	StringLen = strlen(String);

	StringDesc = (USB_STD_STRING_DESC *) TmpBuf;

	/* Index 0 is special as we can not represent the string required in
	 * the table above. Therefore we handle index 0 as a special case.
	 */
	if (0 == Index) {
		StringDesc->bLength = 4;
		StringDesc->bDescriptorType = USB_STRING_DESC;
		StringDesc->wLANGID[0] = be2les(0x0409);
	}
	/* All other strings can be pulled from the table above. */
	else {
		StringDesc->bLength = StringLen * 2 + 2;
		StringDesc->bDescriptorType = USB_STRING_DESC;

		for (i = 0; i < StringLen; i++) {
			StringDesc->wLANGID[i] = be2les((u16) String[i]);
		}
	}
	DescLen = StringDesc->bLength;

	/* Check if the provided buffer is big enough to hold the descriptor. */
	if (DescLen > BufLen) {
		return 0;
	}

	memcpy(BufPtr, StringDesc, DescLen);

	return DescLen;
}


/****************************************************************************/
/*
* This function handles a "set configuration" request.
*
* @param[in] InstancePtr is a pointer to XUsbPs instance of the controller.
* @param[in] ConfigIdx is the Index of the desired configuration.
*
******************************************************************************/
void XUsbPs_SetConfiguration(XUsbPs *InstancePtr, int ConfigIdx)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/* We only have one configuration. Its index is 1. Ignore anything
	 * else.
	 */
	if (1 != ConfigIdx) {
		return;
	}

	XUsbPs_EpEnable(InstancePtr, 1, XUSBPS_EP_DIRECTION_OUT);
	XUsbPs_EpEnable(InstancePtr, 1, XUSBPS_EP_DIRECTION_IN);

	/* Set BULK mode for both directions.  */
	XUsbPs_SetBits(InstancePtr, XUSBPS_EPCR1_OFFSET,
		       XUSBPS_EPCR_TXT_BULK_MASK |
		       XUSBPS_EPCR_RXT_BULK_MASK |
		       XUSBPS_EPCR_TXR_MASK |
		       XUSBPS_EPCR_RXR_MASK);

	/* Prime the OUT endpoint. */
	XUsbPs_EpPrime(InstancePtr, 1, XUSBPS_EP_DIRECTION_OUT);
}

/***************************************************************************/
/*
 * This function is called by Chapter9 handler when SET_CONFIGURATION command
 * is received from Host.
 *
 * @param[in] InstancePtr is pointer to XUsbPs instance of the controller.
 * @param[in] SetupData is the setup packet received from Host.
 *
 * @note
 *		Non control endpoints must be enabled after SET_CONFIGURATION
 *		command since hardware clears all previously enabled endpoints
 *		except control endpoints when this command is received.
 *
 *****************************************************************************/
void XUsbPs_SetConfigurationApp(XUsbPs *InstancePtr,
				XUsbPs_SetupData *SetupData)
{
	(void)InstancePtr;
	(void)SetupData;
}

/***************************************************************************/
/*
 * This function is called by Chapter9 handler when SET_CONFIGURATION command
 * or SET_INTERFACE command is received from Host.
 *
 * @param[in] InstancePtr is pointer to XUsbPs instance of the controller.
 * @param[in] SetupData is the setup packet received from Host.
 *
 * @note
 *
 *****************************************************************************/
void XUsbPs_SetInterfaceHandler(XUsbPs *InstancePtr,
				XUsbPs_SetupData *SetupData)
{
	(void)InstancePtr;
	(void)SetupData;
}
