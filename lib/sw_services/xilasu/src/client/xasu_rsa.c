/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       yog  01/28/26 Added RSA key ID validation in RSA operations.
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
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaEnc(XAsu_ClientParams *ClientParamPtr, XAsu_RsaParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RsaClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate InputDataAddr and that exactly one of KeyCompAddr or KeyId is provided. */
	Status = XAsu_RsaValidateInputParams(RsaClientParamPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->OutputDataAddr == 0U) ||
	    (RsaClientParamPtr->OutputLenAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->OutputDataLen < RsaClientParamPtr->KeySize) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaClientParamPtr->KeySize);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->Len > RsaClientParamPtr->KeySize) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
					(u8 *)(UINTPTR)RsaClientParamPtr->OutputLenAddr,
					XASU_RSA_OUTPUT_LEN_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_RSA_PUB_ENC_CMD_ID, UniqueId, XASU_MODULE_RSA_ID,
				   (u8)(sizeof(XAsu_RsaParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, RsaClientParamPtr,
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
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaDec(XAsu_ClientParams *ClientParamPtr, XAsu_RsaParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RsaClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate InputDataAddr and that exactly one of KeyCompAddr or KeyId is provided. */
	Status = XAsu_RsaValidateInputParams(RsaClientParamPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->OutputDataAddr == 0U) ||
	    (RsaClientParamPtr->OutputLenAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->OutputDataLen < RsaClientParamPtr->KeySize) {
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
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
					(u8 *)(UINTPTR)RsaClientParamPtr->OutputLenAddr,
					XASU_RSA_OUTPUT_LEN_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_RSA_PVT_DEC_CMD_ID, UniqueId, XASU_MODULE_RSA_ID,
				   (u8)(sizeof(XAsu_RsaParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, RsaClientParamPtr,
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
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
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

	/** Validate InputDataAddr and that exactly one of KeyCompAddr or KeyId is provided. */
	Status = XAsu_RsaValidateInputParams(RsaClientParamPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->OutputDataAddr == 0U) ||
	    (RsaClientParamPtr->OutputLenAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->OutputDataLen < RsaClientParamPtr->KeySize) {
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
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
					(u8 *)(UINTPTR)RsaClientParamPtr->OutputLenAddr,
					XASU_RSA_OUTPUT_LEN_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_RSA_PVT_CRT_DEC_CMD_ID, UniqueId, XASU_MODULE_RSA_ID,
				   (u8)(sizeof(XAsu_RsaParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, RsaClientParamPtr,
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
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
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

	/** Validations of inputs. */
	Status = XAsu_RsaValidateInputParams(&(RsaClientParamPtr->XAsu_RsaOpComp));
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->XAsu_RsaOpComp.OutputDataAddr == 0U) ||
	    (RsaClientParamPtr->OptionalLabelAddr == 0U) ||
	    (RsaClientParamPtr->XAsu_RsaOpComp.OutputLenAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->XAsu_RsaOpComp.OutputDataLen < RsaClientParamPtr->XAsu_RsaOpComp.KeySize) {
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

	if (XAsu_ShaValidateModeAndType(RsaClientParamPtr->ShaType, RsaClientParamPtr->ShaMode) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate Unique ID and register the callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
					(u8 *)(UINTPTR)RsaClientParamPtr->XAsu_RsaOpComp.OutputLenAddr,
					XASU_RSA_OUTPUT_LEN_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Set the command ID based on the selected SHA type. */
	if (RsaClientParamPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_RSA_OAEP_ENC_SHA2_CMD_ID;
	} else {
		CmdId = XASU_RSA_OAEP_ENC_SHA3_CMD_ID;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_RSA_ID,
				   (u8)(sizeof(XAsu_RsaOaepPaddingParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, RsaClientParamPtr,
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
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
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

	/** Validations of inputs. */
	Status = XAsu_RsaValidateInputParams(&(RsaClientParamPtr->XAsu_RsaOpComp));
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->XAsu_RsaOpComp.OutputDataAddr == 0U) ||
	    (RsaClientParamPtr->OptionalLabelAddr == 0U) ||
	    (RsaClientParamPtr->XAsu_RsaOpComp.OutputLenAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->XAsu_RsaOpComp.OutputDataLen == 0U) {
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

	if (XAsu_ShaValidateModeAndType(RsaClientParamPtr->ShaType, RsaClientParamPtr->ShaMode) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate unique ID and register the callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
					(u8 *)(UINTPTR)RsaClientParamPtr->XAsu_RsaOpComp.OutputLenAddr,
					XASU_RSA_OUTPUT_LEN_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Set the command ID based on the selected SHA type. */
	if (RsaClientParamPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_RSA_OAEP_DEC_SHA2_CMD_ID;
	} else {
		CmdId = XASU_RSA_OAEP_DEC_SHA3_CMD_ID;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_RSA_ID,
				   (u8)(sizeof(XAsu_RsaOaepPaddingParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, RsaClientParamPtr,
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
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
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

	/** Validations of inputs. */
	Status = XAsu_RsaValidateInputParams(&(RsaClientParamPtr->XAsu_RsaOpComp));
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->XAsu_RsaOpComp.OutputDataAddr == 0U) ||
	    (RsaClientParamPtr->XAsu_RsaOpComp.OutputLenAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (RsaClientParamPtr->XAsu_RsaOpComp.OutputDataLen < RsaClientParamPtr->XAsu_RsaOpComp.KeySize) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(RsaClientParamPtr->XAsu_RsaOpComp.KeySize);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_SHAKE_256_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_384_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_512_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_SHAKE_256_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHAKE_256_MAX_HASH_LEN) &&
	    (RsaClientParamPtr->InputDataType == XASU_RSA_HASHED_INPUT_DATA)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (XAsu_ShaValidateModeAndType(RsaClientParamPtr->ShaType, RsaClientParamPtr->ShaMode) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate unique ID and register the callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
					(u8 *)(UINTPTR)RsaClientParamPtr->XAsu_RsaOpComp.OutputLenAddr,
					XASU_RSA_OUTPUT_LEN_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	if (RsaClientParamPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_RSA_PSS_SIGN_GEN_SHA2_CMD_ID;
	} else{
		CmdId = XASU_RSA_PSS_SIGN_GEN_SHA3_CMD_ID;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_RSA_ID,
				   (u8)(sizeof(XAsu_RsaPaddingParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, RsaClientParamPtr,
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
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
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

	/** Validations of inputs. */
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

	if ((RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_SHAKE_256_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_384_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_512_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHA_SHAKE_256_HASH_LEN) &&
	    (RsaClientParamPtr->XAsu_RsaOpComp.Len != XASU_SHAKE_256_MAX_HASH_LEN) &&
	    (RsaClientParamPtr->InputDataType == XASU_RSA_HASHED_INPUT_DATA)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((RsaClientParamPtr->SignatureLen != RsaClientParamPtr->XAsu_RsaOpComp.KeySize)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (XAsu_ShaValidateModeAndType(RsaClientParamPtr->ShaType, RsaClientParamPtr->ShaMode) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate unique ID and register the callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Select the command ID based on SHA type. */
	if (RsaClientParamPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_RSA_PSS_SIGN_VER_SHA2_CMD_ID;
	} else {
		CmdId = XASU_RSA_PSS_SIGN_VER_SHA3_CMD_ID;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_RSA_ID,
				   (u8)(sizeof(XAsu_RsaPaddingParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, RsaClientParamPtr,
					(u32)(sizeof(XAsu_RsaPaddingParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform RSA OAEP encrypt and decrypt
 * 		Known Answer Tests (KAT's).
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the
 * 				client input parameters.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaOaepEncDecKat(XAsu_ClientParams *ClientParamPtr)
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
	Header = XAsu_CreateHeader(XASU_RSA_OAEP_ENC_DEC_KAT_CMD_ID, UniqueId,
				   XASU_MODULE_RSA_ID, XASU_CMD_LEN_ZERO,
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to perform RSA PSS sign generation and
 * 		verification Known Answer Tests (KAT's).
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the
 * 				client input parameters.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaPssSignGenAndVerKat(XAsu_ClientParams *ClientParamPtr)
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
	Header = XAsu_CreateHeader(XASU_RSA_PSS_SIGN_GEN_VER_KAT_CMD_ID, UniqueId,
				   XASU_MODULE_RSA_ID, XASU_CMD_LEN_ZERO,
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}

/** @} */
