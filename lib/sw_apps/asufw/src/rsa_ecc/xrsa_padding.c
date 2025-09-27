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
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   02/04/25 Initial release
 * 1.1   am   05/18/25 Fixed implicit conversion of operands
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xrsa_padding_apis RSA Padding Server APIs
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

#ifdef XASU_RSA_PADDING_ENABLE
/************************************ Constant Definitions ***************************************/
#define XRSA_MAX_DB_LEN		(479U)	/**< RSA max padding data block size in bytes */
#define XRSA_PSS_MAX_MSG_LEN	(550U)	/**< RSA PSS padding max message length */
#define XRSA_DATA_BLOCK_FIRST_INDEX				(0x00U)	/**< RSA output data block
									first index */
#define XRSA_DATA_BLOCK_SECOND_INDEX				(0x01U)	/**< RSA output data block
									second index */
#define XRSA_DATA_BLOCK_DELIMITER				(0x01U)	/**< RSA output data block
								value after memory zeroization */
#define XRSA_DATA_BLOCK_LENGTH_OFFSET				(0x01U)	/**< RSA Data block length
								value calculation offset */
#define XRSA_DATA_BLOCK_RANDOM_STRING_OFFSET			(0x02U)	/**< RSA Padding
								random string index offset value */
#define XRSA_OAEP_OUTPUT_DATA_BLOCK_FIRST_INDEX_VALUE		(0x00U)	/**< RSA OAEP
							output data block value in first index */
#define XRSA_OAEP_OUTPUT_DATA_BLOCK_MSG_SEPERATION_VALUE	(0x01U)	/**< RSA OAEP
						output data block seperation value from msg */
#define XRSA_OAEP_ZERO_PADDING_DATA_BLOCK_OFFSET		(0x01U)	/**< RSA OAEP
								zero padding index offset value */
#define XRSA_OAEP_ZERO_PADDING_DATA_VALUE			(0x00U)	/**< RSA OAEP
								zero padding string values */
#define XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN				(0x08U)	/**< RSA PSS
								no of zeroes length in hash block */
#define XRSA_PSS_MSB_PADDING_MASK				(0x7FU)	/**< RSA PSS
									mask value */
#define XRSA_PSS_MSB_PADDING_CHECK_MASK				(0x80U)	/**< RSA PSS
									mask check value */
#define XRSA_PSS_OUTPUT_END_BYTE_VALUE				(0xBCU)	/**< RSA PSS
								output block end byte value */
#define XRSA_PSS_DATA_BLOCK_END_BYTE_OFFSET			(0x01U)	/**< RSA Data block
								end byte calculation offset */
#define XRSA_PSS_ONE_PADDING_DATA_BLOCK_OFFSET			(0x01U)	/**< RSA PSS
								one padding index offset value*/
#define XRSA_INPUT_PADDING_ZERO_AND_ONE_OFFSET			(0x02U)	/**< RSA input message
								length value calculation offset */
/** @} */
/************************************** Type Definitions *****************************************/
/** Structure has input and output parameters used for MGF */
typedef struct {
	u8 *Seed;      /**< Input seed on which mask should be generated */
	u8 *Output;    /**< Output buffer to store the mask */
	u32 SeedLen;   /**< Seed length */
	u32 OutputLen; /**< Output buffer length */
} XAsufw_MgfInput;

/**
* @addtogroup xrsa_padding_apis RSA Padding Server APIs
* @{
*/
/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XRsa_MaskGenFunc(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u8 ShaMode,
			    const XAsufw_MgfInput *MgfInput);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function performs RSA OAEP padding for the provided message to output a data
 * 		of length equal to key size.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	ShaInstancePtr		Pointer to the SHA instance.
 * @param	PaddingParamsPtr	Pointer to the XAsu_RsaOaepPaddingParams structure which
 * 					holds OAEP padding related parameters.
 *
 * @return
 *		- XASUFW_SUCCESS, if RSA OAEP encoding is successful.
 *		- XASUFW_FAILURE, if RSA OAEP encoding fails.
 *		- XASUFW_RSA_INVALID_PARAM, if input parameters validation fails.
 *		- XASUFW_RSA_OAEP_INVALID_LEN, if input length is greater than OAEP defined length.
 *		- XASUFW_CMD_IN_PROGRESS, if command is in progress when SHA is operating in DMA
 *		  non-blocking mode.
 *		- XASUFW_RSA_OAEP_ENCRYPT_ERROR, if Public encryption error occurs after
 *		  OAEP padding.
 *		- XASUFW_ZEROIZE_MEMSET_FAIL, if memset with zero fails.
 *		- XASUFW_DMA_COPY_FAIL, if DMA copy fails.
 *		- XASUFW_MEM_COPY_FAIL, if memcpy fails
 *
 *************************************************************************************************/
