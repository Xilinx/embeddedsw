/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 * @file xusb_types.h
 *
 * This file contains the constants, type definitions, variables as used in the
 * USB chapter 9 and mass storage demo application.
 *
 * @note     None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 1.00a hvm  2/22/07 First release
 * 4.00a hvm  12/20/10 Updated with the Audio device definitions.
 *
 * </pre>
 *****************************************************************************/

#ifndef __XUSBTYPES_H__
#define __XUSBTYPES_H__
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Application specific constant.
 */
#undef HID_DEVICES
#undef USB_KEYBOARD
#undef USB_MOUSE
#define MASS_STORAGE_DEVICE
#undef MICROPHONE
/*
 * Chapter 9 requests
 */
#define GET_STATUS				0x00
#define CLEAR_FEATURE				0x01
#define GET_STATE				0x02
#define SET_FEATURE				0x03
#define SET_ADDRESS				0x05
#define GET_DESCRIPTOR				0x06
#define SET_DESCRIPTOR				0x07
#define GET_CONFIGURATION			0x08
#define SET_CONFIGURATION		 	0x09
#define GET_INTERFACE				0x0A
#define SET_INTERFACE				0x0B
#define SYCH_FRAME				0x0C

/*
 * Test Mode Options
 */
#define TEST_J					1
#define TEST_K					2
#define TEST_SE0_NAK				3
#define TEST_PKT				4

/*
 * USB Mass Storage requests
 */
#define MS_RESET				0xFF
#define MS_GETMAXLUN		   		0xFE

/*
 * USB Human Interface Device constant.
 */
#define USB_CLASS_HID			0x03

/*
 * Request types used during USB enumeration.
 */
#define STANDARD_IN_DEVICE			0x80
#define STANDARD_IN_INTERFACE			0x81
#define STANDARD_IN_ENDPOINT			0x82
#define STANDARD_OUT_DEVICE			0x00
#define STANDARD_OUT_INTERFACE			0x01
#define STANDARD_OUT_ENDPOINT			0x02
#define TYPE_MASK			 	0x60
#define TYPE_STANDARD				0x00
#define TYPE_CLASS				0x20
#define TYPE_VENDOR				0x40
#define TYPE_RESERVED				0x60

/*
 * DATA Transfer Direction
 */
#define DIR_DEVICE_TO_HOST			0x80

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

/*
 * Feature Selectors
 */

#define DEVICE_REMOTE_WAKEUP	0x01
#define TEST_MODE				0x02

/*
 * Phase States
 */
#define SETUP_PHASE				0x0000
#define DATA_PHASE				0x0001
#define STATUS_PHASE		   	0x0002

/*
 * End point types.
 */
#define EP_CONTROL			0	/**< Control Endpoint */
#define EP_ISOCHRONOUS			1	/**< Isochronous Endpoint */
#define EP_BULK				2	/**< Bulk Endpoint */
#define EP_INTERRUPT			3	/**< Interrupt Endpoint */

/*
 * Maximum number of USB interfaces.
 */
#define MAX_INTERFACES			0x01

/*
 * FPGA Configuration Number
 */
#define CONFIGURATION_ONE	   	0x01

/*
 * EP0 Setup data size.
 */
#define EP0_SETUP_DATA				64

/*
 * Command Buffer Structure.
 */
typedef struct {
	union {
		u8 StandardDeviceRequest;
		u8 bmRequestType;
	} Byte0;
	union {
		u8 FbRequest;
		u8 bRequest;
	} Byte1;
	union {
		struct {
			u8 bDescriptorType;
			u8 bDescriptorIndex;
		} Byte23;
		u16 FwValue;
		u16 wValue;
		u16 wFeatureSelector;
	} Word1;
	union {
		struct {
			u8 Byteh;
			u8 Bytel;
		} Byte45;
		u16 wTargetSelector;
		u16 FwIndex;
		u16 wIndex;
	} Word2;
	union {
		struct {
			u8 Byteh;
			u8 Bytel;
		} Byte67;
		u16 wLength;
	} Word3;
	u8 *ContReadPtr;
	u8 *ContWritePtr;
	u32 ContReadCount;
	u32 ContWriteCount;
	u32 SetupSeqTX;
	u32 SetupSeqRX;
	u8 ContReadDataBuffer[EP0_SETUP_DATA];
} USB_CMD_BUF;

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

} USB_STD_DEV_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bVersionLow;
	u8 bVersionHigh;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bProtocol;
	u8 bMaxPkt0;
	u8 bNumberConfigurations;
	u8 breserved;
} USB_STD_QUAL_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bTotalLength;
	u8 bCorrection;
	u8 bNumberInterfaces;
	u8 bConfigValue;
	u8 bIConfigString;
	u8 bAttributes;
	u8 bMaxPower;
} USB_STD_CFG_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bTotalLength;
	u8 bCorrection;
	u8 bNumberInterfaces;
	u8 bConfigValue;
	u8 bIConfigString;
	u8 bAttributes;
	u8 bMaxPower;
} USB_STD_QUA_DESC;

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

} USB_STD_IF_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bcdHIDL;
	u8 bcdHIDH;
	u8 bCountryCode;
	u8 bNumDescriptors;
	u8 bReportDescriptorType;
	u8 wDescriptorLengthL;
	u8 wDescriptorLengthH;

} USB_STD_HID_DESC;

