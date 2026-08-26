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
 *       kp   08/22/26 Added vault ID validation and composite key ID handling.
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
#include "xasu_mldsainfo.h"
#include "xasu_mlkeminfo.h"

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
	if (KmParamsPtr->KeyObjectAddr == 0U) {
		Status = XST_FAILURE;
		Status = XAsu_KmValidateKeyMetadata(&KmParamsPtr->KeyMetadata);
	}

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
	u8 VaultId;

	if (MetadataPtr == NULL) {
		goto END;
	}

	VaultId = XASU_KM_GET_VAULT_ID(MetadataPtr->CompositeKeyId);
	if ((MetadataPtr->KeyUseCase == 0U) ||
	    (MetadataPtr->UsageCount == 0U) ||
	    (VaultId == 0U) ||
	    (VaultId >= XASU_KM_MAX_VAULTS)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the key attributes field against the key type.
 *
 * @param	KeyType		Key type identifier. May include the wrapped-key bit
 *				(XASU_KM_KEYTYPE_WRAPPED_BIT_MASK), which is masked out before
 *				comparing against the sub-vault identifier.
 * @param	KeyAttributes	Key attributes value from the key metadata.
 *
 * @return
 *	- XST_SUCCESS, if the key attributes are valid for the given key type.
 *	- XST_FAILURE, if the key attributes are invalid.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateKeyAttributes(u8 KeyType, u8 KeyAttributes)
{
	s32 Status = XST_FAILURE;

	/**
	 * For an RSA private key, the key attributes encode the private key format
	 * (prime/totient presence) and must not exceed the maximum supported value.
	 */
	if (((KeyType & (u8)(~XASU_KM_KEYTYPE_WRAPPED_BIT_MASK)) == (u8)XASU_RSA_PVT_SUBVAULT_ID) &&
	    (KeyAttributes > XASU_KM_RSA_PVT_ATTR_MAX)) {
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
 * @brief	This function resolves the ML-DSA public and private key lengths of a parameter
 *		set. Unlike the other algorithms, an ML-DSA key pair has no single "key length":
 *		the two halves differ in size and neither is derivable from the other, so both are
 *		returned from the parameter set carried in the key attributes.
 *
 * @param	ParamSet	ML-DSA parameter set (see XAsu_MldsaParamSet).
 * @param	PubKeyLenPtr	Pointer to the output public key length in bytes.
 * @param	PrivKeyLenPtr	Pointer to the output private key length in bytes.
 *
 * @return
 *	- XST_SUCCESS, if the parameter set is supported and both lengths are returned.
 *	- XST_FAILURE, if a pointer is NULL or the parameter set is unsupported.
 *
 *************************************************************************************************/
s32 XAsu_KmGetMldsaKeyLengths(u8 ParamSet, u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr)
{
	s32 Status = XST_FAILURE;

	if ((PubKeyLenPtr == NULL) || (PrivKeyLenPtr == NULL)) {
		goto END;
	}

	if (ParamSet == (u8)XASU_MLDSA_44) {
		*PubKeyLenPtr = XASU_MLDSA_44_PUB_KEY_LEN;
		*PrivKeyLenPtr = XASU_MLDSA_44_PRIV_KEY_LEN;
	} else if (ParamSet == (u8)XASU_MLDSA_65) {
		*PubKeyLenPtr = XASU_MLDSA_65_PUB_KEY_LEN;
		*PrivKeyLenPtr = XASU_MLDSA_65_PRIV_KEY_LEN;
	} else if (ParamSet == (u8)XASU_MLDSA_87) {
		*PubKeyLenPtr = XASU_MLDSA_87_PUB_KEY_LEN;
		*PrivKeyLenPtr = XASU_MLDSA_87_PRIV_KEY_LEN;
	} else {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function resolves the ML-KEM encapsulation (public) and decapsulation
 *		(private) key lengths of a parameter set. See XAsu_KmGetMldsaKeyLengths for why
 *		both lengths are returned rather than a single key length.
 *
 * @param	ParamSet	ML-KEM parameter set (see XAsu_MlkemParamSet).
 * @param	PubKeyLenPtr	Pointer to the output encapsulation key length in bytes.
 * @param	PrivKeyLenPtr	Pointer to the output decapsulation key length in bytes.
 *
 * @return
 *	- XST_SUCCESS, if the parameter set is supported and both lengths are returned.
 *	- XST_FAILURE, if a pointer is NULL or the parameter set is unsupported.
 *
 *************************************************************************************************/
s32 XAsu_KmGetMlkemKeyLengths(u8 ParamSet, u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr)
{
	s32 Status = XST_FAILURE;

	if ((PubKeyLenPtr == NULL) || (PrivKeyLenPtr == NULL)) {
		goto END;
	}

	if (ParamSet == (u8)XASU_MLKEM_512) {
		*PubKeyLenPtr = XASU_MLKEM_512_PUB_KEY_LEN;
		*PrivKeyLenPtr = XASU_MLKEM_512_PRIV_KEY_LEN;
	} else if (ParamSet == (u8)XASU_MLKEM_768) {
		*PubKeyLenPtr = XASU_MLKEM_768_PUB_KEY_LEN;
		*PrivKeyLenPtr = XASU_MLKEM_768_PRIV_KEY_LEN;
	} else if (ParamSet == (u8)XASU_MLKEM_1024) {
		*PubKeyLenPtr = XASU_MLKEM_1024_PUB_KEY_LEN;
		*PrivKeyLenPtr = XASU_MLKEM_1024_PRIV_KEY_LEN;
	} else {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the ML-DSA key pair generation parameters against the
 *		contract both the client and the server hold the request to, and returns the key
 *		lengths of the requested parameter set as a by-product, since every caller needs
 *		them right after.
 *
 * @param	PqcParamsPtr	Pointer to XAsu_KeyManagerPqcKeyGenParams structure that holds the
 *				key generation parameters.
 * @param	PubKeyLenPtr	Pointer to the output public key length in bytes.
 * @param	PrivKeyLenPtr	Pointer to the output private key length in bytes.
 *
 * @return
 *	- XST_SUCCESS, if input validation is successful.
 *	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateMldsaKeyGenParams(const XAsu_KeyManagerPqcKeyGenParams *PqcParamsPtr,
				u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr)
{
	volatile s32 Status = XST_FAILURE;

	if (PqcParamsPtr == NULL) {
		goto END;
	}

	/**
	 * An explicit seed must come with its exact length and a zero address with a zero length,
	 * so that a request cannot name a seed the core would then draw for itself.
	 */
	if (((PqcParamsPtr->SeedAddr == 0U) && (PqcParamsPtr->SeedLen != 0U)) ||
	    ((PqcParamsPtr->SeedAddr != 0U) &&
	     (PqcParamsPtr->SeedLen != XASU_MLDSA_KEYGEN_SEED_LEN))) {
		goto END;
	}

	/** Resolve both key lengths from the parameter set. */
	Status = XAsu_KmGetMldsaKeyLengths(PqcParamsPtr->KmParams.KeyMetadata.KeyAttributes,
					PubKeyLenPtr, PrivKeyLenPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Confirm the declared length is the public key length of that set. Both halves of the
	 * pair are sized from the parameter set, so the declared length is a cross-check rather
	 * than an input.
	 */
	Status = XST_FAILURE;
	if (PqcParamsPtr->KmParams.KeyMetadata.Length != (u16)(*PubKeyLenPtr)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the ML-KEM key pair generation parameters against the
 *		contract both the client and the server hold the request to. See
 *		XAsu_KmValidateMldsaKeyGenParams for why the key lengths are returned as well.
 *
 * @param	PqcParamsPtr	Pointer to XAsu_KeyManagerPqcKeyGenParams structure that holds the
 *				key generation parameters.
 * @param	PubKeyLenPtr	Pointer to the output encapsulation key length in bytes.
 * @param	PrivKeyLenPtr	Pointer to the output decapsulation key length in bytes.
 *
 * @return
 *	- XST_SUCCESS, if input validation is successful.
 *	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateMlkemKeyGenParams(const XAsu_KeyManagerPqcKeyGenParams *PqcParamsPtr,
				u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr)
{
	volatile s32 Status = XST_FAILURE;

	if (PqcParamsPtr == NULL) {
		goto END;
	}

	/**
	 * An explicit seed must come with its exact length and a zero address with a zero length,
	 * so that a request cannot name a seed the core would then draw for itself.
	 */
	if (((PqcParamsPtr->SeedAddr == 0U) && (PqcParamsPtr->SeedLen != 0U)) ||
	    ((PqcParamsPtr->SeedAddr != 0U) &&
	     (PqcParamsPtr->SeedLen != XASU_MLKEM_KEYGEN_SEED_LEN))) {
		goto END;
	}

	/** Resolve both key lengths from the parameter set. */
	Status = XAsu_KmGetMlkemKeyLengths(PqcParamsPtr->KmParams.KeyMetadata.KeyAttributes,
					PubKeyLenPtr, PrivKeyLenPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Confirm the declared length is the encapsulation key length of that set. */
	Status = XST_FAILURE;
	if (PqcParamsPtr->KmParams.KeyMetadata.Length != (u16)(*PubKeyLenPtr)) {
		goto END;
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
	    (ParamsPtr->X509KeyVaultCapacity == 0U) &&
	    (ParamsPtr->MldsaPvtKeyVaultCapacity == 0U) &&
	    (ParamsPtr->MldsaPubKeyVaultCapacity == 0U) &&
	    (ParamsPtr->MlkemPvtKeyVaultCapacity == 0U) &&
	    (ParamsPtr->MlkemPubKeyVaultCapacity == 0U)) {
		goto END;
	}

	if ((ParamsPtr->Restrictions != XASU_KEYMANAGER_NON_EXPORTABLE_VAULT) &&
		(ParamsPtr->Restrictions != XASU_KEYMANAGER_EXPORTABLE_VAULT)) {
		goto END;
	}

	/** Reject requested vault ID out of user range (0 = auto-assign). */
	if (ParamsPtr->VaultId >= XASU_KM_MAX_VAULTS) {
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}
/** @} */
