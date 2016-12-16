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
/*****************************************************************************/
/**
 * @file xusbpsu_ch9_dfu.c
 *
 * This file contains the implementation of the DFU specific chapter 9 code
 * for the example.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.0	 vak  30/11/16 Addded DFU support
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xusbpsu_ch9_dfu.h"

#include "xparameters.h"	/* XPAR parameters */
#include "xusbpsu.h"		/* USB controller driver */
#include "xusbpsu_ch9.h"
#include "xusbpsu_dfu.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define MAX_STRLEN				128
/**************************** Type Definitions *******************************/
/*
 * Descriptor Types
 */
#define DEVICE_DESCR			0x01
#define CONFIG_DESCR			0x02
#define STRING_DESCR			0x03
#define INTERFACE_DESCR			0x04
#define ENDPOINT_DESCR			0x05
#define QUALIFIER_DESCR			0x06
#define OSD_CONFIG_DESCR		0x07
#define HID_DESC			0x21	/* Get descriptor: HID */
#define REPORT_DESC			0x22	/* Get descriptor:Report */
#define DFUFUNC_DESCR			0x21    /* DFU Functional Desc */

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#define USB_DEVICE_DESC			0x01
#define USB_CONFIG_DESC			0x02
#define USB_STRING_DESC			0x03
#define USB_INTERFACE_CFG_DESC		0x04
#define USB_ENDPOINT_CFG_DESC		0x05

/* Device Descriptors */
USB_STD_DEV_DESC __attribute__ ((aligned(16))) deviceDesc[] = {
	{/* USB 2.0 */
		sizeof(USB_STD_DEV_DESC),	/* bLength */
		USB_DEVICE_DESC,		/* bDescriptorType */
		(0x0200),			/* bcdUSB 2.0 */
		0x00,				/* bDeviceClass */
		0x00,				/* bDeviceSubClass */
		0x00,				/* bDeviceProtocol */
		0x40,				/* bMaxPackedSize0 */
		(0x03Fd),			/* idVendor */
		(0x0500),			/* idProduct */
		(0x0100),			/* bcdDevice */
		0x01,				/* iManufacturer */
		0x02,				/* iProduct */
		0x03,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	},
	{/* USB 3.0 */
		sizeof(USB_STD_DEV_DESC),	/* bLength */
		USB_DEVICE_DESC,		/* bDescriptorType */
		(0x0300),			/* bcdUSB 3.0 */
		0x00,				/* bDeviceClass */
		0x00,				/* bDeviceSubClass */
		0x00,				/* bDeviceProtocol */
		0x09,				/* bMaxPackedSize0 */
		(0x0525),			/* idVendor */
		(0xA4A5),			/* idProduct */
		(0x0404),			/* bcdDevice */
		0x01,				/* iManufacturer */
		0x02,				/* iProduct */
		0x03,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	}
};

