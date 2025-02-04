/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xrsa_padding.c
*
* This file contains implementation of the padding interface functions for RSA.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   02/04/25 Initial release
*
* </pre>
*
**************************************************************************************************/
/**
* @addtogroup xrsa_padding_apis RSA Padding APIs
* @{
*/
/*************************************** Include Files *******************************************/
#include "xrsa_padding.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xil_util.h"
#include "xfih.h"
#include "xasufw_trnghandler.h"
#include "xasu_rsa_common.h"

/************************************ Constant Definitions ***************************************/
#define XRSA_MAX_DB_LEN		(479U)	/**< RSA max padding data block size in bytes */
#define XRSA_PSS_MAX_MSG_LEN	(550U)	/**< RSA PSS padding max message length */
#define XRSA_DATA_BLOCK_FIRST_INDEX				(0X00U)	/**< RSA output data block
									first index */
#define XRSA_DATA_BLOCK_SECOND_INDEX				(0X01U)	/**< RSA output data block
									second index */
#define XRSA_DATA_BLOCK_VALUE_AFTR_ZEROIZED_MEM			(0X01U)	/**< RSA output data block
								value after memory zeroization */
#define XRSA_OAEP_OUTPUT_DATA_BLOCK_FIRST_INDEX_VALUE		(0X00U)	/**< RSA OAEP
							output data block value in first index */
#define XRSA_OAEP_OUTPUT_DATA_BLOCK_MSG_SEPERATION_VALUE	(0X01U)	/**< RSA OAEP
						output data block seperation value from msg */
#define XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN				(0X08U)	/**< RSA PSS
								no of zeroes len in hash block */
#define XRSA_PSS_MSB_PADDING_MASK				(0X7FU)	/**< RSA PSS
									mask value */
#define XRSA_PSS_MSB_PADDING_CHECK_MASK				(0X80U)	/**< RSA PSS
									mask check value */
#define XRSA_PSS_OUTPUT_END_BYTE_VALUE				(0XBCU)	/**< RSA PSS
								output block end byte value */

/************************************** Type Definitions *****************************************/
/** Structure has input and output parameters used for MGF */
typedef struct {
	u8 *Seed;      /**< Input seed on which mask should be generated */
	u8 *Output;    /**< Output buffer to store the mask */
	u32 SeedLen;   /**< Seed length */
	u32 OutputLen; /**< Output buffer length */
} XAsufw_MgfInput;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XRsa_MaskGenFunc(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u8 ShaMode,
			    const XAsufw_MgfInput *MgfInput);
/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function performs RSA OAEP padding for the provided message to output a data
 * 		of length equal to keysize.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	ShaInstancePtr		Pointer to the Sha instance.
 * @param	PaddingParamsPtr	Pointer to the XAsu_RsaOaepPaddingParams structure which
 * 					holds OAEP padding related parameters.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- XASUFW_RSA_OAEP_INVALID_LEN, when input length is greater than OAEP defined length.
 * 		- XASUFW_CMD_IN_PROGRESS,when serving operation is not completed due to
 *		  SHA update operation using DMA.
 *		- XASUFW_RSA_OAEP_ENCRYPT_ERROR,when Public encryption error occurs after
 *		  OAEP padding.
 *		- XASUFW_RSA_ZEROIZE_MEMSET_FAIL,when memset with zero fails.
 * 		- XASUFW_RSA_DMA_COPY_FAIL,when DMA copy fails.
 *		- XASUFW_RSA_MEM_COPY_FAIL,when memcpy fails
 *
 *************************************************************************************************/
