/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_aes.c
 * @addtogroup Overview
 * @{
 *
 * This file contains the implementation of the client interface functions for aes module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   08/01/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasu_aes.h"
#include "xasu_def.h"
#include "xasu_status.h"

/************************************ Constant Definitions ***************************************/

/************************************ Type Definitions *******************************************/

/************************************ Macros (Inline Functions) Definitions **********************/

/************************************ Function Prototypes ****************************************/
static inline s32 XAsu_AesValidateIv(u8 EngineMode, u64 IvAddr, u32 IvLen);
static inline s32 XAsu_AesValidateTag(u8 EngineMode, u64 TagAddr, u32 TagLen);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends request to ASU to perform AES encryption operation on a given
 *		payload data.
 *
 * @param	ClientParamsPtr	Pointer to XAsu_ClientParams structure.
 * @param	AesParamsPtr	Pointer to Asu_AesParams structure.
 *
 * @return
 *		- Upon successful encryption of data, it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
s32 XAsu_AesEncrypt(XAsu_ClientParams *ClientParamsPtr, Asu_AesParams *AesParamsPtr)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/* Validatations of inputs */
	if ((ClientParamsPtr == NULL) || (AesParamsPtr == NULL)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ClientParamsPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamsPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesParamsPtr->InputDataAddr == 0U) || (AesParamsPtr->KeyObjectAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/* During multiple updates, address of output data should be zero while updating AAD */
	if ((AesParamsPtr->OutputDataAddr == 0U) && (AesParamsPtr->AadAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/* Minimum Length of Plaintext/Aad should be of atleast 8bits */
	if ((AesParamsPtr->DataLen == 0U) && (AesParamsPtr->AadLen == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesParamsPtr->EngineMode > XASU_AES_GCM_MODE) &&
	    (AesParamsPtr->EngineMode != XASU_AES_CMAC_MODE) &&
	    (AesParamsPtr->EngineMode != XASU_AES_GHASH_MODE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (((AesParamsPtr->OperationFlags & XASU_AES_INIT) != XASU_AES_INIT) &&
	    ((AesParamsPtr->OperationFlags & XASU_AES_UPDATE) != XASU_AES_UPDATE) &&
	    ((AesParamsPtr->OperationFlags & XASU_AES_FINAL) != XASU_AES_FINAL) &&
	    ((AesParamsPtr->OperationFlags &
	      (XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL)) !=
	     (XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesParamsPtr->IsLast != TRUE) && (AesParamsPtr->IsLast != FALSE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (AesParamsPtr->OperationType != XASU_AES_ENCRYPT_OPERATION) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_AesValidateIv(AesParamsPtr->EngineMode, AesParamsPtr->IvAddr, AesParamsPtr->IvLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_AesValidateTag(AesParamsPtr->EngineMode, AesParamsPtr->TagAddr, AesParamsPtr->TagLen);
	if (Status != XST_SUCCESS) {
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
		Status = XASU_QUEUE_FULL;
		goto END;
	}

	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_AES_OPERATION_CMD_ID, 0U,
				  XASU_MODULE_AES_ID, 0U);

	Status = Xil_SecureMemCpy(QueueBuf->ReqBuf.Arg, sizeof(QueueBuf->ReqBuf.Arg),
				  AesParamsPtr, sizeof(Asu_AesParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends request to ASU to perform AES decryption operation on a given
 *		encrypted data.
 *
 * @param	ClientParamsPtr	Pointer to XAsu_ClientParams structure.
 * @param	AesParamsPtr	Pointer to Asu_AesParams structure.
 *
 * @return
 *		- Upon successful decryption of data, it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
s32 XAsu_AesDecrypt(XAsu_ClientParams *ClientParamsPtr, Asu_AesParams *AesParamsPtr)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/* Validatations of inputs */
	if ((ClientParamsPtr == NULL) || (AesParamsPtr == NULL)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ClientParamsPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamsPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesParamsPtr->InputDataAddr == 0U) || (AesParamsPtr->KeyObjectAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/* During multiple updates, address of output data should be zero while updating AAD */
	if ((AesParamsPtr->OutputDataAddr == 0U) && (AesParamsPtr->AadAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/* Minimum Length of Plaintext/Aad should be of atleast 8bits */
	if ((AesParamsPtr->DataLen == 0U) && (AesParamsPtr->AadLen == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesParamsPtr->EngineMode > XASU_AES_GCM_MODE) &&
	    (AesParamsPtr->EngineMode != XASU_AES_CMAC_MODE) &&
	    (AesParamsPtr->EngineMode != XASU_AES_GHASH_MODE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (((AesParamsPtr->OperationFlags & XASU_AES_INIT) != XASU_AES_INIT) &&
	    ((AesParamsPtr->OperationFlags & XASU_AES_UPDATE) != XASU_AES_UPDATE) &&
	    ((AesParamsPtr->OperationFlags & XASU_AES_FINAL) != XASU_AES_FINAL) &&
	    ((AesParamsPtr->OperationFlags &
	      (XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL)) !=
	     (XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesParamsPtr->IsLast != TRUE) && (AesParamsPtr->IsLast != FALSE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (AesParamsPtr->OperationType != XASU_AES_DECRYPT_OPERATION) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_AesValidateIv(AesParamsPtr->EngineMode, AesParamsPtr->IvAddr, AesParamsPtr->IvLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_AesValidateTag(AesParamsPtr->EngineMode, AesParamsPtr->TagAddr, AesParamsPtr->TagLen);
	if (Status != XST_SUCCESS) {
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
		Status = XASU_QUEUE_FULL;
		goto END;
	}

	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_AES_OPERATION_CMD_ID, 0U,
				  XASU_MODULE_AES_ID, 0U);

	Status = Xil_SecureMemCpy(QueueBuf->ReqBuf.Arg, sizeof(QueueBuf->ReqBuf.Arg),
				  AesParamsPtr, sizeof(Asu_AesParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends request to ASU to perform AES KAT operation.
 *
 * @return
 *		- XST_SUCCESS, if operation is successful
 *		- Error code, if operation fails
 *
 *************************************************************************************************/
s32 XAsu_AesKat(void)
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

	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_AES_KAT_CMD_ID, 0U, XASU_MODULE_AES_ID, 0U);

	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates IV for given AES engine mode.
 *
 * @param	EngineMode	AES engine mode.
 * @param	IvAddr		Address of buffer holding IV.
 * @param	IvLen		Lenght of the IV in bytes.
 *
 * @return
 *		- Upon successful validation of IV, it returns XST_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
static inline s32 XAsu_AesValidateIv(u8 EngineMode, u64 IvAddr, u32 IvLen)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	/*
	 * IV Validation for respective AES engine modes
	 * AES Standard mode (ECB, CBC, CTR, CFB, OFB).
	 * AES MAC mode (GCM, CCM, GMAC, CMAC).
	 *
	 * |   Engine Mode     |   IvAddress    |   IvLength           |
	 * |-------------------|----------------|----------------------|
	 * | AES-ECB, AES-CMAC |     N/A        |      N/A             |
	 * | AES-GCM           |   Non-zero     |  Any non-zero Length |
	 * | Remaining modes   |   Non-zero     |  12 or 16 Bytes      |
	 */
	switch (EngineMode) {
		case XASU_AES_ECB_MODE:
		case XASU_AES_CMAC_MODE:
			if ((IvAddr == 0U) && (IvLen == 0U)) {
				Status = XST_SUCCESS;
			}
			break;
		case XASU_AES_GCM_MODE:
			if ((IvAddr != 0U) && (IvLen != 0U)) {
				Status = XST_SUCCESS;
			}
			break;
		case XASU_AES_CBC_MODE:
		case XASU_AES_CFB_MODE:
		case XASU_AES_OFB_MODE:
		case XASU_AES_CTR_MODE:
			if ((IvAddr != 0U) && ((IvLen == XASU_AES_IV_SIZE_96BIT_IN_BYTES) ||
					       (IvLen == XASU_AES_IV_SIZE_128BIT_IN_BYTES))) {
				Status = XST_SUCCESS;
			}
			break;
		default:
			Status = XASU_INVALID_ARGUMENT;
			break;
	}
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates Tag for given AES engine mode.
 *
 * @param	EngineMode	AES engine mode.
 * @param	TagAddr		Address of the Input/Output Tag.
 * @param	TagLen		Length of Tag in bytes and it will be zero for all AES
 *				standard modes like, ECB, CBC, OFB, CFB, CTR
 *
 * @return
 *		- Upon successful validation of Tag, it returns XST_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
static inline s32 XAsu_AesValidateTag(u8 EngineMode, u64 TagAddr, u32 TagLen)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	/*
	 * Tag Validation for respective AES engine modes
	 * AES Standard mode (ECB, CBC, CTR, CFB, OFB).
	 * AES MAC mode (GCM, CCM, GMAC, CMAC).
	 *
	 * |   Engine Mode       |   TagAddress   |   TagLength          |
	 * |---------------------|----------------|----------------------|
	 * | Standard mode       |     N/A        |      N/A             |
	 * | AES-GCM, CMAC       |   Non-zero     |  8<=TagLen<=16       |
	 * | AES-CCM             |   Non-zero     |  4,6,8,10,12,14,16   |
	 *
	 * NIST recommends using a tag length of atleast 64 bits to provide adequate protection
	 * against guessing attacks.
	 */
	switch (EngineMode) {
		case XASU_AES_CBC_MODE:
		case XASU_AES_CFB_MODE:
		case XASU_AES_OFB_MODE:
		case XASU_AES_CTR_MODE:
		case XASU_AES_ECB_MODE:
			if ((TagAddr == 0U) && (TagLen == 0U)) {
				Status = XST_SUCCESS;
			}
			break;
		case XASU_AES_CCM_MODE:
			if ((TagAddr != 0U) && ((TagLen % XASU_AES_EVEN_MODULUS == 0U) &&
						(TagLen >= XASU_AES_RECOMMENDED_TAG_LENGTH_IN_BYTES) &&
						(TagLen <= XASU_AES_MAX_TAG_LENGTH_IN_BYTES))) {
				Status = XST_SUCCESS;
			}
			break;
		case XASU_AES_GCM_MODE:
		case XASU_AES_CMAC_MODE:
			if ((TagAddr != 0U) && ((TagLen >= XASU_AES_RECOMMENDED_TAG_LENGTH_IN_BYTES) &&
						(TagLen <= XASU_AES_MAX_TAG_LENGTH_IN_BYTES))) {
				Status = XST_SUCCESS;
			}
			break;
		default:
			Status = XASU_INVALID_ARGUMENT;
			break;
	}
	return Status;
}
/** @} */