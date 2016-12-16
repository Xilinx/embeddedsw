/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
/*****************************************************************************/
/**
 *
 * @file xusbpsu_ch9_dfu.h
 *
 * This file contains definitions used in the chapter 9 code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0	 vak  30/11/16 Addded DFU support
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef XUSBPSU_CH9_STORAGE_H
#define XUSBPSU_CH9_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xusbpsu.h"
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

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

u32 XUsbPsu_Ch9SetupDevDescReply(struct XUsbPsu *InstancePtr,
                                 u8 *BufPtr, u32 BufLen);
u32 XUsbPsu_Ch9SetupCfgDescReply(struct XUsbPsu *InstancePtr,
                                 u8 *BufPtr, u32 BufLen);
u32 XUsbPsu_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufLen);
u32 XUsbPsu_Ch9SetupStrDescReply(struct XUsbPsu *InstancePtr,
                                 u8 *BufPtr, u32 BufLen, u8 Index);
s32 XUsbPsu_SetConfiguration(struct XUsbPsu *InstancePtr, SetupPacket *Ctrl);
s32 XUsbPsu_SetConfigurationApp(struct XUsbPsu *InstancePtr, SetupPacket *Ctrl);

void XUsbPsu_DfuClassReq(struct XUsbPsu *InstancePtr, SetupPacket *SetupData);
void XUsbPsu_DfuDisconnect(struct XUsbPsu *InstancePtr);
void XUsbPsu_DfuReset(struct XUsbPsu *InstancePtr);
void XUsbPsu_DfuSetIntf(struct XUsbPsu *InstancePtr, SetupPacket *SetupData);

#ifdef __cplusplus
}
#endif

#endif /* XUSBPSU_CH9_STORAGE_H */