/* Configuration Descriptors */
USB30_CONFIG __attribute__ ((aligned(16))) config3 = {
		/* Std Config */
		{sizeof(USB_STD_CFG_DESC),	/* bLength */
		 USB_CONFIG_DESC,		/* bDescriptorType */
		 sizeof(USB30_CONFIG),		/* wTotalLength */
		 0x01,				/* bNumInterfaces */
		 0x01,				/* bConfigurationValue */
		 0x00,				/* iConfiguration */
		 0xc0,				/* bmAttribute */
		 0x00},				/* bMaxPower  */

		/* Interface Config */
		{sizeof(USB_STD_IF_DESC),	/* bLength */
		 USB_INTERFACE_CFG_DESC,	/* bDescriptorType */
		 0x00,				/* bInterfaceNumber */
		 0x00,				/* bAlternateSetting */
		 0x02,				/* bNumEndPoints */
		 0xFF,				/* bInterfaceClass */
		 0xFF,				/* bInterfaceSubClass */
		 0xFF,				/* bInterfaceProtocol */
		 0x04},				/* iInterface */

		/* Bulk In Endpoint Config */
		{sizeof(USB_STD_EP_DESC),	/* bLength */
		 USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
		 0x81,				/* bEndpointAddress */
		 0x02,				/* bmAttribute  */
		 0x00,				/* wMaxPacketSize - LSB */
		 0x04,				/* wMaxPacketSize - MSB */
		 0x00},				/* bInterval */

		 /* SS Endpoint companion  */
		 {sizeof(USB_STD_EP_SS_COMP_DESC),	/* bLength */
		 0x30, 				/* bDescriptorType */
		 0x0F,				/* bMaxBurst */
		 0x00,				/* bmAttributes */
		 0x00},				/* wBytesPerInterval */

		/* Bulk Out Endpoint Config */
		{sizeof(USB_STD_EP_DESC),	/* bLength */
		 USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
		 0x01,				/* bEndpointAddress */
		 0x02,				/* bmAttribute  */
		 0x00,				/* wMaxPacketSize - LSB */
		 0x04,				/* wMaxPacketSize - MSB */
		 0x00},				/* bInterval */

		 /* SS Endpoint companion  */
		 {sizeof(USB_STD_EP_SS_COMP_DESC),	/* bLength */
		 0x30,				/* bDescriptorType */
		 0x0F,				/* bMaxBurst */
		 0x00,				/* bmAttributes */
		 0x00},				/* wBytesPerInterval */

		/*** DFU Interface descriptor ***/
		{sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_INTERFACE_CFG_DESC,		/* bDescriptorType */
		0x00,				/* bInterfaceNumber */
		0x01,				/* bAlternateSetting */
		0x00,				/* bNumEndPoints */
		0xFE,				/* bInterfaceClass DFU application specific class code */
		0x01,				/* bInterfaceSubClass DFU device firmware upgrade code*/
		0x02,				/* bInterfaceProtocol DFU mode protocol*/
		0x04},				/* iInterface DFU string descriptor*/

		/**** DFU functional descriptor ****/
		{sizeof(USB_DFU_FUNC_DESC),	/* bLength*/
		DFUFUNC_DESCR,			/* bDescriptorType DFU functional descriptor type */
		0x3,				/* bmAttributes Device is only download/upload capable */
		8192,				/* wDetatchTimeOut 8192 ms */
		DFU_MAX_TRANSFER,		/* wTransferSize DFU block size 1024*/
		0x0110				/* bcdDfuVersion 1.1 */
		}
	};

USB_CONFIG __attribute__ ((aligned(16))) config2 = {
		/* Std Config */
		{sizeof(USB_STD_CFG_DESC),	/* bLength */
		 USB_CONFIG_DESC,		/* bDescriptorType */
		 sizeof(USB_CONFIG),		/* wTotalLength */
		 0x01,				/* bNumInterfaces */
		 0x01,				/* bConfigurationValue */
		 0x00,				/* iConfiguration */
		 0xc0,				/* bmAttribute */
		 0x00},				/* bMaxPower  */

		/* Interface Config */
		{sizeof(USB_STD_IF_DESC),	/* bLength */
		 USB_INTERFACE_CFG_DESC,	/* bDescriptorType */
		 0x00,				/* bInterfaceNumber */
		 0x00,				/* bAlternateSetting */
		 0x02,				/* bNumEndPoints */
		 0xFF,				/* bInterfaceClass */
		 0xFF,				/* bInterfaceSubClass */
		 0xFF,				/* bInterfaceProtocol */
		 0x04},				/* iInterface */

		/* Bulk In Endpoint Config */
		{sizeof(USB_STD_EP_DESC),	/* bLength */
		 USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
		 0x81,				/* bEndpointAddress */
		 0x02,				/* bmAttribute  */
		 0x00,				/* wMaxPacketSize - LSB */
		 0x02,				/* wMaxPacketSize - MSB */
		 0x00},				/* bInterval */

		/* Bulk Out Endpoint Config */
		{sizeof(USB_STD_EP_DESC),	/* bLength */
		 USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
		 0x01,				/* bEndpointAddress */
		 0x02,				/* bmAttribute  */
		 0x00,				/* wMaxPacketSize - LSB */
		 0x02,				/* wMaxPacketSize - MSB */
		 0x00},				/* bInterval */

		/*** DFU Interface Descriptor ***/
		{sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_INTERFACE_CFG_DESC,	/* bDescriptorType */
		0x00,				/* bInterfaceNumber */
		0x01,				/* bAlternateSetting */
		0x00,				/* bNumEndPoints */
		0xFE,				/* bInterfaceClass DFU application specific class code */
		0x01,				/* bInterfaceSubClass DFU device firmware upgrade code*/
		0x02,				/* bInterfaceProtocol DFU mode protocol*/
		0x04},				/* iInterface DFU string descriptor*/

		/*****DFU functional descriptor*****/
		{sizeof(USB_DFU_FUNC_DESC),	/* bLength*/
		DFUFUNC_DESCR,			/* bDescriptorType DFU functional descriptor type */
		0x3,				/* bmAttributes Device is only download capable bitCanDnload */
		8192,				/* wDetatchTimeOut 8192 ms*/
		DFU_MAX_TRANSFER,		/* wTransferSize DFU block size 1024*/
		0x0110				/* bcdDfuVersion 1.1 */
		}
	};

