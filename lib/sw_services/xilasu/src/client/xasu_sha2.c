/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_sha2.c
 *
 * This file contains the implementation of the client interface functions for
 * SHA 2 driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  08/22/24 Initial release
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       am   10/22/24 Fixed validation of hash buffer size.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_sha2_client_apis SHA2 Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_sha2.h"
#include "xasu_def.h"
#include "xasu_client.h"
#include "xasu_status.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function generates the digest for the provided input message using
 * 		the specified SHA algorithm.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds
 * 					client input arguments.
 * @param	ShaClientParamPtr	Pointer to the XAsu_ShaOperationCmd structure which holds
 * 					parameters of sha input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_Sha2Operation(XAsu_ClientParams *ClientParamPtr, XAsu_ShaOperationCmd *ShaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (ShaClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ShaClientParamPtr->OperationFlags &
			(XASU_SHA_START | XASU_SHA_UPDATE | XASU_SHA_FINISH)) == 0x0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((((ShaClientParamPtr->OperationFlags & XASU_SHA_UPDATE) == XASU_SHA_UPDATE) &&
			(ShaClientParamPtr->DataAddr == 0U)) ||
			(((ShaClientParamPtr->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) &&
			(ShaClientParamPtr->HashAddr == 0U))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * The maximum length of input data should be less than 0x1FFFFFFC bytes, which is the
	 * ASU DMA's maximum supported data transfer length.
	 */
	 if (((ShaClientParamPtr->OperationFlags & XASU_SHA_UPDATE) == XASU_SHA_UPDATE) &&
			(ShaClientParamPtr->DataSize > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ShaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA256) &&
			(ShaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA384) &&
			(ShaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA512)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (((ShaClientParamPtr->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) &&
			(((ShaClientParamPtr->ShaMode == XASU_SHA_MODE_SHA256) &&
			(ShaClientParamPtr->HashBufSize != XASU_SHA_256_HASH_LEN)) ||
			((ShaClientParamPtr->ShaMode == XASU_SHA_MODE_SHA384) &&
			(ShaClientParamPtr->HashBufSize != XASU_SHA_384_HASH_LEN)) ||
			((ShaClientParamPtr->ShaMode == XASU_SHA_MODE_SHA512) &&
			(ShaClientParamPtr->HashBufSize != XASU_SHA_512_HASH_LEN)))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ShaClientParamPtr->IsLast != TRUE) && (ShaClientParamPtr->IsLast != FALSE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_SHA_OPERATION_CMD_ID, UniqueId,
				   XASU_MODULE_SHA2_ID, 0U);

	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, ShaClientParamPtr,
						sizeof(XAsu_ShaOperationCmd), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs SHA2 Known Answer Tests (KAT's).
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_Sha2Kat(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_SHA_KAT_CMD_ID, UniqueId, XASU_MODULE_SHA2_ID, 0U);

	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}
