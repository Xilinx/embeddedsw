/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_ch9_audio_dfu.h
 *
 * This file contains definitions used in AUDIO-DFU composite device
 * specific chapter 9 code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   22/02/18 First release
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef XUSB_CH9_DFU_H
#define XUSB_CH9_DFU_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xusb_ch9.h"

/************************** Constant Definitions *****************************/
#define DFU_MAX_TRANSFER	1024	/**< Maximum DFU block transfer size in bytes. */

/* USB types, the second of three bRequestType fields */
/** @brief USB request type: Class. */
#define USB_TYPE_CLASS			(0x01 << 5)

/*
 * Conventional codes for class-specific descriptors.  The convention is
 * defined in the USB "Common Class" Spec (3.11).  Individual class specs
 * are authoritative for their usage, not the "common class" writeup.
 */
/** @brief Class-specific interface descriptor type. */
#define USB_DT_CS_INTERFACE		(USB_TYPE_CLASS | USB_TYPE_INTERFACE_DESC)
/** @brief Class-specific endpoint descriptor type. */
#define USB_DT_CS_ENDPOINT		(USB_TYPE_CLASS | USB_TYPE_ENDPOINT_CFG_DESC)

/* Audio Interface Subclass Codes */
#define USB_SUBCLASS_AUDIOCONTROL	0x01	/**< USB Audio Control subclass code. */
#define USB_SUBCLASS_AUDIOSTREAMING	0x02	/**< USB Audio Streaming subclass code. */

/* Audio Class-Specific AC Interface Descriptor Subtypes */
#define UAC_INPUT_TERMINAL		0x02	/**< UAC Input Terminal descriptor subtype. */
#define UAC_OUTPUT_TERMINAL		0x03	/**< UAC Output Terminal descriptor subtype. */
#define UAC_FEATURE_UNIT		0x06	/**< UAC Feature Unit descriptor subtype. */

/* Format Type Codes */
#define UAC_FORMAT_TYPE_I		0x1	/**< UAC format type I subtype. */
#define UAC_FORMAT_TYPE_SUBTYPE		0x02	/**< UAC format type subtype identifier. */

/* Audio Class-Specific AS Interface Descriptor Subtypes */
#define UAC_AS_GENERAL			0x01	/**< UAC AudioStreaming General subtype. */

/* Audio Class-Specific Endpoint Descriptor Subtypes */
#define UAC_EP_GENERAL			0x01	/**< UAC endpoint general subtype. */

/* bInterfaceProtocol values to denote the version of the standard used */
#define UAC_VERSION			0x20	/**< UAC version identifier. */

/* Terminal Unit */
#define USB_OUT_IT_ID			1	/**< USB output input terminal ID. */
#define USB_OUT_OT_ID			2	/**< USB output terminal ID. */
#define USB_IN_IT_ID			3	/**< USB input input terminal ID. */
#define USB_IN_OT_ID			4	/**< USB input output terminal ID. */

/* Clock Control Unit */
#define NUM_CLK_SRC			1	/**< Number of clock sources. */
#define USB_CLK_SRC_ID			5	/**< USB clock source ID. */
#define USB_CLK_SEL_ID			6	/**< Usb Clk Sel Id. */

/* Feature Unit */
#define OUT_FETR_UNT_ID			7	/**< Out Fetr Unt Id. */
#define IN_FETR_UNT_ID			8	/**< In Fetr Unt Id. */

#define CONTROL_RDONLY			1	/**< Control Rdonly. */
#define CONTROL_RDWR			3	/**< Control Rdwr. */

#define CLK_FREQ_CTRL			0	/**< Clk Freq Ctrl. */
#define COPY_CTRL			0	/**< Copy Ctrl. */

/* Audio Function Category Codes */
#define UAC2_FUNCTION_SUBCLASS_UNDEFINED	0x00	/**< Uac2 Function Subclass Undefined. */
#define UAC2_FUNCTION_IO_BOX			0x08	/**< Uac2 Function Io Box. */

/* Audio Class-Specific Clock Source Descriptor Subtype */
#define UAC2_CLOCK_SOURCE		0x0a	/**< Uac2 Clock Source. */

/* Audio Class-Specific Clock Selector Descriptor Subtype */
#define UAC2_CLOCK_SELECTOR		0x0b	/**< Uac2 Clock Selector. */