/* String Descriptors */
static char StringList[2][6][MAX_STRLEN] = {
		{
			"UNUSED",
			"XILINX INC",
			"DFU 2.0 emulation v 1.1",
			"2A49876D9CC1AA4",
			"DEFAULT DFU RUNTIME ITERFACE",
			"7ABC7ABC7ABC7ABC7ABC7ABC"
		},
		{
			"UNUSED",
			"XILINX INC",
			"DFU 3.0 emulation v 1.1",
			"2A49876D9CC1AA4",
			"DEFAULT DFU RUNTIME ITERFACE",
			"7ABC7ABC7ABC7ABC7ABC7ABC"
		},
	};

/*****************************************************************************/
/**
*
* This function returns the device descriptor for the device.
*
* @param	BufPtr is pointer to the buffer that is to be filled
*		with the descriptor.
* @param	BufLen is the size of the provided buffer.
*
* @return	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 XUsbPsu_Ch9SetupDevDescReply(struct XUsbPsu *InstancePtr,
		u8 *BufPtr, u32 BufLen)
{
	s32 Status;
	USBCH9_DATA *ch9_ptr =
			(USBCH9_DATA *)XUsbPsu_get_drvdata(InstancePtr);
	struct dfu_if *DFU = (struct dfu_if *)(ch9_ptr->data_ptr);

	/* Check buffer pointer is there and buffer is big enough. */
	if (!BufPtr)
		return 0;

	if (BufLen < sizeof(USB_STD_DEV_DESC))
		return 0;

	Status = XUsbPsu_IsSuperSpeed(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* USB 2.0 */
		if (DFU->is_dfu == 1) {
			char *ptr = (char *)(&DFU->DFUdeviceDesc[0]);
			memcpy(BufPtr, ptr, sizeof(USB_STD_DEV_DESC));
		} else {
			memcpy(BufPtr, &deviceDesc[0],
					sizeof(USB_STD_DEV_DESC));
		}
	} else {
		/* USB 3.0 */
		if (DFU->is_dfu == 1) {
			char *ptr = (char *)(&DFU->DFUdeviceDesc[1]);
			memcpy(BufPtr, ptr, sizeof(USB_STD_DEV_DESC));
		} else {
			memcpy(BufPtr, &deviceDesc[1],
					sizeof(USB_STD_DEV_DESC));
		}
	}

	return sizeof(USB_STD_DEV_DESC);
}


/*****************************************************************************/
/**
*
* This function returns the configuration descriptor for the device.
*
* @param	BufPtr is the pointer to the buffer that is to be filled with
*		the descriptor.
* @param	BufLen is the size of the provided buffer.
*
* @return	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 XUsbPsu_Ch9SetupCfgDescReply(struct XUsbPsu *InstancePtr,
		u8 *BufPtr, u32 BufLen)
{
	s32 Status;
	char *config;
	u32 CfgDescLen;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)XUsbPsu_get_drvdata(InstancePtr);
	struct dfu_if *DFU = (struct dfu_if *)(ch9_ptr->data_ptr);

	/* Check buffer pointer is OK and buffer is big enough. */
	if (!BufPtr)
		return 0;

	if (BufLen < sizeof(USB_STD_CFG_DESC))
		return 0;

	Status = XUsbPsu_IsSuperSpeed(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* USB 2.0 */

		if (DFU->is_dfu == 1) {
			char *ptr = (char *)(DFU->DFUconfig2);
			config = (char *)ptr;
			CfgDescLen  = sizeof(DFU_USB_CONFIG);
		} else {
			config = (char *)&config2;
			CfgDescLen  = sizeof(USB_CONFIG);
		}
	} else {
		/* USB 3.0 */
		if (DFU->is_dfu == 1) {
			char *ptr = (char *)(DFU->DFUconfig3);
			config = (char *)ptr;
			CfgDescLen  = sizeof(DFU_USB30_CONFIG);
		} else {
			config = (char *)&config3;
			CfgDescLen  = sizeof(USB30_CONFIG);
		}
	}

	memcpy(BufPtr, config, CfgDescLen);

	return CfgDescLen;
}