/*
 * USB class specific audio interface header descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bcdADCL;
	u8 bcdADCH;
	u8 wTotalLengthL;
	u8 wTotalLengthH;
	u8 bInCollection;
	u8 baInterfaceNr;

}USB_CLASS_SPECIFIC_AC_IF_HEADER_DESC;

/*
 * USB class specific audio Input terminal descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalId;
	u8 wTerminalTypeL;
	u8 wTerminalTypeH;
	u8 bAssocTerminal;
	u8 bNrChannels;
	u8 wChannelConfigL;
	u8 wChannelConfigH;
	u8 iChannelNames;
	u8 iTerminal;

}USB_INPUT_TERMINAL_DESC;

/*
 * USB class specific audio Output terminal descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalId;
	u8 wTerminalTypeL;
	u8 wTerminalTypeH;
	u8 bAssocTerminal;
	u8 bSourceId;
	u8 iTerminal;

}USB_OUTPUT_TERMINAL_DESC;

/*
 * USB class specific audio feature unit descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bUnitID;
	u8 bSourceID;
	u8 bControlsize;
	u8 bmaControls0;
	u8 bmaControls1;
	u8 bmaControls2;
	u8 iFeature;

}USB_FEATURE_UNIT_DESC;

/*
 * USB class specific audio class specific AS General Inerface descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalLink;
	u8 bDelay;
	u8 wFormatTagL;
	u8 wFormatTagH;

}USB_CLASS_SPECIFIC_AS_IF_DESC;

/*
 * USB audio Type I Format descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bFormatType;
	u8 bNrChannels;
	u8 bSubFrameSize;
	u8 bBitResolution;
	u8 bSamFreqType;
	u8 tSamFreByteOneL; /* LSB of the three bytes data */
	u8 tSamFreByteTwo;
	u8 tSamFreByteThreeH;/* MSB of the three bytes data */

}USB_AUDIO_TYPE_I_FORMAT_DESC;

/*
 * USB Standard Audio Endpoint descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u8 bMaxPacketSizeL;
	u8 bMaxPacketSizeH;
	u8 bInterval;
	u8 bRefresh;
	u8 bSyncAddress;

}USB_STD_AUDIO_EP_DESC;

/*
 * USB Class Specific Audio Data Endpoint descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bmAttributes;
	u8 bLockDelayUnits;
	u8 wLockDelayL;
	u8 wLockDelayH;

}USB_CLASS_SPECIFIC_AUDIO_DATA_EP_DESC;

/*
 * The standard USB structures as per USB 2.0 specification.
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u8 bMaxPacketSizeL;
	u8 bMaxPacketSizeH;
	u8 bInterval;
} USB_STD_EP_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u16 wLANGID[1];
} USB_STD_STRING_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bString[14];
} USB_STD_STRING_MAN_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bString[10];
} USB_STD_STRING_PS_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bString[42];
} USB_STD_STRING_SN_DESC;

#ifdef MASS_STORAGE_DEVICE
/*
 * USB configuration structure.
 */
typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg;
	USB_STD_EP_DESC epCfg1;
	USB_STD_EP_DESC epCfg2;
} FPGA1_CONFIGURATION;

#endif

#ifdef HID_DEVICES
/*
 * USB configuration structure.
 */
typedef struct {
	USB_STD_CFG_DESC	stdCfg;
	USB_STD_IF_DESC		ifCfg;
	USB_STD_HID_DESC	hidCfg;
	USB_STD_EP_DESC		epCfg1;

} FPGA1_CONFIGURATION;

#endif

#ifdef MICROPHONE
typedef struct {
	USB_STD_IF_DESC				ifCfg;
	USB_CLASS_SPECIFIC_AS_IF_DESC 		audifCfg;
	USB_AUDIO_TYPE_I_FORMAT_DESC		typeICfg;
	USB_STD_AUDIO_EP_DESC			audEpCfg;
	USB_CLASS_SPECIFIC_AUDIO_DATA_EP_DESC 	audDataEpCfg;

} USB_ALT_CFG_SET;
/*
 * USB configuration structure.
 */
typedef struct {
	USB_STD_CFG_DESC			stdCfg;
	USB_STD_IF_DESC				ifCfg;
	USB_CLASS_SPECIFIC_AC_IF_HEADER_DESC	adIfHdCfg;
	USB_INPUT_TERMINAL_DESC			ipTermCfg;
	USB_OUTPUT_TERMINAL_DESC		opTermCfg0;
	USB_STD_IF_DESC				altIfCfg;
	USB_ALT_CFG_SET				altCfg;

} FPGA1_CONFIGURATION;

#endif
#ifdef __cplusplus
}
#endif
#endif /* __XUSBTYPES_H__ */

