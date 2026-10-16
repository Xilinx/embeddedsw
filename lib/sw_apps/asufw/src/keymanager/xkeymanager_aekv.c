/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xkeymanager_aekv.c
 *
 * AEKV key vault export and import.
 *
 * This file owns the AEKV blob end to end: the blob format and its sizing helpers, the off-chip
 * revoke eFuse readers, the crypto that protects the chunk-0 secrets, and the two orchestrations
 * that turn the live key vault into a portable encrypted blob and rebuild the vault from one. The
 * per-chunk and per-block work is delegated to the chunk layer in xkeymanager_chunk.c, and the
 * primitives both layers are built from live in xkeymanager_crypto.c, so neither of them has to
 * know the blob format.
 *
 * The export stages, top to bottom, are:
 *
 *   - Pre-export checks, before any key material is touched. The caller buffer has to be large
 *     enough, an exportable vault has to exist, and the requested revocation eFuse has to be
 *     usable.
 *   - Partition the vault into blocks and size the blob, so the header can record the plaintext
 *     size and the body layout is fixed before encryption starts.
 *   - Draw the header IV and derive the keys that wrap chunk secrets, before any chunk exists,
 *     because every chunk trailer carries the secrets of its successor wrapped under them.
 *   - Build the encrypted chunk body. The chunk layer generates the per-chunk IV and block KEKs
 *     and the block layer encrypts, one chunk at a time, so the export context only ever holds
 *     one chunk of secrets rather than all of them.
 *   - Wrap the chunk-0 IV and KEKs into the header, which is what makes the body recoverable on
 *     import.
 *   - Assemble and sign the header with the UDE key and write it to the export buffer.
 *   - Publish the export result to the caller and mark the vault clean.
 *
 * The body is built before the header is signed. Chunks are walked last to first, so chunk 0 is
 * built last, and its digest is the anchor of the chain that covers the whole body. That digest
 * goes into the header, which is why the header cannot be signed any earlier.
 *
 * The import stages mirror them:
 *
 *   - Load the header from the caller blob, after XKeyManager_AekvImport has confirmed at entry
 *     that the crypto known-answer tests have run.
 *   - Validate the header fields, so a blob that is not an AEKV blob of a supported version is
 *     rejected before any key is derived.
 *   - Verify the UDE signature over the header, which is what proves the blob came from this
 *     device and has not been altered.
 *   - Reject a blob whose epoch the caller has already revoked, so an old blob cannot be
 *     replayed.
 *   - Decrypt the chunk-0 IV and KEKs out of the header, which is what makes the body
 *     recoverable.
 *   - Import the chunk body, authenticating each chunk against the digest the chain expects
 *     before its blocks are decrypted.
 *   - Validate the recovered contents and rebuild the vault registry.
 *
 * The header is authenticated before any key is derived, and each chunk is authenticated against
 * the chain before its blocks are decrypted, so nothing is ever decrypted on the strength of an
 * unverified blob. Plaintext is written straight into the vault region as it is recovered, which
 * is why the import region has to be empty first: the caller deletes any vault it still needs
 * before asking for an import. Chunks are walked first to last here, the opposite of export,
 * because the chain is verified forward from chunk 0.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vm   09/15/26 Initial release
 *       vm   10/17/26 Passed the DMA instance to the import region prepare and revert
 *                     interfaces
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
#include "xkeymanager.h"
#include "xkeymanager_aekv.h"
#include "xkeymanager_chunk.h"
#include "xkeymanager_crypto.h"
#include "xasu_keymanagerinfo.h"
#include "xasufw_status.h"
#include "xasufw_dma.h"
#include "xasufw_hw.h"
#include "xasufw_util.h"
#include "xasu_generic.h"
#include "xaes.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xkdf.h"
#include "xtrng.h"
#include "xtrng_hw.h"
#include "xil_io.h"
#include "xil_sutil.h"
#ifdef XASU_OCP_ENABLE
#include "xocp_ude.h"
#endif

#ifdef XASU_KEYMANAGER_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/
/**
 * Engines and key material one secrets wrap or unwrap borrows from the running operation. The
 * chunk layer hands the wrap only the chunk index and the buffers, so everything else it needs
 * travels here.
 */
typedef struct {
	XAsufw_Dma *DmaPtr;		/**< Pointer to the XAsufw_Dma instance. */
	XAes *AesInstancePtr;		/**< Pointer to the XAes instance. */
	const u8 *KeyMaterial;		/**< Derived keys, one per piece. */
	const u8 *BaseIv;		/**< Header IV every piece IV counts up from. */
} XKeyManager_SecretsCryptArg;

/*************************** Macros (Inline Functions) Definitions *******************************/
/** The running export session, inside the storage export and import share. */
#define XKEYMANAGER_EXPORT_SESSION	(XKEYMANAGER_SESSION->Export)

/** The running import session, inside the same storage. */
#define XKEYMANAGER_IMPORT_SESSION	(XKEYMANAGER_SESSION->Import)

/************************************ Function Prototypes ****************************************/
/** Blob format and revocation. */
static u32 XKeyManager_CalcBlobSize(u32 PlaintextSize);
static s32 XKeyManager_ReadRevokeReg(u32 RegIdx, u32 *RegValPtr);
static s32 XKeyManager_ReadRevokeRegAlt(u32 RegIdx, u32 *RegValPtr);
static s32 XKeyManager_CheckRevokeEfuseUsable(u32 RevokeEfuseNum);

/** Chunk secrets keys. */
static s32 XKeyManager_DeriveSecretKeys(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				    const u8 *Context, u8 *KeyMaterial);
static s32 XKeyManager_TrailerSecretsWrap(void *ArgPtr, u32 ChunkIndex, const u8 *InPtr,
					  u8 *OutPtr);
