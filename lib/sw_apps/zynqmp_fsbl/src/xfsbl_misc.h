/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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

/*****************************************************************************/
/**
*
* @file xfsbl_common.c
*
* This is the file which contains common code for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.0   bv   12/02/16 Made compliance to MISRAC 2012 guidelines
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_MISC_H
#define XFSBL_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xil_exception.h"

/************************** Constant Definitions *****************************/
#define XFSBL_SD_DRV_NUM_0	0U
#define XFSBL_SD_DRV_NUM_1	1U
#define BLOCK_SIZE_2MB 0x200000U
#define BLOCK_SIZE_1GB 0x40000000U
#define ADDRESS_LIMIT_4GB 0x100000000UL
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XFsbl_PrintArray (u32 DebugType, const u8 Buf[], u32 Len, const char *Str);
void *XFsbl_MemCpy(void * DestPtr, const void * SrcPtr, u32 Len);
char *XFsbl_Strcpy(char *DestPtr, const char *SrcPtr);
char * XFsbl_Strcat(char* Str1Ptr, const char* Str2Ptr);
void XFsbl_MakeSdFileName(char *XFsbl_SdEmmcFileName,
		u32 MultibootReg, u32 DrvNum);
u32 XFsbl_GetDrvNumSD(u32 DeviceFlags);
u32 XFsbl_PowerUpIsland(u32 PwrIslandMask);
u32 XFsbl_IsolationRestore(u32 IsolationMask);
void XFsbl_SetTlbAttributes(INTPTR Addr, UINTPTR attrib);
const char *XFsbl_GetSiliconIdName(void);
const char *XFsbl_GetProcEng(void);
u32 XFsbl_CheckSupportedCpu(u32 CpuId);
u32 XFsbl_AdmaCopy(void * DestPtr, void * SrcPtr, u32 Size);

#ifndef ARMA53_64
void XFsbl_RegisterHandlers(void);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_MISC_H */
