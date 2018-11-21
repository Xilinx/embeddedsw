/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_util.h
*
* This is the header file PMC FW utilities code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_UTIL_H_
#define XPLMI_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_ARRAY_SIZE(x)	(u32)(sizeof(x) / sizeof(x[0U]))
#define MASK_ALL	(0XFFFFFFFFU)
#define ENABLE_ALL	(0XFFFFFFFFU)
#define ALL_HIGH	(0XFFFFFFFFU)
#define FLAG_ALL	(0XFFFFFFFFU)
#define MASK32_ALL_HIGH	(0xFFFFFFFFU)
#define MASK32_ALL_LOW	(0x0U)
#define XPLMI_ACCESS_ALLOWED	(0x01U)
#define XPLMI_ACCESS_DENIED		(0x00U)
#define XPLMI_TIME_OUT_DEFAULT	(0x10000000U)
#define XPLMI_WORD_LEN			(4U)

/************************** Function Prototypes ******************************/

void XPlmi_UtilRMW(u32 RegAddr, u32 Mask, u32 Value);
int XPlmi_UtilSafetyWrite(u32 RegAddr, u32 Mask, u32 Value);
int XPlmi_UtilPollForMask(u32 RegAddr, u32 Mask, u32 TimeOutCount);
int XPlmi_UtilPoll(u32 RegAddr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs);
int XPlmi_UtilPoll64(u64 Addr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs);
void XPlmi_UtilWrite64(u32 HighAddr, u32 LowAddr, u32 Value);
int XPlmi_UtilPollForMask64(u32 HighAddr, u32 LowAddr, u32 Mask,
	u32 TimeOutInUs);
void XPlmi_UtilRMW64(u32 HighAddr, u32 LowAddr, u32 Mask, u32 Value);
int XPlmi_UtilSafetyRMW64(u32 HighAddr, u32 LowAddr, u32 Mask, u32 Value);
void XPlmi_PrintArray (u32 DebugType, const u64 BufAddr, u32 Len, const char *Str);
char *XPlmi_Strcpy(char *DestPtr, const char *SrcPtr);
char * XPlmi_Strcat(char* Str1Ptr, const char* Str2Ptr);
void* XPlmi_MemCpy(void * DestPtr, const void * SrcPtr, u32 Len);
int XPlmi_MemCmp(const void * Buf1Ptr, const void * Buf2Ptr, u32 Len);

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_UTIL_H_ */
