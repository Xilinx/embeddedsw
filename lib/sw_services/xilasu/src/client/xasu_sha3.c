/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_sha3.c
 *
 * This file contains the implementation of the client interface functions for
 * SHA 3 driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  06/04/24 Initial release
 *       ma   06/14/24 Updated XAsufw_ShaOperationCmd structure to have 64-bit hash address
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       am   10/22/24 Fixed validation of hash buffer size.
 * 1.1   ma   12/12/24 Updated hash buffer address to the response buffer of the callback function
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_sha3_client_apis SHA3 Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_sha3.h"
#include "xasu_def.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to generate the digest for the provided
 * 		input message using the specified SHA algorithm.
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
s32 XAsu_Sha3Operation(XAsu_ClientParams *ClientParamPtr, XAsu_ShaOperationCmd *ShaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	void *Sha3Ctx = NULL;

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
			(ShaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA512) &&
			(ShaClientParamPtr->ShaMode != XASU_SHA_MODE_SHAKE256)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (((ShaClientParamPtr->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) &&
			(((ShaClientParamPtr->ShaMode == XASU_SHA_MODE_SHA256) &&
			(ShaClientParamPtr->HashBufSize != XASU_SHA_256_HASH_LEN)) ||
			((ShaClientParamPtr->ShaMode == XASU_SHA_MODE_SHA384) &&
			(ShaClientParamPtr->HashBufSize != XASU_SHA_384_HASH_LEN)) ||
			((ShaClientParamPtr->ShaMode == XASU_SHA_MODE_SHA512) &&
			(ShaClientParamPtr->HashBufSize != XASU_SHA_512_HASH_LEN)) ||
			((ShaClientParamPtr->ShaMode == XASU_SHA_MODE_SHAKE256) &&
			(ShaClientParamPtr->HashBufSize != XASU_SHAKE_256_HASH_LEN)))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ShaClientParamPtr->IsLast != XASU_TRUE) && (ShaClientParamPtr->IsLast != XASU_FALSE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** If operation flag is set to START, */
	if ((ShaClientParamPtr->OperationFlags & XASU_SHA_START) == XASU_SHA_START) {
		/** - Generate Unique ID. */
		UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_FALSE);
		if (UniqueId >= XASU_UNIQUE_ID_MAX) {
			Status = XASU_INVALID_UNIQUE_ID;
			goto END;
		}
		/** - Save the Context. */
		Sha3Ctx = XAsu_UpdateNGetCtx(UniqueId);
		if (Sha3Ctx == NULL) {
			Status = XASU_FAIL_SAVE_CTX;
			goto END;
		}
		ClientParamPtr->ClientCtx = Sha3Ctx;
	}
	/** If operation flag is either UPDATE or FINISH, */
	else {
		/** - Verify the context. */
		Status = XAsu_VerifyNGetUniqueIdCtx(ClientParamPtr->ClientCtx, &UniqueId);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** If FINISH operation flag is set, update response buffer details. */
	if ((ShaClientParamPtr->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) {
		XAsu_UpdateCallBackDetails(UniqueId, (u8 *)ShaClientParamPtr->HashAddr,
			ShaClientParamPtr->HashBufSize, XASU_TRUE);
		/* Free Sha3 Ctx */
		XAsu_FreeCtx(ClientParamPtr->ClientCtx);
	}
	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_SHA_OPERATION_CMD_ID, UniqueId, XASU_MODULE_SHA3_ID, 0U);
	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, ShaClientParamPtr,
						sizeof(XAsu_ShaOperationCmd), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to run the SHA3 Known Answer Tests (KAT's).
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_Sha3Kat(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate unique ID and register the callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_SHA_KAT_CMD_ID, UniqueId,
				   XASU_MODULE_SHA3_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}
/** @} */
