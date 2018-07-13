/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xfsbl_usb.h
*
* This file contains declarations of the descriptor structures to be used
* in USB boot mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   bvikram  02/01/17 First release
*
* </pre>
*
*****************************************************************************/
#ifndef XFSBL_USB_H
#define XFSBL_USB_H

#ifdef __cplusplus
extern "C" {
#endif
#ifdef XFSBL_USB
#include "xusbpsu.h"

/************************** TypeDef Definitions *****************************/
/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @name Request types
 * @{
 */
#define XFSBL_REQ_TYPE_MASK		0x60U	/**< Mask for request opcode */
#define XFSBL_CMD_STDREQ		0x00U	/**< Standard Request */
#define XFSBL_CMD_CLASSREQ		0x20U	/**< Class Request */



/**
 * @name Request Values
 * @{
 */
#define XFSBL_REQ_GET_STATUS			0x00U
#define XFSBL_REQ_SET_FEATURE			0x03U
#define XFSBL_REQ_SET_ADDRESS			0x05U
#define XFSBL_REQ_GET_DESCRIPTOR		0x06U
#define XFSBL_REQ_GET_CONFIGURATION		0x08U
#define XFSBL_REQ_SET_CONFIGURATION		0x09U
#define XFSBL_REQ_SET_INTERFACE			0x0BU
#define XFSBL_REQ_SET_SEL				0x30U
#define XFSBL_ENDPOINT_NUMBER_MASK		0x0FU
#define XFSBL_ENDPOINT_DIR_MASK			0x80U
#define XFSBL_ENDPOINT_HALT				0x00U
/* @} */


/**
 * @name Descriptor Types
 * @{
 */
#define XFSBL_TYPE_DEVICE_DESC		0x01U
#define XFSBL_TYPE_CONFIG_DESC		0x02U
#define XFSBL_TYPE_STRING_DESC		0x03U
#define XFSBL_TYPE_DEVICE_QUALIFIER	0x06U
#define XFSBL_TYPE_BOS_DESC			0x0FU
/* @} */


/**
 * @name Status type
 * @{
 */
#define XFSBL_STATUS_MASK			0x3U
#define XFSBL_STATUS_DEVICE			0x0U
#define XFSBL_STATUS_INTERFACE		0x1U
#define XFSBL_STATUS_ENDPOINT		0x2U
/* @} */

#define DFU_STATUS_SIZE				6U
#define XFSBL_USB_ENDPOINT_NUMBER_MASK	0xFU
#define XFSBL_USB_ENDPOINT_DIR_MASK		0x80U
/************************** Function Prototypes **************************/
void XFsbl_DfuInit(void);
void XFsbl_DfuSetIntf(SetupPacket *SetupData);
void XFsbl_DfuClassReq(SetupPacket *SetupData);
void XFsbl_DfuReset(struct Usb_DevData* InstancePtr);
u32 XFsbl_Ch9SetupDevDescReply(u8 *BufPtr, u32 BufferLen);
u32 XFsbl_Ch9SetupCfgDescReply(u8 *BufPtr, u32 BufferLen);
u32 XFsbl_Ch9SetupStrDescReply(u8 *BufPtr, u32 BufferLen, u8 Index);
u32 XFsbl_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufferLen);
s32 XFsbl_SetConfiguration(SetupPacket *Ctrl);
u32 XFsbl_UsbInit(u32 DeviceFlags);
u32 XFsbl_UsbCopy(u32 SrcAddress, PTRSIZE DestAddress, u32 Length);
u32 XFsbl_UsbRelease(void);
u32 XFsbl_CheckTempDfuMemory(u32 Offset);

#endif/*XFSBL_USB*/
#ifdef __cplusplus
extern "C" }
#endif

#endif /* XFSBL_USB_H */
