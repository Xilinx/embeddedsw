/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_lms.c
 *
 * This file contains the implementation of the client interface functions for
 * LMS signature verification module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   01/21/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_lms_client_apis LMS Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_lms.h"
#include "xasu_def.h"
#include "xasu_shainfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_LmsValidateParams(const XAsu_LmsHssSignVerifyParams *LmsParamsPtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform LMS signature verification
 *		on the provided message using the given LMS public key.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	LmsParamsPtr	Pointer to XAsu_LmsHssSignVerifyParams structure which holds the
 *				parameters for LMS signature verification.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_QUEUE_FULL, if Queue buffer is full.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 *
 *************************************************************************************************/
s32 XAsu_LmsSignVerify(XAsu_ClientParams *ClientParamsPtr,
		XAsu_LmsHssSignVerifyParams *LmsParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 CmdId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_LmsValidateParams(LmsParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate SHA type and mode. */
	if (XAsu_ShaValidateModeAndType(LmsParamsPtr->ShaType, LmsParamsPtr->ShaMode) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Set the command ID based on the selected SHA type. */
	if (LmsParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_LMS_SIGN_VERIFY_SHA2_CMD_ID;
	} else {
		CmdId = XASU_LMS_SIGN_VERIFY_SHA3_CMD_ID;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_LMS_ID,
				   (u8)(sizeof(XAsu_LmsHssSignVerifyParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamsPtr, LmsParamsPtr,
				sizeof(XAsu_LmsHssSignVerifyParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform HSS (Hierarchical Signature
 *		System) signature verification on the provided message using the given HSS
 *		public key.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	HssParamsPtr	Pointer to XAsu_LmsHssSignVerifyParams structure which holds the
 *				parameters for HSS signature verification.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_QUEUE_FULL, if Queue buffer is full.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 *
 *************************************************************************************************/
s32 XAsu_HssSignVerify(XAsu_ClientParams *ClientParamsPtr,
		XAsu_LmsHssSignVerifyParams *HssParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 CmdId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_LmsValidateParams(HssParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate SHA type and mode. */
	if (XAsu_ShaValidateModeAndType(HssParamsPtr->ShaType, HssParamsPtr->ShaMode) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Set the command ID based on the selected SHA type. */
	if (HssParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_HSS_SIGN_VERIFY_SHA2_CMD_ID;
	} else {
		CmdId = XASU_HSS_SIGN_VERIFY_SHA3_CMD_ID;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_LMS_ID,
				   (u8)(sizeof(XAsu_LmsHssSignVerifyParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamsPtr, HssParamsPtr,
				sizeof(XAsu_LmsHssSignVerifyParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to perform LMS Known Answer Tests (KAT's).
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_QUEUE_FULL, if Queue buffer is full.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 *
 *************************************************************************************************/
s32 XAsu_LmsKat(XAsu_ClientParams *ClientParamsPtr)
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
	Header = XAsu_CreateHeader(XASU_LMS_KAT_CMD_ID, UniqueId, XASU_MODULE_LMS_ID,
				   XASU_CMD_LEN_ZERO, ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the LMS signature verification parameters.
 *
 * @param	LmsParamsPtr	Pointer to the XAsu_LmsHssSignVerifyParams structure which holds
 *				the LMS signature verification parameters.
 *
 * @return
 *		- XST_SUCCESS, if validation of LMS parameters is successful.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *
 *************************************************************************************************/
static s32 XAsu_LmsValidateParams(const XAsu_LmsHssSignVerifyParams *LmsParamsPtr)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	/** Validate LMS params pointer. */
	if (LmsParamsPtr == NULL) {
		goto END;
	}

	/** Validate message address. */
	if (LmsParamsPtr->MsgAddr == 0U) {
		goto END;
	}

	/** Validate signature address and length. */
	if ((LmsParamsPtr->SignatureAddr == 0U) || (LmsParamsPtr->SignatureLen == 0U)) {
		goto END;
	}

	/** Validate public key address and length. */
	if ((LmsParamsPtr->PublicKeyAddr == 0U) || (LmsParamsPtr->PublicKeyLen == 0U)) {
		goto END;
	}

	/**
	 * Validate message length - if message is not pre-hashed, length should be non-zero.
	 * If message is pre-hashed, length should be the hash length (typically 32 bytes for SHA-256).
	 */
	if ((LmsParamsPtr->PreHashedMsg == XASU_LMS_MSG_NOT_PREHASHED) &&
			(LmsParamsPtr->MsgLen == 0U)) {
		goto END;
	}

	/** Validate message length doesn't exceed DMA max transfer length. */
	if (LmsParamsPtr->MsgLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) {
		goto END;
	}

	/** Validate pre-hashed message flag. */
	if ((LmsParamsPtr->PreHashedMsg != XASU_LMS_MSG_NOT_PREHASHED) &&
			(LmsParamsPtr->PreHashedMsg != XASU_LMS_MSG_PREHASHED)) {
		goto END;
	}

	/** Validate public key length is at least the minimum LMS public key size. */
	if (LmsParamsPtr->PublicKeyLen < XASU_LMS_PUB_KEY_SIZE) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
