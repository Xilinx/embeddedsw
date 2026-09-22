/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xkeymanager_aekv.h
 *
 * This file contains the AEKV key-rolling format constants, the header layout and the export and
 * import APIs that xkeymanager_aekv.c implements. AEKV stands for Authenticated Encrypted Key
 * Vault, the portable blob a key vault export produces and an import consumes.
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
#ifndef XKEYMANAGER_AEKV_H_
#define XKEYMANAGER_AEKV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif
#include "xasufw_dma.h"
#include "xasufw_memory.h"
#include "xasufw_util.h"
#include "xaes.h"
#include "xkeymanager_chunk.h"
#include "xasu_keymanagerinfo.h"

#ifdef XASU_KEYMANAGER_ENABLE
/**
 * AEKV signs and verifies the header with the UDE key through OCP, and derives the keys that wrap
 * the secrets of every chunk with the CMAC KDF over the eFUSE-0 seed. A build without OCP still
 * compiles and registers the commands, and export and import then reject the request with
 * XASUFW_KEYMANAGER_OCP_NOT_ENABLED.
 */

/************************************ Constant Definitions ***************************************/
/**
 * The blob tag, the format version, the KDF context size and the revoke eFUSE count describe a
 * blob a caller stores and hands back, so xasu_keymanagerinfo.h publishes them and this file uses
 * those definitions rather than restating the values. The signed region is never read by a caller,
 * so its geometry is stated here alone.
 */

/** Oldest AEKV format version an import accepts. */
#define XKEYMANAGER_IMPORT_MIN_VERSION		XASU_KM_AEKV_VERSION_1

/** AEKV format version an export writes, which is also the newest an import accepts. */
#define XKEYMANAGER_EXPORT_VERSION		XASU_KM_AEKV_VERSION_1

/** Chunk-0 digest field size in the header. */
#define XKEYMANAGER_HEADER_HASH_SIZE			(48U)

/** ECC P-384 signature size (R || S), the signature field of a version 1 header. */
#define XKEYMANAGER_SIGNATURE_SIZE_P384		(96U)

/** IV increment that selects the eFUSE-0 black key the KDF seed is decrypted from. */
#define XKEYMANAGER_KDF_IV_INC_VAL			(0x06U)

/**
 * Byte size of the scratch arm of the shared storage, which is what sizes the storage as a whole
 * rather than the session. Provisioning stages its working data there, and asks for more room
 * than an export or an import needs, so the arm is stated here rather than derived.
 */
#define XKEYMANAGER_SHARED_SCRATCH_SIZE		(18368U)

/************************************** Type Definitions *****************************************/
/** Caller-supplied inputs of one AEKV export. */
typedef struct {
	u64 ExportAddr;		/**< Destination address of the blob. */
	u32 ExportBufSize;	/**< Destination buffer capacity in bytes. */
	u32 ExportResultAddr;	/**< Address of the export result block in the response buffer. */
	u32 RevokeEfuseNum;	/**< Off-chip revocation eFUSE number bound to the blob. */
	u32 SubsystemId;	/**< Identity of the calling subsystem. */
	u8 Context[XASU_KM_KV_CONTEXT_SIZE_IN_BYTES];	/**< KDF context, held by value so the
							     caller cannot change it between the
							     check and the use. */
} XKeyManager_AekvExportParams;

/** Caller-supplied inputs of one AEKV import. */
typedef struct {
	u64 ImportAddr;		/**< Source address of the blob. */
	u32 ImportSize;		/**< Blob size in bytes. */
	u32 SubsystemId;	/**< Identity of the calling subsystem. */
	u8 Context[XASU_KM_KV_CONTEXT_SIZE_IN_BYTES];	/**< KDF context the blob was exported
							     under, held by value so the caller
							     cannot change it between the check
							     and the use. */
} XKeyManager_AekvImportParams;

/**
 * AEKV header layout Version 1 with N = 16 and a P-384 signature.
 *
 * Everything ahead of the signature is version independent, so a new version changes only the
 * signature field and, through it, the header size. A future ML-DSA-87 version therefore keeps
 * the signature at offset 608 and widens the field to 4627 bytes for a 5235 byte header. The
 * Version field is what selects the algorithm the signature is verified with, so an importer
 * reads Version first and derives the header size and the signed length from it rather than
 * assuming V1.
 */
