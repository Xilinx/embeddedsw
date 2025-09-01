/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_hmac.c
 *
 * This file contains the implementation of the client interface functions for Hash-Based Message
 * Authentication Code(HMAC) module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  01/02/25 Initial release
 *       yog  07/10/25 Added support for priority based multiple request and context verification
 * 1.1   kd   07/23/25 Fixed gcc warnings
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_hmac_client_apis HMAC Client APIs
 * @{
*/

/*************************************** Include Files *******************************************/
#include "xasu_hmac.h"
#include "xasu_def.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_ValidateHmacParameters(const XAsu_HmacParams *HmacParamsPtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to compute the Message Authentication Code (MAC)
 * 		for the given message using the specified hash function and the provided key.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	HmacParamsPtr	Pointer to XAsu_HmacParams structure which holds the parameters of
 * 				HMAC input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 * 		- XASU_CLIENT_CTX_NOT_CREATED, if client context is not created.
 * 		- XASU_FAIL_SAVE_CTX, if saving context fails.
 * 		- XASU_REQUEST_INPROGRESS, if split request already in progress.
 *
 *************************************************************************************************/
s32 XAsu_HmacCompute(XAsu_ClientParams *ClientParamsPtr, XAsu_HmacParams *HmacParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 CommandId;
	u8 UniqueId;
	static void *P0HmacCtx = NULL;
	static void *P1HmacCtx = NULL;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_ValidateHmacParameters(HmacParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** If the operation flag is set to INIT, */
	if ((HmacParamsPtr->OperationFlags & XASU_HMAC_INIT) == XASU_HMAC_INIT) {
		/**
		 * - If either P0HmacCtx or P1HmacCtx is not NULL depending on whether the priority
		 * is HIGH or LOW, it indicates that a multi-request operation is already in progress.
		 */
		if (((ClientParamsPtr->Priority == XASU_PRIORITY_HIGH) && (P0HmacCtx != NULL)) ||
			((ClientParamsPtr->Priority == XASU_PRIORITY_LOW) && (P1HmacCtx != NULL))) {
			/** - Additionally, if the operation flag is set to UPDATE and FINAL, */
			if (((HmacParamsPtr->OperationFlags & XASU_HMAC_UPDATE)
				== XASU_HMAC_UPDATE) &&
			   ((HmacParamsPtr->OperationFlags & XASU_HMAC_FINAL)
				== XASU_HMAC_FINAL)) {
				/** - Generate a Unique ID for the new request. */
				UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr,
								NULL, 0U, XASU_FALSE);
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
			UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr,
							NULL, 0U, XASU_FALSE);
			if (UniqueId >= XASU_UNIQUE_ID_MAX) {
				Status = XASU_INVALID_UNIQUE_ID;
				goto END;
			}
			/** If P0 priority and respective context is NULL, */
			if ((ClientParamsPtr->Priority == XASU_PRIORITY_HIGH) && (P0HmacCtx == NULL)) {
				/** - Save the P0 Context. */
				P0HmacCtx = XAsu_UpdateNGetCtx(UniqueId);
				if (P0HmacCtx == NULL) {
					Status = XASU_FAIL_SAVE_CTX;
					goto END;
				}
				ClientParamsPtr->ClientCtx = P0HmacCtx;
			} else {
				/** If P1 priority and respective context is NULL, */
				if ((ClientParamsPtr->Priority == XASU_PRIORITY_LOW) && (P1HmacCtx == NULL)) {
					/** - Save the P1 Context. */
					P1HmacCtx = XAsu_UpdateNGetCtx(UniqueId);
					if (P1HmacCtx == NULL) {
						Status = XASU_FAIL_SAVE_CTX;
						goto END;
					}
					ClientParamsPtr->ClientCtx = P1HmacCtx;
				}
			}
		}
	}
	/** If the operation flag is either UPDATE or FINAL, */
	else {
		/** - Check if the context already exists. If not, return an error. */
		if (ClientParamsPtr->ClientCtx != NULL) {
			Status = XAsu_VerifyNGetUniqueIdCtx(ClientParamsPtr->ClientCtx, &UniqueId);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		} else {
			Status = XASU_CLIENT_CTX_NOT_CREATED;
			goto END;
		}
	}

	/** If FINISH operation flag is set, update response buffer details. */
	if ((HmacParamsPtr->OperationFlags & XASU_HMAC_FINAL) == XASU_HMAC_FINAL) {
		XAsu_UpdateCallBackDetails(UniqueId, (u8 *)(UINTPTR)HmacParamsPtr->HmacAddr,
			HmacParamsPtr->HmacLen, XASU_TRUE);
		if (ClientParamsPtr->ClientCtx == P0HmacCtx) {
			P0HmacCtx = NULL;
		} else {
			if (ClientParamsPtr->ClientCtx == P1HmacCtx) {
				P1HmacCtx = NULL;
			}
		}
		/* Free HMAC Ctx */
		XAsu_FreeCtx(ClientParamsPtr->ClientCtx);
		ClientParamsPtr->ClientCtx = NULL;
	}

	/** Get the command ID based on SHA type. */
	if (HmacParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CommandId = XASU_HMAC_COMPUTE_SHA2_CMD_ID;
	} else {
		CommandId = XASU_HMAC_COMPUTE_SHA3_CMD_ID;
	}
	/** Create command header. */
	Header = XAsu_CreateHeader(CommandId, UniqueId, XASU_MODULE_HMAC_ID, 0U,
				ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, HmacParamsPtr,
						(u32)(sizeof(XAsu_HmacParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform HMAC Known Answer Tests (KAT's).
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
s32 XAsu_HmacKat(XAsu_ClientParams *ClientParamsPtr)
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
	Header = XAsu_CreateHeader(XASU_HMAC_KAT_CMD_ID, UniqueId, XASU_MODULE_HMAC_ID, 0U,
				ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the input HMAC parameters.
 *
 * @param	HmacParamsPtr	Pointer to XAsu_HmacParams structure which holds the parameters of
 * 				HMAC input arguments.
 *
 * @return
 * 	- XST_SUCCESS, if HMAC input parameters validation is successful.
 * 	- XST_FAILURE, if HMAC input parameters validation fails.
 *
 *************************************************************************************************/
static s32 XAsu_ValidateHmacParameters(const XAsu_HmacParams *HmacParamsPtr)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	if (HmacParamsPtr == NULL) {
		goto END;
	}

	if ((HmacParamsPtr->OperationFlags &
	     (XASU_HMAC_INIT | XASU_HMAC_UPDATE | XASU_HMAC_FINAL)) == 0U) {
		goto END;
	}

	if (((HmacParamsPtr->OperationFlags & XASU_HMAC_INIT) == XASU_HMAC_INIT)) {
		if ((HmacParamsPtr->ShaType != XASU_SHA2_TYPE) &&
		    (HmacParamsPtr->ShaType != XASU_SHA3_TYPE)) {
			goto END;
		}

		if ((HmacParamsPtr->ShaMode != XASU_SHA_MODE_256) &&
		    (HmacParamsPtr->ShaMode != XASU_SHA_MODE_384) &&
		    (HmacParamsPtr->ShaMode != XASU_SHA_MODE_512) &&
		    (((HmacParamsPtr->ShaType != XASU_SHA3_TYPE) ||
		    (HmacParamsPtr->ShaMode != XASU_SHA_MODE_SHAKE256)))) {
			goto END;
		}

		if (((HmacParamsPtr->ShaMode == XASU_SHA_MODE_256) &&
		     (HmacParamsPtr->HmacLen != XASU_SHA_256_HASH_LEN)) ||
		    ((HmacParamsPtr->ShaMode == XASU_SHA_MODE_384) &&
		     (HmacParamsPtr->HmacLen != XASU_SHA_384_HASH_LEN)) ||
		    ((HmacParamsPtr->ShaMode == XASU_SHA_MODE_512) &&
		     (HmacParamsPtr->HmacLen != XASU_SHA_512_HASH_LEN)) ||
		    ((HmacParamsPtr->ShaMode == XASU_SHA_MODE_SHAKE256) &&
		     (HmacParamsPtr->HmacLen != XASU_SHA_256_HASH_LEN))) {
			goto END;
		}

		if ((HmacParamsPtr->KeyAddr == 0U) ||
		    (HmacParamsPtr->KeyLen == 0U) ||
		    (HmacParamsPtr->KeyLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) {
			goto END;
		}
	}

	if ((HmacParamsPtr->OperationFlags & XASU_HMAC_UPDATE) == XASU_HMAC_UPDATE) {
		if ((HmacParamsPtr->IsLast != XASU_TRUE) && (HmacParamsPtr->IsLast != XASU_FALSE)) {
			goto END;
		}

		if ((HmacParamsPtr->MsgBufferAddr == 0U) ||
		    (HmacParamsPtr->MsgLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) {
			goto END;
		}
	}

	if ((((HmacParamsPtr->OperationFlags & XASU_HMAC_FINAL) == XASU_HMAC_FINAL) &&
	     (HmacParamsPtr->HmacAddr == 0U))) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
