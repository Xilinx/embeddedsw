/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_aes_common.h
 * @addtogroup Overview
 * @{
 *
 * This file contains the AES function prototypes which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   10/03/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASU_AES_COMMON_H
#define XASU_AES_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *************************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_AesValidateIv(u8 EngineMode, u64 IvAddr, u32 IvLen);
s32 XAsu_AesValidateTag(u8 EngineMode, u64 TagAddr, u32 TagLen);

#ifdef __cplusplus
}
#endif

#endif  /* XAES_H */
/** @} */