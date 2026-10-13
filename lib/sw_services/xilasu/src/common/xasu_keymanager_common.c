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
 *       vm   09/15/26 Accepted the full exportable range in key vault restrictions
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
#include "xasu_eccinfo.h"
#include "xasu_ecies_common.h"
#include "xasu_hmacinfo.h"
#include "xasu_mldsainfo.h"
#include "xasu_mlkeminfo.h"
#include "xasu_slhdsainfo.h"

/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/

/**************************** Type Definitions ***************************************************/

/************************** Variable Definitions *************************************************/

/************************** Inline Function Definitions ******************************************/

/************************** Function Prototypes **************************************************/
static s32 XAsu_KmGetSlhdsaKeyLengths(u8 ParamSet, u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr);
static s32 XAsu_KmValidateVaultParams(const XAsu_KeyManagerParams *KmParamsPtr);

/*************************************************************************************************/
/**
 * @brief	Validate the output destination of a key generate request, and the key metadata
 *		when the generated key is stored in the vault.
 *
 * @param	KmParamsPtr	Pointer to XAsu_KeyManagerParams structure.
 *
 * @return
 *	- XST_SUCCESS, if input validation is successful.
 *	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
static s32 XAsu_KmValidateVaultParams(const XAsu_KeyManagerParams *KmParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	/** Require exactly one destination; the export buffer length is checked by the caller. */
	Status = XAsu_KmValidateKeyAddrNdKeyId(KmParamsPtr->KeyObjectAddr, 0U,
					       KmParamsPtr->KeyIdAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	if (KmParamsPtr->KeyObjectAddr == 0U) {
		/** Validate metadata when storing generated key into vault. */
		Status = XAsu_KmValidateKeyMetadata(&KmParamsPtr->KeyMetadata);
		if (Status != XST_SUCCESS) {
			goto END;
		}
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
 * @brief	This function validates the key type and its key attributes field. It rejects a
 *		key type that has no storable sub-vault (invalid or unknown values) and then
 *		validates the key attributes against the key type.
 *
 * @param	KeyType		Key type identifier. May include the wrapped-key bit
 *				(XASU_KM_KEYTYPE_WRAPPED_BIT_MASK), which is masked out before
 *				comparing against the sub-vault identifier.
 * @param	KeyAttributes	Key attributes value from the key metadata.
 *
 * @return
 *	- XST_SUCCESS, if the key attributes are valid for the given key type.
 *	- XST_FAILURE, if the key attributes are invalid or the key type has no storable sub-vault.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateKeyAttributes(u8 KeyType, u8 KeyAttributes)
{
	s32 Status = XST_FAILURE;
	u8 SubVaultId = (u8)(KeyType & (u8)(~XASU_KM_KEYTYPE_WRAPPED_BIT_MASK));

	/**
	 * Reject a key type that does not map to a storable sub-vault. The sub-vault IDs run
	 * without gaps from XASU_AES_SUBVAULT_ID up to XASU_INVALID_SUBVAULT_ID, so a single range
	 * check covers every storable type. Invalid sub-vaults are excluded by design;
	 * no caller of this helper stores or generates them.
	 */
	if (SubVaultId >= (u8)XASU_INVALID_SUBVAULT_ID) {
		goto END;
	}

	/**
	 * For an RSA private key, the key attributes encode the private key format
	 * (prime/totient presence) and must not exceed the maximum supported value.
	 */
	if ((SubVaultId == (u8)XASU_RSA_PVT_SUBVAULT_ID) &&
	    (KeyAttributes > XASU_KM_RSA_PVT_ATTR_MAX)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates that an AES key does not combine the key split use case
 *		with any other use case. The key type is taken as a parameter rather than read from
 *		the key metadata, because the Generate Key APIs pass their own key type. For any
 *		other key type the function returns success, since the key split use case bit is
 *		reserved there.
 *
 * @param	KeyType		Key type identifier. May include the wrapped-key bit
 *				(XASU_KM_KEYTYPE_WRAPPED_BIT_MASK), which is masked out before
 *				comparing against the sub-vault identifier.
 * @param	KeyUseCase	Key use case value from the key metadata.
 *
 * @return
 *	- XST_SUCCESS, if the key use case is valid for the given key type.
 *	- XST_FAILURE, if the key use case is invalid.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateAesKeySplitUseCase(u8 KeyType, u8 KeyUseCase)
{
	s32 Status = XST_FAILURE;

	/**
	 * For an AES key the key split use case is exclusive. A key which also carries another AES
	 * use case is usable as a key in its own right, which defeats the purpose of splitting it.
	 */
	if (((u8)(KeyType & ~XASU_KM_KEYTYPE_WRAPPED_BIT_MASK) == (u8)XASU_AES_SUBVAULT_ID) &&
	    ((KeyUseCase & XASU_KEYMANAGER_AES_KEY_SPLIT_USE_CASE) != 0U) &&
	    (KeyUseCase != XASU_KEYMANAGER_AES_KEY_SPLIT_USE_CASE)) {
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
 * @param	KeyType		Type of key to be validated.
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
	if (KeyType == (u8)XASU_AES_SUBVAULT_ID) {
		if ((KeyLength != XASU_AES_KEY_SIZE_128BIT_IN_BYTES) &&
		    (KeyLength != XASU_AES_KEY_SIZE_256BIT_IN_BYTES)) {
			goto END;
		}
	}

	/** Validate IV length is non-zero and does not exceed maximum size. */
	if (KeyType == (u8)XASU_IV_SUBVAULT_ID) {
		if ((KeyLength == 0U) ||
		    (KeyLength > XASU_AES_IV_SIZE_128BIT_IN_BYTES)) {
			goto END;
		}
	}

	/** Validate key length is non-zero and does not exceed maximum size for KDF/HMAC. */
	if (KeyType == (u8)XASU_KDF_HMAC_SUBVAULT_ID) {
		if ((KeyLength == 0U) ||
		    (KeyLength > XASU_KM_KDF_HMAC_MAX_KEY_LENGTH)) {
			goto END;
		}
	}

	/** Validate key length based on compile-time RSA key generation configuration. */
	if (KeyType == (u8)XASU_RSA_PVT_SUBVAULT_ID) {
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
 * @brief	This function resolves the SLH-DSA public and private key lengths of a parameter
 *		set. Both follow the FIPS 205 security parameter n of the set, the public key
 *		being 2n bytes and the secret key 4n, so no single "key length" describes the pair
 *		and each length is returned separately. The two sets sharing a security strength
 *		differ only in their hash family, so they resolve to the same pair of lengths.
 *
 * @param	ParamSet	SLH-DSA parameter set (see xasu_slhdsainfo.h).
 * @param	PubKeyLenPtr	Pointer to the output public key length in bytes.
 * @param	PrivKeyLenPtr	Pointer to the output private key length in bytes.
 *
 * @return
 *	- XST_SUCCESS, if the parameter set is supported and both lengths are returned.
 *	- XST_FAILURE, if a pointer is NULL or the parameter set is unsupported.
 *
 *************************************************************************************************/
static s32 XAsu_KmGetSlhdsaKeyLengths(u8 ParamSet, u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr)
{
	s32 Status = XST_FAILURE;

	if ((PubKeyLenPtr == NULL) || (PrivKeyLenPtr == NULL)) {
		goto END;
	}

	if ((ParamSet == (u8)XASU_SLHDSA_PARAM_SHAKE_256S) ||
	    (ParamSet == (u8)XASU_SLHDSA_PARAM_SHA2_256S)) {
		*PubKeyLenPtr = XASU_SLHDSA_256S_PK_SIZE;
		*PrivKeyLenPtr = XASU_SLHDSA_256S_SK_SIZE;
	} else if ((ParamSet == (u8)XASU_SLHDSA_PARAM_SHAKE_128S) ||
		   (ParamSet == (u8)XASU_SLHDSA_PARAM_SHA2_128S)) {
		*PubKeyLenPtr = XASU_SLHDSA_128S_PK_SIZE;
		*PrivKeyLenPtr = XASU_SLHDSA_128S_SK_SIZE;
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
 * @brief	This function validates the SLH-DSA key pair generation parameters against the
 *		contract both the client and the server hold the request to, and returns the key
 *		lengths of the requested parameter set as a by-product, since every caller needs
 *		them right after.
 *
 * @param	KeyGenParamsPtr	Pointer to XAsu_SlhdsaKeyGenParams structure that holds the key
 *				generation parameters.
 * @param	PubKeyLenPtr	Pointer to the output public key length in bytes.
 * @param	PrivKeyLenPtr	Pointer to the output secret key length in bytes.
 *
 * @return
 *	- XST_SUCCESS, if input validation is successful.
 *	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateSlhdsaKeyGenParams(const XAsu_SlhdsaKeyGenParams *KeyGenParamsPtr,
				u32 *PubKeyLenPtr, u32 *PrivKeyLenPtr)
{
	volatile s32 Status = XST_FAILURE;

	if (KeyGenParamsPtr == NULL) {
		goto END;
	}

	/**
	 * Both output addresses are mandatory. A zero address is treated as invalid rather than
	 * as a request to skip an output, because key generation always produces both keys.
	 */
	if ((KeyGenParamsPtr->SkOutAddr == 0U) || (KeyGenParamsPtr->PkOutAddr == 0U)) {
		goto END;
	}

	/**
	 * Resolve both key lengths from the parameter set, which also confirms the parameter set
	 * is one of the supported sets.
	 */
	Status = XAsu_KmGetSlhdsaKeyLengths(KeyGenParamsPtr->ParamSet, PubKeyLenPtr,
					PrivKeyLenPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 * Both declared output lengths must equal the key sizes of the requested parameter set.
	 * A client declaring any other length has not sized its buffers to the keys ASUFW is
	 * about to write.
	 */
	if ((KeyGenParamsPtr->SkOutLen != *PrivKeyLenPtr) ||
	    (KeyGenParamsPtr->PkOutLen != *PubKeyLenPtr)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates that exactly one of KeyCompAddr or KeyId is provided, and
 * 		that a vault sourced component carries no length. It applies to anything the key
 * 		vault holds, a key or an IV. KeyId is u64 so the same helper can test either a
 * 		composite vault KeyId or a full 64-bit KeyIdAddr without truncating high addresses.
 *
 * @param	KeyCompAddr	Address of the key or IV component, 0 when it is taken from the
 * 				key vault.
 * @param	KeyCompLen	Length of the key or IV component buffer in bytes. A component
 * 				whose length the caller does not supply passes 0.
 * @param	KeyId		Composite vault key/IV identifier, or a destination KeyIdAddr
 * 				when used by generate-parameter validation.
 *
 * @return
 * 	- XST_SUCCESS, if exactly one of (KeyCompAddr, KeyId) is non-zero and a vault sourced
 * 	  component carries a zero length.
 * 	- XST_FAILURE, if both are zero (no source) or both are non-zero (ambiguous source), or if
 * 	  a vault sourced component carries a non-zero length.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateKeyAddrNdKeyId(u64 KeyCompAddr, u32 KeyCompLen, u64 KeyId)
{
	s32 Status = XST_FAILURE;

	/** Reject if both are zero (no source) or both are non-zero (ambiguous source). */
	if (((KeyCompAddr == 0U) && (KeyId == 0U)) ||
	    ((KeyCompAddr != 0U) && (KeyId != 0U))) {
		goto END;
	}

	/**
	 * A component held in the key vault is not passed by the caller and the firmware takes
	 * its length from the stored metadata, so the request must leave the length at zero.
	 * Reject a left over length here, where the caller learns it supplied an invalid
	 * parameter, rather than letting the server range check a buffer at address zero.
	 */
	if ((KeyCompAddr == 0U) && (KeyCompLen != 0U)) {
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

	/** Reject the restriction values that are reserved. */
	if (ParamsPtr->Restrictions > XASU_KEYMANAGER_EXPORTABLE_VAULT_MAX) {
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

/*************************************************************************************************/
/**
 * @brief	This function validates the input parameters for combining two AES key splits using
 *		XOR. The key type of the input key splits and the length of the combined key are
 *		validated by the server against the stored key splits.
 *
 * @param	ParamsPtr	Pointer to the XAsu_KmXorKeyParams structure.
 *
 * @return
 *	- XST_SUCCESS, if the input parameters are valid.
 *	- XST_FAILURE, if any input parameter is invalid.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateXorKeyParams(const XAsu_KmXorKeyParams *ParamsPtr)
{
	s32 Status = XST_FAILURE;

	/** Validate input parameters. */
	if (ParamsPtr == NULL) {
		goto END;
	}

	/** Validate that both input key IDs are provided. */
	if ((ParamsPtr->SplitKeyId1 == 0U) || (ParamsPtr->SplitKeyId2 == 0U)) {
		goto END;
	}

	/**
	 * The address for the output key ID is required when the destination slot is auto-assigned,
	 * and shall not be supplied when the caller selects the slot, since the composite key ID is
	 * then already known to the caller.
	 */
	if (XASU_KM_GET_KEY_INDEX(ParamsPtr->OutKeyMetadata.CompositeKeyId) == 0U) {
		if (ParamsPtr->OutKeyIdAddr == 0U) {
			goto END;
		}
	} else {
		if (ParamsPtr->OutKeyIdAddr != 0U) {
			goto END;
		}
	}

	/**
	 * Reject identical input key IDs. A key XORed with itself yields an all zero key, so
	 * allowing this would let the caller install a known key of its choosing.
	 */
	if (ParamsPtr->SplitKeyId1 == ParamsPtr->SplitKeyId2) {
		goto END;
	}

	/** The combined key must not overwrite either input key split. */
	if ((ParamsPtr->OutKeyMetadata.CompositeKeyId == ParamsPtr->SplitKeyId1) ||
	    (ParamsPtr->OutKeyMetadata.CompositeKeyId == ParamsPtr->SplitKeyId2)) {
		goto END;
	}

	/**
	 * Key splitting is defined for AES keys only. The key slot may be zero, which requests an
	 * auto-assigned slot.
	 */
	if (XASU_KM_GET_KEY_TYPE(ParamsPtr->OutKeyMetadata.CompositeKeyId) !=
			(u8)XASU_AES_SUBVAULT_ID) {
		goto END;
	}

	/**
	 * Validate the metadata of the combined key. This rejects a zero key use case, which is
	 * required here because the use case of the input key splits cannot be inherited.
	 */
	Status = XAsu_KmValidateKeyMetadata(&ParamsPtr->OutKeyMetadata);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate the key attributes of the combined key against its key type. */
	Status = XAsu_KmValidateKeyAttributes(
			XASU_KM_GET_KEY_TYPE(ParamsPtr->OutKeyMetadata.CompositeKeyId),
			ParamsPtr->OutKeyMetadata.KeyAttributes);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate the key use case of the combined key against its key type. */
	Status = XAsu_KmValidateAesKeySplitUseCase(
			XASU_KM_GET_KEY_TYPE(ParamsPtr->OutKeyMetadata.CompositeKeyId),
			ParamsPtr->OutKeyMetadata.KeyUseCase);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate the length of the combined key. */
	Status = XAsu_KmValidateKeyLength((u32)ParamsPtr->OutKeyMetadata.Length,
					  (u8)XASU_AES_SUBVAULT_ID);
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Validate key manager Generate and Store requests.
 *
 * @param	KmParamsPtr	Pointer to XAsu_KeyManagerParams structure that holds the input
 *				parameters for Key manager APIs.
 * @param	ValidationFlow	Validation flow selector (XASU_KM_VALIDATE_FLOW_INVALID is
 *				rejected):
 *				- XASU_KM_VALIDATE_FLOW_GENERATE: exactly one of KeyObjectAddr or
 *				  KeyIdAddr is set; when exporting to client memory, KeyObjectLen must
 *				  cover the larger of KeyMetadata.Length and MinimumKeyObjectLen;
 *				  when storing in the vault, KeyMetadata is validated and the key
 *				  type in CompositeKeyId must equal KeyType.
 *				- XASU_KM_VALIDATE_FLOW_STORE: KeyObjectAddr, KeyIdAddr, and
 *				  KeyObjectLen are mandatory; KeyMetadata, key attributes, key type, and
 *				  plain/wrapped length rules are checked.
 *				Both flows check that the key type is a valid sub-vault ID and apply
 *				the AES key split use-case rule.
 * @param	KeyType		Generate flow key type: the sub-vault ID for AES, IV and
 *				KDF/HMAC keys, or the private sub-vault ID for RSA, ECC, ML-DSA
 *				and ML-KEM key pairs. It must be a valid sub-vault ID without the
 *				wrapped-key bit. It selects the key length rule for AES, IV,
 *				KDF/HMAC and RSA, the RSA attribute rule, and the ECC curve check.
 *				ML-DSA and ML-KEM parameters are checked by their
 *				algorithm-specific validators. Ignored for the Store flow, where
 *				the key type is obtained from CompositeKeyId.
 * @param	MinimumKeyObjectLen Size of the complete exported object for the Generate flow.
 *				When the exported object is only the key bytes, pass
 *				KeyMetadata.Length; 0U has the same effect. Ignored when the generated
 *				key is stored in the vault and for the Store flow.
 *
 * @return
 *	- XST_SUCCESS, if input validation is successful.
 *	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateParams(const XAsu_KeyManagerParams *KmParamsPtr, u8 ValidationFlow,
			  u8 KeyType, u32 MinimumKeyObjectLen)
{
	volatile s32 Status = XST_FAILURE;
	u8 ValidatedKeyType = (u8)XASU_INVALID_SUBVAULT_ID;
	u8 SubVaultId = (u8)XASU_INVALID_SUBVAULT_ID;
	u32 RequiredKeyObjectLen = 0U;

	if (KmParamsPtr == NULL) {
		goto END;
	}

	if (ValidationFlow == XASU_KM_VALIDATE_FLOW_GENERATE) {
		Status = XAsu_KmValidateVaultParams(KmParamsPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** A key stored in the vault must carry the API's key type in CompositeKeyId. */
		if (KmParamsPtr->KeyObjectAddr == 0U) {
			Status = XST_FAILURE;
			ValidatedKeyType =
				XASU_KM_GET_KEY_TYPE(KmParamsPtr->KeyMetadata.CompositeKeyId);
			if (ValidatedKeyType != KeyType) {
				goto END;
			}
		}

		/** A Generate key type must not carry the wrapped-key bit, so it is not masked. */
		SubVaultId = KeyType;
	} else if (ValidationFlow == XASU_KM_VALIDATE_FLOW_STORE) {
		/** Store API requires both input key buffer and output key ID address. */
		if ((KmParamsPtr->KeyObjectAddr == 0U) || (KmParamsPtr->KeyIdAddr == 0U)) {
			goto END;
		}

		if (KmParamsPtr->KeyObjectLen == 0U) {
			goto END;
		}

		if (XAsu_KmValidateKeyMetadata(&KmParamsPtr->KeyMetadata) != XST_SUCCESS) {
			goto END;
		}

		ValidatedKeyType = XASU_KM_GET_KEY_TYPE(KmParamsPtr->KeyMetadata.CompositeKeyId);

		if (XAsu_KmValidateKeyAttributes(ValidatedKeyType,
						KmParamsPtr->KeyMetadata.KeyAttributes) != XST_SUCCESS) {
			goto END;
		}

		SubVaultId = (u8)(ValidatedKeyType & (u8)(~XASU_KM_KEYTYPE_WRAPPED_BIT_MASK));
	} else {
		goto END;
	}

	/** Checks common to both flows. */
	Status = XST_FAILURE;

	if (SubVaultId >= (u8)XASU_INVALID_SUBVAULT_ID) {
		goto END;
	}

	if (XAsu_KmValidateAesKeySplitUseCase(SubVaultId,
					      KmParamsPtr->KeyMetadata.KeyUseCase) != XST_SUCCESS) {
		goto END;
	}

	if (ValidationFlow == XASU_KM_VALIDATE_FLOW_GENERATE) {
		Status = XAsu_KmValidateKeyLength(KmParamsPtr->KeyMetadata.Length, KeyType);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XST_FAILURE;

		if (((KeyType == (u8)XASU_RSA_PVT_SUBVAULT_ID) ||
		     (KeyType == (u8)XASU_RSA_PUB_SUBVAULT_ID)) &&
		    (KmParamsPtr->KeyMetadata.KeyAttributes !=
		     XASU_KM_RSA_PVT_PRIME_NUM_PRESENT)) {
			goto END;
		}

		/** Check the ECC curve and its private-key length in the shared generate path. */
		if ((KeyType == (u8)XASU_ECC_PUB_SUBVAULT_ID) ||
		    (KeyType == (u8)XASU_ECC_PVT_SUBVAULT_ID)) {
			if (XAsu_EccValidateCurveInfo(KmParamsPtr->KeyMetadata.KeyAttributes,
						      KmParamsPtr->KeyMetadata.Length) != XST_SUCCESS) {
				goto END;
			}
		}

		/**
		 * An export buffer must hold the larger of the key and the complete exported
		 * object. AES, IV and KDF/HMAC export only the key, so the key length is that
		 * size. A key pair passes the object size, which is already at least the key
		 * length.
		 */
		RequiredKeyObjectLen = KmParamsPtr->KeyMetadata.Length;
		if (MinimumKeyObjectLen > RequiredKeyObjectLen) {
			RequiredKeyObjectLen = MinimumKeyObjectLen;
		}
		if ((KmParamsPtr->KeyObjectAddr != 0U) &&
		    ((KmParamsPtr->KeyObjectLen == 0U) ||
		     (KmParamsPtr->KeyObjectLen < RequiredKeyObjectLen))) {
			goto END;
		}
	} else {
		if ((ValidatedKeyType & XASU_KM_KEYTYPE_WRAPPED_BIT_MASK) ==
		    XASU_KM_KEYTYPE_WRAPPED_BIT_MASK) {
			if ((KmParamsPtr->WrappedInputLen == 0U) ||
			    (KmParamsPtr->WrappedInputLen > KmParamsPtr->KeyObjectLen)) {
				goto END;
			}
		} else {
			/**
			 * Plain Store Key input size depends on key type (RSA/ECC public objects
			 * are larger than KeyMetadata.Length alone). Require KeyObjectLen to cover
			 * the bytes the store path will consume from KeyObjectAddr.
			 */
			if (XAsu_KmGetStoreKeyObjectLen(KmParamsPtr->KeyMetadata.CompositeKeyId,
							KmParamsPtr->KeyMetadata.Length,
							&RequiredKeyObjectLen) != XST_SUCCESS) {
				goto END;
			}
			if (RequiredKeyObjectLen > KmParamsPtr->KeyObjectLen) {
				goto END;
			}
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Compute the minimum KeyObjectLen required for a plain Store Key request.
 *
 * Matches the client-facing object sizes used by provisioning store serialization and by the
 * Store Key DMA/repack path: RSA private and public use fixed object layouts, ECC public uses
 * twice the curve length, and other types use KeyLen.
 *
 * @param	CompositeKeyId		Composite key identifier that encodes the key type.
 * @param	KeyLen			KeyMetadata.Length from the Store Key request.
 * @param	KeyObjectLenPtr		Output pointer for the required buffer length in bytes.
 *
 * @return
 *	- XST_SUCCESS, if the required length is returned.
 *	- XST_FAILURE, if KeyObjectLenPtr is NULL, KeyLen is zero, or ECC public KeyLen is
 *	  unsupported.
 *
 *************************************************************************************************/
s32 XAsu_KmGetStoreKeyObjectLen(u32 CompositeKeyId, u16 KeyLen, u32 *KeyObjectLenPtr)
{
	s32 Status = XST_FAILURE;
	u32 DataLen;
	u8 KeyType;

	if ((KeyObjectLenPtr == NULL) || (KeyLen == 0U)) {
		goto END;
	}

	KeyType = XASU_KM_GET_KEY_TYPE(CompositeKeyId) &
		  (u8)(~XASU_KM_KEYTYPE_WRAPPED_BIT_MASK);

	switch (KeyType) {
	case XASU_RSA_PVT_SUBVAULT_ID:
		DataLen = (u32)sizeof(XAsu_RsaKeyPairObject);
		break;
	case XASU_RSA_PUB_SUBVAULT_ID:
		DataLen = (u32)sizeof(XAsu_RsaPubKeyComp);
		break;
	case XASU_ECC_PUB_SUBVAULT_ID:
		if (KeyLen > XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES) {
			goto END;
		}
		DataLen = XAsu_DoubleCurveLength((u32)KeyLen);
		break;
	default:
		DataLen = (u32)KeyLen;
		break;
	}

	*KeyObjectLenPtr = DataLen;
	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
