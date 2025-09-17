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

	if (XAsu_ShaValidateModeAndType(XASU_SHA3_TYPE, ShaClientParamPtr->ShaMode) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (((ShaClientParamPtr->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) &&
			(XAsu_ShaValidateHashLen(ShaClientParamPtr->ShaMode,
			ShaClientParamPtr->HashBufSize) != XST_SUCCESS)) {
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
		 * - If either P0Sha3Ctx or P1Sha3Ctx is not NULL depending on whether the priority
		 * is HIGH or LOW, it indicates that a multi-request operation is already in progress.
		 */
		if (((ClientParamPtr->Priority == XASU_PRIORITY_HIGH) && (P0Sha3Ctx != NULL)) ||
			((ClientParamPtr->Priority == XASU_PRIORITY_LOW) && (P1Sha3Ctx != NULL))) {
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
			if ((ClientParamPtr->Priority == XASU_PRIORITY_HIGH) && (P0Sha3Ctx == NULL)) {
				/** - Save the P0 Context. */
				P0Sha3Ctx = XAsu_UpdateNGetCtx(UniqueId);
				if (P0Sha3Ctx == NULL) {
					Status = XASU_FAIL_SAVE_CTX;
					goto END;
				}
				ClientParamPtr->ClientCtx = P0Sha3Ctx;
			} else {
				/** If P1 priority and respective context is NULL, */
				if ((ClientParamPtr->Priority == XASU_PRIORITY_LOW) && (P1Sha3Ctx == NULL)) {
					/** - Save the P1 Context. */
					P1Sha3Ctx = XAsu_UpdateNGetCtx(UniqueId);
					if (P1Sha3Ctx == NULL) {
						Status = XASU_FAIL_SAVE_CTX;
						goto END;
					}
					ClientParamPtr->ClientCtx = P1Sha3Ctx;
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
		Status = XAsu_UpdateCallBackDetails(UniqueId,
						    (u8 *)(UINTPTR)ShaClientParamPtr->HashAddr,
						    ShaClientParamPtr->HashBufSize, XASU_TRUE);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (ClientParamPtr->ClientCtx == P0Sha3Ctx) {
			P0Sha3Ctx = NULL;
		} else {
			if (ClientParamPtr->ClientCtx == P1Sha3Ctx) {
				P1Sha3Ctx = NULL;
			}
		}
		/* Free Sha3 Ctx */
		XAsu_FreeCtx(ClientParamPtr->ClientCtx);
		ClientParamPtr->ClientCtx = NULL;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_SHA_OPERATION_CMD_ID, UniqueId, XASU_MODULE_SHA3_ID, 0U,
				ClientParamPtr->SecureFlag);
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
	Header = XAsu_CreateHeader(XASU_SHA_KAT_CMD_ID, UniqueId,
				   XASU_MODULE_SHA3_ID, 0U, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}
/** @} */