s32 XRsa_OaepEncode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		    const XAsu_RsaOaepPaddingParams *PaddingParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 KeySize = 0U;
	u32 Len = 0U;
	u32 HashLen = 0U;
	u32 Index = 0U;
	u32 DataBlockLen = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *MaskedDataBlock = DataBlock + XRSA_MAX_DB_LEN;
	u8 *SeedBuffer = MaskedDataBlock + XRSA_MAX_DB_LEN;
	u8 *MaskSeedBuffer = SeedBuffer + XASU_SHA_512_HASH_LEN;
	u8 PaddedOutputData[XRSA_MAX_KEY_SIZE_IN_BYTES];
	XAsufw_MgfInput MgfInput;
	XAsu_ShaOperationCmd ShaParamsInput;

	/** Validate the input parameters. */
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
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((PaddingParamsPtr->OptionalLabelAddr == 0U) ||
	    (PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr == 0U) || (DmaPtr == NULL) ||
	    (ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	KeySize = PaddingParamsPtr->XAsu_RsaOpComp.KeySize;
	Len = PaddingParamsPtr->XAsu_RsaOpComp.Len;

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

	/** Validate input length which should not be greater than KeySize – (2 * hashLen) – 2. */
	if (Len > (KeySize - (XASUFW_DOUBLE_VALUE(HashLen)) - XRSA_INPUT_PADDING_ZERO_AND_ONE_OFFSET)) {
		Status = XASUFW_RSA_OAEP_INVALID_LEN;
		goto END;
	}

	/**
	* Calculate data block length which is KeySize - HashLen - 1 as it is
	* of the format Datablock = Hashofoptionallabel(HashLen) ||
	* 0x00(length= (KeySize - Len - (2U * HashLen) - 2U) || 0x01 || M(Len).
	*/
	DataBlockLen = KeySize - HashLen - XRSA_DATA_BLOCK_LENGTH_OFFSET;

	/** Calculate digest for optional label and append to data block. */
	ShaParamsInput.DataAddr = PaddingParamsPtr->OptionalLabelAddr;
	ShaParamsInput.HashAddr = (u64)(UINTPTR)DataBlock;
	ShaParamsInput.DataSize = PaddingParamsPtr->OptionalLabelSize;
	ShaParamsInput.HashBufSize = HashLen;
	ShaParamsInput.ShaMode = PaddingParamsPtr->ShaMode;

	XFIH_CALL(XSha_Digest, XFihVar, Status, ShaInstancePtr, DmaPtr, &ShaParamsInput);
	XFIH_IF_FAILOUT_WITH_VALUE (XFihVar, ==, (u32)XASUFW_CMD_IN_PROGRESS) {
		goto RET;
	}
	XFIH_IF_FAILOUT_WITH_VALUE (XFihVar, !=, (u32)XASUFW_SUCCESS) {
		Status = XASUFW_RSA_SHA_DIGEST_CALC_FAIL;
		goto END;
	}

	/** Append 0x00's of length (KeySize - Len - (2U * HashLen) - 2U) to data block. */
	if ((KeySize - Len - (XASUFW_DOUBLE_VALUE(HashLen)) - XRSA_DATA_BLOCK_RANDOM_STRING_OFFSET) != 0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemSet(&DataBlock[HashLen], XRSA_MAX_DB_LEN, 0U,
				     (KeySize - Len - (XASUFW_DOUBLE_VALUE(HashLen))
				     - XRSA_DATA_BLOCK_RANDOM_STRING_OFFSET));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ZEROIZE_MEMSET_FAIL;
			goto END;
		}
	}

	/** Append 0x01U to data block after the zeroized memory. */
	DataBlock[KeySize - Len - HashLen - XRSA_DATA_BLOCK_RANDOM_STRING_OFFSET]
	= XRSA_DATA_BLOCK_DELIMITER;

	/** Copy input data to server memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr,
			       (u64)(UINTPTR)&DataBlock[DataBlockLen - Len], Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Get random numbers for seed input. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_TrngGetRandomNumbers(SeedBuffer, HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_RAND_GEN_ERROR;
		goto END;
	}

	/** Generate mask of required length for data block using MGF (Mask Generation Function). */
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

	/** Generate mask of required length for seed block using MGF. */
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
	/** Increment output data index by one after maskedseed and copy masked datablock to output. */
	Status = Xil_SMemCpy(&PaddedOutputData[HashLen + XRSA_OAEP_ZERO_PADDING_DATA_BLOCK_OFFSET],
			      XRSA_MAX_KEY_SIZE_IN_BYTES, DataBlock,
			      XRSA_MAX_DB_LEN, DataBlockLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
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
	ClearStatus = XAsufw_SMemSet(DataBlock, XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PaddedOutputData,
					XRSA_MAX_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

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
 *		- XASUFW_SUCCESS, if OAEP decode operation is successful.
 *		- XASUFW_FAILURE, if OAEP decode operation fails.
 *		- XASUFW_RSA_INVALID_PARAM, if input parameters are invalid.
 *		- XASUFW_RSA_OAEP_INVALID_LEN, if input length is greater than OAEP defined length.
 *		- XASUFW_CMD_IN_PROGRESS, if command is in progress when SHA is operating in DMA
 *		  non-blocking mode.
 *		- XASUFW_DMA_COPY_FAIL, if DMA copy fails.
 *		- XASUFW_MEM_COPY_FAIL, if memcpy fails
 *		- Also, this function can return termination error codes 0xBBU, 0xBCU and 0xBDU,
 *		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_OaepDecode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		    const XAsu_RsaOaepPaddingParams *PaddingParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XFih_Var XFihBufClearStatus = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 KeySize = 0U;
	u32 Len = 0U;
	u32 HashLen = 0U;
	volatile u32 Index = 0U;
	u32 DataBlockLen = 0U;
	u32 MsgLen = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *MaskedDataBlock = DataBlock + XRSA_MAX_DB_LEN;
	u8 *SeedBuffer = MaskedDataBlock + XRSA_MAX_DB_LEN;
	u8 *MaskedSeedBuffer = SeedBuffer + XASU_SHA_512_HASH_LEN;
	u8 HashBuffer[XASU_SHA_512_HASH_LEN];
	u8 DecryptOutputData[XRSA_MAX_KEY_SIZE_IN_BYTES];
	XAsufw_MgfInput MgfInput;
	XAsu_ShaOperationCmd ShaParamsInput;

	/** Validate the input parameters. */
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
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((PaddingParamsPtr->OptionalLabelAddr == 0U) ||
	    (PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr == 0U) || (DmaPtr == NULL) ||
	    (ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	KeySize = PaddingParamsPtr->XAsu_RsaOpComp.KeySize;
	Len = PaddingParamsPtr->XAsu_RsaOpComp.Len;

	/** Validate input key size. */
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

	/** Validate input length which should not be less than (2 * hashLen) + 2. */
	if ((Len != KeySize) || (Len < ((XASUFW_DOUBLE_VALUE(HashLen)) +
		XRSA_INPUT_PADDING_ZERO_AND_ONE_OFFSET))) {
		Status = XASUFW_RSA_OAEP_INVALID_LEN;
		goto END;
	}

	/**
	* Calculate data block length which is KeySize - HashLen - 1 as it is
	* of the format Datablock = Hashofoptionallabel(HashLen) ||
	* 0x00(length= (KeySize - Len - (2U * HashLen) - 2U) || 0x01 || M(Len).
	*/
	DataBlockLen = KeySize - HashLen - XRSA_DATA_BLOCK_LENGTH_OFFSET;

	/** Calculate digest for optional label. */
	ShaParamsInput.DataAddr = PaddingParamsPtr->OptionalLabelAddr;
	ShaParamsInput.HashAddr = (u64)(UINTPTR)HashBuffer;
	ShaParamsInput.DataSize = PaddingParamsPtr->OptionalLabelSize;
	ShaParamsInput.HashBufSize = HashLen;
	ShaParamsInput.ShaMode = PaddingParamsPtr->ShaMode;

	XFIH_CALL(XSha_Digest, XFihVar, Status, ShaInstancePtr, DmaPtr, &ShaParamsInput);
	XFIH_IF_FAILOUT_WITH_VALUE (XFihVar, ==, (u32)XASUFW_CMD_IN_PROGRESS) {
		goto RET;
	}
	XFIH_IF_FAILOUT_WITH_VALUE (XFihVar, !=, (u32)XASUFW_SUCCESS) {
		Status = XASUFW_RSA_SHA_DIGEST_CALC_FAIL;
		goto END;
	}

	/** Perform private exponentiation decryption operation. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_PvtExp(DmaPtr, PaddingParamsPtr->XAsu_RsaOpComp.Len,
			     PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr,
			     (u64)(UINTPTR)DecryptOutputData,
			     PaddingParamsPtr->XAsu_RsaOpComp.KeyCompAddr,
			     PaddingParamsPtr->XAsu_RsaOpComp.ExpoCompAddr);
	if ((Status != XASUFW_SUCCESS) || (ReturnStatus != XASUFW_RSA_DECRYPTION_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_DECRYPT_ERROR);
		goto END;
	}
	ReturnStatus = XASUFW_FAILURE;

	/** Copy seed buffer and data block from decrypted output. */
	/** EM = Y || maskedSeed || maskedDB . */
	/** Increment output data index by one after maskedseed and point to masked datablock. */
	MaskedSeedBuffer = &DecryptOutputData[XRSA_DATA_BLOCK_SECOND_INDEX];
	MaskedDataBlock = &DecryptOutputData[HashLen + XRSA_OAEP_ZERO_PADDING_DATA_BLOCK_OFFSET];

	/** Generate mask of required length for seed block using MGF. */
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

	/** Generate mask of required length for data block using MGF. */
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

	/**
	* DB = lHash(hash of optional label) || 0x00(length= (KeySize - Len - (2U * HashLen) - 2U)
	* || 0x01 || M.
	*/

	/**
	* Iterate a loop until last but one index in data block to check for 0x01U from which
	* original message is separated from rest of the data block.
	*/
	Index = HashLen;
	while ((Index < (DataBlockLen - XASUFW_BUFFER_INDEX_ONE))
		&& (DataBlock[Index] == XRSA_OAEP_ZERO_PADDING_DATA_VALUE)) {
		Index++;
	}
	/**
	* Check for OAEP decoding errors.
	* a.Leading octet check in first byte of decrypted data to be zero.
	* b.Compare generated hash from optional label with data block from index zero until
	* hash length.
	* c.Validating the padding string to have zeroes until 0x01U is found and copy data to
	* output data address using DMA on successful comparison.
	*/
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((DecryptOutputData[XRSA_DATA_BLOCK_FIRST_INDEX] != 0U) ||
		(XASUFW_SUCCESS != Xil_SMemCmp(DataBlock, XRSA_MAX_DB_LEN, HashBuffer,
		XASU_SHA_512_HASH_LEN, HashLen)) || ((Index >= (DataBlockLen - XASUFW_BUFFER_INDEX_ONE)) ||
		(DataBlock[Index] != XRSA_OAEP_OUTPUT_DATA_BLOCK_MSG_SEPERATION_VALUE))) {
		Status = XASUFW_RSA_OAEP_DECODE_ERROR;
		XFIH_GOTO(END);
	}
	Index++;
	MsgLen = DataBlockLen - Index;
	/** Validate input length which should not be greater than KeySize – (2 * hashLen) – 2. */
	if (MsgLen > (KeySize - (XASUFW_DOUBLE_VALUE(HashLen)) - XRSA_INPUT_PADDING_ZERO_AND_ONE_OFFSET)) {
		Status = XASUFW_RSA_OAEP_INVALID_LEN;
		goto END;
	}

	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)(&(DataBlock[Index])),
				PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr,
				MsgLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
	}

END:
	/** Zeroize local copy of all the parameters. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufClearStatus, ClearStatus, DataBlock,
					XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihBufClearStatus, ClearStatus, HashBuffer,
			XASU_SHA_512_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihBufClearStatus, ClearStatus, DecryptOutputData,
			XRSA_MAX_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA PSS padding for the provided message to output a data
 * 		of length equal to key size.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	ShaInstancePtr		Pointer to the Sha instance.
 * @param	PaddingParamsPtr	Pointer to the XAsu_RsaPaddingParams structure which
 * 					holds PSS padding related parameters.
 *
 * @return
 *		- XASUFW_SUCCESS, if PSS sign generation is successful.
 *		- XASUFW_FAILURE, if PSS sign generation fails.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- XASUFW_CMD_IN_PROGRESS, if command is in progress when SHA is operating in DMA
 *		  non-blocking mode.
 *		- XASUFW_ZEROIZE_MEMSET_FAIL, if memset with zero fails.
 *		- XASUFW_DMA_COPY_FAIL, if DMA copy fails.
 *		- Also, this function can return termination error codes from 0xBFU to 0xC3U and 0xD0U,
 *		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_PssSignGenerate(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
			const XAsu_RsaPaddingParams *PaddingParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 SaltLen = 0U;
	u32 KeySize = 0U;
	u32 HashLen = 0U;
	u32 DataBlockLen = 0U;
	u32 Index = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *MPrime = DataBlock + XRSA_MAX_DB_LEN;
	u8 *HashBuffer = MPrime + XRSA_PSS_MAX_MSG_LEN;
	u8 OutputData[XRSA_MAX_KEY_SIZE_IN_BYTES];
	XAsufw_MgfInput MgfInput;
	XAsu_ShaOperationCmd ShaParamsInput;

	/** Validate the input parameters. */
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
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr == 0U) || (DmaPtr == NULL) ||
	    (ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	SaltLen = PaddingParamsPtr->SaltLen;
	KeySize = PaddingParamsPtr->XAsu_RsaOpComp.KeySize;

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

	/** The length (in bytes) of the salt shall satisfy 0 <= SaltLen <= HashLen. */
	if (SaltLen > HashLen) {
		Status = XASUFW_RSA_PSS_INVALID_SALT_LEN;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Validate input length which should not be less than HashLen + SaltLen + 2. */
	if ((PaddingParamsPtr->InputDataType != XASU_RSA_HASHED_INPUT_DATA) &&
		(PaddingParamsPtr->XAsu_RsaOpComp.Len < (HashLen + SaltLen +
		XRSA_INPUT_PADDING_ZERO_AND_ONE_OFFSET))) {
		Status = XASUFW_RSA_PSS_INVALID_LEN;
		goto END;
	}

	if ((PaddingParamsPtr->InputDataType == XASU_RSA_HASHED_INPUT_DATA) &&
		(PaddingParamsPtr->XAsu_RsaOpComp.Len != HashLen)) {
			Status = XASUFW_RSA_INVALID_PARAM;
			goto END;
	}

	/**
	* Calculate data block length which is KeySize - HashLen - 1 as it is of the format
	* Datablock = 0x00(length= (KeySize - SaltLen - HashLen - 2) || 0x01 || salt (SaltLen).
	*/
	DataBlockLen = KeySize - HashLen - XRSA_DATA_BLOCK_LENGTH_OFFSET;

	/**
	* Calculate digest for input if it is non hashed input type and copy calculated hash to
	* MPrime after the zeroized area or only copy if input is already hashed input type.
	*/

	if (PaddingParamsPtr->InputDataType != XASU_RSA_HASHED_INPUT_DATA) {
		ShaParamsInput.DataAddr = PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr;
		ShaParamsInput.HashAddr = (u64)(UINTPTR)&MPrime[XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN];
		ShaParamsInput.DataSize = PaddingParamsPtr->XAsu_RsaOpComp.Len;
		ShaParamsInput.HashBufSize = HashLen;
		ShaParamsInput.ShaMode = PaddingParamsPtr->ShaMode;

		XFIH_CALL(XSha_Digest, XFihVar, Status, ShaInstancePtr, DmaPtr, &ShaParamsInput);
		XFIH_IF_FAILOUT_WITH_VALUE (XFihVar, ==, (u32)XASUFW_CMD_IN_PROGRESS) {
			goto RET;
		}
		XFIH_IF_FAILOUT_WITH_VALUE (XFihVar, !=, (u32)XASUFW_SUCCESS) {
			Status = XASUFW_RSA_SHA_DIGEST_CALC_FAIL;
			goto END;
		}
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr,
				       (u64)(UINTPTR)&MPrime[XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
				       PaddingParamsPtr->XAsu_RsaOpComp.Len, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
	}

	/** Zeroize eight bytes of hash block. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(MPrime, XRSA_PSS_MAX_MSG_LEN, 0U, XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/**
	* Generate random numbers based on input salt length and append to hash block from
	* the index after the calculated hash.
	*/
	/** MPrime = 00 00 00 00 00 00 00 00 || mHash(hash of input) || salt. */
	if (SaltLen != 0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_TrngGetRandomNumbers(&MPrime[HashLen + XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
						     SaltLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_RAND_GEN_ERROR;
			goto END;
		}
	}

	/** Calculate digest for hash block after salt is added. */
	ShaParamsInput.DataAddr = (u64)(UINTPTR)MPrime;
	ShaParamsInput.HashAddr = (u64)(UINTPTR)HashBuffer;
	ShaParamsInput.DataSize = HashLen + SaltLen + XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN;
	ShaParamsInput.HashBufSize = HashLen;
	ShaParamsInput.ShaMode = PaddingParamsPtr->ShaMode;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaParamsInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_SHA_DIGEST_CALC_FAIL);
		goto END;
	}

	/**
	* Append 0x00 of length KeySize - SaltLen - HashLen - 2 to data block and then add 0x01
	* and salt i.e of the form DB = PS || 0x01 || salt.
	*/

	/**
	* Check length of randomized string that if it is zero it is an error else zerioze required
	* length which is KeySize - SaltLen - HashLen - 2.
	*/
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((KeySize - SaltLen - HashLen - XRSA_DATA_BLOCK_RANDOM_STRING_OFFSET) == 0U) {
		Status = XASUFW_RSA_PSS_INVALID_LEN;
		goto END;
	}
	Status = Xil_SMemSet(DataBlock, XRSA_MAX_DB_LEN, 0U, (KeySize - SaltLen - HashLen -
			     XRSA_DATA_BLOCK_RANDOM_STRING_OFFSET));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}
	DataBlock[KeySize - SaltLen - HashLen - XRSA_DATA_BLOCK_RANDOM_STRING_OFFSET]
	= XRSA_DATA_BLOCK_DELIMITER;
	if (SaltLen != 0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy(&DataBlock[DataBlockLen - SaltLen], XRSA_MAX_DB_LEN,
				     &MPrime[HashLen + XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
				     XRSA_PSS_MAX_MSG_LEN, SaltLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}
	}

	/** Generate mask of required length for data block using MGF. */
	MgfInput.Output = OutputData;
	MgfInput.OutputLen = DataBlockLen;
	MgfInput.Seed = HashBuffer;
	MgfInput.SeedLen = HashLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_MaskGenFunc(DmaPtr, ShaInstancePtr, PaddingParamsPtr->ShaMode, &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR);
		goto END;
	}

	for (Index = 0U; Index < DataBlockLen; Index++) {
		OutputData[Index] ^= DataBlock[Index];
	}

	/** Make MSBit of output data as zero. */
	OutputData[XRSA_DATA_BLOCK_FIRST_INDEX] = OutputData[XRSA_DATA_BLOCK_FIRST_INDEX]
							& XRSA_PSS_MSB_PADDING_MASK;

	/** Append hash buffer and end byte 0xBC for final padded output. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(&OutputData[DataBlockLen],
			     XRSA_MAX_KEY_SIZE_IN_BYTES, HashBuffer, XASU_SHA_512_HASH_LEN,
			     HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/**
	* Append 0xBC to the last index of the output by decrementing key size with one as
	* output data length is equal to key size which makes encoded message of the form
	* EM = maskedDB || H(Hash buffer) || 0xBC.
	*/
	OutputData[KeySize - XRSA_PSS_DATA_BLOCK_END_BYTE_OFFSET] = XRSA_PSS_OUTPUT_END_BYTE_VALUE;

	/** Perform private exponentiation decryption operation. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_PvtExp(DmaPtr, KeySize,
			     (u64)(UINTPTR)OutputData, PaddingParamsPtr->XAsu_RsaOpComp.OutputDataAddr,
			     PaddingParamsPtr->XAsu_RsaOpComp.KeyCompAddr,
			     PaddingParamsPtr->XAsu_RsaOpComp.ExpoCompAddr);
	if ((Status != XASUFW_SUCCESS ) || (ReturnStatus != XASUFW_RSA_DECRYPTION_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_ENCRYPT_ERROR);
	}
	ReturnStatus = XASUFW_FAILURE;

END:
	/** Zeroize local copy of all the parameters. */
	XFIH_CALL(Xil_SecureZeroize, XFihVar, ClearStatus, DataBlock,
					XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)OutputData,
					XRSA_MAX_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

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
 *		- XASUFW_SUCCESS, if PSS sign verification is successful.
 *		- XASUFW_FAILURE, if PSS sign verification fails.
 *		- XASUFW_RSA_INVALID_PARAM, if input parameter validation fails.
 *		- XASUFW_CMD_IN_PROGRESS, if command is in progress when SHA is operating in DMA
 *		  non-blocking mode.
 *		- XASUFW_ZEROIZE_MEMSET_FAIL, if memset with zero fails.
 *		- XASUFW_DMA_COPY_FAIL, if DMA copy fails.
 *		- Also, this function can return termination error codes from 0xC4U to 0xCAU and 0xD0U,

 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_PssSignVerify(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
			const XAsu_RsaPaddingParams *PaddingParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	volatile u32 Index = XASUFW_FAILURE;
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XFih_Var XFihBufClearStatus = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 SaltLen = 0U;
	u32 KeySize = 0U;
	u32 HashLen = 0U;
	u32 DataBlockLen = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *MaskedDataBlock = DataBlock + XRSA_MAX_DB_LEN;
	u8 *MsgBlock = MaskedDataBlock + XRSA_MAX_DB_LEN;
	u8 *HashBuffer = MsgBlock + XRSA_PSS_MAX_MSG_LEN;
	u8 *EncodedMsgHashBuffer = HashBuffer + XASU_SHA_512_HASH_LEN;
	u8 HashedInputData[XASU_SHA_512_HASH_LEN];
	u8 SignedInputData[XRSA_MAX_KEY_SIZE_IN_BYTES];
	XAsufw_MgfInput MgfInput;
	XAsu_ShaOperationCmd ShaParamsInput;

	/** Validate the input parameters. */
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
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((PaddingParamsPtr->SignatureDataAddr == 0U) || (DmaPtr == NULL) ||
	     (ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	SaltLen = PaddingParamsPtr->SaltLen;
	KeySize = PaddingParamsPtr->XAsu_RsaOpComp.KeySize;

	Status = XAsu_RsaValidateKeySize(KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (PaddingParamsPtr->SignatureLen != KeySize) {
		Status = XASUFW_RSA_PSS_INVALID_LEN;
		goto END;
	}

	/** Get hash length based on SHA mode. */
	Status = XSha_GetHashLen(PaddingParamsPtr->ShaMode, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SHA_INVALID_SHA_MODE;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((PaddingParamsPtr->InputDataType == XASU_RSA_HASHED_INPUT_DATA) &&
		(PaddingParamsPtr->XAsu_RsaOpComp.Len != HashLen)) {
			Status = XASUFW_RSA_INVALID_PARAM;
			goto END;
	}

	/** Validate input length which should not be less than HashLen + SaltLen + 2. */
	if ((PaddingParamsPtr->InputDataType != XASU_RSA_HASHED_INPUT_DATA) &&
		(PaddingParamsPtr->XAsu_RsaOpComp.Len < (HashLen + SaltLen +
		XRSA_INPUT_PADDING_ZERO_AND_ONE_OFFSET))) {
		Status = XASUFW_RSA_PSS_INVALID_LEN;
		goto END;
	}

	/** The length (in bytes) of the salt shall satisfy 0 <= SaltLen <= HashLen. */
	if (SaltLen > HashLen) {
		Status = XASUFW_RSA_PSS_INVALID_SALT_LEN;
		goto END;
	}

	/**
	* Calculate data block length which is KeySize - HashLen - 1 as it is of the format
	* Datablock = 0x00(length= (KeySize - SaltLen - HashLen - 2) || 0x01 || salt (SaltLen).
	*/
	DataBlockLen = KeySize - HashLen - XRSA_DATA_BLOCK_LENGTH_OFFSET;

	/**
	* Calculate digest for input if it is non hashed input type and copy calculated hash to
	* hash input after the zeroized area or only copy if input is already hashed input type.
	*/
	if (PaddingParamsPtr->InputDataType != XASU_RSA_HASHED_INPUT_DATA) {
		ShaParamsInput.DataAddr = PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr;
		ShaParamsInput.HashAddr = (u64)(UINTPTR)HashedInputData;
		ShaParamsInput.DataSize = PaddingParamsPtr->XAsu_RsaOpComp.Len;
		ShaParamsInput.HashBufSize = HashLen;
		ShaParamsInput.ShaMode = PaddingParamsPtr->ShaMode;

		XFIH_CALL(XSha_Digest, XFihVar, Status, ShaInstancePtr, DmaPtr, &ShaParamsInput);
		XFIH_IF_FAILOUT_WITH_VALUE (XFihVar, ==, (u32)XASUFW_CMD_IN_PROGRESS) {
			goto RET;
		}
		XFIH_IF_FAILOUT_WITH_VALUE (XFihVar, !=, (u32)XASUFW_SUCCESS) {
			Status = XASUFW_RSA_SHA_DIGEST_CALC_FAIL;
			goto END;
		}
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, PaddingParamsPtr->XAsu_RsaOpComp.InputDataAddr,
				       (u64)(UINTPTR)HashedInputData,
				       PaddingParamsPtr->XAsu_RsaOpComp.Len, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
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

	/**
	* Check for 0xBCU in the end byte i.e indexed at keysize -1 of the decrypted output
	* as it is of length which is equal to keysize and copy masked data block and hash value
	* on successful comparison else error out.
	*/
	/**EM = maskedDB || H || 0xbc. */
	if (SignedInputData[KeySize - XRSA_PSS_DATA_BLOCK_END_BYTE_OFFSET]
		!= XRSA_PSS_OUTPUT_END_BYTE_VALUE) {
		Status = XASUFW_RSA_PSS_RIGHT_MOST_CMP_FAIL;
		XFIH_GOTO(END);
	}

	/**
	* If MSbit is not 0x00 error out, else calculate the mask of data block on successful
	* comparison using MGF.
	*/
	if ((MaskedDataBlock[XRSA_DATA_BLOCK_FIRST_INDEX] &
	     XRSA_PSS_MSB_PADDING_CHECK_MASK) != 0x00U) {
		Status = XASUFW_RSA_PSS_LEFT_MOST_BIT_CMP_FAIL;
		XFIH_GOTO(END);
	}

	MaskedDataBlock = SignedInputData;
	HashBuffer = &SignedInputData[DataBlockLen];

	MgfInput.Output = DataBlock;
	MgfInput.OutputLen = DataBlockLen;
	MgfInput.Seed = HashBuffer;
	MgfInput.SeedLen = HashLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_MaskGenFunc(DmaPtr, ShaInstancePtr, PaddingParamsPtr->ShaMode,
				  &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
						  XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR);
		XFIH_GOTO(END);
	}

	for (Index = 0U; Index < DataBlockLen; Index++) {
		DataBlock[Index] ^= MaskedDataBlock[Index];
	}
	DataBlock[XRSA_DATA_BLOCK_FIRST_INDEX] = DataBlock[XRSA_DATA_BLOCK_FIRST_INDEX]
		& XRSA_PSS_MSB_PADDING_MASK;
	/**
	* Check for 0x00's of length KeySize - SaltLen - HashLen - 2 in data block and then
	* check 0x01U and seperate salt if available i.e of the form DB = PS || 0x01 || salt.
	*/

	/** Check for zeroes of length which is KeySize - SaltLen - HashLen - 2 which is PS. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((KeySize - SaltLen - HashLen - XRSA_DATA_BLOCK_RANDOM_STRING_OFFSET) != 0U) {
		for (Index = 0U; Index < (KeySize - SaltLen - HashLen - XRSA_DATA_BLOCK_RANDOM_STRING_OFFSET); Index++) {
			if (DataBlock[Index] != 0U) {
				Status = XASUFW_RSA_PSS_DECODE_ERROR;
				goto END;
			}
		}
	}
	if (Index != (KeySize - SaltLen - HashLen - XRSA_DATA_BLOCK_RANDOM_STRING_OFFSET)) {
		Status = XASUFW_RSA_LOOP_INDEX_CMP_ERROR;
		goto END;
	}

	if (DataBlock[Index] != XRSA_DATA_BLOCK_DELIMITER) {
		Status = XASUFW_RSA_PSS_DECODE_ERROR;
		goto END;
	}

	if (SaltLen != 0U) {
	/** Increment data block index by one and copy salt to msg block after hash. */
		Status = Xil_SMemCpy(&MsgBlock[HashLen + XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
				     XRSA_PSS_MAX_MSG_LEN,
				     &DataBlock[Index + XRSA_PSS_ONE_PADDING_DATA_BLOCK_OFFSET],
				     XRSA_MAX_DB_LEN, SaltLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}
	}

	/**
	* After arranging message block with (0x)00 00 00 00 00 00 00 00 || mHash || salt
	* calculate the hash and compare with the hash input.
	*/
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(MsgBlock, XRSA_PSS_MAX_MSG_LEN, 0U, XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(&MsgBlock[XRSA_PSS_HASH_BLOCK_ZEROIZE_LEN], XRSA_PSS_MAX_MSG_LEN,
			     HashedInputData, XASU_SHA_512_HASH_LEN, HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
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
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_SHA_DIGEST_CALC_FAIL);
		goto END;
	}

	Status = Xil_SMemCmp(HashBuffer, XASU_SHA_512_HASH_LEN, EncodedMsgHashBuffer,
					XASU_SHA_512_HASH_LEN, HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PSS_HASH_CMP_FAIL;
		goto END;
	}

	ReturnStatus = XASUFW_RSA_PSS_SIGNATURE_VERIFIED;

END:
	/** Zeroize local copy of all the parameters. */
	ClearStatus = XAsufw_SMemSet(DataBlock, XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihBufClearStatus, ClearStatus, HashedInputData,
				   XASU_SHA_512_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihBufClearStatus, ClearStatus, SignedInputData,
				   XRSA_MAX_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function takes an input of variable length and a desired output length as input,
 * and provides fixed output mask using cryptographic hash function.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	ShaInstancePtr	Pointer to the SHA instance.
 * @param	ShaMode		SHA mode selection.
 * @param	MgfInput	Pointer to all required parameters of MGF.
 *
 * @return
 *		- XASUFW_SUCCESS, if mask generation is successful.
 *		- XASUFW_FAILURE, if mask generation fails.
 *		- XASUFW_RSA_INVALID_PARAM, if input parameter validation fails.
 *		- XASUFW_ZEROIZE_MEMSET_FAIL, if memset with zero fails.
 *
 *************************************************************************************************/
static s32 XRsa_MaskGenFunc(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u8 ShaMode,
		     const XAsufw_MgfInput *MgfInput)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufClearStatus = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	u32 Counter = 0U;
	u32 HashLen = 0U;
	u8 Hash[XASU_SHA_512_HASH_LEN];
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

	OutputPtr = MgfInput->Output;
	NoOfIterations = (MgfInput->OutputLen + HashLen - XASUFW_VALUE_ONE) / HashLen;

	while (Counter < NoOfIterations) {
		XAsufw_I2Osp(Counter, XASUFW_WORD_LEN_IN_BYTES, Bytes);

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Start(ShaInstancePtr, ShaMode);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_ERROR);
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(ShaInstancePtr, DmaPtr, (u64)(UINTPTR)MgfInput->Seed,
			MgfInput->SeedLen, (u32)XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_ERROR);
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(ShaInstancePtr, DmaPtr, (u64)(UINTPTR)Bytes,
			XASUFW_WORD_LEN_IN_BYTES, (u32)XASU_TRUE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_ERROR);
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Finish(ShaInstancePtr, DmaPtr, (u32 *)Hash, HashLen,
			XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_ERROR);
			XFIH_GOTO(END);
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy(OutputPtr, OutputSize, Hash, OutputSize, OutputSize);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}

		OutputPtr = &OutputPtr[OutputSize];
		Counter++;
		/** Calculate the output size for last iteration as it can be less than hash length. */
		if (Counter == (NoOfIterations - 1U)) {
			OutputSize = MgfInput->OutputLen - ((NoOfIterations - 1U) * HashLen);
		}
	}

END:
	/** Zeroize local copy of all the parameters. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufClearStatus, ClearStatus, Hash, XASU_SHA_512_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Bytes, XASUFW_WORD_LEN_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}
#endif /* XASU_RSA_PADDING_ENABLE */
/** @} */
