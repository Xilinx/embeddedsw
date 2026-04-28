/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       yog  07/10/25 Added support for priority based multiple request and context verification
 *       kd   07/23/25 Fixed gcc warnings
 *       kp   02/25/26 Added client-side AES CBC/CCM encrypt/decrypt KATs
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
#include "xasu_keymanager_common.h"
#include "xil_sutil.h"

/************************************ Constant Definitions ***************************************/
#define XASU_AES_KAT_DATA_LEN_IN_BYTES		(32U)	/**< AES KAT data length */
#define XASU_AES_KAT_KEY_LEN_IN_BYTES		(32U)	/**< AES KAT key length */
#define XASU_AES_KAT_CBC_IV_LEN_IN_BYTES	(16U)	/**< AES CBC IV length */
#define XASU_AES_KAT_CCM_NONCE_LEN_IN_BYTES	(12U)	/**< AES CCM nonce length */
#define XASU_AES_KAT_CCM_AAD_LEN_IN_BYTES	(16U)	/**< AES CCM AAD length */
#define XASU_AES_KAT_CCM_TAG_LEN_IN_BYTES	(16U)	/**< AES CCM tag length */

/************************************ Type Definitions *******************************************/
/** Internal structure to describe a KAT vector set. */
typedef struct {
	const u8 *Key;		/**< Pointer to AES key */
	const u8 *Iv;		/**< Pointer to IV/Nonce */
	u32 IvLen;		/**< IV/Nonce length in bytes */
	const u8 *Aad;		/**< Pointer to AAD (NULL if none) */
	u32 AadLen;		/**< AAD length in bytes */
	const u8 *InData;	/**< Pointer to input data */
	const u8 *ExpOutData;	/**< Pointer to expected output data */
	u32 DataLen;		/**< Data length in bytes */
	const u8 *ExpTag;	/**< Pointer to expected tag (NULL if none) */
	u32 TagLen;		/**< Tag length in bytes */
	u8 EngineMode;		/**< AES engine mode */
	u8 OperationType;	/**< Encrypt or decrypt */
} XAsu_AesKatVectors;

/** Per-invocation KAT callback state, passed through CallBackRefPtr. */
typedef struct {
	volatile u8 Notify;		/**< Completion flag */
	volatile s32 CallBackStatus;	/**< Callback status */
} XAsu_AesKatCbState;

/************************************ Macros (Inline Functions) Definitions **********************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_AesValidateKeyObjectParams(const XAsu_AesKeyObject *KeyObjectPtr);
static inline s32 XAsu_IsModeValidForAad(u8 EngineMode);
static inline s32 XAsu_ValidateAadLen(const XAsu_AesParams *AesClientParamPtr);
static inline s32 XAsu_ValidateAesEngineMode(const XAsu_AesParams *AesClientParamPtr);
static inline s32 XAsu_ValidateCcmOpMode(const XAsu_AesParams *AesClientParamPtr);
static inline s32 XAsu_ValidateDataLen(const XAsu_AesParams *AesClientParamPtr);
static s32 XAsu_AesRunClientKat(const XAsu_AesKatVectors *KatParams);

/************************************ Variable Definitions ***************************************/

/* AES KAT key - AES-256 (NIST SP 800-38A) */
static const u8 AesKatKey[XASU_AES_KAT_KEY_LEN_IN_BYTES] = {
	0x60U, 0x3DU, 0xEBU, 0x10U, 0x15U, 0xCAU, 0x71U, 0xBEU,
	0x2BU, 0x73U, 0xAEU, 0xF0U, 0x85U, 0x7DU, 0x77U, 0x81U,
	0x1FU, 0x35U, 0x2CU, 0x07U, 0x3BU, 0x61U, 0x08U, 0xD7U,
	0x2DU, 0x98U, 0x10U, 0xA3U, 0x09U, 0x14U, 0xDFU, 0xF4U
};

