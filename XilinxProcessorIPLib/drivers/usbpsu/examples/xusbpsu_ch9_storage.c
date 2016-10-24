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
 * @file xusbpsu_ch9_storage.c
 *
 * This file contains the implementation of the storage specific chapter 9 code
 * for the example.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.0   sg   06/06/16 First release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include <string.h>

#include "xparameters.h"	/* XPAR parameters */
#include "xusbpsu.h"		/* USB controller driver */
#include "xusbpsu_ch9.h"
#include "xusbpsu_ch9_storage.h"
#include "xusbpsu_class_storage.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/
/*
 * Descriptor Types
 */
#define DEVICE_DESCR				0x01
#define CONFIG_DESCR				0x02
#define STRING_DESCR				0x03
#define INTERFACE_DESCR				0x04
#define ENDPOINT_DESCR				0x05
#define QUALIFIER_DESCR				0x06
#define OSD_CONFIG_DESCR			0x07
#define HID_DESC			      	0x21	// Get descriptor: HID
#define REPORT_DESC			        0x22	// Get descriptor:Report
#define DFUFUNC_DESCR				0x21  //DFU Functional Desc

/*
 * Standard USB structures as per 2.0 specification
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize0;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;
} __attribute__((__packed__))USB_STD_DEV_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u16 wTotalLength;
	u8 bNumberInterfaces;
	u8 bConfigValue;
	u8 bIConfigString;
	u8 bAttributes;
	u8 bMaxPower;
}  __attribute__((__packed__))USB_STD_CFG_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bInterfaceNumber;
	u8 bAlternateSetting;
	u8 bNumEndPoints;
	u8 bInterfaceClass;
	u8 bInterfaceSubClass;
	u8 bInterfaceProtocol;
	u8 iInterface;
}  __attribute__((__packed__))USB_STD_IF_DESC;


typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u8 bMaxPacketSizeL;
	u8 bMaxPacketSizeH;
	u8 bInterval;
}  __attribute__((__packed__))USB_STD_EP_DESC;

/*
 * SUPERSPEED USB ENDPOINT COMPANION descriptor structure
 */
typedef struct {
  u8 bLength;
  u8 bDescriptorType;
  u8 bMaxBurst;
  u8 bmAttributes;
  u16 wBytesPerInterval;
} __attribute__((__packed__))USB_STD_EP_SS_COMP_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 wLANGID[1];
}  __attribute__((__packed__))USB_STD_STRING_DESC;

typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg;
	USB_STD_EP_DESC epin;
	USB_STD_EP_DESC epout;
} __attribute__((__packed__))USB_CONFIG;

typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg;
	USB_STD_EP_DESC epin;
	USB_STD_EP_SS_COMP_DESC epssin;
	USB_STD_EP_DESC epout;
	USB_STD_EP_SS_COMP_DESC epssout;
} __attribute__((__packed__))USB30_CONFIG;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 wTotalLength;
	u8 bNumDeviceCaps;
} __attribute__((__packed__))USB_STD_BOS_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabiltyType;
	u32 bmAttributes;
} __attribute__((__packed__))USB_STD_DEVICE_CAP_7BYTE;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabiltyType;
	u8 bmAttributes;
	u16 wSpeedsSupported;
	u8 bFunctionalitySupport;
	u8 bU1DevExitLat;
	u16 wU2DevExitLat;
} __attribute__((__packed__))USB_STD_DEVICE_CAP_10BYTE;

typedef struct{
	USB_STD_BOS_DESC	bos_desc;
	USB_STD_DEVICE_CAP_7BYTE dev_cap7;
	USB_STD_DEVICE_CAP_10BYTE dev_cap10;
} __attribute__((__packed__))USB_BOS_DESC;

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern USB_CBW CBW;

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
		0x40,		/* bMaxPackedSize0 */
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
		0x09,		/* bMaxPackedSize0 */
		(0x0525),			/* idVendor */
		(0xA4A5),			/* idProduct */
		(0x0404),			/* bcdDevice */
		0x03,				/* iManufacturer */
		0x04,				/* iProduct */
		0x05,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	}};

