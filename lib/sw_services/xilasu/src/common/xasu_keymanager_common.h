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
#define XASU_KM_AES_KEYTYPE     	(0U)    /**< Key type identifier for AES key */
#define XASU_KM_IV_KEYTYPE      	(1U)    /**< Key type identifier for IV */
#define XASU_KM_KDF_HMAC_KEYTYPE     	(2U)    /**< Key type identifier for KDF/HMAC key */
#define XASU_KM_RSA_KEYTYPE     	(3U)    /**< Key type identifier for RSA key */


#define XASU_KM_ACCESS_RIGHTS_LOWER_BYTE_MASK	(0xFFU) /**< Mask to extract lower byte of AccessRights */
#define XASU_KM_ACCESS_RIGHTS_UPPER_BYTE_SHIFT	(8U) /**< Shift to extract upper byte of AccessRights */

#define XASU_KEYMANAGER_NON_EXPORTABLE_VAULT		(0x00U) /**< Non-exportable vault value */
#define XASU_KEYMANAGER_EXPORTABLE_VAULT		(0x01U) /**< Exportable vault value */
/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_IS_REDUNDANT_BYTE_VALID(Val)  (((Val) & XASU_KM_ACCESS_RIGHTS_LOWER_BYTE_MASK) == \
						((Val) >> XASU_KM_ACCESS_RIGHTS_UPPER_BYTE_SHIFT))
				/**< Check if lower byte and upper byte of AccessRights are same */

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_KmValidateVaultParams(const XAsu_KeyManagerParams *KmParamsPtr);
s32 XAsu_KmValidateKeyMetadata(const XAsu_KeyManagerKeyMetadata *MetadataPtr);
s32 XAsu_KmValidateKeyLength(u32 KeyLength, u8 KeyType);
s32 XAsu_KmValidateKeyAddrNdKeyId(u64 KeyCompAddr, u32 KeyId);
s32 XAsu_KmValidateVaultCreateParams(const XAsu_KeyManagerSubVaultParams *ParamsPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYMANAGER_COMMON_H_ */
/** @} */