/* AES KAT plaintext (32 bytes, NIST SP 800-38A) */
static const u8 AesKatPt[XASU_AES_KAT_DATA_LEN_IN_BYTES] = {
	0x6BU, 0xC1U, 0xBEU, 0xE2U, 0x2EU, 0x40U, 0x9FU, 0x96U,
	0xE9U, 0x3DU, 0x7EU, 0x11U, 0x73U, 0x93U, 0x17U, 0x2AU,
	0xAEU, 0x2DU, 0x8AU, 0x57U, 0x1EU, 0x03U, 0xACU, 0x9CU,
	0x9EU, 0xB7U, 0x6FU, 0xACU, 0x45U, 0xAFU, 0x8EU, 0x51U
};

/* AES-CBC IV (16 bytes) */
static const u8 AesKatCbcIv[XASU_AES_KAT_CBC_IV_LEN_IN_BYTES] = {
	0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U,
	0x08U, 0x09U, 0x0AU, 0x0BU, 0x0CU, 0x0DU, 0x0EU, 0x0FU
};

/* AES-CBC expected ciphertext (32 bytes) */
static const u8 AesKatCbcCt[XASU_AES_KAT_DATA_LEN_IN_BYTES] = {
	0xF5U, 0x8CU, 0x4CU, 0x04U, 0xD6U, 0xE5U, 0xF1U, 0xBAU,
	0x77U, 0x9EU, 0xABU, 0xFBU, 0x5FU, 0x7BU, 0xFBU, 0xD6U,
	0x9CU, 0xFCU, 0x4EU, 0x96U, 0x7EU, 0xDBU, 0x80U, 0x8DU,
	0x67U, 0x9FU, 0x77U, 0x7BU, 0xC6U, 0x70U, 0x2CU, 0x7DU
};

/* AES-CCM nonce (12 bytes) */
static const u8 AesKatCcmNonce[XASU_AES_KAT_CCM_NONCE_LEN_IN_BYTES] = {
	0xCAU, 0xFEU, 0xBAU, 0xBEU, 0xFAU, 0xCEU, 0xDBU, 0xADU,
	0xDEU, 0xCAU, 0xF8U, 0x88U
};

/* AES-CCM AAD (16 bytes) */
static const u8 AesKatCcmAad[XASU_AES_KAT_CCM_AAD_LEN_IN_BYTES] = {
	0xFEU, 0xEDU, 0xFAU, 0xCEU, 0xDEU, 0xADU, 0xBEU, 0xEFU,
	0xFEU, 0xEDU, 0xFAU, 0xCEU, 0xDEU, 0xADU, 0xBEU, 0xEFU
};

/* AES-CCM expected ciphertext (32 bytes) */
static const u8 AesKatCcmCt[XASU_AES_KAT_DATA_LEN_IN_BYTES] = {
	0xF5U, 0xE9U, 0x26U, 0xDEU, 0x99U, 0x05U, 0x35U, 0x3CU,
	0xFFU, 0xE6U, 0xFBU, 0xAFU, 0x20U, 0x58U, 0x05U, 0x68U,
	0x4CU, 0x55U, 0x57U, 0xEAU, 0xCAU, 0x2FU, 0x90U, 0x80U,
	0x17U, 0xDFU, 0x8DU, 0x33U, 0x59U, 0xECU, 0x1DU, 0x44U
};

/* AES-CCM expected tag (16 bytes) */
static const u8 AesKatCcmTag[XASU_AES_KAT_CCM_TAG_LEN_IN_BYTES] = {
	0x7CU, 0x09U, 0x11U, 0xC0U, 0xADU, 0x5DU, 0x68U, 0xF0U,
	0x2EU, 0xEAU, 0x6EU, 0xB1U, 0xD4U, 0xF2U, 0x92U, 0xABU
};



