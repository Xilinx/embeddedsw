/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xkeymanager_chunk.h
 *
 * This file contains the chunked blob body format and the export and import APIs that
 * xkeymanager_chunk.c implements, shared by the blob formats that differ only in their header. It
 * is reusable and format agnostic: nothing declared here knows about any particular blob header,
 * so a new format only has to supply its own header handling on top.
 *
 * Layer hierarchy (top to bottom):
 *
 *   Blob format  -- xkeymanager_aekv.c: header, signature, chunk-0 secrets
 *        |
 *   Body         -- xkeymanager_chunk.c: geometry, the chunk walk, one 528-byte block at a time
 *        |
 *   Primitives   -- xkeymanager_crypto.c: weak-key rejection, IV advance, SHA-384, AES-256-CBC
 *
 * The lower two layers never look at a blob header, so neither depends on the format layer above
 * them.
 *
 * A blob body is a sequence of equal-sized chunks. Every chunk holds N-1 data blocks followed by
 * a trailer block, each encrypted under its own KEK and IV. The trailer of an intermediate chunk
 * carries the next chunk secrets and digest; the final chunk trailer is an all-zero end marker.
 * Chunk-0 secrets and digest come from the blob header.
 *
 * The secrets a trailer carries go through the caller wrap on the way in and the caller unwrap on
 * the way out, so they are protected the same way as the chunk-0 secrets the blob header carries
 * rather than by the trailer KEK alone.
 *
 * An import walks chunk 0 first (forward digest chain). An export walks the last chunk first
 * (reverse assembly order). Callers move the whole body in one call via ChunkBlobExport or
 * ChunkBlobImport, which walk the chunks internally.
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
#ifndef XKEYMANAGER_CHUNK_H_
#define XKEYMANAGER_CHUNK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif
#include "xasufw_dma.h"
#include "xasufw_util.h"
#include "xaes.h"
#include "xsha.h"
#include "xkeymanager_crypto.h"

#ifdef XASU_KEYMANAGER_ENABLE
/************************************ Constant Definitions ***************************************/
/** Fixed 528-byte encrypted data block size. */
#define XKEYMANAGER_BLOCK_SIZE			(528U)

/**
 * Keyed units per chunk (N). N = 16 -> 15 data blocks followed by one trailer. Each unit is
 * encrypted under a KEK of its own and under the chunk IV advanced by the unit index, so the
 * secrets a chunk hands to the next one are one IV plus N KEKs.
 */
#define XKEYMANAGER_BLOCKS_PER_CHUNK		(16U)

/** Data blocks per chunk (units 0 ... N-2). */
#define XKEYMANAGER_DATA_BLOCKS_PER_CHUNK	(XKEYMANAGER_BLOCKS_PER_CHUNK - 1U)

/** Unit index of the chunk trailer. */
#define XKEYMANAGER_TRAILER_BLOCK_INDEX		(XKEYMANAGER_BLOCKS_PER_CHUNK - 1U)

/** One bit per block, the value a randomized block order must cover exactly. */
#define XKEYMANAGER_BLOCK_ORDER_FULL_MASK	(((u32)1U << XKEYMANAGER_BLOCKS_PER_CHUNK) - 1U)

/** One IV per chunk; every unit of that chunk uses it advanced by the unit index. */
#define XKEYMANAGER_CHUNK_IV_SIZE		(XKEYMANAGER_AES_IV_SIZE)

/** Per-chunk KEK field size (N x 32). */
#define XKEYMANAGER_CHUNK_KEKS_SIZE		(XKEYMANAGER_BLOCKS_PER_CHUNK * \
						 XKEYMANAGER_AES_KEY_SIZE)

/**
 * Per-chunk secrets: the chunk IV followed by the N unit KEKs. Every chunk needs both to be
 * decrypted, so the two travel together - in the blob header for chunk 0 and in the trailer of
 * the preceding chunk for every other chunk.
 */
