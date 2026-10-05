/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 2.11  ka      29/01/26 Fix codespell errors.
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
#define XUSBPS_UAC1 /**< Define for UAC1; undefine to enable UAC2. */

#ifdef XUSBPS_UAC1
/* Undef this for Speaker configuration */
#define XUSBPS_MICROPHONE /**< Define for microphone; undefine for speaker. */
#endif

/**
 * @name Endpoint Type
 * @{
 */
#define USB_EP_TYPE_NONE		XUSBPS_EP_TYPE_NONE /**< Endpoint type: not in use. */
#define USB_EP_TYPE_CONTROL		XUSBPS_EP_TYPE_CONTROL /**< Endpoint type: control transfer. */
#define USB_EP_TYPE_ISOCHRONOUS	XUSBPS_EP_TYPE_ISOCHRONOUS /**< Endpoint type: isochronous transfer. */
#define USB_EP_TYPE_BULK		XUSBPS_EP_TYPE_BULK /**< Endpoint type: bulk transfer. */
#define USB_EP_TYPE_INTERRUPT	XUSBPS_EP_TYPE_INTERRUPT /**< Endpoint type: interrupt transfer. */
/** @} */

/*
 * USB types, the second of three bRequestType fields
 */
#define XUSBPS_TYPE__CLASS			(0x01 << 5) /**< USB class request type bit field. */

/*
 * Conventional codes for class-specific descriptors.  The convention is
 * defined in the USB "Common Class" Spec (3.11).  Individual class specs
 * are authoritative for their usage, not the "common class" writeup.
 */
#define XUSBPS_DT_CS_INTERFACE	(XUSBPS_TYPE__CLASS | XUSBPS_TYPE_IF_CFG_DESC) /**< Class-specific interface descriptor type. */
#define XUSBPS_DT_CS_ENDPOINT	(XUSBPS_TYPE__CLASS | XUSBPS_TYPE_ENDPOINT_CFG_DESC) /**< Class-specific endpoint descriptor type. */

/*
 * Audio Interface Subclass Codes
 */
#define USB_SUBCLASS_AUDIOCONTROL		0x01 /**< Audio Control interface subclass code. */
#define USB_SUBCLASS_AUDIOSTREAMING		0x02 /**< Audio Streaming interface subclass code. */

/*
 * Audio Class-Specific AC Interface Descriptor Subtypes
 */
#define UAC_INPUT_TERMINAL			0x02 /**< UAC Input Terminal descriptor subtype. */
#define UAC_OUTPUT_TERMINAL			0x03 /**< UAC Output Terminal descriptor subtype. */
#define UAC_FEATURE_UNIT			0x06 /**< UAC Feature Unit descriptor subtype. */

/*
 * Format Type Codes
 */
#define UAC_FORMAT_TYPE_I			0x1 /**< UAC Format Type I code. */
#define UAC_FORMAT_TYPE_SUBTYPE		0x02 /**< UAC format type subtype code. */

/*
 * Audio Class-Specific AS Interface Descriptor Subtypes
 */
#define UAC_AS_GENERAL				0x01 /**< UAC AudioStreaming General interface subtype. */

/*
 * Audio Class-Specific Endpoint Descriptor Subtypes
 */
#define UAC_EP_GENERAL				0x01 /**< UAC AudioStreaming General endpoint subtype. */

#ifdef XUSBPS_UAC1

/*
 * bInterfaceProtocol values to denote the version of the standard used
 */
#define UAC_VERSION			0x00 /**< UAC interface protocol version code. */

/*
 * Terminal/Unit ID
 */
#define USB_IT_ID		1 /**< USB input terminal unit ID. */
#define USB_OT_ID		2 /**< USB output terminal unit ID. */
#define FETR_UNT_ID		3 /**< USB feature unit ID. */

/*
 * Number of channel in AUDIO
 * Stereo->2 / Mono->1
 */
#define AUDIO_CHANNEL_NUM		0x01 /**< Number of audio channels (1=mono, 2=stereo). */

#else 	/*	UAC2 */

/*
 * bInterfaceProtocol values to denote the version of the standard used
 */
#define UAC_VERSION			0x20 /**< UAC interface protocol version code. */

/*
 * Terminal Unit
 */
#define USB_OUT_IT_ID			1 /**< USB playback input terminal ID. */
#define USB_OUT_OT_ID			2 /**< USB playback output terminal ID. */
#define USB_IN_IT_ID			3 /**< USB capture input terminal ID. */
#define USB_IN_OT_ID			4 /**< USB capture output terminal ID. */


