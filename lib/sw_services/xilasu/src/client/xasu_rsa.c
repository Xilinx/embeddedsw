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
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/** Validatations of inputs. */
	if (((RsaClientParamPtr->InputDataAddr == 0U) ||
	     (RsaClientParamPtr->OutputDataAddr == 0U) ||
	     (RsaClientParamPtr->KeyCompAddr == 0U)) || ((RsaClientParamPtr->Len !=
		     XRSA_2048_KEY_SIZE) && (RsaClientParamPtr->Len != XRSA_3072_KEY_SIZE) &&
		     (RsaClientParamPtr->Len != XRSA_4096_KEY_SIZE))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ClientParamPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Get the pointer to QueueInfo structure for provided priority. */
	QueueInfo = XAsu_GetQueueInfo(ClientParamPtr->Priority);
	if (QueueInfo == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/* Get Queue memory. */
	QueueBuf = XAsu_GetChannelQueueBuf(QueueInfo);
	if (QueueBuf == NULL) {
		Status = XASU_QUEUE_FULL;
		goto END;
	}

	/** Update the request buffer. */
	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_RSA_PUB_ENC_CMD_ID, 0U,
				  XASU_MODULE_RSA_ID, 0U);
	Status = Xil_SecureMemCpy((XAsu_RsaClientParams *)QueueBuf->ReqBuf.Arg,
				  sizeof(QueueBuf->ReqBuf.Arg), RsaClientParamPtr,
				  sizeof(XAsu_RsaClientParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Send IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

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
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/** Validatations of inputs. */
	if (((RsaClientParamPtr->InputDataAddr == 0U) ||
	     (RsaClientParamPtr->OutputDataAddr == 0U) ||
	     (RsaClientParamPtr->KeyCompAddr == 0U)) || ((RsaClientParamPtr->Len !=
		     XRSA_2048_KEY_SIZE) && (RsaClientParamPtr->Len != XRSA_3072_KEY_SIZE) &&
		     (RsaClientParamPtr->Len != XRSA_4096_KEY_SIZE))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ClientParamPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Get the pointer to QueueInfo structure for provided priority. */
	QueueInfo = XAsu_GetQueueInfo(ClientParamPtr->Priority);
	if (QueueInfo == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/* Get Queue memory */
	QueueBuf = XAsu_GetChannelQueueBuf(QueueInfo);
	if (QueueBuf == NULL) {
		Status = XASU_QUEUE_FULL;
		goto END;
	}

	/** Update the request buffer. */
	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_RSA_PVT_DEC_CMD_ID, 0U,
				  XASU_MODULE_RSA_ID, 0U);
	Status = Xil_SecureMemCpy((XAsu_RsaClientParams *)QueueBuf->ReqBuf.Arg,
				  sizeof(QueueBuf->ReqBuf.Arg), RsaClientParamPtr,
				  sizeof(XAsu_RsaClientParams));

	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Send IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

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
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/** Validatations of inputs. */
	if (((RsaClientParamPtr->InputDataAddr == 0U) ||
	     (RsaClientParamPtr->OutputDataAddr == 0U) ||
	     (RsaClientParamPtr->KeyCompAddr == 0U)) || ((RsaClientParamPtr->Len !=
		     XRSA_2048_KEY_SIZE) && (RsaClientParamPtr->Len != XRSA_3072_KEY_SIZE) &&
		     (RsaClientParamPtr->Len != XRSA_4096_KEY_SIZE))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ClientParamPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Get the pointer to QueueInfo structure for provided priority. */
	QueueInfo = XAsu_GetQueueInfo(ClientParamPtr->Priority);
	if (QueueInfo == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}
	/* Get Queue memory */
	QueueBuf = XAsu_GetChannelQueueBuf(QueueInfo);
	if (QueueBuf == NULL) {
		Status = XASU_QUEUE_FULL;
		goto END;
	}

	/** Update the request buffer. */
	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_RSA_PVT_CRT_DEC_CMD_ID, 0U,
				  XASU_MODULE_RSA_ID, 0U);
	Status = Xil_SecureMemCpy((XAsu_RsaClientParams *)QueueBuf->ReqBuf.Arg,
				  sizeof(QueueBuf->ReqBuf.Arg), RsaClientParamPtr,
				  sizeof(XAsu_RsaClientParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Send IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA Known Answer Tests (KAT's).
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaKat(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/** Get the pointer to QueueInfo structure for provided priority. */
	QueueInfo = XAsu_GetQueueInfo(XASU_PRIORITY_HIGH);
	if (QueueInfo == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}
	/* Get Queue memory */
	QueueBuf = XAsu_GetChannelQueueBuf(QueueInfo);
	if (QueueBuf == NULL) {
		Status = XASU_QUEUE_FULL;
		goto END;
	}

	/** Update the request buffer. */
	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_RSA_KAT_CMD_ID, 0U, XASU_MODULE_RSA_ID,
				  0U);

	/** Send IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}
/** @} */