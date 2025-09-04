/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * 1.1   ma   12/12/24 Updated hash buffer address to the response buffer of the callback function
 *       lp   07/10/25 Added support for priority based multiple request and context verification
 *       kd   07/23/25 Fixed gcc warnings
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

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to generate the digest for the provided
 * 		input message using	the specified SHA algorithm.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds
 * 					client input arguments.
 * @param	ShaClientParamPtr	Pointer to the XAsu_ShaOperationCmd structure which holds
 * 					parameters of SHA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 * 		- XASU_REQUEST_INPROGRESS, if split request already in progress.
 * 		- XASU_CLIENT_CTX_NOT_CREATED, if client context is not created.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XASU_FAIL_SAVE_CTX, if saving context fails.
 *
 *************************************************************************************************/
s32 XAsu_Sha2Operation(XAsu_ClientParams *ClientParamPtr, XAsu_ShaOperationCmd *ShaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	static void *P0Sha2Ctx = NULL;
	static void *P1Sha2Ctx = NULL;

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

	if ((ShaClientParamPtr->ShaMode != XASU_SHA_MODE_256) &&
			(ShaClientParamPtr->ShaMode != XASU_SHA_MODE_384) &&
			(ShaClientParamPtr->ShaMode != XASU_SHA_MODE_512)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (((ShaClientParamPtr->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) &&
			(((ShaClientParamPtr->ShaMode == XASU_SHA_MODE_256) &&
			(ShaClientParamPtr->HashBufSize != XASU_SHA_256_HASH_LEN)) ||
			((ShaClientParamPtr->ShaMode == XASU_SHA_MODE_384) &&
			(ShaClientParamPtr->HashBufSize != XASU_SHA_384_HASH_LEN)) ||
			((ShaClientParamPtr->ShaMode == XASU_SHA_MODE_512) &&
			(ShaClientParamPtr->HashBufSize != XASU_SHA_512_HASH_LEN)))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ShaClientParamPtr->IsLast != XASU_TRUE) && (ShaClientParamPtr->IsLast != XASU_FALSE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** If the operation flag is set to START, */
	if ((ShaClientParamPtr->OperationFlags & XASU_SHA_START) == XASU_SHA_START) {
		/**
		 * - If either P0Sha2Ctx or P1Sha2Ctx is not NULL depending on whether the priority
		 * is HIGH or LOW, it indicates that a multi-request operation is already in progress.
		 */
		if (((ClientParamPtr->Priority == XASU_PRIORITY_HIGH) && (P0Sha2Ctx != NULL)) ||
			((ClientParamPtr->Priority == XASU_PRIORITY_LOW) && (P1Sha2Ctx != NULL))) {
			/** - Additionally, if the operation flag is set to FINISH, */
			if ((ShaClientParamPtr->OperationFlags & XASU_SHA_FINISH)
				== XASU_SHA_FINISH) {
				/** - Generate a Unique ID for the new request. */
				UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
								NULL, 0U, XASU_TRUE);
				if (UniqueId >= XASU_UNIQUE_ID_MAX) {
					Status = XASU_INVALID_UNIQUE_ID;
					goto END;
				}
			} else {
				/** - Else, return an error. */
				Status = XASU_REQUEST_INPROGRESS;
				goto END;
			}
		} else {
			/** - If context is NULL based on priority, generate Unique ID. */
			UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
							NULL, 0U, XASU_FALSE);
			if (UniqueId >= XASU_UNIQUE_ID_MAX) {
				Status = XASU_INVALID_UNIQUE_ID;
				goto END;
			}
			/** If P0 priority and respective context is NULL, */
			if ((ClientParamPtr->Priority == XASU_PRIORITY_HIGH) && (P0Sha2Ctx == NULL)) {
				/** - Save the P0 Context. */
				P0Sha2Ctx = XAsu_UpdateNGetCtx(UniqueId);
				if (P0Sha2Ctx == NULL) {
					Status = XASU_FAIL_SAVE_CTX;
					goto END;
				}
				ClientParamPtr->ClientCtx = P0Sha2Ctx;
			} else {
				/** If P1 priority and respective context is NULL, */
				if ((ClientParamPtr->Priority == XASU_PRIORITY_LOW) && (P1Sha2Ctx == NULL)) {
					/** - Save the P1 Context. */
					P1Sha2Ctx = XAsu_UpdateNGetCtx(UniqueId);
					if (P1Sha2Ctx == NULL) {
						Status = XASU_FAIL_SAVE_CTX;
						goto END;
					}
					ClientParamPtr->ClientCtx = P1Sha2Ctx;
				}
			}
		}
	}
	/** If the operation flag is either UPDATE or FINISH, */
	else {
		/** - Check if the context already exists. If not, return an error. */
		if (ClientParamPtr->ClientCtx != NULL) {
			Status = XAsu_VerifyNGetUniqueIdCtx(ClientParamPtr->ClientCtx, &UniqueId);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		} else {
			Status = XASU_CLIENT_CTX_NOT_CREATED;
			goto END;
		}
	}

	/** If FINISH operation flag is set, update response buffer details. */
	if ((ShaClientParamPtr->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) {
		XAsu_UpdateCallBackDetails(UniqueId, (u8 *)(UINTPTR)ShaClientParamPtr->HashAddr,
			ShaClientParamPtr->HashBufSize, XASU_TRUE);
		if (ClientParamPtr->ClientCtx == P0Sha2Ctx) {
			P0Sha2Ctx = NULL;
		} else {
			if (ClientParamPtr->ClientCtx == P1Sha2Ctx) {
				P1Sha2Ctx = NULL;
			}
		}
		/* Free Sha3 Ctx */
		XAsu_FreeCtx(ClientParamPtr->ClientCtx);
		ClientParamPtr->ClientCtx = NULL;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_SHA_OPERATION_CMD_ID, UniqueId, XASU_MODULE_SHA2_ID, 0U,
				ClientParamPtr->SecureFlag);
	/** Update request buffer and send an IPI request to ASU. */
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
 * @param	Sha2Mode	SHA2 mode to test KAT (XASU_SHA_MODE_256 or XASU_SHA_MODE_512).
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 *
 *************************************************************************************************/
s32 XAsu_Sha2Kat(XAsu_ClientParams *ClientParamPtr, u32 Sha2Mode)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((Sha2Mode != XASU_SHA_MODE_256) && (Sha2Mode != XASU_SHA_MODE_512)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate unique ID and register the callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_SHA_KAT_CMD_ID, UniqueId, XASU_MODULE_SHA2_ID, 0U,
				ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, &Sha2Mode, sizeof(Sha2Mode), Header);

END:
	return Status;
}
/** @} */
