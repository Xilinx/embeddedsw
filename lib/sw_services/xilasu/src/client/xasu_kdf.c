/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_kdf.c
 *
 * This file contains the implementation of the client interface functions for HMAC based Key
 * Derivation Function (HKDF) module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/21/25 Initial release
 *       rmv  09/11/25 Move KDF parameter validation function to common file
 *       kp   02/26/26 Added client-side KDF SHA3-256 KAT
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_kdf_client_apis KDF Client APIs
 * @{
*/

/*************************************** Include Files *******************************************/
#include "xasu_kdf.h"
#include "xasu_def.h"
#include "xasu_shainfo.h"
#include "xasu_hmacinfo.h"
#include "xasu_kdf_common.h"
#include "xil_sutil.h"
#include "xasu_aesinfo.h"

/************************************ Constant Definitions ***************************************/
#define XASU_KDF_KAT_KEY_LEN_IN_BYTES		(32U)	/**< KDF KAT key length in bytes */
#define XASU_KDF_KAT_CTX_LEN_IN_BYTES		(32U)	/**< KDF KAT context length in bytes */
#define XASU_KDF_KAT_SHA3_256_OUT_LEN		(32U)	/**< KDF KAT SHA3-256 output length
										in bytes */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_ValidateCmacKdfParameters(const XAsu_CmacKdfParams *CmacKdfParamsPtr);

/************************************ Variable Definitions ***************************************/

/* KDF KAT input key - same as server-side EccPrivKey */
static const u8 KdfKatKey[XASU_KDF_KAT_KEY_LEN_IN_BYTES] = {
	0x22U, 0x17U, 0x96U, 0x4FU, 0xB2U, 0x14U, 0x35U, 0x33U,
	0xBAU, 0x93U, 0xAAU, 0x35U, 0xFEU, 0x09U, 0x37U, 0xA6U,
	0x69U, 0x5EU, 0x20U, 0x87U, 0x27U, 0x07U, 0x06U, 0x44U,
	0x99U, 0x21U, 0x7CU, 0x5FU, 0x6AU, 0xB8U, 0x09U, 0xDFU
};

/* KDF KAT context - same as server-side KatMessage */
static const u8 KdfKatCtx[XASU_KDF_KAT_CTX_LEN_IN_BYTES] = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U,
	0xC3U, 0x5AU, 0x83U, 0xBCU, 0xE5U, 0x63U, 0x2CU, 0xE7U,
	0x89U, 0x43U, 0x6CU, 0x68U, 0x82U, 0xCAU, 0x1CU, 0x71U
};

/* Expected KDF output with HMAC-SHA3-256 in counter mode */
static const u8 ExpKdfSha3_256[XASU_KDF_KAT_SHA3_256_OUT_LEN] = {
	0x09U, 0x9DU, 0xCAU, 0xB0U, 0xF6U, 0xD7U, 0x0FU, 0x41U,
	0x66U, 0x8FU, 0xEBU, 0x22U, 0xE9U, 0xFEU, 0x9DU, 0x0AU,
	0x82U, 0x9BU, 0x6CU, 0x5BU, 0x19U, 0x4DU, 0x79U, 0x29U,
	0xD7U, 0x8AU, 0x99U, 0xB8U, 0xE6U, 0xBDU, 0x51U, 0x23U
};

