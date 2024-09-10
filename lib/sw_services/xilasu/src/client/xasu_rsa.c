/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_rsa.c
 * @addtogroup Overview
 * @{
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
 *
 * </pre>
 *
 *************************************************************************************************/

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
 * @brief	This function sends request to ASU for public encryption operation.
 *
 * @param	ClientParamPtr Pointer to parameters for client function.
 * @param	RsaClientParamPtr Pointer to parameters for RSA operation.
 *
 * @return
 *		- XST_SUCCESS, if operation is successful
 *		- Error code, if operation fails
 *
 *************************************************************************************************/
s32 XAsu_RsaEnc(XAsu_ClientParams *ClientParamPtr, XAsu_RsaClientParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/* Validatations of inputs */
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

	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_RSA_PUB_ENC_CMD_ID, 0U,
				  XASU_MODULE_RSA_ID, 0U);
	Status = Xil_SecureMemCpy((XAsu_RsaClientParams *)QueueBuf->ReqBuf.Arg,
				  sizeof(QueueBuf->ReqBuf.Arg), RsaClientParamPtr,
				  sizeof(XAsu_RsaClientParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}
/*************************************************************************************************/
/**
 * @brief	This function sends request to ASU for private decryption operation.
 *
 * @param	ClientParamPtr Pointer to parameters for client function.
 * @param	RsaClientParamPtr Pointer to parameters for RSA operation.
 *
 * @return
 *		- XST_SUCCESS, if operation is successful
 *		- Error code, if operation fails
 *
 *************************************************************************************************/
s32 XAsu_RsaDec(XAsu_ClientParams *ClientParamPtr, XAsu_RsaClientParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/* Validatations of inputs */
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

	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_RSA_PVT_DEC_CMD_ID, 0U,
				  XASU_MODULE_RSA_ID, 0U);
	Status = Xil_SecureMemCpy((XAsu_RsaClientParams *)QueueBuf->ReqBuf.Arg,
				  sizeof(QueueBuf->ReqBuf.Arg), RsaClientParamPtr,
				  sizeof(XAsu_RsaClientParams));

	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends request to ASU for CRT decryption operation.
 *
 * @param	ClientParamPtr Pointer to parameters for client function.
 * @param	RsaClientParamPtr Pointer to parameters for RSA operation.
 *
 * @return
 *		- XST_SUCCESS, if operation is successful
 *		- Error code, if operation fails
 *
 *************************************************************************************************/
s32 XAsu_RsaCrtDec(XAsu_ClientParams *ClientParamPtr, XAsu_RsaClientParams *RsaClientParamPtr)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/* Validatations of inputs */
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

	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_RSA_PVT_CRT_DEC_CMD_ID, 0U,
				  XASU_MODULE_RSA_ID, 0U);
	Status = Xil_SecureMemCpy((XAsu_RsaClientParams *)QueueBuf->ReqBuf.Arg,
				  sizeof(QueueBuf->ReqBuf.Arg), RsaClientParamPtr,
				  sizeof(XAsu_RsaClientParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends request to ASU for RSA KAT operation.
 *
 * @return
 *		- XST_SUCCESS, if operation is successful
 *		- Error code, if operation fails
 *
 *************************************************************************************************/
s32 XAsu_RsaKat(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

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

	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_RSA_KAT_CMD_ID, 0U, XASU_MODULE_RSA_ID,
				  0U);

	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}