#define XKEYMANAGER_CHUNK_SECRETS_SIZE		(XKEYMANAGER_CHUNK_IV_SIZE + \
						 XKEYMANAGER_CHUNK_KEKS_SIZE)

/** Byte offset of the chunk IV inside a chunk secrets buffer. */
#define XKEYMANAGER_SECRETS_IV_OFFSET		(0U)

/** Byte offset of the KEK of unit index Idx inside a chunk secrets buffer. */
#define XKEYMANAGER_SECRETS_KEK_OFFSET(Idx)	(XKEYMANAGER_CHUNK_IV_SIZE + \
						 ((Idx) * XKEYMANAGER_AES_KEY_SIZE))

/**
 * The secrets of a chunk are wrapped piece by piece rather than as one buffer: the chunk IV
 * under the first derived key, and the N unit KEKs in groups of two under one derived key each.
 * The same nine keys wrap the secrets of every chunk, whether they travel in the blob header as
 * the chunk-0 secrets or in the trailer of the preceding chunk, and a piece takes the header IV
 * advanced past every piece the earlier chunks consumed, so no derived key ever encrypts twice
 * under the same IV.
 */
#define XKEYMANAGER_SECRETS_KEKS_PER_GROUP		(2U)

/** Number of unit KEK groups the secrets of a chunk are wrapped in. */
#define XKEYMANAGER_SECRETS_GROUP_COUNT		(XKEYMANAGER_BLOCKS_PER_CHUNK / \
						 XKEYMANAGER_SECRETS_KEKS_PER_GROUP)

/** Pieces the secrets of a chunk are wrapped in, that is the KEK groups and the chunk IV. */
#define XKEYMANAGER_SECRETS_PIECE_COUNT		(XKEYMANAGER_SECRETS_GROUP_COUNT + 1U)

/** Byte size of one wrapped KEK group. */
#define XKEYMANAGER_SECRETS_GROUP_SIZE		(XKEYMANAGER_SECRETS_KEKS_PER_GROUP * \
						 XKEYMANAGER_AES_KEY_SIZE)

/** Byte size of the derived key material, one key per piece. */
#define XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE		(XKEYMANAGER_SECRETS_PIECE_COUNT * \
						 XKEYMANAGER_AES_KEY_SIZE)

/** Byte offset of the next chunk secrets in the trailer plaintext. */
#define XKEYMANAGER_TRAILER_SECRETS_OFFSET	(0U)

/** Byte offset of the next chunk digest in the trailer plaintext. */
#define XKEYMANAGER_TRAILER_HASH_OFFSET		(XKEYMANAGER_CHUNK_SECRETS_SIZE)

/**
 * Trailer plaintext size, that is the next chunk secrets followed by its digest.
 *
 * The trailer is one 576-byte CBC operation over both fields, so the next chunk digest is
 * encrypted along with the 528 bytes of secrets rather than left in the clear beside them. The
 * digest is taken over ciphertext that travels in the same blob, so encrypting it hides nothing
 * that a holder of the blob could not recompute; what it does give is that the digest is covered
 * by the digest of this chunk, so the chain cannot be retargeted without breaking the chunk the
 * digest lives in.
 */
#define XKEYMANAGER_TRAILER_SIZE			(XKEYMANAGER_TRAILER_HASH_OFFSET + \
						 XKEYMANAGER_HASH_SIZE)

/** Byte offset of the trailer inside a chunk; the data blocks occupy everything before it. */
#define XKEYMANAGER_TRAILER_OFFSET		(XKEYMANAGER_DATA_BLOCKS_PER_CHUNK * \
						 XKEYMANAGER_BLOCK_SIZE)

/** Chunk size in bytes: N-1 data blocks followed by the trailer. */
#define XKEYMANAGER_CHUNK_SIZE			(XKEYMANAGER_TRAILER_OFFSET + \
						 XKEYMANAGER_TRAILER_SIZE)

