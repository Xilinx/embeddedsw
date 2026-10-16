/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xkeymanager_chunk.c
 *
 * Body of the chunked blob format.
 *
 * One chunk holds N-1 data blocks and a trailer block, and each block is encrypted independently
 * under its own KEK and an IV derived from the chunk IV. Export builds each chunk last-to-first
 * so the trailer can carry the next chunk secrets and digest. Import walks chunks first-to-last,
 * authenticating each chunk before decrypting its blocks.
 *
 * This file owns the body geometry, the per-block crypt and the whole-body walks, because how
 * many chunks a body holds, in which direction they are visited and how a block sits inside a
 * chunk are all properties of the chunk chain itself. It takes its crypto from
 * xkeymanager_crypto.c and knows nothing about any blob header, so the AEKV format in
 * xkeymanager_aekv.c is free to change without touching it.
 *
 * Flow (top to bottom):
 *
 *   Body geometry     -- plaintext size to block, chunk and body counts
 *   Block layer       -- one fixed-size block at a time, inside a chunk buffer
 *   Export one chunk  -- secrets, plaintext layout, block encrypt, digest, hand over
 *   Export body       -- walk every chunk last-to-first
 *   Import one chunk  -- authenticate, block decrypt, trailer, store, advance
 *   Import body       -- walk every chunk first-to-last
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
#include "xkeymanager_chunk.h"
#include "xkeymanager_crypto.h"
#include "xasufw_status.h"
#include "xasufw_dma.h"
#include "xasufw_util.h"
#include "xasu_generic.h"
#include "xtrng.h"
#include "xil_sutil.h"

#ifdef XASU_KEYMANAGER_ENABLE
/************************************** Type Definitions *****************************************/
typedef s32 (*XKeyManager_BlockCrypt_t)(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
					const XKeyManager_CbcParams *ParamsPtr);

/** One block of a chunk as the block walk hands it to the crypto step. */
typedef struct {
	const u8 *Kek;		/**< Block KEK, taken from the secrets buffer of the chunk. */
	const u8 *BaseIv;	/**< Chunk IV every block IV counts up from. */
	u8 *BlockPtr;		/**< The block inside the chunk buffer. */
	u32 BlockIndex;		/**< Zero-based block index within the chunk. */
	u32 BlockLen;		/**< Block length in bytes. */
	u32 BlockCapacity;	/**< Chunk buffer bytes left from the block onwards. */
	XKeyManager_UnitScratch *ScratchPtr;	/**< Per-block scratch. */
	u8 IsEncrypt;		/**< XASU_TRUE to encrypt, XASU_FALSE to decrypt. */
} XKeyManager_BlockCryptParams;

/************************************ Function Prototypes ****************************************/
static s32 XKeyManager_GetSecretsPiece(u32 PieceIdx, u32 *OffsetPtr, u32 *LenPtr);
static s32 XKeyManager_SecretsCrypt(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				    const XKeyManager_SecretsParams *ParamsPtr, u8 IsEncrypt);
/** AES-256-CBC over one block of a chunk, bounded by the largest block a chunk holds. */
static s32 XKeyManager_EncryptBlockCbc(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				       const XKeyManager_CbcParams *ParamsPtr);
static s32 XKeyManager_DecryptBlockCbc(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				       const XKeyManager_CbcParams *ParamsPtr);

/** Chunk step: one whole chunk of an export or an import. */
static s32 XKeyManager_ExportBegin(const XKeyManager_ExportParams *ParamsPtr, u32 *NumChunksPtr);
static s32 XKeyManager_ExportChunk(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, XSha *ShaInstancePtr,
			   const XKeyManager_ExportParams *ParamsPtr, u32 ChunkIndex);
static s32 XKeyManager_ExportFinish(const XKeyManager_ExportParams *ParamsPtr);
static s32 XKeyManager_ImportBegin(const XKeyManager_ImportParams *ParamsPtr, u32 *NumChunksPtr);
static s32 XKeyManager_ImportChunk(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, XSha *ShaInstancePtr,
			   const XKeyManager_ImportParams *ParamsPtr, u32 ChunkIndex,
			   u32 *PlaintextBytesPtr);
static s32 XKeyManager_ImportFinish(const XKeyManager_ImportParams *ParamsPtr);

/** Block layer. */
static s32 XKeyManager_GetBlockExtent(u32 BlockIndex, u32 *OffsetPtr, u32 *LenPtr);
static s32 XKeyManager_CryptOneBlock(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				     const XKeyManager_BlockCryptParams *ParamsPtr);
static s32 XKeyManager_CryptAllBlocks(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
					u8 *ChunkSecretsPtr, u8 *ChunkBufPtr,
					XKeyManager_UnitScratch *ScratchPtr, u8 IsEncrypt);
static s32 XKeyManager_EncryptChunk(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 *ChunkSecretsPtr,
				    u8 *ChunkBufPtr, XKeyManager_UnitScratch *ScratchPtr);
static s32 XKeyManager_DecryptChunk(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 *ChunkSecretsPtr,
				    u8 *ChunkBufPtr, XKeyManager_UnitScratch *ScratchPtr);

/** Chunk layer. */
static s32 XKeyManager_ValidateExportParams(const XKeyManager_ExportParams *ParamsPtr,
					    u32 *NumChunksPtr);
static s32 XKeyManager_ValidateImportParams(const XKeyManager_ImportParams *ParamsPtr,
					    u32 *NumChunksPtr);
static s32 XKeyManager_ChunkGenerateSecrets(u8 *SecretsBuf);
static s32 XKeyManager_ChunkAssemblePlaintext(const XKeyManager_ExportParams *ParamsPtr,
					      u32 ChunkIndex, u32 NumChunks);
static s32 XKeyManager_ChunkHandOverChain(const XKeyManager_ExportParams *ParamsPtr,
					  u32 ChunkIndex, const u8 *ChunkHashPtr);
static s32 XKeyManager_ChunkAuthenticate(XSha *ShaInstancePtr, XAsufw_Dma *DmaPtr,
					 const XKeyManager_ImportParams *ParamsPtr,
					 u32 ChunkIndex);
static s32 XKeyManager_ChunkProcessTrailer(const XKeyManager_ImportParams *ParamsPtr,
					   u32 ChunkIndex, u32 NumChunks);
static s32 XKeyManager_ChunkStoreBlocks(const XKeyManager_ImportParams *ParamsPtr, u32 ChunkIndex,
					u32 *PlaintextBytesPtr);
static s32 XKeyManager_ChunkAdvanceChain(const XKeyManager_ImportParams *ParamsPtr);

/*************************************************************************************************/
/**
 * @brief	This function encrypts one block of a chunk using AES-256-CBC. The block is
 *		encrypted under the KEK that belongs to it and the IV of its chunk, and the
 *		length is bounded by the largest block a chunk holds before any caller buffer
 *		is touched.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ParamsPtr	Pointer to the block KEK, IV, buffers and length.
 *
 * @return
 *		- XASUFW_SUCCESS, if block encryption is successful.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if any pointer is NULL or the length is zero
 *			or longer than the largest block a chunk holds.
 *		- XASUFW_KEYMANAGER_AEKV_WEAK_KEY_MATERIAL, if the block KEK is all zeros or all
 *			ones.
 *		- Error codes from AES CBC, if encryption fails.
 *
 *************************************************************************************************/
