/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keymanager_common.h
 *
 * This file contains the keymanager function prototypes which are common across the
 * client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   11/30/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_keymanager_common_apis KeyManager Common APIs
 * @{
*/
#ifndef XASU_KEYMANAGER_COMMON_H_
#define XASU_KEYMANAGER_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_keymanagerinfo.h"

/************************** Constant Definitions *************************************************/
#define XASU_KM_AES_KEYTYPE     (0U)    /**< Key type identifier for AES key */
#define XASU_KM_IV_KEYTYPE      (1U)    /**< Key type identifier for IV */
#define XASU_KM_RSA_KEYTYPE     (2U)    /**< Key type identifier for RSA key */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_KmValidateVaultParams(const XAsu_KeyManagerParams *KmParamsPtr);
s32 XAsu_KmValidateKeyLength(const XAsu_KeyManagerParams *KmSubVaultParamPtr, u8 KeyType);
s32 XAsu_KmValidateKeyAddrNdKeyId(u64 KeyCompAddr, u32 KeyId);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYMANAGER_COMMON_H_ */
/** @} */