/** A chunk is the data blocks packed back to back with the trailer last and no gap. */
_Static_assert(XKEYMANAGER_CHUNK_SIZE == ((XKEYMANAGER_DATA_BLOCKS_PER_CHUNK *
					  XKEYMANAGER_BLOCK_SIZE) + XKEYMANAGER_TRAILER_SIZE),
	       "Chunk must hold N-1 data blocks followed by the trailer");

/** The trailer is exactly the next chunk secrets and digest, so it has no spare room. */
_Static_assert(XKEYMANAGER_TRAILER_SIZE == (XKEYMANAGER_CHUNK_SECRETS_SIZE +
					   XKEYMANAGER_HASH_SIZE),
	       "Trailer must hold the next chunk secrets and its digest");

/** Every unit goes through AES-CBC, so both unit lengths must be whole AES blocks. */
_Static_assert((XKEYMANAGER_BLOCK_SIZE % XASU_AES_BLOCK_SIZE_IN_BYTES) == 0U,
	       "Data block size must be a multiple of the AES block length");
_Static_assert((XKEYMANAGER_TRAILER_SIZE % XASU_AES_BLOCK_SIZE_IN_BYTES) == 0U,
	       "Trailer size must be a multiple of the AES block length");

/*************************** Macros (Inline Functions) Definitions *******************************/
/**
 * Rounds the division of Numerator by Denominator up to the next whole number, which is how the
 * blob geometry counts the blocks and chunks a partly filled unit still needs. Both arguments
 * must be unsigned, and Denominator must not be zero.
 */
#define XKEYMANAGER_CEIL_DIV(Numerator, Denominator) \
	(((Numerator) / (Denominator)) + \
	 ((((Numerator) % (Denominator)) != 0U) ? 1U : 0U))

/************************************** Type Definitions *****************************************/
/**
 * Sink that stores one decrypted data block. The blob body says where a block belongs in the
 * plaintext, and the caller decides what that plaintext is and where it is kept, so the import
 * never writes to a destination of its own.
 */
typedef s32 (*XKeyManager_BlockSink_t)(u32 Offset, const u8 *BlockPtr, u32 Len);

/**
 * Wrap or unwrap of the secrets of one chunk, applied as they enter or leave a trailer. The
 * secrets of chunk 0 reach the caller through its own blob header, so the caller protects both
 * the same way and passes the chunk index the secrets describe, which is what lets the caller
 * keep every wrap of one operation under a key and IV of its own.
 */
typedef s32 (*XKeyManager_SecretsCrypt_t)(void *ArgPtr, u32 ChunkIndex, const u8 *InPtr,
					  u8 *OutPtr);

/**
 * Scratch one chunk crypto pass needs. A chunk is encrypted and decrypted unit by unit in place,
 * so this holds only what one unit is worth, and the same shape serves both directions.
 */
typedef struct {
	u8 UnitOrder[XKEYMANAGER_BLOCKS_PER_CHUNK];	/**< Randomized unit processing
							     order. */
	u8 UnitIv[XKEYMANAGER_AES_IV_SIZE];		/**< Chunk IV advanced by the index of
							     the current unit. */
	u8 UnitBuf[XKEYMANAGER_TRAILER_SIZE];		/**< Result of the current unit before
							     it goes back into the chunk; the
							     trailer is the longest unit. */
} XKeyManager_UnitScratch;

/**
 * Working buffers of one import, owned by the caller so the import holds no state of its own.
 * The caller seeds the first chunk secrets and the first chunk digest from its own blob header
 * before the call, and the import advances both across the chunk chain.
 */
