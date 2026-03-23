/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keywrap_common.h
 *
 * This file contains the key wrap unwrap function prototypes which are common across the
 * client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   02/24/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_keywrap_common_apis KeyWrap Common APIs
 * @{
*/
#ifndef XASU_KEYWRAP_COMMON_H_
#define XASU_KEYWRAP_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_shainfo.h"
#include "xasu_keywrapinfo.h"

/************************** Constant Definitions *************************************************/
#define XASU_KEYWRAP_AES_RSA_KWPUNWP (1U) /**< Operation type for AES/RSA key wrap/unwrap operation */
#define XASU_KEYWRAP_AES_KWPUNWP (2U) /**< Operation type for AES key wrap/unwrap operation */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_KeyWrapUnwrapValidateInputParams(const XAsu_KeyWrapParams *KwpunwpParamsPtr, u8 OperationType);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYWRAP_COMMON_H_ */
/** @} */