typedef struct {
	u32 Tag;					/**< XASU_KM_AEKV_TAG */
	u32 Version;					/**< AEKV format version */
	u32 RevocationCounter;				/**< Off-chip revocation eFUSE number */
	u32 KvSize;					/**< Plaintext key vault size in bytes */
	u8 KekIv[XKEYMANAGER_AES_IV_SIZE];		/**< TRNG base IV of chunk-0 secrets */
	u8 Chunk0SecretsEnc[XKEYMANAGER_CHUNK_SECRETS_SIZE]; /**< Encrypted chunk-0 IV and KEKs */
	u8 Chunk0Hash[XKEYMANAGER_HEADER_HASH_SIZE];	/**< SHA-384 digest of chunk-0 ciphertext */
	u8 Signature[XKEYMANAGER_SIGNATURE_SIZE_P384];	/**< UDE P-384 signature */
} XKeyManager_HeaderV1;

/**
 * The header is a persisted wire format shared with every device that imports the blob, so the
 * offsets below are fixed and cannot follow the compiler. Padding introduced by a field reorder
 * or a type change would silently shift the signed region and every offset after it, so pin the
 * whole layout here. The fields a caller reads are pinned to the offsets the client header
 * publishes, so a change on either side stops the build rather than the import.
 */
_Static_assert(offsetof(XKeyManager_HeaderV1, Tag) == XASU_KM_AEKV_HDR_OFFSET_TAG,
	       "AEKV header Tag must be where the client header says");
_Static_assert(offsetof(XKeyManager_HeaderV1, Version) == XASU_KM_AEKV_HDR_OFFSET_VERSION,
	       "AEKV header Version must be where the client header says");
_Static_assert(offsetof(XKeyManager_HeaderV1, RevocationCounter) ==
	       XASU_KM_AEKV_HDR_OFFSET_REV_COUNTER,
	       "AEKV header RevocationCounter must be where the client header says");
_Static_assert(offsetof(XKeyManager_HeaderV1, KvSize) == XASU_KM_AEKV_HDR_OFFSET_KV_SIZE,
	       "AEKV header KvSize must be where the client header says");
_Static_assert(offsetof(XKeyManager_HeaderV1, KekIv) == 16U,
	       "AEKV header KekIv must be at offset 16");
_Static_assert(offsetof(XKeyManager_HeaderV1, Chunk0SecretsEnc) == 32U,
	       "AEKV header Chunk0SecretsEnc must be at offset 32");
_Static_assert(offsetof(XKeyManager_HeaderV1, Chunk0Hash) == (32U + XKEYMANAGER_CHUNK_SECRETS_SIZE),
	       "AEKV header Chunk0Hash must follow the chunk-0 secrets");
_Static_assert(offsetof(XKeyManager_HeaderV1, Signature) == (80U + XKEYMANAGER_CHUNK_SECRETS_SIZE),
	       "AEKV header Signature must follow the chunk-0 digest");
_Static_assert(sizeof(XKeyManager_HeaderV1) == (80U + XKEYMANAGER_CHUNK_SECRETS_SIZE +
					  XKEYMANAGER_SIGNATURE_SIZE_P384),
	       "AEKV header must have no trailing padding");

/** The header carries the chunk-0 digest, so its field must be the chunk digest size. */
_Static_assert(XKEYMANAGER_HEADER_HASH_SIZE == XKEYMANAGER_HASH_SIZE,
	       "AEKV header digest field must match the chunk digest size");

/** The pieces have to cover the chunk-0 secrets exactly, with nothing left unwrapped. */
_Static_assert(XKEYMANAGER_CHUNK_SECRETS_SIZE == (XKEYMANAGER_CHUNK_IV_SIZE +
						 (XKEYMANAGER_SECRETS_GROUP_COUNT *
						  XKEYMANAGER_SECRETS_GROUP_SIZE)),
	       "AEKV secrets pieces must cover the chunk IV and every unit KEK");

/** Each piece is a whole number of AES blocks, as each one is a CBC operation of its own. */
_Static_assert((XKEYMANAGER_SECRETS_GROUP_SIZE % XASU_AES_BLOCK_SIZE_IN_BYTES) == 0U,
	       "AEKV secrets group size must be a multiple of the AES block length");

