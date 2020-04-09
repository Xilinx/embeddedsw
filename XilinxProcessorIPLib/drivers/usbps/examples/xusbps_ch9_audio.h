/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xusbps_ch9_audio.h
*
* This file contains the implementation of chapter 9 specific code for
* the example.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who	Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   pm	20/02/20 First release
*
* </pre>
*
*****************************************************************************/
#ifndef XUSB_CH9_AUDIO_H
#define XUSB_CH9_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xusbps_ch9.h"
#include "xparameters.h"	/* XPAR parameters */
#include <string.h>
#include "xusbps.h"

/************************** Constant Definitions *****************************/
/* UAC1 works fine with Linux */
/* UAC2 works fine with Linux but not with Windows */
/*
 * UAC2 native driver support has been added in Windows 10 Creator's Update,
 * however, it does not work.
 */

/*
 * Define this for UAC2 configuration otherwise UAC1
 * is enabled by default.
 */
#define XUSBPS_UAC1

#ifdef XUSBPS_UAC1
/* Undef this for Speaker configuration */
#define XUSBPS_MICROPHONE
#endif

/**
 * @name Endpoint Type
 * @{
 */
#define USB_EP_TYPE_NONE		XUSBPS_EP_TYPE_NONE
#define USB_EP_TYPE_CONTROL		XUSBPS_EP_TYPE_CONTROL
#define USB_EP_TYPE_ISOCHRONOUS	XUSBPS_EP_TYPE_ISOCHRONOUS
#define USB_EP_TYPE_BULK		XUSBPS_EP_TYPE_BULK
#define USB_EP_TYPE_INTERRUPT	XUSBPS_EP_TYPE_INTERRUPT
/* @} */

/*
 * USB types, the second of three bRequestType fields
 */
#define XUSBPS_TYPE__CLASS			(0x01 << 5)

/*
 * Conventional codes for class-specific descriptors.  The convention is
 * defined in the USB "Common Class" Spec (3.11).  Individual class specs
 * are authoritative for their usage, not the "common class" writeup.
 */
#define XUSBPS_DT_CS_INTERFACE	(XUSBPS_TYPE__CLASS | XUSBPS_TYPE_IF_CFG_DESC)
#define XUSBPS_DT_CS_ENDPOINT	(XUSBPS_TYPE__CLASS | XUSBPS_TYPE_ENDPOINT_CFG_DESC)

/*
 * Audio Interface Subclass Codes
 */
#define USB_SUBCLASS_AUDIOCONTROL		0x01
#define USB_SUBCLASS_AUDIOSTREAMING		0x02

/*
 * Audio Class-Specific AC Interface Descriptor Subtypes
 */
#define UAC_INPUT_TERMINAL			0x02
#define UAC_OUTPUT_TERMINAL			0x03
#define UAC_FEATURE_UNIT			0x06

/*
 * Format Type Codes
 */
#define UAC_FORMAT_TYPE_I			0x1
#define UAC_FORMAT_TYPE_SUBTYPE		0x02

/*
 * Audio Class-Specific AS Interface Descriptor Subtypes
 */
#define UAC_AS_GENERAL				0x01

/*
 * Audio Class-Specific Endpoint Descriptor Subtypes
 */
#define UAC_EP_GENERAL				0x01

#ifdef XUSBPS_UAC1

/*
 * bInterfaceProtocol values to denote the version of the standard used
 */
#define UAC_VERSION			0x00

/*
 * Terminal/Unit ID
 */
#define USB_IT_ID		1
#define USB_OT_ID		2
#define FETR_UNT_ID		3

/*
 * Number of channel in AUDIO
 * Stereo->2 / Mono->1
 */
#define AUDIO_CHANNEL_NUM		0x01

#else 	/*	UAC2 */

/*
 * bInterfaceProtocol values to denote the version of the standard used
 */
#define UAC_VERSION			0x20

/*
 * Terminal Unit
 */
#define USB_OUT_IT_ID			1
#define USB_OUT_OT_ID			2
#define USB_IN_IT_ID			3
#define USB_IN_OT_ID			4


/*
 * Clock Control Unit
 */
#define NUM_CLK_SRC				1
#define USB_CLK_SRC_ID			5
#define USB_CLK_SEL_ID			6

/*
 * Feature Unit
 */
#define OUT_FETR_UNT_ID			7
#define IN_FETR_UNT_ID			8

#define CONTROL_RDONLY			1
#define CONTROL_RDWR			3

#define CLK_FREQ_CTRL			0
#define COPY_CTRL				0

/*
 * Audio Function Category Codes
 */
#define UAC2_FUNCTION_SUBCLASS_UNDEFINED	0x00
#define UAC2_FUNCTION_IO_BOX				0x08

/*
 * Audio Class-Specific Clock Source Descriptor Subtype
 */
#define UAC2_CLOCK_SOURCE			0x0a

/*
 * Audio Class-Specific Clock Selector Descriptor Subtype
 */
#define UAC2_CLOCK_SELECTOR			0x0b

/*
 * bmAttribute fields
 */
