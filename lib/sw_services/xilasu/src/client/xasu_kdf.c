/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_kdf.c
 *
 * This file contains the implementation of the client interface functions for HMAC based Key
 * Derivation Function (HKDF) module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/21/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_kdf_client_apis KDF Client APIs
 * @{
*/

/*************************************** Include Files *******************************************/
#include "xasu_kdf.h"
#include "xasu_def.h"
#include "xasu_status.h"
#include "xasu_shainfo.h"
#include "xasu_hmacinfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_ValidateKdfParameters(const XAsu_KdfParams *KdfParamsPtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform KDF compute operation using HMAC as
 * pseudorandom function with the user provided inputs to generate the keying material object of
 * given number of bytes in counter mode.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	KdfParamsPtr	Pointer to XAsu_KdfParams structure which holds the parameters of
 * 				KDF input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KdfCompute(XAsu_ClientParams *ClientParamsPtr, XAsu_KdfParams *KdfParamsPtr)
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

	Status = XAsu_ValidateKdfParameters(KdfParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0x0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Get the command ID based on SHA type. */
	if (KdfParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CommandId = XASU_KDF_COMPUTE_SHA2_CMD_ID;
	} else {
		CommandId = XASU_KDF_COMPUTE_SHA3_CMD_ID;
	}
	Header = XAsu_CreateHeader(CommandId, UniqueId, XASU_MODULE_KDF_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, KdfParamsPtr,
						(u32)(sizeof(XAsu_KdfParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs KDF Known Answer Tests (KAT's).
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
s32 XAsu_KdfKat(XAsu_ClientParams *ClientParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_KDF_KAT_CMD_ID, UniqueId, XASU_MODULE_KDF_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the KDF input parameters.
 *
 * @param	KdfParamsPtr	Pointer to XAsu_KdfParams structure which holds the parameters of
 * 				KDF input arguments.
 *
 * @return
 * 	- XST_SUCCESS, upon successful validation.
 * 	- XASU_INVALID_ARGUMENT, upon invalid arguments.
 *
 *************************************************************************************************/
static s32 XAsu_ValidateKdfParameters(const XAsu_KdfParams *KdfParamsPtr)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	if (KdfParamsPtr == NULL) {
		goto END;
	}

	if ((KdfParamsPtr->ShaType != XASU_SHA2_TYPE) &&
		(KdfParamsPtr->ShaType != XASU_SHA3_TYPE)) {
		goto END;
	}

	if ((KdfParamsPtr->ShaMode != XASU_SHA_MODE_SHA256) &&
		(KdfParamsPtr->ShaMode != XASU_SHA_MODE_SHA384) &&
		(KdfParamsPtr->ShaMode != XASU_SHA_MODE_SHA512) &&
		((KdfParamsPtr->ShaType != XASU_SHA3_TYPE) ||
		 (KdfParamsPtr->ShaMode != XASU_SHA_MODE_SHAKE256))) {
		goto END;
	}

	if ((KdfParamsPtr->KeyInAddr == 0U) || (KdfParamsPtr->KeyInLen == 0U) ||
		(KdfParamsPtr->KeyInLen > XASU_HMAC_MAX_KEY_LENGTH) ||
		(KdfParamsPtr->ContextAddr == 0U) || (KdfParamsPtr->ContextLen == 0U) ||
		(KdfParamsPtr->KeyOutAddr == 0U) || (KdfParamsPtr->KeyOutLen == 0U)) {
		goto END;
	}

	if (((KdfParamsPtr->ShaMode == XASU_SHA_MODE_SHA256) &&
		 (KdfParamsPtr->KeyInLen < XASU_SHA_256_HASH_LEN)) ||
		((KdfParamsPtr->ShaMode == XASU_SHA_MODE_SHA384) &&
		 (KdfParamsPtr->KeyInLen < XASU_SHA_384_HASH_LEN)) ||
		((KdfParamsPtr->ShaMode == XASU_SHA_MODE_SHA512) &&
		 (KdfParamsPtr->KeyInLen < XASU_SHA_512_HASH_LEN)) ||
		((KdfParamsPtr->ShaMode == XASU_SHA_MODE_SHAKE256) &&
		 (KdfParamsPtr->KeyInLen < XASU_SHA_256_HASH_LEN))) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