/** AEKV export working context. */
typedef struct {
	u32 PlaintextSize;	/**< Packed exportable plaintext size, the header KV_SIZE. */
	u32 BodySize;		/**< Total chunk bytes after the header. */
	u32 BlobSize;		/**< Header plus chunk bytes. */
	u32 RevokeEfuseNum;	/**< Off-chip revocation eFUSE number the caller bound. */
	u32 ModifiedValue;	/**< XASU_KM_EXPORT_MODIFIED when the vault changed since the last
				     export, XASU_KM_EXPORT_NOT_MODIFIED otherwise. The value the
				     caller is given is carried from the moment it is read, so no
				     one-bit flag holds the answer while the export runs. */
	u8 Context[XASU_KM_KV_CONTEXT_SIZE_IN_BYTES];		/**< Caller KDF context. */
	u8 SecretKeys[XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE]; /**< Keys wrapping the chunk-0
							     secrets. */
	u8 Chunk0Secrets[XKEYMANAGER_CHUNK_SECRETS_SIZE]; /**< Chunk-0 IV and KEKs. The wrap
							     writes its result into the header
							     field, so no staged copy is kept. */
	XKeyManager_ExportWorkBuf Work;			/**< Working buffers of the chunk
							     build. */
	XKeyManager_HeaderV1 Header;				/**< Blob header. */
} XKeyManager_ExportCtx;

/** Export session holding the large export context and the parameters of the running export. */
typedef struct {
	XKeyManager_ExportCtx Ctx;		/**< Export working context. */
	u32 ExportResultAddr;		/**< Address of the export result block. */
	u32 SubsystemId;		/**< Identity of the calling subsystem. */
	XSha *ShaInstancePtr;		/**< Pointer to the XSha instance. */
} XKeyManager_ExportSession;

/** AEKV import working context. */
typedef struct {
	u64 ImportAddr;			/**< Source address of the blob. */
	u32 ImportSize;			/**< Blob size in bytes. */
	u32 PlaintextSize;		/**< Plaintext vault size the header declares. */
	u32 BodySize;			/**< Total chunk bytes after the header. */
	u32 PlaintextBytesRestored;	/**< Plaintext bytes written to the vault region. */
	u8 Context[XASU_KM_KV_CONTEXT_SIZE_IN_BYTES];		/**< Caller KDF context. */
	u8 SecretKeys[XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE]; /**< Keys wrapping the chunk-0
							     secrets. */
	XKeyManager_WorkBuf Work;			/**< Working buffers of the chunk walk. */
	XKeyManager_HeaderV1 Header;				/**< Blob header. */
} XKeyManager_ImportCtx;

/** Import session holding the large import context and the parameters of the running import. */
typedef struct {
	XKeyManager_ImportCtx Ctx;		/**< Import working context. */
	u32 SubsystemId;		/**< Identity of the calling subsystem. */
	XSha *ShaInstancePtr;		/**< Pointer to the XSha instance. */
} XKeyManager_ImportSession;

/**
 * Export and import are separate commands that never run at the same time, so the two sessions
 * share one allocation instead of each reserving several kilobytes for the life of the firmware.
 */
typedef union {
	XKeyManager_ExportSession Export;	/**< Export session state. */
	XKeyManager_ImportSession Import;	/**< Import session state. */
} XKeyManager_Session;

/**
 * The shared pool of the crypto algorithm data block seen as one allocation. An export or an
 * import places its session here; provisioning stages its working data in the scratch arm. Every
 * user of the pool holds XASUFW_OVERLAY for its whole duration, which is what the pool asks of
 * all of them, so no two arms are ever live together.
 */
typedef union {
	XKeyManager_Session AekvSession;		/**< Key vault export or import session. */
	u8 Scratch[XKEYMANAGER_SHARED_SCRATCH_SIZE];	/**< Provisioning scratch buffer. */
} XKeyManager_SharedStorage;

/**
 * The pool is a fixed reservation the linker never fills, so an overrun would only show up as
 * silent damage to the auxiliary buffer past its end. Check the fit here instead.
 */
_Static_assert(sizeof(XKeyManager_SharedStorage) <= XASUFW_CRYPTO_SHARED_POOL_SIZE,
	       "shared storage exceeds the crypto shared pool");

