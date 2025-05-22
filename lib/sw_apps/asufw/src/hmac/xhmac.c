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
*       yog  02/20/25 Updated the HMAC key length validation.
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
#include "xasu_def.h"

#ifdef XASU_HMAC_ENABLE
/************************************** Type Definitions *****************************************/
/** This typedef is used to update the state of HMAC. */
typedef enum {
	XHMAC_INITIALIZED = 0x1, /**< HMAC is in initialized state */
	XHMAC_STARTED, /**< HMAC is in start state */
	XHMAC_UPDATE_IN_PROGRESS, /**< HMAC update is in progress state during data chunk updates */
	XHMAC_UPDATE_COMPLETED, /**< HMAC update is in completed state after the final data chunk */
} XHmac_State;

/** @} */

/**
* HMAC driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
struct _XHmac {
	XSha *ShaInstancePtr; /**< SHA instance pointer */
	u8 BlockLen; /**< SHA block length */
	u8 ShaMode; /**< SHA mode */
	u8 HashBufLen; /**< SHA hash buffer length */
	u8 Reserved; /**< Reserved for future */
	u8 IntHash[XASUFW_HMAC_SHA_HASH_MAX_LEN]; /**< Buffer to store intermediate hash */
	u8 OPadRes[XASUFW_SHAKE_SHA3_256_BLOCK_LEN]; /**< Buffer to store OPAD result */
	XHmac_State HmacState; /**< HMAC current state */
};

/**
* @addtogroup xhmac_server_apis HMAC Server APIs
* @{
*/
/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
static s32 XHmac_ProcessKeyWithPadding(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, u64 KeyAddr,
				       u32 KeyLen, u8 *ProcessedKey);
static inline void XHmac_Xor(const u32 *Data, const u32 Value, u32 *Result, u32 BlockLen);

/************************************** Macros Definitions ***************************************/
#define XASUFW_HMAC_IPAD_VALUE		(0x36363636U) /**< Each byte of Key provided is XOR'ed with
							this IPAD value 0x36U */
#define XASUFW_HMAC_OPAD_VALUE		(0x5C5C5C5CU) /**< Each byte of Key provided is XOR'ed with
							this OPAD value 0x5CU. */

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function returns HMAC instance pointer.
 *
 * @return
 * 		- Pointer to the XHmac_Instance.
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

	/** Update HMAC state to XHMAC_INITIALIZED. */
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
 * @param	HashLen		Length of the HASH for the provided SHA type and mode.
 *
 * @return
 * 	- XASUFW_SUCCESS, if initialization is successful.
 * 	- XASUFW_HMAC_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_HMAC_INVALID_KEY_LENGTH, if key length input is invalid.
 * 	- XASUFW_HMAC_STATE_MISMATCH_ERROR, if HMAC state is mismatched.
 * 	- XASUFW_HMAC_INVALID_HASHLEN, if input hash length is invalid.
 *
 *************************************************************************************************/