static s32 XKeyManager_TrailerSecretsUnwrap(void *ArgPtr, u32 ChunkIndex, const u8 *InPtr,
					    u8 *OutPtr);

/** Export stages. */
static s32 XKeyManager_AekvExportPreChecks(u32 ExportBufSize, u32 PlaintextSize,
					   u32 RevokeEfuseNum);
static s32 XKeyManager_AekvExportPartitionBlocks(XKeyManager_ExportCtx *CtxPtr);
static s32 XKeyManager_AekvExportDeriveSecrets(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
					       XKeyManager_ExportCtx *CtxPtr);
static s32 XKeyManager_AekvExportBuildBody(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
					   XKeyManager_ExportCtx *CtxPtr, u64 ExportAddr);
static s32 XKeyManager_AekvExportWrapChunk0Secrets(XAsufw_Dma *DmaPtr,
						   XAes *AesInstancePtr,
						   XKeyManager_ExportCtx *CtxPtr);

/** Import stages. */
static s32 XKeyManager_AekvImportCheckRevocation(const XKeyManager_ImportCtx *CtxPtr);
static s32 XKeyManager_AekvImportUnwrapChunk0Secrets(XAsufw_Dma *DmaPtr,
						     XAes *AesInstancePtr,
						     XKeyManager_ImportCtx *CtxPtr);
