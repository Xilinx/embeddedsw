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
 *       vm   09/15/26 Added highest exportable key vault restriction value
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
#define XASU_KM_ACCESS_RIGHTS_LOWER_BYTE_MASK	(0xFFU) /**< Mask to extract lower byte of AccessRights */
#define XASU_KM_ACCESS_RIGHTS_UPPER_BYTE_SHIFT	(8U) /**< Shift to extract upper byte of AccessRights */

/**
 * Values the Restrictions field of a key vault takes. Zero keeps the vault inside the device and
 * every value from XASU_KEYMANAGER_EXPORTABLE_VAULT to XASU_KEYMANAGER_EXPORTABLE_VAULT_MAX marks
 * it exportable, so 0x01, 0x02 and 0x03 all grant export.
 */
#define XASU_KEYMANAGER_NON_EXPORTABLE_VAULT		(0x00U)	/**< Vault stays in the device */
#define XASU_KEYMANAGER_EXPORTABLE_VAULT		(0x01U)	/**< Lowest exportable value */
#define XASU_KEYMANAGER_EXPORTABLE_VAULT_MAX		(0x03U)	/**< Highest exportable value */
/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_IS_REDUNDANT_BYTE_VALID(Val)  (((Val) & XASU_KM_ACCESS_RIGHTS_LOWER_BYTE_MASK) == \
						((Val) >> XASU_KM_ACCESS_RIGHTS_UPPER_BYTE_SHIFT))
				/**< Check if lower byte and upper byte of AccessRights are same */

#define XASU_KM_VALIDATE_FLOW_INVALID		(0U)	/**< Invalid / unset validation flow */
#define XASU_KM_VALIDATE_FLOW_GENERATE		(1U)	/**< Generate API validation flow */
#define XASU_KM_VALIDATE_FLOW_STORE		(2U)	/**< Store API validation flow */

#define XASU_KM_RSA_EXPORT_LEN(KeyLen)	((u32)sizeof(u32) + \
		((((u32)sizeof(XAsu_RsaKeyPairObject) - (u32)sizeof(u32)) * (u32)(KeyLen)) / \
		 XRSA_4096_KEY_SIZE))
				/**< Bytes an RSA key-pair export writes for KeyLen. The export is
				 * packed and every field except PubExp scales with the key length. */

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_KmValidateParams(const XAsu_KeyManagerParams *KmParamsPtr, u8 ValidationFlow,
			  u8 KeyType, u32 MinimumKeyObjectLen);
s32 XAsu_KmGetStoreKeyObjectLen(u32 CompositeKeyId, u16 KeyLen, u32 *KeyObjectLenPtr);
s32 XAsu_KmValidateKeyMetadata(const XAsu_KeyManagerKeyMetadata *MetadataPtr);
s32 XAsu_KmValidateKeyAttributes(u8 KeyType, u8 KeyAttributes);
s32 XAsu_KmValidateAesKeySplitUseCase(u8 KeyType, u8 KeyUseCase);
s32 XAsu_KmValidateKeyLength(u32 KeyLength, u8 KeyType);
s32 XAsu_KmGetMldsaKeyLengths(u8 ParamSet, u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr);
s32 XAsu_KmGetMlkemKeyLengths(u8 ParamSet, u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr);
s32 XAsu_KmValidateMldsaKeyGenParams(const XAsu_KeyManagerPqcKeyGenParams *PqcParamsPtr,
				u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr);
s32 XAsu_KmValidateMlkemKeyGenParams(const XAsu_KeyManagerPqcKeyGenParams *PqcParamsPtr,
				u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr);
s32 XAsu_KmValidateSlhdsaKeyGenParams(const XAsu_SlhdsaKeyGenParams *KeyGenParamsPtr,
				u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr);
s32 XAsu_KmValidateKeyAddrNdKeyId(u64 KeyCompAddr, u32 KeyCompLen, u64 KeyId);
s32 XAsu_KmValidateVaultCreateParams(const XAsu_KeyManagerSubVaultParams *ParamsPtr);
s32 XAsu_KmValidateXorKeyParams(const XAsu_KmXorKeyParams *ParamsPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYMANAGER_COMMON_H_ */
/** @} */