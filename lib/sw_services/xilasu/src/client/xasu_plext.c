/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_plext.c
 *
 * This file contains the implementation of the client interface functions for PL extension module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   08/21/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_plext_client_apis PL Extension Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_plext.h"
#include "xasu_def.h"

/************************************ Constant Definitions ***************************************/

/************************************ Type Definitions *******************************************/

/************************************ Macros (Inline Functions) Definitions **********************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_PlExtValidateKeyObjectParams(const XAsu_AesKeyObject *KeyObjectPtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform encryption operation on a given
 * 		payload data using a crypto block instantiated in the PL.
 *
 * 		The whole operation is performed in a single request, hence OperationFlags and
 * 		IsLast fields of PlExtClientParamPtr are not used. The key is taken from user
 * 		memory.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	PlExtClientParamPtr	Pointer to XAsu_AesParams structure which holds the
 * 				parameters of PL extension input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_PlExtOperation(XAsu_ClientParams *ClientParamPtr, XAsu_AesParams *PlExtClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate the input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (PlExtClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate the engine mode and the operation type supported by the PL crypto block. */
	if ((PlExtClientParamPtr->EngineMode != XASU_AES_ECB_MODE) ||
			(PlExtClientParamPtr->OperationType != XASU_AES_ENCRYPT_OPERATION)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate that IV, AAD and tag are unused for AES ECB mode on the PL path. */
	if ((PlExtClientParamPtr->IvObj.IvAddr != 0U) ||
			(PlExtClientParamPtr->IvObj.IvLen != 0U) ||
			(PlExtClientParamPtr->IvObj.IvId != 0U) ||
			(PlExtClientParamPtr->AadAddr != 0U) ||
			(PlExtClientParamPtr->AadLen != 0U) ||
			(PlExtClientParamPtr->TagAddr != 0U) ||
			(PlExtClientParamPtr->TagLen != 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate AES key object structure parameters. */
	Status = XAsu_PlExtValidateKeyObjectParams(
		(const XAsu_AesKeyObject *)(UINTPTR)PlExtClientParamPtr->KeyObjectAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Validate the input and output data addresses and length. The minimum length of the
	 * payload is one AES block, while the maximum length can be 0x1FFFFFFC bytes, which is the
	 * ASU DMA's maximum supported data transfer length. ECB mode input data should be 16Bytes
	 * aligned.
	 */
	if ((PlExtClientParamPtr->InputDataAddr == 0U) ||
			(PlExtClientParamPtr->OutputDataAddr == 0U) ||
			(PlExtClientParamPtr->DataLen == 0U) ||
			(PlExtClientParamPtr->DataLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) ||
			((PlExtClientParamPtr->DataLen % XASU_AES_BLOCK_SIZE_IN_BYTES) != 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_PLEXT_OPERATION_CMD_ID, UniqueId, XASU_MODULE_PLEXT_ID,
				   (u8)(sizeof(XAsu_AesParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, PlExtClientParamPtr, sizeof(XAsu_AesParams),
		Header);

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
 * 		- XST_SUCCESS, if validation of key object parameters is successful.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *
 *************************************************************************************************/
static s32 XAsu_PlExtValidateKeyObjectParams(const XAsu_AesKeyObject *KeyObjectPtr)
{
	s32 Status = XASU_INVALID_ARGUMENT;

	if (KeyObjectPtr == NULL) {
		goto END;
	}

	/** Validate that the key is provided from user memory. */
	if ((KeyObjectPtr->KeyAddress == 0U) || (KeyObjectPtr->KeyId != 0U)) {
		goto END;
	}

	/** Validate the key size and the key source supported by the PL crypto block. */
	if ((KeyObjectPtr->KeySize != XASU_AES_KEY_SIZE_256_BITS) ||
			(KeyObjectPtr->KeySrc != XASU_AES_USER_KEY_7)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