s32 XRsa_OaepEncode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		    const XAsu_RsaOaepPaddingParams *PaddingParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u32 KeySize = 0U;
	u32 Len = 0U;
	u32 HashLen = 0U;
	u32 Index = 0U;
	u32 DataBlockLen = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *MaskedDataBlock = DataBlock + XRSA_MAX_DB_LEN;
	u8 *SeedBuffer = MaskedDataBlock + XRSA_MAX_DB_LEN;
	u8 *MaskSeedBuffer = SeedBuffer + XASU_SHAKE_256_MAX_HASH_LEN;
	u8 PaddedOutputData[XRSA_MAX_KEY_SIZE_IN_BYTES] = {0U};
	XAsufw_MgfInput MgfInput;
	XAsu_ShaOperationCmd ShaParamsInput;

	/** Validatations of inputs. */
	if (PaddingParamsPtr == NULL) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_RsaValidateInputParams(&(PaddingParamsPtr->XAsu_RsaOpComp));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Validate the input arguments. */
	if ((PaddingParamsPtr->OptionalLabelAddr == 0U) ||
	    (PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr == 0U) || (DmaPtr == NULL) ||
	    (ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	KeySize = PaddingParamsPtr->XAsu_RsaOpComp.KeySize;
	Len = PaddingParamsPtr->XAsu_RsaOpComp.Len;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_RsaValidateKeySize(KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Get hash length based on SHA mode. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_GetHashLen(PaddingParamsPtr->ShaMode, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SHA_INVALID_SHA_MODE;
		goto END;
	}

	if (Len > (KeySize - (2U * HashLen) - 2U)) {
		Status = XASUFW_RSA_OAEP_INVALID_LEN;
		goto END;
	}

	DataBlockLen = (KeySize - HashLen - 1U);

	/** Calculate digest for optional label and append to data block. */
	ShaParamsInput.DataAddr = PaddingParamsPtr->OptionalLabelAddr;
	ShaParamsInput.HashAddr = (u64)(UINTPTR)DataBlock;
	ShaParamsInput.DataSize = PaddingParamsPtr->OptionalLabelSize;
	ShaParamsInput.HashBufSize = HashLen;
	ShaParamsInput.ShaMode = PaddingParamsPtr->ShaMode;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaParamsInput);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		goto RET;
	} else if (Status != XASUFW_SUCCESS) {
		goto END;
	} else {
		/* Do nothing */
	}

	/** Append 0x00's of length (KeySize - Len - (2U * HashLen) - 2U) to data block. */
	if ((KeySize - Len - (2U * HashLen) - 2U) != 0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemSet(&DataBlock[HashLen], XRSA_MAX_DB_LEN, 0U,
				     (KeySize - Len - (2U * HashLen) - 2U));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
			goto END;
		}
	}

	/** Append 0x01U to data block after the zeroized memory. */
	DataBlock[KeySize - Len - HashLen - 2U] = XRSA_DATA_BLOCK_VALUE_AFTR_ZEROIZED_MEM;

	/** Copy input data to server memory using DMA. */
	/** Datablock = Hashofoptionallabel || 0x00(length= (KeySize - Len - (2U * HashLen) - 2U)
		|| 0x01 || M . */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr,
			       (u64)(UINTPTR)&DataBlock[KeySize - Len - HashLen - 1U], Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_DMA_COPY_FAIL;
		goto END;
	}

	/** Get random numbers for seed input. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_TrngGetRandomNumbers(SeedBuffer, HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_RAND_GEN_ERROR;
		goto END;
	}

	/** Generate mask of required length for data block. */
	MgfInput.Output = MaskedDataBlock;
	MgfInput.OutputLen = DataBlockLen;
	MgfInput.Seed = SeedBuffer;
	MgfInput.SeedLen = HashLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_MaskGenFunc(DmaPtr, ShaInstancePtr, PaddingParamsPtr->ShaMode, &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR);
		goto END;
	}

	for (Index = 0U; Index < DataBlockLen; Index++) {
		DataBlock[Index] ^= MaskedDataBlock[Index];
	}

	/** Generate mask of required length for seed block. */
	MgfInput.Output = MaskSeedBuffer;
	MgfInput.OutputLen = HashLen;
	MgfInput.Seed = DataBlock;
	MgfInput.SeedLen = DataBlockLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_MaskGenFunc(DmaPtr, ShaInstancePtr, PaddingParamsPtr->ShaMode, &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_SEED_BUFFER_ERROR);
		goto END;
	}

	for (Index = 0U; Index < HashLen; Index++) {
		PaddedOutputData[Index + XRSA_DATA_BLOCK_SECOND_INDEX] =
		SeedBuffer[Index] ^ MaskSeedBuffer[Index];
	}

	/** Append 0x00U to data block after masking the data. */
	/** PaddedOutputData = 0x00 || maskedSeed || maskedDB. */
	PaddedOutputData[XRSA_DATA_BLOCK_FIRST_INDEX] =
		XRSA_OAEP_OUTPUT_DATA_BLOCK_FIRST_INDEX_VALUE;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(&PaddedOutputData[HashLen + 1U],
			      XRSA_MAX_KEY_SIZE_IN_BYTES, DataBlock,
			      XRSA_MAX_DB_LEN, DataBlockLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MEM_COPY_FAIL;
		goto END;
	}

	/** Perform public exponentiation encryption operation on padded data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_PubExp(DmaPtr, KeySize,
			     (u64)(UINTPTR)PaddedOutputData,
			     PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr,
			     PaddingParamsPtr->XAsu_RsaOpComp.KeyCompAddr,
			     PaddingParamsPtr->XAsu_RsaOpComp.ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_ENCRYPT_ERROR);
	}

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = XAsufw_DmaMemSet(DmaPtr, (u32)(UINTPTR)DataBlock, 0U, XRSA_MAX_KEY_SIZE_IN_BYTES *
				   XRSA_TOTAL_PARAMS);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, SStatus, PaddedOutputData,
				   XRSA_MAX_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA OAEP decode operation for the provided decrypted data
 * 		to extract the original message.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	ShaInstancePtr		Pointer to the Sha instance.
 * @param	PaddingParamsPtr	Pointer to the XAsu_RsaOaepPaddingParams structure which
 * 					holds OAEP padding related parameters.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- XASUFW_RSA_OAEP_INVALID_LEN, when input length is greater than OAEP defined length.
 * 		- XASUFW_CMD_IN_PROGRESS,when serving operation is not completed due to
 *		  SHA update operation using DMA.
 * 		- XASUFW_RSA_DMA_COPY_FAIL,when DMA copy fails.
 *		- XASUFW_RSA_MEM_COPY_FAIL,when memcpy fails
 *		- Also can return termination error codes from 0xBBU,0xBCU and 0xBDU,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_OaepDecode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		    const XAsu_RsaOaepPaddingParams *PaddingParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u32 KeySize = 0U;
	u32 Len = 0U;
	u32 HashLen = 0U;
	u32 Index = 0U;
	u32 DataBlockLen = 0U;
	u32 MsgLen = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *MaskedDataBlock = DataBlock + XRSA_MAX_DB_LEN;
	u8 *SeedBuffer = MaskedDataBlock + XRSA_MAX_DB_LEN;
	u8 *MaskedSeedBuffer = SeedBuffer + XASU_SHAKE_256_MAX_HASH_LEN;
	u8 HashBuffer[XASU_SHAKE_256_MAX_HASH_LEN];
	u8 DecryptOutputData[XRSA_MAX_KEY_SIZE_IN_BYTES] = {0U};
	XAsufw_MgfInput MgfInput;
	XAsu_ShaOperationCmd ShaParamsInput;

	/** Validatations of inputs. */
	if (PaddingParamsPtr == NULL) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_RsaValidateInputParams(&(PaddingParamsPtr->XAsu_RsaOpComp));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Validate the input arguments. */
	if ((PaddingParamsPtr->OptionalLabelAddr == 0U) ||
	    (PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr == 0U) || (DmaPtr == NULL) ||
	    (ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	KeySize = PaddingParamsPtr->XAsu_RsaOpComp.KeySize;
	Len = PaddingParamsPtr->XAsu_RsaOpComp.Len;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_RsaValidateKeySize(KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Get hash length based on SHA mode. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_GetHashLen(PaddingParamsPtr->ShaMode, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SHA_INVALID_SHA_MODE;
		goto END;
	}

	if ((Len != KeySize) || (Len < ((2U * HashLen) + 2U))) {
		Status = XASUFW_RSA_OAEP_INVALID_LEN;
		goto END;
	}

	DataBlockLen = (KeySize - HashLen - 1U);

	/** Calculate digest for optional label. */
	ShaParamsInput.DataAddr = PaddingParamsPtr->OptionalLabelAddr;
	ShaParamsInput.HashAddr = (u64)(UINTPTR)HashBuffer;
	ShaParamsInput.DataSize = PaddingParamsPtr->OptionalLabelSize;
	ShaParamsInput.HashBufSize = HashLen;
	ShaParamsInput.ShaMode = PaddingParamsPtr->ShaMode;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaParamsInput);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		goto RET;
	} else if (Status != XASUFW_SUCCESS) {
		goto END;
	} else {
		/* Do nothing */
	}

	/** Perform private exponentiation decryption operation. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_PvtExp(DmaPtr, PaddingParamsPtr->XAsu_RsaOpComp.Len,
			     PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr,
			     (u64)(UINTPTR)DecryptOutputData,
			     PaddingParamsPtr->XAsu_RsaOpComp.KeyCompAddr,
			     PaddingParamsPtr->XAsu_RsaOpComp.ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_DECRYPT_ERROR);
		goto END;
	}

	/** Copy seed buffer and data block from decrypted output. */
	/** EM = Y || maskedSeed || maskedDB . */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(MaskedSeedBuffer, XASU_SHAKE_256_MAX_HASH_LEN,
			     &DecryptOutputData[XRSA_DATA_BLOCK_SECOND_INDEX],
			     XRSA_MAX_KEY_SIZE_IN_BYTES, HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(MaskedDataBlock, XRSA_MAX_DB_LEN,
			     &DecryptOutputData[HashLen + 1U],
			     XRSA_MAX_KEY_SIZE_IN_BYTES, DataBlockLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MEM_COPY_FAIL;
		goto END;
	}

	/** Generate mask of required length for seed block. */
	MgfInput.Output = SeedBuffer;
	MgfInput.OutputLen = HashLen;
	MgfInput.Seed = MaskedDataBlock;
	MgfInput.SeedLen = DataBlockLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_MaskGenFunc(DmaPtr, ShaInstancePtr, PaddingParamsPtr->ShaMode, &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_SEED_BUFFER_ERROR);
		goto END;
	}

	for (Index = 0U; Index < HashLen; Index++) {
		SeedBuffer[Index] ^= MaskedSeedBuffer[Index];
	}

	/** Generate mask of required length for data block. */
	MgfInput.Output = DataBlock;
	MgfInput.OutputLen = DataBlockLen;
	MgfInput.Seed = SeedBuffer;
	MgfInput.SeedLen = HashLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_MaskGenFunc(DmaPtr, ShaInstancePtr, PaddingParamsPtr->ShaMode, &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR);
		goto END;
	}

	for (Index = 0U; Index < DataBlockLen; Index++) {
		DataBlock[Index] ^= MaskedDataBlock[Index];
	}

	/** Compare generated hash from optional label with data block from index zero until
		hash length. */
	/** DB = lHash(hash of optional label) || 0x00(length= (KeySize - Len - (2U * HashLen) - 2U)
		|| 0x01 || M . */

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(DataBlock, XRSA_MAX_DB_LEN, HashBuffer, XASU_SHAKE_256_MAX_HASH_LEN,
			     HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_OAEP_HASH_CMP_FAIL;
		goto END;
	}

	/** Check for 0x01U from which decrypted data is seperated from rest of the data block and
		copy data to output data address using DMA on successful comparison. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (DataBlock[HashLen] == XRSA_OAEP_OUTPUT_DATA_BLOCK_MSG_SEPERATION_VALUE) {
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&DataBlock[HashLen + 1U],
				       PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr,
				       DataBlockLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_DMA_COPY_FAIL;
		}
	} else {
		Index = HashLen;
		while ((DataBlock[Index] != XRSA_OAEP_OUTPUT_DATA_BLOCK_MSG_SEPERATION_VALUE)
		       || (Index == DataBlockLen)) {
			Index++;
		}
		if (Index == Len) {
			Status = XASUFW_RSA_OAEP_ONE_SEP_CMP_FAIL;
		} else {
			Index++;
			MsgLen = DataBlockLen - Index;
			if (MsgLen > (KeySize - (2U * HashLen) - 2U)) {
				Status = XASUFW_RSA_OAEP_INVALID_LEN;
			} else {
				Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)(&(DataBlock[Index])),
						       PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr,
						       MsgLen, 0U);
				if (Status != XASUFW_SUCCESS) {
					Status = XASUFW_RSA_DMA_COPY_FAIL;
				}
			}
		}
	}

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = XAsufw_DmaMemSet(DmaPtr, (u32)(UINTPTR)DataBlock, 0U, XRSA_MAX_KEY_SIZE_IN_BYTES *
				   XRSA_TOTAL_PARAMS);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, SStatus, DecryptOutputData,
				   XRSA_MAX_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA PSS padding for the provided message to output a data
 * 		of length equal to keysize.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	ShaInstancePtr		Pointer to the Sha instance.
 * @param	PaddingParamsPtr	Pointer to the XAsu_RsaPaddingParams structure which
 * 					holds PSS padding related parameters.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 * 		- XASUFW_CMD_IN_PROGRESS,when serving operation is not completed due to
 *		  SHA update operation using DMA.
 *		- XASUFW_RSA_ZEROIZE_MEMSET_FAIL,when memset with zero fails.
 * 		- XASUFW_RSA_DMA_COPY_FAIL,when DMA copy fails.
 *		- Also can return termination error codes from 0xBFU to 0xC3U and 0xD0U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_PssEncode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		   const XAsu_RsaPaddingParams *PaddingParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u32 SaltLen = 0U;
	u32 KeySize = 0U;
	u32 HashLen = 0U;
	u32 Index = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *HashBlock = DataBlock + XRSA_MAX_DB_LEN;
	u8 *HashBuffer = HashBlock + XRSA_PSS_MAX_MSG_LEN;
	u8 OutputData[XRSA_MAX_KEY_SIZE_IN_BYTES] = {0U};
	XAsufw_MgfInput MgfInput;
	XAsu_ShaOperationCmd ShaParamsInput;

	/** Validatations of inputs. */
	if (PaddingParamsPtr == NULL) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_RsaValidateInputParams(&(PaddingParamsPtr->XAsu_RsaOpComp));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Validate the input arguments. */
	if ((PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr == 0U) || (DmaPtr == NULL) ||
	    (ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	SaltLen = PaddingParamsPtr->SaltLen;
	KeySize = PaddingParamsPtr->XAsu_RsaOpComp.KeySize;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_RsaValidateKeySize(KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Get hash length based on SHA mode. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_GetHashLen(PaddingParamsPtr->ShaMode, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SHA_INVALID_SHA_MODE;
		goto END;
	}

	if ((PaddingParamsPtr->InputDataType == XASU_RSA_HASHED_INPUT_DATA) &&
		(PaddingParamsPtr->XAsu_RsaOpComp.Len != HashLen)) {
			Status = XASUFW_RSA_INVALID_PARAM;
			goto END;
	}

	if (SaltLen > HashLen) {
		Status = XASUFW_RSA_PSS_INVALID_SALT_LEN;
		goto END;
	}

	/** Calculate digest for input if it is non hashed input type and copy calculated hash
		to hash block after the zeroized area or only copy if input is already hashed
		input type. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (PaddingParamsPtr->InputDataType != XASU_RSA_HASHED_INPUT_DATA) {
		ShaParamsInput.DataAddr = PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr;
		ShaParamsInput.HashAddr = (u64)(UINTPTR)&HashBlock[XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN];
		ShaParamsInput.DataSize = PaddingParamsPtr->XAsu_RsaOpComp.Len;
		ShaParamsInput.HashBufSize = HashLen;
		ShaParamsInput.ShaMode = PaddingParamsPtr->ShaMode;

		Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaParamsInput);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			goto RET;
		} else if (Status != XASUFW_SUCCESS) {
			goto END;
		} else {
			/* Do nothing */
		}
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr,
				       (u64)(UINTPTR)&HashBlock[XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
				       PaddingParamsPtr->XAsu_RsaOpComp.Len, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_DMA_COPY_FAIL;
			goto END;
		}
	}

	/** Zeroize eight bytes of hash block. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(HashBlock, XRSA_PSS_MAX_MSG_LEN, 0U, XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Generate random numbers based on input salt length and append to
	hash block from the index after the calculated hash. */
	/** Hashblock = 00 00 00 00 00 00 00 00 || mHash(hash of input) || salt. */
	if (SaltLen != 0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_TrngGetRandomNumbers(&HashBlock[HashLen + XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
						     SaltLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_RAND_GEN_ERROR;
			goto END;
		}
	}

	/** Calculate digest for hash block after salt is added. */
	ShaParamsInput.DataAddr = (u64)(UINTPTR)HashBlock;
	ShaParamsInput.HashAddr = (u64)(UINTPTR)HashBuffer;
	ShaParamsInput.DataSize = HashLen + SaltLen + XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN;
	ShaParamsInput.HashBufSize = HashLen;
	ShaParamsInput.ShaMode = PaddingParamsPtr->ShaMode;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaParamsInput);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Append 0x00 of length KeySize - SaltLen - HashLen - 2 to data block and then add 0x01
		and salt. */
	/** DB = PS || 0x01 || salt. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((KeySize - SaltLen - HashLen - 2U) != 0U) {
		Status = Xil_SMemSet(DataBlock, XRSA_MAX_DB_LEN, 0U,
				     (KeySize - SaltLen - HashLen - 2U));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
			goto END;
		}
		DataBlock[KeySize - SaltLen - HashLen - 2U] = XRSA_DATA_BLOCK_VALUE_AFTR_ZEROIZED_MEM;
		if (SaltLen != 0U) {
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = Xil_SMemCpy(&DataBlock[KeySize - SaltLen - HashLen - 1U], XRSA_MAX_DB_LEN,
					     &HashBlock[HashLen + XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
					     XRSA_PSS_MAX_MSG_LEN, SaltLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_RSA_MEM_COPY_FAIL;
				goto END;
			}
		}
	} else {
		DataBlock[XRSA_DATA_BLOCK_FIRST_INDEX] = XRSA_DATA_BLOCK_VALUE_AFTR_ZEROIZED_MEM;
		if (SaltLen != 0U) {
			Status = Xil_SMemCpy(&DataBlock[XRSA_DATA_BLOCK_SECOND_INDEX], XRSA_MAX_DB_LEN,
					     &HashBlock[HashLen + XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
					     XRSA_PSS_MAX_MSG_LEN, SaltLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_RSA_MEM_COPY_FAIL;
				goto END;
			}
		} else {
			Status = XASUFW_RSA_PSS_NO_SALT_NO_RANDOM_STRING;
			goto END;
		}
	}

	/** Generate mask of required length for data block. */
	MgfInput.Output = OutputData;
	MgfInput.OutputLen = (KeySize - HashLen - 1U);
	MgfInput.Seed = HashBuffer;
	MgfInput.SeedLen = HashLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_MaskGenFunc(DmaPtr, ShaInstancePtr, PaddingParamsPtr->ShaMode, &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR);
		goto END;
	}

	for (Index = 0U; Index < (KeySize - HashLen - 1U); Index++) {
		OutputData[Index] ^= DataBlock[Index];
	}

	/** Make MSBit of output data as zero. */
	OutputData[XRSA_DATA_BLOCK_FIRST_INDEX] = OutputData[XRSA_DATA_BLOCK_FIRST_INDEX]
							& XRSA_PSS_MSB_PADDING_MASK;
	/** Append hash buffer, and end byte 0xBC for final padded output. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(&OutputData[KeySize - HashLen - 1U], XRSA_MAX_KEY_SIZE_IN_BYTES,
			     HashBuffer, XASU_SHAKE_256_MAX_HASH_LEN, HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MEM_COPY_FAIL;
		goto END;
	}

	/** EM = maskedDB || H(Hash buffer) || 0xBC. */
	OutputData[KeySize - 1U] = XRSA_PSS_OUTPUT_END_BYTE_VALUE;

	/** Perform private exponentiation decryption operation. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_PvtExp(DmaPtr, KeySize,
			     (u64)(UINTPTR)OutputData, PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr,
			     PaddingParamsPtr->XAsu_RsaOpComp.KeyCompAddr,
			     PaddingParamsPtr->XAsu_RsaOpComp.ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_ENCRYPT_ERROR);
	}

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = XAsufw_DmaMemSet(DmaPtr, (u32)(UINTPTR)DataBlock, 0U, XRSA_MAX_KEY_SIZE_IN_BYTES *
				   XRSA_TOTAL_PARAMS);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, SStatus, OutputData,
				   XRSA_MAX_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA PSS decode operation and verifies the signature of
 * 		the provided message.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	ShaInstancePtr		Pointer to the Sha instance.
 * @param	PaddingParamsPtr	Pointer to the XAsu_RsaPaddingParams structure which
 * 					holds PSS padding related parameters.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 * 		- XASUFW_CMD_IN_PROGRESS,when serving operation is not completed due to
 *		  SHA update operation using DMA.
 *		- XASUFW_RSA_ZEROIZE_MEMSET_FAIL,when memset with zero fails.
 * 		- XASUFW_RSA_DMA_COPY_FAIL,when DMA copy fails.
 *		- Also can return termination error codes from 0xC4U to 0xCAU and 0xD0U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_PssDecode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		   const XAsu_RsaPaddingParams *PaddingParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u32 SaltLen = 0U;
	u32 KeySize = 0U;
	u32 HashLen = 0U;
	u32 Index = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *MaskedDataBlock = DataBlock + XRSA_MAX_DB_LEN;
	u8 *MsgBlock = MaskedDataBlock + XRSA_MAX_DB_LEN;
	u8 *HashBuffer = MsgBlock + XRSA_PSS_MAX_MSG_LEN;
	u8 *EncodedMsgHashBuffer = HashBuffer + XASU_SHAKE_256_MAX_HASH_LEN;
	u8 HashedInputData[XASU_SHAKE_256_MAX_HASH_LEN] = {0U};
	u8 SignedInputData[XRSA_MAX_KEY_SIZE_IN_BYTES] = {0U};
	XAsufw_MgfInput MgfInput;
	XAsu_ShaOperationCmd ShaParamsInput;

	/** Validatations of inputs. */
	if (PaddingParamsPtr == NULL) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_RsaValidateInputParams(&(PaddingParamsPtr->XAsu_RsaOpComp));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Validate the input arguments. */
	if ((PaddingParamsPtr->SignatureDataAddr == 0U) || (DmaPtr == NULL) ||
	     (ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	SaltLen = PaddingParamsPtr->SaltLen;
	KeySize = PaddingParamsPtr->XAsu_RsaOpComp.KeySize;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_RsaValidateKeySize(KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if (PaddingParamsPtr->SignatureLen != KeySize) {
		Status = XASUFW_RSA_PSS_INVALID_LEN;
		goto END;
	}

	/** Get hash length based on SHA mode. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_GetHashLen(PaddingParamsPtr->ShaMode, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SHA_INVALID_SHA_MODE;
		goto END;
	}

	if ((PaddingParamsPtr->InputDataType == XASU_RSA_HASHED_INPUT_DATA) &&
		(PaddingParamsPtr->XAsu_RsaOpComp.Len != HashLen)) {
			Status = XASUFW_RSA_INVALID_PARAM;
			goto END;
	}

	if (SaltLen > HashLen) {
		Status = XASUFW_RSA_PSS_INVALID_SALT_LEN;
		goto END;
	}
	/** Calculate digest for input if it is non hashed input type and copy calculated hash
		to hash block after the zeroized area or only copy if input is already hashed
		input type. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (PaddingParamsPtr->InputDataType != XASU_RSA_HASHED_INPUT_DATA) {
		ShaParamsInput.DataAddr = PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr;
		ShaParamsInput.HashAddr = (u64)(UINTPTR)HashedInputData;
		ShaParamsInput.DataSize = PaddingParamsPtr->XAsu_RsaOpComp.Len;
		ShaParamsInput.HashBufSize = HashLen;
		ShaParamsInput.ShaMode = PaddingParamsPtr->ShaMode;

		Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaParamsInput);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			goto RET;
		} else if (Status != XASUFW_SUCCESS) {
			goto END;
		} else {
			/* Do nothing */
		}
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr,
				       (u64)(UINTPTR)HashedInputData,
				       PaddingParamsPtr->XAsu_RsaOpComp.Len, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_DMA_COPY_FAIL;
			goto END;
		}
	}

	/** Perform public exponentiation encryption operation. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_PubExp(DmaPtr, PaddingParamsPtr->SignatureLen,
			     PaddingParamsPtr->SignatureDataAddr, (u64)(UINTPTR)SignedInputData,
			     PaddingParamsPtr->XAsu_RsaOpComp.KeyCompAddr,
			     PaddingParamsPtr->XAsu_RsaOpComp.ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_DECRYPT_ERROR);
		goto END;
	}

	/** Check for 0xBCU in the end byte of the decrypted output and copy masked data block and
		hash value on successful comparison else error out. */
	/**EM = maskedDB || H || 0xbc. */
	if (SignedInputData[KeySize - 1U] == XRSA_PSS_OUTPUT_END_BYTE_VALUE) {
		Status = Xil_SMemCpy(MaskedDataBlock, XRSA_MAX_DB_LEN,
				     SignedInputData, XRSA_MAX_KEY_SIZE_IN_BYTES,
				     (KeySize - HashLen - 1U));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_MEM_COPY_FAIL;
			goto END;
		}
		Status = Xil_SMemCpy(HashBuffer, XASU_SHAKE_256_MAX_HASH_LEN,
				     &SignedInputData[KeySize - HashLen - 1U],
				     XRSA_MAX_KEY_SIZE_IN_BYTES, HashLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_MEM_COPY_FAIL;
			goto END;
		}
	} else {
		Status = XASUFW_RSA_PSS_RIGHT_MOST_CMP_FAIL;
		goto END;
	}
	/** Check whether MSbit is 0x00 and calculate the mask of data block on successful
		comparison else error out. */
	if ((MaskedDataBlock[XRSA_DATA_BLOCK_FIRST_INDEX] &
	     XRSA_PSS_MSB_PADDING_CHECK_MASK) == 0x00U) {
		MgfInput.Output = DataBlock;
		MgfInput.OutputLen = (KeySize - HashLen - 1U);
		MgfInput.Seed = HashBuffer;
		MgfInput.SeedLen = HashLen;

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XRsa_MaskGenFunc(DmaPtr, ShaInstancePtr, PaddingParamsPtr->ShaMode,
					  &MgfInput);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR);
			goto END;
		}

		for (Index = 0U; Index < (KeySize - HashLen - 1U); Index++) {
			DataBlock[Index] ^= MaskedDataBlock[Index];
		}
		DataBlock[XRSA_DATA_BLOCK_FIRST_INDEX] = DataBlock[XRSA_DATA_BLOCK_FIRST_INDEX]
			& XRSA_PSS_MSB_PADDING_MASK;
	} else {
		Status = XASUFW_RSA_PSS_LEFT_MOST_BIT_CMP_FAIL;
		goto END;
	}
	/** Check for 0x00's of length KeySize - SaltLen - HashLen - 2 in data block and then check
		0x01U and seperate salt if available. */
	/** DB = PS || 0x01 || salt. */
	if ((KeySize - HashLen - SaltLen - 2U) != 0U) {
		for (Index = 0U; Index < (KeySize - HashLen - SaltLen - 2U); Index++) {
			if (DataBlock[Index] != 0U) {
				Status = XASUFW_RSA_PSS_DB_LEFT_MOST_BYTE_CMP_FAIL;
				goto END;
			}
		}
	} else {
		Index = 0U;
	}

	if (DataBlock[Index] != XRSA_DATA_BLOCK_VALUE_AFTR_ZEROIZED_MEM) {
		Status = XASUFW_RSA_PSS_DB_BYTE_ONE_CMP_FAIL;
		goto END;
	}

	if (SaltLen != 0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy(&MsgBlock[HashLen + XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
				     XRSA_PSS_MAX_MSG_LEN, &DataBlock[Index + 1U], XRSA_MAX_DB_LEN,
				     SaltLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_MEM_COPY_FAIL;
			goto END;
		}
	}

	/** After arranging message block with (0x)00 00 00 00 00 00 00 00 || mHash || salt
		calculate the hash and compare with the hash input. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(MsgBlock, XRSA_PSS_MAX_MSG_LEN, 0U, XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(&MsgBlock[XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN], XRSA_PSS_MAX_MSG_LEN,
			     HashedInputData, XASU_SHAKE_256_MAX_HASH_LEN, HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MEM_COPY_FAIL;
		goto END;
	}

	/** Calculate digest for hash block after salt is added. */
	ShaParamsInput.DataAddr = (u64)(UINTPTR)MsgBlock;
	ShaParamsInput.HashAddr = (u64)(UINTPTR)EncodedMsgHashBuffer;
	ShaParamsInput.DataSize = HashLen + SaltLen + XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN;
	ShaParamsInput.HashBufSize = HashLen;
	ShaParamsInput.ShaMode = PaddingParamsPtr->ShaMode;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaParamsInput);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(HashBuffer, XASU_SHAKE_256_MAX_HASH_LEN, EncodedMsgHashBuffer,
			     XASU_SHAKE_256_MAX_HASH_LEN, HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PSS_HASH_CMP_FAIL;
	}

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = XAsufw_DmaMemSet(DmaPtr, (u32)(UINTPTR)DataBlock, 0U, XRSA_MAX_KEY_SIZE_IN_BYTES *
				   XRSA_TOTAL_PARAMS);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, SStatus, HashedInputData,
				   XASU_SHAKE_256_MAX_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, SStatus, SignedInputData,
				   XRSA_MAX_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function takes an input of variable length and
 *		a desired output length as input, and provides fixed output
 *		mask using cryptographic hash function.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	ShaInstancePtr	Pointer to the Sha instance.
 * @param	ShaMode		SHA mode selection.
 * @param       MgfInput	Pointer to all required parameters of MGF.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- XASUFW_RSA_ZEROIZE_MEMSET_FAIL,when memset with zero fails.
 *
 *************************************************************************************************/
