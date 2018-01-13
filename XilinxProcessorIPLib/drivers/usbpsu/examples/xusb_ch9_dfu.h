/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_ch9_dfu.h
 *
 * This file contains definitions used in DFU specific chapter 9 code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0	 vak  30/11/16 Added DFU support
 * 1.4	 BK   12/01/18 Renamed the file to be in sync with usb common code
 *		       changes for all USB IPs
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
#define DFU_MAX_TRANSFER	1024

/**************************** Type Definitions *******************************/
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
} attribute(USB_DFU_FUNC_DESC);

typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg;
	USB_STD_EP_DESC epin;
	USB_STD_EP_DESC epout;
	USB_STD_IF_DESC ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC dfu_func_desc;
} attribute(USB_CONFIG);

typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg;
	USB_STD_EP_DESC epin;
	USB_STD_EP_SS_COMP_DESC epssin;
	USB_STD_EP_DESC epout;
	USB_STD_EP_SS_COMP_DESC epssout;
	USB_STD_IF_DESC ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC dfu_func_desc;
} attribute(USB30_CONFIG);

typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC dfu_func_desc;
} attribute(DFU_USB_CONFIG);

typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC dfu_func_desc;
} attribute(DFU_USB30_CONFIG);

#ifdef __ICCARM__
#pragma pack(pop)
#endif

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
u32 Usb_Ch9SetupDevDescReply(struct Usb_DevData *InstancePtr,
                                 u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupCfgDescReply(struct Usb_DevData *InstancePtr,
                                 u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupStrDescReply(struct Usb_DevData *InstancePtr,
                                 u8 *BufPtr, u32 BufLen, u8 Index);
s32 Usb_SetConfiguration(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl);
s32 Usb_SetConfigurationApp(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CH9_DFU_H */
