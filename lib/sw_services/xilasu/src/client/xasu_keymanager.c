/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xasu_aesinfo.h"
#include "xasu_def.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

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
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
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
				XASU_MODULE_KEYMANAGER_ID, 0U, ClientParamPtr->SecureFlag);

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

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (KmSubVaultParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate key length is either 128-bit or 256-bit. */
	if ((KmSubVaultParamPtr->Length != XASU_AES_KEY_SIZE_128BIT_IN_BYTES) &&
	   (KmSubVaultParamPtr->Length != XASU_AES_KEY_SIZE_256BIT_IN_BYTES)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate at least one output destination is provided. */
	if ((KmSubVaultParamPtr->KeyObjectAddr == 0U) && (KmSubVaultParamPtr->KeyIdAddr == 0U)) {
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
				XASU_MODULE_KEYMANAGER_ID, 0U, ClientParamPtr->SecureFlag);

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
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
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

	if (KmSubVaultParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate IV length is non-zero. */
	if (KmSubVaultParamPtr->Length == 0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate IV length does not exceed maximum size. */
	if (KmSubVaultParamPtr->Length > XASU_AES_IV_SIZE_128BIT_IN_BYTES) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate at least one output destination is provided. */
	if ((KmSubVaultParamPtr->KeyObjectAddr == 0U) && (KmSubVaultParamPtr->KeyIdAddr == 0U)) {
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
				XASU_MODULE_KEYMANAGER_ID, 0U, ClientParamPtr->SecureFlag);

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
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
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
				XASU_MODULE_KEYMANAGER_ID, 0U, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}
/** @} */