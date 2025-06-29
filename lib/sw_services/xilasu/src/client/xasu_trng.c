/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_trng.c
 *
 * This file contains the implementation of the client interface functions for
 * TRNG driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  08/27/24 Initial release
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ma   02/03/25 Pass UniqueId as part of header in XAsu_TrngGetRandomNum API.
 *       ma   02/07/25 Added DRBG support in client
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_trng_client_apis TRNG Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_def.h"
#include "xasu_trnginfo.h"
#include "xasu_trng.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to generate the random number using TRNG.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds
 * 				client input arguments.
 * @param	BufPtr		Pointer to the buffer to store the random number.
 * @param	Length		Length of the buffer in bytes.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_TrngGetRandomNum(XAsu_ClientParams *ClientParamPtr, const u8 *BufPtr, u32 Length)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if ((BufPtr == NULL) || (Length == 0U) || (Length > XASU_TRNG_RANDOM_NUM_IN_BYTES)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate unique ID and register the callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, BufPtr, Length, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}
	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_TRNG_GET_RANDOM_BYTES_CMD_ID, UniqueId,
				   XASU_MODULE_TRNG_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to run the TRNG Known Answer Tests (KAT's).
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds
 * 				client input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_QUEUE_FULL, if Queue buffer is full.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_TrngKat(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate unique ID and register callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_TRNG_KAT_CMD_ID, UniqueId, XASU_MODULE_TRNG_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}

#ifdef XASU_TRNG_ENABLE_DRBG_MODE
/*************************************************************************************************/
/**
 * @brief	This function sends TRNG DRBG instantiate command to ASUFW.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds client input
 *                          arguments.
 * @param	CmdParamsPtr	Pointer to the DRBG instantiate command parameters.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
 s32 XAsu_TrngDrbgInstantiate(XAsu_ClientParams *ClientParamPtr,
		XAsu_DrbgInstantiateCmd *CmdParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate unique ID and register callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_TRNG_DRBG_INSTANTIATE_CMD_ID, UniqueId, XASU_MODULE_TRNG_ID, 0U);

	/** Send IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, CmdParamsPtr,
			sizeof(XAsu_DrbgInstantiateCmd), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends TRNG DRBG reseed command to ASUFW.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds client input
 *                          arguments.
 * @param	CmdParamsPtr	Pointer to the DRBG reseed command parameters.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
 s32 XAsu_TrngDrbgReseed(XAsu_ClientParams *ClientParamPtr, XAsu_DrbgReseedCmd *CmdParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate unique ID and register the callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}
	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_TRNG_DRBG_RESEED_CMD_ID, UniqueId, XASU_MODULE_TRNG_ID, 0U);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, CmdParamsPtr,
			sizeof(XAsu_DrbgReseedCmd), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends TRNG DRBG generate command to ASUFW.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds client input
 *                          arguments.
 * @param	CmdParamPtr		Pointer to the DRBG generate command parameters.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XST_FAILURE, if sending IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_TrngDrbgGenerate(XAsu_ClientParams *ClientParamPtr, XAsu_DrbgGenerateCmd *CmdParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate unique ID and register the callback. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}
	/** create command header. */
	Header = XAsu_CreateHeader(XASU_TRNG_DRBG_GENERATE_CMD_ID, UniqueId,
				   XASU_MODULE_TRNG_ID, 0U);
	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, CmdParamsPtr,
			sizeof(XAsu_DrbgGenerateCmd), Header);

END:
	return Status;
}
#endif /* XASU_TRNG_ENABLE_DRBG_MODE */
/** @} */