#define UAC2_CLOCK_SOURCE_TYPE_INT_FIXED		0x1

/*
 * Audio Data Format Type I Codes
 */
#define UAC2_FORMAT_TYPE_I_PCM		0x01

/*
 * The USB 3.0 spec redefines bits 5:4 of bmAttributes as interrupt ep type.
 */
#define USB_ENDPOINT_SYNC_ASYNC		(1 << 2)

/*
 * Number of channel in AUDIO
 * Stereo->2 / Mono->1
 */
#define AUDIO_CHANNEL_NUM		0x02

#endif

/**
 * @name Endpoint Address
 * @{
 */
#define USB_EP1_IN		0x81
#define USB_EP1_OUT		0x01
#define USB_EP2_IN		0x82
#define USB_EP2_OUT		0x02
/* @} */
/**************************** Type Definitions *******************************/
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

typedef struct {
u8 bLength;
u8 bDescriptorType;
u16 wLANGID[1];
}  __attribute__((__packed__))USB_STD_STRING_DESC;

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

#ifdef XUSBPS_UAC1

/*
 * UAC1 class specific audio interface header descriptor
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
} __attribute__((__packed__))UAC1_AC_HEADER_DESC;

/*
 * UAC1 class specific audio Input terminal descriptor
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
} __attribute__((__packed__))UAC1_INPUT_TERMINAL_DESC;

/*
 * UAC1 class specific audio Output terminal descriptor
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
} __attribute__((__packed__))UAC1_OUTPUT_TERMINAL_DESC;

/*
 * UAC1 class specific audio feature unit descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bUnitID;
	u8 bSourceID;
	u8 bControlSize;
	u16 bmaControls[AUDIO_CHANNEL_NUM + 1];
	u8 iFeature;
} __attribute__ ((packed))UAC1_FEATURE_UNIT_DESC;

/*
 * UAC1 Class-Specific AS Interface Descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalLink;
	u8 bDelay;
	u8 wFormatTagL;
	u8 wFormatTagH;
} __attribute__((__packed__))UAC1_AS_HEADER_DESC;

/*
 * UAC1 class specific audio Type I Format descriptor
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
} __attribute__((__packed__))UAC1_FORMAT_TYPE_I_DESC;

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
} __attribute__((__packed__))USB_EP_DESC;

/*
 * UAC1 Class Specific Audio Data Endpoint descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bmAttributes;
	u8 bLockDelayUnits;
	u8 wLockDelayL;
	u8 wLockDelayH;
} __attribute__((__packed__))UAC1_ISO_EP_DESC;

typedef struct {
	USB_STD_CFG_DESC 			stdCfg;
	USB_STD_IF_DESC				ifCfg;
	UAC1_AC_HEADER_DESC			adIfHdCfg;

	UAC1_INPUT_TERMINAL_DESC	ipTermCfg;
	UAC1_FEATURE_UNIT_DESC		featureUnitDesc;
	UAC1_OUTPUT_TERMINAL_DESC	opTermCfg0;

	USB_STD_IF_DESC				altIfCfg;
	USB_STD_IF_DESC				ifCfg1;

	UAC1_AS_HEADER_DESC 		audifCfg;
	UAC1_FORMAT_TYPE_I_DESC		typeICfg;
	USB_EP_DESC					audEpCfg;
	UAC1_ISO_EP_DESC 			audDataEpCfg;
} __attribute__((__packed__))USB_CONFIG;

#else	//	UAC2

/*
 * USB_DT_INTERFACE_ASSOCIATION: groups interfaces
 */
typedef struct {
	u8  bLength;
	u8  bDescriptorType;

	u8  bFirstInterface;
	u8  bInterfaceCount;
	u8  bFunctionClass;
	u8  bFunctionSubClass;
	u8  bFunctionProtocol;
	u8  iFunction;
} __attribute__ ((packed))USB_IF_ASSOC_DESC;

