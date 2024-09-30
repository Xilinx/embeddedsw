/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
static s32 XAsu_EccValidateCurveType(u32 CurveType);

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
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/** Validate input parameters. */
	if ((ClientParamsPtr == NULL) || (EccParamsPtr == NULL)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ClientParamsPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamsPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (XAsu_EccValidateCurveType(EccParamsPtr->CurveType) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Get the pointer to QueueInfo structure for provided priority. */
	QueueInfo = XAsu_GetQueueInfo(ClientParamsPtr->Priority);
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
	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_ECC_GEN_SIGNATURE_CMD_ID, 0U,
				  XASU_MODULE_ECC_ID, 0U);
	Status = Xil_SecureMemCpy((void *)&QueueBuf->ReqBuf.Arg[0],
				  sizeof(XAsu_EccParams), (void *)EccParamsPtr, sizeof(XAsu_EccParams));
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
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/** Validate input parameters. */
	if ((ClientParamsPtr == NULL) || (EccParamsPtr == NULL)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ClientParamsPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamsPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (XAsu_EccValidateCurveType(EccParamsPtr->CurveType) != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Get the pointer to QueueInfo structure for provided priority. */
	QueueInfo = XAsu_GetQueueInfo(ClientParamsPtr->Priority);
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
	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_ECC_VERIFY_SIGNATURE_CMD_ID, 0U,
				  XASU_MODULE_ECC_ID, 0U);
	Status = Xil_SecureMemCpy((XAsu_EccParams *)&QueueBuf->ReqBuf.Arg[0],
				  sizeof(XAsu_EccParams), EccParamsPtr, sizeof(XAsu_EccParams));
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
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/** Validate input parameters. */
	if (ClientParamsPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ClientParamsPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamsPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Get the pointer to QueueInfo structure for provided priority. */
	QueueInfo = XAsu_GetQueueInfo(ClientParamsPtr->Priority);
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
	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_ECC_KAT_CMD_ID, 0U, XASU_MODULE_ECC_ID, 0U);
	/** Send IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the given curve type.
 *
 * @param	CurveType	Curve type provided.
 *
 * @return
 * 		- XST_SUCCESS, if curve type is valid.
 * 		- XST_FAILURE, if curve type is invalid.
 *
 *************************************************************************************************/
static s32 XAsu_EccValidateCurveType(u32 CurveType)
{
	s32 Status = XST_FAILURE;

	if ((CurveType == XASU_ECC_NIST_P192) || (CurveType == XASU_ECC_NIST_P224) ||
	    (CurveType == XASU_ECC_NIST_P256) || (CurveType == XASU_ECC_NIST_P384) ||
	    (CurveType == XASU_ECC_NIST_P521) || (CurveType == XASU_ECC_BRAINPOOL_P256) ||
	    (CurveType == XASU_ECC_BRAINPOOL_P320) || (CurveType == XASU_ECC_BRAINPOOL_P384) ||
	    (CurveType == XASU_ECC_BRAINPOOL_P512)) {
		Status = XST_SUCCESS;
	}

	return Status;
}
/** @} */
