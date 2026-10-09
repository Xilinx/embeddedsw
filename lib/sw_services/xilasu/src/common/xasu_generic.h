/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_generic.h
 *
 * This file contains the generic function prototypes which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date     Changes
 * ----- ----  -------- ----------------------------------------------------------------------------
 * 1.0   yog   11/24/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_generic_apis Generic APIs
 * @{
*/

#ifndef XASU_GENERIC_H_
#define XASU_GENERIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************** Constant Definitions *************************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_IsBufferNonZero(u8 *Buffer, u32 Length);
u8 XAsu_IsRangeOverlap(u64 FirstAddr, u32 FirstLen, u64 SecondAddr, u32 SecondLen);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_GENERIC_H_ */
/** @} */
