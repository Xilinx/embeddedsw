/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 * @file xusb_keyboard.h
 *
 * This file contains the constants, type definitions, variables and function
 * prototypes used in the USB keyboard example.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 1.00a hvm  5/31/07 First release
 * 3.02a hvm  08/16/10 Updated with the little endian support changes.
 * 4.00a hvm  08/11/11 Updated the Message[] variable data to
 *			handle the address alignment issue.
 * 4.02a bss  11/01/11 Number of endpoints changed from 0x00 to 0x01 in
 *			FsUsbConfig to support Full Speed (CR 627573).
 *
 * </pre>
 *****************************************************************************/

#ifndef  XUSB_KEYBOARD_H
#define  XUSB_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/

#include "xusb_cp9.h"

/************************** Constant Definitions ******************************/

/************************** Variable Definitions ******************************/

/*
 * Flags used to abort read and write command loops.
 */
extern u16 MaxControlSize;

USB_STD_DEV_DESC DeviceDescriptor __attribute__ ((aligned(4))) = {
	sizeof(USB_STD_DEV_DESC),	/* Descriptor Size 18 bytes */
		DEVICE_DESCR,	/* This is a device descriptor */
#ifdef __LITTLE_ENDIAN__
		0x0200,		/* USB version */
#else
		0x02,		/* USB version */
#endif
		0,		/* Vendor Specific */
		00,		/* Unused */
		00,		/* Unused */
		0x40,		/* Ep0 Max Pkt Size 64 bytes */


#ifdef __LITTLE_ENDIAN__
		0x03FD,		/* Vendor Id */
		0x0200,		/* Product Id */
		0x0100,		/* BCD device */
#else
		0xFD03,		/* Vendor Id */
		0x0002,		/* Product Id */
		0x01,		/* BCD device */
#endif
		01,		/* String Index of manufacturer */
		02,		/* String Index of product */
		03,		/* String Index of serial number */
		01		/* Number of configurations */
};

USB_STD_QUAL_DESC QualifierDescriptor __attribute__ ((aligned(4))) = {
sizeof(USB_STD_QUAL_DESC),
		QUALIFIER_DESCR, 00, 02, 0, 00, 00, 0x40, 01, 0};

FPGA1_CONFIGURATION __attribute__ ((aligned(4))) HsUsbConfig = {

	{
		/*
		 * Configuration descriptor.
		 */
		sizeof(USB_STD_CFG_DESC),/* Size of config descriptor 9
						bytes */
			CONFIG_DESCR,	/* This is a conifig descriptor */
			sizeof(HsUsbConfig),	/* Total size of configuration
			LS */
			0x00,	/* Total size of configuration MS */
			0x01,	/* No. Of interfaces 1 */
			CONFIGURATION_ONE,	/* No of configuration values */
			0x00,	/* Configuration string */
			0xC0,	/* Self Powered */
			0x01	/* Uses 2mA from the USB bus */
	}
	,
	{
		/*
		 * FPGA1 Class interface.
		 */
		sizeof(USB_STD_IF_DESC),	/* Interface Descriptor size 9
							bytes */
			INTERFACE_DESCR,	/* This is an interface
						descriptor */
			0x00,			/* Interface number 0 */
			0x00,			/* Alternate set 0 */
			0x01,			/* Number of end points 1 */
			USB_CLASS_HID,		/* Vendor specific */
			0x00,			/* Interface sub class */
			0x00,			/* Interface protocol  */
			0x00			/* Interface unused */
	},
	{
		/*
		 * HID Descriptor
		 */
		sizeof(USB_STD_HID_DESC),	/* bLength */
			0x21,			/* bDescriptorType = HID */
			0x10,			/* bcdHID L */
			0x01,			/* bcdHID H Rev 1.1 */
			0x00,			/* bCountryCode (none) */
			0x01,			/* bNumDescriptors
						(one report descriptor) */
			0x22,			/* bDescriptorType (report)*/
			0x2b,			/* wDescriptorLength
						(L/H) (report descriptor size
						is 43 bytes) */
			0x00
	},
	{
		/*
		 * End_point 1 RX descriptor  from device to host.
		 */
		sizeof(USB_STD_EP_DESC),	/* End point descriptor size */
			ENDPOINT_DESCR,	/* This is an end point descriptor */
			0x81,		/* End point one */
			EP_INTERRUPT,	/* End point type */
			0x40,	/* Maximum packet  size 64 bytes LS */
			0x00,	/* Maximum packetsize MS */
			0x10	/* Nak rate */
	}

};