/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform AES encryption/decryption operation
 * 		on a given payload data	with specified AES mode.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	AesClientParamPtr	Pointer to XAsu_AesParams structure which holds the parameters of
 * 				AES input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 * 		- XASU_CLIENT_CTX_NOT_CREATED, if client context is not created.
 * 		- XASU_REQUEST_INPROGRESS, if split request already in progress.
 *  		- XASU_FAIL_SAVE_CTX, if saving context fails.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XASU_FREE_CTX_FAIL, if freeing context fails.
 *
 * @note	Verify the additional status if operation flag is set to XASU_FINISH.
 * 		- XASU_AES_TAG_READ, if the encryption operation is successfully done.
 * 		- XASU_AES_TAG_MATCHED, if the decryption operation is successfully done.
 * 		- Any other value shall be treated as failure.
 *
 *************************************************************************************************/
s32 XAsu_AesOperation(XAsu_ClientParams *ClientParamPtr, XAsu_AesParams *AesClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	static void *P0AesCtx = NULL;
	static void *P1AesCtx = NULL;

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
			(XASU_INIT | XASU_UPDATE | XASU_FINISH)) == 0x0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate the operation flags for AES CCM mode to ensure all the flags are set. */
	Status = XAsu_ValidateCcmOpMode(AesClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((((AesClientParamPtr->OperationFlags & XASU_INIT) == XASU_INIT) &&
	      (AesClientParamPtr->EngineMode != XASU_AES_CMAC_MODE) &&
	      (AesClientParamPtr->EngineMode != XASU_AES_ECB_MODE)) ||
	     (((AesClientParamPtr->OperationFlags & XASU_UPDATE) == XASU_UPDATE) &&
	      (AesClientParamPtr->EngineMode == XASU_AES_CCM_MODE))) {

		/** Validate that exactly one of IvAddr or IvId is provided. */
		Status = XAsu_KmValidateKeyAddrNdKeyId(AesClientParamPtr->IvAddr,
						       AesClientParamPtr->IvId);
		if (Status != XST_SUCCESS) {
			Status = XASU_INVALID_ARGUMENT;
			goto END;
		}
		if (AesClientParamPtr->IvAddr != 0U) {
			Status = XAsu_AesValidateIvParams(AesClientParamPtr->EngineMode,
				AesClientParamPtr->IvAddr, AesClientParamPtr->IvLen);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
	}

	/** Validate required parameters for AES initialization operation. */
	if ((AesClientParamPtr->OperationFlags & XASU_INIT) == XASU_INIT) {
		/** Validate AES operation type. */
		if ((AesClientParamPtr->OperationType != XASU_AES_ENCRYPT_OPERATION) &&
				(AesClientParamPtr->OperationType != XASU_AES_DECRYPT_OPERATION)) {
			Status = XASU_INVALID_ARGUMENT;
			goto END;
		}

		/** Validate AES engine mode. */
		Status = XAsu_ValidateAesEngineMode(AesClientParamPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** Validate AES key object structure parameters. */
		Status = XAsu_AesValidateKeyObjectParams(
			(const XAsu_AesKeyObject *)(UINTPTR)AesClientParamPtr->KeyObjectAddr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** Validate required parameters for AES update operation. */
	if ((AesClientParamPtr->OperationFlags & XASU_UPDATE) == XASU_UPDATE) {
		/**
		 * Both AAD and InputData/OutputData address and lengths cannot be zero at once.
		 * The minimum length of plaintext/AAD must be at least 1 byte, while the
		 * maximum length can be 0x1FFFFFFC bytes, which is the ASU DMA's maximum supported
		 * data transfer length.
		 */

		Status = XAsu_ValidateAadLen(AesClientParamPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XAsu_ValidateDataLen(AesClientParamPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** Validate the input data alignment. */
		if ((AesClientParamPtr->DataLen % XASU_AES_BLOCK_SIZE_IN_BYTES) != 0U) {
			if (XASU_AES_MODE_REQUIRES_ALIGNMENT(AesClientParamPtr->EngineMode) ||
					((AesClientParamPtr->OperationFlags & XASU_FINISH) != XASU_FINISH)) {
				Status = XASU_INVALID_ARGUMENT;
				goto END;
			}
		}

		/** Validate IsLast flag. */
		if ((AesClientParamPtr->IsLast != XASU_TRUE) && (AesClientParamPtr->IsLast != XASU_FALSE)) {
			Status = XASU_INVALID_ARGUMENT;
			goto END;
		}
	}

	/** Validate tag for AES final operation. */
	if ((AesClientParamPtr->OperationFlags & XASU_FINISH) == XASU_FINISH) {
		Status = XAsu_AesValidateTagParams(AesClientParamPtr->EngineMode, AesClientParamPtr->TagAddr,
			AesClientParamPtr->TagLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** Validate required parameters for AES update operation. */
	if (((AesClientParamPtr->OperationFlags & XASU_INIT) == XASU_INIT) &&
	    ((AesClientParamPtr->OperationFlags & XASU_FINISH) == XASU_FINISH) &&
	    ((AesClientParamPtr->OperationFlags & XASU_UPDATE) != XASU_UPDATE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Handle context operation. */
	Status = XAsu_HandleContextOperation(ClientParamPtr, &P0AesCtx, &P1AesCtx,
					     AesClientParamPtr->OperationFlags, &UniqueId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Cleanup on FINAL operation. */
	if ((AesClientParamPtr->OperationFlags & XASU_FINISH) == XASU_FINISH) {
		Status = XAsu_CleanupFinishOperation(ClientParamPtr, &P0AesCtx, &P1AesCtx,
						     UniqueId, NULL, 0U);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_AES_OPERATION_CMD_ID, UniqueId, XASU_MODULE_AES_ID,
				   (u8)(sizeof(XAsu_AesParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, AesClientParamPtr,
		sizeof(XAsu_AesParams), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to perform AES Known Answer Tests (KAT's).
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	KatMode		AES supported KAT mode (for standard mode : XASU_AES_ECB_MODE,
 * 				for mac mode : XASU_AES_GCM_MODE, and
 * 				for DPA mode : XASU_AES_KAT_DPA_MODE in CTR engine mode)
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 *
 *************************************************************************************************/
s32 XAsu_AesKat(XAsu_ClientParams *ClientParamsPtr, u32 KatMode)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate KAT mode. */
	if ((KatMode != XASU_AES_ECB_MODE) && (KatMode != XASU_AES_GCM_MODE) &&
			(KatMode != XASU_AES_KAT_DPA_MODE)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_AES_KAT_CMD_ID, UniqueId, XASU_MODULE_AES_ID,
				   (u8)(sizeof(KatMode) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamsPtr, &KatMode,
		sizeof(KatMode), Header);

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

	/** Validate that exactly one of KeyAddress or KeyId is provided. */
	if (XAsu_KmValidateKeyAddrNdKeyId(KeyObjectPtr->KeyAddress,
					  KeyObjectPtr->KeyId) != XST_SUCCESS) {
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

/*************************************************************************************************/
/**
 * @brief	This function validates operation flags for AES CCM mode.
 *
 * @param	AesClientParamPtr	Pointer to the XAsu_AesParams structure which contains AES
 *					AES input and output parameters.
 *
 * @return
 *		- XST_SUCCESS, if flags are valid for AES CCM mode.
 *		- XASU_INVALID_ARGUMENT, if flags are invalid.
 *
 *************************************************************************************************/
static inline s32 XAsu_ValidateCcmOpMode(const XAsu_AesParams *AesClientParamPtr)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	/**
	 * For CCM, INIT combined with UPDATE but without FINAL is not allowed.
	 * AadLen/DataLen on INIT encode the B0 block totals; if UPDATE is also set
	 * without FINAL, they carry per-chunk sizes instead, producing an invalid tag.
	 * INIT|UPDATE|FINAL (single-shot) is valid since totals equal chunk sizes.
	 */
	if (AesClientParamPtr->EngineMode != XASU_AES_CCM_MODE) {
		Status = XST_SUCCESS;
	} else {
		if (!(((AesClientParamPtr->OperationFlags & XASU_INIT) == XASU_INIT) &&
				((AesClientParamPtr->OperationFlags & XASU_UPDATE) == XASU_UPDATE) &&
				((AesClientParamPtr->OperationFlags & XASU_FINISH) != XASU_FINISH))) {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates AES Engine mode.
 *
 * @param	AesClientParamPtr	Pointer to the XAsu_AesParams structure which contains AES
 *					AES input and output parameters.
 *
 * @return
 *		- XST_SUCCESS, if AES Engine mode is valid.
 *		- XASU_INVALID_ARGUMENT, if mode is invalid.
 *
 *************************************************************************************************/
static inline s32 XAsu_ValidateAesEngineMode(const XAsu_AesParams *AesClientParamPtr)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	if (!((AesClientParamPtr->EngineMode > XASU_AES_GCM_MODE) &&
			(AesClientParamPtr->EngineMode != XASU_AES_CMAC_MODE) &&
			(AesClientParamPtr->EngineMode != XASU_AES_GHASH_MODE))) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates AES mode for AAD.
 *
 * @param	EngineMode	AES Engine mode.
 *
 * @return
 *		- XST_SUCCESS, if AES mode for AAD is valid.
 *		- XASU_INVALID_ARGUMENT, if AES mode for AAD is invalid.
 *
 *************************************************************************************************/
static inline s32 XAsu_IsModeValidForAad(u8 EngineMode)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	if ((EngineMode == XASU_AES_GCM_MODE) || (EngineMode == XASU_AES_CMAC_MODE) ||
		(EngineMode == XASU_AES_CCM_MODE)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates AAD length.
 *
 * @param	AesClientParamPtr	Pointer to the XAsu_AesParams structure which contains AES
 *					AES input and output parameters.
 *
 * @return
 *		- XST_SUCCESS, if AAD length is valid.
 *		- XASU_INVALID_ARGUMENT, if AAD length is invalid.
 *
 *************************************************************************************************/
static inline s32 XAsu_ValidateAadLen(const XAsu_AesParams *AesClientParamPtr)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	Status = XAsu_IsModeValidForAad(AesClientParamPtr->EngineMode);
	if (Status != XST_SUCCESS) {
		/** For standard modes, AAD length and address must be zero. */
		if ((AesClientParamPtr->AadLen == 0U) && (AesClientParamPtr->AadAddr == 0U)) {
			Status = XST_SUCCESS;
		}
	}
	else {
		/**
		 * In MAC only modes (AES-CMAC), AAD is mandatory: AadLen and AadAddr must be non-zero,
		 * within the allowed DMA limit, and block-size aligned for all intermediate
		 * transfers (unaligned length allowed only for final transfer).
		 *
		 * In GCM/CCM modes, AAD is optional: zero AAD is permitted when valid
		 * input/output data and data length are provided, allowing payload-only,
		 * AAD-only, or combined operation.
		 */
		Status = XASU_INVALID_ARGUMENT;
		if ((AesClientParamPtr->AadLen > 0U) && (AesClientParamPtr->AadAddr != 0U) &&
				(AesClientParamPtr->AadLen <= XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) {
			if (((AesClientParamPtr->AadLen % XASU_AES_BLOCK_SIZE_IN_BYTES) == 0U) ||
					((AesClientParamPtr->OperationFlags & XASU_FINISH) ==
					XASU_FINISH)) {
				Status = XST_SUCCESS;
			}
		}
		else if ((AesClientParamPtr->AadLen == 0U) && (AesClientParamPtr->AadAddr == 0U) &&
				((AesClientParamPtr->EngineMode == XASU_AES_GCM_MODE) ||
				(AesClientParamPtr->EngineMode == XASU_AES_CCM_MODE)) &&
				((AesClientParamPtr->InputDataAddr != 0U) &&
				(AesClientParamPtr->OutputDataAddr != 0U) &&
				(AesClientParamPtr->DataLen != 0U))) {
			Status = XST_SUCCESS;
		}
		else {
			/* MISRA C compliant. */
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates Data length.
 *
 * @param	AesClientParamPtr	Pointer to the XAsu_AesParams structure which contains AES
 *					AES input and output parameters.
 *
 * @return
 *		- XST_SUCCESS, if Data length is valid.
 *		- XASU_INVALID_ARGUMENT, if Data length is invalid.
 *
 *************************************************************************************************/
static inline s32 XAsu_ValidateDataLen(const XAsu_AesParams *AesClientParamPtr)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	if (XASU_AES_MODE_REQUIRES_DATA(AesClientParamPtr->EngineMode)) {
		/**
		 * For standard modes, data length and addresses must be present and
		 * length must be within maximum DMA limit.
		 */
		if ((AesClientParamPtr->DataLen == 0U) ||
				(AesClientParamPtr->DataLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) ||
				(AesClientParamPtr->InputDataAddr == 0U) ||
				(AesClientParamPtr->OutputDataAddr == 0U)) {
			goto END;
		}
	}
	else {
		/** If Data Length is zero, addresses must also be zero. */
		if (AesClientParamPtr->DataLen == 0U) {
			if ((AesClientParamPtr->InputDataAddr != 0U) ||
					(AesClientParamPtr->OutputDataAddr != 0U)) {
				goto END;
			}
		}
		else {
			/**
			 * If Data length is non-zero, both input and output addresses must be valid and
			 * data length must be within maximum DMA limit.
			 * AES-CMAC engine mode must not allow data updates.
			 */
			if ((AesClientParamPtr->InputDataAddr == 0U) ||
					(AesClientParamPtr->OutputDataAddr == 0U) ||
					(AesClientParamPtr->DataLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) ||
					(AesClientParamPtr->EngineMode == XASU_AES_CMAC_MODE)) {
				goto END;
			}
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Callback handler for AES client KAT operations. Sets the completion flag and
 *		captures the operation status via per-invocation state.
 *
 * @param	CallBackRef	Pointer to the caller's XAsu_AesKatCbState on stack.
 * @param	Status		Operation status returned by ASUFW.
 *
 *************************************************************************************************/
static void XAsu_AesKatCallBack(void *CallBackRef, u32 Status)
{
	XAsu_AesKatCbState *State = (XAsu_AesKatCbState *)CallBackRef;

	State->CallBackStatus = (s32)Status;
	State->Notify = 1U;
}

/*************************************************************************************************/
/**
 * @brief	Internal helper that executes a single AES client-side KAT. It sends an
 *		AES operation request to ASUFW, busy-waits for the response, and compares
 *		the result against expected vectors.
 *
 * @param	KatParams	Pointer to the KAT vector set describing the test case.
 *
 * @return
 *		- XST_SUCCESS, if the KAT passes.
 *		- XST_FAILURE, if the AES operation fails or output comparison fails.
 *
 *************************************************************************************************/
static s32 XAsu_AesRunClientKat(const XAsu_AesKatVectors *KatParams)
{
	s32 Status = XST_FAILURE;
	s32 SStatus = XST_FAILURE;
	u8 OutData[XASU_AES_KAT_DATA_LEN_IN_BYTES] = {0U};
	u8 Tag[XASU_AES_KAT_CCM_TAG_LEN_IN_BYTES] = {0U};
	XAsu_AesKeyObject KeyObj = {0U};
	XAsu_AesParams AesParams = {0U};
	XAsu_ClientParams ClientParams = {0U};
	XAsu_AesKatCbState KatState = {0U, (s32)XST_FAILURE};

	/**
	 * For authenticated decrypt, pre-fill the tag buffer with the
	 * expected tag so the server can verify it.
	 */
	if ((KatParams->EngineMode == XASU_AES_CCM_MODE) &&
	    (KatParams->OperationType == XASU_AES_DECRYPT_OPERATION)) {
		Status = Xil_SMemCpy(Tag, sizeof(Tag),
				     KatParams->ExpTag, KatParams->TagLen,
				     KatParams->TagLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** Configure key object. */
	KeyObj.KeyAddress = (u64)(UINTPTR)KatParams->Key;
	KeyObj.KeySize = XASU_AES_KEY_SIZE_256_BITS;
	KeyObj.KeySrc = XASU_AES_USER_KEY_7;
	KeyObj.KeyId = 0U;

	/** Configure AES parameters for single-shot operation. */
	AesParams.OperationFlags = (u8)(XASU_INIT | XASU_UPDATE |
					XASU_FINISH);
	AesParams.OperationType = KatParams->OperationType;
	AesParams.EngineMode = KatParams->EngineMode;
	AesParams.KeyObjectAddr = (u64)(UINTPTR)&KeyObj;
	AesParams.IvAddr = (u64)(UINTPTR)KatParams->Iv;
	AesParams.IvLen = KatParams->IvLen;
	AesParams.InputDataAddr = (u64)(UINTPTR)KatParams->InData;
	AesParams.OutputDataAddr = (u64)(UINTPTR)OutData;
	AesParams.DataLen = KatParams->DataLen;
	AesParams.IsLast = XASU_TRUE;
	AesParams.AadAddr = (u64)(UINTPTR)KatParams->Aad;
	AesParams.AadLen = KatParams->AadLen;
	AesParams.TagAddr = (KatParams->TagLen > 0U) ?
			    (u64)(UINTPTR)Tag : 0U;
	AesParams.TagLen = KatParams->TagLen;

	/** Configure client parameters for synchronous operation. */
	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_AesKatCallBack);
	ClientParams.CallBackRefPtr = (void *)&KatState;
	ClientParams.AdditionalStatus = (u32)XST_FAILURE;

	/** Send AES operation request to ASUFW. */
	Status = XAsu_AesOperation(&ClientParams, &AesParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Busy-wait for ASUFW response. */
	while (KatState.Notify == 0U) {
		/* Wait */
	}

	/** Check operation status from callback. */
	if (KatState.CallBackStatus != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	/** Compare output data with expected result. */
	Status = Xil_SMemCmp_CT(KatParams->ExpOutData, KatParams->DataLen,
				 OutData, KatParams->DataLen, KatParams->DataLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * For CCM, verify server-reported AdditionalStatus and compare
	 * generated tag with expected tag on encrypt.
	 */
	if (KatParams->EngineMode == XASU_AES_CCM_MODE) {
		if (KatParams->OperationType == XASU_AES_ENCRYPT_OPERATION) {
			/**
			 * For encrypt, the server writes the computed tag
			 * into the tag buffer and reports TAG_READ.
			 * Verify that status, then compare the tag.
			 */
			if (ClientParams.AdditionalStatus != XASU_AES_TAG_READ) {
				Status = XST_FAILURE;
				goto END;
			}
			Status = Xil_SMemCmp_CT(KatParams->ExpTag, KatParams->TagLen,
						Tag, KatParams->TagLen, KatParams->TagLen);
		} else {
			/**
			 * For decrypt, the server verifies the pre-filled
			 * tag internally and reports TAG_MATCHED on success.
			 */
			if (ClientParams.AdditionalStatus != XASU_AES_TAG_MATCHED) {
				Status = XST_FAILURE;
			}
		}
	}

END:
	/** Zeroize local output buffers. */
	SStatus = Xil_SecureZeroize(OutData,
				    XASU_AES_KAT_DATA_LEN_IN_BYTES);
	SStatus |= Xil_SecureZeroize(Tag,
				      XASU_AES_KAT_CCM_TAG_LEN_IN_BYTES);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs AES-CBC encryption KAT from the client side.
 *
 * @return
 *		- XST_SUCCESS, if AES-CBC encryption KAT passes.
 *		- XST_FAILURE, if the KAT fails.
 *
 *************************************************************************************************/
s32 XAsu_AesCbcEncryptKat(void)
{
	s32 Status = XST_FAILURE;
	const XAsu_AesKatVectors KatParams = {
		.Key = AesKatKey,
		.Iv = AesKatCbcIv,
		.IvLen = XASU_AES_KAT_CBC_IV_LEN_IN_BYTES,
		.Aad = NULL,
		.AadLen = 0U,
		.InData = AesKatPt,
		.ExpOutData = AesKatCbcCt,
		.DataLen = XASU_AES_KAT_DATA_LEN_IN_BYTES,
		.ExpTag = NULL,
		.TagLen = 0U,
		.EngineMode = XASU_AES_CBC_MODE,
		.OperationType = XASU_AES_ENCRYPT_OPERATION,
	};

	Status = XAsu_AesRunClientKat(&KatParams);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs AES-CBC decryption KAT from the client side.
 *
 * @return
 *		- XST_SUCCESS, if AES-CBC decryption KAT passes.
 *		- XST_FAILURE, if the KAT fails.
 *
 *************************************************************************************************/
s32 XAsu_AesCbcDecryptKat(void)
{
	s32 Status = XST_FAILURE;
	const XAsu_AesKatVectors KatParams = {
		.Key = AesKatKey,
		.Iv = AesKatCbcIv,
		.IvLen = XASU_AES_KAT_CBC_IV_LEN_IN_BYTES,
		.Aad = NULL,
		.AadLen = 0U,
		.InData = AesKatCbcCt,
		.ExpOutData = AesKatPt,
		.DataLen = XASU_AES_KAT_DATA_LEN_IN_BYTES,
		.ExpTag = NULL,
		.TagLen = 0U,
		.EngineMode = XASU_AES_CBC_MODE,
		.OperationType = XASU_AES_DECRYPT_OPERATION,
	};

	Status = XAsu_AesRunClientKat(&KatParams);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs AES-CCM encryption KAT from the client side.
 *
 * @return
 *		- XST_SUCCESS, if AES-CCM encryption KAT passes.
 *		- XST_FAILURE, if the KAT fails.
 *
 *************************************************************************************************/
s32 XAsu_AesCcmEncryptKat(void)
{
	s32 Status = XST_FAILURE;
	const XAsu_AesKatVectors KatParams = {
		.Key = AesKatKey,
		.Iv = AesKatCcmNonce,
		.IvLen = XASU_AES_KAT_CCM_NONCE_LEN_IN_BYTES,
		.Aad = AesKatCcmAad,
		.AadLen = XASU_AES_KAT_CCM_AAD_LEN_IN_BYTES,
		.InData = AesKatPt,
		.ExpOutData = AesKatCcmCt,
		.DataLen = XASU_AES_KAT_DATA_LEN_IN_BYTES,
		.ExpTag = AesKatCcmTag,
		.TagLen = XASU_AES_KAT_CCM_TAG_LEN_IN_BYTES,
		.EngineMode = XASU_AES_CCM_MODE,
		.OperationType = XASU_AES_ENCRYPT_OPERATION,
	};

	Status = XAsu_AesRunClientKat(&KatParams);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs AES-CCM decryption KAT from the client side.
 *
 * @return
 *		- XST_SUCCESS, if AES-CCM decryption KAT passes.
 *		- XST_FAILURE, if the KAT fails.
 *
 *************************************************************************************************/
s32 XAsu_AesCcmDecryptKat(void)
{
	s32 Status = XST_FAILURE;
	const XAsu_AesKatVectors KatParams = {
		.Key = AesKatKey,
		.Iv = AesKatCcmNonce,
		.IvLen = XASU_AES_KAT_CCM_NONCE_LEN_IN_BYTES,
		.Aad = AesKatCcmAad,
		.AadLen = XASU_AES_KAT_CCM_AAD_LEN_IN_BYTES,
		.InData = AesKatCcmCt,
		.ExpOutData = AesKatPt,
		.DataLen = XASU_AES_KAT_DATA_LEN_IN_BYTES,
		.ExpTag = AesKatCcmTag,
		.TagLen = XASU_AES_KAT_CCM_TAG_LEN_IN_BYTES,
		.EngineMode = XASU_AES_CCM_MODE,
		.OperationType = XASU_AES_DECRYPT_OPERATION,
	};

	Status = XAsu_AesRunClientKat(&KatParams);

	return Status;
}
/** @} */
