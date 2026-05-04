/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keymanager_common.c
 *
 * This file contains the Keymanager function definitions which are common across
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

/***************************** Include Files *****************************************************/
#include "xasu_keymanager_common.h"
#include "xasu_aesinfo.h"
#include "xasu_hmacinfo.h"

/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/

/**************************** Type Definitions ***************************************************/

/************************** Variable Definitions *************************************************/

/************************** Inline Function Definitions ******************************************/

/************************** Function Prototypes **************************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates input parameters for keymanager.
 *
 * @param	KmParamsPtr	Pointer to XAsu_KeyManagerParams structure that holds the input
 *				parameters for Key manager
 *
 * @return
 *	- XST_SUCCESS, if input validation is successful.
 *	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateVaultParams(const XAsu_KeyManagerParams *KmParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	if (KmParamsPtr == NULL) {
		goto END;
	}

	/** Validate at least one output destination is provided. */
	Status = XAsu_KmValidateKeyAddrNdKeyId(KmParamsPtr->KeyObjectAddr,
				(u32)(KmParamsPtr->KeyIdAddr));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate key metadata. */
	if ((KmParamsPtr->KeyObjectAddr == 0U) &&
	    (XAsu_KmValidateKeyMetadata(&KmParamsPtr->KeyMetadata) != XST_SUCCESS)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates key metadata fields.
 *
 * @param	MetadataPtr	Pointer to XAsu_KeyManagerKeyMetadata structure.
 *
 * @return
 *	- XST_SUCCESS, if metadata validation is successful.
 *	- XST_FAILURE, if metadata validation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateKeyMetadata(const XAsu_KeyManagerKeyMetadata *MetadataPtr)
{
	s32 Status = XST_FAILURE;

	if (MetadataPtr == NULL) {
		goto END;
	}

	if ((MetadataPtr->KeyUseCase == 0U) ||
	    (MetadataPtr->UsageCount == 0U) ||
	    (MetadataPtr->VaultId == 0U) ||
	    (MetadataPtr->VaultId >= XASU_KM_MAX_VAULTS)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates key length parameters for a given key.
 *
 * @param	KeyLength	Length of the key material in bytes.
 * @param	KeyType		Type of key to be validated (see XASU_KM_*_KEYTYPE).
 *
 * @return
 *	- XST_SUCCESS, if input validation is successful.
 *	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateKeyLength(u32 KeyLength, u8 KeyType)
{
	volatile s32 Status = XST_FAILURE;

	/** Validate key length is either 128-bit or 256-bit for AES. */
	if (KeyType == XASU_KM_AES_KEYTYPE) {
		if ((KeyLength != XASU_AES_KEY_SIZE_128BIT_IN_BYTES) &&
		    (KeyLength != XASU_AES_KEY_SIZE_256BIT_IN_BYTES)) {
			goto END;
		}
	}

	/** Validate IV length is non-zero and does not exceed maximum size. */
	if (KeyType == XASU_KM_IV_KEYTYPE) {
		if ((KeyLength == 0U) ||
		    (KeyLength > XASU_AES_IV_SIZE_128BIT_IN_BYTES)) {
			goto END;
		}
	}

	/** Validate key length is non-zero and does not exceed maximum size for KDF/HMAC. */
	if (KeyType == XASU_KM_KDF_HMAC_KEYTYPE) {
		if ((KeyLength == 0U) ||
		    (KeyLength > XASU_KM_KDF_HMAC_MAX_KEY_LENGTH)) {
			goto END;
		}
	}

	/** Validate key length based on compile-time RSA key generation configuration. */
	if (KeyType == XASU_KM_RSA_KEYTYPE) {
#if defined(XASU_RSA_3072_KEYGEN_ENABLE)
		if (KeyLength != XRSA_3072_KEY_SIZE) {
			goto END;
		}
#elif defined(XASU_RSA_2048_KEYGEN_ENABLE)
		if (KeyLength != XRSA_2048_KEY_SIZE) {
			goto END;
		}
#else
		if (KeyLength != XRSA_4096_KEY_SIZE) {
			goto END;
		}
#endif
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates that exactly one of KeyCompAddr or KeyId is provided.
 *
 * @param	KeyCompAddr	Address of the key component structure.
 * @param	KeyId		Composite key identifier for vault-based key retrieval.
 *
 * @return
 * 	- XST_SUCCESS, if exactly one of (KeyCompAddr, KeyId) is non-zero.
 * 	- XST_FAILURE, if both are zero (no source) or both are non-zero (ambiguous source).
 *
 *************************************************************************************************/
s32 XAsu_KmValidateKeyAddrNdKeyId(u64 KeyCompAddr, u32 KeyId)
{
	s32 Status = XST_FAILURE;

	/** Reject if both are zero (no source) or both are non-zero (ambiguous source). */
	if (((KeyCompAddr == 0U) && (KeyId == 0U)) ||
	    ((KeyCompAddr != 0U) && (KeyId != 0U))) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates input parameters for key vault creation.
 *
 * @param	ParamsPtr	Pointer to XAsu_KeyManagerSubVaultParams structure that holds the
 * 				input parameters for Key manager.
 *
 * @return
 * 	- XST_SUCCESS, if input validation is successful.
 * 	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateVaultCreateParams(const XAsu_KeyManagerSubVaultParams *ParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	if (ParamsPtr == NULL) {
		goto END;
	}

	/** Validate at least one sub-vault has non-zero capacity. */
	if ((ParamsPtr->AESKeyVaultCapacity == 0U) &&
	    (ParamsPtr->IVVaultCapacity == 0U) &&
	    (ParamsPtr->KDFHmacKeyVaultCapacity == 0U) &&
	    (ParamsPtr->RSAPvtKeyVaultCapacity == 0U) &&
	    (ParamsPtr->RSAPubKeyVaultCapacity == 0U) &&
	    (ParamsPtr->ECCPvtKeyVaultCapacity == 0U) &&
	    (ParamsPtr->ECCPubKeyVaultCapacity == 0U) &&
	    (ParamsPtr->LMSKeyVaultCapacity == 0U) &&
	    (ParamsPtr->X509KeyVaultCapacity == 0U)) {
		goto END;
	}

	if ((ParamsPtr->Restrictions != XASU_KEYMANAGER_NON_EXPORTABLE_VAULT) &&
		(ParamsPtr->Restrictions != XASU_KEYMANAGER_EXPORTABLE_VAULT)) {
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}
/** @} */