typedef struct {
	u8 ChunkSecrets[XKEYMANAGER_CHUNK_SECRETS_SIZE];		/**< IV and KEKs of the
								     current chunk, chunk-0
								     secrets on entry. */
	u8 NextChunkSecrets[XKEYMANAGER_CHUNK_SECRETS_SIZE];	/**< IV and KEKs recovered from
								     the trailer. */
	u8 ExpectedHash[XKEYMANAGER_HASH_SIZE];			/**< Digest the current chunk
								     must match, chunk-0 digest
								     on entry. */
	u8 NextChunkHash[XKEYMANAGER_HASH_SIZE];			/**< Digest recovered from
								     the trailer. */
	XKeyManager_UnitScratch Scratch;				/**< Scratch of the chunk
								     decrypt pass. */
	u8 ChunkBuf[XKEYMANAGER_CHUNK_SIZE];			/**< Chunk being decrypted. */
} XKeyManager_WorkBuf;

/** Keys, IV and buffers of one wrap or unwrap of the secrets of a chunk. */
typedef struct {
	const u8 *KeyMaterial;	/**< XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE bytes of
				     derived keys, one per piece. */
	const u8 *BaseIv;	/**< Header IV that every piece IV is derived from. */
	const u8 *Input;	/**< Secrets buffer to read, XKEYMANAGER_CHUNK_SECRETS_SIZE. */
	u8 *Output;		/**< Secrets buffer to write, XKEYMANAGER_CHUNK_SECRETS_SIZE. */
	u32 ChunkIndex;		/**< Index of the chunk the secrets describe, which spaces
				     this wrap past the pieces every earlier chunk
				     consumed. */
} XKeyManager_SecretsParams;

/** Source, destination and working buffers of one chunked blob body import. */
typedef struct {
	u64 BodyAddr;			/**< Address of chunk 0, past the blob header. */
	u32 BodySize;			/**< Body size in bytes, a multiple of the chunk size. */
	u32 MaxPlaintextSize;		/**< Plaintext bytes the sink accepts. A body is padded up
					     to a chunk boundary, so the blocks beyond this are
					     padding and are dropped. */
	XKeyManager_BlockSink_t SinkFn;	/**< Sink of every decrypted data block. */
	XKeyManager_SecretsCrypt_t SecretsUnwrapFn;	/**< Unwrap of the secrets a trailer
							     carries. */
	void *SecretsArgPtr;		/**< Argument handed back to the unwrap unchanged. */
	XKeyManager_WorkBuf *WorkPtr;	/**< Working buffers, seeded with the chunk-0 secrets and
					     digest. */
} XKeyManager_ImportParams;

/**
 * Source that supplies one span of the plaintext. The blob body says which part of the plaintext
 * it needs, and the caller decides what that plaintext is and where it is kept, so the export
 * never reads from a source of its own. The export asks only for bytes the plaintext holds and
 * zero-fills the rest of the block itself, so the source never sees an out-of-range request and
 * never has to deal with the padding at the end of the body.
 */
typedef s32 (*XKeyManager_BlockSource_t)(u32 Offset, u8 *BlockPtr, u32 Len);

/**
 * Working buffers of one export, owned by the caller so the export holds no state of its own.
 * The export leaves the secrets and the digest of chunk 0 with the caller, which is what the
 * caller needs to build its own blob header.
 */
typedef struct {
	u8 ChunkSecrets[XKEYMANAGER_CHUNK_SECRETS_SIZE];		/**< IV and KEKs of the
								     chunk being built. */
	u8 NextChunkSecrets[XKEYMANAGER_CHUNK_SECRETS_SIZE];	/**< IV and KEKs of the chunk
								     built before this one. */
	u8 NextChunkHash[XKEYMANAGER_HASH_SIZE];			/**< Digest of that
								     chunk. */
	XKeyManager_UnitScratch Scratch;				/**< Scratch of the chunk
								     encrypt pass. */
	u8 ChunkBuf[XKEYMANAGER_CHUNK_SIZE];			/**< Chunk under construction,
								     assembled as plaintext and
								     encrypted in place. */
} XKeyManager_ExportWorkBuf;