/*****************************************************************************/
/**
*
* This function returns a string descriptor for the given index.
*
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
u32 XUsbPsu_Ch9SetupStrDescReply(struct XUsbPsu *InstancePtr,
		u8 *BufPtr,	u32 BufLen, u8 Index)
{
	int i;
	char *String;
	u32 StringLen;
	u32 DescLen;
	u8 TmpBuf[128];
	s32 Status;
	USBCH9_DATA *ch9_ptr =
			(USBCH9_DATA *)XUsbPsu_get_drvdata(InstancePtr);
	struct dfu_if *DFU = (struct dfu_if *)(ch9_ptr->data_ptr);
	USB_STD_STRING_DESC *StringDesc;

	if (Index >= (sizeof(StringList) / sizeof(char *)))
		return 0;

	if (!BufPtr)
		return 0;

	Status = XUsbPsu_IsSuperSpeed(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* USB 2.0 */
		if (DFU->is_dfu == 1) {

			char (*ptr)[MAX_STRLEN] = (char (*)[MAX_STRLEN])DFU->DFUStringList2;
			String = (char *)&ptr[Index];

		} else {
			String = (char *)&StringList[0][Index];
		}
	} else {
		/* USB 3.0 */
		if (DFU->is_dfu == 1) {
			char (*ptr)[MAX_STRLEN] = (char (*)[MAX_STRLEN])DFU->DFUStringList3;
			String = (char *)&ptr[Index];
		} else {
			String = (char *)&StringList[1][Index];
		}
	}

	StringLen = strlen(String);

	StringDesc = (USB_STD_STRING_DESC *) TmpBuf;

	/*
	 * Index 0 is langid which is special as we can not
	 * represent the string required in the table above.
	 * Therefore we handle index 0 as a special case.
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

		for (i = 0; i < StringLen; i++) {
			StringDesc->wLANGID[i] = (u16) String[i];
		}
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
u32 XUsbPsu_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufLen)
{

	/* Check buffer pointer is OK and buffer is big enough. */
	if (!BufPtr)
		return 0;

	if (BufLen < sizeof(USB_STD_BOS_DESC)) {
		return 0;
	}

	USB_BOS_DESC __attribute__ ((aligned(16))) bosDesc = {
		/* BOS descriptor */
		{sizeof(USB_STD_BOS_DESC), /* bLength */
		XUSBPSU_TYPE_BOS_DESC, /* DescriptorType */
		sizeof(USB_BOS_DESC), /* wTotalLength */
		0x02}, /* bNumDeviceCaps */

		{sizeof(USB_STD_DEVICE_CAP_7BYTE), /* bLength */
		0x10, /* bDescriptorType */
		0x02, /* bDevCapabiltyType */
#ifdef XUSBPSU_LPM_MODE
		0x06}, /* bmAttributes */
#else
		0x00}, /* Disable LPM/BESL for USB 2.0*/
#endif

		{sizeof(USB_STD_DEVICE_CAP_10BYTE), /* bLength */
		0x10, /* bDescriptorType */
		0x03, /* bDevCapabiltyType */
		0x00, /* bmAttributes */
		(0x000F), /* wSpeedsSupported */
		0x01, /* bFunctionalitySupport */
#ifdef XUSBPSU_LPM_MODE
		0x01, /* bU1DevExitLat */
		(0x01F4)} /* wU2DevExitLat */
#else
		0x00, /* Disable LPM for USB 3.0 */
		0x00} /* Disable LPM for USB 3.0 */
#endif
	};

	memcpy(BufPtr, &bosDesc, sizeof(USB_BOS_DESC));

	return sizeof(USB_BOS_DESC);
}


/****************************************************************************/
/**
* Changes State of Core to USB configured State.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Ctrl is a pointer to the Setup packet data.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_SetConfiguration(struct XUsbPsu *InstancePtr, SetupPacket *Ctrl)
{
	u8 State;
	s32 Ret = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Ctrl != NULL);

	State = InstancePtr->State;
	InstancePtr->IsConfigDone = 0U;

	switch (State) {

		case XUSBPSU_STATE_DEFAULT:
			Ret = XST_FAILURE;
			break;

		case XUSBPSU_STATE_ADDRESS:
			InstancePtr->State = XUSBPSU_STATE_CONFIGURED;
			break;

		case XUSBPSU_STATE_CONFIGURED:
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
* @param	InstancePtr is pointer to XUsbPsu instance.
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
s32 XUsbPsu_SetConfigurationApp(struct XUsbPsu *InstancePtr,
						SetupPacket *SetupData)
{

	/* Do nothing */

	return XST_SUCCESS;
}