/*************************** Macros (Inline Functions) Definitions *******************************/
/**
 * The session, at the base of the crypto algorithm data block. It lives in the shared pool rather
 * than in .bss, which the resident instruction RAM has no room for.
 */
#define XKEYMANAGER_SESSION	(&((XKeyManager_SharedStorage *)(UINTPTR) \
				   XASUFW_CRYPTO_ALGO_DATA_BLOCK)->AekvSession)

/** Byte size of the version 1 AEKV blob header. */
#define XKEYMANAGER_HEADER_V1_SIZE			((u32)sizeof(XKeyManager_HeaderV1))

/** Byte length of AEKV header fields covered by the UDE signature. */
#define XKEYMANAGER_HEADER_SIGN_LEN			(XKEYMANAGER_HEADER_V1_SIZE - \
							 XKEYMANAGER_SIGNATURE_SIZE_P384)

/**
 * The UDE sign and verify APIs hash the header in one blocking DMA transfer, so a header that
 * outgrew that window would be rejected rather than signed. Catch such a growth at build time.
 */
_Static_assert(XKEYMANAGER_HEADER_SIGN_LEN <= XASUFW_DMA_BLOCKING_SIZE,
	       "Signed AEKV header length must fit the blocking DMA window");

/** Highest OFFCHIP_REVOKE register index holding epoch bits. */
#define XKEYMANAGER_MAX_REVOKE_REG_INDEX		(7U)

/** Epoch bits one OFFCHIP_REVOKE register holds. */
#define XKEYMANAGER_REVOKE_BITS_PER_REG			(32U)

/**
 * An eFUSE number the export accepts has to land in a register the read walks, so the count the
 * client publishes and the register window this file walks describe the same eFUSE range.
 */
_Static_assert(XASU_KM_KV_REVOKE_EFUSE_COUNT ==
	       ((XKEYMANAGER_MAX_REVOKE_REG_INDEX + 1U) * XKEYMANAGER_REVOKE_BITS_PER_REG),
	       "Revoke eFuse count must cover exactly the OFFCHIP_REVOKE registers walked");

/** Shift that converts a word index into a byte offset. */
#define XKEYMANAGER_WORD_SHIFT			(2U)

/*************************************************************************************************/
/**
 * @brief	This function returns the header size the given AEKV format version writes.
 *
 * @param	Version		AEKV format version taken from the blob or from the export.
 *
 * @return
 *		- Header size in bytes of that version.
 *		- 0, if the version is outside the supported range.
 *
 * @note	The version alone fixes the header size, because it also fixes the signature
 *		algorithm and every field ahead of the signature is version independent.
 *
 *		This and the signed length below are stated here rather than in
 *		xkeymanager_aekv.c because the resident sequencer sizes and verifies a header
 *		before it activates the overlay that file is linked into.
 *
 *************************************************************************************************/
static inline u32 XKeyManager_GetHeaderSize(u32 Version)
{
	u32 HeaderSize = 0U;

	if (Version == XASU_KM_AEKV_VERSION_1) {
		HeaderSize = XKEYMANAGER_HEADER_V1_SIZE;
	}

	return HeaderSize;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the length of the header fields the signature of the given
 *		AEKV format version covers.
 *
 * @param	Version		AEKV format version taken from the blob or from the export.
 *
 * @return
 *		- Signed length in bytes of that version.
 *		- 0, if the version is outside the supported range.
 *
 *************************************************************************************************/
static inline u32 XKeyManager_GetHeaderSignLen(u32 Version)
{
	u32 SignLen = 0U;

	if (Version == XASU_KM_AEKV_VERSION_1) {
		SignLen = XKEYMANAGER_HEADER_SIGN_LEN;
	}

	return SignLen;
}

/************************************ Function Prototypes ****************************************/
/**
 * The two entry points of the key vault overlay image. XKeyManager_ExportKeyVault() and
 * XKeyManager_ImportKeyVault() call these with that image active, and hold the signature work on
 * their own side of the switch, so the format layer never reaches the ECC core itself.
 */
s32 XKeyManager_AekvExportBody(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			       const XKeyManager_AekvExportParams *ParamsPtr);
s32 XKeyManager_AekvImportBody(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			       const XKeyManager_AekvImportParams *ParamsPtr);
#endif /* XASU_KEYMANAGER_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XKEYMANAGER_AEKV_H_ */
/** @} */
