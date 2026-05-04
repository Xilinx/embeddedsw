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
#include "xasu_def.h"
#include "xasu_ecies_common.h"

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

	if ((KmVaultParamPtr->AccessRights == 0U) ||
		(!XASU_IS_REDUNDANT_BYTE_VALID(KmVaultParamPtr->AccessRights))) {
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
 * @param	KeyId		Composite key identifier containing VaultId (bits [23:16]),
 *				KeyType (bits [31:24]), and KeyIndex (bits [15:0]).
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

	/** Validate key length is either 2078-bit or 3072-bit or 4096-bit for RSA. */
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
	    (ExportParams->BufSize == 0U) || (ExportParams->ActualSizeAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
						(u8 *)(UINTPTR)ExportParams->ActualSizeAddr,
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
/** @} */