/*
 * Clock Control Unit
 */
#define NUM_CLK_SRC				1 /**< Number of clock sources. */
#define USB_CLK_SRC_ID			5 /**< USB clock source unit ID. */
#define USB_CLK_SEL_ID			6 /**< USB clock selector unit ID. */

/*
 * Feature Unit
 */
#define OUT_FETR_UNT_ID			7 /**< Playback feature unit ID. */
#define IN_FETR_UNT_ID			8 /**< Capture feature unit ID. */

#define CONTROL_RDONLY			1 /**< Control read-only attribute. */
#define CONTROL_RDWR			3 /**< Control read/write attribute. */

#define CLK_FREQ_CTRL			0 /**< Clock frequency control selector. */
#define COPY_CTRL				0 /**< Copy protect control selector. */

/*
 * Audio Function Category Codes
 */
#define UAC2_FUNCTION_SUBCLASS_UNDEFINED	0x00 /**< UAC2 undefined function subclass. */
#define UAC2_FUNCTION_IO_BOX				0x08 /**< UAC2 I/O box function category code. */

/*
 * Audio Class-Specific Clock Source Descriptor Subtype
 */
#define UAC2_CLOCK_SOURCE			0x0a /**< UAC2 clock source descriptor subtype. */

/*
 * Audio Class-Specific Clock Selector Descriptor Subtype
 */
#define UAC2_CLOCK_SELECTOR			0x0b /**< UAC2 clock selector descriptor subtype. */

/*
 * bmAttribute fields
 */
#define UAC2_CLOCK_SOURCE_TYPE_INT_FIXED		0x1 /**< UAC2 internal fixed frequency clock type. */

/*
 * Audio Data Format Type I Codes
 */
#define UAC2_FORMAT_TYPE_I_PCM		0x01 /**< UAC2 PCM audio data format. */

/*
 * The USB 3.0 spec redefines bits 5:4 of bmAttributes as interrupt ep type.
 */
#define USB_ENDPOINT_SYNC_ASYNC		(1 << 2) /**< Isochronous endpoint asynchronous synchronization type. */

/*
 * Number of channel in AUDIO
 * Stereo->2 / Mono->1
 */
#define AUDIO_CHANNEL_NUM		0x02 /**< Number of audio channels (1=mono, 2=stereo). */

#endif

/**
 * @name Endpoint Address
 * @{
 */
#define USB_EP1_IN		0x81 /**< IN endpoint 1 address. */
#define USB_EP1_OUT		0x01 /**< OUT endpoint 1 address. */
#define USB_EP2_IN		0x82 /**< IN endpoint 2 address. */
#define USB_EP2_OUT		0x02 /**< OUT endpoint 2 address. */
/** @} */
/**************************** Type Definitions *******************************/
/*
* Standard USB structures as per 2.0 specification
 */
/** @brief Standard USB device descriptor (USB spec Table 9-8). */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u16 bcdUSB; /**< USB specification version (BCD). */
	u8 bDeviceClass; /**< Device class code. */
	u8 bDeviceSubClass; /**< Device subclass code. */
	u8 bDeviceProtocol; /**< Device protocol code. */
	u8 bMaxPacketSize0; /**< Maximum packet size for endpoint 0. */
	u16 idVendor; /**< Vendor ID. */
	u16 idProduct; /**< Product ID. */
	u16 bcdDevice; /**< Device release number (BCD). */
	u8 iManufacturer; /**< Index of manufacturer string descriptor. */
	u8 iProduct; /**< Index of product string descriptor. */
	u8 iSerialNumber; /**< Index of serial number string descriptor. */
	u8 bNumConfigurations; /**< Number of possible configurations. */
} __attribute__((__packed__))USB_STD_DEV_DESC;

/** @brief Standard USB configuration descriptor (USB spec Table 9-10). */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bType; /**< Descriptor type. */
	u16 wTotalLength; /**< Total length of configuration descriptor. */
	u8 bNumberInterfaces; /**< Number of interfaces. */
	u8 bConfigValue; /**< Value for SET_CONFIGURATION request. */
	u8 bIConfigString; /**< Index of string descriptor for this configuration. */
	u8 bAttributes; /**< Configuration attributes (bus/self-powered, remote wakeup). */
	u8 bMaxPower; /**< Maximum power consumption in 2mA units. */
}  __attribute__((__packed__))USB_STD_CFG_DESC;

