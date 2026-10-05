/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_freertos_ch9_composite.h
 *
 * This file contains definitions used in composite device specific
 * chapter 9 code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   28/03/18 First release
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
#define DFU_MAX_TRANSFER		1024	/**< Maximum DFU block transfer size in bytes. */

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

/** @brief Keyboard Config. */
#define KEYBOARD_CONFIG			(1 << 0)
/** @brief Keyboard Unconfig. */
#define KEYBOARD_UNCONFIG		(1 << 1)
/** @brief Msg Config. */
#define MSG_CONFIG			(1 << 2)
/** @brief Msg Unconfig. */
#define MSG_UNCONFIG			(1 << 3)
/** @brief Record Start. */
#define RECORD_START			(1 << 4)
/** @brief Record Stop. */
#define RECORD_STOP			(1 << 5)
/** @brief Play Start. */
#define PLAY_START			(1 << 6)
/** @brief Play Stop. */
#define PLAY_STOP			(1 << 7)

/**************************** Type Definitions *******************************/
/* USB_DT_INTERFACE_ASSOCIATION: groups interfaces */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8  bLength;
	u8  bDescriptorType;

	u8  bFirstInterface;
	u8  bInterfaceCount;
	u8  bFunctionClass;
	u8  bFunctionSubClass;
	u8  bFunctionProtocol;
	u8  iFunction;
#ifdef __ICCARM__
} USB_IF_ASSOC_DESC;
#pragma pack(pop)
#else
}
__attribute__((__packed__))USB_IF_ASSOC_DESC;
#endif

/* UAC2 class specific audio interface header descriptor */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bcdADCL;
	u8 bcdADCH;
	u8 bCategory;
	u16 wTotalLength;
	u8 bmControls;
#ifdef __ICCARM__
} UAC2_AC_HEADER_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))UAC2_AC_HEADER_DESC;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bClockId;
	u8 bmAttributes;
	u8 bmControls;
	u8 bAssocTerminal;
	u8 iClockSource;
#ifdef __ICCARM__
} UAC2_CLOCK_SOURCE_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))UAC2_CLOCK_SOURCE_DESC;
#endif

/* UAC2 class specific audio Input terminal descriptor */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
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
#ifdef __ICCARM__
} UAC2_INPUT_TERMINAL_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))UAC2_INPUT_TERMINAL_DESC;
#endif

/* UAC2 class specific audio Output terminal descriptor */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
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
#ifdef __ICCARM__
} UAC2_OUTPUT_TERMINAL_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))UAC2_OUTPUT_TERMINAL_DESC;
#endif

/* UAC2 class specific audio feature unit descriptor */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bUnitID;
	u8 bSourceID;
	u32 bmaControls[AUDIO_CHANNEL_NUM + 1];
	u8 iFeature;
#ifdef __ICCARM__
} UAC2_FEATURE_UNIT_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))UAC2_FEATURE_UNIT_DESC;
#endif

/* UAC2 Class-Specific AS Interface Descriptor */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
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
#ifdef __ICCARM__
} UAC2_AS_HEADER_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))UAC2_AS_HEADER_DESC;
#endif

/* Type I Format Type Descriptor (Frmts20 final.pdf) */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bFormatType;
	u8 bSubSlotSize;
	u8 bBitResolution;
#ifdef __ICCARM__
} UAC2_FORMAT_TYPE_I_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))UAC2_FORMAT_TYPE_I_DESC;
#endif

/* USB Standard Audio Endpoint descriptor */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u8 bMaxPacketSizeL;
	u8 bMaxPacketSizeH;
	u8 bInterval;
#ifdef __ICCARM__
} USB_EP_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB_EP_DESC;
#endif

/* UAC2 Class Specific Audio Data Endpoint descriptor */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bmAttributes;
	u8 bmControls;
	u8 bLockDelayUnits;
	u8 wLockDelayL;
	u8 wLockDelayH;
#ifdef __ICCARM__
} UAC2_ISO_EP_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))UAC2_ISO_EP_DESC;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bClockId;
	u8 bNrPins;
	u8 baCSourceID[NUM_CLK_SRC];
	u8 bmControl;
	u8 iClockSelector;
