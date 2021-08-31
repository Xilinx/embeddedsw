/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xloader_usb.h
*
* This file contains declarations of the descriptor structures to be used
* in USB boot mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00   bsv 02/10/2019 First release
*        bsv 04/09/2020 Code clean up
* 1.01   bsv 07/08/2020 Moved Ch9Handler APIs to xloader_dfu_util.c
*        skd 07/14/2020 XLoader_UsbCopy prototype changed
*        td  08/19/2020 Fixed MISRA C violations Rule 10.3
* 1.02   bsv 08/31/2021 Code clean up
*
* </pre>
*
*****************************************************************************/
#ifndef XLOADER_USB_H
#define XLOADER_USB_H

#ifdef __cplusplus
extern "C" {
#endif
#ifdef XLOADER_USB
#include "xusbpsu.h"

/************************** TypeDef Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_REQ_REPLY_LEN		(256U)	/* Max size of reply buffer. */

/*
 * @brief	Request types
 */
#define XLOADER_REQ_TYPE_MASK		(0x60U)	/* Mask for request opcode */
#define XLOADER_CMD_STDREQ		(0x00U)	/* Standard Request */
#define XLOADER_CMD_CLASSREQ		(0x20U)	/* Class Request */

/*
 * @brief	Request Values
 */
#define XLOADER_REQ_GET_STATUS			(0x00U)
#define XLOADER_REQ_SET_FEATURE			(0x03U)
#define XLOADER_REQ_SET_ADDRESS			(0x05U)
#define XLOADER_REQ_GET_DESCRIPTOR		(0x06U)
#define XLOADER_REQ_GET_CONFIGURATION		(0x08U)
#define XLOADER_REQ_SET_CONFIGURATION		(0x09U)
#define XLOADER_REQ_SET_INTERFACE		(0x0BU)
#define XLOADER_REQ_SET_SEL			(0x30U)
#define XLOADER_ENDPOINT_HALT			(0x00U)
#define XLOADER_ENDPOINT_SELF_PWRD_STATUS	(0x0100U)
#define XLOADER_USB_ENDPOINT_NUMBER_MASK	(0xFU)
#define XLOADER_USB_ENDPOINT_DIR_MASK		(0x80U)

/*
 * @brief	Descriptor Types
 */
#define XLOADER_TYPE_DEVICE_DESC		(0x01U)
#define XLOADER_TYPE_CONFIG_DESC		(0x02U)
#define XLOADER_TYPE_STRING_DESC		(0x03U)
#define XLOADER_TYPE_DEVICE_QUALIFIER		(0x06U)
#define XLOADER_TYPE_BOS_DESC			(u8)(0x0FU)

/*
 * @brief	Status type
 */
#define XLOADER_STATUS_MASK			(0x3U)
#define XLOADER_STATUS_DEVICE			(0x0U)
#define XLOADER_STATUS_INTERFACE		(0x1U)
#define XLOADER_STATUS_ENDPOINT			(0x2U)
#define XLOADER_DFU_STATUS_SIZE			(0x6U)

/************************** Function Prototypes **************************/
void XLoader_Ch9Handler(struct Usb_DevData *InstancePtr,
	SetupPacket *SetupData);
int XLoader_UsbInit(u32 DeviceFlags);
int XLoader_UsbCopy(u64 SrcAddress, u64 DestAddress, u32 Length, u32 Flags);
int XLoader_UsbRelease(void);

#endif/*XLOADER_USB*/

#ifdef __cplusplus
}
#endif

#endif /* XLOADER_USB_H */
