/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ecies.c
 *
 * This file contains the implementation of the client interface functions for ECIES module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  02/21/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_ecies_client_apis ECIES Client APIs
 * @{
*/

/*************************************** Include Files *******************************************/
#include "xasu_ecies.h"
#include "xasu_def.h"
#include "xasu_status.h"
#include "xasu_shainfo.h"
#include "xasu_ecies_common.h"
#include "xasu_ecc.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends a command to perform the encryption of the given plaintext
 * 		message using the ECIES protocol.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	EciesParamsPtr	Pointer to XAsu_EciesParams structure which holds the parameters of
 * 				ECIES encryption input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_EciesEncrypt(XAsu_ClientParams *ClientParamsPtr, XAsu_EciesParams *EciesParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 CommandId;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_ValidateEciesParameters(EciesParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	if (EciesParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CommandId = XASU_ECIES_ENCRYPT_SHA2_CMD_ID;
	} else {
		CommandId = XASU_ECIES_ENCRYPT_SHA3_CMD_ID;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(CommandId, UniqueId, XASU_MODULE_ECIES_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, EciesParamsPtr,
					(u32)(sizeof(XAsu_EciesParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform the decryption of the given ciphertext
 * 		using the ECIES protocol.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	EciesParamsPtr	Pointer to XAsu_EciesParams structure which holds the parameters of
 * 				ECIES decryption input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_EciesDecrypt(XAsu_ClientParams *ClientParamsPtr, XAsu_EciesParams *EciesParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 CommandId;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_ValidateEciesParameters(EciesParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_EccValidateCurveInfo(EciesParamsPtr->EccCurveType,
			EciesParamsPtr->EccKeyLength);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	if (EciesParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CommandId = XASU_ECIES_DECRYPT_SHA2_CMD_ID;
	} else {
		CommandId = XASU_ECIES_DECRYPT_SHA3_CMD_ID;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(CommandId, UniqueId, XASU_MODULE_ECIES_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, EciesParamsPtr,
					(u32)(sizeof(XAsu_EciesParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform ECIES Known Answer Tests (KAT's).
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 *
 * @return
 * 	- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 	- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 	- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_EciesKat(XAsu_ClientParams *ClientParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_ECIES_KAT_CMD_ID, UniqueId, XASU_MODULE_ECIES_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}
/** @} */