/** @brief Standard USB interface descriptor (USB spec Table 9-12). */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bInterfaceNumber; /**< Interface number. */
	u8 bAlternateSetting; /**< Alternate setting number. */
	u8 bNumEndPoints; /**< Number of endpoints. */
	u8 bInterfaceClass; /**< Interface class code. */
	u8 bInterfaceSubClass; /**< Interface subclass code. */
	u8 bInterfaceProtocol; /**< Interface protocol code. */
	u8 iInterface; /**< Index of string descriptor for this interface. */
}  __attribute__((__packed__))USB_STD_IF_DESC;


/** @brief Standard USB endpoint descriptor (USB spec Table 9-13). */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bEndpointAddress; /**< Endpoint address (direction + number). */
	u8 bmAttributes; /**< Endpoint/capability attributes. */
	u8 bMaxPacketSizeL; /**< Maximum packet size low byte. */
	u8 bMaxPacketSizeH; /**< Maximum packet size high byte. */
	u8 bInterval; /**< Polling interval in frames. */
}  __attribute__((__packed__))USB_STD_EP_DESC;

/** @brief Standard USB string descriptor (USB spec Table 9-15). */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u16 wLANGID[1]; /**< Supported language IDs. */
}  __attribute__((__packed__))USB_STD_STRING_DESC;

/** @brief USB STD DEVICE CAP 7BYTE structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDevCapabiltyType; /**< Device capability type. */
	u32 bmAttributes; /**< Endpoint/capability attributes. */
} __attribute__((__packed__))USB_STD_DEVICE_CAP_7BYTE;

/** @brief USB STD DEVICE CAP 10BYTE structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDevCapabiltyType; /**< Device capability type. */
	u8 bmAttributes; /**< Endpoint/capability attributes. */
	u16 wSpeedsSupported; /**< Supported USB speeds bitmask. */
	u8 bFunctionalitySupport; /**< Lowest speed with full functionality. */
	u8 bU1DevExitLat; /**< U1 exit latency. */
	u16 wU2DevExitLat; /**< U2 exit latency. */
} __attribute__((__packed__))USB_STD_DEVICE_CAP_10BYTE;

#ifdef XUSBPS_UAC1

/*
 * UAC1 class specific audio interface header descriptor
 */
/** @brief UAC1 AC HEADER DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bcdADCL; /**< Audio Device Class specification version low byte (BCD). */
	u8 bcdADCH; /**< Audio Device Class specification version high byte (BCD). */
	u8 wTotalLengthL; /**< Total length of class-specific descriptors low byte. */
	u8 wTotalLengthH; /**< Total length of class-specific descriptors high byte. */
	u8 bInCollection; /**< Number of audio streaming interfaces. */
	u8 baInterfaceNr; /**< Interface number of the streaming interface. */
} __attribute__((__packed__))UAC1_AC_HEADER_DESC;

/*
 * UAC1 class specific audio Input terminal descriptor
 */
/** @brief UAC1 INPUT TERMINAL DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bTerminalId; /**< Unique terminal identifier. */
	u8 wTerminalTypeL; /**< Terminal type low byte. */
	u8 wTerminalTypeH; /**< Terminal type high byte. */
	u8 bAssocTerminal; /**< Associated terminal ID. */
	u8 bNrChannels; /**< Number of logical output channels. */
	u8 wChannelConfigL; /**< Channel configuration bitmap low byte. */
	u8 wChannelConfigH; /**< Channel configuration bitmap high byte. */
	u8 iChannelNames; /**< Index of channel names string descriptor. */
	u8 iTerminal; /**< Index of terminal string descriptor. */
} __attribute__((__packed__))UAC1_INPUT_TERMINAL_DESC;

/*
 * UAC1 class specific audio Output terminal descriptor
 */
/** @brief UAC1 OUTPUT TERMINAL DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bTerminalId; /**< Unique terminal identifier. */
	u8 wTerminalTypeL; /**< Terminal type low byte. */
	u8 wTerminalTypeH; /**< Terminal type high byte. */
	u8 bAssocTerminal; /**< Associated terminal ID. */
	u8 bSourceId; /**< ID of connected input terminal. */
	u8 iTerminal; /**< Index of terminal string descriptor. */
} __attribute__((__packed__))UAC1_OUTPUT_TERMINAL_DESC;

