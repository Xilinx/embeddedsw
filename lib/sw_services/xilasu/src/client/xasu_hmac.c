/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       kp   02/26/26 Added client-side HMAC SHA3-256 KAT
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
#include "xasu_keymanager_common.h"
#include "xil_sutil.h"

/************************************ Constant Definitions ***************************************/
#define XASU_HMAC_KAT_KEY_LEN_IN_BYTES		(32U)	/**< HMAC KAT key length in bytes */
#define XASU_HMAC_KAT_MSG_LEN_IN_BYTES		(32U)	/**< HMAC KAT message length in bytes */
#define XASU_HMAC_KAT_SHA3_256_HASH_LEN		(32U)	/**< HMAC KAT SHA3-256 hash length
										in bytes */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_ValidateHmacParameters(const XAsu_HmacParams *HmacParamsPtr);

/************************************ Variable Definitions ***************************************/

/* HMAC KAT key - same as server-side EccPrivKey */
static const u8 HmacKatKey[XASU_HMAC_KAT_KEY_LEN_IN_BYTES] = {
	0x22U, 0x17U, 0x96U, 0x4FU, 0xB2U, 0x14U, 0x35U, 0x33U,
	0xBAU, 0x93U, 0xAAU, 0x35U, 0xFEU, 0x09U, 0x37U, 0xA6U,
	0x69U, 0x5EU, 0x20U, 0x87U, 0x27U, 0x07U, 0x06U, 0x44U,
	0x99U, 0x21U, 0x7CU, 0x5FU, 0x6AU, 0xB8U, 0x09U, 0xDFU
};

/* HMAC KAT message - same as server-side KatMessage */
static const u8 HmacKatMsg[XASU_HMAC_KAT_MSG_LEN_IN_BYTES] = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U,
	0xC3U, 0x5AU, 0x83U, 0xBCU, 0xE5U, 0x63U, 0x2CU, 0xE7U,
	0x89U, 0x43U, 0x6CU, 0x68U, 0x82U, 0xCAU, 0x1CU, 0x71U
};

/* Expected HMAC-SHA3-256 output for above key and message */
static const u8 ExpHmacSha3_256[XASU_HMAC_KAT_SHA3_256_HASH_LEN] = {
	0xFFU, 0x32U, 0x0AU, 0xF4U, 0x3BU, 0x15U, 0x34U, 0x12U,
	0xCDU, 0x51U, 0x64U, 0x38U, 0x38U, 0xFAU, 0x1BU, 0xD1U,
	0xBAU, 0x1EU, 0xABU, 0x78U, 0x92U, 0xB2U, 0x20U, 0x46U,
	0xD8U, 0x96U, 0xE0U, 0x25U, 0xCFU, 0x0AU, 0xE6U, 0x71U
};