static s32 XRsa_MaskGenFunc(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u8 ShaMode,
		     const XAsufw_MgfInput *MgfInput)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u32 Counter = 0U;
	u32 HashLen = 0U;
	u8 Hash[XASU_SHAKE_256_MAX_HASH_LEN];
	u8 Bytes[XASUFW_WORD_LEN_IN_BYTES];
	u32 OutputSize = 0U;
	u32 NoOfIterations = 0U;
	u8 *OutputPtr;

	/** Validate the input arguments. */
	if ((MgfInput == NULL) || (DmaPtr == NULL) || (ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if ((MgfInput->Seed == NULL) || (MgfInput->Output == NULL) || (MgfInput->OutputLen == 0U)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Get hash length based on SHA mode. */
	Status = XSha_GetHashLen(ShaMode, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SHA_INVALID_SHA_MODE;
		goto END;
	}

	OutputSize = MgfInput->OutputLen;
	if (OutputSize > HashLen) {
		OutputSize = HashLen;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(Bytes, XASUFW_WORD_LEN_IN_BYTES, 0U, XASUFW_WORD_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	OutputPtr = MgfInput->Output;
	NoOfIterations = (MgfInput->OutputLen + HashLen - 1U) / HashLen;
	while (Counter < NoOfIterations) {
		XAsufw_I2Osp(Counter, XASUFW_WORD_LEN_IN_BYTES, Bytes);

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Start(ShaInstancePtr, ShaMode);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(ShaInstancePtr, DmaPtr, (u64)(UINTPTR)MgfInput->Seed,
				     MgfInput->SeedLen, (u32)FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(ShaInstancePtr, DmaPtr, (u64)(UINTPTR)Bytes,
				     XASUFW_WORD_LEN_IN_BYTES, (u32)TRUE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Finish(ShaInstancePtr, (u32 *)Hash, HashLen,
				     XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy(OutputPtr, OutputSize, Hash, OutputSize, OutputSize);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		OutputPtr = &OutputPtr[OutputSize];
		Counter = Counter + 1U;
		if (Counter == (NoOfIterations - 1U)) {
			OutputSize = MgfInput->OutputLen - ((NoOfIterations - 1U) * HashLen);
		}
	}

END:
	/** Zeroize local copy of all the parameters. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, SStatus, Hash,
				   XASU_SHAKE_256_MAX_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, SStatus, Bytes,
				   XASUFW_WORD_LEN_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	return Status;
}
/** @} */