/* Configuration Descriptors */
USB30_CONFIG __attribute__ ((aligned(16))) config3 = {
		/* Std Config */
		{sizeof(USB_STD_CFG_DESC),	/* bLength */
		 USB_CONFIG_DESC,		/* bDescriptorType */
		 sizeof(USB30_CONFIG),				/* wTotalLength */
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
		 0x08,				/* bInterfaceClass */
		 0x06,				/* bInterfaceSubClass */
		 0x50,				/* bInterfaceProtocol */
		 0x01},				/* iInterface */

		/* Bulk In Endpoint Config */
		{sizeof(USB_STD_EP_DESC),	/* bLength */
		 USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
		 0x81,				/* bEndpointAddress */
		 0x02,				/* bmAttribute  */
		 0x00,					/* wMaxPacketSize - LSB */
		 0x04,					/* wMaxPacketSize - MSB */
		 0x00},				/* bInterval */

		 /* SS Endpoint companion  */
		 {sizeof(USB_STD_EP_SS_COMP_DESC),	/* bLength */
		 0x30, 						/* bDescriptorType */
		 0x0F,						/* bMaxBurst */
		 0x00,			/* bmAttributes */
		 0x00},			/* wBytesPerInterval */

		/* Bulk Out Endpoint Config */
		{sizeof(USB_STD_EP_DESC),	/* bLength */
		 USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
		 0x01,				/* bEndpointAddress */
		 0x02,				/* bmAttribute  */
		 0x00,					/* wMaxPacketSize - LSB */
		 0x04,					/* wMaxPacketSize - MSB */
		 0x00},				/* bInterval */

		 /* SS Endpoint companion  */
		 {sizeof(USB_STD_EP_SS_COMP_DESC),	/* bLength */
		 0x30, 						/* bDescriptorType */
		 0x0F,						/* bMaxBurst */
		 0x00,			/* bmAttributes */
		 0x00}			/* wBytesPerInterval */
	};

USB_CONFIG __attribute__ ((aligned(16))) config2 = {
		/* Std Config */
		{sizeof(USB_STD_CFG_DESC),	/* bLength */
		 USB_CONFIG_DESC,		/* bDescriptorType */
		 sizeof(USB_CONFIG),	/* wTotalLength */
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
		 0x08,				/* bInterfaceClass */
		 0x06,				/* bInterfaceSubClass */
		 0x50,				/* bInterfaceProtocol */
		 0x05},				/* iInterface */

		/* Bulk In Endpoint Config */
		{sizeof(USB_STD_EP_DESC),	/* bLength */
		 USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
		 0x81,				/* bEndpointAddress */
		 0x02,				/* bmAttribute  */
		 0x00,					/* wMaxPacketSize - LSB */
		 0x02,					/* wMaxPacketSize - MSB */
		 0x00},				/* bInterval */

		/* Bulk Out Endpoint Config */
		{sizeof(USB_STD_EP_DESC),	/* bLength */
		 USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
		 0x01,				/* bEndpointAddress */
		 0x02,				/* bmAttribute  */
		 0x00,					/* wMaxPacketSize - LSB */
		 0x02,					/* wMaxPacketSize - MSB */
		 0x00}				/* bInterval */
	};