/* bmAttribute fields */
#define UAC2_CLOCK_SOURCE_TYPE_INT_FIXED	0x1	/**< Uac2 Clock Source Type Int Fixed. */

/* Audio Data Format Type I Codes */
#define UAC2_FORMAT_TYPE_I_PCM		0x01	/**< Uac2 Format Type I Pcm. */

/* The USB 3.0 spec redefines bits 5:4 of bmAttributes as interrupt ep type */
/** @brief Usb Endpoint Sync Async. */
#define USB_ENDPOINT_SYNC_ASYNC		(1 << 2)

/*
 * Number of channel in AUDIO
 * Stereo->2 / Mono->1
 */
#define AUDIO_CHANNEL_NUM		0x02	/**< Audio Channel Num. */


/**************************** Type Definitions *******************************/

#if defined (__ICCARM__)
#pragma pack(push, 1)
#endif

/* USB_DT_INTERFACE_ASSOCIATION: groups interfaces */
typedef struct {
	u8  bLength;
	u8  bDescriptorType;

	u8  bFirstInterface;	/**< B First Interface. */
	u8  bInterfaceCount;	/**< B Interface Count. */
	u8  bFunctionClass;	/**< B Function Class. */
	u8  bFunctionSubClass;	/**< B Function Sub Class. */
	u8  bFunctionProtocol;	/**< B Function Protocol. */
	u8  iFunction;	/**< I Function. */
} attribute(USB_IF_ASSOC_DESC);

/* UAC2 class specific audio interface header descriptor */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;	/**< B Descriptor Subtype. */
	u8 bcdADCL;	/**< Bcd A D C L. */
	u8 bcdADCH;	/**< Bcd A D C H. */
	u8 bCategory;	/**< B Category. */
	u16 wTotalLength;
	u8 bmControls;	/**< Bm Controls. */
} attribute(UAC2_AC_HEADER_DESC);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bClockId;	/**< B Clock Id. */
	u8 bmAttributes;
	u8 bmControls;	/**< Bitmap of supported controls. */
	u8 bAssocTerminal;	/**< B Assoc Terminal. */
	u8 iClockSource;	/**< I Clock Source. */
} attribute(UAC2_CLOCK_SOURCE_DESC);

/* UAC2 class specific audio Input terminal descriptor */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalId;	/**< B Terminal Id. */
	u8 wTerminalTypeL;	/**< W Terminal Type L. */
	u8 wTerminalTypeH;	/**< W Terminal Type H. */
	u8 bAssocTerminal;
	u8 bCSourceId;	/**< B C Source Id. */
	u8 bNrChannels;	/**< B Nr Channels. */
	u8 wChannelConfigL1;	/**< W Channel Config L1. */
	u8 wChannelConfigL2;	/**< W Channel Config L2. */
	u8 wChannelConfigL3;	/**< W Channel Config L3. */
	u8 wChannelConfigL4;	/**< W Channel Config L4. */
	u8 iChannelNames;	/**< I Channel Names. */
	u16 bmControls;	/**< Bm Controls. */
	u8 iTerminal;	/**< I Terminal. */
} attribute(UAC2_INPUT_TERMINAL_DESC);

/* UAC2 class specific audio Output terminal descriptor */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalId;
	u8 wTerminalTypeL;
	u8 wTerminalTypeH;
	u8 bAssocTerminal;
	u8 bSourceId;	/**< B Source Id. */
	u8 bCSourceId;
	u16 bmControls;
	u8 iTerminal;
} attribute(UAC2_OUTPUT_TERMINAL_DESC);

/* UAC2 class specific audio feature unit descriptor */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bUnitID;	/**< B Unit I D. */
	u8 bSourceID;	/**< B Source I D. */
	u32 bmaControls[AUDIO_CHANNEL_NUM + 1];	/**< Bma Controls. */
	u8 iFeature;	/**< I Feature. */
} attribute(UAC2_FEATURE_UNIT_DESC);

