/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_aes.c
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
/**
 * @addtogroup xasu_aes_client_apis AES Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_aes.h"
#include "xasu_def.h"
#include "xasu_status.h"
#include "xasu_aes_common.h"

/************************************ Constant Definitions ***************************************/

/************************************ Type Definitions *******************************************/

/************************************ Macros (Inline Functions) Definitions **********************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function performs AES encryption operation on a given payload data with
 * 		specified AES mode.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	AesParamsPtr	Pointer to Asu_AesParams structure which holds the parameters of
 * 				AES input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_AesEncrypt(XAsu_ClientParams *ClientParamPtr, Asu_AesParams *AesClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validatations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (AesClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesClientParamPtr->OperationFlags &
			(XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL)) == 0x0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (((AesClientParamPtr->OperationFlags & XASU_AES_INIT) == XASU_AES_INIT) &&
		(AesClientParamPtr->KeyObjectAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Both InputDataAddr/OutputDataAddr and AadAddr cannot be zero.
	 */
	if (((AesClientParamPtr->OperationFlags & XASU_AES_UPDATE) == XASU_AES_UPDATE) &&
		(((AesClientParamPtr->OutputDataAddr == 0U) && (AesClientParamPtr->AadAddr == 0U)) ||
		((AesClientParamPtr->InputDataAddr == 0U) && (AesClientParamPtr->AadAddr == 0U)))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * The minimum length of plaintext/AAD data must be at least 8 bits, while the
	 * maximum length should be less than 0x1FFFFFFC bytes, which is the
	 * ASU DMA's maximum supported data transfer length.
	 */
	if ((((AesClientParamPtr->DataLen == 0U) ||
			(AesClientParamPtr->DataLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) &&
			(AesClientParamPtr->AadLen == 0U)) ||
			(((AesClientParamPtr->AadLen == 0U) ||
			(AesClientParamPtr->AadLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) &&
			(AesClientParamPtr->DataLen == 0U))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesClientParamPtr->EngineMode > XASU_AES_GCM_MODE) &&
			(AesClientParamPtr->EngineMode != XASU_AES_CMAC_MODE) &&
			(AesClientParamPtr->EngineMode != XASU_AES_GHASH_MODE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesClientParamPtr->IsLast != TRUE) && (AesClientParamPtr->IsLast != FALSE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (AesClientParamPtr->OperationType != XASU_AES_ENCRYPT_OPERATION) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_AesValidateIv(AesClientParamPtr->EngineMode, AesClientParamPtr->IvAddr,
		AesClientParamPtr->IvLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_AesValidateTag(AesClientParamPtr->EngineMode, AesClientParamPtr->TagAddr,
		AesClientParamPtr->TagLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_AES_OPERATION_CMD_ID, UniqueId, XASU_MODULE_AES_ID, 0U);

	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, AesClientParamPtr,
					sizeof(Asu_AesParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs AES decryption operation on a given payload data with
 * 		specified AES mode.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	AesParamsPtr	Pointer to Asu_AesParams structure which holds the parameters of
 * 				AES input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_AesDecrypt(XAsu_ClientParams *ClientParamPtr, Asu_AesParams *AesClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validatations of inputs. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (AesClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((ClientParamPtr->Priority != XASU_PRIORITY_HIGH) &&
			(ClientParamPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesClientParamPtr->OperationFlags &
			(XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL)) == 0x0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (((AesClientParamPtr->OperationFlags & XASU_AES_INIT) == XASU_AES_INIT) &&
		(AesClientParamPtr->KeyObjectAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Both InputDataAddr/OutputDataAddr and AadAddr cannot be zero.
	 */
	if (((AesClientParamPtr->OperationFlags & XASU_AES_UPDATE) == XASU_AES_UPDATE) &&
		(((AesClientParamPtr->OutputDataAddr == 0U) && (AesClientParamPtr->AadAddr == 0U)) ||
		((AesClientParamPtr->InputDataAddr == 0U) && (AesClientParamPtr->AadAddr == 0U)))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}
	/**
	 * The minimum length of plaintext/AAD data must be at least 8 bits, while the
	 * maximum length should be less than 0x1FFFFFFC bytes, which is the
	 * ASU DMA's maximum supported data transfer length.
	 */
	if ((((AesClientParamPtr->DataLen == 0U) ||
			(AesClientParamPtr->DataLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) &&
			(AesClientParamPtr->AadLen == 0U)) ||
			(((AesClientParamPtr->AadLen == 0U) ||
			(AesClientParamPtr->AadLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) &&
			(AesClientParamPtr->DataLen == 0U))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesClientParamPtr->EngineMode > XASU_AES_GCM_MODE) &&
			(AesClientParamPtr->EngineMode != XASU_AES_CMAC_MODE) &&
			(AesClientParamPtr->EngineMode != XASU_AES_GHASH_MODE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((AesClientParamPtr->IsLast != TRUE) && (AesClientParamPtr->IsLast != FALSE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (AesClientParamPtr->OperationType != XASU_AES_DECRYPT_OPERATION) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_AesValidateIv(AesClientParamPtr->EngineMode, AesClientParamPtr->IvAddr,
		AesClientParamPtr->IvLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_AesValidateTag(AesClientParamPtr->EngineMode, AesClientParamPtr->TagAddr,
		AesClientParamPtr->TagLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_AES_OPERATION_CMD_ID, UniqueId, XASU_MODULE_AES_ID, 0U);

	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, AesClientParamPtr,
					sizeof(Asu_AesParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs AES Known Answer Tests (KAT's).
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
s32 XAsu_AesKat(XAsu_ClientParams *ClientParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_AES_KAT_CMD_ID, UniqueId, XASU_MODULE_AES_ID, 0U);

	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}

/** @} */
