/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_mlkem.c
 *
 * This file contains the implementation of the client interface functions for the ML-KEM
 * (FIPS 203) module. Each API validates the client-supplied parameters, registers a completion
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
 * @addtogroup xasu_mlkem_client_apis ML-KEM Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_mlkem.h"
#include "xasu_def.h"
#include "xasu_mlkem_common.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform ML-KEM encapsulation using the
 *		provided encapsulation (public) key, producing a ciphertext and a shared secret.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	MlkemParamPtr	Pointer to the XAsu_MlkemEncapsDecapsParams structure which holds the
 *				ML-KEM encapsulation parameters.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_MlkemEncapsulate(XAsu_ClientParams *ClientParamPtr,
			  XAsu_MlkemEncapsDecapsParams *MlkemParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate ML-KEM encapsulation parameters. */
	Status = XAsu_MlkemValidateParams(MlkemParamPtr);
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
	Header = XAsu_CreateHeader(XASU_MLKEM_ENCAPS_CMD_ID, UniqueId, XASU_MODULE_MLKEM_ID,
				   (u8)(sizeof(XAsu_MlkemEncapsDecapsParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, MlkemParamPtr,
				   (u32)(sizeof(XAsu_MlkemEncapsDecapsParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform ML-KEM encapsulation with the
 *		message m supplied by the caller instead of drawn from the TRNG, so that the ACVP
 *		encapsulation vectors can be driven through the module. FIPS 203 requires an
 *		approved ML-KEM.Encaps to draw m from an approved RBG, so only an ASUFW built with
 *		CAVP support serves this command; any other build rejects it as an invalid command.
 *		The encapsulation key must be passed by address, as the key vault is not available
 *		to this command.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	MlkemParamPtr	Pointer to the XAsu_MlkemEncapsDerandParams structure which holds
 *				the ML-KEM encapsulation parameters and the message m.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_MlkemEncapsulateDerand(XAsu_ClientParams *ClientParamPtr,
				XAsu_MlkemEncapsDerandParams *MlkemParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate ML-KEM encapsulation parameters and the supplied message m. */
	Status = XAsu_MlkemValidateEncapsDerandParams(MlkemParamPtr);
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
	Header = XAsu_CreateHeader(XASU_MLKEM_ENCAPS_DERAND_CMD_ID, UniqueId, XASU_MODULE_MLKEM_ID,
				   (u8)(sizeof(XAsu_MlkemEncapsDerandParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, MlkemParamPtr,
				   (u32)(sizeof(XAsu_MlkemEncapsDerandParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform ML-KEM decapsulation using the
 *		provided decapsulation (secret) key and ciphertext, producing a shared secret.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	MlkemParamPtr	Pointer to the XAsu_MlkemEncapsDecapsParams structure which holds the
 *				ML-KEM decapsulation parameters.
 *
 * @return
 *		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if the unique ID is invalid.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 * @note	Per FIPS 203, a ciphertext that does not match the decapsulation key yields a
 *		pseudorandom shared secret rather than an error, so that decapsulation gives an
 *		attacker no way to test ciphertexts. A successful decapsulation therefore does not
 *		mean the ciphertext was genuine. Callers must establish that through key
 *		confirmation on the derived shared secret, never by expecting a failure here.
 *
 *************************************************************************************************/
s32 XAsu_MlkemDecapsulate(XAsu_ClientParams *ClientParamPtr,
			  XAsu_MlkemEncapsDecapsParams *MlkemParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate ML-KEM decapsulation parameters. */
	Status = XAsu_MlkemValidateParams(MlkemParamPtr);
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
	Header = XAsu_CreateHeader(XASU_MLKEM_DECAPS_CMD_ID, UniqueId, XASU_MODULE_MLKEM_ID,
				   (u8)(sizeof(XAsu_MlkemEncapsDecapsParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, MlkemParamPtr,
				   (u32)(sizeof(XAsu_MlkemEncapsDecapsParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to run the ML-KEM Known Answer Test, which
 *		encapsulates to and then decapsulates with a fixed NIST ACVP ML-KEM-512 vector on
 *		ASUFW and checks that the shared secrets match. Key generation is not part of it:
 *		it carries its own pairwise consistency test instead.
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
s32 XAsu_MlkemKat(XAsu_ClientParams *ClientParamPtr)
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
	Header = XAsu_CreateHeader(XASU_MLKEM_KAT_CMD_ID, UniqueId, XASU_MODULE_MLKEM_ID,
				   XASU_CMD_LEN_ZERO, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}

/** @} */
