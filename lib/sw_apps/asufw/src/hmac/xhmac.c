/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xhmac.c
*
* This file contains the implementation of the Hash-Based Message Authentication Code(HMAC) APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   yog  01/02/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xhmac_server_apis HMAC Server APIs
* @{
*/

/*************************************** Include Files *******************************************/
#include "xhmac.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"
#include "xasu_hmacinfo.h"

/************************************** Type Definitions *****************************************/
/** This typedef is used to update the state of HMAC. */
typedef enum {
	XHMAC_INITIALIZED = 0x1, /**< HMAC in initialized state */
	XHMAC_STARTED, /**< HMAC in start state */
	XHMAC_UPDATE_IN_PROGRESS, /**< HMAC is in progress state during multiple data chunk updates */
	XHMAC_UPDATE_COMPLETED, /**< HMAC is in completed state after the final data chunk update */
} XHmac_State;

struct _XHmac {
	XSha *ShaInstancePtr; /**< SHA Instance Pointer. */
	u8 BlockLen; /**< SHA block length. */
	u8 ShaMode; /**< SHA mode. */
	u8 HashBufLen; /**< SHA hash buffer length. */
	u8 Reserved; /**< Reserved */
	u8 IntHash[XASUFW_HMAC_SHA_HASH_MAX_LEN]; /**< Buffer to store intermediate hash. */
	u8 OPadRes[XASUFW_SHA3_256_BLOCK_LEN]; /**< Buffer to store OPAD result. */
	XHmac_State HmacState; /**< HMAC current state. */
};

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
static s32 XHmac_ProcessKeyWithPadding(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, u8 ShaType,
				       u64 KeyAddr, u32 KeyLen, u8 *ProcessedKey);
static void XHmac_Xor(const u32 *Data, const u8 Value, u32 *Result, u32 BlockLen);

/************************************** Macros Definitions ***************************************/
#define XASUFW_HMAC_8BIT_SHIFT		(8U) /**< Macro to shift for 8 bits. */
#define XASUFW_HMAC_16BIT_SHIFT		(16U) /**< Macro to shift for 16 bits. */
#define XASUFW_HMAC_24BIT_SHIFT		(24U) /**< Macro to shift for 24 bits. */
#define XASUFW_HMAC_IPAD_VALUE		(0x36U) /**< Each byte os Key provided is XOR'ed with this
							IPAD value. */
#define XASUFW_HMAC_OPAD_VALUE		(0x5CU) /**< Each byte os Key provided is XOR'ed with this
							OPAD value. */

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function returns HMAC instance pointer.
 *
 * @return
 * 		- It returns pointer to the XHmac_Instance.
 *
 *************************************************************************************************/
XHmac *XHmac_GetInstance(void)
{
	/** Define a variable for XHmac structure and return the address of it. */
	static XHmac XHmac_Instance = {0U};

	return &XHmac_Instance;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes the HMAC instance.
 *
 * @param	InstancePtr	Pointer to the HMAC instance.
 *
 * @return
*		- XASUFW_SUCCESS, if initialization is successful.
*		- XASUFW_HMAC_INVALID_PARAM, if InstancePtr is NULL.
 *
 *************************************************************************************************/
s32 XHmac_CfgInitialize(XHmac *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_HMAC_INVALID_PARAM;
		goto END;
	}

	/** Update HMAC state to INITIALIZED. */
	InstancePtr->HmacState = XHMAC_INITIALIZED;
	Status = XASUFW_SUCCESS;

END:
	return Status;

}
/*************************************************************************************************/
/**
 *
 * @brief	This function performs init operation of the HMAC.
 *
 * @param	InstancePtr	Pointer to the XHmac instance.
 * @param	AsuDmaPtr	Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	KeyAddr		Address which holds the key for HMAC.
 * @param	KeyLen		Variable which holds the length of the key.
 * @param	ShaMode		SHA mode selection.
 * @param	ShaType		SHA type selection.
 * @param	HashLen		Length of the HASH for the provided SHA type and mode.
 *
 * @return
 * 	- XASUFW_SUCCESS, if initialization was successful.
 * 	- XASUFW_HMAC_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_HMAC_STATE_MISMATCH_ERROR, if HMAC state is mismatched.
 * 	- XASUFW_HMAC_INVALID_HASHLEN, if input hashlength is invalid.
 *
 *************************************************************************************************/
s32 XHmac_Init(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, XSha *ShaInstancePtr, u64 KeyAddr,
	       u32 KeyLen, u8 ShaMode, u8 ShaType, u32 HashLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 K0[XASUFW_SHA3_256_BLOCK_LEN] = {0U};
	u32 ExpHashLen = 0U;

	/** Validate input parameters. */
	if ((InstancePtr == NULL) || (AsuDmaPtr == NULL) || (ShaInstancePtr == NULL) ||
	    (KeyAddr == 0U) || (KeyLen == 0U)) {
		Status = XASUFW_HMAC_INVALID_PARAM;
		goto END;
	}

	if ((ShaType != XASU_SHA2_TYPE) && (ShaType != XASU_SHA3_TYPE)) {
		Status = XASUFW_HMAC_INVALID_PARAM;
		goto END;
	}

	if ((ShaMode != XASU_SHA_MODE_SHA256) && (ShaMode != XASU_SHA_MODE_SHA384) &&
	    (ShaMode != XASU_SHA_MODE_SHA512) && (ShaMode != XASU_SHA_MODE_SHAKE256)) {
		Status = XASUFW_HMAC_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->HmacState != XHMAC_INITIALIZED) {
		Status = XASUFW_HMAC_STATE_MISMATCH_ERROR;
		goto END;
	}
	/** Get hash length based on the SHA mode to validate the input hash length. */
	Status = XSha_GetHashLen((u8)ShaMode, &ExpHashLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Validate the hash length. */
	if (HashLen != ExpHashLen) {
		Status = XASUFW_HMAC_INVALID_HASHLEN;
		goto END;
	}

	InstancePtr->ShaInstancePtr = ShaInstancePtr;
	InstancePtr->HashBufLen = (u8)HashLen;
	InstancePtr->ShaMode = ShaMode;

	/** Preprocess key. */
	Status = XHmac_ProcessKeyWithPadding(InstancePtr, AsuDmaPtr, ShaType, KeyAddr, KeyLen,
					     (u8 *)K0);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	/** Initialize Sha engine to calculate the hash on IPad || Data. */
	Status = XSha_Start(InstancePtr->ShaInstancePtr, (u32)InstancePtr->ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	/** Update SHA with IPad data. */
	Status = XSha_Update(InstancePtr->ShaInstancePtr, AsuDmaPtr,
			     (u64)(UINTPTR)K0, (u32)InstancePtr->BlockLen, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	/** Update HMAC state to started. */
	InstancePtr->HmacState = XHMAC_STARTED;

END_CLR:
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, K0, XASUFW_SHA3_256_BLOCK_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	if ((Status != XASUFW_SUCCESS) && (InstancePtr != NULL)) {
		InstancePtr->HmacState = XHMAC_INITIALIZED;
		XSha_Reset(InstancePtr->ShaInstancePtr);
	}

	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function updates the input data to SHA engine and
 * 		calculates the digest for (iPad || Data).
 *
 * @param	InstancePtr	Pointer to the XHmac instance.
 * @param	AsuDmaPtr	Pointer to the XAsufw_Dma instance.
 * @param	DataAddr	Address which holds the input message.
 * @param	DataLen		Variable which holds the length of the message.
 * @param	IsLastUpdate	Variable which indicates the last update.
 *
 * @return
 * 	- XASUFW_SUCCESS, if update was successful.
 * 	- XASUFW_HMAC_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_HMAC_STATE_MISMATCH_ERROR, if HMAC state is mismatched.
 *
 *************************************************************************************************/
s32 XHmac_Update(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, u64 DataAddr, u32 DataLen,
		 u32 IsLastUpdate)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	static u32 CmdStage = 0x0U;

	/** Validate input parameters. */
	if ((InstancePtr == NULL) || (AsuDmaPtr == NULL) || (DataAddr == 0U)) {
		Status = XASUFW_HMAC_INVALID_PARAM;
		goto END;
	}

	if ((IsLastUpdate != XASU_TRUE) && (IsLastUpdate != XASU_FALSE)) {
		Status = XASUFW_HMAC_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->HmacState != XHMAC_STARTED)
	    && (InstancePtr->HmacState != XHMAC_UPDATE_IN_PROGRESS)) {
		Status = XASUFW_HMAC_STATE_MISMATCH_ERROR;
		goto END;
	}

	if (CmdStage != 0U) {
		goto SHA_IN_HMAC_STAGE_UPDATE_DONE;
	}
	/** Update SHA with input data for which HMAC has to be calculated. */
	Status = XSha_Update(InstancePtr->ShaInstancePtr, AsuDmaPtr, DataAddr,
			     DataLen, IsLastUpdate);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		CmdStage = SHA_UPDATE_IN_HMAC_UPDATE_DONE;
		goto DONE;
	} else if (Status != XASUFW_SUCCESS) {
		goto END;
	} else {
		/* Do nothing */
	}

SHA_IN_HMAC_STAGE_UPDATE_DONE:
	if (IsLastUpdate == XASU_TRUE) {
		/**
		 * If this is the last update, get the HASH calculated for input (iPad || Data)
		 * and update HMAC state to update completed state.
		*/
		Status = XSha_Finish(InstancePtr->ShaInstancePtr, (u32 *)(InstancePtr->IntHash),
				     (u32)InstancePtr->HashBufLen, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		InstancePtr->HmacState = XHMAC_UPDATE_COMPLETED;
	} else {
		/** If this is not the last update, update HMAC state to update still in progress. */
		InstancePtr->HmacState = XHMAC_UPDATE_IN_PROGRESS;
	}
END:
	if ((Status != XASUFW_SUCCESS) && (InstancePtr != NULL)) {
		InstancePtr->HmacState = XHMAC_INITIALIZED;
		XSha_Reset(InstancePtr->ShaInstancePtr);
	}
DONE:
	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function calculates the final HMAC.
 *
 * @param	InstancePtr	Pointer to the XHmac instance.
 * @param	AsuDmaPtr	Pointer to the XAsufw_Dma instance.
 * @param	HmacPtr		Pointer to store the HMAC output.
 *
 * @return
 * 	- XASUFW_SUCCESS, on successful calculation of HMAC.
 * 	- XASUFW_HMAC_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_HMAC_STATE_MISMATCH_ERROR, if HMAC state is mismatched.
 *
 *************************************************************************************************/
s32 XHmac_Final(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, u32 *HmacPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);

	/** Validate input parameters. */
	if ((InstancePtr == NULL) || (AsuDmaPtr == NULL) || (HmacPtr == NULL)) {
		Status = XASUFW_HMAC_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->HmacState != XHMAC_UPDATE_COMPLETED) {
		Status = XASUFW_HMAC_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Initialize Sha engine to calculate the hash on Opad || IntHash. */
	Status = XSha_Start(InstancePtr->ShaInstancePtr, (u32)InstancePtr->ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Update SHA with OPad data. */
	Status = XSha_Update(InstancePtr->ShaInstancePtr, AsuDmaPtr,
			     (u64)(UINTPTR)(InstancePtr->OPadRes), (u32)InstancePtr->BlockLen, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Update SHA with IntHash = (HASH(IPad || input)) data. */
	Status = XSha_Update(InstancePtr->ShaInstancePtr, AsuDmaPtr,
			     (u64)(UINTPTR)(InstancePtr->IntHash), (u32)InstancePtr->HashBufLen, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Get the final HMAC output = HASH(OPad || IntHash). */
	Status = XSha_Finish(InstancePtr->ShaInstancePtr, (u32 *)HmacPtr, (u32)InstancePtr->HashBufLen,
			     XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

END:
	if (InstancePtr != NULL) {
		/** Zeroize local IntHash copy. */
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus,
			  (InstancePtr->IntHash), XASUFW_HMAC_SHA_HASH_MAX_LEN);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

		/** Zeroize local OpadRes copy. */
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus,
			  (InstancePtr->OPadRes), XASUFW_SHA3_256_BLOCK_LEN);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

		/** Update the HMAC state to initialized. */
		InstancePtr->HmacState = XHMAC_INITIALIZED;
		XSha_Reset(InstancePtr->ShaInstancePtr);
	}

	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function preprocesses the key to SHA block length as per the standard based
 * 		on the length comparision between provided KeyLen and the SHA block length.
 *
 * @param	InstancePtr	Pointer to the XHmac instance.
 * @param	AsuDmaPtr	Pointer to the XAsufw_Dma instance.
 * @param	ShaType		SHA type selection.
 * @param	KeyAddr		is the address which holds the key for HMAC.
 * @param	KeyLen		variable holds the length of the key.
 * @param	ProcessedKey	Pointer to hold the processed key.
 *
 * @return
 * 	- XASUFW_SUCCESS, if preprocessing key was successful.
 *
 *************************************************************************************************/
static s32 XHmac_ProcessKeyWithPadding(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, u8 ShaType,
				       u64 KeyAddr, u32 KeyLen, u8 *ProcessedKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 PaddingIndex = 0U;

	/** Get block length based on the sha mode and type. */
	InstancePtr->BlockLen = XSha_GetShaBlockLen(ShaType, InstancePtr->ShaMode);

	if (KeyLen > InstancePtr->BlockLen) {
		/*
		 * If provided key length is greater than the calculated block length,
		 *  - Calculate hash on key and append with zero to make K0 to the length of the
		 *    block length.
		 */
		Status = XSha_Start(InstancePtr->ShaInstancePtr, (u32)InstancePtr->ShaMode);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		Status = XSha_Update(InstancePtr->ShaInstancePtr, AsuDmaPtr, KeyAddr,
				     KeyLen, XASU_TRUE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		Status = XSha_Finish(InstancePtr->ShaInstancePtr, (u32 *)ProcessedKey,
				     (u32)InstancePtr->HashBufLen, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		PaddingIndex = InstancePtr->HashBufLen;
	} else {
		/**
		 * If the key provided is less than the calculated block length,
		 *  - Append Zeros to the key provided.
		 * If Key provided is equal to block length,
		 *  - The key provided is the processed key.
		 */
		Status = XAsufw_DmaXfr(AsuDmaPtr, KeyAddr, (u64)(UINTPTR)ProcessedKey, KeyLen,
				       0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		PaddingIndex = (u8)KeyLen;
	}

	/** Pad the key with zeros to ensure the key length matches the block length. */
	if (KeyLen < InstancePtr->BlockLen) {
		Status = Xil_SMemSet((ProcessedKey + PaddingIndex), XASUFW_SHA3_256_BLOCK_LEN, 0x0U,
				  (u32)(InstancePtr->BlockLen - PaddingIndex));
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/** Calculate OPad by xor'ing each byte of K0 with oPad value. */
	XHmac_Xor((u32 *)ProcessedKey, XASUFW_HMAC_OPAD_VALUE, (u32 *)InstancePtr->OPadRes,
		  (u32)InstancePtr->BlockLen);

	/** Calculate IPad by xor'ing each byte of K0 with iPad value. */
	XHmac_Xor((u32 *)ProcessedKey, XASUFW_HMAC_IPAD_VALUE, (u32 *)ProcessedKey,
		  (u32)InstancePtr->BlockLen);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function performs XOR operation on provided data of SHA block length
 *		with constant provided on whole bytes of data and result is been updated.
 *
 * @param	Data	is the pointer which holds data to be XORed.
 * @param	Value	with which XOR operation to be performed.
 * @param	Result	is the pointer of SHA3 block length array which is been
 * 			updated with the result.
 *
 * @return	None.
 *
 *************************************************************************************************/
static void XHmac_Xor(const u32 *Data, const u8 Value, u32 *Result, u32 BlockLen)
{
	u32 Index;
	u32 ValData = ((u32)Value << XASUFW_HMAC_24BIT_SHIFT) |
		      ((u32)Value << XASUFW_HMAC_16BIT_SHIFT) |
		      ((u32)Value << XASUFW_HMAC_8BIT_SHIFT) | (u32)Value;

	/** Perform XOR operation of each byte of Data with Value. */
	for (Index = 0x0U; Index < (BlockLen / XASUFW_WORD_LEN_IN_BYTES); Index++) {
		Result[Index] = Data[Index] ^ ValData;
	}
}
/** @} */
