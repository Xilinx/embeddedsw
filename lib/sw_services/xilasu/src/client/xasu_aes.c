/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * 1.1   am   01/20/25 Added AES CCM support
 *       am   03/14/25 Replaced XAsu_AesValidateIv() with XAsu_AesValidateIvParams() and
 *                     XAsu_AesValidateTag() with XAsu_AesValidateTagParams() function calls
 *       am   04/03/25 Optimized engine mode check for AAD update
 *       am   04/26/25 Cleaned and simplified AAD and input data validation logic
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
#include "xasu_aes_common.h"

/************************************ Constant Definitions ***************************************/

/************************************ Type Definitions *******************************************/

/************************************ Macros (Inline Functions) Definitions **********************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_AesValidateKeyObjectParams(const XAsu_AesKeyObject *KeyObjectPtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform AES encryption/decryption operation
 * 		on a given payload data	with specified AES mode.
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
 * @note	Verify the additional status if operation flag is set to XASU_AES_FINAL.
 * 		- XASU_AES_TAG_READ, if encryption operation successfully done.
 * 		- XASU_AES_TAG_MATCHED, if decryption operation successfully done.
 * 		- Any other value shall be treated as failure.
 *
 *************************************************************************************************/
s32 XAsu_AesOperation(XAsu_ClientParams *ClientParamPtr, Asu_AesParams *AesClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	void *AesCtx = NULL;

	/** Validate the input parameters. */
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

	/** Validate client additional status pointer for critical security calls. */
	if (((AesClientParamPtr->EngineMode == XASU_AES_CCM_MODE) ||
		(AesClientParamPtr->EngineMode == XASU_AES_GCM_MODE) ||
		(AesClientParamPtr->EngineMode == XASU_AES_CMAC_MODE)) &&
		((AesClientParamPtr->OperationFlags & XASU_AES_FINAL) != 0x0U) &&
		(ClientParamPtr->AdditionalStatusPtr == NULL)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate the operation flags for AES CCM mode to ensure all the flags are set. */
	if ((AesClientParamPtr->EngineMode == XASU_AES_CCM_MODE) && ((AesClientParamPtr->OperationFlags &
			(XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL)) !=
			(XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL))) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate required parameters for AES initialization operation. */
	if ((AesClientParamPtr->OperationFlags & XASU_AES_INIT) == XASU_AES_INIT) {
		/** Validate AES operation type. */
		if ((AesClientParamPtr->OperationType != XASU_AES_ENCRYPT_OPERATION) &&
				(AesClientParamPtr->OperationType != XASU_AES_DECRYPT_OPERATION)) {
			Status = XASU_INVALID_ARGUMENT;
			goto END;
		}

		/** Validate AES engine mode. */
		if ((AesClientParamPtr->EngineMode > XASU_AES_GCM_MODE) &&
				(AesClientParamPtr->EngineMode != XASU_AES_CMAC_MODE) &&
				(AesClientParamPtr->EngineMode != XASU_AES_GHASH_MODE)) {
			Status = XASU_INVALID_ARGUMENT;
			goto END;
		}

		/** Validate AES key object structure parameters. */
		Status = XAsu_AesValidateKeyObjectParams(
			(const XAsu_AesKeyObject *)AesClientParamPtr->KeyObjectAddr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** Validate IV. */
		Status = XAsu_AesValidateIvParams(AesClientParamPtr->EngineMode,
			AesClientParamPtr->IvAddr, AesClientParamPtr->IvLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** Validate required parameters for AES update operation. */
	if ((AesClientParamPtr->OperationFlags & XASU_AES_UPDATE) == XASU_AES_UPDATE) {
		/**
		 * Both AAD and InputData/OutputData address and lengths cannot be zero at once.
		 * The minimum length of plaintext/AAD length must be at least 1 byte, while the
		 * maximum length can be 0x1FFFFFFC bytes, which is the ASU DMA's maximum supported
		 * data transfer length.
		 */
		if (AesClientParamPtr->AadLen != 0U) {
			/**
			 * If AAD length is non-zero, AAD address must be valid and AAD length within limit.
			 */
			if ((AesClientParamPtr->AadLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) ||
					(AesClientParamPtr->AadAddr == 0U)) {
				Status = XASU_INVALID_ARGUMENT;
				goto END;
			}
			if (!(XASU_AES_IS_AAD_SUPPORTED_MODE(AesClientParamPtr->EngineMode))) {
				Status = XASU_INVALID_ARGUMENT;
				goto END;
			}
		}
		else {
			/** If AAD Length is zero, AAD address must also be zero. */
			if ((AesClientParamPtr->AadAddr != 0U) &&
					(XASU_AES_IS_AAD_SUPPORTED_MODE(AesClientParamPtr->EngineMode))) {
				Status = XASU_INVALID_ARGUMENT;
				goto END;
			}
		}

		if (AesClientParamPtr->DataLen != 0U) {
			/**
			 * If Data length is non-zero, both input and output addresses must be valid and
			 * data length within limit.
			 * AES-CMAC engine mode must not allow data updates.
			 */
			if ((AesClientParamPtr->DataLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) ||
					(AesClientParamPtr->InputDataAddr == 0U) ||
					(AesClientParamPtr->OutputDataAddr == 0U)) {
				Status = XASU_INVALID_ARGUMENT;
				goto END;
			}
			if (AesClientParamPtr->EngineMode == XASU_AES_CMAC_MODE) {
				Status = XASU_INVALID_ARGUMENT;
				goto END;
			}
		}
		else {
			/** If Data Length is zero, addresses must also be zero. */
			if ((AesClientParamPtr->InputDataAddr != 0U) ||
					(AesClientParamPtr->OutputDataAddr != 0U)) {
				Status = XASU_INVALID_ARGUMENT;
				goto END;
			}
		}

		/**
		 * For the ECB, CBC, and CFB modes, the plaintext must be a sequence of one or more
		 * complete data blocks.
		 */
		if (((AesClientParamPtr->EngineMode == XASU_AES_ECB_MODE) ||
				(AesClientParamPtr->EngineMode == XASU_AES_CBC_MODE) ||
				(AesClientParamPtr->EngineMode == XASU_AES_CFB_MODE)) &&
				((AesClientParamPtr->DataLen % XASU_AES_BLOCK_SIZE_IN_BYTES) != 0U)) {
			Status = XASU_INVALID_ARGUMENT;
			goto END;
		}
		/** Validate IsLast flag. */
		if ((AesClientParamPtr->IsLast != XASU_TRUE) && (AesClientParamPtr->IsLast != XASU_FALSE)) {
			Status = XASU_INVALID_ARGUMENT;
			goto END;
		}
	}

	/** Validate tag for AES final operation. */
	if ((AesClientParamPtr->OperationFlags & XASU_AES_FINAL) == XASU_AES_FINAL) {
		Status = XAsu_AesValidateTagParams(AesClientParamPtr->EngineMode, AesClientParamPtr->TagAddr,
			AesClientParamPtr->TagLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** If operation flag is set to INIT, */
	if ((AesClientParamPtr->OperationFlags & XASU_AES_INIT) == XASU_AES_INIT) {
		/** - Generate Unique ID. */
		UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_FALSE);
		if (UniqueId >= XASU_UNIQUE_ID_MAX) {
			Status = XASU_INVALID_UNIQUE_ID;
			goto END;
		}
		/** - Save the context. */
		AesCtx = XAsu_UpdateNGetCtx(UniqueId);
		if (AesCtx == NULL) {
			Status = XASU_FAIL_SAVE_CTX;
			goto END;
		}
		ClientParamPtr->ClientCtx = AesCtx;
	}
	/** If operation flag is either UPDATE or FINAL, */
	else {
		/** - Verify the context. */
		Status = XAsu_VerifyNGetUniqueIdCtx(ClientParamPtr->ClientCtx, &UniqueId);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** If FINISH operation flag is set, update response buffer details. */
	if ((AesClientParamPtr->OperationFlags & XASU_AES_FINAL) == XASU_AES_FINAL) {
		XAsu_UpdateCallBackDetails(UniqueId, NULL, 0U, XASU_TRUE);
		/* Free AES Ctx */
		XAsu_FreeCtx(ClientParamPtr->ClientCtx);
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_AES_OPERATION_CMD_ID, UniqueId, XASU_MODULE_AES_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, AesClientParamPtr,
		sizeof(Asu_AesParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to perform AES Known Answer Tests (KAT's).
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

	/** Generate unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_AES_KAT_CMD_ID, UniqueId, XASU_MODULE_AES_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the XAsu_AesKeyObject structure parameters.
 *
 * @param	KeyObjectPtr	Pointer to the XAsu_AesKeyObject structure which holds the key
 * 				object parameters.
 *
 * @return
 * 		- XST_SUCCESS, if validation of key object parameters is successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *
 *************************************************************************************************/
static s32 XAsu_AesValidateKeyObjectParams(const XAsu_AesKeyObject *KeyObjectPtr)
{
	s32 Status =  XASU_INVALID_ARGUMENT;

	if (KeyObjectPtr == NULL) {
		goto END;
	}

	if (KeyObjectPtr->KeyAddress == 0U) {
		goto END;
	}

	if ((KeyObjectPtr->KeySize != XASU_AES_KEY_SIZE_128_BITS) &&
		(KeyObjectPtr->KeySize != XASU_AES_KEY_SIZE_256_BITS)) {
		goto END;
	}

	if ((KeyObjectPtr->KeySrc >= XASU_AES_MAX_KEY_SOURCES) ||
		(KeyObjectPtr->KeySrc == XASU_AES_EFUSE_KEY_0) ||
		(KeyObjectPtr->KeySrc == XASU_AES_EFUSE_KEY_1)){
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