/*
 * UAC1 class specific audio feature unit descriptor
 */
/** @brief UAC1 FEATURE UNIT DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bUnitID; /**< Unique feature unit ID. */
	u8 bSourceID; /**< ID of the source audio unit. */
	u8 bControlSize; /**< Size of bmaControls field in bytes. */
	u16 bmaControls[AUDIO_CHANNEL_NUM + 1]; /**< Bitmask of audio controls per channel. */
	u8 iFeature; /**< Index of feature unit string descriptor. */
} __attribute__ ((packed))UAC1_FEATURE_UNIT_DESC;

/*
 * UAC1 Class-Specific AS Interface Descriptor
 */
/** @brief UAC1 AS HEADER DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bTerminalLink; /**< Terminal ID of the connected input terminal. */
	u8 bDelay; /**< Interface delay in frames. */
	u8 wFormatTagL; /**< Audio data format tag low byte. */
	u8 wFormatTagH; /**< Audio data format tag high byte. */
} __attribute__((__packed__))UAC1_AS_HEADER_DESC;

/*
 * UAC1 class specific audio Type I Format descriptor
 */
/** @brief UAC1 FORMAT TYPE I DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bFormatType; /**< Audio format type. */
	u8 bNrChannels; /**< Number of logical output channels. */
	u8 bSubFrameSize; /**< Number of bytes per audio subframe. */
	u8 bBitResolution; /**< Effective number of bits per sample. */
	u8 bSamFreqType; /**< Number of supported sampling frequencies. */
	u8 tSamFreByteOneL; /**< LSB of the three bytes data */
	u8 tSamFreByteTwo; /**< Sampling frequency middle byte. */
	u8 tSamFreByteThreeH;/**< MSB of the three bytes data */
} __attribute__((__packed__))UAC1_FORMAT_TYPE_I_DESC;

/*
 * USB Standard Audio Endpoint descriptor
 */
/** @brief USB EP DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bEndpointAddress; /**< Endpoint address (direction + number). */
	u8 bmAttributes; /**< Endpoint/capability attributes. */
	u8 bMaxPacketSizeL; /**< Maximum packet size low byte. */
	u8 bMaxPacketSizeH; /**< Maximum packet size high byte. */
	u8 bInterval; /**< Polling interval in frames. */
	u8 bRefresh; /**< Rate of synchronization feedback endpoint. */
	u8 bSyncAddress; /**< Synchronization endpoint address. */
} __attribute__((__packed__))USB_EP_DESC;

/*
 * UAC1 Class Specific Audio Data Endpoint descriptor
 */
/** @brief UAC1 ISO EP DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bmAttributes; /**< Endpoint/capability attributes. */
	u8 bLockDelayUnits; /**< Lock delay units (0=undefined, 1=ms, 2=PCM samples). */
	u8 wLockDelayL; /**< Lock delay low byte. */
	u8 wLockDelayH; /**< Lock delay high byte. */
} __attribute__((__packed__))UAC1_ISO_EP_DESC;

/** @brief Combined USB full configuration descriptor structure. */
typedef struct {
	USB_STD_CFG_DESC 			stdCfg; /**< Standard configuration descriptor. */
	USB_STD_IF_DESC				ifCfg; /**< Standard interface descriptor (control interface). */
	UAC1_AC_HEADER_DESC			adIfHdCfg; /**< UAC1 AC interface header descriptor. */

	UAC1_INPUT_TERMINAL_DESC	ipTermCfg; /**< UAC1 input terminal descriptor. */
	UAC1_FEATURE_UNIT_DESC		featureUnitDesc; /**< UAC1 feature unit descriptor. */
	UAC1_OUTPUT_TERMINAL_DESC	opTermCfg0; /**< UAC1 output terminal descriptor. */

	USB_STD_IF_DESC				altIfCfg; /**< Alternate setting interface descriptor. */
	USB_STD_IF_DESC				ifCfg1; /**< Standard interface descriptor (streaming interface). */

	UAC1_AS_HEADER_DESC 		audifCfg; /**< UAC1 AS header descriptor. */
	UAC1_FORMAT_TYPE_I_DESC		typeICfg; /**< UAC1 Type I format descriptor. */
	USB_EP_DESC					audEpCfg; /**< USB endpoint descriptor for audio data. */
	UAC1_ISO_EP_DESC 			audDataEpCfg; /**< UAC1 isochronous endpoint descriptor. */
} __attribute__((__packed__))USB_CONFIG;

