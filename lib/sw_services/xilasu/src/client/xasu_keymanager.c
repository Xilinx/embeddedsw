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
#include "xasu_def.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_KM_RSA_NUM_OF_KEY_IDS	(2U) /**< Number of key IDs returned for RSA key pair generation command */

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

	/** Validate at least one sub-vault has non-zero capacity. */
	if ((KmVaultParamPtr->AESKeyVaultCapacity == 0U) &&
	    (KmVaultParamPtr->IVVaultCapacity == 0U) &&
	    (KmVaultParamPtr->RSAPvtKeyVaultCapacity == 0U) &&
	    (KmVaultParamPtr->RSAPubKeyVaultCapacity == 0U) &&
	    (KmVaultParamPtr->ECCPvtKeyVaultCapacity == 0U) &&
	    (KmVaultParamPtr->ECCPubKeyVaultCapacity == 0U) &&
	    (KmVaultParamPtr->KDFKeyVaultCapacity == 0U) &&
	    (KmVaultParamPtr->LMSKeyVaultCapacity == 0U) &&
	    (KmVaultParamPtr->X509KeyVaultCapacity == 0U)) {
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

	Status = XAsu_KmValidateKeyLength(KmSubVaultParamPtr, XASU_KM_AES_KEYTYPE);
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

	Status = XAsu_KmValidateKeyLength(KmSubVaultParamPtr, XASU_KM_IV_KEYTYPE);
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
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KmDeleteKeyVault(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
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
				XASU_MODULE_KEYMANAGER_ID, XASU_CMD_LEN_ZERO,
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

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
	Status = XAsu_KmValidateKeyLength(KmSubVaultParamPtr, XASU_KM_RSA_KEYTYPE);
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
	 *   (XASU_KM_RSA_NUM_OF_KEY_IDS * XASU_KM_OUTPUT_ID_SIZE_IN_BYTES)
	 *   to store both the key IDs.
	 */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
						(u8 *)(UINTPTR)KmSubVaultParamPtr->KeyIdAddr,
						(XASU_KM_RSA_NUM_OF_KEY_IDS * XASU_KM_OUTPUT_ID_SIZE_IN_BYTES),
						XASU_TRUE);
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
/** @} */