/** Per-invocation KAT callback state, passed through CallBackRefPtr. */
typedef struct {
	volatile u8 Notify;		/**< Completion flag */
	volatile s32 CallBackStatus;	/**< Callback status */
} XAsu_HmacKatCbState;

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
 * 		- XASU_FREE_CTX_FAIL, if freeing context fails.
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

	/** Validate required parameters for HMAC update operation. */
	if (((HmacParamsPtr->OperationFlags & XASU_INIT) == XASU_INIT) &&
	    ((HmacParamsPtr->OperationFlags & XASU_FINISH) == XASU_FINISH) &&
	    ((HmacParamsPtr->OperationFlags & XASU_UPDATE) != XASU_UPDATE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Handle context operation. */
	Status = XAsu_HandleContextOperation(ClientParamsPtr, &P0HmacCtx, &P1HmacCtx,
				     HmacParamsPtr->OperationFlags, &UniqueId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Cleanup on FINAL operation. */
	if ((HmacParamsPtr->OperationFlags & XASU_FINISH) == XASU_FINISH) {
		Status = XAsu_CleanupFinishOperation(ClientParamsPtr, &P0HmacCtx, &P1HmacCtx,
						     UniqueId,
						     (u8 *)(UINTPTR)HmacParamsPtr->HmacAddr,
						     HmacParamsPtr->HmacLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** Get the command ID based on SHA type. */
	if (HmacParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CommandId = XASU_HMAC_COMPUTE_SHA2_CMD_ID;
	} else {
		CommandId = XASU_HMAC_COMPUTE_SHA3_CMD_ID;
	}
	/** Create command header. */
	Header = XAsu_CreateHeader(CommandId, UniqueId, XASU_MODULE_HMAC_ID,
				   (u8)(sizeof(XAsu_HmacParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamsPtr, HmacParamsPtr,
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
	Header = XAsu_CreateHeader(XASU_HMAC_KAT_CMD_ID, UniqueId, XASU_MODULE_HMAC_ID,
				   XASU_CMD_LEN_ZERO, ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Callback function for HMAC client-side KAT. Sets the notify flag and stores
 *		the completion status via per-invocation state.
 *
 * @param	CallBackRefPtr	Pointer to the caller's XAsu_HmacKatCbState on stack.
 * @param	Status		Completion status from the server.
 *
 *************************************************************************************************/
static void XAsu_HmacKatCallBack(void *CallBackRefPtr, u32 Status)
{
	XAsu_HmacKatCbState *State = (XAsu_HmacKatCbState *)CallBackRefPtr;

	State->CallBackStatus = (s32)Status;
	State->Notify = 1U;
}

/*************************************************************************************************/
/**
 * @brief	This function runs the client-side HMAC KAT using SHA3-256.
 *		It calls XAsu_HmacCompute with INIT|UPDATE|FINAL, busy-waits for the callback,
 *		and compares the output against the expected HMAC.
 *
 * @return
 *	- XST_SUCCESS, if HMAC SHA3-256 KAT passes.
 *	- XST_FAILURE, if HMAC computation or comparison fails.
 *
 *************************************************************************************************/
s32 XAsu_HmacSha3Kat(void)
{
	s32 Status = XST_FAILURE;
	s32 SStatus = XST_FAILURE;
	XAsu_ClientParams ClientParams = {0U};
	XAsu_HmacParams HmacParams = {0U};
	u8 HmacOutput[XASU_SHA_SHAKE_256_HASH_LEN] = {0U};
	XAsu_HmacKatCbState KatState = {0U, (s32)XST_FAILURE};

	/** Set up client parameters with KAT callback. */
	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr = XAsu_HmacKatCallBack;
	ClientParams.CallBackRefPtr = (void *)&KatState;

	/** Set up HMAC parameters for single-shot operation. */
	HmacParams.ShaType = XASU_SHA3_TYPE;
	HmacParams.ShaMode = XASU_SHA_MODE_256;
	HmacParams.OperationFlags = XASU_INIT | XASU_UPDATE | XASU_FINISH;
	HmacParams.IsLast = XASU_TRUE;
	HmacParams.KeyObject.KeyInAddr = (u64)(UINTPTR)HmacKatKey;
	HmacParams.KeyObject.KeyInLen = XASU_HMAC_KAT_KEY_LEN_IN_BYTES;
	HmacParams.KeyObject.KeyId = 0U;
	HmacParams.MsgBufferAddr = (u64)(UINTPTR)HmacKatMsg;
	HmacParams.MsgLen = XASU_HMAC_KAT_MSG_LEN_IN_BYTES;
	HmacParams.HmacAddr = (u64)(UINTPTR)HmacOutput;
	HmacParams.HmacLen = XASU_HMAC_KAT_SHA3_256_HASH_LEN;

	/** Send HMAC compute request to the server. */
	Status = XAsu_HmacCompute(&ClientParams, &HmacParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Busy-wait for the server response. */
	while (KatState.Notify == 0U) {
		/* Wait */
	}

	/** Check the callback status. */
	if (KatState.CallBackStatus != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	/** Compare the computed HMAC against the expected value. */
	Status = Xil_SMemCmp_CT(ExpHmacSha3_256,
		XASU_HMAC_KAT_SHA3_256_HASH_LEN, HmacOutput,
		XASU_HMAC_KAT_SHA3_256_HASH_LEN,
		XASU_HMAC_KAT_SHA3_256_HASH_LEN);

END:
	/** Zeroize local output buffer. */
	SStatus = Xil_SecureZeroize(HmacOutput, XASU_HMAC_KAT_SHA3_256_HASH_LEN);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

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
	     (XASU_INIT | XASU_UPDATE | XASU_FINISH)) == 0U) {
		goto END;
	}

	if (((HmacParamsPtr->OperationFlags & XASU_INIT) == XASU_INIT)) {
		/** Validate SHA Mode and SHA Type. */
		if (XAsu_ShaValidateModeAndType(HmacParamsPtr->ShaType, HmacParamsPtr->ShaMode) != XST_SUCCESS) {
			goto END;
		}

		if (XAsu_ShaValidateHashLen(HmacParamsPtr->ShaMode, HmacParamsPtr->HmacLen) != XST_SUCCESS) {
			goto END;
		}

		/** Validate that exactly one of KeyInAddr or KeyId is provided. */
		if (XAsu_KmValidateKeyAddrNdKeyId(HmacParamsPtr->KeyObject.KeyInAddr,
						  HmacParamsPtr->KeyObject.KeyId) != XST_SUCCESS) {
			goto END;
		}

		/** When a direct key is provided, its length must be valid. */
		if ((HmacParamsPtr->KeyObject.KeyInAddr != 0U) &&
		    ((HmacParamsPtr->KeyObject.KeyInLen == 0U) ||
		     (HmacParamsPtr->KeyObject.KeyInLen > XASU_HMAC_MAX_KEY_LENGTH))) {
			goto END;
		}
	}

	if ((HmacParamsPtr->OperationFlags & XASU_UPDATE) == XASU_UPDATE) {
		if ((HmacParamsPtr->IsLast != XASU_TRUE) && (HmacParamsPtr->IsLast != XASU_FALSE)) {
			goto END;
		}

		if ((HmacParamsPtr->MsgBufferAddr == 0U) ||
		    (HmacParamsPtr->MsgLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) {
			goto END;
		}
	}

	if ((((HmacParamsPtr->OperationFlags & XASU_FINISH) == XASU_FINISH) &&
	     (HmacParamsPtr->HmacAddr == 0U))) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
