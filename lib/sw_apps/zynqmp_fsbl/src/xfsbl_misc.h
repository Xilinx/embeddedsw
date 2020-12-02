/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
s32 XFsbl_Ceil(float Num);
s32 XFsbl_Round(float Num);
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
s32 XFsbl_PollTimeout(u32 Addr,u32 Value, u32 cond, u32 TimeOutInUs);

#ifndef ARMA53_64
void XFsbl_RegisterHandlers(void);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_MISC_H */