/** Source, destination and working buffers of one chunked blob body export. */
typedef struct {
	u64 BodyAddr;				/**< Address chunk 0 is written to, past the
						     blob header. */
	u32 BodySize;				/**< Body size in bytes, a multiple of the chunk
						     size. */
	u32 PlaintextSize;			/**< Plaintext bytes the source holds. */
	XKeyManager_BlockSource_t SourceFn;	/**< Source of every plaintext data block. */
	XKeyManager_SecretsCrypt_t SecretsWrapFn; /**< Wrap of the secrets a trailer carries. */
	void *SecretsArgPtr;			/**< Argument handed back to the wrap
						     unchanged. */
	XKeyManager_ExportWorkBuf *WorkPtr;	/**< Working buffers. */
	u8 *Chunk0SecretsPtr;			/**< Output IV and KEKs of chunk 0, for the
						     caller blob header. */
	u8 *Chunk0HashPtr;			/**< Output digest of chunk 0, for the caller
						     blob header. */
} XKeyManager_ExportParams;

/*************************************************************************************************/
/**
 * @brief	This function computes the number of data blocks a plaintext occupies.
 *
 * @param	PlaintextSize	Plaintext size in bytes.
 *
 * @return
 *		- Block count (ceil(PlaintextSize / block size)); 0 when PlaintextSize is 0.
 *
 *************************************************************************************************/
static inline u32 XKeyManager_CalcNumBlocks(u32 PlaintextSize)
{
	u32 NumBlocks = 0U;

	if (PlaintextSize > 0U) {
		NumBlocks = XKEYMANAGER_CEIL_DIV(PlaintextSize, XKEYMANAGER_BLOCK_SIZE);
	}

	return NumBlocks;
}

/*************************************************************************************************/
/**
 * @brief	This function computes the number of chunks a block count occupies.
 *
 * @param	NumBlocks	Total plaintext block count.
 *
 * @return
 *		- Chunk count (ceil(NumBlocks / (N - 1))); 0 when NumBlocks is 0.
 *
 *************************************************************************************************/
static inline u32 XKeyManager_CalcNumChunks(u32 NumBlocks)
{
	u32 NumChunks = 0U;

	if (NumBlocks > 0U) {
		NumChunks = XKEYMANAGER_CEIL_DIV(NumBlocks, XKEYMANAGER_DATA_BLOCKS_PER_CHUNK);
	}

	return NumChunks;
}

/*************************************************************************************************/
/**
 * @brief	This function computes the blob body size a plaintext needs.
 *
 * @param	PlaintextSize	Plaintext size in bytes.
 *
 * @return
 *		- Body size in bytes (chunk count x chunk size); 0 when PlaintextSize is 0.
 *
 * @note	The geometry is stated here rather than in xkeymanager_chunk.c because the resident
 *		blob format layer sizes a blob before it activates the overlay the chunk walk is
 *		linked into, and an overlay image cannot be called from outside its own phase.
 *
 *************************************************************************************************/
static inline u32 XKeyManager_CalcBodySize(u32 PlaintextSize)
{
	return XKeyManager_CalcNumChunks(XKeyManager_CalcNumBlocks(PlaintextSize)) *
	       XKEYMANAGER_CHUNK_SIZE;
}

/************************************ Function Prototypes ****************************************/
/** Secrets of one chunk, wrapped and recovered piece by piece. */
s32 XKeyManager_WrapSecrets(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
		      const XKeyManager_SecretsParams *ParamsPtr);
s32 XKeyManager_UnwrapSecrets(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
		      const XKeyManager_SecretsParams *ParamsPtr);

/** Whole body: every chunk in one call. */
s32 XKeyManager_ChunkBlobExport(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, XSha *ShaInstancePtr,
		      const XKeyManager_ExportParams *ParamsPtr);
s32 XKeyManager_ChunkBlobImport(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, XSha *ShaInstancePtr,
		      const XKeyManager_ImportParams *ParamsPtr, u32 *PlaintextBytesPtr);

#endif /* XASU_KEYMANAGER_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XKEYMANAGER_CHUNK_H_ */
/** @} */
