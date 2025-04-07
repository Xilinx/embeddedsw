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
#include "xasu_status.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_ValidateHmacParameters(const XAsu_HmacParams *HmacParamsPtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to computes the Message Authentication Code (MAC)
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
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_HmacCompute(XAsu_ClientParams *ClientParamsPtr, XAsu_HmacParams *HmacParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 CommandId;
	u8 UniqueId;
	void *HmacCtx = NULL;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_ValidateHmacParameters(HmacParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** If operation flag is set to START, */
	if ((HmacParamsPtr->OperationFlags & XASU_HMAC_INIT) == XASU_HMAC_INIT) {
		/** - Generate a unique ID. */
		UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_FALSE);
		if (UniqueId >= XASU_UNIQUE_ID_MAX) {
			Status = XASU_INVALID_UNIQUE_ID;
			goto END;
		}
		/** - Save the Context. */
		HmacCtx = XAsu_UpdateNGetCtx(UniqueId);
		if (HmacCtx == NULL) {
			Status = XASU_FAIL_SAVE_CTX;
			goto END;
		}
		ClientParamsPtr->ClientCtx = HmacCtx;
	}
	/** If operation flag is either UPDATE or FINISH, */
	else {
		/** - Verify the context. */
		Status = XAsu_VerifyNGetUniqueIdCtx(ClientParamsPtr->ClientCtx, &UniqueId);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** If FINISH operation flag is set, update response buffer details. */
	if ((HmacParamsPtr->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) {
		XAsu_UpdateCallBackDetails(UniqueId, (u8 *)HmacParamsPtr->HmacAddr,
			HmacParamsPtr->HmacLen, XASU_TRUE);
		/* Free Sha2 Ctx */
		XAsu_FreeCtx(ClientParamsPtr->ClientCtx);
	}

	/** Get the command ID based on SHA type. */
	if (HmacParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CommandId = XASU_HMAC_COMPUTE_SHA2_CMD_ID;
	} else {
		CommandId = XASU_HMAC_COMPUTE_SHA3_CMD_ID;
	}
	/** Create command header. */
	Header = XAsu_CreateHeader(CommandId, UniqueId, XASU_MODULE_HMAC_ID, 0U);

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
 * 	- XST_FAILURE, if sending IPI request to ASU fails.
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
	Header = XAsu_CreateHeader(XASU_HMAC_KAT_CMD_ID, UniqueId, XASU_MODULE_HMAC_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the input ECC parameters.
 *
 * @param	EccParamsPtr	Pointer to XAsu_EccParams structure which holds the parameters of
 * 				ECC input arguments.
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

		if ((HmacParamsPtr->ShaMode != XASU_SHA_MODE_SHA256) &&
		    (HmacParamsPtr->ShaMode != XASU_SHA_MODE_SHA384) &&
		    (HmacParamsPtr->ShaMode != XASU_SHA_MODE_SHA512) &&
		    (((HmacParamsPtr->ShaType != XASU_SHA3_TYPE) ||
		    (HmacParamsPtr->ShaMode != XASU_SHA_MODE_SHAKE256)))) {
			goto END;
		}

		if (((HmacParamsPtr->ShaMode == XASU_SHA_MODE_SHA256) &&
		     (HmacParamsPtr->HmacLen != XASU_SHA_256_HASH_LEN)) ||
		    ((HmacParamsPtr->ShaMode == XASU_SHA_MODE_SHA384) &&
		     (HmacParamsPtr->HmacLen != XASU_SHA_384_HASH_LEN)) ||
		    ((HmacParamsPtr->ShaMode == XASU_SHA_MODE_SHA512) &&
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
