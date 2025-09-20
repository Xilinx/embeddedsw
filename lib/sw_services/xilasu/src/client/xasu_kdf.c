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
 *       rmv  09/11/25 Move KDF parameter validation function to common file
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
#include "xasu_shainfo.h"
#include "xasu_hmacinfo.h"
#include "xasu_kdf_common.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to generate the derived key of specified key
 * length by using the Key Derivative Function (KDF) in counter mode.
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
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KdfGenerate(XAsu_ClientParams *ClientParamsPtr, XAsu_KdfParams *KdfParamsPtr)
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
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0x0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Get the command ID based on SHA type. */
	if (KdfParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CommandId = XASU_KDF_GENERATE_SHA2_CMD_ID;
	} else {
		CommandId = XASU_KDF_GENERATE_SHA3_CMD_ID;
	}
	Header = XAsu_CreateHeader(CommandId, UniqueId, XASU_MODULE_KDF_ID, 0U,
				ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, KdfParamsPtr,
						(u32)(sizeof(XAsu_KdfParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform KDF Known Answer Tests (KAT's).
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 *
 * @return
 * 	- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 	- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 	- XST_FAILURE, if sending an IPI request to ASU fails.
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

	Header = XAsu_CreateHeader(XASU_KDF_KAT_CMD_ID, UniqueId, XASU_MODULE_KDF_ID, 0U,
				ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}
/** @} */
