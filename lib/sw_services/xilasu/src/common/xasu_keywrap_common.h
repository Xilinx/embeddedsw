/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keywrap_common.h
 * @addtogroup Overview
 * @{
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

#ifndef XASU_KEYWRAP_COMMON_H
#define XASU_KEYWRAP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_shainfo.h"
#include "xasu_keywrapinfo.h"

/************************** Constant Definitions *************************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_KeyWrapUnwrapValidateInputParams(const XAsu_KeyWrapParams *KwpunwpParamsPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYWRAP_COMMON_H */
/** @} */