/** Per-invocation KAT callback state, passed through CallBackRefPtr. */
typedef struct {
	volatile u8 Notify;		/**< Completion flag */
	volatile s32 CallBackStatus;	/**< Callback status */
} XAsu_KdfKatCbState;

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to generate the derived key of specified key
 * length by using the Key Derivative Function (KDF) in counter mode.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	KdfParamsPtr	Pointer to XAsu_KdfParams structure which holds the parameters of
 * 				KDF input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KdfGenerate(XAsu_ClientParams *ClientParamsPtr, XAsu_KdfParams *KdfParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 CommandId;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_ValidateKdfParameters(KdfParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0x0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Get the command ID based on SHA type. */
	if (KdfParamsPtr->ShaType == XASU_SHA2_TYPE) {
		CommandId = XASU_KDF_GENERATE_SHA2_CMD_ID;
	} else {
		CommandId = XASU_KDF_GENERATE_SHA3_CMD_ID;
	}
	Header = XAsu_CreateHeader(CommandId, UniqueId, XASU_MODULE_KDF_ID,
				   (u8)(sizeof(XAsu_KdfParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamsPtr, KdfParamsPtr,
						(u32)(sizeof(XAsu_KdfParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to generate the derived key of specified key
 * length by using the Key Derivative Function (KDF) in counter mode with CMAC as PRF.
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	CmacKdfParamsPtr Pointer to XAsu_CmacKdfParams structure which holds the parameters
 * 				of KDF input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_CmacKdfGenerate(XAsu_ClientParams *ClientParamsPtr, XAsu_CmacKdfParams *CmacKdfParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_ValidateCmacKdfParameters(CmacKdfParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0x0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header for the CMAC KDF command. */
	Header = XAsu_CreateHeader(XASU_KDF_CMAC_CMD_ID, UniqueId, XASU_MODULE_KDF_ID,
				   (u8)(sizeof(XAsu_CmacKdfParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamsPtr, CmacKdfParamsPtr,
						(u32)(sizeof(XAsu_CmacKdfParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to generate the derived key of specified key
 * length by using the HKDF (HMAC-based Extract-and-Expand Key Derivation Function).
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	HkdfParamsPtr	Pointer to XAsu_HkdfParams structure which holds the parameters of
 * 				KDF input arguments.
 *
 * @return
 * 		- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 		- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 		- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_HKdfGenerate(XAsu_ClientParams *ClientParamsPtr, XAsu_HkdfParams *HkdfParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 CommandId;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (HkdfParamsPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate HMAC KDF parameters. */
	Status = XAsu_ValidateKdfParameters(&HkdfParamsPtr->KdfParams);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}
	/* Key manager is not supported for HKDF, KeyId must be 0. */
	if(HkdfParamsPtr->KdfParams.KeyObject.KeyId != 0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamsPtr, NULL, 0x0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Get the command ID based on SHA type. */
	if (HkdfParamsPtr->KdfParams.ShaType == XASU_SHA2_TYPE) {
		CommandId = XASU_KDF_HKDF_SHA2_CMD_ID;
	} else {
		CommandId = XASU_KDF_HKDF_SHA3_CMD_ID;
	}
	Header = XAsu_CreateHeader(CommandId, UniqueId, XASU_MODULE_KDF_ID,
				   (u8)(sizeof(XAsu_HkdfParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamsPtr, HkdfParamsPtr,
						(u32)(sizeof(XAsu_HkdfParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform KDF Known Answer Tests (KAT's).
 *
 * @param	ClientParamsPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 *
 * @return
 * 	- XST_SUCCESS, if IPI request to ASU is sent successfully.
 * 	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 	- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 * 	- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_KdfKat(XAsu_ClientParams *ClientParamsPtr)
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

	Header = XAsu_CreateHeader(XASU_KDF_KAT_CMD_ID, UniqueId, XASU_MODULE_KDF_ID,
				   XASU_CMD_LEN_ZERO, ClientParamsPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASU. */
	Status = XAsu_SendCmdToAsu(ClientParamsPtr, NULL, 0U, Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Callback function for KDF client-side KAT. Sets the notify flag and stores
 *		the completion status via per-invocation state.
 *
 * @param	CallBackRefPtr	Pointer to the caller's XAsu_KdfKatCbState on stack.
 * @param	Status		Completion status from the server.
 *
 *************************************************************************************************/
static void XAsu_KdfKatCallBack(void *CallBackRefPtr, u32 Status)
{
	XAsu_KdfKatCbState *State = (XAsu_KdfKatCbState *)CallBackRefPtr;

	State->CallBackStatus = (s32)Status;
	State->Notify = 1U;
}

/*************************************************************************************************/
/**
 * @brief	This function runs the client-side KDF KAT using SHA3-256.
 *		It calls XAsu_KdfGenerate, busy-waits for the callback, and compares the
 *		derived key output against the expected value.
 *
 * @return
 *	- XST_SUCCESS, if KDF SHA3-256 KAT passes.
 *	- XST_FAILURE, if KDF computation or comparison fails.
 *
 *************************************************************************************************/
s32 XAsu_KdfSha3Kat(void)
{
	s32 Status = XST_FAILURE;
	s32 SStatus = XST_FAILURE;
	XAsu_ClientParams ClientParams = {0U};
	XAsu_KdfParams KdfParams = {0U};
	u8 KdfOutput[XASU_SHA_SHAKE_256_HASH_LEN] = {0U};
	XAsu_KdfKatCbState KatState = {0U, (s32)XST_FAILURE};

	/** Set up client parameters with KAT callback. */
	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr = XAsu_KdfKatCallBack;
	ClientParams.CallBackRefPtr = (void *)&KatState;

	/** Set up KDF parameters. */
	KdfParams.ShaType = XASU_SHA3_TYPE;
	KdfParams.ShaMode = XASU_SHA_MODE_256;
	KdfParams.KeyObject.KeyInAddr = (u64)(UINTPTR)KdfKatKey;
	KdfParams.KeyObject.KeyInLen = XASU_KDF_KAT_KEY_LEN_IN_BYTES;
	KdfParams.KeyObject.KeyId = 0U;
	KdfParams.ContextAddr = (u64)(UINTPTR)KdfKatCtx;
	KdfParams.ContextLen = XASU_KDF_KAT_CTX_LEN_IN_BYTES;
	KdfParams.KeyOutAddr = (u64)(UINTPTR)KdfOutput;
	KdfParams.KeyOutLen = XASU_KDF_KAT_SHA3_256_OUT_LEN;

	/** Send KDF generate request to the server. */
	Status = XAsu_KdfGenerate(&ClientParams, &KdfParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Busy-wait for the server response. */
	while (KatState.Notify == 0U) {
		/* Wait */
	}

	/** Check the callback status. */
	if (KatState.CallBackStatus != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	/** Compare the derived key against the expected value. */
	Status = Xil_SMemCmp_CT(ExpKdfSha3_256,
		XASU_KDF_KAT_SHA3_256_OUT_LEN, KdfOutput,
		XASU_KDF_KAT_SHA3_256_OUT_LEN,
		XASU_KDF_KAT_SHA3_256_OUT_LEN);

END:
	/** Zeroize local output buffer. */
	SStatus = Xil_SecureZeroize(KdfOutput, XASU_KDF_KAT_SHA3_256_OUT_LEN);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the CMAC KDF input parameters.
 *
 * @param	CmacKdfParamsPtr Pointer to XAsu_CmacKdfParams structure which holds the parameters of CMAC KDF input arguments.
 *
 * @return
 * 	- XST_SUCCESS, if CMAC KDF input parameters validation is successful.
 * 	- XST_FAILURE, if CMAC KDF input parameters validation fails.
 *
 *************************************************************************************************/
static s32 XAsu_ValidateCmacKdfParameters(const XAsu_CmacKdfParams *CmacKdfParamsPtr)
{
	s32 Status = XST_FAILURE;

	/** Validate CMAC KDF parameters pointer. */
	if (CmacKdfParamsPtr == NULL) {
		goto END;
	}

	/** Key manager is not supported for CMAC KDF, KeyId must be 0. */
	if (CmacKdfParamsPtr->KdfParams.KeyObject.KeyId != 0U) {
		goto END;
	}

	/** Validate CMAC KDF parameters. */
	if ((CmacKdfParamsPtr->KdfParams.KeyObject.KeyInAddr == 0U) || (CmacKdfParamsPtr->KdfParams.ContextAddr == 0U) ||
	    (CmacKdfParamsPtr->KdfParams.ContextLen == 0U) || (CmacKdfParamsPtr->KdfParams.ContextLen > XASU_KDF_MAX_CONTEXT_LEN) ||
	    (CmacKdfParamsPtr->KdfParams.KeyOutAddr == 0U) || (CmacKdfParamsPtr->KdfParams.KeyOutLen == 0U)) {
		goto END;
	}

	/** Validate that the input key length is either 128 bits or 256 bits for CMAC KDF. */
	if ((CmacKdfParamsPtr->KdfParams.KeyObject.KeyInLen != XASU_AES_KEY_SIZE_128_BITS) &&
	    (CmacKdfParamsPtr->KdfParams.KeyObject.KeyInLen != XASU_AES_KEY_SIZE_256_BITS)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