/* String Descriptors */
static char StringList[2][6][128] = {
			{
				"UNUSED",
				"Mass Storage",
				"USB 2.0 Flash Drive Disk Emulation",
				"2A49876D9CC1AA4",
				"Mass Storage Gadget",
				"Default Interface"
			},
			{
				"UNUSED",
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
* @param	BufPtr is pointer to the buffer that is to be filled
*		with the descriptor.
* @param	BufLen is the size of the provided buffer.
*
* @return 	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 XUsbPsu_Ch9SetupDevDescReply(struct XUsbPsu *InstancePtr,
		u8 *BufPtr, u32 BufLen)
{
	u8 Index;
	s32 Status;

	Status = XUsbPsu_IsSuperSpeed(InstancePtr);
	if(Status != XST_SUCCESS) {
		/* USB 2.0 */
		Index = 0;
	} else {
		/* USB 3.0 */
		Index = 1;
	}

	/* Check buffer pointer is there and buffer is big enough. */
	if (!BufPtr) {
		return 0;
	}

	if (BufLen < sizeof(USB_STD_DEV_DESC)) {
		return 0;
	}

	memcpy(BufPtr, &deviceDesc[Index], sizeof(USB_STD_DEV_DESC));

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
* @return 	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 XUsbPsu_Ch9SetupCfgDescReply(struct XUsbPsu *InstancePtr,
		u8 *BufPtr, u32 BufLen)
{
	s32 Status;
	char *config;
	u32 CfgDescLen;

	Status = XUsbPsu_IsSuperSpeed(InstancePtr);
	if(Status != XST_SUCCESS) {
		/* USB 2.0 */
		config = (char *)&config2;
		CfgDescLen  = sizeof(USB_CONFIG);
	} else {
		/* USB 3.0 */
		config = (char *)&config3;
		CfgDescLen  = sizeof(USB30_CONFIG);
	}

	/* Check buffer pointer is OK and buffer is big enough. */
	if (!BufPtr) {
		return 0;
	}

	if (BufLen < sizeof(USB_STD_CFG_DESC)) {
		return 0;
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
* @return 	Length of the descriptor in the buffer on success.
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
	u8 StrArray;

	USB_STD_STRING_DESC *StringDesc;

	Status = XUsbPsu_IsSuperSpeed(InstancePtr);
	if(Status != XST_SUCCESS) {
		/* USB 2.0 */
		StrArray = 0;
	} else {
		/* USB 3.0 */
		StrArray = 1;
	}

	if (!BufPtr) {
		return 0;
	}

	String = (char *)&StringList[StrArray][Index];

	if (Index >= sizeof(StringList) / sizeof(char *)) {
		return 0;
	}


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

		for (i = 0; i < StringLen; i++) {
			StringDesc->wLANGID[i] = (u16) String[i];
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


/*****************************************************************************/
/**
*
* This function returns the BOS descriptor for the device.
*
* @param	BufPtr is the pointer to the buffer that is to be filled with
*		the descriptor.
* @param	BufLen is the size of the provided buffer.
*
* @return 	Length of the descriptor in the buffer on success.
*		0 on error.
*
******************************************************************************/
u32 XUsbPsu_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufLen)
{

	USB_BOS_DESC __attribute__ ((aligned(16))) bosDesc = {
		/* BOS descriptor */
		{sizeof(USB_STD_BOS_DESC), /* bLength */
		XUSBPSU_TYPE_BOS_DESC, /* DescriptorType */
		sizeof(USB_BOS_DESC), /* wTotalLength */
		0x02}, /* bNumDeviceCaps */

		{sizeof(USB_STD_DEVICE_CAP_7BYTE), /* bLength */
		0x10, /* bDescriptorType */
		0x02, /* bDevCapabiltyType */
		0x06}, /* bmAttributes */

		{sizeof(USB_STD_DEVICE_CAP_10BYTE), /* bLength */
		0x10, /* bDescriptorType */
		0x03, /* bDevCapabiltyType */
		0x00, /* bmAttributes */
		(0x000F), /* wSpeedsSupported */
		0x01, /* bFunctionalitySupport */
		0x01, /* bU1DevExitLat */
		(0x01F4)} /* wU2DevExitLat */
	};

	/* Check buffer pointer is OK and buffer is big enough. */
	if (!BufPtr) {
		return 0;
	}

	if (BufLen < sizeof(USB_STD_BOS_DESC)) {
		return 0;
	}

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
	int RetVal;
	u16 MaxPktSize;

	if(InstancePtr->Speed == XUSBPSU_SPEED_SUPER) {
		MaxPktSize = 1024;
	} else {
		MaxPktSize = 512;
	}

	/* Endpoint enables - not needed for Control EP */
	RetVal = XUsbPsu_EpEnable(InstancePtr, 1, XUSBPSU_EP_DIR_IN, MaxPktSize,
							  XUSBPSU_ENDPOINT_XFER_BULK);
	if (RetVal != XST_SUCCESS) {
		xil_printf("failed to enable BULK IN Ep\r\n");
		return XST_FAILURE;
	}

	RetVal = XUsbPsu_EpEnable(InstancePtr, 1, XUSBPSU_EP_DIR_OUT, MaxPktSize,
							  XUSBPSU_ENDPOINT_XFER_BULK);
	if (RetVal != XST_SUCCESS) {
		xil_printf("failed to enable BULK OUT Ep\r\n");
		return XST_FAILURE;
	}

	/*
     * As per Mass storage specification we receive 31 byte length
	 * Command Block Wrapper first. So lets make OUT Endpoint ready
	 * to receive it. OUT Ep Handler will be called when data is
	 * received
	 */
	XUsbPsu_EpBufferRecv(InstancePtr, 1, (u8*)&CBW, sizeof(CBW));

	return XST_SUCCESS;
}
