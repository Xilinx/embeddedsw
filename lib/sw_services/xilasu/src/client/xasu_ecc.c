/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ecc.c
 *
 * This file contains the implementation of the client interface functions for
 * ECC driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  08/19/24 Initial release
 *       yog  09/26/24 Added doxygen grouping and fixed doxygen comments.
 *       yog  11/27/24 Included input parameters validation.
 *       ss   12/02/24 Added support for ECDH
 *       yog  02/21/25 Changed XAsu_EccValidateCurveInfo() API to be non-static
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_ecc_client_apis ECC Client APIs
 * @{
*/

/*************************************** Include Files *******************************************/
#include "xasu_ecc.h"
#include "xasu_def.h"
#include "xasu_status.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_ValidateEccParameters(XAsu_EccParams *EccParamsPtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function generates an ECDSA signature for the provided hash by using the
 * 		given private key associated with the elliptic curve.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	EccParamsPtr	Pointer to XAsu_EccParams structure which holds the parameters of
 * 				ECC input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_EccGenSign(XAsu_ClientParams *ClientParamsPtr, XAsu_EccParams *EccParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_ValidateEccParameters(EccParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_ECC_GEN_SIGNATURE_CMD_ID, UniqueId, XASU_MODULE_ECC_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, EccParamsPtr,
					sizeof(XAsu_EccParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function verifies the validity of an ECDSA signature for the provided hash
 * 		using the provided ecc public key.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	EccParamsPtr	Pointer to XAsu_EccParams structure which holds the parameters of
 * 				ECC input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_EccVerifySign(XAsu_ClientParams *ClientParamsPtr, XAsu_EccParams *EccParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/* Validatations of inputs */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_ValidateEccParameters(EccParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_ECC_VERIFY_SIGNATURE_CMD_ID, UniqueId, XASU_MODULE_ECC_ID,
					0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, EccParamsPtr,
					sizeof(XAsu_EccParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs ECC Known Answer Tests (KAT's).
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_EccKat(XAsu_ClientParams *ClientParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_ECC_KAT_CMD_ID, UniqueId, XASU_MODULE_ECC_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates an ECDH shared secret by using the given public key and
 * 		given private key associated with the elliptic curve.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters
 * @param	EcdhParamsPtr	Pointer to XAsu_EcdhParams structure which holds the parameters of
 * 				ECDH input arguments.
 *
 * @return
 *		- XST_SUCCESS, if signature generated successfully
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid
 *		- XASU_INVALID_UNIQUE_ID, if unique ID is invalid
 *		- XST_FAILURE, if send IPI fails
 *
 *************************************************************************************************/
s32 XAsu_EcdhGenSharedSecret(XAsu_ClientParams *ClientParamsPtr, XAsu_EcdhParams *EcdhParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (EcdhParamsPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (XAsu_EccValidateCurveInfo(EcdhParamsPtr->CurveType, EcdhParamsPtr->KeyLen) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_ECDH_SHARED_SECRET_CMD_ID, UniqueId, XASU_MODULE_ECC_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, EcdhParamsPtr,
					sizeof(XAsu_EcdhParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs ECDH Known Answer Tests (KAT's).
 *
 * @param	ClientParamsPtr	Pointer to client params structure.
 *
 * @return
 *		- XST_SUCCESS, if ECDH Kat runs succefully
 *		- XASU_INVALID_UNIQUE_ID, if unique ID is invalid
 *		- XST_FAILURE, if send IPI fails
 *
 *************************************************************************************************/
s32 XAsu_EcdhKat(XAsu_ClientParams *ClientParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_ECDH_KAT_CMD_ID, UniqueId, XASU_MODULE_ECC_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the given curve type and curve length.
 *
 * @param	CurveType	Curve type provided.
 * @param	CurveLen	Curve length provided.
 *
 * @return
 * 		- XST_SUCCESS, if curve type is valid.
 * 		- XST_FAILURE, if curve type is invalid.
 *
 *************************************************************************************************/
s32 XAsu_EccValidateCurveInfo(u32 CurveType, u32 CurveLen)
{
	s32 Status = XST_FAILURE;
	u32 Len = 0U;

	switch (CurveType) {
		case XASU_ECC_NIST_P192:
			Len = XASU_ECC_P192_SIZE_IN_BYTES;
			break;
		case XASU_ECC_NIST_P224:
			Len = XASU_ECC_P224_SIZE_IN_BYTES;
			break;
		case XASU_ECC_NIST_P256:
		case XASU_ECC_BRAINPOOL_P256:
			Len = XASU_ECC_P256_SIZE_IN_BYTES;
			break;
		case XASU_ECC_BRAINPOOL_P320:
			Len = XASU_ECC_P320_SIZE_IN_BYTES;
			break;
		case XASU_ECC_NIST_P384:
		case XASU_ECC_BRAINPOOL_P384:
			Len = XASU_ECC_P384_SIZE_IN_BYTES;
			break;
		case XASU_ECC_BRAINPOOL_P512:
			Len = XASU_ECC_P512_SIZE_IN_BYTES;
			break;
		case XASU_ECC_NIST_P521:
			Len = XASU_ECC_P521_SIZE_IN_BYTES;
			break;
		default:
			Status = XASU_INVALID_ARGUMENT;
			break;
	}

	if ((Status != XASU_INVALID_ARGUMENT) && (CurveLen == Len)) {
		Status = XST_SUCCESS;
	}

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
 * 	- XST_SUCCESS upon successful validation.
 * 	- XASU_INVALID_ARGUMENT, upon invalid arguments.
 *
 *************************************************************************************************/
static s32 XAsu_ValidateEccParameters(XAsu_EccParams *EccParamsPtr)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	if (EccParamsPtr == NULL) {
		goto END;
	}

	if ((EccParamsPtr->DigestAddr == 0U) || (EccParamsPtr->KeyAddr == 0U) ||
		(EccParamsPtr->SignAddr == 0U) || (EccParamsPtr->KeyLen != EccParamsPtr->DigestLen)) {
		goto END;
	}

	if (XAsu_EccValidateCurveInfo(EccParamsPtr->CurveType, EccParamsPtr->KeyLen)!= XST_SUCCESS) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