/* UAC2 Class-Specific AS Interface Descriptor */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalLink;	/**< B Terminal Link. */
	u8 bmControls;
	u8 bFormatType;	/**< B Format Type. */
	u8 bmFormatsL1;	/**< Bm Formats L1. */
	u8 bmFormatsL2;	/**< Bm Formats L2. */
	u8 bmFormatsL3;	/**< Bm Formats L3. */
	u8 bmFormatsL4;	/**< Bm Formats L4. */
	u8 bNrChannels;
	u8 bmChannelConfigL0;	/**< Bm Channel Config L0. */
	u8 bmChannelConfigL1;	/**< Bm Channel Config L1. */
	u8 bmChannelConfigL2;	/**< Bm Channel Config L2. */
	u8 bmChannelConfigL3;	/**< Bm Channel Config L3. */
	u8 iChannelNames;
} attribute(UAC2_AS_HEADER_DESC);

/* Type I Format Type Descriptor (Frmts20 final.pdf) */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bFormatType;
	u8 bSubSlotSize;	/**< B Sub Slot Size. */
	u8 bBitResolution;	/**< B Bit Resolution. */
} attribute(UAC2_FORMAT_TYPE_I_DESC);

/* USB Standard Audio Endpoint descriptor */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u8 bMaxPacketSizeL;
	u8 bMaxPacketSizeH;
	u8 bInterval;
} attribute(USB_EP_DESC);

/* UAC2 Class Specific Audio Data Endpoint descriptor */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bmAttributes;
	u8 bmControls;
	u8 bLockDelayUnits;	/**< B Lock Delay Units. */
	u8 wLockDelayL;	/**< W Lock Delay L. */
	u8 wLockDelayH;	/**< W Lock Delay H. */
} attribute(UAC2_ISO_EP_DESC);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bClockId;
	u8 bNrPins;	/**< B Nr Pins. */
	u8 baCSourceID[NUM_CLK_SRC];	/**< Ba C Source I D. */
	u8 bmControl;	/**< Bm Control. */
	u8 iClockSelector;	/**< I Clock Selector. */
} attribute(UAC2_CLOCK_SELECTOR_DESC);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bmAttributes;
	u16 wDetachTimeOut;	/**< W Detach Time Out. */
	u16 wTransferSize;	/**< W Transfer Size. */
	u16 bcdDFUVersion;	/**< Bcd D F U Version. */
} attribute(USB_DFU_FUNC_DESC);

typedef struct {
	USB_STD_CFG_DESC		stdCfg;	/**< Std Cfg. */

	USB_IF_ASSOC_DESC		iadDesc;	/**< Iad Desc. */
	USB_STD_IF_DESC 		stdAcIfdesc;	/**< Std Ac Ifdesc. */
	UAC2_AC_HEADER_DESC		acHdrDesc;	/**< Ac Hdr Desc. */

	UAC2_CLOCK_SOURCE_DESC		ClkSrcDesc;	/**< Clk Src Desc. */
	UAC2_CLOCK_SELECTOR_DESC	ClkSltrDesc;	/**< Clk Sltr Desc. */

	/* Out Direction */
	UAC2_INPUT_TERMINAL_DESC	usbOutItDesc;	/**< Usb Out It Desc. */
	UAC2_FEATURE_UNIT_DESC		OutfeatureUnitDesc;	/**< Outfeature Unit Desc. */
	UAC2_OUTPUT_TERMINAL_DESC	usbOutOtDesc;	/**< Usb Out Ot Desc. */

	/* In Direction */
	UAC2_INPUT_TERMINAL_DESC	usbInItDesc;	/**< Usb In It Desc. */
	UAC2_FEATURE_UNIT_DESC		InfeatureUnitDesc;	/**< Infeature Unit Desc. */
	UAC2_OUTPUT_TERMINAL_DESC	usbInOtDesc;	/**< Usb In Ot Desc. */

	/* Output Stream */
	USB_STD_IF_DESC			stdAsOutIf0Desc;	/**< Std As Out If0 Desc. */
	USB_STD_IF_DESC			stdAsOutIf1Desc;	/**< Std As Out If1 Desc. */
	UAC2_AS_HEADER_DESC 		asOutHdrDesc;	/**< As Out Hdr Desc. */
	UAC2_FORMAT_TYPE_I_DESC		asOutFmt1Desc;	/**< As Out Fmt1 Desc. */
	USB_EP_DESC			asEpOutDesc;	/**< As Ep Out Desc. */
	UAC2_ISO_EP_DESC		asIsoEpOutDesc;	/**< As Iso Ep Out Desc. */

	/* Input Stream */
	USB_STD_IF_DESC			stdAsInIf0Desc;	/**< Std As In If0 Desc. */
	USB_STD_IF_DESC			stdAsInIf1Desc;	/**< Std As In If1 Desc. */
	UAC2_AS_HEADER_DESC 		asInHdrDesc;	/**< As In Hdr Desc. */
	UAC2_FORMAT_TYPE_I_DESC		asInFmt1Desc;	/**< As In Fmt1 Desc. */
	USB_EP_DESC			asEpInDesc;	/**< As Ep In Desc. */
	UAC2_ISO_EP_DESC		asIsoEpInDesc;	/**< As Iso Ep In Desc. */

	USB_STD_IF_DESC			ifCfg;	/**< If Cfg. */
	USB_STD_EP_DESC			epin;	/**< Epin. */
	USB_STD_EP_DESC			epout;	/**< Epout. */
	USB_STD_IF_DESC			ifCfg_alt_dfu;	/**< If Cfg_alt_dfu. */
	USB_DFU_FUNC_DESC		dfu_func_desc;	/**< Dfu_func_desc. */
} attribute(USB_CONFIG);