static s32 XKeyManager_EncryptBlockCbc(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				       const XKeyManager_CbcParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/**
	 * The block length decides how much of the caller buffers is read and written, so it is
	 * bounded by the largest block a chunk can hold before any of them is touched.
	 */
	if ((ParamsPtr == NULL) || (ParamsPtr->Key == NULL) || (ParamsPtr->Iv == NULL) ||
	    (ParamsPtr->Input == NULL) || (ParamsPtr->Output == NULL) ||
	    (ParamsPtr->Len == 0U) || (ParamsPtr->Len > XKEYMANAGER_TRAILER_SIZE)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** A block KEK that is all zeros or all ones means the KEK source is broken. */
	Status = XKeyManager_RejectWeakKey(ParamsPtr->Key, XKEYMANAGER_AES_KEY_SIZE);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Encrypt the block with the KEK that belongs to it and the IV of its chunk. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_EncryptBufferCbc(DmaPtr, AesInstancePtr, ParamsPtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function decrypts one block of a chunk using AES-256-CBC. The block is
 *		decrypted under the KEK that belongs to it and the IV of its chunk, and the
 *		length is bounded by the largest block a chunk holds before any caller buffer
 *		is touched.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ParamsPtr	Pointer to the block KEK, IV, buffers and length.
 *
 * @return
 *		- XASUFW_SUCCESS, if block decryption is successful.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if any pointer is NULL or the length is zero
 *			or longer than the largest block a chunk holds.
 *		- XASUFW_KEYMANAGER_AEKV_WEAK_KEY_MATERIAL, if the block KEK is all zeros or all
 *			ones.
 *		- Error codes from AES CBC, if decryption fails.
 *
 *************************************************************************************************/
static s32 XKeyManager_DecryptBlockCbc(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				       const XKeyManager_CbcParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/**
	 * The block length decides how much of the caller buffers is read and written, so it is
	 * bounded by the largest block a chunk can hold before any of them is touched.
	 */
	if ((ParamsPtr == NULL) || (ParamsPtr->Key == NULL) || (ParamsPtr->Iv == NULL) ||
	    (ParamsPtr->Input == NULL) || (ParamsPtr->Output == NULL) ||
	    (ParamsPtr->Len == 0U) || (ParamsPtr->Len > XKEYMANAGER_TRAILER_SIZE)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** A block KEK that is all zeros or all ones means the KEK source is broken. */
	Status = XKeyManager_RejectWeakKey(ParamsPtr->Key, XKEYMANAGER_AES_KEY_SIZE);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Decrypt the block with the KEK that belongs to it and the IV of its chunk. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_DecryptBufferCbc(DmaPtr, AesInstancePtr, ParamsPtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the byte offset and length of one block within a chunk.
 *
 * @param	BlockIndex	Zero-based block index within the chunk (0 ... N-1).
 * @param	OffsetPtr	Output byte offset of the block inside the chunk.
 * @param	LenPtr		Output block length in bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if the block index lies within the chunk.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if a pointer is NULL or the index is outside
 *			the chunk.
 *
 *************************************************************************************************/
static s32 XKeyManager_GetBlockExtent(u32 BlockIndex, u32 *OffsetPtr, u32 *LenPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	if ((OffsetPtr == NULL) || (LenPtr == NULL) ||
	    (BlockIndex >= XKEYMANAGER_BLOCKS_PER_CHUNK)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * The trailer is the last block of a chunk and is shorter than a data block, so it is
	 * placed from its own offset rather than by multiplying the block index.
	 */
	if (BlockIndex == XKEYMANAGER_TRAILER_BLOCK_INDEX) {
		*OffsetPtr = XKEYMANAGER_TRAILER_OFFSET;
		*LenPtr = XKEYMANAGER_TRAILER_SIZE;
	} else {
		*OffsetPtr = BlockIndex * XKEYMANAGER_BLOCK_SIZE;
		*LenPtr = XKEYMANAGER_BLOCK_SIZE;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts or decrypts one block inside a chunk buffer. Both
 *		directions share this walk and differ only in the CBC call they make. The block
 *		IV is advanced from the chunk IV, the result is produced in scratch and copied
 *		back over the block, and the scratch is wiped afterwards.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ParamsPtr	Pointer to the block the walk is on: its KEK, the chunk IV, its
 *				place in the chunk buffer, its scratch and the direction.
 *
 * @return
 *		- XASUFW_SUCCESS, if the block is processed.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if the parameter or the scratch pointer is
 *		  NULL.
 *		- XASUFW_KEYMANAGER_IV_GEN_ERROR, if the block IV cannot be advanced from the
 *		  chunk IV.
 *		- XASUFW_MEM_COPY_FAIL, if the scratch cannot be copied back over the block.
 *		- Error codes from AES CBC or scratch wipe.
 *
 *************************************************************************************************/
static s32 XKeyManager_CryptOneBlock(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				     const XKeyManager_BlockCryptParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihBlock = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XKeyManager_BlockCrypt_t CryptFn = XKeyManager_DecryptBlockCbc;
	XKeyManager_CbcParams CbcParams = {0U};

	if ((ParamsPtr == NULL) || (ParamsPtr->ScratchPtr == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** Both directions share this walk and differ only in the CBC call they make. */
	if (ParamsPtr->IsEncrypt == (u8)XASU_TRUE) {
		CryptFn = XKeyManager_EncryptBlockCbc;
	}

	/** Give the block an IV of its own, the chunk IV advanced by the block index. */
	Status = XKeyManager_AdvanceIv(ParamsPtr->BaseIv, ParamsPtr->BlockIndex,
				       ParamsPtr->ScratchPtr->UnitIv);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_IV_GEN_ERROR);
		goto END;
	}

	/**
	 * Work through the scratch rather than in place, so a block that fails midway leaves
	 * the chunk buffer as it was instead of half transformed.
	 */
	CbcParams.Key = ParamsPtr->Kek;
	CbcParams.Iv = ParamsPtr->ScratchPtr->UnitIv;
	CbcParams.Input = ParamsPtr->BlockPtr;
	CbcParams.Output = ParamsPtr->ScratchPtr->UnitBuf;
	CbcParams.Len = ParamsPtr->BlockLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = CryptFn(DmaPtr, AesInstancePtr, &CbcParams);
	if (XASUFW_SUCCESS != Status) {
		goto END_CLR;
	}

	/**
	 * The copy back is bounded by what each side really holds, the chunk bytes left from
	 * this block onwards and the whole scratch, rather than by the block length on both
	 * sides, which would leave the bounds check with nothing to compare.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(ParamsPtr->BlockPtr, ParamsPtr->BlockCapacity,
			     ParamsPtr->ScratchPtr->UnitBuf,
			     (u32)sizeof(ParamsPtr->ScratchPtr->UnitBuf), ParamsPtr->BlockLen);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END_CLR;
	}

END_CLR:
	/** The scratch held plaintext, so it is wiped before the block is left. */
	XFIH_CALL(Xil_SecureZeroize, XFihBlock, ClearStatus, ParamsPtr->ScratchPtr->UnitBuf,
		  ParamsPtr->BlockLen);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts or decrypts every block of one chunk in a random order.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ChunkSecretsPtr	Pointer to the chunk IV followed by the N block KEKs.
 * @param	ChunkBufPtr	Pointer to the chunk, transformed where it lies.
 * @param	ScratchPtr	Pointer to the scratch of one block.
 * @param	IsEncrypt	XASU_TRUE to encrypt, XASU_FALSE to decrypt.
 *
 * @return
 *		- XASUFW_SUCCESS, if every block of the chunk is processed.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if a pointer is NULL.
 *		- XASUFW_RAND_GEN_ERROR, with the TRNG error embedded, if the block order cannot be
 *		  drawn.
 *		- XASUFW_KEYMANAGER_AEKV_ZEROIZE_FAILED, if a block KEK or scratch wipe fails.
 *		- Error codes from AES CBC, if a block cannot be encrypted or decrypted.
 *
 * @note	A decrypt wipes each block KEK as soon as it has been used. An encrypt keeps
 *		them, because the export still has to hand the whole secrets buffer to the chunk
 *		that precedes this one.
 *
 *************************************************************************************************/
static s32 XKeyManager_CryptAllBlocks(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
					u8 *ChunkSecretsPtr, u8 *ChunkBufPtr,
					XKeyManager_UnitScratch *ScratchPtr, u8 IsEncrypt)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihOrder = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 OrderIdx;
	u32 BlockIdx;
	u32 Index;
	u32 SeenMask;
	u32 BlockOffset = 0U;
	u32 BlockLen = 0U;
	u8 *KekPtr = NULL;
	const u8 *IvPtr = NULL;
	XKeyManager_BlockCryptParams BlockParams = {0U};

	if ((ChunkSecretsPtr == NULL) || (ChunkBufPtr == NULL) || (ScratchPtr == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** The chunk IV heads the secrets buffer and the block KEKs follow it. */
	IvPtr = ChunkSecretsPtr + XKEYMANAGER_SECRETS_IV_OFFSET;

	/**
	 * Draw a fresh order for the blocks of this chunk. Every block has a key and an IV of
	 * its own, so the order does not affect the result, and taking it from the TRNG on each
	 * call keeps the sequence of AES operations from tracking the layout of the chunk.
	 */
	for (Index = 0U; Index < XKEYMANAGER_BLOCKS_PER_CHUNK; Index++) {
		ScratchPtr->UnitOrder[Index] = (u8)Index;
	}

	Status = XTrng_FisherYatesShuffle(ScratchPtr->UnitOrder, XKEYMANAGER_BLOCKS_PER_CHUNK);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RAND_GEN_ERROR);
		goto END;
	}

	/**
	 * The walk below trusts the order to name every block exactly once. A fault in the
	 * shuffle that repeated one index and dropped another would encrypt one block twice and
	 * leave the other in the clear, so the order is confirmed to be a permutation before any
	 * block is touched. One bit per block index is set and the whole set is compared.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	SeenMask = 0U;
	for (Index = 0U; Index < XKEYMANAGER_BLOCKS_PER_CHUNK; Index++) {
		BlockIdx = (u32)ScratchPtr->UnitOrder[Index];
		if (BlockIdx >= XKEYMANAGER_BLOCKS_PER_CHUNK) {
			Status = XASUFW_KEYMANAGER_AEKV_INVALID_BLOCK_ORDER;
			goto END;
		}
		SeenMask |= ((u32)1U << BlockIdx);
	}

	if (SeenMask != XKEYMANAGER_BLOCK_ORDER_FULL_MASK) {
		Status = XASUFW_KEYMANAGER_AEKV_INVALID_BLOCK_ORDER;
		goto END;
	}

	for (OrderIdx = 0U; OrderIdx < XKEYMANAGER_BLOCKS_PER_CHUNK; OrderIdx++) {
		BlockIdx = (u32)ScratchPtr->UnitOrder[OrderIdx];

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_GetBlockExtent(BlockIdx, &BlockOffset, &BlockLen);
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}

		/** Each block takes the KEK stored at its own index in the secrets buffer. */
		KekPtr = ChunkSecretsPtr + XKEYMANAGER_SECRETS_KEK_OFFSET(BlockIdx);

		BlockParams.Kek = KekPtr;
		BlockParams.BaseIv = IvPtr;
		BlockParams.BlockPtr = ChunkBufPtr + BlockOffset;
		BlockParams.BlockIndex = BlockIdx;
		BlockParams.BlockLen = BlockLen;
		BlockParams.BlockCapacity = XKEYMANAGER_CHUNK_SIZE - BlockOffset;
		BlockParams.ScratchPtr = ScratchPtr;
		BlockParams.IsEncrypt = IsEncrypt;

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_CryptOneBlock(DmaPtr, AesInstancePtr, &BlockParams);
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}

		/**
		 * A decrypt has no further use for the KEK once its block is out, so it goes
		 * immediately. An encrypt keeps them, because the whole secrets buffer still has
		 * to be handed to the chunk that precedes this one.
		 */
		if (IsEncrypt != (u8)XASU_TRUE) {
			ASSIGN_VOLATILE(ClearStatus, XASUFW_FAILURE);
			XFIH_CALL(Xil_SecureZeroize, XFihOrder, ClearStatus, KekPtr,
				  XKEYMANAGER_AES_KEY_SIZE);
			Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
			if (XASUFW_SUCCESS != Status) {
				goto END;
			}
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts every block of one chunk in place. The blocks are walked
 *		in a random order, each under its own KEK and a block IV advanced from the chunk
 *		IV, so the chunk buffer holds ciphertext when this returns.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ChunkSecretsPtr	Pointer to the chunk IV followed by the N block KEKs.
 * @param	ChunkBufPtr	Pointer to the chunk plaintext, replaced by the ciphertext.
 * @param	ScratchPtr	Pointer to the scratch of one block.
 *
 * @return
 *		- XASUFW_SUCCESS, if every block of the chunk is encrypted.
 *
 * @note	The chunk buffer holds the whole chunk laid out as plaintext, the trailer
 *		included, so the caller places the trailer before it calls this.
 *
 *************************************************************************************************/
static s32 XKeyManager_EncryptChunk(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 *ChunkSecretsPtr,
				    u8 *ChunkBufPtr, XKeyManager_UnitScratch *ScratchPtr)
{
	return XKeyManager_CryptAllBlocks(DmaPtr, AesInstancePtr, ChunkSecretsPtr, ChunkBufPtr,
					    ScratchPtr, (u8)XASU_TRUE);
}

/*************************************************************************************************/
/**
 * @brief	This function decrypts every block of one chunk in place and wipes each KEK.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ChunkSecretsPtr	Pointer to the chunk IV followed by the N block KEKs.
 * @param	ChunkBufPtr	Pointer to the chunk ciphertext, replaced by the plaintext.
 * @param	ScratchPtr	Pointer to the scratch of one block.
 *
 * @return
 *		- XASUFW_SUCCESS, if every block of the chunk is decrypted.
 *
 * @note	The caller authenticates the chunk before it calls this.
 *
 *************************************************************************************************/
static s32 XKeyManager_DecryptChunk(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 *ChunkSecretsPtr,
				    u8 *ChunkBufPtr, XKeyManager_UnitScratch *ScratchPtr)
{
	return XKeyManager_CryptAllBlocks(DmaPtr, AesInstancePtr, ChunkSecretsPtr, ChunkBufPtr,
					    ScratchPtr, (u8)XASU_FALSE);
}

/*************************************************************************************************/
/**
 * @brief	This function generates the chunk IV and the per-block KEKs from the TRNG.
 *
 * @param	SecretsBuf	Output buffer for the chunk IV followed by the N block KEKs.
 *
 * @return
 *		- XASUFW_SUCCESS, if secret generation is successful.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if SecretsBuf is NULL.
 *		- XASUFW_KEYMANAGER_IV_GEN_ERROR, with the draw error embedded, if the chunk IV
 *		  cannot be drawn or is all zeros or all ones.
 *		- XASUFW_KEYMANAGER_KV_KEK_GEN_FAIL, with the draw error embedded, if a block KEK
 *		  cannot be drawn or is all zeros or all ones.
 *
 *************************************************************************************************/
static s32 XKeyManager_ChunkGenerateSecrets(u8 *SecretsBuf)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 BlockIdx;
	u8 *KekPtr = NULL;
	u8 *IvPtr = NULL;

	if (SecretsBuf == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Draw the chunk IV that every block IV in this chunk counts up from. A degenerate chunk
	 * IV would repeat across the blocks that count up from it, so the draw screens it.
	 */
	IvPtr = SecretsBuf + XKEYMANAGER_SECRETS_IV_OFFSET;
	Status = XKeyManager_GenerateStrongRandom(IvPtr, XKEYMANAGER_CHUNK_IV_SIZE);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_IV_GEN_ERROR);
		goto END;
	}

	/** Draw one KEK per block of the chunk and reject a weak one. */
	for (BlockIdx = 0U; BlockIdx < XKEYMANAGER_BLOCKS_PER_CHUNK; BlockIdx++) {
		KekPtr = SecretsBuf + XKEYMANAGER_SECRETS_KEK_OFFSET(BlockIdx);
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_GenerateStrongRandom(KekPtr, XKEYMANAGER_AES_KEY_SIZE);
		if (XASUFW_SUCCESS != Status) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_KEYMANAGER_KV_KEK_GEN_FAIL);
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function lays out the plaintext of one chunk: data blocks and trailer.
 *
 * @param	ParamsPtr	Pointer to the export parameters.
 * @param	ChunkIndex	Zero-based chunk index.
 * @param	NumChunks	Number of chunks the body holds.
 *
 * @return
 *		- XASUFW_SUCCESS, if the chunk plaintext is laid out.
 *		- XASUFW_ZEROIZE_MEMSET_FAIL, if the chunk buffer cannot be cleared.
 *		- XASUFW_MEM_COPY_FAIL, if the successor digest cannot be placed in the trailer.
 *		- Error codes from the caller source or the caller secrets wrap.
 *
 *************************************************************************************************/
static s32 XKeyManager_ChunkAssemblePlaintext(const XKeyManager_ExportParams *ParamsPtr,
					      u32 ChunkIndex, u32 NumChunks)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XKeyManager_ExportWorkBuf *WorkPtr = ParamsPtr->WorkPtr;
	u8 *TrailerPtr = WorkPtr->ChunkBuf + XKEYMANAGER_TRAILER_OFFSET;
	u32 SlotIdx;
	u32 ReadOffset;
	u32 ReadLen;

	Status = Xil_SMemSet(WorkPtr->ChunkBuf, XKEYMANAGER_CHUNK_SIZE, 0U,
			     XKEYMANAGER_CHUNK_SIZE);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	for (SlotIdx = 0U; SlotIdx < XKEYMANAGER_DATA_BLOCKS_PER_CHUNK; SlotIdx++) {
		ReadOffset = ((ChunkIndex * XKEYMANAGER_DATA_BLOCKS_PER_CHUNK) + SlotIdx) *
			     XKEYMANAGER_BLOCK_SIZE;
		if (ReadOffset >= ParamsPtr->PlaintextSize) {
			break;
		}

		ReadLen = ParamsPtr->PlaintextSize - ReadOffset;
		if (ReadLen > XKEYMANAGER_BLOCK_SIZE) {
			ReadLen = XKEYMANAGER_BLOCK_SIZE;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = ParamsPtr->SourceFn(ReadOffset, WorkPtr->ChunkBuf +
					     (SlotIdx * XKEYMANAGER_BLOCK_SIZE), ReadLen);
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}
	}

	if (ChunkIndex < (NumChunks - 1U)) {
		/**
		 * The secrets go into the trailer wrapped, so they are protected by the caller
		 * keys as well as by the trailer KEK this chunk is about to be encrypted with.
		 * They describe the chunk that follows this one, which is what the wrap is told.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = ParamsPtr->SecretsWrapFn(ParamsPtr->SecretsArgPtr, ChunkIndex + 1U,
						  WorkPtr->NextChunkSecrets,
						  TrailerPtr +
						  XKEYMANAGER_TRAILER_SECRETS_OFFSET);
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy(TrailerPtr + XKEYMANAGER_TRAILER_HASH_OFFSET,
				     XKEYMANAGER_TRAILER_SIZE - XKEYMANAGER_TRAILER_HASH_OFFSET,
				     WorkPtr->NextChunkHash, XKEYMANAGER_HASH_SIZE,
				     XKEYMANAGER_HASH_SIZE);
		if (XASUFW_SUCCESS != Status) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function hands the secrets and digest of a built chunk to the chain holder.
 *
 * @param	ParamsPtr	Pointer to the export parameters.
 * @param	ChunkIndex	Zero-based chunk index.
 * @param	ChunkHashPtr	Pointer to the digest of the chunk just built.
 *
 * @return
 *		- XASUFW_SUCCESS, if the secrets and digest are handed over.
 *		- XASUFW_MEM_COPY_FAIL, if the secrets or the digest cannot be copied out.
 *
 *************************************************************************************************/
static s32 XKeyManager_ChunkHandOverChain(const XKeyManager_ExportParams *ParamsPtr,
					  u32 ChunkIndex, const u8 *ChunkHashPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XKeyManager_ExportWorkBuf *WorkPtr = ParamsPtr->WorkPtr;
	u8 *SecretsPtr = WorkPtr->NextChunkSecrets;
	u8 *HashPtr = WorkPtr->NextChunkHash;

	/**
	 * The secrets and digest of a chunk travel in the trailer of the chunk before it, which
	 * the next pass of the walk builds. Chunk 0 has no such predecessor, so its pair goes to
	 * the caller instead and ends up in the blob header.
	 */
	if (ChunkIndex == 0U) {
		SecretsPtr = ParamsPtr->Chunk0SecretsPtr;
		HashPtr = ParamsPtr->Chunk0HashPtr;
	}

	Status = Xil_SMemCpy(SecretsPtr, XKEYMANAGER_CHUNK_SECRETS_SIZE, WorkPtr->ChunkSecrets,
			     XKEYMANAGER_CHUNK_SECRETS_SIZE, XKEYMANAGER_CHUNK_SECRETS_SIZE);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(HashPtr, XKEYMANAGER_HASH_SIZE, ChunkHashPtr, XKEYMANAGER_HASH_SIZE,
			     XKEYMANAGER_HASH_SIZE);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_MEM_COPY_FAIL;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates export parameters and reports the chunk count. Every
 *		callback and buffer the walk needs must be present, and the body size must be
 *		exactly the whole number of chunks this plaintext needs, since a larger body
 *		would leave chunks the source cannot fill and a smaller one would drop
 *		plaintext.
 *
 * @param	ParamsPtr	Pointer to the export parameters.
 * @param	NumChunksPtr	Output chunk count of the body.
 *
 * @return
 *		- XASUFW_SUCCESS, if the parameters are usable.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if a pointer is NULL, sizes disagree, or the
 *		  chunk count this plaintext needs cannot be sized in a word.
 *
 *************************************************************************************************/
static s32 XKeyManager_ValidateExportParams(const XKeyManager_ExportParams *ParamsPtr,
					    u32 *NumChunksPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	if ((ParamsPtr == NULL) || (NumChunksPtr == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** Every callback and buffer the walk needs must be present, and a body is whole chunks. */
	if ((ParamsPtr->WorkPtr == NULL) || (ParamsPtr->SourceFn == NULL) ||
	    (ParamsPtr->SecretsWrapFn == NULL) ||
	    (ParamsPtr->Chunk0SecretsPtr == NULL) || (ParamsPtr->Chunk0HashPtr == NULL) ||
	    (ParamsPtr->BodySize == 0U) || (ParamsPtr->PlaintextSize == 0U) ||
	    ((ParamsPtr->BodySize % XKEYMANAGER_CHUNK_SIZE) != 0U)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** A chunk count past this wraps the body size below, which would then compare equal. */
	if (XKeyManager_CalcNumChunks(XKeyManager_CalcNumBlocks(ParamsPtr->PlaintextSize)) >
	    (XASUFW_MAX_32BIT_ADDRESS / XKEYMANAGER_CHUNK_SIZE)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * The body must be exactly the size this plaintext needs. A larger one would leave
	 * chunks the source cannot fill, and a smaller one would drop plaintext silently.
	 */
	if (ParamsPtr->BodySize != XKeyManager_CalcBodySize(ParamsPtr->PlaintextSize)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	*NumChunksPtr = ParamsPtr->BodySize / XKEYMANAGER_CHUNK_SIZE;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function opens a chunk-wise export and reports the chunk count to walk.
 *
 * @param	ParamsPtr	Pointer to the export parameters.
 * @param	NumChunksPtr	Output chunk count of the body.
 *
 * @return
 *		- XASUFW_SUCCESS, if the parameters are usable and working buffers are clean.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if a pointer is NULL or sizes disagree.
 *		- XASUFW_KEYMANAGER_AEKV_ZEROIZE_FAILED, if the working buffers do not wipe.
 *
 *************************************************************************************************/
static s32 XKeyManager_ExportBegin(const XKeyManager_ExportParams *ParamsPtr, u32 *NumChunksPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihExport = XFih_VolatileAssignXfihVar(XFIH_FAILURE);

	/** Validate input parameters and learn how many chunks the body holds. */
	Status = XKeyManager_ValidateExportParams(ParamsPtr, NumChunksPtr);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Start from clean buffers, so no chunk can inherit the secrets of an earlier export. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(Xil_SecureZeroize, XFihExport, Status, (u8 *)ParamsPtr->WorkPtr,
		  (u32)sizeof(XKeyManager_ExportWorkBuf));
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_KEYMANAGER_AEKV_ZEROIZE_FAILED;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function builds one export chunk. It draws the IV and block KEKs the chunk
 *		is encrypted under, fills the data blocks from the source callback and the
 *		trailer from the chunk built before, encrypts the chunk in place, digests the
 *		ciphertext for its predecessor to authenticate it by, and writes it to the body.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	ParamsPtr	Pointer to the export parameters.
 * @param	ChunkIndex	Zero-based chunk index.
 *
 * @return
 *		- XASUFW_SUCCESS, if the chunk is built and written.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if an instance pointer is NULL or the chunk
 *		  index is past the last chunk of this body.
 *		- XASUFW_DMA_COPY_FAIL, if the chunk cannot be written to the body.
 *
 * @note	Export walks chunks last-to-first, so each trailer can carry the secrets and
 *		digest of the chunk that follows it.
 *
 *************************************************************************************************/
static s32 XKeyManager_ExportChunk(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, XSha *ShaInstancePtr,
			   const XKeyManager_ExportParams *ParamsPtr, u32 ChunkIndex)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihChunk = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XKeyManager_ExportWorkBuf *WorkPtr = NULL;
	u8 ChunkHash[XKEYMANAGER_HASH_SIZE] = {0U};
	u32 NumChunks = 0U;

	if ((DmaPtr == NULL) || (AesInstancePtr == NULL) || (ShaInstancePtr == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	Status = XKeyManager_ValidateExportParams(ParamsPtr, &NumChunks);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	if (ChunkIndex >= NumChunks) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	WorkPtr = ParamsPtr->WorkPtr;

	/** Draw the IV and the block KEKs this chunk is encrypted under. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ChunkGenerateSecrets(WorkPtr->ChunkSecrets);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Fill the data blocks from the source and the trailer from the chunk built before. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ChunkAssemblePlaintext(ParamsPtr, ChunkIndex, NumChunks);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Encrypt the chunk in place, block by block. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_EncryptChunk(DmaPtr, AesInstancePtr, WorkPtr->ChunkSecrets,
					 WorkPtr->ChunkBuf, &WorkPtr->Scratch);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Digest the ciphertext, which is what the previous chunk will authenticate it by. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_Sha384Digest(ShaInstancePtr, DmaPtr, WorkPtr->ChunkBuf,
				    XKEYMANAGER_CHUNK_SIZE, ChunkHash);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Hand the secrets and the digest to whoever carries them, the caller or the next pass. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ChunkHandOverChain(ParamsPtr, ChunkIndex, ChunkHash);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Write the finished chunk to its place in the body. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)WorkPtr->ChunkBuf,
			       ParamsPtr->BodyAddr + ((u64)ChunkIndex * XKEYMANAGER_CHUNK_SIZE),
			       XKEYMANAGER_CHUNK_SIZE, 0U);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_DMA_COPY_FAIL;
	}

END:
	/** The secrets of this chunk are already handed on, so the working copies go here. */
	if (WorkPtr != NULL) {
		XFIH_CALL(Xil_SecureZeroize, XFihChunk, ClearStatus, WorkPtr->ChunkSecrets,
			  XKEYMANAGER_CHUNK_SECRETS_SIZE);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
		XFIH_CALL(Xil_SecureZeroize, XFihChunk, ClearStatus, (u8 *)&WorkPtr->Scratch,
			  (u32)sizeof(XKeyManager_UnitScratch));
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function closes a chunk-wise export by wiping the working buffers. The
 *		chunk-0 secrets and digest already left the buffers with the caller, so nothing
 *		held here survives the export.
 *
 * @param	ParamsPtr	Pointer to the export parameters.
 *
 * @return
 *		- XASUFW_SUCCESS, if the working buffers are wiped.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if a pointer is NULL.
 *		- XASUFW_KEYMANAGER_AEKV_ZEROIZE_FAILED, if the working buffers do not wipe.
 *
 *************************************************************************************************/
static s32 XKeyManager_ExportFinish(const XKeyManager_ExportParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihExport = XFih_VolatileAssignXfihVar(XFIH_FAILURE);

	if ((ParamsPtr == NULL) || (ParamsPtr->WorkPtr == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** The chunk-0 secrets left the buffers with the caller, so nothing here is kept. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(Xil_SecureZeroize, XFihExport, Status, (u8 *)ParamsPtr->WorkPtr,
		  (u32)sizeof(XKeyManager_ExportWorkBuf));
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_KEYMANAGER_AEKV_ZEROIZE_FAILED;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function exports a chunked blob body by walking every chunk last-to-first.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	ParamsPtr	Pointer to the export parameters.
 *
 * @return
 *		- XASUFW_SUCCESS, if the whole body is built and written.
 *
 * @note	Chunk-0 secrets and digest are left with the caller for the blob header.
 *
 *************************************************************************************************/
s32 XKeyManager_ChunkBlobExport(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, XSha *ShaInstancePtr,
		      const XKeyManager_ExportParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 NumChunks = 0U;
	u32 ChunkIdx;

	Status = XKeyManager_ExportBegin(ParamsPtr, &NumChunks);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/**
	 * Walk the chunks backwards. A trailer carries the secrets and the digest of the chunk
	 * that follows it, so a chunk can only be built once its successor exists.
	 */
	for (ChunkIdx = NumChunks; ChunkIdx > 0U; ChunkIdx--) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_ExportChunk(DmaPtr, AesInstancePtr, ShaInstancePtr, ParamsPtr,
						ChunkIdx - 1U);
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	/** The working buffers are wiped whether the body was finished or abandoned. */
	if ((ParamsPtr != NULL) && (ParamsPtr->WorkPtr != NULL)) {
		ClearStatus = XKeyManager_ExportFinish(ParamsPtr);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates import parameters and reports the chunk count. Every
 *		callback and buffer the walk needs must be present, and the body size must be
 *		exactly the whole number of chunks the declared plaintext needs, since a larger
 *		declared size would let padding blocks reach the sink and be counted as
 *		restored plaintext.
 *
 * @param	ParamsPtr	Pointer to the import parameters.
 * @param	NumChunksPtr	Output chunk count of the body.
 *
 * @return
 *		- XASUFW_SUCCESS, if the parameters are usable.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if a pointer or callback is NULL, a size is
 *		  zero, the body size is not a whole number of chunks, or the chunk count the
 *		  declared plaintext needs cannot be sized in a word.
 *
 *************************************************************************************************/
static s32 XKeyManager_ValidateImportParams(const XKeyManager_ImportParams *ParamsPtr,
					    u32 *NumChunksPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	if ((ParamsPtr == NULL) || (NumChunksPtr == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** Every callback and buffer the walk needs must be present, and a body is whole chunks. */
	if ((ParamsPtr->WorkPtr == NULL) || (ParamsPtr->SinkFn == NULL) ||
	    (ParamsPtr->SecretsUnwrapFn == NULL) ||
	    (ParamsPtr->BodySize == 0U) || (ParamsPtr->MaxPlaintextSize == 0U) ||
	    ((ParamsPtr->BodySize % XKEYMANAGER_CHUNK_SIZE) != 0U)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** A chunk count past this wraps the body size below, which would then compare equal. */
	if (XKeyManager_CalcNumChunks(XKeyManager_CalcNumBlocks(ParamsPtr->MaxPlaintextSize)) >
	    (XASUFW_MAX_32BIT_ADDRESS / XKEYMANAGER_CHUNK_SIZE)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * The body has to be exactly the size the declared plaintext needs, as it is on the
	 * export side. A plaintext size larger than the body really carries would let padding
	 * blocks reach the sink and be counted as restored plaintext.
	 */
	if (ParamsPtr->BodySize != XKeyManager_CalcBodySize(ParamsPtr->MaxPlaintextSize)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	*NumChunksPtr = ParamsPtr->BodySize / XKEYMANAGER_CHUNK_SIZE;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the byte offset and length of one piece of the chunk
 *		secrets.
 *
 * @param	PieceIdx	Zero-based piece index.
 * @param	OffsetPtr	Output byte offset of the piece inside the secrets buffer.
 * @param	LenPtr		Output piece length in bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if the piece index lies within the secrets buffer.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if a pointer is NULL or the index is outside
 *			the secrets buffer.
 *
 * @note	Piece 0 is the chunk IV. Every other piece is a group of block KEKs.
 *
 *************************************************************************************************/
static s32 XKeyManager_GetSecretsPiece(u32 PieceIdx, u32 *OffsetPtr, u32 *LenPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	if ((OffsetPtr == NULL) || (LenPtr == NULL) ||
	    (PieceIdx >= XKEYMANAGER_SECRETS_PIECE_COUNT)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Piece 0 is the chunk IV at the head of the buffer. The KEKs that follow it are taken
	 * in groups of a fixed size, so a piece past the first starts where the IV ends.
	 */
	if (PieceIdx == 0U) {
		*OffsetPtr = XKEYMANAGER_SECRETS_IV_OFFSET;
		*LenPtr = XKEYMANAGER_CHUNK_IV_SIZE;
	} else {
		*OffsetPtr = XKEYMANAGER_CHUNK_IV_SIZE +
			     ((PieceIdx - 1U) * XKEYMANAGER_SECRETS_GROUP_SIZE);
		*LenPtr = XKEYMANAGER_SECRETS_GROUP_SIZE;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts or decrypts the secrets of one chunk piece by piece.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ParamsPtr	Pointer to the chunk secrets parameters.
 * @param	IsEncrypt	XASU_TRUE to encrypt, XASU_FALSE to decrypt.
 *
 * @return
 *		- XASUFW_SUCCESS, if every piece is processed.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if a pointer is NULL or the chunk index is past
 *		  the range of piece IVs a word can address.
 *		- XASUFW_KEYMANAGER_IV_GEN_ERROR, if the IV of a piece cannot be advanced from the
 *		  base IV.
 *		- XASUFW_RAND_GEN_ERROR, with the TRNG error embedded, if the piece order cannot be
 *		  drawn.
 *		- Error codes from AES CBC, if a piece cannot be processed.
 *
 * @note	Each piece has a derived key and an IV of its own, so the pieces are independent
 *		and are taken in a random order drawn fresh on every call. The IV of a piece is
 *		the base IV advanced past every piece the earlier chunks of the same blob already
 *		consumed, so one derived key never encrypts twice under the same IV however many
 *		chunks the blob holds.
 *
 *************************************************************************************************/
static s32 XKeyManager_SecretsCrypt(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				    const XKeyManager_SecretsParams *ParamsPtr, u8 IsEncrypt)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihSecrets = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XKeyManager_CbcParams CbcParams = {0U};
	u8 PieceOrder[XKEYMANAGER_SECRETS_PIECE_COUNT];
	u8 PieceIv[XKEYMANAGER_AES_IV_SIZE] = {0U};
	u32 OrderIdx;
	u32 PieceIdx;
	u32 Index;
	u32 IvIndex;
	u32 Offset = 0U;
	u32 Len = 0U;

	if ((DmaPtr == NULL) || (AesInstancePtr == NULL) || (ParamsPtr == NULL) ||
	    (ParamsPtr->KeyMaterial == NULL) || (ParamsPtr->BaseIv == NULL) ||
	    (ParamsPtr->Input == NULL) || (ParamsPtr->Output == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** A chunk index past this wraps the piece IV index below, which would reuse an IV. */
	if (ParamsPtr->ChunkIndex >
	    ((XASUFW_MAX_32BIT_ADDRESS - (XKEYMANAGER_SECRETS_PIECE_COUNT - 1U)) /
	     XKEYMANAGER_SECRETS_PIECE_COUNT)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Draw a fresh order for the pieces. Each piece has a key and an IV of its own, so the
	 * order does not affect the result and only keeps the sequence of AES operations from
	 * tracking the layout of the secrets buffer.
	 */
	for (Index = 0U; Index < XKEYMANAGER_SECRETS_PIECE_COUNT; Index++) {
		PieceOrder[Index] = (u8)Index;
	}

	Status = XTrng_FisherYatesShuffle(PieceOrder, XKEYMANAGER_SECRETS_PIECE_COUNT);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RAND_GEN_ERROR);
		goto END;
	}

	for (OrderIdx = 0U; OrderIdx < XKEYMANAGER_SECRETS_PIECE_COUNT; OrderIdx++) {
		PieceIdx = (u32)PieceOrder[OrderIdx];

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_GetSecretsPiece(PieceIdx, &Offset, &Len);
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}

		/**
		 * Every chunk of a blob takes a range of the base IV of its own, so the piece is
		 * given the base IV advanced past every piece the earlier chunks consumed. One
		 * derived key therefore never encrypts twice under the same IV, however many
		 * chunks the blob holds.
		 */
		IvIndex = (ParamsPtr->ChunkIndex * XKEYMANAGER_SECRETS_PIECE_COUNT) + PieceIdx;

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_AdvanceIv(ParamsPtr->BaseIv, IvIndex, PieceIv);
		if (XASUFW_SUCCESS != Status) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_IV_GEN_ERROR);
			goto END;
		}

		/** A piece is wrapped under the derived key that belongs to its index. */
		CbcParams.Key = &ParamsPtr->KeyMaterial[PieceIdx * XKEYMANAGER_AES_KEY_SIZE];
		CbcParams.Iv = PieceIv;
		CbcParams.Input = &ParamsPtr->Input[Offset];
		CbcParams.Output = &ParamsPtr->Output[Offset];
		CbcParams.Len = Len;

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		if (IsEncrypt == (u8)XASU_TRUE) {
			Status = XKeyManager_EncryptBufferCbc(DmaPtr, AesInstancePtr, &CbcParams);
		} else {
			Status = XKeyManager_DecryptBufferCbc(DmaPtr, AesInstancePtr, &CbcParams);
		}
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	/** The last piece IV is still on the stack, so it is wiped on every path. */
	ASSIGN_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFIH_CALL(Xil_SecureZeroize, XFihSecrets, ClearStatus, PieceIv, XKEYMANAGER_AES_IV_SIZE);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function wraps the secrets of one chunk, so a chunk trailer can carry the
 *		IV and block KEKs of the chunk that follows it. The secrets are encrypted piece
 *		by piece under the derived key material and a per-piece IV counted up from the
 *		base IV of the export.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ParamsPtr	Pointer to the chunk secrets parameters.
 *
 * @return
 *		- XASUFW_SUCCESS, if the secrets are wrapped.
 *		- Error codes from the piece walk, if a piece cannot be encrypted.
 *
 *************************************************************************************************/
s32 XKeyManager_WrapSecrets(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				    const XKeyManager_SecretsParams *ParamsPtr)
{
	return XKeyManager_SecretsCrypt(DmaPtr, AesInstancePtr, ParamsPtr, (u8)XASU_TRUE);
}

/*************************************************************************************************/
/**
 * @brief	This function recovers the secrets of one chunk from the wrapped form a trailer
 *		carries. It is the import side counterpart of the wrap and walks the same pieces
 *		under the same key material and per-piece IVs.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ParamsPtr	Pointer to the chunk secrets parameters.
 *
 * @return
 *		- XASUFW_SUCCESS, if the secrets are recovered.
 *		- Error codes from the piece walk, if a piece cannot be decrypted.
 *
 *************************************************************************************************/
s32 XKeyManager_UnwrapSecrets(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				      const XKeyManager_SecretsParams *ParamsPtr)
{
	return XKeyManager_SecretsCrypt(DmaPtr, AesInstancePtr, ParamsPtr, (u8)XASU_FALSE);
}

/*************************************************************************************************/
/**
 * @brief	This function authenticates one chunk against the digest the chain expects. The
 *		chunk is copied out of the blob first, so it cannot change under the digest that
 *		is taken next, and nothing of it is decrypted before the digests match. For
 *		chunk 0 the expected digest is the one the signed header carries.
 *
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ParamsPtr	Pointer to the import parameters.
 * @param	ChunkIndex	Zero-based chunk index.
 *
 * @return
 *		- XASUFW_SUCCESS, if chunk authentication succeeds.
 *		- XASUFW_DMA_COPY_FAIL, if the chunk copy from the blob fails.
 *		- XASUFW_KEYMANAGER_AEKV_CHUNK_HASH_FAIL, if hash compare fails.
 *
 *************************************************************************************************/
static s32 XKeyManager_ChunkAuthenticate(XSha *ShaInstancePtr, XAsufw_Dma *DmaPtr,
					 const XKeyManager_ImportParams *ParamsPtr,
					 u32 ChunkIndex)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XKeyManager_WorkBuf *WorkPtr = ParamsPtr->WorkPtr;

	/** Take a local copy, so the blob cannot change under the digest that is taken next. */
	Status = XAsufw_DmaXfr(DmaPtr,
			       ParamsPtr->BodyAddr + ((u64)ChunkIndex * XKEYMANAGER_CHUNK_SIZE),
			       (u64)(UINTPTR)WorkPtr->ChunkBuf, XKEYMANAGER_CHUNK_SIZE, 0U);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/**
	 * Nothing of this chunk is decrypted before it matches the digest the chain expects,
	 * which for chunk 0 is the digest the signed header carries.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_RedundantSha384Compare(ShaInstancePtr, DmaPtr, WorkPtr->ChunkBuf,
					      XKEYMANAGER_CHUNK_SIZE, WorkPtr->ExpectedHash);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_KEYMANAGER_AEKV_CHUNK_HASH_FAIL;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads the trailer of a decrypted chunk or checks the end marker.
 *
 * @param	ParamsPtr	Pointer to the import parameters.
 * @param	ChunkIndex	Zero-based chunk index.
 * @param	NumChunks	Number of chunks the body holds.
 *
 * @return
 *		- XASUFW_SUCCESS, if the trailer is consumed or is a valid end marker.
 *		- XASUFW_KEYMANAGER_AEKV_FINAL_TAIL_INVALID, if the final trailer is invalid.
 *		- XASUFW_MEM_COPY_FAIL, if the successor digest cannot be taken from the trailer.
 *		- Error codes from the caller secrets unwrap.
 *
 *************************************************************************************************/
static s32 XKeyManager_ChunkProcessTrailer(const XKeyManager_ImportParams *ParamsPtr,
					   u32 ChunkIndex, u32 NumChunks)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XKeyManager_WorkBuf *WorkPtr = ParamsPtr->WorkPtr;
	u8 *TrailerPtr = WorkPtr->ChunkBuf + XKEYMANAGER_TRAILER_OFFSET;

	if (ChunkIndex < (NumChunks - 1U)) {
		/**
		 * The trailer decryption above removed only the trailer KEK layer, so the secrets
		 * are still wrapped by the caller keys and are recovered here. They describe the
		 * chunk that follows this one, which is what the unwrap is told.
		 */
		Status = ParamsPtr->SecretsUnwrapFn(ParamsPtr->SecretsArgPtr, ChunkIndex + 1U,
						    TrailerPtr +
						    XKEYMANAGER_TRAILER_SECRETS_OFFSET,
						    WorkPtr->NextChunkSecrets);
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy(WorkPtr->NextChunkHash, XKEYMANAGER_HASH_SIZE,
				     TrailerPtr + XKEYMANAGER_TRAILER_HASH_OFFSET,
				     XKEYMANAGER_HASH_SIZE, XKEYMANAGER_HASH_SIZE);
		if (XASUFW_SUCCESS != Status) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}
	} else {
		/**
		 * An all-zero trailer is what ends the chunk chain, so it is tested twice, as a
		 * glitch past a single test would end the chain early and leave the rest of the
		 * blob unread.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsu_IsBufferNonZero(TrailerPtr, XKEYMANAGER_TRAILER_SIZE);
		if (XASUFW_SUCCESS == Status) {
			Status = XASUFW_KEYMANAGER_AEKV_FINAL_TAIL_INVALID;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsu_IsBufferNonZero(TrailerPtr, XKEYMANAGER_TRAILER_SIZE);
		if (XASUFW_SUCCESS == Status) {
			Status = XASUFW_KEYMANAGER_AEKV_FINAL_TAIL_INVALID;
			goto END;
		}

		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function stores the decrypted data blocks of one chunk via the caller sink.
 *
 * @param	ParamsPtr		Pointer to the import parameters.
 * @param	ChunkIndex		Zero-based chunk index.
 * @param	PlaintextBytesPtr	Running count of plaintext bytes handed to the sink.
 *
 * @return
 *		- XASUFW_SUCCESS, if every block within the plaintext bound is stored.
 *
 *************************************************************************************************/
static s32 XKeyManager_ChunkStoreBlocks(const XKeyManager_ImportParams *ParamsPtr,
					u32 ChunkIndex, u32 *PlaintextBytesPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XKeyManager_WorkBuf *WorkPtr = ParamsPtr->WorkPtr;
	u32 SlotIdx;
	u32 GlobalBlockIdx;
	u32 WriteOffset;
	u32 WriteLen;
	const u8 *BlockPtr = NULL;

	for (SlotIdx = 0U; SlotIdx < XKEYMANAGER_DATA_BLOCKS_PER_CHUNK; SlotIdx++) {
		BlockPtr = WorkPtr->ChunkBuf + (SlotIdx * XKEYMANAGER_BLOCK_SIZE);
		GlobalBlockIdx = (ChunkIndex * XKEYMANAGER_DATA_BLOCKS_PER_CHUNK) + SlotIdx;
		WriteOffset = GlobalBlockIdx * XKEYMANAGER_BLOCK_SIZE;

		/**
		 * The body is padded to whole chunks, so the last chunks may hold blocks the
		 * plaintext never reached. They are dropped here instead of being stored.
		 */
		if (WriteOffset >= ParamsPtr->MaxPlaintextSize) {
			break;
		}

		/** A block that straddles the end of the plaintext is stored only up to it. */
		WriteLen = XKEYMANAGER_BLOCK_SIZE;
		if (WriteLen > (ParamsPtr->MaxPlaintextSize - WriteOffset)) {
			WriteLen = ParamsPtr->MaxPlaintextSize - WriteOffset;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = ParamsPtr->SinkFn(WriteOffset, BlockPtr, WriteLen);
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}

		*PlaintextBytesPtr += WriteLen;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function advances the digest chain to the next chunk. The trailer just
 *		consumed described the successor, so its secrets and that digest become the ones
 *		the next pass of the walk works with.
 *
 * @param	ParamsPtr	Pointer to the import parameters.
 *
 * @return
 *		- XASUFW_SUCCESS, if the chain is advanced.
 *		- XASUFW_MEM_COPY_FAIL, if the successor secrets or digest cannot be copied.
 *
 *************************************************************************************************/
static s32 XKeyManager_ChunkAdvanceChain(const XKeyManager_ImportParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XKeyManager_WorkBuf *WorkPtr = ParamsPtr->WorkPtr;

	/**
	 * The trailer just consumed described the next chunk, so those secrets and that digest
	 * become the ones the next pass of the walk works with.
	 */
	Status = Xil_SMemCpy(WorkPtr->ChunkSecrets, XKEYMANAGER_CHUNK_SECRETS_SIZE,
			     WorkPtr->NextChunkSecrets, XKEYMANAGER_CHUNK_SECRETS_SIZE,
			     XKEYMANAGER_CHUNK_SECRETS_SIZE);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(WorkPtr->ExpectedHash, XKEYMANAGER_HASH_SIZE,
			     WorkPtr->NextChunkHash, XKEYMANAGER_HASH_SIZE,
			     XKEYMANAGER_HASH_SIZE);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_MEM_COPY_FAIL;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function opens a chunk-wise import and reports the chunk count to walk.
 *
 * @param	ParamsPtr	Pointer to the import parameters.
 * @param	NumChunksPtr	Output chunk count of the body.
 *
 * @return
 *		- XASUFW_SUCCESS, if the parameters are usable.
 *
 *************************************************************************************************/
static s32 XKeyManager_ImportBegin(const XKeyManager_ImportParams *ParamsPtr, u32 *NumChunksPtr)
{
	return XKeyManager_ValidateImportParams(ParamsPtr, NumChunksPtr);
}

/*************************************************************************************************/
/**
 * @brief	This function imports one chunk. It authenticates the chunk before anything of
 *		it is decrypted, decrypts it in place under the secrets the chain handed over,
 *		reads what the trailer says about the next chunk, hands the data blocks to the
 *		caller sink, and leaves the chain pointing at the successor.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr		Pointer to the XAes instance.
 * @param	ShaInstancePtr		Pointer to the XSha instance.
 * @param	ParamsPtr		Pointer to the import parameters.
 * @param	ChunkIndex		Zero-based chunk index.
 * @param	PlaintextBytesPtr	Running count of plaintext bytes handed to the sink.
 *
 * @return
 *		- XASUFW_SUCCESS, if the chunk is authenticated, decrypted, and stored.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if an instance pointer is NULL or the chunk
 *		  index is past the last chunk of this body.
 *		- XASUFW_KEYMANAGER_AEKV_ZEROIZE_FAILED, if the unit scratch does not wipe.
 *
 * @note	Import walks chunks first-to-last, following the digest chain forward from
 *		chunk 0.
 *
 *************************************************************************************************/
static s32 XKeyManager_ImportChunk(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, XSha *ShaInstancePtr,
			   const XKeyManager_ImportParams *ParamsPtr, u32 ChunkIndex,
			   u32 *PlaintextBytesPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihChunk = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XKeyManager_WorkBuf *WorkPtr = NULL;
	u32 NumChunks = 0U;

	if ((DmaPtr == NULL) || (AesInstancePtr == NULL) || (ShaInstancePtr == NULL) ||
	    (PlaintextBytesPtr == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	Status = XKeyManager_ValidateImportParams(ParamsPtr, &NumChunks);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	if (ChunkIndex >= NumChunks) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	WorkPtr = ParamsPtr->WorkPtr;

	/** Authenticate the chunk before anything of it is decrypted. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ChunkAuthenticate(ShaInstancePtr, DmaPtr, ParamsPtr, ChunkIndex);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Decrypt it in place under the secrets the chain handed over. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_DecryptChunk(DmaPtr, AesInstancePtr, WorkPtr->ChunkSecrets,
					 WorkPtr->ChunkBuf, &WorkPtr->Scratch);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Read what the trailer says about the next chunk, or check the end marker. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ChunkProcessTrailer(ParamsPtr, ChunkIndex, NumChunks);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Hand the data blocks of this chunk to the caller sink. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ChunkStoreBlocks(ParamsPtr, ChunkIndex, PlaintextBytesPtr);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Every chunk but the last leaves the chain pointing at its successor. */
	if (ChunkIndex < (NumChunks - 1U)) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_ChunkAdvanceChain(ParamsPtr);
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	/**
	 * The scratch held plaintext of this chunk and goes here. The chunk secrets stay,
	 * because the walk still needs them for the chunk that follows.
	 */
	if (WorkPtr != NULL) {
		ASSIGN_VOLATILE(ClearStatus, XASUFW_FAILURE);
		XFIH_CALL(Xil_SecureZeroize, XFihChunk, ClearStatus, (u8 *)&WorkPtr->Scratch,
			  (u32)sizeof(XKeyManager_UnitScratch));
		if (XASUFW_SUCCESS != ClearStatus) {
			ClearStatus = XASUFW_KEYMANAGER_AEKV_ZEROIZE_FAILED;
		}
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function closes a chunk-wise import by wiping the working buffers, which
 *		still hold the secrets and the plaintext of the last chunk walked.
 *
 * @param	ParamsPtr	Pointer to the import parameters.
 *
 * @return
 *		- XASUFW_SUCCESS, if the working buffers are wiped.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if a pointer is NULL.
 *		- XASUFW_KEYMANAGER_AEKV_ZEROIZE_FAILED, if the working buffers do not wipe.
 *
 *************************************************************************************************/
static s32 XKeyManager_ImportFinish(const XKeyManager_ImportParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihImport = XFih_VolatileAssignXfihVar(XFIH_FAILURE);

	if ((ParamsPtr == NULL) || (ParamsPtr->WorkPtr == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** The buffers still hold the secrets and the plaintext of the last chunk walked. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(Xil_SecureZeroize, XFihImport, Status, (u8 *)ParamsPtr->WorkPtr,
		  (u32)sizeof(XKeyManager_WorkBuf));
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_KEYMANAGER_AEKV_ZEROIZE_FAILED;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function imports a chunked blob body by walking every chunk first-to-last.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr		Pointer to the XAes instance.
 * @param	ShaInstancePtr		Pointer to the XSha instance.
 * @param	ParamsPtr		Pointer to the import parameters.
 * @param	PlaintextBytesPtr	Output count of plaintext bytes handed to the sink.
 *
 * @return
 *		- XASUFW_SUCCESS, if the whole body is authenticated and decrypted.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if the plaintext count pointer is NULL.
 *
 * @note	The caller seeds chunk-0 secrets and digest from its blob header before the call.
 *
 *************************************************************************************************/
s32 XKeyManager_ChunkBlobImport(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, XSha *ShaInstancePtr,
		      const XKeyManager_ImportParams *ParamsPtr, u32 *PlaintextBytesPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 NumChunks = 0U;
	u32 ChunkIdx;

	if (PlaintextBytesPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	Status = XKeyManager_ImportBegin(ParamsPtr, &NumChunks);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	*PlaintextBytesPtr = 0U;

	/**
	 * Walk the chunks forward. Each one is authenticated by the digest its predecessor
	 * named, so the chain is followed from the chunk-0 digest the caller seeded.
	 */
	for (ChunkIdx = 0U; ChunkIdx < NumChunks; ChunkIdx++) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_ImportChunk(DmaPtr, AesInstancePtr, ShaInstancePtr, ParamsPtr,
						ChunkIdx, PlaintextBytesPtr);
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	/** The working buffers are wiped whether the body was consumed or abandoned. */
	if ((ParamsPtr != NULL) && (ParamsPtr->WorkPtr != NULL)) {
		ClearStatus = XKeyManager_ImportFinish(ParamsPtr);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}

	return Status;
}

#endif /* XASU_KEYMANAGER_ENABLE */
/** @} */
