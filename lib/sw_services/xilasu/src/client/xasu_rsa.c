/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function performs RSA encryption for the provided message by using specified
 * 		public key.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	RsaClientParamPtr	Pointer to XAsu_RsaClientParams structure which holds the
 *					parameters of RSA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaEnc(XAsu_ClientParams *ClientParamPtr, XAsu_RsaClientParams *RsaClientParamPtr)
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
	     (RsaClientParamPtr->KeyCompAddr == 0U)) || ((RsaClientParamPtr->Len !=
		     XRSA_2048_KEY_SIZE) && (RsaClientParamPtr->Len != XRSA_3072_KEY_SIZE) &&
		     (RsaClientParamPtr->Len != XRSA_4096_KEY_SIZE))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_RSA_PUB_ENC_CMD_ID, UniqueId, XASU_MODULE_RSA_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, RsaClientParamPtr,
					sizeof(XAsu_RsaClientParams), Header);

END:
	return Status;
}
/*************************************************************************************************/
/**
 * @brief	This function performs RSA decryption for the provided message by using specified
 * 		private key.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	RsaClientParamPtr	Pointer to XAsu_RsaClientParams structure which holds the
 *					parameters of RSA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaDec(XAsu_ClientParams *ClientParamPtr, XAsu_RsaClientParams *RsaClientParamPtr)
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
	     (RsaClientParamPtr->KeyCompAddr == 0U)) || ((RsaClientParamPtr->Len !=
		     XRSA_2048_KEY_SIZE) && (RsaClientParamPtr->Len != XRSA_3072_KEY_SIZE) &&
		     (RsaClientParamPtr->Len != XRSA_4096_KEY_SIZE))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_RSA_PVT_DEC_CMD_ID, UniqueId, XASU_MODULE_RSA_ID, 0U);

	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, RsaClientParamPtr,
					sizeof(XAsu_RsaClientParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA decryption using CRT algorithm for the provided
 * 		message by using specified private key.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 * 					client input parameters.
 * @param	RsaClientParamPtr	Pointer to XAsu_RsaClientParams structure which holds the
 *					parameters of RSA input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaCrtDec(XAsu_ClientParams *ClientParamPtr, XAsu_RsaClientParams *RsaClientParamPtr)
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

	if (((RsaClientParamPtr->InputDataAddr == 0U) ||
	     (RsaClientParamPtr->OutputDataAddr == 0U) ||
	     (RsaClientParamPtr->KeyCompAddr == 0U)) || ((RsaClientParamPtr->Len !=
		     XRSA_2048_KEY_SIZE) && (RsaClientParamPtr->Len != XRSA_3072_KEY_SIZE) &&
		     (RsaClientParamPtr->Len != XRSA_4096_KEY_SIZE))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_RSA_PVT_CRT_DEC_CMD_ID, UniqueId, XASU_MODULE_RSA_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, RsaClientParamPtr,
					sizeof(XAsu_RsaClientParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA Known Answer Tests (KAT's).
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

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_RSA_KAT_CMD_ID, UniqueId, XASU_MODULE_RSA_ID, 0U);

	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}
/** @} */