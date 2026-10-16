/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xkeymanager_crypto.c
 *
 * Crypto primitives the chunked blob format is built from: weak-key rejection, random key and IV
 * draw, KAT readiness, IV advance, SHA-384 digest and compare, and AES-256-CBC over a buffer.
 *
 * Nothing here knows about any blob header, so the body layer in xkeymanager_chunk.c can use
 * these primitives without depending on the format layer above it.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vm   09/15/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xkeymanager_server_apis Key Manager Server APIs
* @{
*/
/*************************************** Include Files *******************************************/
#ifdef SDT
#include "xasu_bsp_config.h"
#endif
#include "xkeymanager_crypto.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_dma.h"
#include "xasufw_hw.h"
#include "xsha.h"
#include "xaes.h"
#include "xtrng.h"
#include "xasu_generic.h"
#include "xasu_shainfo.h"
#include "xasu_aesinfo.h"
#include "xil_sutil.h"

#ifdef XASU_KEYMANAGER_ENABLE
/************************************ Constant Definitions ***************************************/
#define XKEYMANAGER_WEAK_KEY_BYTE_ALL_ZERO		(0x00U) /**< All zero weak key byte */
#define XKEYMANAGER_WEAK_KEY_BYTE_ALL_ONE		(0xFFU) /**< All one weak key byte */
#define XKEYMANAGER_SHA_UPDATE_MAX_LEN		((XASUFW_DMA_BLOCKING_SIZE / \
	XASUFW_SHA3_384_BLOCK_LEN) * XASUFW_SHA3_384_BLOCK_LEN) /**< Longest SHA update that the
	DMA still completes in blocking mode, rounded down to whole SHA3-384 blocks */

/** The SHA engine pads the last update only, so every earlier one is a whole number of blocks. */
_Static_assert((XKEYMANAGER_SHA_UPDATE_MAX_LEN % XASUFW_SHA3_384_BLOCK_LEN) == 0U,
	       "AEKV SHA update length must be a multiple of the SHA3-384 block length");

/** A longer update would put the SHA engine on the non-blocking path. */
_Static_assert(XKEYMANAGER_SHA_UPDATE_MAX_LEN <= XASUFW_DMA_BLOCKING_SIZE,
	       "AEKV SHA update length must stay within the DMA blocking size");

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XKeyManager_AesCbcBuffer(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			      const XKeyManager_CbcParams *ParamsPtr, u8 OpType);
static s32 XKeyManager_MapCompareStatus(s32 CompareStatus);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function rejects all-zero or all-one key or IV material.
 *
 * @param	Key	Pointer to the key or IV material.
 * @param	KeyLen	Length of the material in bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if the key is acceptable.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if inputs are invalid.
 *		- XASUFW_KEYMANAGER_AEKV_WEAK_KEY_MATERIAL, if the key is all-zero or all-one.
 *
 *************************************************************************************************/
