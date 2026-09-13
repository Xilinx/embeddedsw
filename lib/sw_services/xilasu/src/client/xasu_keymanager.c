/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keymanager.c
 *
 * This file contains the implementation of the client interface functions for
 * KeyManager driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   11/25/25 Initial release
 *       yog  01/28/26 Added RSA key pair generation client API.
 *       kp   08/22/26 Used composite key ID in key metadata.
 *       vm   09/15/26 Added key vault export and import client APIs
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_keymanager_client_apis KeyManager Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_keymanager.h"
#include "xasu_keymanager_common.h"
#include "xasu_aesinfo.h"
#include "xasu_eccinfo.h"
#include "xasu_mldsainfo.h"
#include "xasu_mlkeminfo.h"
#include "xasu_def.h"
#include "xasu_ecies_common.h"
#include "xasu_generic.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_KM_NUM_OF_KEY_IDS	(2U) /**< Number of key IDs returned for RSA/ECC key pair generation command */
#define XASU_KM_RSA_PVT_PRIME_NUM_PRESENT	(2U) /**< Exact RSA private key attribute value
							indicating presence of prime components */

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform Key Vault creation for the
 * 		requested subsystem with the provided inputs.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	KmVaultParamPtr		Pointer to XAsu_KeyManagerSubVaultParams structure
 * 					which holds the parameters of KeyVault input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmCreateKeyVault(XAsu_ClientParams *ClientParamPtr,
			XAsu_KeyManagerSubVaultParams *KmVaultParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (KmVaultParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (KmVaultParamPtr->VaultIdAddr == 0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateVaultCreateParams(KmVaultParamPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((!XASU_IS_REDUNDANT_BYTE_VALID(KmVaultParamPtr->AccessRights))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, (u8 *)(UINTPTR)KmVaultParamPtr->VaultIdAddr,
						XASU_KM_OUTPUT_ID_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_CREATE_KEYVAULT_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KeyManagerSubVaultParams) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, KmVaultParamPtr,
					(u32)(sizeof(XAsu_KeyManagerSubVaultParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform AES key generation for the
 * 		requested subsystem on the provided key size.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	KmSubVaultParamPtr	Pointer to XAsu_KeyManagerParams structure which holds the
 *					parameters of KeyVault input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmGenerateAesKey(XAsu_ClientParams *ClientParamPtr,
				XAsu_KeyManagerParams *KmSubVaultParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_KmValidateVaultParams(KmSubVaultParamPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate key use case against the key type. */
	if (XAsu_KmValidateAesKeySplitUseCase((u8)XASU_AES_SUBVAULT_ID,
			KmSubVaultParamPtr->KeyMetadata.KeyUseCase) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateKeyLength(KmSubVaultParamPtr->KeyMetadata.Length,
					  XASU_KM_AES_KEYTYPE);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
						(u8 *)(UINTPTR)KmSubVaultParamPtr->KeyIdAddr,
						XASU_KM_OUTPUT_ID_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_GEN_AES_KEY_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KeyManagerParams) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, KmSubVaultParamPtr,
					(u32)(sizeof(XAsu_KeyManagerParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform AES IV generation for the
 * 		requested subsystem on the provided IV size.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	KmSubVaultParamPtr	Pointer to XAsu_KeyManagerParams structure which holds the
 *					parameters of KeyVault input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmGenerateAesIv(XAsu_ClientParams *ClientParamPtr,
				XAsu_KeyManagerParams *KmSubVaultParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_KmValidateVaultParams(KmSubVaultParamPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateKeyLength(KmSubVaultParamPtr->KeyMetadata.Length,
					  XASU_KM_IV_KEYTYPE);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
					(u8 *)(UINTPTR)KmSubVaultParamPtr->KeyIdAddr,
					XASU_KM_OUTPUT_ID_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_GEN_AES_IV_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KeyManagerParams) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, KmSubVaultParamPtr,
					(u32)(sizeof(XAsu_KeyManagerParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform key vault deletion for the
 * 		requested subsystem.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the
 * 				client input parameters.
 * @param	VaultId		ID of the vault to be deleted.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmDeleteKeyVault(XAsu_ClientParams *ClientParamPtr, u32 VaultId)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((VaultId == 0U) || (VaultId >= XASU_KM_MAX_VAULTS)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_DELETE_KEYVAULT_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID, (sizeof(VaultId) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, &VaultId, sizeof(VaultId), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform key deletion from the
 * 		vault for the requested subsystem.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the
 * 				client input parameters.
 * @param	KeyId		Composite key identifier, built with XASU_KM_COMPOSE_KEY_ID().
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmDeleteKey(XAsu_ClientParams *ClientParamPtr, u32 KeyId)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate KeyId is not zero. */
	if (KeyId == 0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_DELETE_KEY_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(XASU_KM_OUTPUT_ID_SIZE_IN_BYTES / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, &KeyId, XASU_KM_OUTPUT_ID_SIZE_IN_BYTES, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform RSA key pair generation for the
 * 		requested subsystem on the provided key size.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	KmSubVaultParamPtr	Pointer to XAsu_KeyManagerParams structure which holds the
 *					parameters of KeyVault input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmGenerateRsaKeyPair(XAsu_ClientParams *ClientParamPtr,
				XAsu_KeyManagerParams *KmSubVaultParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 *RespBufferPtr = NULL;
	u8 RespSize = 0U;

	/** Validation of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_KmValidateVaultParams(KmSubVaultParamPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Key attribute should be set to indicate presence of prime components for RSA private key. */
	if (KmSubVaultParamPtr->KeyMetadata.KeyAttributes != XASU_KM_RSA_PVT_PRIME_NUM_PRESENT) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate key length is either 2048-bit or 3072-bit or 4096-bit for RSA. */
	Status = XAsu_KmValidateKeyLength(KmSubVaultParamPtr->KeyMetadata.Length,
					  XASU_KM_RSA_KEYTYPE);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function.
	 * Two key IDs will be returned for RSA key pair generation command and stores at the
	 * address provided in KmSubVaultParamPtr->KeyIdAddr by the client:
	 * - First key ID will be the public key ID
	 * - Second key ID will be the private key ID
	 * - The size allocated for key IDs should be
	 *   (XASU_KM_NUM_OF_KEY_IDS * XASU_KM_OUTPUT_ID_SIZE_IN_BYTES)
	 *   to store both the key IDs.
	 */
	if (KmSubVaultParamPtr->KeyIdAddr != 0U) {
		RespBufferPtr = (u8 *)(UINTPTR)KmSubVaultParamPtr->KeyIdAddr;
		RespSize = (XASU_KM_NUM_OF_KEY_IDS * XASU_KM_OUTPUT_ID_SIZE_IN_BYTES);
	}
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, RespBufferPtr, RespSize, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KeyManagerParams) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, KmSubVaultParamPtr,
					(u32)(sizeof(XAsu_KeyManagerParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform ECC key pair generation for the
 * 		requested subsystem on the provided key size.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	KmSubVaultParamPtr	Pointer to XAsu_KeyManagerParams structure which holds the
 *					parameters of KeyVault input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmGenerateEccKeyPair(XAsu_ClientParams *ClientParamPtr,
				XAsu_KeyManagerParams *KmSubVaultParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 *RespBufferPtr = NULL;
	u8 RespSize = 0U;

	/** Validation of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_KmValidateVaultParams(KmSubVaultParamPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate the curve length and curve type for ECC. */
	Status = XAsu_EccValidateCurveInfo(KmSubVaultParamPtr->KeyMetadata.KeyAttributes,
			KmSubVaultParamPtr->KeyMetadata.Length);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function.
	 * Two key IDs will be returned for ECC key pair generation command and stores at the
	 * address provided in KmSubVaultParamPtr->KeyIdAddr by the client:
	 * - First key ID will be the public key ID
	 * - Second key ID will be the private key ID
	 * - The size allocated for key IDs should be
	 *   (XASU_KM_NUM_OF_KEY_IDS * XASU_KM_OUTPUT_ID_SIZE_IN_BYTES)
	 *   to store both the key IDs.
	 */
	if (KmSubVaultParamPtr->KeyIdAddr != 0U) {
		RespBufferPtr = (u8 *)(UINTPTR)KmSubVaultParamPtr->KeyIdAddr;
		RespSize = (XASU_KM_NUM_OF_KEY_IDS * XASU_KM_OUTPUT_ID_SIZE_IN_BYTES);
	}
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, RespBufferPtr, RespSize, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KeyManagerParams) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, KmSubVaultParamPtr,
					(u32)(sizeof(XAsu_KeyManagerParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform ML-DSA key pair generation for
 *		the requested subsystem.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	KmSubVaultParamPtr	Pointer to XAsu_KeyManagerPqcKeyGenParams structure which
 *					holds the parameters of KeyVault input arguments. The ML-DSA
 *					parameter set is passed in KmParams.KeyMetadata.KeyAttributes
 *					and the public key length in KmParams.KeyMetadata.Length.
 *					A non-zero SeedAddr derives the pair from that seed instead of
 *					one drawn from the TRNG. When the pair is stored,
 *					KmParams.KeyMetadata.CompositeKeyId selects the vault and the
 *					slot index requested in both the private and the public
 *					sub-vault, or zero to take the first free slot in each.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmGenerateMldsaKeyPair(XAsu_ClientParams *ClientParamPtr,
				XAsu_KeyManagerPqcKeyGenParams *KmSubVaultParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 *RespBufferPtr = NULL;
	u8 RespSize = 0U;
	u32 PubKeyLen = 0U;
	u32 PvtKeyLen = 0U;

	/** Validation of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (KmSubVaultParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateVaultParams(&KmSubVaultParamPtr->KmParams);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate the ML-DSA key generation parameters. */
	Status = XAsu_KmValidateMldsaKeyGenParams(KmSubVaultParamPtr, &PubKeyLen, &PvtKeyLen);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function.
	 * Two key IDs will be returned, as for the ECC key pair generation command:
	 * - First key ID will be the public key ID
	 * - Second key ID will be the private key ID
	 */
	if (KmSubVaultParamPtr->KmParams.KeyIdAddr != 0U) {
		RespBufferPtr = (u8 *)(UINTPTR)KmSubVaultParamPtr->KmParams.KeyIdAddr;
		RespSize = (XASU_KM_NUM_OF_KEY_IDS * XASU_KM_OUTPUT_ID_SIZE_IN_BYTES);
	}
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, RespBufferPtr, RespSize, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KeyManagerPqcKeyGenParams) /
					XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, KmSubVaultParamPtr,
					(u32)(sizeof(XAsu_KeyManagerPqcKeyGenParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform ML-KEM key pair generation for
 *		the requested subsystem.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	KmSubVaultParamPtr	Pointer to XAsu_KeyManagerPqcKeyGenParams structure which
 *					holds the parameters of KeyVault input arguments. The ML-KEM
 *					parameter set is passed in KmParams.KeyMetadata.KeyAttributes
 *					and the encapsulation key length in
 *					KmParams.KeyMetadata.Length. A non-zero SeedAddr derives the
 *					pair from that 64-byte d || z seed instead of one drawn from
 *					the TRNG. When the pair is stored,
 *					KmParams.KeyMetadata.CompositeKeyId selects the vault and the
 *					slot index requested in both the decapsulation and the
 *					encapsulation sub-vault, or zero to take the first free slot
 *					in each.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmGenerateMlkemKeyPair(XAsu_ClientParams *ClientParamPtr,
				XAsu_KeyManagerPqcKeyGenParams *KmSubVaultParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 *RespBufferPtr = NULL;
	u8 RespSize = 0U;
	u32 PubKeyLen = 0U;
	u32 PvtKeyLen = 0U;

	/** Validation of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (KmSubVaultParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateVaultParams(&KmSubVaultParamPtr->KmParams);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Validate the seed and the parameter set against the same contract the server validates
	 * them against, so that the request is rejected here rather than after an IPI round trip.
	 */
	Status = XAsu_KmValidateMlkemKeyGenParams(KmSubVaultParamPtr, &PubKeyLen, &PvtKeyLen);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function. Two key IDs are returned:
	 * the encapsulation key ID first, then the decapsulation key ID.
	 */
	if (KmSubVaultParamPtr->KmParams.KeyIdAddr != 0U) {
		RespBufferPtr = (u8 *)(UINTPTR)KmSubVaultParamPtr->KmParams.KeyIdAddr;
		RespSize = (XASU_KM_NUM_OF_KEY_IDS * XASU_KM_OUTPUT_ID_SIZE_IN_BYTES);
	}
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, RespBufferPtr, RespSize, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KeyManagerPqcKeyGenParams) /
					XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, KmSubVaultParamPtr,
					(u32)(sizeof(XAsu_KeyManagerPqcKeyGenParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform SLH-DSA key pair generation for
 *		the requested subsystem.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 *					client input parameters.
 * @param	KeyGenParamsPtr		Pointer to XAsu_SlhdsaKeyGenParams structure which holds
 *					the key generation parameters. ParamSet selects the key
 *					sizes and the hash family, and SkOutAddr and PkOutAddr name
 *					the buffers that receive the pair.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if no queue slot is available.
 *		- XASU_QUEUE_FULL, if Queue buffer is full.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmGenerateSlhdsaKeyPair(XAsu_ClientParams *ClientParamPtr,
				XAsu_SlhdsaKeyGenParams *KeyGenParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u32 PubKeyLen = 0U;
	u32 PvtKeyLen = 0U;

	/** Validation of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (KeyGenParamsPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate the SLH-DSA key generation parameters. */
	Status = XAsu_KmValidateSlhdsaKeyGenParams(KeyGenParamsPtr, &PubKeyLen, &PvtKeyLen);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_SlhdsaKeyGenParams) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, KeyGenParamsPtr,
					(u32)(sizeof(XAsu_SlhdsaKeyGenParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to generate the wrapped-provisioning (ECIES)
 *		ECC key pair for the requesting subsystem. Both halves are persisted in the
 *		ASU-owned exportable vault and neither the key material nor a key ID is returned:
 *		the ASU locates the private key internally when it unwraps provisioning material.
 *		A subsystem which already holds a provisioning key pair is refused; it must delete
 *		the existing pair first with XAsu_KmDeleteProvisioningKeyPair().
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the
 *				client input parameters. The command takes no other parameter: the
 *				vault id (ASU-owned exportable vault), use case (provisioning/ECIES
 *				unwrap), curve (P-384), key length, epoch time, non-depleting usage
 *				count and the key slot are all set internally by the ASU, the slot
 *				being derived from the requesting subsystem.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the generated unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmGenerateProvisioningKeyPair(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validation of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function. There is no response payload:
	 * the command returns status only and no key material or key ID reaches the caller.
	 */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_GEN_PROV_KEY_PAIR_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID, XASU_CMD_LEN_ZERO,
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to delete the wrapped-provisioning (ECIES)
 *		ECC key pair which the requesting subsystem holds in the ASU-owned exportable
 *		vault. A client cannot delete the pair itself with XAsu_KmDeleteKey(), because that
 *		vault is owned by the ASU, so this command is the only way to release the slot and
 *		roll the key pair over by generating a fresh one.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the
 *				client input parameters. The command takes no other parameter: the
 *				key slot is derived from the requesting subsystem, so a subsystem
 *				can only ever delete its own provisioning key pair.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the generated unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmDeleteProvisioningKeyPair(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validation of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate a unique ID and register the callback function. Status only, no response data. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_DELETE_PROV_KEY_PAIR_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID, XASU_CMD_LEN_ZERO,
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to store a key in the vault for the
 *		requested subsystem.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 *					client input parameters.
 * @param	KeyParams		Pointer to XAsu_KeyManagerParams structure which holds the
 *					key parameters.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the generated unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmStoreKey(XAsu_ClientParams *ClientParamPtr, XAsu_KeyManagerParams *KeyParams)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (KeyParams == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((KeyParams->KeyObjectAddr == 0U) || (KeyParams->KeyIdAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate key metadata. */
	if (XAsu_KmValidateKeyMetadata(&KeyParams->KeyMetadata) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate key attributes against the key type. */
	if (XAsu_KmValidateKeyAttributes(
			XASU_KM_GET_KEY_TYPE(KeyParams->KeyMetadata.CompositeKeyId),
			KeyParams->KeyMetadata.KeyAttributes) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate key use case against the key type. */
	if (XAsu_KmValidateAesKeySplitUseCase(
			XASU_KM_GET_KEY_TYPE(KeyParams->KeyMetadata.CompositeKeyId),
			KeyParams->KeyMetadata.KeyUseCase) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
						(u8 *)(UINTPTR)KeyParams->KeyIdAddr,
						XASU_KM_OUTPUT_ID_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_STORE_KEY_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KeyManagerParams) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, KeyParams,
					(u32)(sizeof(XAsu_KeyManagerParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform raw key generation for the
 * 		requested subsystem on the provided key size. This can be used for KDF and HMAC
 * 		key generation in the key vault.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	KmSubVaultParamPtr	Pointer to XAsu_KeyManagerParams structure which holds the
 *					parameters of KeyVault input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmGenerateRawKey(XAsu_ClientParams *ClientParamPtr,
				XAsu_KeyManagerParams *KmSubVaultParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_KmValidateVaultParams(KmSubVaultParamPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateKeyLength(KmSubVaultParamPtr->KeyMetadata.Length,
					  XASU_KM_KDF_HMAC_KEYTYPE);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
						(u8 *)(UINTPTR)KmSubVaultParamPtr->KeyIdAddr,
						XASU_KM_OUTPUT_ID_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_GEN_RAW_KEY_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KeyManagerParams) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, KmSubVaultParamPtr,
					(u32)(sizeof(XAsu_KeyManagerParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to export the key vault.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the
 *				client input parameters.
 * @param	ExportParams	Pointer to XAsu_KeyVaultTransferParams structure which holds the
 *				export parameters.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if unique ID generation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmExportKeyVault(XAsu_ClientParams *ClientParamPtr,
			  XAsu_KeyVaultTransferParams *ExportParams)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((ExportParams == NULL) || (ExportParams->DataAddr == 0U) ||
	    (ExportParams->BufSize == 0U) || (ExportParams->ExportResultAddr == 0U) ||
	    (ExportParams->RevokeEfuseNum >= XASU_KM_KV_REVOKE_EFUSE_COUNT)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** The context is what an import has to repeat, so an empty one is refused here. */
	if (XAsu_IsBufferNonZero(ExportParams->Context, XASU_KM_KV_CONTEXT_SIZE_IN_BYTES) !=
	    XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
						(u8 *)(UINTPTR)ExportParams->ExportResultAddr,
						XASU_KM_EXPORT_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_EXPORT_KEYVAULT_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KeyVaultTransferParams) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, ExportParams,
					(u32)(sizeof(XAsu_KeyVaultTransferParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to import the key vault.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the
 *				client input parameters.
 * @param	ImportParams	Pointer to XAsu_KeyVaultTransferParams structure which holds the
 *				import parameters.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if unique ID generation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmImportKeyVault(XAsu_ClientParams *ClientParamPtr,
			  XAsu_KeyVaultTransferParams *ImportParams)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((ImportParams == NULL) || (ImportParams->DataAddr == 0U) ||
	    (ImportParams->BufSize == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Only the context the blob was exported under recovers it, so it must be supplied. */
	if (XAsu_IsBufferNonZero(ImportParams->Context, XASU_KM_KV_CONTEXT_SIZE_IN_BYTES) !=
	    XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_IMPORT_KEYVAULT_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KeyVaultTransferParams) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, ImportParams,
					(u32)(sizeof(XAsu_KeyVaultTransferParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to set the epoch time.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the
 *				client input parameters.
 * @param	CurrentEpochTime	Current epoch time.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if unique ID generation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmSetEpochTime(XAsu_ClientParams *ClientParamPtr, u32 CurrentEpochTime)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (CurrentEpochTime == 0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_SET_EPOCH_TIME_CMD_ID, UniqueId,
				XASU_MODULE_KEYMANAGER_ID, (u8)(sizeof(CurrentEpochTime) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, &CurrentEpochTime,
					(u32)(sizeof(CurrentEpochTime)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to generate a key by combining two AES key
 *		splits using XOR and to store the combined key in the specified key vault slot, or
 *		in the first free slot when none is requested.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the
 *				client input parameters.
 * @param	XorParamsPtr	Pointer to the XAsu_KmXorKeyParams structure which holds the input
 *				key IDs, the metadata of the combined key and, when the slot is
 *				auto-assigned, the address where the composite key ID of the
 *				combined key is stored.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if unique ID generation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmXorKeys(XAsu_ClientParams *ClientParamPtr, XAsu_KmXorKeyParams *XorParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 *OutIdBufPtr = NULL;
	u8 OutIdBufSize = 0U;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (XorParamsPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate the XOR key combine parameters. */
	if (XAsu_KmValidateXorKeyParams(XorParamsPtr) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Register a response buffer only when the caller asked for the assigned key ID, which is
	 * required only for an auto-assigned slot.
	 */
	if (XorParamsPtr->OutKeyIdAddr != 0U) {
		OutIdBufPtr = (u8 *)(UINTPTR)XorParamsPtr->OutKeyIdAddr;
		OutIdBufSize = XASU_KM_OUTPUT_ID_SIZE_IN_BYTES;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, OutIdBufPtr, OutIdBufSize,
						XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_KM_XOR_KEY_CMD_ID, UniqueId, XASU_MODULE_KEYMANAGER_ID,
				(u8)(sizeof(XAsu_KmXorKeyParams) / XASU_WORD_LEN_IN_BYTES),
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, XorParamsPtr,
					(u32)(sizeof(XAsu_KmXorKeyParams)), Header);

END:
	return Status;
}
/** @} */