#ifdef __ICCARM__
} UAC2_CLOCK_SELECTOR_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__))UAC2_CLOCK_SELECTOR_DESC;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bmAttributes;
	u16 wDetachTimeOut;
	u16 wTransferSize;
	u16 bcdDFUVersion;
#ifdef __ICCARM__
} USB_DFU_FUNC_DESC;
#pragma pack(pop)
#else
}  __attribute__((__packed__))USB_DFU_FUNC_DESC;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 bcdHID;
	u8 bCountryCode;
	u8 bNumDescriptors;
	u8 bReportDescriptorType;
	u16 wDescriptorLength;
#ifdef __ICCARM__
} USB_STD_HID_DESC;
#pragma pack(pop)
#else
} __attribute__((__packed__)) USB_STD_HID_DESC;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	USB_STD_CFG_DESC		stdCfg;

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

	USB_STD_IF_DESC			ifCfg;
	USB_STD_EP_DESC			epin;
	USB_STD_EP_DESC			epout;
	USB_STD_IF_DESC			ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC		dfu_func_desc;

	USB_STD_IF_DESC			ifStorageCfg;
	USB_STD_EP_DESC			epStoragein;
	USB_STD_EP_DESC			epStorageout;

	USB_STD_IF_DESC			ifKeyboardCfg;
	USB_STD_HID_DESC		Keyboardhiddesc;
	USB_STD_EP_DESC			Keyboardintep;
#ifdef __ICCARM__
} USB_CONFIG;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB_CONFIG;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
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
	USB_STD_EP_SS_COMP_DESC		asEpOutCompDesc;
	UAC2_ISO_EP_DESC		asIsoEpOutDesc;

	/**< Input Stream */
	USB_STD_IF_DESC			stdAsInIf0Desc;
	USB_STD_IF_DESC			stdAsInIf1Desc;
	UAC2_AS_HEADER_DESC 		asInHdrDesc;
	UAC2_FORMAT_TYPE_I_DESC		asInFmt1Desc;
	USB_EP_DESC			asEpInDesc;
	USB_STD_EP_SS_COMP_DESC		asEpInCompDesc;
	UAC2_ISO_EP_DESC		asIsoEpInDesc;

	USB_STD_IF_DESC			ifCfg;
	USB_STD_EP_DESC			epin;
	USB_STD_EP_SS_COMP_DESC		epssin;
	USB_STD_EP_DESC			epout;
	USB_STD_EP_SS_COMP_DESC		epssout;
	USB_STD_IF_DESC			ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC		dfu_func_desc;

	USB_STD_IF_DESC			ifStorageCfg;
	USB_STD_EP_DESC			epStoragein;
	USB_STD_EP_SS_COMP_DESC		epssStoragein;
	USB_STD_EP_DESC			epStorageout;
	USB_STD_EP_SS_COMP_DESC		epssStorageout;

	USB_STD_IF_DESC			ifKeyboardCfg;
	USB_STD_HID_DESC		Keyboardhiddesc;
	USB_STD_EP_DESC			Keyboardep;
	USB_STD_EP_SS_COMP_DESC		epssiKeyboardcomp;
#ifdef __ICCARM__
} USB30_CONFIG;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB30_CONFIG;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	USB_STD_CFG_DESC		stdCfg;
	USB_STD_IF_DESC			ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC		dfu_func_desc;
#ifdef __ICCARM__
} DFU_USB_CONFIG;
#pragma pack(pop)
#else
} __attribute__((__packed__))DFU_USB_CONFIG;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	USB_STD_CFG_DESC		stdCfg;
	USB_STD_IF_DESC			ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC		dfu_func_desc;
#ifdef __ICCARM__
} DFU_USB30_CONFIG;
#pragma pack(pop)
#else
} __attribute__((__packed__))DFU_USB30_CONFIG;
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
u32 Usb_GetDescReply(struct Usb_DevData *InstancePtr, SetupPacket *SetupData, u8 *BufPtr);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CH9_DFU_H */
