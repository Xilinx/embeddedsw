/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_util.h
 *
 * This header file contains APIs for utilities.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

#ifndef XPLM_UTIL_H
#define XPLM_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLM_ARRAY_SIZE(x)	(u32)(sizeof(x) / sizeof(x[0U]))
#define MASK_ALL	(0XFFFFFFFFU)
#define MASK32_ALL_HIGH	(0xFFFFFFFFU)
#define XPLM_TIME_OUT_DEFAULT	(0x10000000U)
#define XPLM_WORD_LEN			(4U)
#define XPLM_BYTES_TO_WORDS(bytes)	((bytes) / XPLM_WORD_LEN)
#define XPLM_WORDS_TO_BYTES(words)	((words) * XPLM_WORD_LEN)
#define XPLM_BIT(pos)			((u32)0x1U << (pos))
#define XPLM_ZERO		(0x0U)
#define XPLM_ONE		(0x1U)
#define XPLM_ALLFS		(0xFFFFFFFFU)
#define XPLM_REG_RTCA_RESET_VAL	(0xDEADBEEFU)
/************************** Function Prototypes ******************************/
void XPlm_UtilRMW(u32 RegAddr, u32 Mask, u32 Value);
int XPlm_UtilPollForMask(u32 RegAddr, u32 Mask, u32 TimeOutInUs);
int XPlm_UtilPoll(u32 RegAddr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs,
		void (*ClearHandler)(void));
void XPlm_PrintArray (u32 DebugType, const u32 BufAddr, u32 Len, const char *Str);
void XPlm_MemCpy32(u32* DestPtr, const u32* SrcPtr, u32 Len);

#ifdef __cplusplus
}
#endif

#endif /* XPLM_UTIL_H */
