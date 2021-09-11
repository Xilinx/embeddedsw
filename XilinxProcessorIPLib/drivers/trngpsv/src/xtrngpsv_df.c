/**************************************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrngpsv_df.c
 * @addtogroup trngpsv_v1_0
 * @{
 *
 * Implements the DF functionality, calls the functions that implement core algorithm for the
 * DF implementation
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  ssc  09/05/21 First release
 *
 * </pre>
 *
 **************************************************************************************************/

/*************************************** Include Files *******************************************/

#include "CompactAES.h"
#include "xtrngpsv.h"

/************************************ Constant Definitions ***************************************/

#define DF_PAD_VAL	0x80U
#define DF_KEY_LEN	32U

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/*************************************************************************************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief
 * This function implements the Derivative Function (per NIST SP80090A) by distilling the entropy
 * available in a lot of bits at its input (DFInput) into a smaller number of bits on the output
 * (DFOutput), thus bringing entropy per bit to 1. Actual length of input to DF need to be provided
 * to the core (block cipher)algorithm for DF, based on if personalization string is present or not
 * and based on actual length of input entropy, the buffer needs to adjusted by pushing all the
 * unused bytes to the end. The Block Cipher algorithm invoked here is as per sections 10.3.2 and
 * 10.3.3 of the NIST.SP.800-90Ar1 document.
 *
 * @param	InstancePtr is a pointer to the XTrngpsv instance to be worked on.
 * @param	DFOutput points to the buffer to which output of DF operation is stored.
 * @param	DF_Flag is the flag used to indicate if this function is used to generate seed
 * 		or random number (used in the PTRNG mode)
 * @param	PersStrPtr is the pointer to Personalization string. If no personalization string
 * 		is present, it should be NULL.
 *
 * @return
 *		- XTRNGPSV_SUCCESS if successful.
 *		- XTRNGPSV_ERROR_DF_CPY if MemCpy failure
 *
 **************************************************************************************************/