FPGA1_CONFIGURATION __attribute__ ((aligned(4))) FsUsbConfig = {

	{
		/*
		 * Configuration descriptor.
		 */
		sizeof(USB_STD_CFG_DESC),	/* Size of config descriptor 9
		bytes */
			CONFIG_DESCR,	/* This is a conifig descriptor */
			sizeof(FsUsbConfig),	/* Total size of configuration
			LS */
			0x00,	/* Total size of configuration MS */
			0x01,	/* No. Of interfaces 1 */
			CONFIGURATION_ONE,	/* No of configuration values */
			0x00,	/* Configuration string */
			0xC0,	/* Self Powered */
			0x01	/* Uses 2mA from the USB bus */
	},
	{
		/*
		 * FPGA1 Class interface.
		 */
		sizeof(USB_STD_IF_DESC),	/* Interface Descriptor size 9
							bytes */
			INTERFACE_DESCR,	/* This is an interface
						descriptor */
			0x00,			/* Interface number 0 */
			0x00,			/* Alternate set 0 */
			0x01,			/* Number of end points 1 */
			USB_CLASS_HID,		/* Vendor specific */
			0x00,			/* Interface sub class */
			0x00,			/* Interface protocol  */
			0x00			/* Interface unused */
	},
	{
		/*
		 * HID Descriptor
		 */
		sizeof(USB_STD_HID_DESC),	/* bLength */
			0x21,			/* bDescriptorType = HID */
			0x10,			/* bcdHID L */
			0x01,			/* bcdHID H Rev 1.1 */
			0x00,			/* bCountryCode (none) */
			0x01,			/* bNumDescriptors
						(one report descriptor) */
			0x22,			/* bDescriptorType (report)*/
			0x2b,			/* wDescriptorLength
						(L/H) (report descriptor size
						is 43 bytes) */
			0x00
	},
	{
		/*
		 * End_point  1 descriptor  from device to host.
		 */
		sizeof(USB_STD_EP_DESC),	/* End point descriptor size */
			ENDPOINT_DESCR,	/* This is an end point descriptor */
			0x81,		/* End point one */
			EP_INTERRUPT,	/* End point type */
			0x40,	/* Maximum packet  size 64 bytes LS */
			0x00,	/* Maximum packetsize MS */
			0x10	/* Nak rate */
	}

};



USB_STD_STRING_DESC LangId __attribute__ ((aligned(4))) = {
	/*
	 * Language ID codes.
	 */
	4, STRING_DESCR, {
	0x0904}
};

USB_STD_STRING_MAN_DESC Manufacturer __attribute__ ((aligned(4))) = {
	/*
	 * Manufacturer String.
	 */
	sizeof(USB_STD_STRING_MAN_DESC), STRING_DESCR, {
	'X', 0, 'I', 0, 'L', 0, 'I', 0, 'N', 0, 'X', 0, ' ', 0}
};

USB_STD_STRING_PS_DESC ProductString __attribute__ ((aligned(4))) = {
	/*
	 * Product ID String.
	 */
	sizeof(USB_STD_STRING_PS_DESC), STRING_DESCR, {
	'F', 0, 'P', 0, 'G', 0, 'A', 0, '2', 0}
};

USB_STD_STRING_SN_DESC SerialNumber __attribute__ ((aligned(4))) = {
	/*
	 * Product ID String.
	 */
	sizeof(USB_STD_STRING_SN_DESC), STRING_DESCR, {
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '1', 0,
			'7', 0, '1', 0, '5', 0, '0', 0, '4', 0, '2', 0,
			'6', 0, '2', 0, '0', 0, '0', 0, '5', 0, '7', 0, '6', 0}
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
			0x2b,			/* wDescriptorLength
						(L/H) (report descriptor size
						is 43 bytes) */
			0x00
};

/*Xilinx specific message*/
unsigned char Message[] __attribute__ ((aligned(4)))= {
	0x00,0x00,0x28,0x00,		// (cr)
	0x02,0x00,0x1B,0x00,		// X
	0x02,0x00,0x0C,0x00,		// I
	0x02,0x00,0x0F,0x00,		// L
	0x02,0x00,0x0C,0x00,		// I
	0x02,0x00,0x11,0x00,		// N
	0x02,0x00,0x1B,0x00,		// X
	0x00,0x00,0x2C,0x00,		// (sp)
	0x02,0x00,0x18,0x00,		// U
	0x02,0x00,0x16,0x00,		// S
	0x02,0x00,0x05,0x00,		// B
	0x00,0x00,0x2C,0x00,		// (sp)
	0x02,0x00,0x0E,0x00,		// K
	0x02,0x00,0x08,0x00,		// E
	0x02,0x00,0x1C,0x00,		// Y
	0x02,0x00,0x05,0x00,		// B
	0x02,0x00,0x12,0x00,		// O
	0x02,0x00,0x04,0x00,		// A
	0x02,0x00,0x15,0x00,		// R
	0x02,0x00,0x07,0x00,		// D
	0x00,0x00,0x2C,0x00,		// (sp)
	0x02,0x00,0x07,0x00,		// D
	0x02,0x00,0x08,0x00,		// E
	0x02,0x00,0x10,0x00,		// M
	0x02,0x00,0x12,0x00,		// O
	0x00,0x00,0x28,0x00};	//(cr)


/************************** Function Prototypes *******************************/

void InitUsbInterface(XUsb * InstancePtr);
void UsbIfIntrHandler(void *CallBackRef, u32 IntrStatus);
void Ep0IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
void Ep1IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_KEYBOARD_H */

