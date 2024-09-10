/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ecc.c
 * @addtogroup Overview
 * @{
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
 *
 * </pre>
 *
 *************************************************************************************************/

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
 * @brief	This function sends request to ASU for signature generation.
 *
 * @param	ClientParamsPtr	Pointer to client params structure.
 * @param	EccParamsPtr	Pointer to ECC params structure.
 *
 * @return
 *		- XST_SUCCESS, if signature generated successfully
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid
 *		- XST_FAILURE, if send IPI fails
 *
 *************************************************************************************************/
s32 XAsu_EccGenSign(XAsu_ClientParams *ClientParamsPtr, XAsu_EccParams *EccParamsPtr)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/* Validatations of inputs */
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

	QueueInfo = XAsu_GetQueueInfo(ClientParamsPtr->Priority);
	if (QueueInfo == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/* Get Queue memory */
	QueueBuf = XAsu_GetChannelQueueBuf(QueueInfo);
	if (QueueBuf == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_ECC_GEN_SIGNATURE_CMD_ID, 0U,
				  XASU_MODULE_ECC_ID, 0U);
	Status = Xil_SecureMemCpy((void *)&QueueBuf->ReqBuf.Arg[0],
				  sizeof(XAsu_EccParams), (void *)EccParamsPtr, sizeof(XAsu_EccParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends request to ASU for signature verification
 *
 * @param	ClientParamsPtr	Pointer to client params structure.
 * @param	EccParamsPtr	Pointer to ECC params structure.
 *
 * @return
 *		- XST_SUCCESS, if signature verified successfully
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid
 *		- XST_FAILURE, if send IPI fails
 *
 *************************************************************************************************/
s32 XAsu_EccVerifySign(XAsu_ClientParams *ClientParamsPtr, XAsu_EccParams *EccParamsPtr)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/* Validatations of inputs */
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

	QueueInfo = XAsu_GetQueueInfo(ClientParamsPtr->Priority);
	if (QueueInfo == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}
	/* Get Queue memory */
	QueueBuf = XAsu_GetChannelQueueBuf(QueueInfo);
	if (QueueBuf == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_ECC_VERIFY_SIGNATURE_CMD_ID, 0U,
				  XASU_MODULE_ECC_ID, 0U);
	Status = Xil_SecureMemCpy((XAsu_EccParams *)&QueueBuf->ReqBuf.Arg[0],
				  sizeof(XAsu_EccParams), EccParamsPtr, sizeof(XAsu_EccParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends request to ASU for signature verification
 *
 * @param	ClientParamsPtr	Pointer to client params structure.
 *
 * @return
 *		- XST_SUCCESS, if ECC Kat runs succefully
 *		- XASU_INVALID_ARGUMENT, if any argument is invalid
 *		- XST_FAILURE, if send IPI fails
 *
 *************************************************************************************************/
s32 XAsu_EccKat(XAsu_ClientParams *ClientParamsPtr)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/* Validatations of inputs */
	if (ClientParamsPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ClientParamsPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamsPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	QueueInfo = XAsu_GetQueueInfo(ClientParamsPtr->Priority);
	if (QueueInfo == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}
	/* Get Queue memory */
	QueueBuf = XAsu_GetChannelQueueBuf(QueueInfo);
	if (QueueBuf == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_ECC_KAT_CMD_ID, 0U, XASU_MODULE_ECC_ID, 0U);

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
 *		- XST_SUCCESS, if curve type is valid
 *		- XST_FAILURE, if curve type is invalid
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
