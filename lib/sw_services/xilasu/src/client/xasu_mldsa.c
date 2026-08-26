/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_mldsa.c
 *
 * This file contains the implementation of the client interface functions for the ML-DSA
 * (FIPS 204) module. Each API validates the client-supplied parameters, registers a completion
 * callback, builds the command header and forwards the request to ASUFW over IPI.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_mldsa_client_apis ML-DSA Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_mldsa.h"
#include "xasu_def.h"
#include "xasu_mldsa_common.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to generate an ML-DSA signature over the
 *		provided message (pure ML-DSA or HashML-DSA, hedged or deterministic, with an
 *		optional context string).
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	MldsaParamPtr	Pointer to the XAsu_MldsaSignVerifyParams structure which holds the
 *				ML-DSA signature generation parameters.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_MldsaSign(XAsu_ClientParams *ClientParamPtr, XAsu_MldsaSignVerifyParams *MldsaParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate ML-DSA signature generation parameters. */
	Status = XAsu_MldsaValidateParams(MldsaParamPtr);
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
	Header = XAsu_CreateHeader(XASU_MLDSA_SIGN_CMD_ID, UniqueId, XASU_MODULE_MLDSA_ID,
				   (u8)(sizeof(XAsu_MldsaSignVerifyParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, MldsaParamPtr,
				   (u32)(sizeof(XAsu_MldsaSignVerifyParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to generate an ML-DSA signature with the
 *		signing randomness rnd supplied by the caller instead of drawn from the TRNG, so
 *		that the ACVP hedged signature vectors can be driven through the module. Hedged
 *		signing draws rnd from an approved RBG, so only an ASUFW built with CAVP support
 *		serves this command; any other build rejects it as an invalid command. The secret
 *		key must be passed by address, as the key vault is not available to this command,
 *		and the Deterministic field is ignored because the supplied rnd decides the
 *		randomness outright.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	MldsaParamPtr	Pointer to the XAsu_MldsaSignDerandParams structure which holds the
 *				ML-DSA signature generation parameters and the rnd to sign with.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_MldsaSignDerand(XAsu_ClientParams *ClientParamPtr,
			 XAsu_MldsaSignDerandParams *MldsaParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate ML-DSA signature generation parameters and the supplied rnd. */
	Status = XAsu_MldsaValidateSignDerandParams(MldsaParamPtr);
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
	Header = XAsu_CreateHeader(XASU_MLDSA_SIGN_DERAND_CMD_ID, UniqueId, XASU_MODULE_MLDSA_ID,
				   (u8)(sizeof(XAsu_MldsaSignDerandParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, MldsaParamPtr,
				   (u32)(sizeof(XAsu_MldsaSignDerandParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to verify an ML-DSA signature over the
 *		provided message.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	MldsaParamPtr	Pointer to the XAsu_MldsaSignVerifyParams structure which holds the
 *				ML-DSA signature verification parameters.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_MldsaVerify(XAsu_ClientParams *ClientParamPtr, XAsu_MldsaSignVerifyParams *MldsaParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate ML-DSA signature verification parameters. */
	Status = XAsu_MldsaValidateParams(MldsaParamPtr);
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
	Header = XAsu_CreateHeader(XASU_MLDSA_VERIFY_CMD_ID, UniqueId, XASU_MODULE_MLDSA_ID,
				   (u8)(sizeof(XAsu_MldsaSignVerifyParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, MldsaParamPtr,
				   (u32)(sizeof(XAsu_MldsaSignVerifyParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to run the ML-DSA Known Answer Test, which
 *		signs and then verifies a fixed NIST ACVP ML-DSA-44 vector on ASUFW. Key generation
 *		is not part of it: it carries its own pairwise consistency test instead.
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
s32 XAsu_MldsaKat(XAsu_ClientParams *ClientParamPtr)
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
	Header = XAsu_CreateHeader(XASU_MLDSA_KAT_CMD_ID, UniqueId, XASU_MODULE_MLDSA_ID,
				   XASU_CMD_LEN_ZERO, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}

/** @} */