typedef struct {
	USB_STD_CFG_DESC		stdCfg;

	USB_IF_ASSOC_DESC 		iadDesc;
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
	USB_STD_EP_SS_COMP_DESC		asEpOutCompDesc;	/**< As Ep Out Comp Desc. */
	UAC2_ISO_EP_DESC		asIsoEpOutDesc;

	/**< Input Stream */
	USB_STD_IF_DESC			stdAsInIf0Desc;
	USB_STD_IF_DESC			stdAsInIf1Desc;
	UAC2_AS_HEADER_DESC 		asInHdrDesc;
	UAC2_FORMAT_TYPE_I_DESC		asInFmt1Desc;
	USB_EP_DESC			asEpInDesc;
	USB_STD_EP_SS_COMP_DESC		asEpInCompDesc;	/**< As Ep In Comp Desc. */
	UAC2_ISO_EP_DESC		asIsoEpInDesc;

	USB_STD_IF_DESC			ifCfg;
	USB_STD_EP_DESC			epin;
	USB_STD_EP_SS_COMP_DESC		epssin;	/**< Epssin. */
	USB_STD_EP_DESC			epout;
	USB_STD_EP_SS_COMP_DESC		epssout;	/**< Epssout. */
	USB_STD_IF_DESC			ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC		dfu_func_desc;
} attribute(USB30_CONFIG);

typedef struct {
	USB_STD_CFG_DESC		stdCfg;
	USB_STD_IF_DESC			ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC		dfu_func_desc;
} attribute(DFU_USB_CONFIG);

typedef struct {
	USB_STD_CFG_DESC		stdCfg;
	USB_STD_IF_DESC			ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC		dfu_func_desc;
} attribute(DFU_USB30_CONFIG);

#if defined (__ICCARM__)
#pragma pack(pop)
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/* Check where these defines need to go  */
/** @brief Be2le(val). */
#define be2le(val)	(u32)(val)
/** @brief Be2les(x). */
#define be2les(x)	(u16) (x)
/** @brief Swap byte order from host to network (32-bit). */
#define htonl(val)	((((u32)(val) & 0x000000FF)<<24) |	\
			 (((u32)(val) & 0x0000FF00)<<8)  |	\
			 (((u32)(val) & 0x00FF0000)>>8)  |	\
			 (((u32)(val) & 0xFF000000)>>24))

/** @brief Swap byte order from host to network (16-bit). */
#define htons(x)	(u16) ((((u16)(x))<<8) | (((u16)(x))>>8))

/************************** Function Prototypes ******************************/

u32 Usb_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupDevDescReply(struct Usb_DevData *InstancePtr, u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupCfgDescReply(struct Usb_DevData *InstancePtr, u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupStrDescReply(struct Usb_DevData *InstancePtr, u8 *BufPtr, u32 BufLen, u8 Index);
s32 Usb_SetConfiguration(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl);
/** @brief Handles the SET_CONFIGURATION request from the host; called by the Chapter 9 handler. */
s32 Usb_SetConfigurationApp(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CH9_DFU_H */