#else	//	UAC2

/*
 * USB_DT_INTERFACE_ASSOCIATION: groups interfaces
 */
/** @brief USB IF ASSOC DESC structure. */
typedef struct {
	u8  bLength; /**< Descriptor length in bytes. */
	u8  bDescriptorType; /**< Descriptor type identifier. */

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
/** @brief UAC2 AC HEADER DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bcdADCL; /**< Audio Device Class specification version low byte (BCD). */
	u8 bcdADCH; /**< Audio Device Class specification version high byte (BCD). */
	u8 bCategory;
	u16 wTotalLength; /**< Total length of class-specific AudioControl descriptors. */
	u8 bmControls;
} __attribute__((__packed__))UAC2_AC_HEADER_DESC;

/** @brief UAC2 CLOCK SOURCE DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bClockId;
	u8 bmAttributes; /**< Clock source type and SOF synchronization attributes. */
	u8 bmControls;
	u8 bAssocTerminal; /**< Associated terminal ID. */
	u8 iClockSource;
} __attribute__((__packed__))UAC2_CLOCK_SOURCE_DESC;

/*
 * UAC2 class specific audio Input terminal descriptor
 */
/** @brief UAC2 INPUT TERMINAL DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bTerminalId; /**< Unique terminal identifier. */
	u8 wTerminalTypeL; /**< Terminal type low byte. */
	u8 wTerminalTypeH; /**< Terminal type high byte. */
	u8 bAssocTerminal; /**< Associated terminal ID. */
	u8 bCSourceId;
	u8 bNrChannels; /**< Number of logical output channels. */
	u8 wChannelConfigL1;
	u8 wChannelConfigL2;
	u8 wChannelConfigL3;
	u8 wChannelConfigL4;
	u8 iChannelNames; /**< Index of channel names string descriptor. */
	u16 bmControls;;
	u8 iTerminal; /**< Index of terminal string descriptor. */
} __attribute__((__packed__))UAC2_INPUT_TERMINAL_DESC;

/*
 * UAC2 class specific audio Output terminal descriptor
 */
/** @brief UAC2 OUTPUT TERMINAL DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bTerminalId; /**< Unique terminal identifier. */
	u8 wTerminalTypeL; /**< Terminal type low byte. */
	u8 wTerminalTypeH; /**< Terminal type high byte. */
	u8 bAssocTerminal; /**< Associated terminal ID. */
	u8 bSourceId; /**< ID of connected input terminal. */
	u8 bCSourceId;
	u16 bmControls;
	u8 iTerminal; /**< Index of terminal string descriptor. */
} __attribute__((__packed__))UAC2_OUTPUT_TERMINAL_DESC;

/*
 * UAC2 class specific audio feature unit descriptor
 */
/** @brief UAC2 FEATURE UNIT DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bUnitID; /**< Unique feature unit ID. */
	u8 bSourceID; /**< ID of the source audio unit. */
	u32 bmaControls[AUDIO_CHANNEL_NUM + 1]; /**< Bitmask of audio controls per channel. */
	u8 iFeature; /**< Index of feature unit string descriptor. */
} __attribute__((__packed__))UAC2_FEATURE_UNIT_DESC;

/*
 * UAC2 Class-Specific AS Interface Descriptor
 */
/** @brief UAC2 AS HEADER DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bTerminalLink; /**< Terminal ID of the connected input terminal. */
	u8 bmControls;
	u8 bFormatType; /**< Audio format type. */
	u8 bmFormatsL1;
	u8 bmFormatsL2;
	u8 bmFormatsL3;
	u8 bmFormatsL4;
	u8 bNrChannels; /**< Number of logical output channels. */
	u8 bmChannelConfigL0;
	u8 bmChannelConfigL1;
	u8 bmChannelConfigL2;
	u8 bmChannelConfigL3;
	u8 iChannelNames; /**< Index of channel names string descriptor. */
} __attribute__((__packed__))UAC2_AS_HEADER_DESC;

/*
 * Type I Format Type Descriptor (Frmts20 final.pdf)
 */
/** @brief UAC2 FORMAT TYPE I DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bFormatType; /**< Audio format type. */
	u8 bSubSlotSize;
	u8 bBitResolution; /**< Effective number of bits per sample. */
} __attribute__((__packed__))UAC2_FORMAT_TYPE_I_DESC;

/*
 * USB Standard Audio Endpoint descriptor
 */
