/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       lp   07/10/25 Added support for priority based multiple request and context verification
 *       kd   07/23/25 Fixed gcc warnings
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
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 * 		- XASU_REQUEST_INPROGRESS, if split request already in progress.
 * 		- XASU_CLIENT_CTX_NOT_CREATED, if client context is not created.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XASU_FAIL_SAVE_CTX, if saving context fails.
 * 		- XASU_FREE_CTX_FAIL, if freeing context fails.
 *
 *************************************************************************************************/
s32 XAsu_Sha3Operation(XAsu_ClientParams *ClientParamPtr, XAsu_ShaOperationCmd *ShaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	static void *P0Sha3Ctx = NULL;
	static void *P1Sha3Ctx = NULL;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (ShaClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ShaClientParamPtr->OperationFlags & (XASU_INIT | XASU_UPDATE | XASU_FINISH)) == 0x0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((((ShaClientParamPtr->OperationFlags & XASU_UPDATE) == XASU_UPDATE) &&
			(ShaClientParamPtr->DataAddr == 0U)) ||
			(((ShaClientParamPtr->OperationFlags & XASU_FINISH) == XASU_FINISH) &&
			(ShaClientParamPtr->HashAddr == 0U))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * The maximum length of input data should be less than 0x1FFFFFFC bytes, which is the
	 * ASU DMA's maximum supported data transfer length.
	 */
	 if (((ShaClientParamPtr->OperationFlags & XASU_UPDATE) == XASU_UPDATE) &&
			(ShaClientParamPtr->DataSize > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (XAsu_ShaValidateModeAndType(XASU_SHA3_TYPE, ShaClientParamPtr->ShaMode) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (((ShaClientParamPtr->OperationFlags & XASU_FINISH) == XASU_FINISH) &&
			(XAsu_ShaValidateHashLen(ShaClientParamPtr->ShaMode,
			ShaClientParamPtr->HashBufSize) != XST_SUCCESS)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ShaClientParamPtr->IsLast != XASU_TRUE) && (ShaClientParamPtr->IsLast != XASU_FALSE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Handle context operation. */
	Status = XAsu_HandleContextOperation(ClientParamPtr, &P0Sha3Ctx, &P1Sha3Ctx,
				     ShaClientParamPtr->OperationFlags, &UniqueId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Cleanup on FINISH operation. */
	if ((ShaClientParamPtr->OperationFlags & XASU_FINISH) == XASU_FINISH) {
		Status = XAsu_CleanupFinishOperation(ClientParamPtr, &P0Sha3Ctx, &P1Sha3Ctx,
					     UniqueId, NULL, 0U);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_SHA_OPERATION_CMD_ID, UniqueId, XASU_MODULE_SHA3_ID,
				   (u8)(sizeof(XAsu_ShaOperationCmd) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);
	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, ShaClientParamPtr,
						sizeof(XAsu_ShaOperationCmd), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to run the SHA3 Known Answer Tests (KAT's).
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
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
	Header = XAsu_CreateHeader(XASU_SHA_KAT_CMD_ID, UniqueId, XASU_MODULE_SHA3_ID,
				   XASU_CMD_LEN_ZERO, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}
/** @} */
