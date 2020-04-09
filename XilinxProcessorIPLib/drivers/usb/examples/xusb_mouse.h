/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 * @file xusb_mouse.h
 *
 * This file contains the constants, type definitions, variables and function
 * prototypes used in the mouse application.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 1.00a hvm  3/30/07 First release
 * 3.02a hvm  08/16/10 Updated with the little endian support changes.
 *
 * </pre>
 *****************************************************************************/

#ifndef  XUSB_MOUSE_H
#define  XUSB_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/

#include "xusb_cp9.h"

/************************** Constant Definitions ******************************/

/*
 * Mouse interface class
 */
#define MOUSE_PROTOCOL			2
#define HID_DESCR			0x21

/************************** Variable Definitions ******************************/

/*
 * Flags used to abort read and write command loops.
 */
extern u16 MaxControlSize;


USB_STD_DEV_DESC DeviceDescriptor __attribute__ ((aligned (4))) = {
	sizeof (USB_STD_DEV_DESC), 	/* Descriptor Size 18 bytes */
	DEVICE_DESCR, 			/* This is a device descriptor */
#ifdef __LITTLE_ENDIAN__
		0x0200,		/* USB version */
#else
		0x02,		/* USB version */
#endif
	0xFF, 				/* Vendor Specific */
	00, 				/* Unused */
	00, 				/* Unused */
	0x40, 				/* Ep0 Max Pkt Size 64 bytes */
#ifdef __LITTLE_ENDIAN__
	0x03FD,				/* Vendor Id */
	0x0300,				/* Product Id */
	0x0100,				/* BCD device */
#else
	0xFD03,				/* Vendor Id */
	0x0003,				/* Product Id */
	0x01,				/* BCD device */
#endif
	01, 				/* String Index of manufacturer */
	02, 				/* String Index of product */
	03, 				/* String Index of serial number */
	01				/* Number of configurations */

};

USB_STD_QUAL_DESC QualifierDescriptor __attribute__ ((aligned(4))) = {
sizeof(USB_STD_QUAL_DESC),
		QUALIFIER_DESCR, 00, 02, 0, 00, 00, 0x40, 01, 0};

FPGA1_CONFIGURATION  __attribute__ ((aligned (4))) FsUsbConfig  = {

	/*
	 * Configuration descriptor.
	 */
	{
		sizeof(USB_STD_CFG_DESC),/* Size of config descriptor 9 bytes */
		CONFIG_DESCR, 		 /* This is a conifig descriptor */
		sizeof(FsUsbConfig), 	 /* Total size of configuration LS */
		0x00,			 /* Total size of configuration MS */
		0x01, 			 /* No. Of interfaces 1 */
		CONFIGURATION_ONE, 	 /* No of configuration values */
		0x00, 			 /* Configuration string */
		0xc0, 			 /* Self Powered */
		0x01			 /* Uses 2mA from the USB bus */
	},

	/*
	 * FPGA1 Class interface.
	 */
	{
		sizeof(USB_STD_IF_DESC), /* Interface Descriptor size 9 bytes */
		INTERFACE_DESCR,	 /* This is an interface descriptor */
		0x00,			 /* Interface number 0 */
		0x00,			 /* Alternate set 0 */
		0x01,			 /* Number of end points 1 */
		USB_CLASS_HID,	 /* Vendor specific */
		1,	 		/* Interface sub class */
		MOUSE_PROTOCOL,		 /* Mouse */
		0x00			 /* Interface unused */
	},

	/*
	 * HID descriptor
	 */
	{
		sizeof(USB_STD_HID_DESC), /* HID descriptor Length */
		HID_DESCR, 		  /* This is an HID descriptor */
		0x10,			/* bcdHID L */
		0x01,			/* bcdHID H Rev 1.1 */
		0x00,			/* bCountryCode (none) */
		0x01,			/* bNumDescriptors
					(one report descriptor) */
		0x22,			/* bDescriptorType (report)*/
		0x34,                    /* wDescriptorLength
					(L/H) (report descriptor size
					is 43 bytes) */
		0x00
	},

	/*
	 * End_point  1 RX descriptor  from device to host.
	 */
	{
		sizeof(USB_STD_EP_DESC), /* End point descriptor size */
		ENDPOINT_DESCR,		 /* This is an end point descriptor */
		0x81,			 /* End point one */
		EP_INTERRUPT,		 /* End point type */
		0x40,			 /* Maximum packet  size 64 bytes LS */
		0x00,			 /* Maximum packetsize MS */
		0x0A			 /* Nak rate */
	},

};