/** @brief USB EP DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bEndpointAddress; /**< Endpoint address (direction + number). */
	u8 bmAttributes; /**< Endpoint/capability attributes. */
	u8 bMaxPacketSizeL; /**< Maximum packet size low byte. */
	u8 bMaxPacketSizeH; /**< Maximum packet size high byte. */
	u8 bInterval; /**< Polling interval in frames. */
} __attribute__((__packed__))USB_EP_DESC;

/*
 * UAC2 Class Specific Audio Data Endpoint descriptor
 */
/** @brief UAC2 ISO EP DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bmAttributes; /**< Endpoint/capability attributes. */
	u8 bmControls;
	u8 bLockDelayUnits; /**< Lock delay units (0=undefined, 1=ms, 2=PCM samples). */
	u8 wLockDelayL; /**< Lock delay low byte. */
	u8 wLockDelayH; /**< Lock delay high byte. */
} __attribute__((__packed__))UAC2_ISO_EP_DESC;

/** @brief UAC2 CLOCK SELECTOR DESC structure. */
typedef struct {
	u8 bLength; /**< Descriptor length in bytes. */
	u8 bDescriptorType; /**< Descriptor type identifier. */
	u8 bDescriptorSubtype; /**< Descriptor subtype for class-specific descriptor. */
	u8 bClockId;
	u8 bNrPins;
	u8 baCSourceID[NUM_CLK_SRC];
	u8 bmControl;
	u8 iClockSelector;
} UAC2_CLOCK_SELECTOR_DESC;

/** @brief Combined USB full configuration descriptor structure. */
typedef struct {
	USB_STD_CFG_DESC 		stdCfg; /**< Standard configuration descriptor. */
	USB_IF_ASSOC_DESC		iadDesc;
	USB_STD_IF_DESC 		stdAcIfdesc;
	UAC2_AC_HEADER_DESC		acHdrDesc;

	UAC2_CLOCK_SOURCE_DESC		ClkSrcDesc;
	UAC2_CLOCK_SELECTOR_DESC	ClkSltrDesc;

	/**< Out Direction */
	UAC2_INPUT_TERMINAL_DESC	usbOutItDesc;
	UAC2_FEATURE_UNIT_DESC		OutfeatureUnitDesc;
	UAC2_OUTPUT_TERMINAL_DESC	usbOutOtDesc;

	/**< In Direction */
	UAC2_INPUT_TERMINAL_DESC	usbInItDesc;
	UAC2_FEATURE_UNIT_DESC		InfeatureUnitDesc;
	UAC2_OUTPUT_TERMINAL_DESC	usbInOtDesc;

	/**< Output Stream */
	USB_STD_IF_DESC			stdAsOutIf0Desc;
	USB_STD_IF_DESC			stdAsOutIf1Desc;
	UAC2_AS_HEADER_DESC 		asOutHdrDesc;
	UAC2_FORMAT_TYPE_I_DESC		asOutFmt1Desc;
	USB_EP_DESC			asEpOutDesc;
	UAC2_ISO_EP_DESC		asIsoEpOutDesc;

	/**< Input Stream */
	USB_STD_IF_DESC			stdAsInIf0Desc;
	USB_STD_IF_DESC			stdAsInIf1Desc;
	UAC2_AS_HEADER_DESC 		asInHdrDesc;
	UAC2_FORMAT_TYPE_I_DESC		asInFmt1Desc;
	USB_EP_DESC			asEpInDesc;
	UAC2_ISO_EP_DESC		asIsoEpInDesc;
} __attribute__((__packed__))USB_CONFIG;

#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define ISO_EP				1 /**< Isochronous endpoint number. */

#define INTERVAL_PER_SECOND		(u32)(8000) /**< Number of USB service intervals per second. */

/* Isoc interval for AUDIO */
#define AUDIO_INTERVAL			0x04 /**< Isochronous endpoint polling interval. */

/*
 * Array of supported sampling frequencies
 */
#define MAX_AUDIO_FREQ			0x04 /**< Maximum number of supported audio frequencies. */

/*
 * Bytes per Audio transferred frame
 */
#define AUDIO_FRAME_SIZE		0x02 /**< Audio frame size in bytes per sample. */

/*
 * Bits per Audio transferred frame
 */
#define BIT_RESOLUTION			((AUDIO_FRAME_SIZE) * (0x08)) /**< Bit resolution per audio sample. */

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