s32 XHmac_Init(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, XSha *ShaInstancePtr, u64 KeyAddr,
	       u32 KeyLen, u8 ShaMode, u32 HashLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 K0[XASUFW_SHAKE_SHA3_256_BLOCK_LEN] = {0U};
	u32 ExpHashLen = 0U;

	/** Validate input parameters. */
	if ((InstancePtr == NULL) || (AsuDmaPtr == NULL) || (ShaInstancePtr == NULL) ||
	    (KeyAddr == 0U)) {
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

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Validate the hash length. */
	if (HashLen != ExpHashLen) {
		Status = XASUFW_HMAC_INVALID_HASHLEN;
		goto END;
	}

	if ((KeyLen == 0U) || (KeyLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) {
		Status = XASUFW_HMAC_INVALID_KEY_LENGTH;
		goto END;
	}

	InstancePtr->ShaInstancePtr = ShaInstancePtr;
	InstancePtr->HashBufLen = (u8)HashLen;
	InstancePtr->ShaMode = ShaMode;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Preprocess key. */
	Status = XHmac_ProcessKeyWithPadding(InstancePtr, AsuDmaPtr, KeyAddr, KeyLen, (u8 *)K0);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Initialize SHA engine to calculate the hash on IPad || Data. */
	Status = XSha_Start(InstancePtr->ShaInstancePtr, (u32)InstancePtr->ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Update SHA with IPad data. */
	Status = XSha_Update(InstancePtr->ShaInstancePtr, AsuDmaPtr,
			     (u64)(UINTPTR)K0, (u32)InstancePtr->BlockLen, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	/** Update HMAC state to XHMAC_STARTED. */
	InstancePtr->HmacState = XHMAC_STARTED;

END_CLR:
	/** Zeroize the hash buffer after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, K0,
		  XASUFW_SHAKE_SHA3_256_BLOCK_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	if ((Status != XASUFW_SUCCESS) && (InstancePtr != NULL)) {
		/** Set HMAC state to XHMAC_INITIALIZED and set SHA under reset upon any failure. */
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
 * 				DataLen can be 0 <= DataLen < ((2^B)-8B). Where B is the block
 * 				length of the provided SHA type and SHA mode.
 * @param	IsLastUpdate	Variable which indicates the last update.
 *
 * @return
 * 	- XASUFW_SUCCESS, if HMAC update is successful.
 * 	- XASUFW_HMAC_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_HMAC_STATE_MISMATCH_ERROR, if HMAC state is mismatched.
 *
 *************************************************************************************************/
s32 XHmac_Update(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, u64 DataAddr, u32 DataLen,
		 u32 IsLastUpdate)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	static u32 HmacUpdateStage = 0x0U;

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

	if (HmacUpdateStage != 0U) {
		goto SHA_IN_HMAC_STAGE_UPDATE_DONE;
	}

	/** Update SHA with input data for which HMAC needs to be calculated. */
	Status = XSha_Update(InstancePtr->ShaInstancePtr, AsuDmaPtr, DataAddr, DataLen,
			     IsLastUpdate);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		HmacUpdateStage = HMAC_UPDATE_IN_PROGRESS;
		goto DONE;
	} else if (Status != XASUFW_SUCCESS) {
		goto END;
	} else {
		/* Do nothing */
	}

SHA_IN_HMAC_STAGE_UPDATE_DONE:
	HmacUpdateStage = 0U;

	if (IsLastUpdate == XASU_TRUE) {
		/**
		 * If this is the last update, get the HASH calculated for input (iPad || Data)
		 * and update HMAC state to XHMAC_UPDATE_COMPLETED.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Finish(InstancePtr->ShaInstancePtr, (u32 *)(InstancePtr->IntHash),
				     (u32)InstancePtr->HashBufLen, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		InstancePtr->HmacState = XHMAC_UPDATE_COMPLETED;
	} else {
		/** If this is not the last update, update HMAC state to XHMAC_UPDATE_IN_PROGRESS. */
		InstancePtr->HmacState = XHMAC_UPDATE_IN_PROGRESS;
	}
END:
	if ((Status != XASUFW_SUCCESS) && (InstancePtr != NULL)) {
		/** Set HMAC state to XHMAC_INITIALIZED and set SHA under reset upon any failure. */
		InstancePtr->HmacState = XHMAC_INITIALIZED;
		XSha_Reset(InstancePtr->ShaInstancePtr);
		/** Zeroize intermediate hash buffer upon any failure. */
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus,
			  (InstancePtr->IntHash), XASUFW_HMAC_SHA_HASH_MAX_LEN);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
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
 * @param	HmacOutPtr	Pointer to store the HMAC output.
 *
 * @return
 * 	- XASUFW_SUCCESS, if calculation of HMAC is successful.
 * 	- XASUFW_HMAC_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_HMAC_STATE_MISMATCH_ERROR, if HMAC state is mismatched.
 * 	- XASUFW_HMAC_ERROR, if any operation fails.
 *
 *************************************************************************************************/
s32 XHmac_Final(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, u32 *HmacOutPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);

	/** Validate input parameters. */
	if ((InstancePtr == NULL) || (AsuDmaPtr == NULL) || (HmacOutPtr == NULL)) {
		Status = XASUFW_HMAC_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->HmacState != XHMAC_UPDATE_COMPLETED) {
		Status = XASUFW_HMAC_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Initialize SHA engine to calculate the hash on Opad || IntHash. */
	Status = XSha_Start(InstancePtr->ShaInstancePtr, (u32)InstancePtr->ShaMode);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_ERROR);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Update SHA with OPad data. */
	Status = XSha_Update(InstancePtr->ShaInstancePtr, AsuDmaPtr,
			     (u64)(UINTPTR)(InstancePtr->OPadRes), (u32)InstancePtr->BlockLen,
			     XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_ERROR);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Update SHA with IntHash = (HASH(IPad || input)) data. */
	Status = XSha_Update(InstancePtr->ShaInstancePtr, AsuDmaPtr,
			     (u64)(UINTPTR)(InstancePtr->IntHash), (u32)InstancePtr->HashBufLen,
			     XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_ERROR);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Get the final HMAC output = HASH(OPad || IntHash). */
	Status = XSha_Finish(InstancePtr->ShaInstancePtr, (u32 *)HmacOutPtr,
			     (u32)InstancePtr->HashBufLen, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_ERROR);
	}

END:
	if (InstancePtr != NULL) {
		/** Zeroize intermediate hash buffer. */
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus,
			  (InstancePtr->IntHash), XASUFW_HMAC_SHA_HASH_MAX_LEN);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

		/** Zeroize local OpadRes buffer. */
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus,
			  (InstancePtr->OPadRes), XASUFW_SHAKE_SHA3_256_BLOCK_LEN);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

		/** Update the HMAC state to XHMAC_INITIALIZED and set SHA under reset. */
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
 * @param	KeyAddr		is the address which holds the key for HMAC.
 * @param	KeyLen		variable holds the length of the key.
 * @param	ProcessedKey	Pointer to hold the processed key.
 *
 * @return
 * 	- XASUFW_SUCCESS, if preprocessing key is successful.
 * 	- XASUFW_SHA_INVALID_SHA_MODE, if get SHA block length fails.
 * 	- XASUFW_ZEROIZE_MEMSET_FAIL, if memset fails.
 * 	- XASUFW_HMAC_ERROR, if any operation fails.
 * 	- XASUFW_MEM_COPY_FAIL, if mem copy fails.
 *
 *************************************************************************************************/
static s32 XHmac_ProcessKeyWithPadding(XHmac *InstancePtr, XAsufw_Dma *AsuDmaPtr, u64 KeyAddr,
				       u32 KeyLen, u8 *ProcessedKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Get block length based on the SHA mode and type. */
	Status = XSha_GetShaBlockLen(InstancePtr->ShaInstancePtr, InstancePtr->ShaMode,
				     &InstancePtr->BlockLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SHA_INVALID_SHA_MODE;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Zeroize the array created to store the processed key. */
	Status = Xil_SMemSet(ProcessedKey, XASUFW_SHAKE_SHA3_256_BLOCK_LEN, 0x0U,
			     XASUFW_SHAKE_SHA3_256_BLOCK_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (KeyLen > InstancePtr->BlockLen) {
		/**
		 * If provided key length is greater than the calculated block length,
		 * it is hashed to reduce it to the hash output size and append it with zeros to
		 * match the block length.
		 */
		Status = XSha_Start(InstancePtr->ShaInstancePtr, (u32)InstancePtr->ShaMode);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_ERROR);
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(InstancePtr->ShaInstancePtr, AsuDmaPtr, KeyAddr,
				     KeyLen, XASU_TRUE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_ERROR);
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Finish(InstancePtr->ShaInstancePtr, (u32 *)ProcessedKey,
				     (u32)InstancePtr->HashBufLen, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_ERROR);
			XFIH_GOTO(END);
		}
	} else {
		/**
		 * If the key provided is less than the calculated block length,
		 *  - The key must be extended by appending zeros to ensure its length matches
		 *    the block length.
		 * If Key provided is equal to block length,
		 *  - The key provided is the processed key.
		 */
		Status = Xil_SMemCpy(ProcessedKey, KeyLen, (u8 *)(UINTPTR)KeyAddr, KeyLen, KeyLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			XFIH_GOTO(END);
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
 * @param	BlockLen Block length of the SHA mode.
 *
 *************************************************************************************************/
static inline void XHmac_Xor(const u32 *Data, const u32 Value, u32 *Result, u32 BlockLen)
{
	u32 Index;

	/** Perform XOR operation of each word of Data with OPAD/IPAD value. */
	for (Index = 0x0U; Index < (BlockLen / XASUFW_WORD_LEN_IN_BYTES); Index++) {
		Result[Index] = Data[Index] ^ Value;
	}
}
#endif /* XASU_HMAC_ENABLE */
/** @} */
