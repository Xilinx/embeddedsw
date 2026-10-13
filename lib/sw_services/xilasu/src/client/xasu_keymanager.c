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
#include "xasu_generic.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_KM_NUM_OF_KEY_IDS	(2U) /**< Number of key IDs returned for RSA/ECC key pair generation command */

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
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmGenerateAesKey(XAsu_ClientParams *ClientParamPtr,
				XAsu_KeyManagerParams *KmSubVaultParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 *RespBufferPtr = NULL;
	u8 RespSize = 0U;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (KmSubVaultParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateParams(KmSubVaultParamPtr, XASU_KM_VALIDATE_FLOW_GENERATE,
				      (u8)XASU_AES_SUBVAULT_ID,
				      KmSubVaultParamPtr->KeyMetadata.Length);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function. A key ID is returned only
	 * when the key is stored in the vault; an exported key is written to KeyObjectAddr.
	 */
	if (KmSubVaultParamPtr->KeyIdAddr != 0U) {
		RespBufferPtr = (u8 *)(UINTPTR)KmSubVaultParamPtr->KeyIdAddr;
		RespSize = XASU_KM_OUTPUT_ID_SIZE_IN_BYTES;
	}
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, RespBufferPtr, RespSize, XASU_TRUE);
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
	u8 *RespBufferPtr = NULL;
	u8 RespSize = 0U;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (KmSubVaultParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateParams(KmSubVaultParamPtr, XASU_KM_VALIDATE_FLOW_GENERATE,
				      (u8)XASU_IV_SUBVAULT_ID,
				      KmSubVaultParamPtr->KeyMetadata.Length);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function. An IV ID is returned only
	 * when the IV is stored in the vault; an exported IV is written to KeyObjectAddr.
	 */
	if (KmSubVaultParamPtr->KeyIdAddr != 0U) {
		RespBufferPtr = (u8 *)(UINTPTR)KmSubVaultParamPtr->KeyIdAddr;
		RespSize = XASU_KM_OUTPUT_ID_SIZE_IN_BYTES;
	}
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, RespBufferPtr, RespSize, XASU_TRUE);
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

	if (KmSubVaultParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateParams(KmSubVaultParamPtr, XASU_KM_VALIDATE_FLOW_GENERATE,
				      (u8)XASU_RSA_PVT_SUBVAULT_ID,
				      XASU_KM_RSA_EXPORT_LEN(
					      KmSubVaultParamPtr->KeyMetadata.Length));
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

	Status = XAsu_KmValidateParams(KmSubVaultParamPtr, XASU_KM_VALIDATE_FLOW_GENERATE,
				      (u8)XASU_ECC_PVT_SUBVAULT_ID,
				      (u32)sizeof(XAsu_EccKeyPairObject));
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

	/** Validate the ML-DSA key generation parameters. */
	Status = XAsu_KmValidateMldsaKeyGenParams(KmSubVaultParamPtr, &PubKeyLen, &PvtKeyLen);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateParams(&KmSubVaultParamPtr->KmParams,
				      XASU_KM_VALIDATE_FLOW_GENERATE,
				      (u8)XASU_MLDSA_PVT_SUBVAULT_ID,
				      PubKeyLen + PvtKeyLen);
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

	/**
	 * Validate the seed and the parameter set against the same contract the server validates
	 * them against, so that the request is rejected here rather than after an IPI round trip.
	 */
	Status = XAsu_KmValidateMlkemKeyGenParams(KmSubVaultParamPtr, &PubKeyLen, &PvtKeyLen);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateParams(&KmSubVaultParamPtr->KmParams,
				      XASU_KM_VALIDATE_FLOW_GENERATE,
				      (u8)XASU_MLKEM_PVT_SUBVAULT_ID,
				      PubKeyLen + PvtKeyLen);
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
 *					sizes and the hash family, SkOutAddr and PkOutAddr name
 *					the buffers that receive the pair, and SkOutLen and
 *					PkOutLen must equal those sizes.
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

	Status = XAsu_KmValidateParams(KeyParams, XASU_KM_VALIDATE_FLOW_STORE,
				      (u8)XASU_INVALID_SUBVAULT_ID, 0U);
	if (Status != XST_SUCCESS) {
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
	u8 *RespBufferPtr = NULL;
	u8 RespSize = 0U;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (KmSubVaultParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_KmValidateParams(KmSubVaultParamPtr, XASU_KM_VALIDATE_FLOW_GENERATE,
				      (u8)XASU_KDF_HMAC_SUBVAULT_ID,
				      KmSubVaultParamPtr->KeyMetadata.Length);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function. A key ID is returned only
	 * when the key is stored in the vault; an exported key is written to KeyObjectAddr.
	 */
	if (KmSubVaultParamPtr->KeyIdAddr != 0U) {
		RespBufferPtr = (u8 *)(UINTPTR)KmSubVaultParamPtr->KeyIdAddr;
		RespSize = XASU_KM_OUTPUT_ID_SIZE_IN_BYTES;
	}
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, RespBufferPtr, RespSize, XASU_TRUE);
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