s32 XKeyManager_RejectWeakKey(const u8 *Key, u32 KeyLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var IsWeakFih = XFIH_FAILURE;
	XFih_Var NotWeakFih = XFIH_FAILURE;
	u32 Index;
	volatile u8 IsAllZero = (u8)TRUE;
	volatile u8 IsAllOne = (u8)TRUE;
	volatile u8 IsAllZeroAgain = (u8)TRUE;
	volatile u8 IsAllOneAgain = (u8)TRUE;

	if ((Key == NULL) || (KeyLen == 0U)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** Walk the key once from the start, testing it against both weak patterns. */
	for (Index = 0U; Index < KeyLen; Index++) {
		if (Key[Index] != XKEYMANAGER_WEAK_KEY_BYTE_ALL_ZERO) {
			IsAllZero = (u8)FALSE;
		}
		if (Key[Index] != XKEYMANAGER_WEAK_KEY_BYTE_ALL_ONE) {
			IsAllOne = (u8)FALSE;
		}
	}

	/**
	 * A weak key would go on to encrypt the vault, so the key is scanned a second time from
	 * the opposite end, as a glitch past a single scan or a single test would let an all-zero
	 * or all-one key through. Each pass keeps its own volatile flags so that the second stays
	 * a separate walk of the key rather than being folded into the first.
	 */
	for (Index = KeyLen; Index > 0U; Index--) {
		if (Key[Index - 1U] != XKEYMANAGER_WEAK_KEY_BYTE_ALL_ZERO) {
			IsAllZeroAgain = (u8)FALSE;
		}
		if (Key[Index - 1U] != XKEYMANAGER_WEAK_KEY_BYTE_ALL_ONE) {
			IsAllOneAgain = (u8)FALSE;
		}
	}

	IsWeakFih = XFih_VolatileAssignU32((u32)IsAllZero | (u32)IsAllOne |
					   (u32)IsAllZeroAgain | (u32)IsAllOneAgain);
	NotWeakFih = XFih_VolatileAssignU32((u32)FALSE);

	ASSIGN_VOLATILE(Status, XASUFW_KEYMANAGER_AEKV_WEAK_KEY_MATERIAL);
	XFIH_IF_FAILIN_NEQ(IsWeakFih, NotWeakFih) {
		Status = XASUFW_KEYMANAGER_AEKV_WEAK_KEY_MATERIAL;
		XFIH_GOTO(END);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function draws key or IV material from the TRNG and rejects weak material.
 *
 * @param	Buf	Pointer to the buffer the material is written to.
 * @param	Len	Length of the material in bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if the material is drawn and is acceptable.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if the buffer is NULL or the length is zero.
 *		- XASUFW_RAND_GEN_ERROR, with the TRNG error embedded, if the material cannot be
 *		  drawn.
 *		- XASUFW_KEYMANAGER_AEKV_WEAK_KEY_MATERIAL, if the material is all-zero or all-one.
 *
 * @note	Every KEK and IV of a blob is drawn through this function, so the weak-material
 *		test is never left out of a draw. The caller names the step it failed on, because
 *		this function knows only the buffer it filled.
 *
 *************************************************************************************************/
s32 XKeyManager_GenerateStrongRandom(u8 *Buf, u32 Len)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	if ((Buf == NULL) || (Len == 0U)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	Status = XTrng_GetRandomNumbers(Buf, Len);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RAND_GEN_ERROR);
		goto END;
	}

	/** Material that is all-zero or all-one means the entropy source is not delivering. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_RejectWeakKey(Buf, Len);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function advances an IV by a whole number of steps.
 *
 * @param	BaseIv		Pointer to the IV to advance.
 * @param	Increment	Number of steps to add.
 * @param	OutIv		Output buffer for the advanced IV.
 *
 * @return
 *		- XASUFW_SUCCESS, if the IV is advanced.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if a pointer is NULL.
 *		- XASUFW_MEM_COPY_FAIL, if the base IV cannot be copied into the output.
 *		- XASUFW_KEYMANAGER_IV_GEN_ERROR, if the step loop did not run to completion, or a
 *		  non-zero step left the counter unmoved.
 *
 * @note	The IV is treated as a big-endian 128-bit counter. Both users of this - the blocks
 *		of a chunk and the pieces of the secrets of a chunk - store one IV and give every
 *		operation an IV of its own, so no key is ever used twice under the same IV. The
 *		secrets pieces of a large blob count well past a byte, because every chunk of the
 *		blob takes a range of its own, so the step is applied a byte at a time.
 *
 *************************************************************************************************/
s32 XKeyManager_AdvanceIv(const u8 *BaseIv, u32 Increment, u8 *OutIv)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	volatile u32 ByteIdx = 0U;
	u8 StepVal;

	if ((BaseIv == NULL) || (OutIv == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	Status = Xil_SMemCpy(OutIv, XKEYMANAGER_AES_IV_SIZE, BaseIv, XKEYMANAGER_AES_IV_SIZE,
			     XKEYMANAGER_AES_IV_SIZE);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/**
	 * The counter helper adds a single byte to the last byte of the buffer it is given and
	 * carries left from there, so each byte of the step is added over a buffer shortened by
	 * its own place, which lands it on the right byte of the counter.
	 */
	for (ByteIdx = 0U; ByteIdx < (u32)sizeof(Increment); ByteIdx++) {
		StepVal = (u8)((Increment >> (ByteIdx * XASUFW_BYTE_LEN_IN_BITS)) &
			       XASUFW_BYTE_MASK);
		if (StepVal != 0U) {
			Xil_IncrementBuffer(OutIv, XKEYMANAGER_AES_IV_SIZE - ByteIdx, StepVal);
		}
	}

	/** A loop cut short drops the higher step bytes, which still moves the counter. */
	if (ByteIdx != (u32)sizeof(Increment)) {
		Status = XASUFW_KEYMANAGER_IV_GEN_ERROR;
		goto END;
	}

	/** A skipped step loop would reuse the base IV under a key that already used it. */
	if (Increment != 0U) {
		/** A mismatch is the pass here, so the preset must not be XASUFW_FAILURE. */
		ASSIGN_VOLATILE(Status, XASUFW_KEYMANAGER_IV_GEN_ERROR);
		Status = Xil_SMemCmp_CT(OutIv, XKEYMANAGER_AES_IV_SIZE, BaseIv,
					XKEYMANAGER_AES_IV_SIZE, XKEYMANAGER_AES_IV_SIZE);
		if (Status != XASUFW_FAILURE) {
			Status = XASUFW_KEYMANAGER_IV_GEN_ERROR;
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function computes a SHA-384 digest over the given data.
 *
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	Data		Pointer to input data.
 * @param	DataLen		Input length in bytes.
 * @param	Hash		Output buffer for 48-byte digest.
 *
 * @return
 *		- XASUFW_SUCCESS, if digest computation is successful.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if any pointer is NULL.
 *		- XASUFW_SHA_START_FAILED, with the SHA error embedded, if the engine cannot start.
 *		- XASUFW_SHA_UPDATE_FAIL, with the SHA error embedded, if a segment cannot be fed.
 *		- XASUFW_SHA_FINISH_FAILED, with the SHA error embedded, if the digest cannot be
 *		  read.
 *
 * @note	Segments are capped at XKEYMANAGER_SHA_UPDATE_MAX_LEN only to keep each update on
 *		the blocking DMA path, not to align it to blocks. The engine pads only the last
 *		update of a digest, so either SHA engine can be used.
 *
 *************************************************************************************************/
s32 XKeyManager_Sha384Digest(XSha *ShaInstancePtr, XAsufw_Dma *DmaPtr, const u8 *Data, u32 DataLen,
		       u8 *Hash)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Offset = 0U;
	u32 SegmentLen;
	u32 EndLast;

	if ((ShaInstancePtr == NULL) || (DmaPtr == NULL) || (Data == NULL) || (Hash == NULL) ||
	    (DataLen == 0U)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	Status = XSha_Start(ShaInstancePtr, XASU_SHA_MODE_384);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA_START_FAILED);
		goto END;
	}

	/**
	 * Feed the data in segments of at most the DMA blocking size. A longer update puts the SHA
	 * engine on the non-blocking path, which reports XASUFW_CMD_IN_PROGRESS and expects the
	 * caller to resume once DMA completes, whereas AEKV needs the digest before it can proceed.
	 */
	while (Offset < DataLen) {
		SegmentLen = DataLen - Offset;
		if (SegmentLen > XKEYMANAGER_SHA_UPDATE_MAX_LEN) {
			SegmentLen = XKEYMANAGER_SHA_UPDATE_MAX_LEN;
		}
		EndLast = ((Offset + SegmentLen) == DataLen) ? (u32)XASU_TRUE : (u32)XASU_FALSE;

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(ShaInstancePtr, DmaPtr, (u64)(UINTPTR)&Data[Offset],
				     SegmentLen, EndLast);
		if (XASUFW_SUCCESS != Status) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA_UPDATE_FAIL);
			goto END;
		}

		Offset += SegmentLen;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Finish(ShaInstancePtr, DmaPtr, (u64)(UINTPTR)Hash, XKEYMANAGER_HASH_SIZE,
			     XASU_FALSE);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA_FINISH_FAILED);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function turns a failed constant time compare into a key manager status.
 *
 * @param	CompareStatus	Status returned by Xil_SMemCmp_CT.
 *
 * @return
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if the compare was rejected on its lengths.
 *		- XASUFW_KEYMANAGER_AEKV_CHUNK_HASH_FAIL, if the compared digests differ.
 *
 * @note	A rejected compare never looked at the data, so it must not be reported as a
 *		digest mismatch that an operator would read as a tampered blob.
 *
 *************************************************************************************************/
static s32 XKeyManager_MapCompareStatus(s32 CompareStatus)
{
	s32 Status = XASUFW_KEYMANAGER_AEKV_CHUNK_HASH_FAIL;

	if (CompareStatus == (s32)XST_INVALID_PARAM) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function computes SHA-384 twice and compares digest to expected hash.
 *
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	Data		Pointer to input data.
 * @param	DataLen		Input length in bytes.
 * @param	ExpectedHash	Pointer to expected 48-byte digest.
 *
 * @return
 *		- XASUFW_SUCCESS, if both digests match each other and ExpectedHash.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if any pointer is NULL or a compare was
 *			rejected on its lengths.
 *		- XASUFW_KEYMANAGER_AEKV_CHUNK_HASH_FAIL, if digests disagree.
 *
 *************************************************************************************************/
s32 XKeyManager_RedundantSha384Compare(XSha *ShaInstancePtr, XAsufw_Dma *DmaPtr, const u8 *Data,
				 u32 DataLen, const u8 *ExpectedHash)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihAekv = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 HashA[XKEYMANAGER_HASH_SIZE] = {0U};
	u8 HashB[XKEYMANAGER_HASH_SIZE] = {0U};

	if ((ShaInstancePtr == NULL) || (DmaPtr == NULL) || (Data == NULL) ||
	    (ExpectedHash == NULL) || (DataLen == 0U)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_Sha384Digest(ShaInstancePtr, DmaPtr, Data, DataLen, HashA);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_Sha384Digest(ShaInstancePtr, DmaPtr, Data, DataLen, HashB);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/**
	 * This is the authentication gate for a chunk, so both compares run under FIH. They run
	 * over the whole length whatever they find, so how far two digests agree cannot be read
	 * off the time a compare takes.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(Xil_SMemCmp_CT, XFihAekv, Status, HashA, XKEYMANAGER_HASH_SIZE, HashB,
		  XKEYMANAGER_HASH_SIZE, XKEYMANAGER_HASH_SIZE);
	if (XASUFW_SUCCESS != Status) {
		Status = XKeyManager_MapCompareStatus(Status);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(Xil_SMemCmp_CT, XFihAekv, Status, HashA, XKEYMANAGER_HASH_SIZE, ExpectedHash,
		  XKEYMANAGER_HASH_SIZE, XKEYMANAGER_HASH_SIZE);
	if (XASUFW_SUCCESS != Status) {
		Status = XKeyManager_MapCompareStatus(Status);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs a one-shot AES-256-CBC operation.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ParamsPtr	Pointer to the key, IV, buffers and length of the operation.
 * @param	OpType		XASU_AES_ENCRYPT_OPERATION or XASU_AES_DECRYPT_OPERATION.
 *
 * @return
 *		- XASUFW_SUCCESS, if the AES operation is successful.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if inputs are invalid.
 *		- XASUFW_AES_WRITE_KEY_FAILED, with the AES error embedded, if the key write fails.
 *		- Error codes from XAes_Compute, which name the failing stage of the operation.
 *
 *************************************************************************************************/
static s32 XKeyManager_AesCbcBuffer(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			      const XKeyManager_CbcParams *ParamsPtr, u8 OpType)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihAekv = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XAsu_AesKeyObject KeyObject = {0U};
	XAsu_AesParams AesParams = {0U};

	if ((DmaPtr == NULL) || (AesInstancePtr == NULL) || (ParamsPtr == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	if ((ParamsPtr->Key == NULL) || (ParamsPtr->Iv == NULL) || (ParamsPtr->Input == NULL) ||
	    (ParamsPtr->Output == NULL) || (ParamsPtr->Len == 0U) ||
	    ((ParamsPtr->Len % XASU_AES_BLOCK_SIZE_IN_BYTES) != 0U)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	KeyObject.KeySize = (u32)XASU_AES_KEY_SIZE_256_BITS;
	KeyObject.KeySrc = XASU_AES_USER_KEY_7;
	KeyObject.KeyAddress = (u64)(UINTPTR)ParamsPtr->Key;

	AesParams.EngineMode = (u8)XASU_AES_CBC_MODE;
	AesParams.OperationType = OpType;
	AesParams.KeyObjectAddr = 0U;
	AesParams.IvObj.IvAddr = (u64)(UINTPTR)ParamsPtr->Iv;
	AesParams.IvObj.IvLen = XKEYMANAGER_AES_IV_SIZE;
	AesParams.InputDataAddr = (u64)(UINTPTR)ParamsPtr->Input;
	AesParams.OutputDataAddr = (u64)(UINTPTR)ParamsPtr->Output;
	AesParams.DataLen = ParamsPtr->Len;
	AesParams.AadAddr = 0U;
	AesParams.TagAddr = 0U;

	/** The operation follows the key write, so the key is loaded for that call alone. */
	Status = XAes_WriteKey(AesInstancePtr, DmaPtr, &KeyObject);
	if (XASUFW_SUCCESS != Status) {
		/** A partial write can still leave key bytes in the user key register. */
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_WRITE_KEY_FAILED);
		goto END_CLEAR_KEY;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_Compute(AesInstancePtr, DmaPtr, &AesParams);

END_CLEAR_KEY:
	XFIH_CALL(XAes_KeyClear, XFihAekv, ClearStatus, AesInstancePtr, XASU_AES_USER_KEY_7);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts a buffer using AES-256-CBC.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ParamsPtr	Pointer to the key, IV, plaintext, ciphertext and length.
 *
 * @return
 *		- XASUFW_SUCCESS, if encryption is successful.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if inputs are invalid.
 *		- Error codes from AES CBC operation, if encryption fails.
 *
 *************************************************************************************************/
s32 XKeyManager_EncryptBufferCbc(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			   const XKeyManager_CbcParams *ParamsPtr)
{
	return XKeyManager_AesCbcBuffer(DmaPtr, AesInstancePtr, ParamsPtr,
				  (u8)XASU_AES_ENCRYPT_OPERATION);
}

/*************************************************************************************************/
/**
 * @brief	This function decrypts a buffer using AES-256-CBC.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ParamsPtr	Pointer to the key, IV, ciphertext, plaintext and length.
 *
 * @return
 *		- XASUFW_SUCCESS, if decryption is successful.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if inputs are invalid.
 *		- Error codes from AES CBC operation, if decryption fails.
 *
 *************************************************************************************************/
s32 XKeyManager_DecryptBufferCbc(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			   const XKeyManager_CbcParams *ParamsPtr)
{
	return XKeyManager_AesCbcBuffer(DmaPtr, AesInstancePtr, ParamsPtr,
				  (u8)XASU_AES_DECRYPT_OPERATION);
}

#endif /* XASU_KEYMANAGER_ENABLE */
/** @} */
