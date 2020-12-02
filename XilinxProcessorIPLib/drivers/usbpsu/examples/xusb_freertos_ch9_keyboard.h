/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_freertos_ch9_keyboard.h
 *
 * This file contains definitions used in the keyboard specific chapter 9 code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   22/03/18 First release
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef XUSB_CH9_KEYBOARD_H
#define XUSB_CH9_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xusb_ch9.h"

/************************** Constant Definitions *****************************/
#define USB_EP_STATE_KEY		1
#define USB_EP_STATE_NOKEY		2

#define KEYBOARD_EP			1

#define KEYBOARD_CONFIG			(1 << 0)
#define KEYBOARD_UNCONFIG		(1 << 1)

/************************** TypeDef Definitions *****************************/
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
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg;
	USB_STD_HID_DESC hiddesc;
	USB_STD_EP_DESC intep;
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
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg;
	USB_STD_HID_DESC hiddesc;
	USB_STD_EP_DESC intep;
	USB_STD_EP_SS_COMP_DESC epsscomp;
#ifdef __ICCARM__
} USB30_CONFIG;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB30_CONFIG;
#endif

/************************** Variable Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/

/* Check where these defines need to go  */
#define be2le(val)	(u32)(val)
#define be2les(x)	(u16) (x)
#define htonl(val)	((((u32)(val) & 0x000000FF)<<24) |	\
			 (((u32)(val) & 0x0000FF00)<<8)  |	\
			 (((u32)(val) & 0x00FF0000)>>8)  |	\
			 (((u32)(val) & 0xFF000000)>>24))
#define htons(x)	(u16) ((((u16)(x))<<8) | (((u16)(x))>>8))

/************************** Function Prototypes ******************************/

u32 Usb_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupDevDescReply(struct Usb_DevData *InstancePtr, u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupCfgDescReply(struct Usb_DevData *InstancePtr, u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupStrDescReply(struct Usb_DevData *InstancePtr, u8 *BufPtr, u32 BufLen, u8 Index);
s32 Usb_SetConfiguration(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl);
s32 Usb_SetConfigurationApp(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl);
u32 Usb_GetDescReply(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl, u8 *BufPtr);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CH9_keyboard_H */
