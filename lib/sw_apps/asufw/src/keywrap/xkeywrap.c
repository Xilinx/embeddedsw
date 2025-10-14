/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
 *
 * @file xkeywrap.c
 *
 * This file contains the implementation of the Key Wrap Unwrap APIs.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   02/24/25 Initial release
 * 1.1   am   05/18/25 Fixed implicit conversion of operands
 *       kd   07/23/25 Fixed gcc warnings
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xkeywrap_server_apis Keywrap Server APIs
* @{
*/

/*************************************** Include Files *******************************************/
#include "xkeywrap.h"
#include "xasufw_status.h"
#include "xil_mem.h"
#include "xasufw_util.h"
#include "xfih.h"
#include "xasu_keywrap_common.h"
#include "xasu_aesinfo.h"
#include "xasufw_trnghandler.h"

#ifdef XASU_KEYWRAP_ENABLE
/************************************ Constant Definitions ***************************************/
#define XASUFW_KEYWRAP_BLOCK_ROUND_INDEX	(1U)	/**< Key wrap unwrap half block size in
							bytes */
#define XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES	(8U)	/**< Key wrap unwrap half block size in
							bytes */
#define XASUFW_KEYWRAP_MAX_AES_ROUNDS		(5U)	/**< Key wrap unwrap maximum no of rounds */
#define XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES		(520U)	/**< Key wrap maximum output size */
#define XASUFW_KEYWRAP_MAX_PAD_LEN		(7U)	/**< Key wrap maximum padding length */

/************************************** Type Definitions *****************************************/

/************************************ Variable Definitions ***************************************/
#if XASUFW_ENABLE_PERF_MEASUREMENT
static u64 StartTime; /**< Performance measurement start time. */
static XAsufw_PerfTime PerfTime; /**< Structure holding performance timing results. */
#endif

/************************************ Function Prototypes ****************************************/
static s32 XKeywrap_WrapOp(const XAsu_KeyWrapParams *KeyWrapParamsPtr, XAes *AesInstancePtr,
					XAsufw_Dma *AsuDmaPtr, u8 *OutData);
static s32 XKeyWrap_UnwrapOp(const XAsu_KeyWrapParams *KeyUnwrapParamsPtr, XAes *AesInstancePtr,
					XAsufw_Dma *AsuDmaPtr, u32 *OutDataLenPtr);
/************************************** Macros Definitions ***************************************/

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 *
 * @brief	This function performs key wrap operation using AES engine.
 *
 * @param	KeyWrapParamsPtr	Pointer to the XAsu_KeyWrapParams structure.
 * @param	AsuDmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr		Pointer to the SHA instance.
 * @param	AesInstancePtr		Pointer to the AES instance.
 * @param	OutDataLenPtr		Pointer to the variable to store the actual output length.
 *
 * @return
 * 	- XASUFW_SUCCESS, if initialization is successful.
 * 	- XASUFW_KEYWRAP_INVALID_PARAM, if input parameter validation fails.
 * 	- XASUFW_RSA_RAND_GEN_ERROR, if ephemeral AES key using TRNG generation fails.
 * 	- XASUFW_RSA_OAEP_ENCODE_ERROR, if OAEP encode operation fails.
 * 	- XASUFW_AES_WRITE_KEY_FAILED, if AES write key fails.
 * 	- XASUFW_KEYWRAP_AES_KEY_CLEAR_FAIL, if AES key clear fails.
 * 	- XASUFW_KEYWRAP_AES_WRAPPED_KEY_ERROR, if AES key wrap fails.
 * 	- XASUFW_DMA_COPY_FAIL, if coping wrapped key to user provided address using DMA fails.
 *
 *************************************************************************************************/
s32 XKeyWrap(const XAsu_KeyWrapParams *KeyWrapParamsPtr, XAsufw_Dma *AsuDmaPtr,
			XSha *ShaInstancePtr, XAes *AesInstancePtr, u32 *OutDataLenPtr)
{
	/**
	 * Capture the start time of the key wrap operation, if performance measurement is
	 * enabled.
	 */
	XASUFW_MEASURE_PERF_START();

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 AesKeySizeInBytes = 0U;
	u32 OutDataLen = 0U;
	u32 PadLen = 0U;
	u8 OutData[XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES];
	u8 EphemeralAesKey[XASU_AES_KEY_SIZE_256BIT_IN_BYTES];
	XAsu_RsaOaepPaddingParams PaddingParams;
	XAsu_AesKeyObject AesKeyObj;

	/** Validate input parameters. */
	if ((KeyWrapParamsPtr == NULL) || (AsuDmaPtr == NULL) || (ShaInstancePtr == NULL)
		|| (AesInstancePtr == NULL) || (OutDataLenPtr == NULL)) {
		Status = XASUFW_KEYWRAP_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_KeyWrapUnwrapValidateInputParams(KeyWrapParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_INVALID_PARAM;
		goto END;
	}

	if (KeyWrapParamsPtr->AesKeySize == XASU_AES_KEY_SIZE_128_BITS) {
		AesKeySizeInBytes = XASU_AES_KEY_SIZE_128BIT_IN_BYTES;
	} else {
		AesKeySizeInBytes = XASU_AES_KEY_SIZE_256BIT_IN_BYTES;
	}

	/** Calculate padding length. */
	PadLen = (XASUFW_BYTE_LEN_IN_BITS *
		((KeyWrapParamsPtr->InputDataLen + XASUFW_BYTE_LEN_IN_BITS - XASUFW_VALUE_ONE)
		/ XASUFW_BYTE_LEN_IN_BITS)) - KeyWrapParamsPtr->InputDataLen;

	OutDataLen = KeyWrapParamsPtr->InputDataLen + PadLen + XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES;

	/** Updating actual output length. */
	*OutDataLenPtr = OutDataLen + KeyWrapParamsPtr->RsaKeySize;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (KeyWrapParamsPtr->OutuputDataLen < (*OutDataLenPtr)) {
		Status = XASUFW_KEYWRAP_INVALID_OUTPUT_BUF_LEN;
		goto END;
	}

	/** Generate ephemeral AES key using TRNG. */
	Status = XAsufw_TrngGetRandomNumbers(EphemeralAesKey, AesKeySizeInBytes);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_RAND_GEN_ERROR;
		goto END;
	}

	/**
	 * Update RSA OAEP related parameters for encryption of generated AES Key and perform RSA OAEP
	 * encode operation.
	 */
	PaddingParams.XAsu_RsaOpComp.InputDataAddr = (u64)(UINTPTR)EphemeralAesKey;
	PaddingParams.XAsu_RsaOpComp.OutputDataAddr = KeyWrapParamsPtr->OutputDataAddr;
	PaddingParams.XAsu_RsaOpComp.ExpoCompAddr = KeyWrapParamsPtr->ExpoCompAddr;
	PaddingParams.XAsu_RsaOpComp.KeyCompAddr= KeyWrapParamsPtr->KeyCompAddr;
	PaddingParams.XAsu_RsaOpComp.Len = AesKeySizeInBytes;
	PaddingParams.XAsu_RsaOpComp.KeySize = KeyWrapParamsPtr->RsaKeySize;
	PaddingParams.OptionalLabelAddr = KeyWrapParamsPtr->OptionalLabelAddr;
	PaddingParams.OptionalLabelSize = KeyWrapParamsPtr->OptionalLabelSize;
	PaddingParams.ShaMode = KeyWrapParamsPtr->ShaMode;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_OaepEncode(AsuDmaPtr, ShaInstancePtr, &PaddingParams);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_OAEP_ENCODE_ERROR;
		goto END;
	}

	AesKeyObj.KeyAddress = (u64)(UINTPTR)EphemeralAesKey;
	AesKeyObj.KeySize = KeyWrapParamsPtr->AesKeySize;
	AesKeyObj.KeySrc = XASU_AES_USER_KEY_7;

	/** Write AES key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_WriteKey(AesInstancePtr, AsuDmaPtr, (u64)(UINTPTR)&AesKeyObj);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_WRITE_KEY_FAILED;
		goto END_KEY_CLR;
	}

	/** Zeroize AES key immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, EphemeralAesKey,
		XASU_AES_KEY_SIZE_256BIT_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_AES_KEY_CLEAR_FAIL;
		goto END_KEY_CLR;
	}

	/** Perform AES Key Wrap. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeywrap_WrapOp(KeyWrapParamsPtr, AesInstancePtr, AsuDmaPtr, OutData);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_AES_WRAPPED_KEY_ERROR;
		goto END_CLR;
	}

	/** Copy wrapped output data to the user provided output memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(AsuDmaPtr, (u64)(UINTPTR)OutData,
				(KeyWrapParamsPtr->OutputDataAddr + KeyWrapParamsPtr->RsaKeySize),
				OutDataLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
	}

	/**
	 * Measure and print the performance time for the key wrap operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(__func__);

END_CLR:
	/** Zeroize output data. */
	ClearStatus = Xil_SecureZeroize(OutData, XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END_KEY_CLR:
	/** Clear the key written to the XASU_AES_USER_KEY_7 key source. */
	Status = XAsufw_UpdateErrorStatus(Status, XAes_KeyClear(AesInstancePtr, AesKeyObj.KeySrc));

END:
	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function performs key unwrap operation using AES.
 *
 * @param	KeyUnwrapParamsPtr	Pointer to the XAsu_KeyWrapParams structure.
 * @param	AsuDmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr		Pointer to the SHA instance.
 * @param	AesInstancePtr		Pointer to the AES instance.
 * @param	OutDataLenPtr		Pointer to the variable to store the actual output length.
 *
 * @return
 * 	- XASUFW_SUCCESS, if initialization is successful.
 * 	- XASUFW_KEYWRAP_INVALID_PARAM, if input parameter validation fails.
 * 	- XASUFW_RSA_OAEP_DECODE_ERROR, if OAEP decode operation fails.
 * 	- XASUFW_AES_WRITE_KEY_FAILED, if AES write key fails.
 * 	- XASUFW_KEYWRAP_AES_KEY_CLEAR_FAIL, if AES key clear fails.
 * 	- XASUFW_KEYWRAP_AES_UNWRAPPED_KEY_ERROR, if AES key unwrap fails.
 *
 *************************************************************************************************/
s32 XKeyUnwrap(const XAsu_KeyWrapParams *KeyUnwrapParamsPtr, XAsufw_Dma *AsuDmaPtr,
			XSha *ShaInstancePtr, XAes *AesInstancePtr,  u32 *OutDataLenPtr)
{
	/**
	 * Capture the start time of the key unwrap operation, if performance measurement is
	 * enabled.
	 */
	XASUFW_MEASURE_PERF_START();

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 AesKeyOut[XASU_AES_KEY_SIZE_256BIT_IN_BYTES];
	XAsu_RsaOaepPaddingParams PaddingParams;
	XAsu_AesKeyObject AesKeyObj;

	/** Validate input parameters. */
	if ((KeyUnwrapParamsPtr == NULL) || (AsuDmaPtr == NULL) || (ShaInstancePtr == NULL)
		|| (AesInstancePtr == NULL) ||  (OutDataLenPtr == NULL)) {
		Status = XASUFW_KEYWRAP_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_KeyWrapUnwrapValidateInputParams(KeyUnwrapParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_INVALID_PARAM;
		goto END;
	}

	/**
	 * Update RSA OAEP related parameters for decryption of provided input to extract AES key
	 * and perform RSA OAEP decode operation.
	 */
	PaddingParams.XAsu_RsaOpComp.InputDataAddr = KeyUnwrapParamsPtr->InputDataAddr;
	PaddingParams.XAsu_RsaOpComp.OutputDataAddr = (u64)(UINTPTR)AesKeyOut;
	PaddingParams.XAsu_RsaOpComp.ExpoCompAddr = KeyUnwrapParamsPtr->ExpoCompAddr;
	PaddingParams.XAsu_RsaOpComp.KeyCompAddr= KeyUnwrapParamsPtr->KeyCompAddr;
	PaddingParams.XAsu_RsaOpComp.Len = KeyUnwrapParamsPtr->RsaKeySize;
	PaddingParams.XAsu_RsaOpComp.KeySize = KeyUnwrapParamsPtr->RsaKeySize;
	PaddingParams.OptionalLabelAddr = KeyUnwrapParamsPtr->OptionalLabelAddr;
	PaddingParams.OptionalLabelSize = KeyUnwrapParamsPtr->OptionalLabelSize;
	PaddingParams.ShaMode = KeyUnwrapParamsPtr->ShaMode;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_OaepDecode(AsuDmaPtr, ShaInstancePtr, &PaddingParams);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_OAEP_DECODE_ERROR;
		goto END;
	}

	AesKeyObj.KeyAddress = (u64)(UINTPTR)AesKeyOut;
	AesKeyObj.KeySize = KeyUnwrapParamsPtr->AesKeySize;
	AesKeyObj.KeySrc = XASU_AES_USER_KEY_7;

	/** Write AES key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_WriteKey(AesInstancePtr, AsuDmaPtr, (u64)(UINTPTR)&AesKeyObj);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_WRITE_KEY_FAILED;
		goto END_KEY_CLR;
	}

	/** Zeroize AES key immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, AesKeyOut,
		XASU_AES_KEY_SIZE_256BIT_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_AES_KEY_CLEAR_FAIL;
		goto END_KEY_CLR;
	}

	/** Perform AES Key Unwrap. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyWrap_UnwrapOp(KeyUnwrapParamsPtr, AesInstancePtr, AsuDmaPtr, OutDataLenPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_AES_UNWRAPPED_KEY_ERROR;
	}

	/**
	 * Measure and print the performance time for the key unwrap operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(__func__);

END_KEY_CLR:
	ASSIGN_VOLATILE(ClearStatus, XASUFW_FAILURE);
	ClearStatus = XAes_KeyClear(AesInstancePtr, AesKeyObj.KeySrc);
	if (ClearStatus != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, ClearStatus);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function performs key wrap operation using AES.
 *
 * @param	KeyWrapParamsPtr	Pointer to the XAsu_KeyWrapParams structure.
 * @param	AesInstancePtr		Pointer to the XAes instance.
 * @param	AsuDmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	OutData			Pointer to store wrapped output.
 *
 * @return
 * 	- XASUFW_SUCCESS, if initialization is successful.
 * 	- XASUFW_KEYWRAP_INVALID_PARAM, if input parameter validation fails.
 * 	- XASUFW_MEM_COPY_FAIL, if copying of ICV fails.
 * 	- XASUFW_DMA_COPY_FAIL, if input data copy using DMA fails.
 * 	- XASUFW_ZEROIZE_MEMSET_FAIL, if zeroization fails for padding length.
 * 	- XASUFW_KEYWRAP_AES_DATA_CALC_FAIL, if AES operation fails.
 *
 *************************************************************************************************/
static s32 XKeywrap_WrapOp(const XAsu_KeyWrapParams *KeyWrapParamsPtr, XAes *AesInstancePtr,
				XAsufw_Dma *AsuDmaPtr, u8 *OutData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	const u8 InitValue[XASUFW_WORD_LEN_IN_BYTES] = {0xA6U,0x59U,0x59U,0xA6U};
	u64 AesOutValue = 0U;
	u32 PadLen = 0U;
	u32 MaxRounds = 0U;
	u8 *InData = XRsa_GetDataBlockAddr();
	u8 *AesInData = InData + XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES;
	u8 *AesOutData = AesInData + XASU_AES_BLOCK_SIZE_IN_BYTES;
	u32 RoundNum = 0U;
	u32 BlkRoundNum = 0U;
	u32 CopyLen = 0U;
	u32 *AesInDataSemiBlockPtr = (u32 *)(AesInData + XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES);
	XAsu_AesParams AesParams;
	XAsu_AesKeyObject AesKeyObj;

	/** Validate input parameters. */
	if ((KeyWrapParamsPtr == NULL) || (AsuDmaPtr == NULL) || (AesInstancePtr == NULL)
		|| (OutData == NULL)) {
		Status = XASUFW_KEYWRAP_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_KeyWrapUnwrapValidateInputParams(KeyWrapParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_INVALID_PARAM;
		goto END;
	}

	/**
	 * Calculate number of padding bytes that are needed to align input data length ensuring total
	 * length is a multiple of 8 bytes.
	 */
	PadLen = (XASUFW_BYTE_LEN_IN_BITS *
		((KeyWrapParamsPtr->InputDataLen + XASUFW_BYTE_LEN_IN_BITS - XASUFW_VALUE_ONE)
		/ XASUFW_BYTE_LEN_IN_BITS)) - KeyWrapParamsPtr->InputDataLen;

	/**
	 * Calculate the number of rounds required for the wrap operation by calculating the number
	 * of semi-blocks present in the input data.
	 */
	MaxRounds = ((PadLen + KeyWrapParamsPtr->InputDataLen +
			XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES) / XASUFW_BYTE_LEN_IN_BITS) - 1U;

	/** Copy integrity check value (ICV) to input data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(InData, XRSA_MAX_KEY_SIZE_IN_BYTES, InitValue,
				XASUFW_WORD_LEN_IN_BYTES, XASUFW_WORD_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END_CLR;
	}

	/** Copy message length indicator (MLI) to input data. */
	XAsufw_I2Osp(KeyWrapParamsPtr->InputDataLen, XASUFW_WORD_LEN_IN_BYTES,
			&InData[XASUFW_WORD_LEN_IN_BYTES]);

	/** Copy input data from user memory to ASU memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(AsuDmaPtr, KeyWrapParamsPtr->InputDataAddr,
				(u64)(UINTPTR)&InData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES],
				KeyWrapParamsPtr->InputDataLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END_CLR;
	}

	/**
	 * Append 0's if calculated padding length is non zero.
	 * This arranges the input data block in the format ICV || MLI || Plaintext || Padding.
	 */
	if (PadLen != 0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemSet(&InData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES +
					KeyWrapParamsPtr->InputDataLen],
					XRSA_MAX_KEY_SIZE_IN_BYTES, 0U, PadLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ZEROIZE_MEMSET_FAIL;
			goto END_CLR;
		}
	}

	/** Update AES related parameters for encryption of provided input. */
	AesParams.EngineMode = XASU_AES_ECB_MODE;
	AesParams.OperationType = XASU_AES_ENCRYPT_OPERATION;

	AesKeyObj.KeyAddress = 0U;
	AesKeyObj.KeySize = KeyWrapParamsPtr->AesKeySize;
	AesKeyObj.KeySrc = XASU_AES_USER_KEY_7;

	AesParams.KeyObjectAddr = (u64)(UINTPTR)&AesKeyObj;
	AesParams.IvAddr = 0U;
	AesParams.IvLen = 0U;
	AesParams.InputDataAddr = (u64)(UINTPTR)AesInData;
	AesParams.OutputDataAddr = (u64)(UINTPTR)AesOutData;
	AesParams.DataLen = XASU_AES_BLOCK_SIZE_IN_BYTES;
	AesParams.AadAddr = 0U;
	AesParams.AadLen = 0U;
	AesParams.TagAddr = 0U;
	AesParams.TagLen = 0U;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(AesInData, XASU_AES_BLOCK_SIZE_IN_BYTES, InData,
				XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES,
				XASU_AES_BLOCK_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END_CLR;
	}

	/**
	 * If the input data length exceeds 8 bytes, apply the AES Key Wrap with Padding (KWP)
	 * operation to wrap the data. Else, perform a standard AES encryption operation on the input data.
	 */
	if (KeyWrapParamsPtr->InputDataLen > XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES) {
		for(RoundNum = 0U; RoundNum <= XASUFW_KEYWRAP_MAX_AES_ROUNDS; RoundNum++) {
			for(BlkRoundNum = XASUFW_KEYWRAP_BLOCK_ROUND_INDEX; BlkRoundNum <= MaxRounds;
				BlkRoundNum++) {
				/**
				 * - Copy input/output data based on round number of length eight bytes in terms
				 * of two words to AES input data.
				 */
				if (RoundNum == 0U) {
					Xil_MemCpy(AesInDataSemiBlockPtr, InData +
						(XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES
						* BlkRoundNum), XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES);
				} else {
					Xil_MemCpy(AesInDataSemiBlockPtr, OutData +
						(XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES
						* BlkRoundNum), XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES);
				}
				/** - Perform AES operation. */
				ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
				Status = XAes_Compute(AesInstancePtr, AsuDmaPtr, &AesParams);
				if (Status != XASUFW_SUCCESS) {
					Status =  XASUFW_KEYWRAP_AES_DATA_CALC_FAIL;
					XFIH_GOTO(END_CLR);
				}
				AesOutValue = ((u64)AesOutData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES
						- XASUFW_BUFFER_INDEX_TWO] << XASUFW_ONE_BYTE_SHIFT_VALUE)
						| AesOutData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES
								- XASUFW_BUFFER_INDEX_ONE];
				AesOutValue = AesOutValue ^ (((u64)MaxRounds * RoundNum) + BlkRoundNum);
				AesOutData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES - XASUFW_BUFFER_INDEX_TWO]
				= (u8)(AesOutValue >> XASUFW_ONE_BYTE_SHIFT_VALUE);
				AesOutData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES
						- XASUFW_BUFFER_INDEX_ONE] = (u8)AesOutValue;
				/**
				 * - Copy AES output data of length eight bytes in terms of two words to AES input
				 * data for next iteration.
				 */
				ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
				Status = Xil_SMemCpy(AesInData, XASU_AES_BLOCK_SIZE_IN_BYTES, AesOutData, XASU_AES_BLOCK_SIZE_IN_BYTES,
						XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES);
				if (Status != XASUFW_SUCCESS) {
					Status = XASUFW_MEM_COPY_FAIL;
					goto END_CLR;
				}
				/**
				 * - Copy AES output data of length eight bytes in terms of two words to output
				 * data.
				 */
				ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
				Status = Xil_SMemCpy(OutData + (XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES * BlkRoundNum), XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES,
						AesOutData + XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES, XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES,
						XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES);
				if (Status != XASUFW_SUCCESS) {
					Status = XASUFW_MEM_COPY_FAIL;
					goto END_CLR;
				}
			}
		}
		CopyLen = XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES;
	} else {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAes_Compute(AesInstancePtr, AsuDmaPtr, &AesParams);
		if (Status != XASUFW_SUCCESS) {
			Status =  XASUFW_KEYWRAP_AES_DATA_CALC_FAIL;
			goto END_CLR;
		}
		CopyLen = XASU_AES_BLOCK_SIZE_IN_BYTES;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(OutData, XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES, AesOutData,
		XASU_AES_BLOCK_SIZE_IN_BYTES, CopyLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
	}

END_CLR:
	/** Zeroize local buffer of input data. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, InData,
			XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES + (XASU_AES_BLOCK_SIZE_IN_BYTES * 2U));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function performs key unwrap operation using AES.
 *
 * @param	KeyUnwrapParamsPtr	Pointer to the XAsu_KeyWrapParams structure.
 * @param	AesInstancePtr		Pointer to the XAes instance.
 * @param	AsuDmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	OutDataLenPtr		Pointer to the variable to store the actual output length.
 *
 * @return
 * 	- XASUFW_SUCCESS, if initialization is successful.
 * 	- XASUFW_KEYWRAP_INVALID_PARAM, if input parameter validation fails.
 * 	- XASUFW_MEM_COPY_FAIL, if Xil_SMemCpy fails.
 * 	- XASUFW_DMA_COPY_FAIL, if input data copy using DMA fails.
 * 	- XASUFW_KEYWRAP_AES_DATA_CALC_FAIL, if AES operation fails.
 * 	- XASUFW_KEYWRAP_ICV_CMP_FAIL, if ICV comparison with first 4 bytes of output fails.
 *
 *************************************************************************************************/
static s32 XKeyWrap_UnwrapOp(const XAsu_KeyWrapParams *KeyUnwrapParamsPtr, XAes *AesInstancePtr,
				XAsufw_Dma *AsuDmaPtr, u32 *OutDataLenPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	const u8 InitValue[XASUFW_WORD_LEN_IN_BYTES] = {0xA6U,0x59U,0x59U,0xA6U};
	u64 AesInValue = 0U;
	u32 MaxRounds = 0U;
	u8 *InData = XRsa_GetDataBlockAddr();
	u8 *AesInData = InData + XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES;
	u8 *AesOutData = AesInData + XASU_AES_BLOCK_SIZE_IN_BYTES;
	u8 *OutData = AesOutData + XASU_AES_BLOCK_SIZE_IN_BYTES;
	s32 RoundNum = 0;
	u32 BlkRoundNum = 0U;
	u32 PadLen = 0U;
	u32 CopyLen = 0U;
	volatile u32 Index = 0U;
	u32 *AesInDataSemiBlockPtr = (u32 *)(AesInData + XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES);
	XAsu_AesParams AesParams;
	XAsu_AesKeyObject AesKeyObj;

	/** Validate input parameters. */
	if ((KeyUnwrapParamsPtr == NULL) || (AsuDmaPtr == NULL) || (AesInstancePtr == NULL)
		||  (OutDataLenPtr == NULL)) {
		Status = XASUFW_KEYWRAP_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_KeyWrapUnwrapValidateInputParams(KeyUnwrapParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_INVALID_PARAM;
		goto END;
	}

	/** Check if input data length exceeds max allocated input size for key unwrap operation. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((KeyUnwrapParamsPtr->InputDataLen - KeyUnwrapParamsPtr->RsaKeySize) >
		XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES) {
		Status = XASUFW_KEYWRAP_INVALID_PARAM;
		goto END;
	}

	/** Copy wrapped input data from user memory to ASU memory using DMA. */
	Status = XAsufw_DmaXfr(AsuDmaPtr, (KeyUnwrapParamsPtr->InputDataAddr +
				KeyUnwrapParamsPtr->RsaKeySize), (u64)(UINTPTR)InData,
				(KeyUnwrapParamsPtr->InputDataLen - KeyUnwrapParamsPtr->RsaKeySize),
				0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END_CLR;
	}

	MaxRounds = ((KeyUnwrapParamsPtr->InputDataLen - KeyUnwrapParamsPtr->RsaKeySize) /
			XASUFW_BYTE_LEN_IN_BITS) - 1U;

	/** Update AES related parameters for decryption of provided input. */
	AesParams.EngineMode = (u8)XASU_AES_ECB_MODE;
	AesParams.OperationType = XASU_AES_DECRYPT_OPERATION;

	AesKeyObj.KeyAddress = 0U;
	AesKeyObj.KeySize = KeyUnwrapParamsPtr->AesKeySize;
	AesKeyObj.KeySrc = XASU_AES_USER_KEY_7;

	AesParams.KeyObjectAddr = (u64)(UINTPTR)&AesKeyObj;
	AesParams.IvAddr = 0U;
	AesParams.IvLen = 0U;
	AesParams.InputDataAddr = (u64)(UINTPTR)AesInData;
	AesParams.OutputDataAddr = (u64)(UINTPTR)AesOutData;
	AesParams.DataLen = XASU_AES_BLOCK_SIZE_IN_BYTES;
	AesParams.AadAddr = 0U;
	AesParams.AadLen = 0U;
	AesParams.TagAddr = 0U;
	AesParams.TagLen = 0U;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(AesInData, XASU_AES_BLOCK_SIZE_IN_BYTES, InData,
				XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES,
				XASU_AES_BLOCK_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END_CLR;
	}

	/**
	 * If the input data length, after removing RSA-related and padding data, exceeds 8 bytes,
	 * perform AES unwrapping using the Key Unwrap with Padding (KWP) operation. Else, apply a
	 * standard AES operation to the input data.
	 */
	if ((KeyUnwrapParamsPtr->InputDataLen - KeyUnwrapParamsPtr->RsaKeySize -
		XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES) > XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES) {
		for(RoundNum = (s32)XASUFW_KEYWRAP_MAX_AES_ROUNDS; RoundNum >= 0; RoundNum--) {
			for(BlkRoundNum = MaxRounds; BlkRoundNum >= XASUFW_KEYWRAP_BLOCK_ROUND_INDEX;
				BlkRoundNum--) {
				/**
				 * - Copy input/output data based on round number of length eight bytes
				 * in terms of two words to AES input data.
				 */
				if (RoundNum == (s32)XASUFW_KEYWRAP_MAX_AES_ROUNDS) {
					Xil_MemCpy(AesInDataSemiBlockPtr, InData +
						(XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES
						* BlkRoundNum), XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES);
				} else {
					Xil_MemCpy(AesInDataSemiBlockPtr, OutData +
						(XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES
						* BlkRoundNum), XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES);
				}
				AesInValue = ((u64)AesInData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES
						- XASUFW_BUFFER_INDEX_TWO] << XASUFW_ONE_BYTE_SHIFT_VALUE)
						| AesInData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES
								- XASUFW_BUFFER_INDEX_ONE];
				AesInValue = AesInValue ^ (((u64)MaxRounds * (u32)RoundNum) + BlkRoundNum);
				AesInData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES - XASUFW_BUFFER_INDEX_TWO]
				= (u8)(AesInValue >> XASUFW_ONE_BYTE_SHIFT_VALUE);
				AesInData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES
						- XASUFW_BUFFER_INDEX_ONE] = (u8)AesInValue;
				/** - Perform AES operation. */
				ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
				Status = XAes_Compute(AesInstancePtr, AsuDmaPtr, &AesParams);
				if (Status != XASUFW_SUCCESS) {
					Status =  XASUFW_KEYWRAP_AES_DATA_CALC_FAIL;
					XFIH_GOTO(END_CLR);
				}
				/**
				 * - Copy AES output data of length eight bytes in terms of two words
				 * to AES input data for next iteration.
				 */
				ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
				Status = Xil_SMemCpy(AesInData, XASU_AES_BLOCK_SIZE_IN_BYTES, AesOutData, XASU_AES_BLOCK_SIZE_IN_BYTES,
							XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES);
				if (Status != XASUFW_SUCCESS) {
					Status = XASUFW_MEM_COPY_FAIL;
					goto END_CLR;
				}
				/**
				 * - Copy AES output data of length eight bytes in terms of two words
				 * to output data.
				 */
				ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
				Status = Xil_SMemCpy(OutData + (XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES * BlkRoundNum), XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES,
						AesOutData + XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES, XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES,
						XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES);
				if (Status != XASUFW_SUCCESS) {
					Status = XASUFW_MEM_COPY_FAIL;
					goto END_CLR;
				}
			}

		}
		CopyLen = XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES;
	} else {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAes_Compute(AesInstancePtr, AsuDmaPtr, &AesParams);
		if (Status != XASUFW_SUCCESS) {
			Status =  XASUFW_KEYWRAP_AES_DATA_CALC_FAIL;
			goto END_CLR;
		}
		CopyLen = XASU_AES_BLOCK_SIZE_IN_BYTES;
	}

	/** Copy AES unwrap output data to local output buffer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(OutData, XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES, AesOutData,
		XASU_AES_BLOCK_SIZE_IN_BYTES, CopyLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END_CLR;
	}

	/** Compare default integrity check value (ICV) with first four bytes of output data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(OutData, XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES, InitValue,
				XASUFW_WORD_LEN_IN_BYTES, XASUFW_WORD_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_ICV_CMP_FAIL;
		XFIH_GOTO(END_CLR);
	}

	/** Copy actual plain text length to pointer. */
	*OutDataLenPtr = *(u32 *)(OutData + XASUFW_WORD_LEN_IN_BYTES);

	/** Endianness change of output length. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)OutDataLenPtr, XASUFW_WORD_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_CHANGE_ENDIANNESS_ERROR;
		goto END_CLR;
	}

	/**
	 * Calculate and compare padding length which should not exceed maximum allowed padding
	 * length i.e 7 bytes.
	 */
	PadLen = (XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES * MaxRounds) - (*OutDataLenPtr);

	if (PadLen > XASUFW_KEYWRAP_MAX_PAD_LEN) {
		Status = XASUFW_KEYWRAP_INVALID_PAD_LEN;
		goto END_CLR;
	}

	/** Check for padded zeroes of padding length. */
	for (Index = 0U; Index < PadLen; Index++) {
		if (OutData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES + (*OutDataLenPtr) + Index] != 0U) {
			Status = XASUFW_KEYWRAP_INVALID_PAD_VALUE;
			goto END_CLR;
		}
	}

	if (Index != PadLen) {
		Status = XASUFW_KEYWRAP_LOOP_INDEX_CMP_ERROR;
		goto END_CLR;
	}

	/** Copy output data from ASU memory to the user provided memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (KeyUnwrapParamsPtr->OutuputDataLen < (*OutDataLenPtr)) {
		Status = XASUFW_KEYWRAP_INVALID_OUTPUT_BUF_LEN;
		goto END_CLR;
	}

	Status = XAsufw_DmaXfr(AsuDmaPtr,
				(u64)(UINTPTR)&OutData[XASUFW_KEYWRAP_SEMI_BLOCK_SIZE_IN_BYTES],
				KeyUnwrapParamsPtr->OutputDataAddr,
				(*OutDataLenPtr), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
	}

END_CLR:
	/** Zeroize input data local buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, InData,
			XASUFW_KEYWRAP_MAX_OUTPUT_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}
#endif /* XASU_KEYWRAP_ENABLE */
/** @} */
