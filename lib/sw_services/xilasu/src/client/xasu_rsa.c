/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_rsa.c
 *
 * This file contains the implementation of the client interface functions for
 * RSA driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/24 Initial release
 *       ss   09/26/24 Fixed doxygen comments
 *       ss   02/04/25 Added client API's for RSA padding scheme
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_rsa_client_apis RSA Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_rsa.h"
#include "xasu_def.h"
#include "xasu_status.h"
#include "xasu_shainfo.h"
#include "xasu_rsa_common.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform RSA encryption on the provided
 * 		data by using specified	public key.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	RsaClientParamPtr	Pointer to XAsu_RsaParams structure which holds the
 *					parameters of RSA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaEnc(XAsu_ClientParams *ClientParamPtr, XAsu_RsaParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validatations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RsaClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (((RsaClientParamPtr->InputDataAddr == 0U) ||
	     (RsaClientParamPtr->OutputDataAddr == 0U) ||
	     (RsaClientParamPtr->KeyCompAddr == 0U))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaClientParamPtr->KeySize);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->Len != RsaClientParamPtr->KeySize) {
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
	Header = XAsu_CreateHeader(XASU_RSA_PUB_ENC_CMD_ID, UniqueId, XASU_MODULE_RSA_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, RsaClientParamPtr,
					(u32)(sizeof(XAsu_RsaParams)), Header);

END:
	return Status;
}
/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform RSA decryption on the provided
 * 	data by using specified	private key.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	RsaClientParamPtr	Pointer to XAsu_RsaParams structure which holds the
 *					parameters of RSA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaDec(XAsu_ClientParams *ClientParamPtr, XAsu_RsaParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validatations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RsaClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (((RsaClientParamPtr->InputDataAddr == 0U) ||
	     (RsaClientParamPtr->OutputDataAddr == 0U) ||
	     (RsaClientParamPtr->KeyCompAddr == 0U))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaClientParamPtr->KeySize);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->Len != RsaClientParamPtr->KeySize) {
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
	Header = XAsu_CreateHeader(XASU_RSA_PVT_DEC_CMD_ID, UniqueId, XASU_MODULE_RSA_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, RsaClientParamPtr,
					(u32)(sizeof(XAsu_RsaParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform RSA decryption using CRT algorithm
 * 	for the provided message by using specified private key.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	RsaClientParamPtr	Pointer to XAsu_RsaParams structure which holds the
 *					parameters of RSA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaCrtDec(XAsu_ClientParams *ClientParamPtr, XAsu_RsaParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RsaClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validatations of inputs. */
	if (((RsaClientParamPtr->InputDataAddr == 0U) ||
	     (RsaClientParamPtr->OutputDataAddr == 0U) ||
	     (RsaClientParamPtr->KeyCompAddr == 0U))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaClientParamPtr->KeySize);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->Len != RsaClientParamPtr->KeySize) {
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
	Header = XAsu_CreateHeader(XASU_RSA_PVT_CRT_DEC_CMD_ID, UniqueId, XASU_MODULE_RSA_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, RsaClientParamPtr,
					(u32)(sizeof(XAsu_RsaParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform encoding using
 * 		RSA OAEP padding algorithm by using the specified RSA key.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	RsaClientParamPtr	Pointer to XAsu_RsaOaepPaddingParams structure which holds
 * 					the parameters of RSA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaOaepEnc(XAsu_ClientParams *ClientParamPtr,
			XAsu_RsaOaepPaddingParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 CmdId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RsaClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validatations of inputs. */
	Status = XAsu_RsaValidateInputParams(&(RsaClientParamPtr->XAsu_RsaOpComp));
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->XAsu_RsaOpComp.OutputDataAddr == 0U) ||
	    (RsaClientParamPtr->OptionalLabelAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaClientParamPtr->XAsu_RsaOpComp.KeySize);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->XAsu_RsaOpComp.Len > RsaClientParamPtr->XAsu_RsaOpComp.KeySize) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Set the command ID based on the selected SHA type. */
	if (RsaClientParamPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_RSA_OAEP_ENC_SHA2_CMD_ID;
	} else if (RsaClientParamPtr->ShaType == XASU_SHA3_TYPE) {
		CmdId = XASU_RSA_OAEP_ENC_SHA3_CMD_ID;
	} else {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA256) &&
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA384) &&
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA512) &&
	    ((RsaClientParamPtr->ShaType != XASU_SHA3_TYPE) ||
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHAKE256))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate Unique ID and register the callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_RSA_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, RsaClientParamPtr,
					(u32)(sizeof(XAsu_RsaOaepPaddingParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform decoding using
 * 		RSA OAEP padding algorithm of the provided message by using the specified key.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	RsaClientParamPtr	Pointer to XAsu_RsaOaepPaddingParams structure which holds
 * 					the parameters of RSA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaOaepDec(XAsu_ClientParams *ClientParamPtr,
			XAsu_RsaOaepPaddingParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 CmdId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RsaClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validatations of inputs. */
	Status = XAsu_RsaValidateInputParams(&(RsaClientParamPtr->XAsu_RsaOpComp));
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->XAsu_RsaOpComp.OutputDataAddr == 0U) ||
	    (RsaClientParamPtr->OptionalLabelAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->OptionalLabelAddr == 0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaClientParamPtr->XAsu_RsaOpComp.KeySize);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->XAsu_RsaOpComp.Len > RsaClientParamPtr->XAsu_RsaOpComp.KeySize) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Set the command ID based on selected SHA type. */
	if (RsaClientParamPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_RSA_OAEP_DEC_SHA2_CMD_ID;
	} else if (RsaClientParamPtr->ShaType == XASU_SHA3_TYPE) {
		CmdId = XASU_RSA_OAEP_DEC_SHA3_CMD_ID;
	} else {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA256) &&
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA384) &&
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA512) &&
	    ((RsaClientParamPtr->ShaType != XASU_SHA3_TYPE) ||
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHAKE256))) {
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
	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_RSA_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, RsaClientParamPtr,
					(u32)(sizeof(XAsu_RsaOaepPaddingParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to generate signature using
 * 		RSA PSS padding on provided message by using the specified key.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	RsaClientParamPtr	Pointer to XAsu_RsaPaddingParams structure which holds the
 *					parameters of RSA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaPssSignGen(XAsu_ClientParams *ClientParamPtr, XAsu_RsaPaddingParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 CmdId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RsaClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validatations of inputs. */
	Status = XAsu_RsaValidateInputParams(&(RsaClientParamPtr->XAsu_RsaOpComp));
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->XAsu_RsaOpComp.OutputDataAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaClientParamPtr->XAsu_RsaOpComp.KeySize);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_256_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_384_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_512_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHAKE_256_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHAKE_256_MAX_HASH_LEN) &&
	    (RsaClientParamPtr->InputDataType == XASU_RSA_HASHED_INPUT_DATA)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_RSA_PSS_SIGN_GEN_SHA2_CMD_ID;
	} else if (RsaClientParamPtr->ShaType == XASU_SHA3_TYPE) {
		CmdId = XASU_RSA_PSS_SIGN_GEN_SHA3_CMD_ID;
	} else {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA256) &&
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA384) &&
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA512) &&
	    ((RsaClientParamPtr->ShaType != XASU_SHA3_TYPE) ||
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHAKE256))) {
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
	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_RSA_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, RsaClientParamPtr,
					(u32)(sizeof(XAsu_RsaPaddingParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends request to ASUFW to verify the specified signature by
 * 		using RSA PSS padding by using the provided key.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	RsaClientParamPtr	Pointer to XAsu_RsaPaddingParams structure which holds the
 *					parameters of RSA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaPssSignVer(XAsu_ClientParams *ClientParamPtr, XAsu_RsaPaddingParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 CmdId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RsaClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validatations of inputs. */
	Status = XAsu_RsaValidateInputParams(&(RsaClientParamPtr->XAsu_RsaOpComp));
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->SignatureDataAddr == 0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaClientParamPtr->XAsu_RsaOpComp.KeySize);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_256_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_384_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_512_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHAKE_256_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHAKE_256_MAX_HASH_LEN) &&
	    (RsaClientParamPtr->InputDataType == XASU_RSA_HASHED_INPUT_DATA)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->SignatureLen != RsaClientParamPtr->XAsu_RsaOpComp.KeySize)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Select the command ID based on SHA type. */
	if (RsaClientParamPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_RSA_PSS_SIGN_VER_SHA2_CMD_ID;
	} else if (RsaClientParamPtr->ShaType == XASU_SHA3_TYPE) {
		CmdId = XASU_RSA_PSS_SIGN_VER_SHA3_CMD_ID;
	} else {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA256) &&
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA384) &&
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHA512) &&
	    ((RsaClientParamPtr->ShaType != XASU_SHA3_TYPE) ||
	    (RsaClientParamPtr->ShaMode != XASU_SHA_MODE_SHAKE256))) {
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
	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_RSA_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, RsaClientParamPtr,
					(u32)(sizeof(XAsu_RsaPaddingParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform RSA Known Answer Tests (KAT's).
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaKat(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate unique ID and register callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}
	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_RSA_KAT_CMD_ID, UniqueId, XASU_MODULE_RSA_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}
/** @} */