static s32 XKeyManager_AekvImportChunkBody(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				      XKeyManager_ImportCtx *CtxPtr, u64 ImportAddr);
static s32 XKeyManager_AekvImportFinalize(const XKeyManager_ImportCtx *CtxPtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function computes the total AEKV blob size for a plaintext vault.
 *
 * @param	PlaintextSize	Exportable plaintext vault size in bytes.
 *
 * @return
 *		- Total blob size = header size of the newest version + KV_SIZE; 0 when
 *		  PlaintextSize is 0.
 *
 * @note	KV_SIZE is the chunked blob body, whose geometry AEKV shares with every other
 *		blob format, so only the header size is added here.
 *
 *************************************************************************************************/
static u32 XKeyManager_CalcBlobSize(u32 PlaintextSize)
{
	u32 KvSize;
	u32 BlobSize = 0U;

	KvSize = XKeyManager_CalcBodySize(PlaintextSize);
	if (KvSize > 0U) {
		BlobSize = XKeyManager_GetHeaderSize(XKEYMANAGER_EXPORT_VERSION) + KvSize;
	}

	return BlobSize;
}

/*************************************************************************************************/
/**
 * @brief	This function reads one off-chip revoke eFuse cache register as a word.
 *
 * @param	RegIdx		Register index (0 to XKEYMANAGER_MAX_REVOKE_REG_INDEX).
 * @param	RegValPtr	Pointer to output register value.
 *
 * @return
 *		- XASUFW_SUCCESS, if the read is successful.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if RegValPtr is NULL or RegIdx is out of range.
 *
 *************************************************************************************************/
static s32 XKeyManager_ReadRevokeReg(u32 RegIdx, u32 *RegValPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate input parameters. */
	if ((RegValPtr == NULL) || (RegIdx > XKEYMANAGER_MAX_REVOKE_REG_INDEX)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** Read the register as one word, which is the plain path of the redundant pair. */
	*RegValPtr = XAsufw_ReadReg(EFUSE_CACHE_OFFCHIP_REVOKE_0_ADDR +
				    (RegIdx * XASUFW_WORD_LEN_IN_BYTES));
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads one off-chip revoke eFuse cache register byte by byte. The
 *		caller compares the result against XKeyManager_ReadRevokeReg(), so this path
 *		deliberately uses a different access width and address computation rather than
 *		repeating the same word load.
 *
 * @param	RegIdx		Register index (0 to XKEYMANAGER_MAX_REVOKE_REG_INDEX).
 * @param	RegValPtr	Pointer to output register value.
 *
 * @return
 *		- XASUFW_SUCCESS, if the read is successful.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if RegValPtr is NULL or RegIdx is out of range.
 *
 *************************************************************************************************/
static s32 XKeyManager_ReadRevokeRegAlt(u32 RegIdx, u32 *RegValPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 ByteIdx;
	u32 RegAddr;
	u32 RegVal = 0U;

	/** Validate input parameters. */
	if ((RegValPtr == NULL) || (RegIdx > XKEYMANAGER_MAX_REVOKE_REG_INDEX)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Assemble the register from byte reads at a shifted address. Neither the access width
	 * nor the address arithmetic is shared with the word path, so a fault that lands on one
	 * of them does not carry the same wrong value into both halves of the comparison.
	 */
	RegAddr = EFUSE_CACHE_OFFCHIP_REVOKE_0_ADDR + (RegIdx << XKEYMANAGER_WORD_SHIFT);
	for (ByteIdx = 0U; ByteIdx < XASUFW_WORD_LEN_IN_BYTES; ByteIdx++) {
		RegVal |= ((u32)Xil_In8(RegAddr + ByteIdx)) <<
			  (ByteIdx * XASUFW_BYTE_LEN_IN_BITS);
	}

	*RegValPtr = RegVal;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks that an off-chip revocation eFuse is not blown. The eFuse
 *		word is read over two independent paths and the copies are compared before the
 *		revocation bit is tested, so a glitch past a single read or a single compare
 *		cannot make a revoked blob look usable.
 *
 * @param	RevokeEfuseNum	Revocation eFuse number a blob is bound to.
 *
 * @return
 *		- XASUFW_SUCCESS, if the eFuse is in range and not blown.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if the number has no eFuse bit.
 *		- XASUFW_KEYMANAGER_AEKV_EPOCH_REVOKED, if that bit is blown.
 *		- XASUFW_KEYMANAGER_AEKV_EFUSE_READ_MISMATCH, if the redundant reads disagree.
 *
 * @note	Blowing the eFuse a blob names is what retires that blob, so an import refuses a
 *		blob whose eFuse is blown and an export refuses to produce one that no import
 *		could ever accept.
 *
 *************************************************************************************************/
static s32 XKeyManager_CheckRevokeEfuseUsable(u32 RevokeEfuseNum)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var RegValAFih = XFIH_FAILURE;
	XFih_Var RegValBFih = XFIH_FAILURE;
	XFih_Var RevokeBitFih = XFIH_FAILURE;
	XFih_Var NotRevokedFih = XFIH_FAILURE;
	u32 RegIdx;
	u32 BitPos;
	u32 RegValA = 0U;
	u32 RegValB = 0U;

	/** Validate that the number names one of the off-chip revocation bits. */
	if (RevokeEfuseNum >= XASU_KM_KV_REVOKE_EFUSE_COUNT) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** Split the number into the eFuse word it lives in and its bit position in that word. */
	RegIdx = RevokeEfuseNum / XASUFW_BITS_IN_WORD;
	BitPos = RevokeEfuseNum % XASUFW_BITS_IN_WORD;

	/** Read the eFuse word twice over independent paths. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ReadRevokeReg(RegIdx, &RegValA);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ReadRevokeRegAlt(RegIdx, &RegValB);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/**
	 * The word is read twice over independent paths, so the two copies are compared
	 * redundantly, as a glitch past a single compare would waste the second read.
	 */
	RegValAFih = XFih_VolatileAssignU32(RegValA);
	RegValBFih = XFih_VolatileAssignU32(RegValB);

	ASSIGN_VOLATILE(Status, XASUFW_KEYMANAGER_AEKV_EFUSE_READ_MISMATCH);
	XFIH_IF_FAILIN_NEQ(RegValAFih, RegValBFih) {
		XFIH_GOTO(END);
	}

	/**
	 * A blown bit retires the blob, so the bit is taken from both reads and tested
	 * redundantly, as a glitch past a single test would accept a revoked blob.
	 */
	RevokeBitFih = XFih_VolatileAssignU32((RegValA | RegValB) & ((u32)1U << BitPos));
	NotRevokedFih = XFih_VolatileAssignU32(0U);

	ASSIGN_VOLATILE(Status, XASUFW_KEYMANAGER_AEKV_EPOCH_REVOKED);
	XFIH_IF_FAILIN_NEQ(RevokeBitFih, NotRevokedFih) {
		XFIH_GOTO(END);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function derives the keys that protect the chunk-0 secrets. A single CMAC KDF
 *		run seeded from the device unique eFUSE-0 black key produces all of the key
 *		material, which binds the keys to the device as well as to the caller context,
 *		and each derived key is rejected if it comes back all zeros or all ones.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	Context		Pointer to the caller context, XASU_KM_KV_CONTEXT_SIZE_IN_BYTES
 *				bytes.
 * @param	KeyMaterial	Output buffer for XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if the keys are derived.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if any input pointer is NULL.
 *		- XASUFW_KEYMANAGER_AEKV_WEAK_KEY_MATERIAL, if a derived key is all zeros or all
 *			ones.
 *		- XASUFW_KEYMANAGER_KV_KEK_GEN_FAIL, with the KDF error embedded, if the eFUSE-0
 *			CMAC KDF cannot derive the key material.
 *
 * @note	The keys are derived on demand for one export or one import and are never stored,
 *		so the same context has to be given on both sides for the blob to be recoverable.
 *
 *************************************************************************************************/
static s32 XKeyManager_DeriveSecretKeys(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				    const u8 *Context, u8 *KeyMaterial)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 KeyIdx;

	if ((DmaPtr == NULL) || (AesInstancePtr == NULL) || (Context == NULL) ||
	    (KeyMaterial == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * One KDF run produces every key the header needs. The seed is the eFUSE-0 black key,
	 * which is device unique, so the keys are bound to the device as well as to the context.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKdf_GenerateKekFromEfuse0(DmaPtr, AesInstancePtr, Context,
					    XASU_KM_KV_CONTEXT_SIZE_IN_BYTES,
					    XKEYMANAGER_KDF_IV_INC_VAL,
					    XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE, KeyMaterial);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_KV_KEK_GEN_FAIL);
		goto END;
	}

	/** A KDF that faults into an all-zero or all-one key must not be used to wrap secrets. */
	for (KeyIdx = 0U; KeyIdx < XKEYMANAGER_SECRETS_PIECE_COUNT; KeyIdx++) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_RejectWeakKey(&KeyMaterial[KeyIdx * XKEYMANAGER_AES_KEY_SIZE],
					     XKEYMANAGER_AES_KEY_SIZE);
		if (XASUFW_SUCCESS != Status) {
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is the callback the chunk layer invokes to wrap the secrets a chunk
 *		trailer hands to the next chunk. The keys and the base IV of the running export
 *		come from the argument the caller registered, since the chunk layer passes only
 *		the chunk index and the two buffers.
 *
 * @param	ArgPtr		Pointer to the XKeyManager_SecretsCryptArg of this export.
 * @param	ChunkIndex	Index of the chunk the secrets describe.
 * @param	InPtr		Plaintext secrets, XKEYMANAGER_CHUNK_SECRETS_SIZE bytes.
 * @param	OutPtr		Wrapped secrets, XKEYMANAGER_CHUNK_SECRETS_SIZE bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if the secrets are wrapped.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if the argument pointer is NULL.
 *		- Error codes from the piece walk, if a piece cannot be encrypted.
 *
 * @note	The trailer this writes into is then encrypted under the trailer KEK like every
 *		other block of its chunk, so the secrets carry both protections.
 *
 *************************************************************************************************/
static s32 XKeyManager_TrailerSecretsWrap(void *ArgPtr, u32 ChunkIndex, const u8 *InPtr,
					  u8 *OutPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XKeyManager_SecretsCryptArg *ArgumentPtr = NULL;
	XKeyManager_SecretsParams SecretsParams = {0U};

	if (ArgPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	ArgumentPtr = (const XKeyManager_SecretsCryptArg *)ArgPtr;

	/**
	 * The chunk layer knows only the chunk index and the two buffers, so the keys and the
	 * base IV of the running operation are taken from the argument the caller registered.
	 */
	SecretsParams.KeyMaterial = ArgumentPtr->KeyMaterial;
	SecretsParams.BaseIv = ArgumentPtr->BaseIv;
	SecretsParams.Input = InPtr;
	SecretsParams.Output = OutPtr;
	SecretsParams.ChunkIndex = ChunkIndex;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_WrapSecrets(ArgumentPtr->DmaPtr, ArgumentPtr->AesInstancePtr,
					 &SecretsParams);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is the callback the chunk layer invokes to recover the secrets a
 *		chunk trailer carries. It is the import side counterpart of the wrap callback and
 *		takes the keys and the base IV from the argument the caller registered.
 *
 * @param	ArgPtr		Pointer to the XKeyManager_SecretsCryptArg of this import.
 * @param	ChunkIndex	Index of the chunk the secrets describe.
 * @param	InPtr		Wrapped secrets, XKEYMANAGER_CHUNK_SECRETS_SIZE bytes.
 * @param	OutPtr		Plaintext secrets, XKEYMANAGER_CHUNK_SECRETS_SIZE bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if the secrets are recovered.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if the argument pointer is NULL.
 *		- Error codes from the piece walk, if a piece cannot be decrypted.
 *
 *************************************************************************************************/
static s32 XKeyManager_TrailerSecretsUnwrap(void *ArgPtr, u32 ChunkIndex, const u8 *InPtr,
					    u8 *OutPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XKeyManager_SecretsCryptArg *ArgumentPtr = NULL;
	XKeyManager_SecretsParams SecretsParams = {0U};

	if (ArgPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	ArgumentPtr = (const XKeyManager_SecretsCryptArg *)ArgPtr;

	/**
	 * The chunk layer knows only the chunk index and the two buffers, so the keys and the
	 * base IV of the running operation are taken from the argument the caller registered.
	 */
	SecretsParams.KeyMaterial = ArgumentPtr->KeyMaterial;
	SecretsParams.BaseIv = ArgumentPtr->BaseIv;
	SecretsParams.Input = InPtr;
	SecretsParams.Output = OutPtr;
	SecretsParams.ChunkIndex = ChunkIndex;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_UnwrapSecrets(ArgumentPtr->DmaPtr, ArgumentPtr->AesInstancePtr,
					   &SecretsParams);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs the pre-export checks before any key material is touched.
 *
 * @param	ExportBufSize	Export buffer size in bytes.
 * @param	PlaintextSize	Packed exportable plaintext size.
 * @param	RevokeEfuseNum	Off-chip revocation eFUSE number the caller chose.
 *
 * @return
 *		- XASUFW_SUCCESS, if all pre-checks pass.
 *		- Error codes from the revocation eFuse read, if the eFuse the blob names is blown
 *		  or the redundant reads disagree.
 *		- Error codes from the TRNG health test, if the TRNG is unhealthy, checked before
 *		  any key material is generated.
 *		- XASUFW_RAND_GEN_ERROR, with the TRNG error embedded, if the TRNG cannot hand out
 *		  a random number.
 *		- XASUFW_KEYMANAGER_AEKV_BUF_TOO_SMALL, if the caller's export buffer is smaller
 *		  than the blob this vault needs.
 *
 *************************************************************************************************/
static s32 XKeyManager_AekvExportPreChecks(u32 ExportBufSize, u32 PlaintextSize,
					   u32 RevokeEfuseNum)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 BlobSize;
	u8 RandByte = 0U;

	/**
	 * The sequencer settles the OCP build and the UDE slot before it enters the overlay, so
	 * the first check here is the one the sequencer cannot make: the eFuse the blob is bound
	 * to. A blob bound to an eFuse the caller already blew could never be imported back. The
	 * check reads that eFuse word over two independent paths and compares them, so nothing
	 * else has to read the revocation eFuses again.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_CheckRevokeEfuseUsable(RevokeEfuseNum);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/**
	 * Every KEK and IV the export writes comes from the TRNG, so its health test is taken
	 * before any key material is generated.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_IsHealthy(XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID));
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_GetRandomNumbers(&RandByte, 1U);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RAND_GEN_ERROR);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_HasExportableVault();
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	BlobSize = XKeyManager_CalcBlobSize(PlaintextSize);
	if ((BlobSize == 0U) || (ExportBufSize < BlobSize)) {
		Status = XASUFW_KEYMANAGER_AEKV_BUF_TOO_SMALL;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sizes the export before any key material exists. It latches the
 *		vault dirty state, sizes the body in whole chunks and the blob as that body
 *		behind a version 1 header, and fills the header fields that are known at this
 *		point.
 *
 * @param	CtxPtr		Pointer to export context.
 *
 * @return
 *		- XASUFW_SUCCESS, if geometry and header prefix fields are set.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if the context pointer is NULL.
 *
 *************************************************************************************************/
static s32 XKeyManager_AekvExportPartitionBlocks(XKeyManager_ExportCtx *CtxPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	if (CtxPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Read the dirty state once, here, and keep it as the hardened value the caller is given
	 * rather than as a flag, so no later stage has to decide what a plain 0 or 1 meant.
	 */
	if (XKeyManager_IsKeyVaultDirty() != 0U) {
		CtxPtr->ModifiedValue = XASU_KM_EXPORT_MODIFIED;
	} else {
		CtxPtr->ModifiedValue = XASU_KM_EXPORT_NOT_MODIFIED;
	}

	/** Size the body in whole chunks and the blob as that body behind a version 1 header. */
	CtxPtr->BodySize = XKeyManager_CalcBodySize(CtxPtr->PlaintextSize);
	CtxPtr->BlobSize = XKeyManager_GetHeaderSize(XKEYMANAGER_EXPORT_VERSION) +
			   CtxPtr->BodySize;

	/** Fill the header fields that are known before any key or chunk exists. */
	CtxPtr->Header.Tag = XASU_KM_AEKV_TAG;
	CtxPtr->Header.Version = XASU_KM_AEKV_VERSION_1;
	CtxPtr->Header.RevocationCounter = CtxPtr->RevokeEfuseNum;
	CtxPtr->Header.KvSize = CtxPtr->PlaintextSize;

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function draws the header IV and derives the keys that wrap chunk secrets.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	CtxPtr		Pointer to export context.
 *
 * @return
 *		- XASUFW_SUCCESS, if the IV is drawn and the keys are derived.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if CtxPtr is NULL.
 *		- XASUFW_KEYMANAGER_IV_GEN_ERROR, with the draw error embedded, if the IV cannot be
 *		  drawn.
 *		- XASUFW_KEYMANAGER_AEKV_WEAK_KEY_MATERIAL, if the IV or a derived key is weak.
 *		- Error codes from the eFUSE-0 derivation, if the key material cannot be produced.
 *
 * @note	Every chunk hands its successor a wrapped copy of that successor's secrets, so
 *		both the IV and the keys have to exist before the body is built rather than at
 *		the point the header is assembled. The derivation clears eFUSE key 0 as it goes,
 *		which is why it runs once per export and the material is held until the last wrap.
 *
 *************************************************************************************************/
static s32 XKeyManager_AekvExportDeriveSecrets(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
					       XKeyManager_ExportCtx *CtxPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	if (CtxPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Every piece IV of every chunk counts up from this one, so it is held to the same
	 * standard as the per-block IVs the chunk layer draws.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_GenerateStrongRandom(CtxPtr->Header.KekIv, XKEYMANAGER_AES_IV_SIZE);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_IV_GEN_ERROR);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_DeriveSecretKeys(DmaPtr, AesInstancePtr, CtxPtr->Context,
					      CtxPtr->SecretKeys);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function builds the encrypted chunk body behind the header. It describes the
 *		body to the chunk layer, which draws the plaintext through the export source
 *		callback, encrypts each chunk and wraps the secrets every trailer hands to the
 *		next chunk.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	CtxPtr		Pointer to export context.
 * @param	ExportAddr	Destination export buffer address.
 *
 * @return
 *		- XASUFW_SUCCESS, if the chunk body is built and written past the header.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if CtxPtr is NULL.
 *		- Error codes from the chunked blob export, if a chunk cannot be built.
 *
 * @note	Chunk-0 secrets and digest are left in CtxPtr for the secrets wrap and the
 *		header sign that follow.
 *
 *************************************************************************************************/
static s32 XKeyManager_AekvExportBuildBody(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
					   XKeyManager_ExportCtx *CtxPtr, u64 ExportAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XKeyManager_ExportParams BlobParams = {0U};
	XKeyManager_SecretsCryptArg SecretsArg = {0U};

	if (CtxPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** Engines and key material the trailer wrap borrows on every chunk. */
	SecretsArg.DmaPtr = DmaPtr;
	SecretsArg.AesInstancePtr = AesInstancePtr;
	SecretsArg.KeyMaterial = CtxPtr->SecretKeys;
	SecretsArg.BaseIv = CtxPtr->Header.KekIv;

	/**
	 * Describe the body to the chunk layer: where it goes, how large it is, where its
	 * plaintext comes from and what wraps the secrets a trailer hands to the next chunk.
	 * The chunk-0 secrets and digest come back in the context for the header to carry.
	 */
	BlobParams.BodyAddr = ExportAddr + XKeyManager_GetHeaderSize(XKEYMANAGER_EXPORT_VERSION);
	BlobParams.BodySize = CtxPtr->BodySize;
	BlobParams.PlaintextSize = CtxPtr->PlaintextSize;
	BlobParams.SourceFn = XKeyManager_ReadExportPlaintext;
	BlobParams.SecretsWrapFn = XKeyManager_TrailerSecretsWrap;
	BlobParams.SecretsArgPtr = &SecretsArg;
	BlobParams.WorkPtr = &CtxPtr->Work;
	BlobParams.Chunk0SecretsPtr = CtxPtr->Chunk0Secrets;
	BlobParams.Chunk0HashPtr = CtxPtr->Header.Chunk0Hash;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ChunkBlobExport(DmaPtr, AesInstancePtr,
					     XKEYMANAGER_EXPORT_SESSION.ShaInstancePtr,
					     &BlobParams);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts the chunk-0 IV and KEKs into the AEKV header.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	CtxPtr		Pointer to export context.
 *
 * @return
 *		- XASUFW_SUCCESS, if chunk-0 secrets encryption is successful.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if CtxPtr is NULL.
 *		- Error codes from the piece walk, if a piece cannot be encrypted.
 *		- Error code from XAsufw_UpdateBufStatus, if a key material wipe fails.
 *
 * @note	The keys and the base IV were established before the body was built, because
 *		every chunk trailer wraps the secrets of its successor with the same material.
 *		Chunk 0 is the last wrap of the export, so the key material is cleared here.
 *
 *************************************************************************************************/
static s32 XKeyManager_AekvExportWrapChunk0Secrets(XAsufw_Dma *DmaPtr,
						   XAes *AesInstancePtr,
						   XKeyManager_ExportCtx *CtxPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihAekv = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XKeyManager_SecretsParams SecretsParams = {0U};

	if (CtxPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Chunk 0 has no predecessor to carry its secrets, so they are wrapped here at chunk
	 * index 0 and travel in the header instead of in a trailer.
	 */
	SecretsParams.KeyMaterial = CtxPtr->SecretKeys;
	SecretsParams.BaseIv = CtxPtr->Header.KekIv;
	SecretsParams.Input = CtxPtr->Chunk0Secrets;
	SecretsParams.Output = CtxPtr->Header.Chunk0SecretsEnc;
	SecretsParams.ChunkIndex = 0U;

	/** The wrap writes into the header field itself, inside the span the signature covers. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_WrapSecrets(DmaPtr, AesInstancePtr, &SecretsParams);

END:
	/** This is the last wrap of the export, so the key material is cleared on every path. */
	if (CtxPtr != NULL) {
		XFIH_CALL(Xil_SecureZeroize, XFihAekv, ClearStatus, CtxPtr->SecretKeys,
			  XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
		XFIH_CALL(Xil_SecureZeroize, XFihAekv, ClearStatus, CtxPtr->Chunk0Secrets,
			  XKEYMANAGER_CHUNK_SECRETS_SIZE);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function builds the AEKV blob body and every header field the signature
 *		covers.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ParamsPtr	Pointer to caller-supplied export inputs.
 *
 * @return
 *		- XASUFW_SUCCESS, if the body and the unsigned header are built.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if an argument pointer is NULL, the export
 *		  result address is unset, or the SHA instance cannot be taken.
 *		- XASUFW_KEYMANAGER_AEKV_INVALID_CONTEXT, if the caller's KDF context is all
 *		  zeros.
 *		- XASUFW_MEM_COPY_FAIL, if the caller's KDF context cannot be taken into the
 *		  session.
 *
 * @note	This runs from the key vault overlay image, so it leaves the session for the
 *		resident sequencer rather than finishing the blob: the header it assembles is
 *		complete except for the signature, and the body is already in the export buffer
 *		past where the header will land. The sequencer signs and writes the header once
 *		it has switched to the image the ECC core lives in.
 *
 *************************************************************************************************/
s32 XKeyManager_AekvExportBody(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			       const XKeyManager_AekvExportParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XKeyManager_ExportCtx *CtxPtr = &XKEYMANAGER_EXPORT_SESSION.Ctx;

	if ((DmaPtr == NULL) || (AesInstancePtr == NULL) || (ParamsPtr == NULL) ||
	    (XKEYMANAGER_EXPORT_SESSION.ShaInstancePtr == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** ExportResultAddr is the handler's own response buffer, so only presence is checked. */
	if (ParamsPtr->ExportResultAddr == 0U) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** The eFuse number needs nothing but itself, so it is rejected before a register read. */
	if (ParamsPtr->RevokeEfuseNum >= XASU_KM_KV_REVOKE_EFUSE_COUNT) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ValidateClientBuffer(ParamsPtr->ExportAddr, ParamsPtr->ExportBufSize);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	XKEYMANAGER_EXPORT_SESSION.ExportResultAddr = ParamsPtr->ExportResultAddr;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero((u8 *)(UINTPTR)ParamsPtr->Context,
				      XASU_KM_KV_CONTEXT_SIZE_IN_BYTES);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_KEYMANAGER_AEKV_INVALID_CONTEXT;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(CtxPtr->Context, XASU_KM_KV_CONTEXT_SIZE_IN_BYTES, ParamsPtr->Context,
			     XASU_KM_KV_CONTEXT_SIZE_IN_BYTES, XASU_KM_KV_CONTEXT_SIZE_IN_BYTES);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	CtxPtr->RevokeEfuseNum = ParamsPtr->RevokeEfuseNum;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_GetExportablePlaintextSize(&CtxPtr->PlaintextSize);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_AekvExportPreChecks(ParamsPtr->ExportBufSize,
						 CtxPtr->PlaintextSize,
						 CtxPtr->RevokeEfuseNum);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_AekvExportPartitionBlocks(CtxPtr);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_AekvExportDeriveSecrets(DmaPtr, AesInstancePtr, CtxPtr);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_AekvExportBuildBody(DmaPtr, AesInstancePtr, CtxPtr,
						 ParamsPtr->ExportAddr);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_AekvExportWrapChunk0Secrets(DmaPtr, AesInstancePtr, CtxPtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function rejects a blob whose epoch the caller already revoked. The header
 *		names the off-chip revocation eFuse the blob is bound to, and blowing that eFuse
 *		is what retires every blob carrying it, so the import stops here before any of
 *		the body is touched.
 *
 * @param	CtxPtr	Pointer to import context.
 *
 * @return
 *		- XASUFW_SUCCESS, if the blob epoch is still usable.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if the context pointer is NULL.
 *		- Error codes from the revocation eFUSE read, if the epoch is revoked.
 *
 *************************************************************************************************/
static s32 XKeyManager_AekvImportCheckRevocation(const XKeyManager_ImportCtx *CtxPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	if (CtxPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** The blob names the eFuse it is bound to, and a blown eFuse retires that blob. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_CheckRevokeEfuseUsable(CtxPtr->Header.RevocationCounter);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function derives the wrapping keys and recovers the chunk-0 IV and KEKs.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	CtxPtr		Pointer to import context.
 *
 * @return
 *		- XASUFW_SUCCESS, if chunk-0 secrets decryption is successful.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if CtxPtr is NULL.
 *		- XASUFW_KEYMANAGER_AEKV_WEAK_KEY_MATERIAL, if a derived key is weak.
 *		- Error codes from the eFUSE-0 derivation or the piece walk.
 *
 * @note	The derived keys stay in the import context because every chunk trailer carries
 *		the secrets of its successor wrapped under the same material. The session reset
 *		clears them once the import ends, whichever way it ends.
 *
 *************************************************************************************************/
static s32 XKeyManager_AekvImportUnwrapChunk0Secrets(XAsufw_Dma *DmaPtr,
						     XAes *AesInstancePtr,
						     XKeyManager_ImportCtx *CtxPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XKeyManager_SecretsParams SecretsParams = {0U};

	if (CtxPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Derive the same keys the export used. They come from eFUSE-0 and the caller's context,
	 * so a device or a context that does not match produces different keys and the unwrap
	 * below fails rather than returning wrong secrets.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_DeriveSecretKeys(DmaPtr, AesInstancePtr, CtxPtr->Context,
					      CtxPtr->SecretKeys);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Recover the chunk-0 secrets from the header, which is where the export left them. */
	SecretsParams.KeyMaterial = CtxPtr->SecretKeys;
	SecretsParams.BaseIv = CtxPtr->Header.KekIv;
	SecretsParams.Input = CtxPtr->Header.Chunk0SecretsEnc;
	SecretsParams.Output = CtxPtr->Work.ChunkSecrets;
	SecretsParams.ChunkIndex = 0U;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_UnwrapSecrets(DmaPtr, AesInstancePtr, &SecretsParams);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function authenticates and decrypts every chunk of the body. The digest chain
 *		is seeded with the chunk-0 digest the signed header carries and each chunk is
 *		authenticated against the digest its predecessor named, so the whole body hangs
 *		from the header signature. Blocks past the declared plaintext are padding and
 *		are dropped rather than stored.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	CtxPtr		Pointer to import context.
 * @param	ImportAddr	Source blob address.
 *
 * @return
 *		- XASUFW_SUCCESS, if the chunk body is imported.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if the import context pointer is NULL.
 *		- XASUFW_MEM_COPY_FAIL, if the chunk-0 digest cannot be copied into the chain.
 *
 *************************************************************************************************/
static s32 XKeyManager_AekvImportChunkBody(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
				      XKeyManager_ImportCtx *CtxPtr, u64 ImportAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XKeyManager_ImportParams BlobParams = {0U};
	XKeyManager_SecretsCryptArg SecretsArg = {0U};

	if (CtxPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** The header states the plaintext size, and the body is that size in whole chunks. */
	CtxPtr->PlaintextSize = CtxPtr->Header.KvSize;
	CtxPtr->BodySize = XKeyManager_CalcBodySize(CtxPtr->PlaintextSize);

	/**
	 * Seed the digest chain with the chunk-0 digest the header carries. Each chunk is
	 * authenticated against the digest its predecessor named, so the signed header is what
	 * the whole chain hangs from.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(CtxPtr->Work.ExpectedHash, XKEYMANAGER_HASH_SIZE,
			     CtxPtr->Header.Chunk0Hash, XKEYMANAGER_HASH_SIZE,
			     XKEYMANAGER_HASH_SIZE);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Engines and key material the trailer unwrap borrows on every chunk. */
	SecretsArg.DmaPtr = DmaPtr;
	SecretsArg.AesInstancePtr = AesInstancePtr;
	SecretsArg.KeyMaterial = CtxPtr->SecretKeys;
	SecretsArg.BaseIv = CtxPtr->Header.KekIv;

	/**
	 * Describe the body to the chunk layer: where it starts, how large it is, how many
	 * plaintext bytes it holds and where each decrypted block is stored. The blocks past
	 * the declared plaintext are padding and are dropped rather than written.
	 */
	BlobParams.BodyAddr = ImportAddr + XKeyManager_GetHeaderSize(CtxPtr->Header.Version);
	BlobParams.BodySize = CtxPtr->BodySize;
	BlobParams.MaxPlaintextSize = CtxPtr->PlaintextSize;
	BlobParams.SinkFn = XKeyManager_WriteImportPlaintext;
	BlobParams.SecretsUnwrapFn = XKeyManager_TrailerSecretsUnwrap;
	BlobParams.SecretsArgPtr = &SecretsArg;
	BlobParams.WorkPtr = &CtxPtr->Work;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ChunkBlobImport(DmaPtr, AesInstancePtr,
					     XKEYMANAGER_IMPORT_SESSION.ShaInstancePtr,
					     &BlobParams, &CtxPtr->PlaintextBytesRestored);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs the final validation and rebuilds the vault registry. It checks
 *		the restored byte count against the size the header records and the identification
 *		string and version of the restored vault, then rebuilds the registry entries that
 *		make the imported keys usable.
 *
 * @param	CtxPtr	Pointer to import context.
 *
 * @return
 *		- XASUFW_SUCCESS, if final validation succeeds.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if the context pointer is NULL.
 *		- XASUFW_KEYMANAGER_AEKV_SIZE_MISMATCH, if the restored size differs from the size
 *			the header records.
 *		- XASUFW_KEYMANAGER_INVALID_DDR_ADDRESS, if the vault region is not configured.
 *		- XASUFW_KEYMANAGER_INVALID_ID_STRING, if the restored bytes carry no vault
 *			identification string.
 *		- XASUFW_KEYMANAGER_INVALID_VAULT_VERSION, if the restored vault version is not
 *			the one this firmware builds.
 *		- Error codes from the imported key object validation and the registry rebuild.
 *
 *************************************************************************************************/
static s32 XKeyManager_AekvImportFinalize(const XKeyManager_ImportCtx *CtxPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var IdStringFih = XFIH_FAILURE;
	XFih_Var ExpectedIdStringFih = XFIH_FAILURE;
	XFih_Var MajorVersionFih = XFIH_FAILURE;
	XFih_Var ExpectedMajorFih = XFIH_FAILURE;
	XFih_Var MinorVersionFih = XFIH_FAILURE;
	XFih_Var ExpectedMinorFih = XFIH_FAILURE;
	const XKeyManager_VaultMainHeader *ImportedMainHdrPtr = NULL;
	u8 *VaultBaseAddr = NULL;

	if (CtxPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/** Check that the body restored exactly the number of bytes the header records. */
	if (CtxPtr->PlaintextBytesRestored != CtxPtr->Header.KvSize) {
		Status = XASUFW_KEYMANAGER_AEKV_SIZE_MISMATCH;
		goto END;
	}

	/** Locate the vault region the body was restored into. */
	VaultBaseAddr = XKeyManager_GetVaultBasePtr();
	if (VaultBaseAddr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_DDR_ADDRESS;
		goto END;
	}

	ImportedMainHdrPtr = (const XKeyManager_VaultMainHeader *)VaultBaseAddr;

	/**
	 * These two checks are what decide that the decrypted bytes really are a key vault, so
	 * they are compared redundantly, as a glitch past either would rebuild the registry from
	 * something that is not one.
	 */
	IdStringFih = XFih_VolatileAssignU32(ImportedMainHdrPtr->KeyVaultHeader);
	ExpectedIdStringFih = XFih_VolatileAssignU32(XKEYMANAGER_IDENTIFICATION_STRING);

	ASSIGN_VOLATILE(Status, XASUFW_KEYMANAGER_INVALID_ID_STRING);
	XFIH_IF_FAILIN_NEQ(IdStringFih, ExpectedIdStringFih) {
		XFIH_GOTO(END);
	}

	/** Compare the vault version the blob carries with the one this firmware builds. */
	MajorVersionFih = XFih_VolatileAssignU32((u32)ImportedMainHdrPtr->MajorVersion);
	ExpectedMajorFih = XFih_VolatileAssignU32((u32)XKEYMANAGER_MAJOR_VERSION);

	ASSIGN_VOLATILE(Status, XASUFW_KEYMANAGER_INVALID_VAULT_VERSION);
	XFIH_IF_FAILIN_NEQ(MajorVersionFih, ExpectedMajorFih) {
		XFIH_GOTO(END);
	}

	MinorVersionFih = XFih_VolatileAssignU32((u32)ImportedMainHdrPtr->MinorVersion);
	ExpectedMinorFih = XFih_VolatileAssignU32((u32)XKEYMANAGER_MINOR_VERSION);

	ASSIGN_VOLATILE(Status, XASUFW_KEYMANAGER_INVALID_VAULT_VERSION);
	XFIH_IF_FAILIN_NEQ(MinorVersionFih, ExpectedMinorFih) {
		XFIH_GOTO(END);
	}

	/** Validate every key object the restored vault holds. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ValidateImportedKeyObjects();
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** Rebuild the vault registry from the restored region. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_RebuildVaultRegistryFromImport();
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/**
	 * The restored vault matches the blob the caller holds, so it starts out unchanged, but
	 * saying so is left to the command once the session teardown has succeeded. Marking it
	 * here would leave a clean vault behind a command that then reports an error.
	 */
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function restores the key vault from the body of an authenticated AEKV blob.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	ParamsPtr	Pointer to caller-supplied import inputs.
 *
 * @return
 *		- XASUFW_SUCCESS, if the vault is restored.
 *		- XASUFW_KEYMANAGER_INVALID_PARAM, if an argument pointer is NULL or the import
 *		  address disagrees with the one staged in the session context.
 *
 * @note	This runs from the key vault overlay image and takes over once the resident
 *		sequencer has staged the header, checked its structure and verified its
 *		signature, so the blob is already known to be one this device exported. Every
 *		failure past the region wipe is reverted here rather than returned outright,
 *		because the wipe destroys the vault the caller had.
 *
 *************************************************************************************************/
s32 XKeyManager_AekvImportBody(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			       const XKeyManager_AekvImportParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XKeyManager_ImportCtx *CtxPtr = &XKEYMANAGER_IMPORT_SESSION.Ctx;

	if ((DmaPtr == NULL) || (AesInstancePtr == NULL) || (ParamsPtr == NULL)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	if (ParamsPtr->ImportAddr != CtxPtr->ImportAddr) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_AekvImportCheckRevocation(CtxPtr);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/**
	 * An import replaces the user vaults wholesale, so it starts only on an empty region.
	 * The size that region can restore is checked against the header while the blob
	 * structure is validated, so it is not queried a second time here.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_IsImportRegionEmpty();
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/**
	 * Preparing the region wipes the user vaults and the registry entries describing them, so
	 * every failure from here on is reverted rather than returned outright.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_PrepareImportRegion(DmaPtr);
	if (XASUFW_SUCCESS != Status) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_AekvImportUnwrapChunk0Secrets(DmaPtr, AesInstancePtr, CtxPtr);
	if (XASUFW_SUCCESS != Status) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_AekvImportChunkBody(DmaPtr, AesInstancePtr, CtxPtr,
					    ParamsPtr->ImportAddr);
	if (XASUFW_SUCCESS != Status) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_AekvImportFinalize(CtxPtr);

END_CLR:
	/** An import that got past the region preparation and then failed is reverted. */
	if (XASUFW_SUCCESS != Status) {
		ClearStatus = XKeyManager_RevertImportRegion(DmaPtr);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}

END:
	return Status;
}

#endif /* XASU_KEYMANAGER_ENABLE */
/** @} */
