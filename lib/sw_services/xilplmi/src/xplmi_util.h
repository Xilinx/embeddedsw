/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
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
