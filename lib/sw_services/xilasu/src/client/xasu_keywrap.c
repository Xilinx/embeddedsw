/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keywrap.c
 *
 * This file contains the implementation of the client interface functions for key wrap unwrap
 * module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   02/24/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasu_keywrap_client_apis Keywrap Client APIs
* @{
*/

/*************************************** Include Files *******************************************/
#include "xasu_keywrap.h"
#include "xasu_def.h"
#include "xasu_status.h"
#include "xasu_keywrap_common.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function wraps a given key using a specified AES wrapping key.
 *
 * @param	ClientParamsPtr		Pointer to the XAsu_ClientParams structure which holds
 * 					the client input parameters.
 * @param	KeyWrapParamsPtr	Pointer to XAsu_KeyWrapParams structure which holds the
 * 					parameters of key wrap input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KeyWrap(XAsu_ClientParams *ClientParamsPtr, XAsu_KeyWrapParams *KeyWrapParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 CmdId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_KeyWrapUnwrapValidateInputParams(KeyWrapParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (KeyWrapParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_KEYWRAP_KEY_WRAP_SHA2_CMD_ID;
	} else if (KeyWrapParamsPtr->ShaType == XASU_SHA3_TYPE) {
		CmdId = XASU_KEYWRAP_KEY_WRAP_SHA3_CMD_ID;
	} else {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_KEYWRAP_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, KeyWrapParamsPtr,
						(u32)(sizeof(XAsu_KeyWrapParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function unwraps a given key using a specified AES wrapping key.
 *
 * @param	ClientParamsPtr		Pointer to the XAsu_ClientParams structure which holds
 * 					the client input parameters.
 * @param	KeyUnwrapParamsPtr	Pointer to XAsu_KeyWrapParams structure which holds the
 * 					parameters of key wrap input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KeyUnwrap(XAsu_ClientParams *ClientParamsPtr, XAsu_KeyWrapParams *KeyUnwrapParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;
	u8 CmdId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_KeyWrapUnwrapValidateInputParams(KeyUnwrapParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (KeyUnwrapParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CmdId = XASU_KEYWRAP_KEY_UNWRAP_SHA2_CMD_ID;
	} else if (KeyUnwrapParamsPtr->ShaType == XASU_SHA3_TYPE) {
		CmdId = XASU_KEYWRAP_KEY_UNWRAP_SHA3_CMD_ID;
	} else {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(CmdId, UniqueId, XASU_MODULE_KEYWRAP_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, KeyUnwrapParamsPtr,
						(u32)(sizeof(XAsu_KeyWrapParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs Key Wrap Unwrap Known Answer Tests (KAT's).
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 *
 * @return
 * 	- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 	- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 	- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KeyWrapKat(XAsu_ClientParams *ClientParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	Header = XAsu_CreateHeader(XASU_KEYWRAP_KAT_CMD_ID, UniqueId, XASU_MODULE_KEYWRAP_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}

/** @} */