s32 XTrngpsv_DF(XTrngpsv *InstancePtr, u8 *DFOutput, u32 DF_Flag, const u8 *PersStrPtr)
{
	s32 Status = XTRNGPSV_FAILURE;
	UINTPTR SrcAddr;
	UINTPTR DestAddr;
	UINTPTR RemDataAddr;
	u32 TransferSize;
	u32 DiffSize;
	u32 ActualDFInputLen;
	u32 Index;
	u8 *AesInBlkPtr;
	u8 *AesOutBlkPtr;

	const u8 DFKey[DF_KEY_LEN] = {
			0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U,
			8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U,
			16U, 17U, 18U, 19U, 20U, 21U, 22U, 23U,
			24U, 25U, 26U, 27U, 28U, 29U, 30U, 31U};

	/*
	 * EntropyData member of DFInputis already populated by calling functions, remaining members
	 * of this data structure to be filled here. Block cipher algorithm expects data to be
	 * present in a specific format which is the XTrngpsv_DFInput format
	 */

	if (PersStrPtr == NULL) {
		InstancePtr->DFInput.InputLen =
		XTRNGPSV_SWAP_ENDIAN(InstancePtr->EntropySize);

		ActualDFInputLen = ((u32)sizeof(XTrngpsv_DFInput) + InstancePtr->EntropySize
				- (u32)sizeof(InstancePtr->DFInput.EntropyData)
				- (u32)sizeof(InstancePtr->DFInput.PersString)) / AES_BLK_SIZE;
	}
	else {
		InstancePtr->DFInput.InputLen =
		XTRNGPSV_SWAP_ENDIAN(
			InstancePtr->EntropySize + (u32)sizeof(InstancePtr->DFInput.PersString));

		ActualDFInputLen = ((u32)sizeof(XTrngpsv_DFInput) + InstancePtr->EntropySize
				- (u32)sizeof(InstancePtr->DFInput.EntropyData)) / AES_BLK_SIZE;

		Status = Xil_SecureMemCpy(InstancePtr->DFInput.PersString, XTRNGPSV_PERS_STR_LEN_BYTES,
				PersStrPtr, XTRNGPSV_PERS_STR_LEN_BYTES);
		if (Status != XTRNGPSV_SUCCESS) {
			Status = (s32)XTRNGPSV_ERROR_DF_CPY;
			goto SET_ERR;
		}
	}

	if (DF_Flag == DF_SEED) {
		InstancePtr->DFInput.PostDfLen = XTRNGPSV_SWAP_ENDIAN(XTRNGPSV_PERS_STR_LEN_BYTES);
	}
	else { /* DF_RAND */
		InstancePtr->DFInput.PostDfLen = XTRNGPSV_SWAP_ENDIAN(XTRNGPSV_GEN_LEN_BYTES);
	}

	InstancePtr->DFInput.PadData[0] = DF_PAD_VAL;

	if (PersStrPtr == NULL) {
		DestAddr = (UINTPTR)InstancePtr->DFInput.PadData + InstancePtr->EntropySize
				- (u32)sizeof(InstancePtr->DFInput.EntropyData)
				- (u32)sizeof(InstancePtr->DFInput.PersString);
		SrcAddr = (UINTPTR)InstancePtr->DFInput.PadData;
		TransferSize = (u32)sizeof(InstancePtr->DFInput.PadData);
	}
	else {
		DestAddr = (UINTPTR)InstancePtr->DFInput.PersString + InstancePtr->EntropySize
				- (u32)sizeof(InstancePtr->DFInput.EntropyData);
		SrcAddr = (UINTPTR)InstancePtr->DFInput.PersString;
		TransferSize = (u32)sizeof(InstancePtr->DFInput.PersString)
				+ (u32)sizeof(InstancePtr->DFInput.PadData);
	}

	/* Below is optional step just to make sure that remaining bytes (beyond what is given
	 * as input to DF) is cleared with 0s
	 */
	DiffSize = (u32)(SrcAddr - DestAddr);
	RemDataAddr = (UINTPTR)&InstancePtr->DFInput + (u32)sizeof(InstancePtr->DFInput) - DiffSize;

	if (DiffSize > 0U) {
		/* Move the block up */
		Status = Xil_SecureMemCpy((u8*)DestAddr, TransferSize, (u8*)SrcAddr, TransferSize);
		if (Status != XTRNGPSV_SUCCESS) {
			Status = (s32)XTRNGPSV_ERROR_DF_CPY;
			goto SET_ERR;
		}

		/* Fill the remaining memory (after moving) with 0s */
		(void)memset((u8*)RemDataAddr, 0, DiffSize);
	}

	/* Perform first part of the DF algorithm */

	(void)aesSetupKey(DFKey, (s32)sizeof(DFKey));
	for (Index = 0U; Index < XTRNGPSV_SEED_LEN_BYTES; Index += AES_BLK_SIZE) {

		(void)memset((u8*)InstancePtr->DFOutput + Index, 0, AES_BLK_SIZE);
		InstancePtr->DFInput.IvCounter[0] =
		XTRNGPSV_SWAP_ENDIAN(Index / AES_BLK_SIZE);
		aesCbcEncrypt((u8*)&InstancePtr->DFInput, NULL, InstancePtr->DFOutput + Index,
				(s32)ActualDFInputLen);
	}

	/* Perform second part of the DF algorithm (final update to DFOutput) */

	(void)aesSetupKey((u8*)InstancePtr->DFOutput, (s32)sizeof(DFKey));

	for (Index = 0U; Index < XTRNGPSV_SEED_LEN_BYTES; Index += AES_BLK_SIZE) {
		if (Index == 0U) {
			AesInBlkPtr = &DFOutput[XTRNGPSV_SEC_STRENGTH_BYTES];
		}
		else {
			AesInBlkPtr = &DFOutput[Index - AES_BLK_SIZE];
		}

		AesOutBlkPtr = &DFOutput[Index];
		aesEncrypt(AesInBlkPtr, AesOutBlkPtr);
	}

	Status = XTRNGPSV_SUCCESS;

SET_ERR:
	return Status;
}
/** @} */