/*
 * UAC2 class specific audio interface header descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bcdADCL;
	u8 bcdADCH;
	u8 bCategory;
	u16 wTotalLength;
	u8 bmControls;
} __attribute__((__packed__))UAC2_AC_HEADER_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bClockId;
	u8 bmAttributes;
	u8 bmControls;
	u8 bAssocTerminal;
	u8 iClockSource;
} __attribute__((__packed__))UAC2_CLOCK_SOURCE_DESC;

/*
 * UAC2 class specific audio Input terminal descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalId;
	u8 wTerminalTypeL;
	u8 wTerminalTypeH;
	u8 bAssocTerminal;
	u8 bCSourceId;
	u8 bNrChannels;
	u8 wChannelConfigL1;
	u8 wChannelConfigL2;
	u8 wChannelConfigL3;
	u8 wChannelConfigL4;
	u8 iChannelNames;
	u16 bmControls;;
	u8 iTerminal;
} __attribute__((__packed__))UAC2_INPUT_TERMINAL_DESC;

/*
 * UAC2 class specific audio Output terminal descriptor
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
	u8 bCSourceId;
	u16 bmControls;
	u8 iTerminal;
} __attribute__((__packed__))UAC2_OUTPUT_TERMINAL_DESC;

/*
 * UAC2 class specific audio feature unit descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bUnitID;
	u8 bSourceID;
	u32 bmaControls[AUDIO_CHANNEL_NUM + 1];
	u8 iFeature;
} __attribute__((__packed__))UAC2_FEATURE_UNIT_DESC;

/*
 * UAC2 Class-Specific AS Interface Descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalLink;
	u8 bmControls;
	u8 bFormatType;
	u8 bmFormatsL1;
	u8 bmFormatsL2;
	u8 bmFormatsL3;
	u8 bmFormatsL4;
	u8 bNrChannels;
	u8 bmChannelConfigL0;
	u8 bmChannelConfigL1;
	u8 bmChannelConfigL2;
	u8 bmChannelConfigL3;
	u8 iChannelNames;
} __attribute__((__packed__))UAC2_AS_HEADER_DESC;

/*
 * Type I Format Type Descriptor (Frmts20 final.pdf)
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bFormatType;
	u8 bSubSlotSize;
	u8 bBitResolution;
} __attribute__((__packed__))UAC2_FORMAT_TYPE_I_DESC;

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
} __attribute__((__packed__))USB_EP_DESC;

/*
 * UAC2 Class Specific Audio Data Endpoint descriptor
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bmAttributes;
	u8 bmControls;
	u8 bLockDelayUnits;
	u8 wLockDelayL;
	u8 wLockDelayH;
} __attribute__((__packed__))UAC2_ISO_EP_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bClockId;
	u8 bNrPins;
	u8 baCSourceID[NUM_CLK_SRC];
	u8 bmControl;
	u8 iClockSelector;
}UAC2_CLOCK_SELECTOR_DESC;

typedef struct {
	USB_STD_CFG_DESC 		stdCfg;
	USB_IF_ASSOC_DESC		iadDesc;
	USB_STD_IF_DESC 		stdAcIfdesc;
	UAC2_AC_HEADER_DESC		acHdrDesc;

	UAC2_CLOCK_SOURCE_DESC		ClkSrcDesc;
	UAC2_CLOCK_SELECTOR_DESC	ClkSltrDesc;

	/* Out Direction */
	UAC2_INPUT_TERMINAL_DESC	usbOutItDesc;
	UAC2_FEATURE_UNIT_DESC		OutfeatureUnitDesc;
	UAC2_OUTPUT_TERMINAL_DESC	usbOutOtDesc;

	/* In Direction */
	UAC2_INPUT_TERMINAL_DESC	usbInItDesc;
	UAC2_FEATURE_UNIT_DESC		InfeatureUnitDesc;
	UAC2_OUTPUT_TERMINAL_DESC	usbInOtDesc;

	/* Output Stream */
	USB_STD_IF_DESC			stdAsOutIf0Desc;
	USB_STD_IF_DESC			stdAsOutIf1Desc;
	UAC2_AS_HEADER_DESC 		asOutHdrDesc;
	UAC2_FORMAT_TYPE_I_DESC		asOutFmt1Desc;
	USB_EP_DESC			asEpOutDesc;
	UAC2_ISO_EP_DESC		asIsoEpOutDesc;

	/* Input Stream */
	USB_STD_IF_DESC			stdAsInIf0Desc;
	USB_STD_IF_DESC			stdAsInIf1Desc;
	UAC2_AS_HEADER_DESC 		asInHdrDesc;
	UAC2_FORMAT_TYPE_I_DESC		asInFmt1Desc;
	USB_EP_DESC			asEpInDesc;
	UAC2_ISO_EP_DESC		asIsoEpInDesc;
} __attribute__((__packed__))USB_CONFIG;

#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define ISO_EP				1

#define INTERVAL_PER_SECOND		(u32)(8000)

/* Isoc interval for AUDIO */
#define AUDIO_INTERVAL			0x04

/*
 * Array of supported sampling frequencies
 */
#define MAX_AUDIO_FREQ			0x04

/*
 * Bytes per Audio transfered frame
 */
#define AUDIO_FRAME_SIZE		0x02

/*
 * Bits per Audio transfered frame
 */
#define BIT_RESOLUTION			((AUDIO_FRAME_SIZE) * (0x08))

/************************** Function Prototypes ******************************/

u32 XUsbPs_Ch9SetupDevDescReply(u8 *BufPtr, u32 BufLen);
u32 XUsbPs_Ch9SetupCfgDescReply(u8 *BufPtr, u32 BufLen);
u32 XUsbPs_Ch9SetupStrDescReply(u8 *BufPtr,	u32 BufLen, u8 Index);
void XUsbPs_SetConfiguration(XUsbPs *InstancePtr, int config);
void XUsbPs_SetConfigurationApp(XUsbPs *InstancePtr,
	XUsbPs_SetupData *SetupData);
void XUsbPs_SetInterfaceHandler(XUsbPs *InstancePtr,
	XUsbPs_SetupData *SetupData);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CH9_AUDIO_H */
