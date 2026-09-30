/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_slhdsa.c
 *
 * This file contains the implementation of the client interface functions for the SLH-DSA
 * (FIPS 205) module. Each API validates the client-supplied parameters, registers a completion
 * callback, builds the command header and forwards the request to ASUFW over IPI.
 *
 * Key pair generation is not an SLH-DSA module command: it is requested through the key manager,
 * using XAsu_KmGenerateSlhdsaKeyPair. All 64-bit addresses in the parameter structures must
 * point to memory visible to both the client and ASUFW, and those buffers must remain valid
 * until the registered callback reports completion.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   kk   07/10/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_slhdsa_client_apis SLH-DSA Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_slhdsa.h"
#include "xasu_def.h"
#include "xasu_slhdsa_common.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to sign a message using an SLH-DSA secret
 *		key.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters. SecureFlag must be XASU_CMD_SECURE.
 * @param	SlhdsaParamPtr	Pointer to the XAsu_SlhdsaSignParams structure which holds the
 *				SLH-DSA signature generation parameters. SignMode defaults to
 *				XASU_SLHDSA_SIGN_MODE_HEDGED, which draws the randomizer from the
 *				TRNG on ASUFW.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 * @note	Signing takes seconds to tens of seconds, so the caller must keep every buffer
 *		valid until the callback reports completion. CtxLen must match the value used
 *		during verification.
 *
 *************************************************************************************************/
s32 XAsu_SlhdsaSign(XAsu_ClientParams *ClientParamPtr, XAsu_SlhdsaSignParams *SlhdsaParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate SLH-DSA signature generation parameters. */
	Status = XAsu_SlhdsaValidateSignParams(SlhdsaParamPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_SLHDSA_SIGN_CMD_ID, UniqueId, XASU_MODULE_SLHDSA_ID,
				   (u8)(sizeof(XAsu_SlhdsaSignParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, SlhdsaParamPtr,
				   (u32)(sizeof(XAsu_SlhdsaSignParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to verify an SLH-DSA signature over the
 *		provided message.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	SlhdsaParamPtr	Pointer to the XAsu_SlhdsaSignVerifyParams structure which holds
 *				the SLH-DSA signature verification parameters.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 * @note	Success here only means the request was queued; the verdict arrives as
 *		XASU_SLHDSA_SIGNATURE_VERIFIED in AdditionalStatus via the callback. CtxLen and
 *		CtxAddr must match the values used when the signature was created.
 *
 *************************************************************************************************/
s32 XAsu_SlhdsaSignVerify(XAsu_ClientParams *ClientParamPtr,
			  XAsu_SlhdsaSignVerifyParams *SlhdsaParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate SLH-DSA signature verification parameters. */
	Status = XAsu_SlhdsaValidateSignVerifyParams(SlhdsaParamPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_SLHDSA_SIGN_VERIFY_CMD_ID, UniqueId, XASU_MODULE_SLHDSA_ID,
				   (u8)(sizeof(XAsu_SlhdsaSignVerifyParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, SlhdsaParamPtr,
				   (u32)(sizeof(XAsu_SlhdsaSignVerifyParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to run the SLH-DSA Known Answer Test, which
 *		verifies one ACVP SLH-DSA-SHAKE-128s signature (pure mode, empty context).
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_SlhdsaKat(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_SLHDSA_KAT_CMD_ID, UniqueId, XASU_MODULE_SLHDSA_ID,
				   XASU_CMD_LEN_ZERO, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}

/** @} */