FPGA1_CONFIGURATION  __attribute__ ((aligned (4))) HsUsbConfig  = {

	/*
	 * Configuration descriptor.
	 */
	{
		sizeof(USB_STD_CFG_DESC),/* Size of config descriptor 9 bytes */
		CONFIG_DESCR, 		 /* This is a conifig descriptor */
		sizeof(HsUsbConfig), 	 /* Total size of configuration LS */
		0x00,			 /* Total size of configuration MS */
		0x01, 			 /* No. Of interfaces 1 */
		CONFIGURATION_ONE, 	 /* No of configuration values */
		0x00, 			 /* Configuration string */
		0xc0, 			 /* Self Powered */
		0x01			 /* Uses 2mA from the USB bus */
	},

	/*
	 * FPGA1 Class interface.
	 */
	{
		sizeof(USB_STD_IF_DESC), /* Interface Descriptor size 9 bytes */
		INTERFACE_DESCR,	 /* This is an interface descriptor */
		0x00,			 /* Interface number 0 */
		0x00,			 /* Alternate set 0 */
		0x01,			 /* Number of end points 1 */
		USB_CLASS_HID,	 /* Vendor specific */
		1,	 		/* Interface sub class */
		MOUSE_PROTOCOL,		 /* Mouse */
		0x00			 /* Interface unused */
	},

	/*
	 * HID descriptor
	 */
	{
		sizeof(USB_STD_HID_DESC), /* HID descriptor Length */
		HID_DESCR, 		  /* This is an HID descriptor */
		0x10,			/* bcdHID L */
		0x01,			/* bcdHID H Rev 1.1 */
		0x00,			/* bCountryCode (none) */
		0x01,			/* bNumDescriptors
					(one report descriptor) */
		0x22,			/* bDescriptorType (report)*/
		0x34,                    /* wDescriptorLength
					(L/H) (report descriptor size
					is 43 bytes) */
		0x00
	},

	/*
	 * End_point  1 RX descriptor  from device to host.
	 */
	{
		sizeof(USB_STD_EP_DESC), /* End point descriptor size */
		ENDPOINT_DESCR,		 /* This is an end point descriptor */
		0x81,			 /* End point one */
		EP_INTERRUPT,		 /* End point type */
		0x40,			 /* Maximum packet  size 64 bytes LS */
		0x00,			 /* Maximum packetsize MS */
		0x0A			 /* Nak rate */
	},

};

USB_STD_STRING_DESC LangId __attribute__ ((aligned (4))) =
{
	/*
	 * Language ID codes.
	 */
	4, STRING_DESCR, {0x0409}
};

USB_STD_STRING_MAN_DESC Manufacturer __attribute__ ((aligned (4))) =
{
	/*
	 * Manufacturer String.
	 */
	sizeof(USB_STD_STRING_MAN_DESC), STRING_DESCR,
	{'X',0,'I',0,'L',0,'I',0,'N',0,'X',0,' ',0}
};

USB_STD_STRING_PS_DESC ProductString __attribute__ ((aligned (4))) =
{
	/*
	 * Product ID String.
	 */
	sizeof(USB_STD_STRING_PS_DESC), STRING_DESCR,
	{'F',0,'P',0,'G',0,'A',0,'3',0}
};

USB_STD_STRING_SN_DESC SerialNumber __attribute__ ((aligned (4))) =
{
	/*
	 * Product ID String.
	 */
	sizeof(USB_STD_STRING_SN_DESC), STRING_DESCR,
	{'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'1',0,
	'7',0,'1',0,'5',0,'0',0,'4',0,'2',0,'6',0,'2',0,
	'0',0,'0',0,'5',0,'7',0,'4',0}
};

USB_STD_HID_DESC HidDescriptor __attribute__ ((aligned(4))) = {
		sizeof(USB_STD_HID_DESC),	/* bLength */
			0x21,			/* bDescriptorType = HID */
			0x10,			/* bcdHID L */
			0x01,			/* bcdHID H Rev 1.1 */
			0x00,			/* bCountryCode (none) */
			0x01,			/* bNumDescriptors
						(one report descriptor) */
			0x22,			/* bDescriptorType (report)*/
			0x34,                    /* wDescriptorLength
						(L/H) (report descriptor size
						is 43 bytes) */
			0x00
};


/************************** Function Prototypes *******************************/

void  	InitUsbInterface(XUsb *InstancePtr);
void 	UsbIfIntrHandler(void *CallBackRef, u32 IntrStatus);
void 	EpIntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
void 	Ep0IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
void 	Ep1IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_STORAGE_H */










