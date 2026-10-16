/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xprovision.c
*
* This file implements provisioning of the USER eFUSE keys. The key comes from one of three sources
* selected via KeySource: ASU-generated (TRNG), AKV vault (KeyId), or user buffer (KeyAddr).
* USER_KEY_0 allows only the ASU-generated source; USER_KEY_1 allows all three. USER_KEY_0 is
* always PUF KEK encrypted (black form). USER_KEY_1 is written in the form selected by KeyForm.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kp   06/27/26 Initial release
*       vm   09/15/26 Dropped the UDE public key cache after a UDE slot write
*       kp   09/25/26 Program USER eFUSE key 1 red/black from KeyForm; reverse key before
*                     eFUSE write (not PMC's format)
*
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup xprovision_server_apis Provisioning Server APIs
 * @{
 */
/************************************** Include Files ********************************************/
#include "xprovision.h"
#include "xasu_provision_common.h"
#include "xasufw_hw.h"
#include "xasufw_dma.h"
#include "xasufw_memory.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_debug.h"
#include "xasufw_init.h"
#include "xasufw_keytransfer.h"
#include "xasufw_ipi.h"
#include "xaes.h"
#include "xtrng.h"
#include "xrsa_ecc.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xkeymanager.h"
#include "xkeymanager_aekv.h"
#include "xkeymanager_chunk.h"
#include "xkeymanager_crypto.h"
#include "xasu_sharedmem.h"
#include "xasu_aesinfo.h"
#include "xasu_eccinfo.h"
#include "xasu_kdfinfo.h"
#include "xasu_keymanagerinfo.h"
#include "xasu_shainfo.h"
#include "xasu_def.h"
#include "xasu_generic.h"
#include "xocp_ude.h"
#include "xecies.h"
#include "xkdf.h"
#include "xfih.h"
#include "xoverlay.h"
#include "xil_sutil.h"

#ifdef XASU_PROVISION_ENABLE
/********************************** Constant Definitions *****************************************/

/************************************ Macro Definitions ******************************************/
#define XPROVISION_EFUSE_KEY_SIZE_IN_BYTES	(32U)	/**< USER eFUSE key size in bytes
							(256-bit) */
#define XPROVISION_EFUSE_IV_SIZE_IN_BYTES	(12U)	/**< IV size in bytes for the black key
							AES-GCM operation (96-bit) */
#define XPROVISION_EFUSE_TAG_SIZE_IN_BYTES	(16U)	/**< GCM tag size in bytes */

#define XPROVISION_UDE_PVT_KEY_LEN_IN_WORDS	(XASU_PROVISION_UDE_PVT_KEY_SIZE_IN_BYTES >> 2U)
							/**< UDE private key length in words */
#define XPROVISION_UDE_KEY_TYPE_CODE_UNPROGRAMMED	(0x3U)	/**< 2-bit UDE key type code:
								slot unprogrammed (matches the
								all-slots-unprogrammed default) */

#define XPROVISION_UDE_CLEAR_CHUNK_SIZE_IN_BYTES	(64U)	/**< Chunk size used to clear the
								caller certificate buffer during
								provisioning rollback */

/*
 * Device Provisioning Certificate (DPC) container layout (SAS Table 18): Device DNA, followed by
 * the length-prefixed UDE-signed authentication key certificate, the length-prefixed UDE-signed
 * wrapping key certificate, and a UDE signature over all of the preceding bytes.
 */
#define XPROVISION_DPC_DNA_SIZE_IN_BYTES	(EFUSE_CACHE_DNA_SIZE)	/**< Device DNA size in
									bytes. */
#define XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES	(4U)	/**< Size of each certificate length prefix
								(one u32) in bytes. */
#define XPROVISION_DPC_SIG_SIZE_IN_BYTES	(XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES * 2U)
							/**< UDE signature size in bytes (R || S over
							NIST P-384). */

#ifdef XASU_ECIES_ENABLE
#define XPROVISION_MAX_PI_SIZE		(XASU_PROVISION_IMAGE_MAX_RECORD_BLOB_LEN)
						/**< Largest PI_SIZE whose expanded AEPI image fits
							in the u32 image-length fields. */

/*
 * Wrapped provisioning IMAGE HEADER layout (byte offsets from the image base) and the fixed crypto
 * profile used by XProvision_UnwrapInstructions. All values must match the sender/host.
 */
#define XPROVISION_HDR_TAG_OFFSET		(0U)	/**< 'AEPI' tag offset */
#define XPROVISION_HDR_TAG_BYTE_0_OFFSET	(XPROVISION_HDR_TAG_OFFSET) /**< Tag byte 0 offset */
#define XPROVISION_HDR_TAG_BYTE_1_OFFSET	(XPROVISION_HDR_TAG_BYTE_0_OFFSET + 1U)
							/**< Tag byte 1 offset */
#define XPROVISION_HDR_TAG_BYTE_2_OFFSET	(XPROVISION_HDR_TAG_BYTE_1_OFFSET + 1U)
							/**< Tag byte 2 offset */
#define XPROVISION_HDR_TAG_BYTE_3_OFFSET	(XPROVISION_HDR_TAG_BYTE_2_OFFSET + 1U)
							/**< Tag byte 3 offset */
#define XPROVISION_HDR_TAG_LEN			(4U)	/**< Tag length in bytes */
#define XPROVISION_HDR_VERSION_OFFSET		(XPROVISION_HDR_TAG_OFFSET + \
						 XPROVISION_HDR_TAG_LEN) /**< Version (u32) offset */
#define XPROVISION_HDR_ALGTYPE_OFFSET		(8U)	/**< ALG_TYPE (u32) offset */
#define XPROVISION_HDR_ECIES_PUB_OFFSET		(12U)	/**< ECIES P-384 Tx public key offset */
#define XPROVISION_HDR_ECIES_TAG_OFFSET		(108U)	/**< ECIES MAC/tag offset */
#define XPROVISION_HDR_CIPHERTEXT_OFFSET	(124U)	/**< ECIES ciphertext (wrapped seed) */
#define XPROVISION_HDR_PI_SIZE_OFFSET		(156U)	/**< PI blob size (u32) offset */
#define XPROVISION_HDR_IV_OFFSET		(160U)	/**< AES-CBC IV offset (16 bytes) */
#define XPROVISION_HDR_KEKS_OFFSET		(176U)	/**< CHUNK_0_IV_KEKS offset */
#define XPROVISION_HDR_KEKS_LEN			(528U)	/**< CHUNK_0_IV_KEKS length (16B inner
							IV + 16 x 32B keys) */
#define XPROVISION_HDR_CHUNK0_HASH_OFFSET	(704U)	/**< Chunk-0 hash offset */
#define XPROVISION_HDR_CHUNK0_HASH_LEN		(XASU_SHA_384_HASH_LEN)
							/**< Chunk-0 hash length (SHA2-384) */
#define XPROVISION_HDR_SIGNATURE_OFFSET		(752U)	/**< Signature offset */

#define XPROVISION_HDR_VERSION_EXPECTED		(0x00000001U)	/**< Only accepted header version */
#define XPROVISION_HDR_ALGTYPE_ECIES		(0x0U)	/**< ALG_TYPE value denoting ECIES
							(the only algorithm accepted now).
							TODO: add ML-KEM branch later. */
#define XPROVISION_HDR_SIG_LEN_P384		(96U)	/**< ECC P-384 signature length (R||S) */
#define XPROVISION_HDR_MIN_TOTAL_LEN		(XPROVISION_HDR_SIGNATURE_OFFSET + \
						 XPROVISION_HDR_SIG_LEN_P384)
							/**< Minimum image length through the signature. */
#define XPROVISION_HDR_SEED_LEN			(32U)	/**< ECIES-recovered seed length */
#define XPROVISION_HDR_HASH_LEN			(XASU_SHA_384_HASH_LEN)
						/**< SHA2-384 header digest length */

_Static_assert(XPROVISION_HDR_SIGNATURE_OFFSET <= XASUFW_DMA_BLOCKING_SIZE,
	       "AEPI header digest requires non-blocking SHA handling");
_Static_assert((XPROVISION_HDR_CIPHERTEXT_OFFSET + XPROVISION_HDR_SEED_LEN) ==
	       XPROVISION_HDR_PI_SIZE_OFFSET, "wrapped seed does not fill the ciphertext field");
#define XPROVISION_HDR_AES_IV_LEN_IN_BYTES	(16U)	/**< AES-CBC IV length in bytes */

/* CMAC-KDF context for the header path. */
#define XPROVISION_HDR_KDF_CONTEXT	"Authenticated Encrypted Provisioning Instructions"
#define XPROVISION_HDR_KDF_CONTEXT_LEN		(49U)	/**< CMAC-KDF header context length */

/* Provisioning instruction record layout and instruction command IDs (see command formats CSV). */
#define XPROVISION_INSTR_HDR_LEN		(8U)	/**< Per-instruction record header length */
#define XPROVISION_INSTR_MODULE_OFFSET		(0U)	/**< Module ID byte offset in a record */
#define XPROVISION_INSTR_CMDID_OFFSET		(1U)	/**< Command ID byte offset in a record */
#define XPROVISION_INSTR_SIZE_OFFSET		(4U)	/**< Payload size (u16 LE) offset */
#define XPROVISION_INSTR_STORE_META_LEN		(16U)	/**< STORE_KEY metadata size */
#define XPROVISION_INSTR_STORE_DATA_OFFSET	(24U)	/**< STORE_KEY key-data offset */
#define XPROVISION_INSTR_STORE_META_OFFSET	(8U)	/**< STORE_KEY metadata offset */
#define XPROVISION_INSTR_XOR_PAYLOAD_LEN	(24U)	/**< XOR payload size */

/*
 * XOR field offsets, counted from the start of the record (its 8-byte header). The three key IDs
 * are 4-byte composite key IDs here, unlike the 2-byte slot STORE_KEY carries.
 */
#define XPROVISION_XOR_KEYID1_OFFSET		(8U)	/**< Input key split 1 ID (u32 LE) */
#define XPROVISION_XOR_KEYID2_OFFSET		(12U)	/**< Input key split 2 ID (u32 LE) */
#define XPROVISION_XOR_OUT_KEYID_OFFSET		(16U)	/**< Combined key ID (u32 LE) */
#define XPROVISION_XOR_USECASE_OFFSET		(20U)	/**< Combined key use case (u8) */
#define XPROVISION_XOR_ATTRIBUTES_OFFSET	(21U)	/**< Combined key attributes (u8) */
#define XPROVISION_XOR_LENGTH_OFFSET		(22U)	/**< Combined key length (u16 LE) */
#define XPROVISION_XOR_EPOCHTIME_OFFSET		(24U)	/**< Combined key epoch time (u32 LE) */
#define XPROVISION_XOR_USAGECOUNT_OFFSET	(28U)	/**< Combined key usage count (u32 LE) */

/**
 * Record window: the largest single provisioning instruction record, which is STORE_KEY of an
 * ML-DSA-87 private key. Records are assembled one at a time, so this bounds the plaintext that
 * is resident at any instant and is also the hard bound a declared record length is checked
 * against.
 */
#define XPROVISION_RECBUF_LEN			(XASU_PROVISION_MAX_RECORD_SIZE)

_Static_assert(XPROVISION_RECBUF_LEN == XASU_PROVISION_MAX_RECORD_SIZE,
	       "client and server provisioning record windows disagree");
_Static_assert((XPROVISION_INSTR_HDR_LEN + XPROVISION_INSTR_STORE_META_LEN +
		XASU_MLDSA_MAX_PRIV_KEY_LEN) <= XPROVISION_RECBUF_LEN,
	       "STORE_KEY record exceeds the provisioning record window");

/*
 * Layout of the two ASU-internal working buffers this path needs inside the key manager's shared
 * session storage: the chunk import working set followed by the record window. They are borrowed
 * rather than reserved here because a key vault export or import never runs while a provisioning
 * image is being processed, so the uses cannot overlap, and the borrow keeps both buffers out of
 * the resident data region.
 */
#define XPROVISION_SCRATCH_LEN			(sizeof(XKeyManager_WorkBuf) + \
						XPROVISION_RECBUF_LEN) /**< Scratch bytes this
									path needs */

_Static_assert(XPROVISION_SCRATCH_LEN <= XKEYMANAGER_SHARED_SCRATCH_SIZE,
	       "provisioning import working set exceeds the shared scratch region");
_Static_assert(XPROVISION_HDR_MIN_TOTAL_LEN <= XPROVISION_RECBUF_LEN,
	       "AEPI header exceeds the reusable record window");

/*
 * The header carries the chunk-0 secrets and digest straight into the import working set, so the
 * two wire fields must be exactly what the chunk layer expects to find there.
 */
_Static_assert(XPROVISION_HDR_KEKS_LEN == XKEYMANAGER_CHUNK_SECRETS_SIZE,
	       "Header CHUNK_0_IV_KEKS must be one chunk secrets block");
_Static_assert(XPROVISION_HDR_CHUNK0_HASH_LEN == XKEYMANAGER_HASH_SIZE,
	       "Header chunk-0 hash must be one chunk digest");
_Static_assert(XPROVISION_HDR_AES_IV_LEN_IN_BYTES == XKEYMANAGER_AES_IV_SIZE,
	       "Header IV must be the base IV the secrets piece walk advances");
_Static_assert(XPROVISION_HDR_MIN_TOTAL_LEN == XASU_PROVISION_IMAGE_HEADER_SIZE,
	       "server and client AEPI header sizes disagree");
_Static_assert(XKEYMANAGER_CHUNK_SIZE == XASU_PROVISION_IMAGE_CHUNK_SIZE,
	       "server and client AEPI chunk sizes disagree");
_Static_assert((XKEYMANAGER_DATA_BLOCKS_PER_CHUNK * XKEYMANAGER_BLOCK_SIZE) ==
	       XASU_PROVISION_IMAGE_CHUNK_DATA_SIZE,
	       "server and client AEPI chunk payload sizes disagree");
_Static_assert(XPROVISION_MAX_PI_SIZE == XASU_PROVISION_IMAGE_MAX_RECORD_BLOB_LEN,
	       "server and client AEPI plaintext limits disagree");
#endif /* XASU_ECIES_ENABLE */

/************************************ Type Definitions *******************************************/

#ifdef XASU_ECIES_ENABLE
/**
 * State the chunk secrets unwrap needs. The KDF derives one key per secrets piece, so that key
 * material travels here with the AES plumbing and base IV; no key persists in a key register.
 */
typedef struct {
	XAsufw_Dma *DmaPtr;			/**< DMA instance used by the CBC operation. */
	XAes *AesInstancePtr;			/**< AES instance used by the CBC operation. */
	const u8 *KeyMaterial;			/**< Derived keys, one per secrets piece. */
	u64 BaseIvAddr;				/**< Header IV, which every piece IV derives from. */
} XProvision_SecretsCtx;

/** Caller record stream published for the span of one chunk export. */
typedef struct {
	XAsufw_Dma *DmaPtr;	/**< DMA instance used to read the caller's record stream. */
	u64 RecordBlobAddr;	/**< Address of the caller's plaintext record stream. */
	u32 RecordBlobLen;	/**< Number of bytes the source may expose. */
} XProvision_SourceCtx;

/** Transient wrap material borrowed from the key manager shared scratch region. */
typedef struct {
	XKeyManager_ExportWorkBuf Work;				/**< Chunk export working set. */
	u8 Chunk0Secrets[XKEYMANAGER_CHUNK_SECRETS_SIZE];	/**< Plain chunk-0 secrets. */
	u8 KeyMaterial[XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE];	/**< CMAC-KDF output. */
	u8 Seed[XPROVISION_HDR_SEED_LEN];			/**< ECIES plaintext seed. */
	u8 WrapPubKey[XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES];	/**< ECIES recipient key. */
	u8 BaseIv[XPROVISION_HDR_AES_IV_LEN_IN_BYTES];		/**< Chunk-secrets base IV. */
	u8 HeaderHash[XPROVISION_HDR_HASH_LEN];			/**< Header SHA2-384 digest. */
} XProvision_WrapScratch;

_Static_assert(sizeof(XProvision_WrapScratch) <= XKEYMANAGER_SHARED_SCRATCH_SIZE,
	       "AEPI wrap working set exceeds the shared scratch region");
_Static_assert(XPROVISION_HDR_MIN_TOTAL_LEN <=
	       sizeof(((XProvision_WrapScratch *)0)->Work.ChunkBuf),
	       "AEPI header exceeds the reusable chunk buffer");

/**
 * State the streaming record executor carries between sink calls. One record is buffered at a
 * time; a record executes the moment its last byte arrives.
 */
typedef struct {
	XAsufw_Dma *DmaPtr;		/**< DMA instance handed to the record executors. */
	XAes *AesInstancePtr;		/**< AES instance handed to the record executors. */
	u8 *RecBuf;			/**< Record window, XPROVISION_RECBUF_LEN bytes. */
	u32 SubSystemId;		/**< Subsystem that owns created vaults and keys. */
	u32 IpiMask;			/**< IPI mask of the requesting channel. */
	u32 Consumed;			/**< Plaintext bytes accepted so far. */
	u32 RecFill;			/**< Bytes currently held in RecBuf. */
	u32 RecLen;			/**< Full record length once the header is parsed, else 0. */
	u32 Executed;			/**< Records executed, for the terminal check. */
} XProvision_SinkCtx;
#endif /* XASU_ECIES_ENABLE */
/********************************** Variable Definitions *****************************************/

#ifdef XASU_ECIES_ENABLE
/*
 * Fixed 12-byte (96-bit) ECIES IV for the provisioning header path. No ECIES IV is transmitted in
 * the header; a fixed IV is cryptographically safe because the ECIES key is unique per ephemeral
 * ECDH exchange. Must match the sender/host constant.
 */
static const u8 ProvHdrEciesIv[XASU_AES_IV_SIZE_96BIT_IN_BYTES] = {
	0xA7U, 0x3FU, 0xE1U, 0x90U, 0x5CU, 0x2BU, 0xD4U, 0x68U, 0xF1U, 0x0AU, 0x9CU, 0x57U
};
/* Context string for the seed-to-piece-key CMAC-KDF. */
static const u8 ProvHdrKdfContext[] = XPROVISION_HDR_KDF_CONTEXT;

/*
 * The chunk layer hands a decrypted block to the sink without a context argument, so the record
 * assembler's state is published here for the span of one import. XProvision_UnwrapImage sets it
 * before the walk and clears it on every exit, so a sink call outside a walk finds no state and
 * refuses rather than writing through a stale pointer.
 */
static XProvision_SinkCtx *ProvActiveSinkCtx = NULL;
static XProvision_SourceCtx *ProvActiveSourceCtx = NULL;
#endif /* XASU_ECIES_ENABLE */
/************************************ Function Prototypes ****************************************/
static s32 XProvision_AcquireKey(XAsufw_Dma *DmaPtr,
				 const XAsu_ProvisionUserEfuseParams *ParamsPtr,
				 u32 SubsystemId, u8 *KeyBuf);
static s32 XProvision_EncryptRedKey(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 EfuseKeyId,
				      const u8 *PlainKey, u8 *BlackKey);
static s32 XProvision_WriteUserKeyToEfuse(u32 EfuseKeyId, const u32 *KeyData);
static s32 XProvision_CheckEfuseKeyBlank(u32 EfuseKeyId);
static s32 XProvision_IsUdeSlotFree(u8 UdeSlot);
static s32 XProvision_WriteUdeKeyToEfuse(u32 UdeSlot, const u32 *KeyData);
static s32 XProvision_GenerateKeyPair(XAsufw_Dma *DmaPtr, u32 SubSystemId, u8 KeyUseCase);
static s32 XProvision_DeleteKeyPair(u32 SubSystemId, u8 KeyUseCase);
static s32 XProvision_GetKeySlot(u32 SubSystemId, u16 *SlotPtr, u8 KeyUseCase);
static s32 XProvision_GetKeyId(u16 KeySlot, u32 *KeyIdPtr, u8 KeyUseCase);
static s32 XProvision_GetPubKey(u32 SubSystemId, u8 KeyUseCase, u8 *PubKeyBuf);
#ifdef XASU_ECIES_ENABLE
static s32 XProvision_ExecRecord(const XProvision_SinkCtx *Ctx);
static s32 XProvision_ExecCreateKeyVault(const u8 *RecPtr, u32 Size, u32 SubSystemId, u32 IpiMask);
static s32 XProvision_ExecStoreKey(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, const u8 *RecPtr,
		u32 Size, u32 SubSystemId);
static s32 XProvision_ExecXorKeys(XAsufw_Dma *DmaPtr, const u8 *RecPtr, u32 Size, u32 SubSystemId);
static s32 XProvision_ChunkSecretsWrap(void *ArgPtr, u32 ChunkIndex, const u8 *InPtr, u8 *OutPtr);
static s32 XProvision_ChunkSecretsUnwrap(void *ArgPtr, u32 ChunkIndex, const u8 *InPtr, u8 *OutPtr);
static s32 XProvision_BlobBlockSource(u32 Offset, u8 *BlockPtr, u32 Len);
static s32 XProvision_BlobBlockSink(u32 Offset, const u8 *BlockPtr, u32 Len);
static s32 XProvision_GetScratch(XKeyManager_WorkBuf **WorkPtr, u8 **RecBufPtr);
static s32 XProvision_GetWrapScratch(XProvision_WrapScratch **ScratchPtr);
#endif /* XASU_ECIES_ENABLE */

/*************************************************************************************************/
/**
 * @brief	This function programs a USER eFUSE key (USER_KEY_0 / USER_KEY_1) with a key
 *		from the source selected in ParamsPtr->KeySource (ASU-generated via TRNG, AKV
 *		vault, or user buffer). USER_KEY_0 is restricted to the ASU-generated source and
 *		is stored in black form. USER_KEY_1 is stored in the form selected by KeyForm.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance (used for red key encryption).
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUserEfuseParams structure.
 * @param	SubsystemId	Subsystem ID of the requesting subsystem (used for AKV access).
 *
 * @return
 *	- XASUFW_SUCCESS, if the USER eFUSE key is provisioned successfully.
 *	- XASUFW_PROVISION_INVALID_PARAM, if any input parameter is invalid.
 *	- XASUFW_PROVISION_KEY_FETCH_FAIL, if obtaining the key fails or the key is all zeros.
 *	- Error code from XAsufw_PmcKeyTransfer, if the PMC key transfer fails.
 *	- XASUFW_PUF_KEK_GEN_FAIL, if PUF KEK generation fails.
 *	- XASUFW_PROVISION_RED_ENCRYPT_FAIL, if red key encryption fails.
 *	- XASUFW_CHANGE_ENDIANNESS_ERROR, if reversing the key buffer fails.
 *	- XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ, if the target key is already programmed, or the PLM
 *	  reports a failure for the blank-key check or the eFUSE write.
 *	- Error code from the IPI transport, if the IPI request/response to PLM fails.
 *	- Error code from XAes_KeyClear or Xil_SecureZeroize, if clearing the key vault or
 *	  zeroizing the local key buffers fails, even when provisioning itself succeeded.
 *
 * @note	Each eFUSE key is checked to be unprogrammed before any key material is produced,
 *		so a given key is encrypted under its RTCA base IV at most once per device. A black
 *		key programmed here is in ASU-accepted form, which is incompatible with the PMC;
 *		provision black from this path only for an ASU use case. Red form is the same for
 *		PMC and ASU.
 *
 *************************************************************************************************/
s32 XProvision_UserEfuse(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			 const XAsu_ProvisionUserEfuseParams *ParamsPtr, u32 SubsystemId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 RedKeyBuf[XASUFW_NVM_EFUSE_AES_KEY_LEN_IN_WORDS] = {0U};
	u32 BlackKeyBuf[XASUFW_NVM_EFUSE_AES_KEY_LEN_IN_WORDS] = {0U};
	u32 *KeyBuf = NULL;
	u32 PufKekGenStatus = XASUFW_FAILURE;

	/** Validate the server-side resource pointers. */
	if ((DmaPtr == NULL) || (AesInstancePtr == NULL)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/**
	 * Validate all command parameters up front: EfuseKeyId, KeyLen, the per-source
	 * KeyId/KeyAddr rules, and the key source against the destination eFUSE.
	 */
	Status = XAsu_ProvisionValidateUserEfuseParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/** User key 0 accepts a benign key only from the ASU-generated source. */
	if ((ParamsPtr->EfuseKeyId == XASU_PROVISION_EFUSE_USER_KEY_0) &&
	    (ParamsPtr->KeySource != XASU_PROVISION_KEY_SOURCE_ASU_GENERATED)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/**
	 * Reject an already programmed key before any key material is produced, so a doomed
	 * request cannot consume a vault key usage count or re-encrypt under the same KEK and IV.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_CheckEfuseKeyBlank((u32)ParamsPtr->EfuseKeyId);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Obtain the plaintext key from the requested source. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_AcquireKey(DmaPtr, ParamsPtr, SubsystemId, (u8 *)RedKeyBuf);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_FETCH_FAIL);
		goto END_CLR;
	}

	/** Reject an all-zero key before it is burned, for every key source. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero((u8 *)RedKeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_KEY_FETCH_FAIL;
		goto END_CLR;
	}

	/**
	 * Bring the PUF KEK with the existing black key transfer, then encrypt the plaintext.
	 * A red USER_KEY_1 is written as plaintext and does not use the PUF key.
	 */
	if ((ParamsPtr->EfuseKeyId == XASU_PROVISION_EFUSE_USER_KEY_1) &&
	    (ParamsPtr->KeyForm == XASU_EFUSE_RED_KEY)) {
		KeyBuf = RedKeyBuf;
	} else {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		XFIH_CALL_GOTO(XAsufw_PmcKeyTransfer, XFihVar, Status, END_CLR, &PufKekGenStatus,
				XASU_EFUSE_BLACK_KEY);

		if (PufKekGenStatus != (u32)XASUFW_SUCCESS) {
			Status = XASUFW_PUF_KEK_GEN_FAIL;
			goto END_CLR;
		}

		/** PUF KEK encrypt the plaintext inside the ASU. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XProvision_EncryptRedKey(DmaPtr, AesInstancePtr, ParamsPtr->EfuseKeyId,
						 (u8 *)RedKeyBuf, (u8 *)BlackKeyBuf);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							 XASUFW_PROVISION_RED_ENCRYPT_FAIL);
			goto END_CLR;
		}
		KeyBuf = BlackKeyBuf;
	}

	/** Byte-reverse KeyBuf for the PLM write. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_CHANGE_ENDIANNESS_ERROR;
		goto END_CLR;
	}

	/** Request the PLM (XilNVM) to program the selected USER eFUSE key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_WriteUserKeyToEfuse((u32)ParamsPtr->EfuseKeyId, KeyBuf);

END_CLR:
	/**
	 * Clear eFUSE key 0, eFUSE key 1, eFUSE red key 0, eFUSE red key 1 and the PUF key
	 * in a single call, covering every path after XAsufw_PmcKeyTransfer.
	 */
	XFIH_CALL(XAes_KeyClear, XFihVar, ClearStatus, AesInstancePtr,
		  XAES_KEY_CLEAR_MULTI_KEY_FLAG |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_0) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_RED_0) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_1) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_RED_1) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_PUF_KEY));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Zeroize the local key buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihVar, ClearStatus, (u8 *)RedKeyBuf,
		  XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	ClearStatus = Xil_SecureZeroize((u8 *)BlackKeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function obtains the 256-bit plaintext key to be programmed, based on the
 *		explicitly selected key source (ASU-generated via TRNG, AKV vault, or user buffer).
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUserEfuseParams structure.
 * @param	SubsystemId	Subsystem ID of the requesting subsystem (used for AKV access).
 * @param	KeyBuf		Pointer to the 32-byte buffer that receives the plaintext key.
 *
 * @return
 *	- XASUFW_SUCCESS, if the key is obtained successfully.
 *	- XASUFW_PROVISION_INVALID_PARAM, if the key source is invalid or the vault key is not
 *		256-bit, is not non-depleting, or has a non-zero EpochTime.
 *	- XASUFW_KEYMANAGER_KEY_TYPE_MISMATCH, if the vault KeyId is not an AES sub-vault key.
 *	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if the AKV key lookup fails.
 *	- XASUFW_MEM_COPY_FAIL, if copying the AKV key (vault path) fails.
 *	- XASUFW_DMA_COPY_FAIL, if the DMA transfer of the user key (buffer path) fails.
 *	- Error code from XTrng_GetRandomNumbers, if TRNG key generation (ASU-generated path) fails.
 *
 *************************************************************************************************/
static s32 XProvision_AcquireKey(XAsufw_Dma *DmaPtr,
				 const XAsu_ProvisionUserEfuseParams *ParamsPtr,
				 u32 SubsystemId, u8 *KeyBuf)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XKeyManager_AesKeyObject *VaultKeyPtr = NULL;
	const u8 *KeyObjPtr = NULL;

	/** Obtain the key based on the explicitly selected key source. */
	switch (ParamsPtr->KeySource) {
		case XASU_PROVISION_KEY_SOURCE_ASU_GENERATED:
			/** ASU-generated path: generate a 256-bit AES key from the TRNG. */
			Status = XTrng_GetRandomNumbers(KeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
			break;
		case XASU_PROVISION_KEY_SOURCE_VAULT:
			/** Vault path: fetch the AES key from the AKV by KeyId. */
			if (XASU_KM_GET_KEY_TYPE(ParamsPtr->KeyId) != (u8)XASU_AES_SUBVAULT_ID) {
				Status = XASUFW_KEYMANAGER_KEY_TYPE_MISMATCH;
				goto END;
			}
			Status = XKeyManager_GetKeyObjectPtr(ParamsPtr->KeyId, SubsystemId,
				(u8)(XASU_KEYMANAGER_AES_ENC_USE_CASE |
				     XASU_KEYMANAGER_AES_DEC_USE_CASE), (u8)FALSE,
				&KeyObjPtr);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
				goto END;
			}
			VaultKeyPtr = (const XKeyManager_AesKeyObject *)(const void *)KeyObjPtr;
			/**
			 * Only a non-expiring (EpochTime 0), non-depleting 256-bit AES vault
			 * key may be programmed into USER eFUSE.
			 */
			if ((VaultKeyPtr->Metadata.EpochTime != 0U) ||
			    (VaultKeyPtr->Metadata.UsageCount !=
			     XASU_KM_USAGE_COUNT_NON_DEPLETING_VALUE) ||
			    (VaultKeyPtr->Metadata.Length != XASU_AES_KEY_SIZE_256BIT_IN_BYTES)) {
				Status = XASUFW_PROVISION_INVALID_PARAM;
				goto END;
			}
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = Xil_SMemCpy(KeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES,
					     VaultKeyPtr->Content,
					     XPROVISION_EFUSE_KEY_SIZE_IN_BYTES,
					     XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MEM_COPY_FAIL;
			}
			break;
		case XASU_PROVISION_KEY_SOURCE_USER_BUFFER:
			/** Buffer path: copy the user supplied key into the local buffer. */
			Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->KeyAddr, (u64)(UINTPTR)KeyBuf,
					       XPROVISION_EFUSE_KEY_SIZE_IN_BYTES, 0U);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DMA_COPY_FAIL;
			}
			break;
		default:
			Status = XASUFW_PROVISION_INVALID_PARAM;
			break;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts the given plaintext key with the PUF KEK to produce the black
 *		key to be programmed into the eFUSE. The caller transfers the PUF KEK before this
 *		call and clears the key vault after it.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	EfuseKeyId	Target USER eFUSE key (selects the KEK IV: eFUSE0 or eFUSE1).
 * @param	PlainKey	Pointer to the 256-bit plaintext key.
 * @param	BlackKey	Pointer to the buffer that receives the 256-bit black
 *				(encrypted) key.
 *
 * @return
 *	- XASUFW_SUCCESS, if the black key is generated successfully.
 *	- XASUFW_MEM_COPY_FAIL, if copying the eFUSE KEK IV fails.
 *	- XASUFW_KEK_IV_IS_ZERO, if the eFUSE KEK IV is zero.
 *	- Error code from XAes_EncryptRedKey, if the AES encryption operation fails.
 *
 * @note	Each eFUSE key has its own RTCA base IV, which is used as-is.
 *
 *************************************************************************************************/
static s32 XProvision_EncryptRedKey(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 EfuseKeyId,
				      const u8 *PlainKey, u8 *BlackKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	const u8 *IvPtr = (EfuseKeyId == XASU_PROVISION_EFUSE_USER_KEY_1) ?
		(const u8 *)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_1_IV_ADDR :
		(const u8 *)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_0_IV_ADDR;
	u8 IvBuf[XPROVISION_EFUSE_IV_SIZE_IN_BYTES] = {0U};
	u8 TagBuf[XPROVISION_EFUSE_TAG_SIZE_IN_BYTES] = {0U};
	XAsu_AesParams AesParams = {0U};

	/** Copy the eFUSE KEK IV (eFUSE0 or eFUSE1) from PLM RTCA and ensure it is non-zero. */
	Status = Xil_SMemCpy(IvBuf, XPROVISION_EFUSE_IV_SIZE_IN_BYTES, IvPtr,
			     XPROVISION_EFUSE_IV_SIZE_IN_BYTES, XPROVISION_EFUSE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero(IvBuf, XPROVISION_EFUSE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEK_IV_IS_ZERO;
		goto END;
	}

	/** AES-GCM encrypt the red key with the PUF KEK, selected by XAes_EncryptRedKey. */
	AesParams.EngineMode = (u8)XASU_AES_GCM_MODE;
	AesParams.OperationType = (u8)XASU_AES_ENCRYPT_OPERATION;
	AesParams.IvObj.IvAddr = (u64)(UINTPTR)IvBuf;
	AesParams.IvObj.IvLen = XPROVISION_EFUSE_IV_SIZE_IN_BYTES;
	AesParams.InputDataAddr = (u64)(UINTPTR)PlainKey;
	AesParams.OutputDataAddr = (u64)(UINTPTR)BlackKey;
	AesParams.DataLen = XPROVISION_EFUSE_KEY_SIZE_IN_BYTES;
	AesParams.TagAddr = (u64)(UINTPTR)TagBuf;
	AesParams.TagLen = XPROVISION_EFUSE_TAG_SIZE_IN_BYTES;

	/** Encrypt the red key with the PUF KEK and return the raw AES status. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_EncryptRedKey(AesInstancePtr, DmaPtr, &AesParams);

	/** Zeroize the local tag buffer. */
	ClearStatus = Xil_SecureZeroize(TagBuf, XPROVISION_EFUSE_TAG_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends an IPI request to the PLM to program a USER eFUSE key
 *		(USER_KEY_0 / USER_KEY_1) with the provided key bytes. Black provisioning passes
 *		the PUF KEK ciphertext. Red USER_KEY_1 provisioning passes the plaintext.
 *
 * @param	EfuseKeyId	USER eFUSE key identifier (XASU_PROVISION_EFUSE_USER_KEY_0 or
 *				XASU_PROVISION_EFUSE_USER_KEY_1).
 * @param	KeyData		Pointer to the 256-bit (8-word) key (red or black) to program.
 *
 * @return
 *	- XASUFW_SUCCESS, if the USER eFUSE key is programmed.
 *	- Error code from the IPI transport, if the IPI request/response to PLM fails.
 *	- XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ, if the PLM reports a failure for the eFUSE write.
 *
 *************************************************************************************************/
static s32 XProvision_WriteUserKeyToEfuse(u32 EfuseKeyId, const u32 *KeyData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XASUFW_PLM_XILNVM_WRITE_PAYLOAD_LEN];
	u32 Response = (u32)XASUFW_FAILURE;

	/**
	 * Build the AES-key-write IPI payload: {hdr, EnvDis|KeyType, KeyAddrLow,
	 * KeyAddrHigh}. The PLM reads the key by address; KeyData is a word-aligned u32 array
	 * as the XilNVM AES key write requires, and it stays valid for the duration of this
	 * blocking IPI exchange.
	 */
	Payload[XASUFW_PLM_XILNVM_CMD_HDR_INDEX] =
		XASUFW_PLM_IPI_HEADER(XASUFW_PLM_XILNVM_PLOAD_LEN,
				      XASUFW_PLM_XILNVM_WRITE_AES_KEY_API_ID,
				      XASUFW_PLM_XILNVM_MODULE_ID);
	Payload[XASUFW_PLM_XILNVM_ENVDIS_KEYTYPE_INDEX] =
		(EfuseKeyId << XASUFW_PLM_XILNVM_KEYTYPE_SHIFT) |
		XASUFW_PLM_XILNVM_ENV_MON_DIS;
	Payload[XASUFW_PLM_XILNVM_KEY_ADDR_LOW_INDEX] = (u32)(UINTPTR)KeyData;
	/* ASU is a 32-bit address space; the high address word is always zero. */
	Payload[XASUFW_PLM_XILNVM_KEY_ADDR_HIGH_INDEX] = 0U;

	/** Send the IPI request to PLM; propagate any IPI transport error. */
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_PLM_XILNVM_WRITE_PAYLOAD_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Read the IPI response; propagate any IPI transport error. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm(&Response, XASUFW_PLM_XILNVM_RESP_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * The IPI read succeeded but the PLM may still report a failure in the response.
	 * Print the PLM response and return the provisioning error code to the client.
	 */
	if (Response != (u32)XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "PLM eFUSE write failed: 0x%08x\r\n", Response);
		Status = XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function asks the PLM to verify the target USER eFUSE key against the CRC of
 * 		an all-zero key, which matches only while the key is unprogrammed.
 *
 * @param	EfuseKeyId	USER eFUSE key identifier (XASU_PROVISION_EFUSE_USER_KEY_0 or
 *				XASU_PROVISION_EFUSE_USER_KEY_1).
 *
 * @return
 *	- XASUFW_SUCCESS, if the target USER eFUSE key is unprogrammed.
 *	- Error code from the IPI transport, if the IPI request/response to PLM fails.
 *	- XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ, if the key is already programmed or the PLM reports any
 *		other failure.
 *
 *************************************************************************************************/
static s32 XProvision_CheckEfuseKeyBlank(u32 EfuseKeyId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XASUFW_PLM_XILNVM_CRC_PAYLOAD_LEN];
	u32 Response = (u32)XASUFW_FAILURE;

	/**
	 * Build the CRC check payload: {hdr, Crc, KeyType}.
	 * KeyType occupies a whole word here.
	 */
	Payload[XASUFW_PLM_XILNVM_CMD_HDR_INDEX] =
		XASUFW_PLM_IPI_HEADER(XASUFW_PLM_XILNVM_CRC_PLOAD_LEN,
				      XASUFW_PLM_XILNVM_CHECK_CRC_API_ID,
				      XASUFW_PLM_XILNVM_MODULE_ID);
	Payload[XASUFW_PLM_XILNVM_CRC_INDEX] = XASUFW_NVM_EFUSE_CRC_AES_ZEROS;
	Payload[XASUFW_PLM_XILNVM_CRC_KEYTYPE_INDEX] = EfuseKeyId;

	/** Send the IPI request to PLM; propagate any IPI transport error. */
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_PLM_XILNVM_CRC_PAYLOAD_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Read the IPI response; propagate any IPI transport error. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm(&Response, XASUFW_PLM_XILNVM_RESP_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * A CRC mismatch means the key is already programmed; any other failure is a controller or
	 * command error. Print the PLM response and return a single error code for both.
	 */
	if (Response != (u32)XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "PLM eFUSE CRC check failed: 0x%08x\r\n",
			      Response);
		Status = XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function provisions a Unique Device Endorsement (UDE) key pair.
 *
 *		The ECC NIST P-384 key pair is generated inside the ASU so the private key is never
 *		exposed outside the ASU. The private key is encrypted with the PUF KEK (reusing the OCP
 *		UDE encryption scheme), the encrypted key is programmed into the requested UDE eFUSE
 *		slot via the PLM (XilNVM), and a self-signed X.509 certificate over the generated
 *		public key is returned to the caller along with its actual size.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	ShaPtr			Pointer to the SHA3 (XSHA_1) instance used to hash the
 *					self-signed certificate.
 * @param	ParamsPtr		Pointer to the XAsu_ProvisionUdeParams structure.
 * @param	CertActualSizePtr	Pointer to the response buffer location that receives the
 *					actual size in bytes of the generated certificate. The client
 *					copies this value to ParamsPtr->CertActualSizeAddr during the
 *					callback.
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE key pair is provisioned successfully.
 *	- XASUFW_PROVISION_INVALID_PARAM, if any input parameter is invalid.
 *	- XASUFW_PROVISION_UDE_SLOT_PROGRAMMED, if the target UDE slot is already programmed.
 *	- XASUFW_PROVISION_UDE_ENCRYPT_FAIL, if UDE key pair generation, private key encryption or
 *	  certificate generation fails.
 *	- XASUFW_PROVISION_UDE_EFUSE_WRITE_FAIL, if the eFUSE write request to the PLM fails.
 *	- XASUFW_PROVISION_UDE_KEY_TYPE_UPDATE_FAIL, if updating the key vault header fails.
 *	- XASUFW_OCP_DISABLED, if OCP is disabled.
 *
 *************************************************************************************************/
s32 XProvision_Ude(XAsufw_Dma *DmaPtr, XSha *ShaPtr, const XAsu_ProvisionUdeParams *ParamsPtr,
		   u32 *CertActualSizePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	u32 CertSize = 0U;
	XFih_Var XFihProvision = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 EncPvtKey[XPROVISION_UDE_PVT_KEY_LEN_IN_WORDS];
	u8 UdeKeyTypeCode;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (ShaPtr == NULL) || (CertActualSizePtr == NULL)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_ProvisionValidateUdeParams(ParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/** Clear the encrypted key buffer before use. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SecureZeroize((u8 *)EncPvtKey, XASU_PROVISION_UDE_PVT_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Ensure the target UDE eFUSE slot is not already programmed. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_IsUdeSlotFree(ParamsPtr->UdeSlot);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_UDE_SLOT_PROGRAMMED);
		XFIH_GOTO(END);
	}

	/**
	 * Generate the UDE key pair, encrypt the private key with the PUF KEK into the local buffer,
	 * and generate a self-signed X.509 certificate over the generated public key into the
	 * caller's certificate buffer. The OCP UDE key encryption scheme is reused so the encrypted
	 * key stays byte-compatible with the UDE challenge response decryption path, and the private
	 * key is never exposed outside the ASU.
	 */
#ifdef XASU_OCP_ENABLE
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_EncryptUdeKeys(DmaPtr, ShaPtr, (u32)ParamsPtr->UdeSlot, (u8 *)EncPvtKey,
						ParamsPtr->CertBufAddr, ParamsPtr->CertBufSize,
						&CertSize);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_UDE_ENCRYPT_FAIL);
		XFIH_GOTO(END_CLR);
	}
#else
	ASSIGN_VOLATILE(Status, XASUFW_OCP_DISABLED);
	XFIH_GOTO(END_CLR);
#endif /* XASU_OCP_ENABLE */

	/**
	 * Record the provisioned UDE key type in the key vault header for the programmed slot only
	 * (00b = ECC P-384, 01b = ML-DSA). The KeyManager packs the two-bit code into the slot's
	 * position (UDE_0 in the lowest two bits, UDE_3 in the highest two bits) without disturbing
	 * the other slots.
	 */
	if (ParamsPtr->UdeKeyType == XASU_PROVISION_UDE_KEY_TYPE_ECC_P384) {
		UdeKeyTypeCode = XKEYMANAGER_UDE_KEY_TYPE_CODE_P384;
	} else {
		UdeKeyTypeCode = XKEYMANAGER_UDE_KEY_TYPE_CODE_MLDSA;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_UpdateUdeKeyType(ParamsPtr->UdeSlot, UdeKeyTypeCode);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_UDE_KEY_TYPE_UPDATE_FAIL);
		goto END_CLR;
	}

	/**
	 * Return the actual size of the generated UDE certificate to the caller through the
	 * response buffer. The client copies this value from the response buffer to
	 * ParamsPtr->CertActualSizeAddr during the callback. The certificate itself was already
	 * copied to the caller's buffer by XOcp_EncryptUdeKeys.
	 */
	*CertActualSizePtr = CertSize;

	/** Program the encrypted UDE private key into the requested eFUSE slot via the PLM. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_WriteUdeKeyToEfuse((u32)ParamsPtr->UdeSlot, (const u32 *)EncPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_UDE_EFUSE_WRITE_FAIL);
	}

#ifdef XASU_OCP_ENABLE
	/**
	 * The UDE public key cache names the slot by the address of its encrypted key, and the
	 * write leaves that address unchanged, so drop the cache to keep a key derived earlier in
	 * this boot from being served for the key just programmed.
	 */
	XOcp_InvalidateUdePubKeyCache();
#endif /* XASU_OCP_ENABLE */

END_CLR:
	/** Zeroize the local encrypted key buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihProvision, ClearStatus, (u8 *)EncPvtKey,
		  XASU_PROVISION_UDE_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks whether the requested UDE eFUSE slot is free (not yet
 *		programmed) by reading its eFUSE cache and ensuring it is all zeros.
 *
 * @param	UdeSlot		UDE eFUSE slot (XASU_OCP_UDE_USER_KEY_0/1/2_ID).
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE slot is free.
 *	- XASUFW_PROVISION_INVALID_PARAM, if the UDE slot is invalid.
 *	- XASUFW_PROVISION_UDE_SLOT_PROGRAMMED, if the UDE slot is already programmed.
 *	- XASUFW_OCP_DISABLED, if OCP is disabled.
 *
 *************************************************************************************************/
static s32 XProvision_IsUdeSlotFree(u8 UdeSlot)
{
	s32 Status = XASUFW_FAILURE;
#ifdef XASU_OCP_ENABLE
	u32 UdeUserKeyAddr = 0U;

	/** Map the UDE slot to its eFUSE cache address. */
	Status = XOcp_GetUdeSlotInfo((u32)UdeSlot, &UdeUserKeyAddr, NULL);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_INVALID_PARAM);
		goto END;
	}

	/**
	 * If the eFUSE cache for the slot is non-zero, the slot is already programmed. Note the PLM
	 * (XilNVM) also enforces this during the key write.
	 */
	Status = XASUFW_FAILURE;
	Status = XAsu_IsBufferNonZero((u8 *)(UINTPTR)UdeUserKeyAddr,
				      XASU_PROVISION_UDE_PVT_KEY_SIZE_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_UDE_SLOT_PROGRAMMED;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
#else
	/** UDE slot mapping lives in the OCP module, so the check is unsupported without OCP. */
	(void)UdeSlot;
	Status = XASUFW_OCP_DISABLED;
#endif /* XASU_OCP_ENABLE */
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends an IPI request to the PLM to program a UDE (DME) private key
 *		into the requested eFUSE slot with the provided encrypted key, and reloads the eFUSE
 *		cache so the newly programmed key is visible.
 *
 * @param	UdeSlot		UDE eFUSE (DME user key) slot to program.
 * @param	KeyData		Pointer to the encrypted UDE private key data to be programmed.
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE eFUSE key is programmed.
 *	- XASUFW_PROVISION_INVALID_PARAM, if input parameters are invalid.
 *	- XASUFW_PROVISION_PLM_IPI_FAIL, if the IPI request/response to the PLM fails.
 *	- XASUFW_PROVISION_PLM_ERROR, if the PLM reports a failure for the eFUSE write.
 *
 *************************************************************************************************/
static s32 XProvision_WriteUdeKeyToEfuse(u32 UdeSlot, const u32 *KeyData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XASUFW_PLM_XILNVM_WRITE_PAYLOAD_LEN];
	u32 Response = (u32)XASUFW_FAILURE;

	if ((KeyData == NULL) || ((UdeSlot != XASU_OCP_UDE_USER_KEY_0_ID) &&
	    (UdeSlot != XASU_OCP_UDE_USER_KEY_1_ID) && (UdeSlot != XASU_OCP_UDE_USER_KEY_2_ID))) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/**
	 * Build the DME-key-write IPI payload: {hdr, EnvDis | (DmeKeyType << 16), KeyAddrLow,
	 * KeyAddrHigh}.
	 */
	Payload[XASUFW_PLM_XILNVM_CMD_HDR_INDEX] =
		XASUFW_PLM_IPI_HEADER(XASUFW_PLM_XILNVM_PLOAD_LEN,
				      XASUFW_PLM_XILNVM_WRITE_DME_KEY_API_ID,
				      XASUFW_PLM_XILNVM_MODULE_ID);
	Payload[XASUFW_PLM_XILNVM_ENVDIS_KEYTYPE_INDEX] =
		(UdeSlot << XASUFW_PLM_XILNVM_KEYTYPE_SHIFT) |
		XASUFW_PLM_XILNVM_ENV_MON_DIS;
	Payload[XASUFW_PLM_XILNVM_KEY_ADDR_LOW_INDEX] = (u32)(UINTPTR)KeyData;
	/* ASU is a 32-bit address space; the high address word is always zero. */
	Payload[XASUFW_PLM_XILNVM_KEY_ADDR_HIGH_INDEX] = 0U;

	/** Send an IPI request to the PLM to write the UDE eFUSE key. */
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_PLM_XILNVM_WRITE_PAYLOAD_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_PLM_IPI_FAIL;
		goto END;
	}

	/** Read the IPI response and check for a PLM-reported failure. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm(&Response, XASUFW_PLM_XILNVM_RESP_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_PLM_IPI_FAIL;
		goto END;
	} else if (Response != (u32)XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "PLM UDE eFUSE write failed: 0x%08x\r\n", Response);
		Status = XASUFW_PROVISION_PLM_ERROR;
		goto END;
	} else {
		/* Do nothing. */
	}

	/**
	 * Request the PLM to reload the eFUSE cache and program the protection bits so the newly
	 * programmed UDE key is reflected in the eFUSE cache.
	 */
	Payload[XASUFW_PLM_XILNVM_CMD_HDR_INDEX] =
		XASUFW_PLM_IPI_HEADER(0U, XASUFW_PLM_XILNVM_RELOAD_API_ID,
				      XASUFW_PLM_XILNVM_MODULE_ID);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_PLM_XILNVM_RELOAD_PAYLOAD_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_PLM_IPI_FAIL;
		goto END;
	}

	Response = (u32)XASUFW_FAILURE;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm(&Response, XASUFW_PLM_XILNVM_RESP_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_PLM_IPI_FAIL;
	} else if (Response != (u32)XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "PLM eFUSE reload failed: 0x%08x\r\n", Response);
		Status = XASUFW_PROVISION_PLM_ERROR;
	} else {
		/* Do nothing. */
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates an ECC key pair for the wrapped-provisioning
 *		feature and stores it in the ASU-owned exportable key vault. The private key is
 *		restricted to the requested provisioning use case only, and neither
 *		half of the pair nor either key ID is returned to the caller: both halves stay in
 *		the vault and the unwrap path locates the private key internally. Each subsystem
 *		may hold one provisioning key pair per use case, so a request whose reserved slot is
 *		already populated is rejected instead of replacing the key: a repeated request
 *		cannot invalidate a public half which has already been handed out. Replacing a pair
 *		remains possible, but only as a deliberate act of the subsystem which owns it,
 *		which has to delete the pair first with XProvision_DeleteKeyPair(). No
 *		other subsystem can do it on its behalf, because the slot is derived from the
 *		requesting subsystem rather than taken from the request.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	SubSystemId	Subsystem requesting the provisioning key pair. Together with the
 *				use case it selects the reserved key slot within the exportable
 *				vault.
 * @param	KeyUseCase	Provisioning use case the key pair is generated for, either the
 *				provisioning-material unwrap use case (ECIES) or the provisioning
 *				authenticated-encryption use case. It selects the reserved slot
 *				range and is stored as the private key's use case. Nothing else is
 *				caller-supplied: the vault id (ASU-owned exportable vault), curve
 *				(P-384), key length, epoch time and non-depleting usage count are
 *				all set internally.
 *
 * @return
 *	- XASUFW_SUCCESS, if the key pair is generated and both halves are stored.
 *	- XASUFW_KEYMANAGER_INVALID_PARAM, if DmaPtr is NULL.
 *	- XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED, if no slot is reserved for the
 *		subsystem and use case, which is the case when the use case is not a supported
 *		provisioning use case or the subsystem is not assigned to any channel. The
 *		underlying cause is packed into the status. The key pair delete path reports a
 *		slot lookup failure with the same code.
 *	- XASUFW_KEYMANAGER_PROVISIONING_KEY_ALREADY_EXISTS, if the reserved slot already holds a
 *		provisioning private key for that use case, or the slot lookup did not confirm the
 *		slot is free.
 *	- Any other status comes through unchanged from XKeyManager_GenerateEccKeyPair(), which
 *		performs the generation and the vault store; see that function for the codes it
 *		returns. XASUFW_KEYMANAGER_KEY_SLOT_IN_USE arrives this way when the reserved slot
 *		is occupied, because the already-exists guard above inspects only the private half
 *		and a slot left holding a stranded public half reaches the store.
 *
 *************************************************************************************************/
static s32 XProvision_GenerateKeyPair(XAsufw_Dma *DmaPtr, u32 SubSystemId, u8 KeyUseCase)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 PvtKeyId = 0U;
	u16 KeySlot = 0U;
	u32 KeyIds[XKEYMANAGER_KEY_PAIR_ID_COUNT] = {0U};
	XAsu_KeyManagerParams KmParams = {0};

	/** Validate input parameters. */
	if (DmaPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Map the subsystem and the requested use case to their reserved slot in the exportable
	 * vault. Status holds the cause from the lookup, an unsupported use case or a subsystem
	 * with no channel, so it is retained rather than replaced.
	 */
	Status = XProvision_GetKeySlot(SubSystemId, &KeySlot, KeyUseCase);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED);
		goto END;
	}

	/**
	 * A subsystem gets one provisioning key pair per use case. The slot is fixed for that
	 * pairing, so a populated slot means a key pair whose public half may already be held
	 * elsewhere: refuse, so that a repeated request cannot invalidate that copy by accident.
	 * This is not a guarantee that the pair cannot be replaced. The owning subsystem can
	 * delete it and generate a new one, which is a deliberate act of the party which handed
	 * the public half out, and no other subsystem can do it on its behalf.
	 * Only an explicit XASUFW_KEYMANAGER_KEY_NOT_FOUND lets the request through, so a lookup
	 * which is skipped or whose result is disturbed refuses the request instead of overwriting
	 * the key.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GetKeyId(KeySlot, &PvtKeyId, KeyUseCase);
	if (Status != XASUFW_KEYMANAGER_KEY_NOT_FOUND) {
		Status = XASUFW_KEYMANAGER_PROVISIONING_KEY_ALREADY_EXISTS;
		goto END;
	}

	/**
	 * The provisioning key pair profile is fixed and never taken from the caller: it is always
	 * generated on the NIST P-384 curve, stored only in the ASU-owned exportable vault, and
	 * restricted to the provisioning (ECIES) use cases, one per half. The private key is kept as a
	 * non-depleting key so it can service repeated unwrap requests. The slot is reserved per
	 * subsystem so the KeyId stays derivable after an export/import cycle. Populate the metadata
	 * internally so the caller cannot alter the profile.
	 */
	KmParams.KeyMetadata.CompositeKeyId = PvtKeyId;
	KmParams.KeyMetadata.KeyUseCase = KeyUseCase;
	KmParams.KeyMetadata.KeyAttributes = (u8)XASU_ECC_NIST_P384;
	KmParams.KeyMetadata.Length = (u16)XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES;
	KmParams.KeyMetadata.EpochTime = 0U;
	KmParams.KeyMetadata.UsageCount = XASU_KM_USAGE_COUNT_NON_DEPLETING_VALUE;
	/**
	 * The key pair goes to the vault, not to a caller buffer, so KeyObjectAddr stays zero.
	 * The vault path requires exactly one of KeyObjectAddr and KeyIdAddr to be set, so
	 * KeyIdAddr points at the local Key IDs; the assigned IDs are written through the KeyIds
	 * argument and never travel outside the ASU.
	 */
	KmParams.KeyObjectAddr = 0U;
	KmParams.KeyIdAddr = (u64)(UINTPTR)&KeyIds[0U];

	/**
	 * Generate and store the key pair through the common ECC key pair path, as the ASU: the
	 * exportable vault is ASU-owned, so its vault access check also rejects the case where an
	 * imported blob has handed vault 1 to another subsystem. Both halves land in the reserved
	 * slot of their sub-vault and the assigned Key IDs stay ASU-internal (not returned to the
	 * caller) so they cannot be reused as handles for other operations.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_GenerateEccKeyPair(DmaPtr, &KmParams, KeyIds,
						XKEYMANAGER_ASU_SUBSYSTEM_ID);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function deletes the wrapped-provisioning ECC key pair held in a
 *		subsystem's reserved slot of the ASU-owned exportable key vault, so that the
 *		subsystem can generate a fresh pair. Both halves are deleted as the ASU, because
 *		the exportable vault is ASU-owned and a client cannot delete from it directly.
 *		A slot holding only one half still has that half cleaned up, but the operation
 *		reports XASUFW_KEYMANAGER_KEY_NOT_FOUND because there was no complete key pair to
 *		delete; success is reported only when both halves are deleted. Each provisioning
 *		use case owns its own reserved slot, so only the pair held for the requested use
 *		case is deleted and the other use case's pair is left untouched.
 *
 * @param	SubSystemId	Subsystem whose provisioning key pair is deleted. Together with the
 *				use case it selects the reserved key slot within the exportable
 *				vault.
 * @param	KeyUseCase	Provisioning use case whose key pair is deleted, either the
 *				provisioning-material unwrap use case (ECIES) or the provisioning
 *				authenticated-encryption use case. It must be the use case the pair
 *				was generated for, because it selects the reserved slot range.
 *				Nothing else is caller-supplied: the vault id and the sub-vaults are
 *				fixed internally.
 *
 * @return
 *	- XASUFW_SUCCESS, only if both halves of the key pair are deleted.
 *	- XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED, if no slot is reserved for the
 *		subsystem and use case, which is the case when the use case is not a supported
 *		provisioning use case or the subsystem is not assigned to any channel. The
 *		underlying cause is packed into the status. The key pair generation path reports a
 *		slot lookup failure with the same code.
 *	- XASUFW_KEYMANAGER_KEY_NOT_FOUND, if the reserved slot did not hold both halves (neither
 *		half, or only one), so there was no complete key pair to delete.
 *	- The delete failure reported by XKeyManager_DeleteKey() packed with
 *		XASUFW_KEYMANAGER_KEY_DELETE_ERROR, if deleting a half which is present fails. The
 *		cause, such as a missing vault, a vault no longer owned by the ASU or a failed
 *		zeroization, is kept in the first error field rather than discarded, so the value
 *		does not compare equal to XASUFW_KEYMANAGER_KEY_DELETE_ERROR on its own.
 *
 *************************************************************************************************/
static s32 XProvision_DeleteKeyPair(u32 SubSystemId, u8 KeyUseCase)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	static const XAsu_KeyManagerSubVaultType SubVaults[XKEYMANAGER_ASYM_KEY_TYPE_COUNT] = {
		XASU_ECC_PUB_SUBVAULT_ID, XASU_ECC_PVT_SUBVAULT_ID
	};
	u16 KeySlot = 0U;
	u32 KeyId = 0U;
	u32 TypeIdx = 0U;
	volatile u32 DeletedCount = 0U;

	/**
	 * Map the subsystem and the requested use case to their reserved slot in the exportable
	 * vault. The lookup failure is reported with the same code as the key pair generation path,
	 * so one cause does not reach the caller as two different codes depending on which command
	 * was issued.
	 */
	Status = XProvision_GetKeySlot(SubSystemId, &KeySlot, KeyUseCase);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED);
		goto END;
	}

	/**
	 * Delete both halves from the reserved slot as the ASU, public half first. The private half
	 * is what XProvision_GetKeyId() looks at, so it is the marker the key pair
	 * generation guard tests. Removing it last keeps that marker truthful: if
	 * the second delete fails, the pair is still usable and the generation guard still refuses
	 * cleanly, and re-issuing this operation clears the slot. Deleting it first would leave a
	 * public half that no path reports, yet which still occupies the reserved slot and makes the
	 * next generation fail inside the store.
	 * A half which is already absent is not an error: the goal is an empty slot, and stopping
	 * there would leave the other half behind and keep blocking a fresh key pair. Only an
	 * absent key is tolerated, not an unusable sub-vault: a vault 1 whose ECC public sub-vault
	 * has no capacity for the reserved slot, as an imported pre-public-half blob would give,
	 * is reported rather than silently half-deleted.
	 */
	for (TypeIdx = 0U; TypeIdx < XKEYMANAGER_ASYM_KEY_TYPE_COUNT; TypeIdx++) {
		KeyId = XASU_KM_COMPOSE_KEY_ID(XKEYMANAGER_ASU_EXPORTABLE_VAULT_ID,
						SubVaults[TypeIdx], KeySlot);

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_DeleteKey(KeyId, XKEYMANAGER_ASU_SUBSYSTEM_ID);
		if (Status == XASUFW_SUCCESS) {
			DeletedCount++;
		} else if (Status != XASUFW_KEYMANAGER_KEY_NOT_FOUND) {
			/* Status holds the cause from XKeyManager_DeleteKey, retain it. */
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_KEYMANAGER_KEY_DELETE_ERROR);
			goto END;
		} else {
			/* This sub-vault holds nothing for the subsystem, continue. */
		}
	}

	/**
	 * Report success only when both halves of the key pair were deleted. If the slot held
	 * neither half or only one of them (an incomplete pair), there was no complete key pair to
	 * delete, so report key not found.
	 */
	if (DeletedCount == XKEYMANAGER_ASYM_KEY_TYPE_COUNT) {
		Status = XASUFW_SUCCESS;
	} else {
		Status = XASUFW_KEYMANAGER_KEY_NOT_FOUND;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Map a subsystem and a provisioning use case to the ECC key slot reserved for that
 *		pairing in the ASU-owned exportable vault. Each use case owns a disjoint range of
 *		XASU_MAX_CHANNELS_SUPPORTED slots and the slot within a range is the subsystem's
 *		channel index plus one, so the unwrap use case occupies slots
 *		1..XASU_MAX_CHANNELS_SUPPORTED and the authenticated-encryption use case occupies
 *		the range above it. A subsystem can therefore hold one key pair per use case
 *		without the two ever resolving to the same slot.
 *
 * @param	SubSystemId	Subsystem whose reserved slot is requested.
 * @param	SlotPtr		Pointer that receives the 1-based slot on success.
 * @param	KeyUseCase	Provisioning use case the slot is requested for. Selects the range.
 *
 * @return
 *	- XASUFW_SUCCESS, if the use case is supported and the subsystem maps to a channel.
 *	- XASUFW_KEYMANAGER_INVALID_PARAM, if SlotPtr is NULL or the subsystem is not assigned to
 *		any channel.
 *	- XASUFW_KEYMANAGER_INVALID_PROVISIONING_USE_CASE, if the use case is not exactly one of
 *		the supported provisioning use cases.
 *
 *************************************************************************************************/
static s32 XProvision_GetKeySlot(u32 SubSystemId, u16 *SlotPtr, u8 KeyUseCase)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	volatile u32 Index = 0U;
	u16 SlotBase = 0U;

	if (SlotPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Select the reserved range for the use case. The comparison is an exact match rather than
	 * a bit test, so a value carrying more than one use case is refused: a provisioning key
	 * pair holds exactly one. An unrecognised use case is refused rather than defaulted to a
	 * range, so a caller can never land on the slot of another use case.
	 */
	if (KeyUseCase == XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE) {
		SlotBase = 0U;
	} else if (KeyUseCase == XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE) {
		SlotBase = (u16)XASU_MAX_CHANNELS_SUPPORTED;
	} else {
		Status = XASUFW_KEYMANAGER_INVALID_PROVISIONING_USE_CASE;
		goto END;
	}

	for (Index = 0U; Index < XASU_MAX_CHANNELS_SUPPORTED; Index++) {
		if (SubSystemId == XAsu_GetSubsysIdFromIpiMask(XAsu_GetIpiMask(Index))) {
			*SlotPtr = (u16)(Index + XKEYMANAGER_KEY_INDEX_OFFSET + SlotBase);
			Status = XASUFW_SUCCESS;
			break;
		}
	}

	/** The subsystem holds no channel, so no slot is reserved for it. */
	if (Index == XASU_MAX_CHANNELS_SUPPORTED) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Derive the provisioning private key ID for a reserved slot in the ASU-owned
 *		exportable vault and confirm the slot is occupied by a key that carries the
 *		requested provisioning use case. Lets the key pair generation guard test the slot
 *		without the caller ever holding the KeyId. Because the ID is derived rather than
 *		remembered, it also resolves after the vault has been exported and imported again.
 *		The caller resolves the slot first with XProvision_GetKeySlot(), so the
 *		mapping is not repeated here.
 *
 * @param	KeySlot		Reserved slot of the subsystem whose private key ID is requested.
 * @param	KeyIdPtr	Pointer that receives the private key ID of the reserved slot. It is
 *				written whenever the ID can be composed, whether or not the slot
 *				currently holds a key, because the key pair generation guard needs
 *				it precisely when the slot is free.
 * @param	KeyUseCase	Provisioning use case the slot must carry. Must be the same use case
 *				the slot was resolved for, so the guard tests the key pair which
 *				would actually be replaced.
 *
 * @return
 *	- XASUFW_SUCCESS, if the slot holds a key carrying the requested use case.
 *	- XASUFW_KEYMANAGER_INVALID_PARAM, if KeyIdPtr is NULL.
 *	- XASUFW_KEYMANAGER_KEY_NOT_FOUND, if the slot holds no key. This is the only value the
 *	  key pair generation guard treats as a free slot.
 *	- Other error codes propagated from XKeyManager_GetKeyObjectPtr, if the vault cannot be
 *	  resolved or the key in the slot is expired, does not carry that use case or cannot be
 *	  accessed.
 *
 *************************************************************************************************/
static s32 XProvision_GetKeyId(u16 KeySlot, u32 *KeyIdPtr, u8 KeyUseCase)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const u8 *KeyObjectPtr = NULL;
	u32 KeyId = 0U;

	if (KeyIdPtr == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	KeyId = XASU_KM_COMPOSE_KEY_ID(XKEYMANAGER_ASU_EXPORTABLE_VAULT_ID,
					XASU_ECC_PVT_SUBVAULT_ID, KeySlot);

	/**
	 * Report the composed ID regardless of what the lookup below decides. It names the
	 * reserved slot rather than the key currently sitting in it, and the key pair generation
	 * guard needs it on the path where that slot turns out to be empty.
	 */
	*KeyIdPtr = KeyId;

	/**
	 * Validate the slot without consuming a use of the key, since the caller fetches the key
	 * object itself when the key is actually used. Nothing here reads the resolved object, so
	 * it is taken through the const output parameter and left untouched.
	 */
	Status = XKeyManager_GetKeyObjectPtr(KeyId, XKEYMANAGER_ASU_SUBSYSTEM_ID, KeyUseCase,
						(u8)FALSE, &KeyObjectPtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the public key of a subsystem's provisioning ECC NIST P-384
 *		key pair held in the ASU-owned exportable key vault, without generating it. The use
 *		case selects which of the two reserved slot bands the pair lives in: the
 *		provisioning-material unwrap use case (ECIES) resolves to the wrapping pair and the
 *		provisioning authenticated-encryption use case resolves to the authentication pair.
 *		Both halves of a provisioning pair are stored with the same use-case value, so the
 *		private use case also matches the stored public half. Neither half nor either key ID
 *		is otherwise exposed: only the public key bytes are copied out for the Device
 *		Provisioning Certificate.
 *
 * @param	SubSystemId	Subsystem whose provisioning public key is requested. Together with
 *				the use case it selects the reserved slot within the exportable vault.
 * @param	KeyUseCase	Provisioning use case whose public key is requested, either
 *				XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE (wrapping/ECIES) or
 *				XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE (authentication). It
 *				must be the use case the pair was generated for, because it selects
 *				the reserved slot range.
 * @param	PubKeyBuf	Buffer that receives the uncompressed (X || Y) public key
 *				(XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES bytes).
 *
 * @return
 *	- XASUFW_SUCCESS, if the public key is returned successfully.
 *	- XASUFW_KEYMANAGER_INVALID_PARAM, if PubKeyBuf is NULL.
 *	- XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED, if no slot is reserved for the
 *		subsystem and use case, with the underlying cause packed into the status.
 *	- XASUFW_KEYMANAGER_KEY_NOT_FOUND, if the reserved slot holds no public key for that use
 *		case, i.e. the key pair does not exist in the exportable vault.
 *	- Error code propagated from XKeyManager_GetKeyObjectPtr, if the public key lookup fails.
 *	- Error code from Xil_SMemCpy, if copying the public key fails.
 *
 *************************************************************************************************/
static s32 XProvision_GetPubKey(u32 SubSystemId, u8 KeyUseCase, u8 *PubKeyBuf)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XKeyManager_EccPubKeyObject *PubKeyObj = NULL;
	const u8 *KeyObjectPtr = NULL;
	u16 KeySlot = 0U;
	u32 PubKeyId = 0U;

	/** Validate input parameters. */
	if (PubKeyBuf == NULL) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	/**
	 * Map the subsystem and the requested use case to their reserved slot in the exportable
	 * vault. The unwrap use case resolves to the wrapping band and the authenticated-encryption
	 * use case to the authentication band, so the same lookup serves both provisioning pairs.
	 */
	Status = XProvision_GetKeySlot(SubSystemId, &KeySlot, KeyUseCase);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_KEYMANAGER_GET_PROVISIONING_KEY_SLOT_FAILED);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	PubKeyId = XASU_KM_COMPOSE_KEY_ID(XKEYMANAGER_ASU_EXPORTABLE_VAULT_ID,
					  XASU_ECC_PUB_SUBVAULT_ID, KeySlot);

	/**
	 * Resolve the public half in the reserved slot without consuming a use of the key. Both
	 * halves of a provisioning pair carry the same use-case value, so the private use case
	 * requested here is a subset of the stored public half's use case and the lookup matches.
	 */
	Status = XKeyManager_GetKeyObjectPtr(PubKeyId, XKEYMANAGER_ASU_SUBSYSTEM_ID, KeyUseCase,
					     (u8)FALSE, &KeyObjectPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}
	PubKeyObj = (const XKeyManager_EccPubKeyObject *)(const void *)KeyObjectPtr;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Copy the uncompressed public key to the caller buffer. */
	Status = Xil_SMemCpy(PubKeyBuf, XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES,
			     PubKeyObj->PublicKey, XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES,
			     XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates and returns the Device Provisioning Certificate (DPC) for the
 *		requesting subsystem (SAS section 12.16.2).
 *
 *		The provisioning authentication key pair (ECC NIST P-384) and provisioning wrapping
 *		key pair (ECIES over NIST P-384) are looked up in the ASU-owned exportable key vault
 *		and generated with the TRNG if absent. A UDE-signed X.509 certificate is generated
 *		over each public key, and the DPC container is assembled directly in the caller
 *		buffer as: Device DNA, the length-prefixed authentication certificate, the
 *		length-prefixed wrapping certificate, and a UDE signature (ECC P-384) over all the
 *		preceding bytes. The active UDE key pair signs both the certificates and the
 *		container; its private key is decrypted inside the ASU and zeroized before returning.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaPtr		Pointer to the SHA instance used to hash the DPC container. The Get
 *				DPC command reserves the SHA3 engine, so this is the SHA3 (XSHA_1)
 *				instance allocated by the resource handler.
 * @param	ParamsPtr	Pointer to the XAsu_DevProvisionCertParams structure.
 * @param	SubsystemId	Subsystem ID of the requesting subsystem (selects the exportable
 *				vault slots that hold its provisioning key pairs).
 * @param	DpcActualSizePtr	Pointer to the response buffer location that receives the
 *					actual DPC size in bytes. The client copies this value to
 *					ParamsPtr->DpcActualSizeAddr during the callback.
 *
 * @return
 *	- XASUFW_SUCCESS, if the DPC is generated successfully.
 *	- XASUFW_PROVISION_DPC_INVALID_PARAM, if any input parameter is invalid.
 *	- XASUFW_PROVISION_DPC_UDE_KEY_FAIL, if the active UDE key pair cannot be obtained.
 *	- XASUFW_PROVISION_DPC_KEY_FETCH_FAIL, if a provisioning key pair cannot be obtained or
 *		generated.
 *	- XASUFW_PROVISION_DPC_CERT_GEN_FAIL, if a UDE-signed certificate cannot be generated.
 *	- XASUFW_PROVISION_DPC_ASSEMBLE_FAIL, if a DPC field cannot be written to the caller buffer.
 *	- XASUFW_PROVISION_DPC_SIGN_FAIL, if the UDE signature over the container cannot be computed.
 *	- XASUFW_PROVISION_DPC_COPY_FAIL, if the signature cannot be copied to the caller.
 *	- XASUFW_OCP_DISABLED, if the OCP module is not enabled in the build.
 *
 *************************************************************************************************/
s32 XProvision_GetDevProvisionCert(XAsufw_Dma *DmaPtr, XSha *ShaPtr,
				   const XAsu_DevProvisionCertParams *ParamsPtr, u32 SubsystemId,
				   u32 *DpcActualSizePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	XFih_Var XFihDpc = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 UdePvtKey[XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES] = {0U};
	u8 HashBuf[XASU_SHA_384_HASH_LEN] = {0U};
	u8 SignBuf[XPROVISION_DPC_SIG_SIZE_IN_BYTES] = {0U};
#ifdef XASU_OCP_ENABLE
	XAsu_ShaOperationCmd ShaCmd;
	u8 UdePubKey[XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES] = {0U};
	u8 AuthPubKey[XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES] = {0U};
	u8 WrapPubKey[XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES] = {0U};
	u64 DpcBufAddr;
	u32 LenField;
	u32 Offset = 0U;
	u32 AuthCertSize = 0U;
	u32 WrapCertSize = 0U;
	u32 TotalSize = 0U;
#endif /* XASU_OCP_ENABLE */

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (ShaPtr == NULL) || (DpcActualSizePtr == NULL)) {
		Status = XASUFW_PROVISION_DPC_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_ProvisionValidateDpcParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_PROVISION_DPC_INVALID_PARAM;
		goto END;
	}

#ifdef XASU_OCP_ENABLE
	DpcBufAddr = ParamsPtr->DpcBufAddr;

	/** Obtain the active (non-revoked) UDE key pair that signs the DPC. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GetActiveUdeKeyPair(DmaPtr, UdePvtKey, UdePubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_UDE_KEY_FAIL);
		goto END_CLR;
	}

	/**
	 * Generate the provisioning authentication (ECC P-384) key pair if it does not already
	 * exist, then fetch its public key. An already-existing pair is reused rather than treated
	 * as an error, so a repeated DPC request rebuilds the certificate over the same key that
	 * was already handed out (SAS section 8.4). The pair lives in the authentication slot band,
	 * selected by the authenticated-encryption provisioning use case.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GenerateKeyPair(DmaPtr, SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE);
	if ((Status != XASUFW_SUCCESS) &&
	    (Status != XASUFW_KEYMANAGER_PROVISIONING_KEY_ALREADY_EXISTS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_KEY_FETCH_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GetPubKey(SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE, AuthPubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_KEY_FETCH_FAIL);
		goto END_CLR;
	}

	/**
	 * Generate the provisioning wrapping (ECIES P-384) key pair if it does not already exist,
	 * reusing the common provisioning key pair generator, then fetch its public key. An
	 * already-existing pair is not an error here. The pair lives in the wrapping slot band,
	 * selected by the provisioning-material unwrap (ECIES) use case.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GenerateKeyPair(DmaPtr, SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE);
	if ((Status != XASUFW_SUCCESS) &&
	    (Status != XASUFW_KEYMANAGER_PROVISIONING_KEY_ALREADY_EXISTS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_KEY_FETCH_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GetPubKey(SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE, WrapPubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_KEY_FETCH_FAIL);
		goto END_CLR;
	}

	/** DPC field 1: Device DNA. Ensure the caller buffer can hold the DNA field. */
	if ((ParamsPtr->DpcBufSize < Offset) ||
	    ((ParamsPtr->DpcBufSize - Offset) < XPROVISION_DPC_DNA_SIZE_IN_BYTES)) {
		Status = XASUFW_PROVISION_DPC_ASSEMBLE_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)EFUSE_CACHE_DNA_0, DpcBufAddr + Offset,
			       XPROVISION_DPC_DNA_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_ASSEMBLE_FAIL);
		goto END_CLR;
	}
	Offset += XPROVISION_DPC_DNA_SIZE_IN_BYTES;

	/**
	 * DPC field 2: length-prefixed UDE-signed authentication key certificate. The certificate
	 * is generated directly into the caller buffer, after its 4-byte length prefix, then the
	 * length is written back.
	 */
	if ((ParamsPtr->DpcBufSize < Offset) ||
	    ((ParamsPtr->DpcBufSize - Offset) < XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES)) {
		Status = XASUFW_PROVISION_DPC_ASSEMBLE_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GenerateUdeSignedKeyCert(DmaPtr, ShaPtr, AuthPubKey, (u8)XASU_TRUE, UdePubKey,
			UdePvtKey, DpcBufAddr + Offset + XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES,
			ParamsPtr->DpcBufSize - (Offset + XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES),
			&AuthCertSize);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_CERT_GEN_FAIL);
		goto END_CLR;
	}
	/** The certificate length prefix is stored in big-endian (network) byte order. */
	LenField = Xil_Htonl(AuthCertSize);
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&LenField, DpcBufAddr + Offset,
			       XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_ASSEMBLE_FAIL);
		goto END_CLR;
	}
	Offset += XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES + AuthCertSize;

	/** DPC field 3: length-prefixed UDE-signed wrapping key certificate. */
	if ((ParamsPtr->DpcBufSize < Offset) ||
	    ((ParamsPtr->DpcBufSize - Offset) < XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES)) {
		Status = XASUFW_PROVISION_DPC_ASSEMBLE_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GenerateUdeSignedKeyCert(DmaPtr, ShaPtr, WrapPubKey, (u8)XASU_FALSE, UdePubKey,
			UdePvtKey, DpcBufAddr + Offset + XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES,
			ParamsPtr->DpcBufSize - (Offset + XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES),
			&WrapCertSize);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_CERT_GEN_FAIL);
		goto END_CLR;
	}
	/** The certificate length prefix is stored in big-endian (network) byte order. */
	LenField = Xil_Htonl(WrapCertSize);
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&LenField, DpcBufAddr + Offset,
			       XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_ASSEMBLE_FAIL);
		goto END_CLR;
	}
	Offset += XPROVISION_DPC_LEN_FIELD_SIZE_IN_BYTES + WrapCertSize;

	/** Ensure the caller buffer can hold the UDE signature field before writing it. */
	if ((ParamsPtr->DpcBufSize < Offset) ||
	    ((ParamsPtr->DpcBufSize - Offset) < XPROVISION_DPC_SIG_SIZE_IN_BYTES)) {
		Status = XASUFW_PROVISION_DPC_ASSEMBLE_FAIL;
		goto END_CLR;
	}
	/**
	 * DPC field 4: UDE signature over all preceding bytes. Compute the SHA-384 digest of the
	 * assembled fields (on the SHA3 engine) and sign it with the active UDE private key
	 * (ECC NIST P-384).
	 */
	ShaCmd.ShaMode = XASU_SHA_MODE_384;
	ShaCmd.DataAddr = DpcBufAddr;
	ShaCmd.DataSize = Offset;
	ShaCmd.HashAddr = (u64)(UINTPTR)HashBuf;
	ShaCmd.HashBufSize = XASU_SHA_384_HASH_LEN;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaPtr, DmaPtr, &ShaCmd);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_SIGN_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGenerateSignature(DmaPtr, XASU_ECC_NIST_P384,
			XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, (u64)(UINTPTR)UdePvtKey, NULL,
			(u64)(UINTPTR)HashBuf, XASU_SHA_384_HASH_LEN, (u64)(UINTPTR)SignBuf);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_SIGN_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)SignBuf, DpcBufAddr + Offset,
			       XPROVISION_DPC_SIG_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_COPY_FAIL);
		goto END_CLR;
	}
	Offset += XPROVISION_DPC_SIG_SIZE_IN_BYTES;
	TotalSize = Offset;

	/**
	 * Return the actual size of the assembled DPC to the caller through the response buffer.
	 * The client copies this value from the response buffer to ParamsPtr->DpcActualSizeAddr
	 * during the callback.
	 */
	*DpcActualSizePtr = TotalSize;

	/**
	 * Record the DPC-specific success sentinel in the global redundant ReturnStatus. The command
	 * layer copies it into the response AdditionalStatus, where the client redundantly confirms
	 * the DPC was generated successfully, hardening the result against a glitched Status.
	 */
	ReturnStatus = XASUFW_PROVISION_DPC_SUCCESS;
#else
	ASSIGN_VOLATILE(Status, XASUFW_OCP_DISABLED);
#endif /* XASU_OCP_ENABLE */

END_CLR:
	/** Zeroize the decrypted UDE private key and the local hash/signature working buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihDpc, ClearStatus, UdePvtKey,
		  XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	ClearStatus = Xil_SecureZeroize(HashBuf, XASU_SHA_384_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	ClearStatus = Xil_SecureZeroize(SignBuf, XPROVISION_DPC_SIG_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function deletes the requesting subsystem's provisioning authentication and
 *		wrapping key pairs from the ASU-owned exportable key vault (SAS section 12.16.3), so
 *		that the next Get Device Provisioning Certificate request regenerates them.
 *
 * @param	SubsystemId	Subsystem ID of the requesting subsystem (selects the exportable
 *				vault slots that hold its provisioning key pairs).
 *
 * @return
 *	- XASUFW_SUCCESS, if the provisioning key pairs are deleted successfully.
 *	- XASUFW_PROVISION_DPC_DELETE_FAIL, with the KeyManager error embedded, if the deletion
 *		fails.
 *
 *************************************************************************************************/
s32 XProvision_DeleteDevProvisionCert(u32 SubsystemId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/**
	 * Delete the provisioning authentication key pair from its reserved slot band. A slot that
	 * holds no key pair is not an error here: the goal is that the next Get Device Provisioning
	 * Certificate request regenerates the keys, so a missing pair already satisfies that.
	 */
	Status = XProvision_DeleteKeyPair(SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE);
	if ((Status != XASUFW_SUCCESS) && (Status != XASUFW_KEYMANAGER_KEY_NOT_FOUND)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_DELETE_FAIL);
		goto END;
	}

	/** Delete the provisioning wrapping (ECIES) key pair from its reserved slot band. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_DeleteKeyPair(SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE);
	if ((Status != XASUFW_SUCCESS) && (Status != XASUFW_KEYMANAGER_KEY_NOT_FOUND)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_DPC_DELETE_FAIL);
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

#ifdef XASU_ECIES_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function decodes one CREATE_KEY_VAULT provisioning instruction record and
 *		creates the requested key vault. Wire capacities are mapped to the sub-vault params
 *		by name. Record byte 8 must explicitly select a user vault from 2 through 63.
 *		The ML-DSA and ML-KEM capacities are mapped like every other sub-vault; a non-zero
 *		SLH-DSA capacity is rejected because no SLH-DSA sub-vault exists to honour it.
 *
 * @param	RecPtr		Pointer to the start of the instruction record (its 8-byte header).
 * @param	Size		Payload size of the record (bytes after the 8-byte header).
 * @param	SubSystemId	Subsystem that owns the created vault.
 * @param	IpiMask		IPI mask of the requesting channel.
 *
 * @return
 *	- XASUFW_SUCCESS, if the vault is created.
 *	- XASUFW_PROVISION_INSTR_PROCESS_FAIL, if the payload size is not exact, an SLH-DSA
 *	  capacity is requested, the requested vault ID is out of range or already in use, or
 *	  vault creation fails.
 *
 *************************************************************************************************/
static s32 XProvision_ExecCreateKeyVault(const u8 *RecPtr, u32 Size, u32 SubSystemId, u32 IpiMask)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAsu_KeyManagerSubVaultParams SubVaultParams = {0U};
	u16 Caps[XASU_PROVISION_CREATE_CAP_COUNT];
	u32 VaultId = 0U;
	u32 Index;

	/** Require the complete CREATE_KEY_VAULT payload. */
	if (Size != XASU_PROVISION_CREATE_VAULT_PAYLOAD_SIZE) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END;
	}

	/** Read the 15 contiguous capacities in wire order. */
	for (Index = 0U; Index < XASU_PROVISION_CREATE_CAP_COUNT; Index++) {
		Caps[Index] = XAsu_ProvisionReadU16Le(
				&RecPtr[XASU_PROVISION_CREATE_CAP_OFFSET(Index)]);
	}

	/*
	 * SLH-DSA has no sub-vault and no member in XAsu_KeyManagerSubVaultParams, so a non-zero
	 * capacity cannot be honoured. Rejecting is what stops a host believing it provisioned
	 * capacity it did not get.
	 */
	/* TODO: Support SLH-DSA sub-vaults once they exist. */
	if ((Caps[XASU_PROVISION_CREATE_CAP_IDX_SLHDSA_PVT] != 0U) ||
			(Caps[XASU_PROVISION_CREATE_CAP_IDX_SLHDSA_PUB] != 0U)) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END;
	}

	/**
	 * Take the explicit user Key Vault ID and map the capacities by name. Wire order and struct
	 * order differ, so every capacity is mapped individually rather than copied as a block.
	 */
	SubVaultParams.VaultId = RecPtr[XASU_PROVISION_CREATE_VAULT_ID_OFFSET];
	SubVaultParams.AESKeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_AES];
	SubVaultParams.IVVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_IV];
	SubVaultParams.KDFHmacKeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_KDF_HMAC];
	SubVaultParams.RSAPvtKeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_RSA_PVT];
	SubVaultParams.RSAPubKeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_RSA_PUB];
	SubVaultParams.ECCPvtKeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_ECC_PVT];
	SubVaultParams.ECCPubKeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_ECC_PUB];
	SubVaultParams.LMSKeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_LMS];
	SubVaultParams.MldsaPvtKeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_MLDSA_PVT];
	SubVaultParams.MldsaPubKeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_MLDSA_PUB];
	SubVaultParams.MlkemPvtKeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_MLKEM_PVT];
	SubVaultParams.MlkemPubKeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_MLKEM_PUB];
	SubVaultParams.X509KeyVaultCapacity = Caps[XASU_PROVISION_CREATE_CAP_IDX_X509];
	SubVaultParams.AccessRights =
		XAsu_ProvisionReadU16Le(&RecPtr[XASU_PROVISION_CREATE_ACCESS_OFFSET]);
	SubVaultParams.Restrictions = RecPtr[XASU_PROVISION_CREATE_RESTRICTIONS_OFFSET];

	Status = XAsu_ProvisionValidateCreateKeyVaultParams(&SubVaultParams);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_CreateKeyVault(&SubVaultParams, SubSystemId, IpiMask, &VaultId,
			(u8)XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_INSTR_PROCESS_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function decodes one STORE_KEY provisioning instruction record and stores the
 *		key into a vault. Record bytes 8..23 are laid out exactly like
 *		XAsu_KeyManagerKeyMetadata and are taken as one copy; the key data follows at
 *		record offset 24 and is stored verbatim. The requested Key ID must explicitly select
 *		a 1-based slot in a user-owned vault; auto-assignment and ASU-owned vaults are rejected.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	RecPtr		Pointer to the start of the instruction record (its 8-byte header).
 * @param	Size		Payload size of the record (bytes after the 8-byte header).
 * @param	SubSystemId	Subsystem that owns the stored key.
 *
 * @return
 *	- XASUFW_SUCCESS, if the key is stored.
 *	- XASUFW_PROVISION_INSTR_PROCESS_FAIL, if the payload does not exactly contain the metadata
 *	  and declared key, the requested key slot is unusable, or the store fails.
 *
 *************************************************************************************************/
static s32 XProvision_ExecStoreKey(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, const u8 *RecPtr,
		u32 Size, u32 SubSystemId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAsu_KeyManagerParams KmParams = {0U};
	u32 DataLen = 0U;
	u32 KeyId = 0U;

	/** The payload must hold at least the 16-byte metadata (plus the L-byte key data). */
	if (Size < XPROVISION_INSTR_STORE_META_LEN) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END;
	}

	/**
	 * Record bytes 8..23 are laid out exactly like XAsu_KeyManagerKeyMetadata (16 bytes, no
	 * padding, little-endian scalars on this target), so the metadata is taken as one copy.
	 * Any change to the wire layout or to the struct members must keep the two in step.
	 * KeyId is the requested 1-based slot within a user-owned sub-vault and is honoured by the
	 * store.
	 */
	Status = Xil_SMemCpy((void *)&KmParams.KeyMetadata, XPROVISION_INSTR_STORE_META_LEN,
			(const void *)&RecPtr[XPROVISION_INSTR_STORE_META_OFFSET],
			XPROVISION_INSTR_STORE_META_LEN, XPROVISION_INSTR_STORE_META_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END;
	}

	/**
	 * Validate the wire metadata and resolve the serialized object size. RSA key objects and
	 * ECC public keys carry more bytes than the key length retained in their metadata.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_ProvisionValidateStoreKeyMetadata(&KmParams.KeyMetadata, &DataLen);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END;
	}

	/** The payload must end exactly after the complete serialized key object. */
	if (DataLen != (Size - XPROVISION_INSTR_STORE_META_LEN)) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END;
	}

	/** The complete key object is copied verbatim from record offset 24 by the store operation. */
	KmParams.KeyObjectAddr = (u64)(UINTPTR)&RecPtr[XPROVISION_INSTR_STORE_DATA_OFFSET];
	/**
	 * KeyIdAddr is the client-side address the assigned Key ID is returned to. The store only
	 * checks it is non-zero and never dereferences it, so the local Id is used here.
	 */
	KmParams.KeyIdAddr = (u64)(UINTPTR)&KeyId;
	KmParams.WrappedInputLen = 0U;
	KmParams.KeyObjectLen = DataLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_StoreKeyInVault(DmaPtr, AesInstancePtr, &KmParams, &KeyId, SubSystemId);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_INSTR_PROCESS_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function executes one XOR provisioning instruction record, combining the two
 *		AES key splits named by the record into a single key held in the destination key
 *		vault slot. The splits are read in place from the vault and combined inside the ASU,
 *		so no key material crosses the record window. The output key ID on the wire is a
 *		full composite key ID, so it explicitly selects the destination user vault, the AES
 *		sub-vault and a non-zero slot within it.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	RecPtr		Pointer to the start of the instruction record (its 8-byte header).
 * @param	Size		Payload size of the record (bytes after the 8-byte header).
 * @param	SubSystemId	Subsystem that owns the input key splits and the combined key.
 *
 * @return
 *	- XASUFW_SUCCESS, if the combined key is stored.
 *	- XASUFW_PROVISION_INSTR_PROCESS_FAIL, if the payload is invalid or the combine fails.
 *
 *************************************************************************************************/
static s32 XProvision_ExecXorKeys(XAsufw_Dma *DmaPtr, const u8 *RecPtr, u32 Size, u32 SubSystemId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAsu_KmXorKeyParams XorParams = {0U};
	u32 KeyId = 0U;

	/** The XOR payload is exactly 24 bytes. */
	if (Size != XPROVISION_INSTR_XOR_PAYLOAD_LEN) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END;
	}

	XorParams.SplitKeyId1 = XAsu_ProvisionReadU32Le(&RecPtr[XPROVISION_XOR_KEYID1_OFFSET]);
	XorParams.SplitKeyId2 = XAsu_ProvisionReadU32Le(&RecPtr[XPROVISION_XOR_KEYID2_OFFSET]);
	XorParams.OutKeyMetadata.CompositeKeyId =
		XAsu_ProvisionReadU32Le(&RecPtr[XPROVISION_XOR_OUT_KEYID_OFFSET]);
	XorParams.OutKeyMetadata.KeyUseCase = RecPtr[XPROVISION_XOR_USECASE_OFFSET];
	XorParams.OutKeyMetadata.KeyAttributes = RecPtr[XPROVISION_XOR_ATTRIBUTES_OFFSET];
	XorParams.OutKeyMetadata.Length =
		XAsu_ProvisionReadU16Le(&RecPtr[XPROVISION_XOR_LENGTH_OFFSET]);
	XorParams.OutKeyMetadata.EpochTime =
		XAsu_ProvisionReadU32Le(&RecPtr[XPROVISION_XOR_EPOCHTIME_OFFSET]);
	XorParams.OutKeyMetadata.UsageCount =
		XAsu_ProvisionReadU32Le(&RecPtr[XPROVISION_XOR_USAGECOUNT_OFFSET]);

	Status = XAsu_ProvisionValidateXorParams(&XorParams);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_CombineKeysXor(DmaPtr, SubSystemId, &XorParams, &KeyId);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_INSTR_PROCESS_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function executes one complete provisioning instruction record held in the
 *		record window. Implemented commands are dispatched to their executors. Every other
 *		command ID is rejected. The module ID was checked by the assembler when the record
 *		header completed.
 *
 * @param	Ctx	Pointer to the streaming record executor state, holding the record window
 *			and everything the executors need.
 *
 * @return
 *	- XASUFW_SUCCESS, if the record is executed or deliberately skipped.
 *	- XASUFW_PROVISION_INVALID_PARAM, if the state or any instance pointer is invalid.
 *	- XASUFW_PROVISION_INSTR_PROCESS_FAIL, if the record is malformed or its execution fails.
 *
 *************************************************************************************************/
static s32 XProvision_ExecRecord(const XProvision_SinkCtx *Ctx)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const u8 *Rec = NULL;
	u8 CmdId = 0U;
	u32 Size = 0U;

	/** Validate input parameters. */
	if ((Ctx == NULL) || (Ctx->RecBuf == NULL) || (Ctx->DmaPtr == NULL) ||
			(Ctx->AesInstancePtr == NULL)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	Rec = Ctx->RecBuf;
	CmdId = Rec[XPROVISION_INSTR_CMDID_OFFSET];
	Size = (u32)XAsu_ProvisionReadU16Le(&Rec[XPROVISION_INSTR_SIZE_OFFSET]);

	switch (CmdId) {
		case XASU_KM_CREATE_KEYVAULT_CMD_ID:
			Status = XProvision_ExecCreateKeyVault(Rec, Size, Ctx->SubSystemId,
					Ctx->IpiMask);
			break;
		case XASU_KM_STORE_KEY_CMD_ID:
			Status = XProvision_ExecStoreKey(Ctx->DmaPtr, Ctx->AesInstancePtr, Rec,
					Size, Ctx->SubSystemId);
			break;
		case XASU_KM_XOR_KEY_CMD_ID:
			Status = XProvision_ExecXorKeys(Ctx->DmaPtr, Rec, Size, Ctx->SubSystemId);
			break;
		default:
			Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
			break;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the shared working region used by the atomic AEPI wrap path.
 *
 * @param	ScratchPtr	Pointer that receives the wrap working-set address.
 *
 * @return
 *	- XASUFW_SUCCESS, if the working region is available and large enough.
 *	- XASUFW_PROVISION_INVALID_PARAM, if ScratchPtr is NULL or shared storage is unavailable.
 *	- XASUFW_PROVISION_SCRATCH_TOO_SMALL, if the shared storage cannot hold the working set.
 *
 *************************************************************************************************/
static s32 XProvision_GetWrapScratch(XProvision_WrapScratch **ScratchPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 ScratchLen = 0U;
	u8 *SharedPtr = NULL;

	if (ScratchPtr == NULL) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	SharedPtr = XKeyManager_GetSharedScratch(&ScratchLen);
	if (SharedPtr == NULL) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	if (ScratchLen < (u32)sizeof(XProvision_WrapScratch)) {
		Status = XASUFW_PROVISION_SCRATCH_TOO_SMALL;
		goto END;
	}

	*ScratchPtr = (XProvision_WrapScratch *)(void *)SharedPtr;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function supplies one span of the caller's plaintext instruction stream to the
 *		chunk export layer.
 *
 * @param	Offset	Offset within the instruction stream.
 * @param	BlockPtr	Destination buffer for the requested bytes.
 * @param	Len	Number of bytes requested.
 *
 * @return
 *	- XASUFW_SUCCESS, if the requested bytes are copied.
 *	- XASUFW_PROVISION_INVALID_PARAM, if no source is active, a pointer is NULL or the span is
 *	  outside the caller's record stream.
 *	- XASUFW_PROVISION_INSTR_PROCESS_FAIL, if the copy fails.
 *
 *************************************************************************************************/
static s32 XProvision_BlobBlockSource(u32 Offset, u8 *BlockPtr, u32 Len)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XProvision_SourceCtx *Ctx = ProvActiveSourceCtx;

	if ((Ctx == NULL) || (Ctx->DmaPtr == NULL) || (BlockPtr == NULL) || (Len == 0U) ||
	    (Offset > Ctx->RecordBlobLen) || (Len > (Ctx->RecordBlobLen - Offset))) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	Status = XAsufw_DmaXfr(Ctx->DmaPtr, Ctx->RecordBlobAddr + (u64)Offset,
			       (u64)(UINTPTR)BlockPtr, Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function wraps one chunk-secrets block with the provisioning key schedule.
 *
 * @param	ArgPtr		Pointer to the XProvision_SecretsCtx state.
 * @param	ChunkIndex	Index of the chunk whose secrets are being wrapped.
 * @param	InPtr		Pointer to the plaintext secrets.
 * @param	OutPtr		Pointer that receives the wrapped secrets.
 *
 * @return
 *	- XASUFW_SUCCESS, if the secrets are wrapped.
 *	- XASUFW_PROVISION_INVALID_PARAM, if the state or a required pointer is invalid.
 *	- XASUFW_PROVISION_KEKS_ENCRYPT_FAIL, if the piece-wise wrap fails.
 *
 *************************************************************************************************/
static s32 XProvision_ChunkSecretsWrap(void *ArgPtr, u32 ChunkIndex, const u8 *InPtr, u8 *OutPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XProvision_SecretsCtx *Ctx = (const XProvision_SecretsCtx *)ArgPtr;
	XKeyManager_SecretsParams SecretsParams = {0U};

	if ((Ctx == NULL) || (InPtr == NULL) || (OutPtr == NULL) || (Ctx->DmaPtr == NULL) ||
	    (Ctx->AesInstancePtr == NULL) || (Ctx->KeyMaterial == NULL) ||
	    (Ctx->BaseIvAddr == 0U)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	SecretsParams.KeyMaterial = Ctx->KeyMaterial;
	SecretsParams.BaseIv = (const u8 *)(UINTPTR)Ctx->BaseIvAddr;
	SecretsParams.Input = InPtr;
	SecretsParams.Output = OutPtr;
	SecretsParams.ChunkIndex = ChunkIndex;

	Status = XKeyManager_WrapSecrets(Ctx->DmaPtr, Ctx->AesInstancePtr, &SecretsParams);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEKS_ENCRYPT_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function lays the two ASU-internal working buffers this path needs over the key
 *		manager's shared session storage and hands back a pointer to each: the chunk import
 *		working set and the record window. The storage is only lent out while no key vault
 *		export or import is running, which a provisioning image never overlaps, so neither
 *		buffer is reserved for the life of the firmware. Both hold recovered plaintext and
 *		key material and are zeroized by the caller.
 *
 * @param	WorkPtr		Output: the chunk import working set. May be NULL when the caller
 *				only wants the other buffer.
 * @param	RecBufPtr	Output: start of the record window. May be NULL when the caller only
 *				wants the other buffer.
 *
 * @return
 *	- XASUFW_SUCCESS, if every requested pointer is handed back.
 *	- XASUFW_PROVISION_INVALID_PARAM, if both output pointers are NULL or the storage is
 *	  unavailable.
 *	- XASUFW_PROVISION_SCRATCH_TOO_SMALL, if the storage no longer hosts both buffers.
 *
 * @note	XKeyManager_WorkBuf is a struct of u8 arrays, so it has an alignment of one and the
 *		cast makes no alignment assumption about the storage base.
 *
 *************************************************************************************************/
static s32 XProvision_GetScratch(XKeyManager_WorkBuf **WorkPtr, u8 **RecBufPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 ScratchLen = 0U;
	u8 *ScratchPtr = NULL;

	if ((WorkPtr == NULL) && (RecBufPtr == NULL)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	ScratchPtr = XKeyManager_GetSharedScratch(&ScratchLen);
	if (ScratchPtr == NULL) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/**
	 * The storage is sized by the key vault sessions that own it, so confirm it still hosts both
	 * buffers rather than trusting the two sizes to stay in step.
	 */
	if (ScratchLen < (u32)XPROVISION_SCRATCH_LEN) {
		Status = XASUFW_PROVISION_SCRATCH_TOO_SMALL;
		goto END;
	}

	if (WorkPtr != NULL) {
		*WorkPtr = (XKeyManager_WorkBuf *)(void *)ScratchPtr;
	}
	if (RecBufPtr != NULL) {
		*RecBufPtr = &ScratchPtr[sizeof(XKeyManager_WorkBuf)];
	}
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function unwraps the 528-byte secrets (inner IV + 16 KEKs) of one chunk by
 *		handing them to the key vault's secrets piece walk, so the provisioning image and
 *		an exported key vault blob are unwrapped by one implementation. It serves both
 *		places the secrets travel: the chunk-0 secrets carried in the image header, and the
 *		secrets of chunk k carried in the trailer of chunk k-1, where the chunk import layer
 *		calls it back.
 *
 * @param	ArgPtr		Pointer to the XProvision_SecretsCtx state.
 * @param	ChunkIndex	Zero-based index of the chunk the secrets describe.
 * @param	InPtr		Pointer to the wrapped secrets.
 * @param	OutPtr		Pointer to the buffer that receives the plaintext secrets.
 *
 * @return
 *	- XASUFW_SUCCESS, if the secrets are unwrapped.
 *	- XASUFW_PROVISION_INVALID_PARAM, if the state or any pointer is invalid.
 *	- XASUFW_PROVISION_KEKS_DECRYPT_FAIL, if the piece walk fails.
 *
 * @note	The piece walk derives each piece IV from the header IV itself, spacing chunk k
 *		past every piece the earlier chunks consumed, and loads and clears the piece key
 *		for each of its own AES calls. Nothing of the key schedule is set up here.
 *
 *************************************************************************************************/
static s32 XProvision_ChunkSecretsUnwrap(void *ArgPtr, u32 ChunkIndex, const u8 *InPtr, u8 *OutPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XProvision_SecretsCtx *Ctx = (const XProvision_SecretsCtx *)ArgPtr;
	XKeyManager_SecretsParams SecretsParams = {0U};

	/** Validate input parameters. */
	if ((Ctx == NULL) || (InPtr == NULL) || (OutPtr == NULL) || (Ctx->DmaPtr == NULL) ||
			(Ctx->AesInstancePtr == NULL) || (Ctx->KeyMaterial == NULL) ||
			(Ctx->BaseIvAddr == 0U)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/**
	 * The header IV is covered by the signature verified before this point, so it is passed
	 * by address and left untouched; the piece walk copies it before advancing it.
	 */
	SecretsParams.KeyMaterial = Ctx->KeyMaterial;
	SecretsParams.BaseIv = (const u8 *)(UINTPTR)Ctx->BaseIvAddr;
	SecretsParams.Input = InPtr;
	SecretsParams.Output = OutPtr;
	SecretsParams.ChunkIndex = ChunkIndex;

	Status = XKeyManager_UnwrapSecrets(Ctx->DmaPtr, Ctx->AesInstancePtr, &SecretsParams);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEKS_DECRYPT_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function receives one decrypted data block from the chunk import layer and
 *		assembles the provisioning instruction records out of it. The blocks form a byte
 *		stream of records, each an 8-byte header followed by Size payload bytes, so one
 *		record is buffered in the record window at a time and executed the moment its last
 *		byte arrives. Nothing larger than one record is ever resident, which is why the whole
 *		instruction stream never has to be materialised.
 *
 * @param	Offset		Offset of this block within the plaintext instruction stream.
 * @param	BlockPtr	Pointer to the decrypted block.
 * @param	Len		Number of valid bytes in the block.
 *
 * @return
 *	- XASUFW_SUCCESS, if the block is consumed and every record it completed was executed.
 *	- XASUFW_PROVISION_INVALID_PARAM, if the state or the block is invalid.
 *	- XASUFW_PROVISION_INSTR_PROCESS_FAIL, if the stream is out of order, a record carries a
 *	  foreign module ID, a record does not fit the window, or a record fails to execute.
 *
 * @note	The chunk layer's sink signature carries no context argument, so the assembler
 *		state comes from ProvActiveSinkCtx, which only holds a value while
 *		XProvision_UnwrapImage is driving a walk.
 *
 *************************************************************************************************/
static s32 XProvision_BlobBlockSink(u32 Offset, const u8 *BlockPtr, u32 Len)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XProvision_SinkCtx *Ctx = ProvActiveSinkCtx;
	u32 Pos = 0U;
	u32 CopyLen = 0U;
	u32 Size = 0U;

	/** Validate input parameters. */
	if ((Ctx == NULL) || (BlockPtr == NULL) || (Len == 0U) || (Ctx->RecBuf == NULL)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/**
	 * The assembler treats the blocks as one byte stream, so a block that does not continue
	 * where the previous one ended must stop the walk rather than be spliced into a record the
	 * host never sent.
	 */
	if (Offset != Ctx->Consumed) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END;
	}

	while (Pos < Len) {
		if (Ctx->RecFill < XPROVISION_INSTR_HDR_LEN) {
			/** Complete the record header before anything can be known about it. */
			CopyLen = XPROVISION_INSTR_HDR_LEN - Ctx->RecFill;
		} else {
			/**
			 * A record is executed and the window reset the moment RecFill reaches
			 * RecLen, so a partially filled record always has room left.
			 */
			if (Ctx->RecLen <= Ctx->RecFill) {
				Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
				goto END;
			}
			CopyLen = Ctx->RecLen - Ctx->RecFill;
		}
		if (CopyLen > (Len - Pos)) {
			CopyLen = Len - Pos;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy(&Ctx->RecBuf[Ctx->RecFill],
				XPROVISION_RECBUF_LEN - Ctx->RecFill, &BlockPtr[Pos], CopyLen,
				CopyLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
			goto END;
		}
		Ctx->RecFill += CopyLen;
		Pos += CopyLen;

		/** Parse the header as soon as it is complete, which fixes the record length. */
		if ((Ctx->RecLen == 0U) && (Ctx->RecFill == XPROVISION_INSTR_HDR_LEN)) {
			/** Every provisioning instruction is a KeyManager-module command. */
			if (Ctx->RecBuf[XPROVISION_INSTR_MODULE_OFFSET] !=
					XASU_MODULE_KEYMANAGER_ID) {
				Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
				goto END;
			}
			Size = (u32)XAsu_ProvisionReadU16Le(
					&Ctx->RecBuf[XPROVISION_INSTR_SIZE_OFFSET]);
			Ctx->RecLen = XPROVISION_INSTR_HDR_LEN + Size;

			/**
			 * A record is executed out of the window, so one that does not fit cannot
			 * be held at all. This is the bound that keeps the assembler inside the
			 * borrowed storage.
			 */
			if (Ctx->RecLen > XPROVISION_RECBUF_LEN) {
				Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
				goto END;
			}
		}

		/** Execute as soon as the record is whole, so a zero-payload record does not wait. */
		if ((Ctx->RecLen != 0U) && (Ctx->RecFill == Ctx->RecLen)) {
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XProvision_ExecRecord(Ctx);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
			Ctx->Executed++;
			Ctx->RecFill = 0U;
			Ctx->RecLen = 0U;
		}
	}

	Ctx->Consumed += Len;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function atomically builds an authenticated encrypted provisioning image. It
 *		encrypts the caller's plaintext instruction stream into the chunk chain, ECIES-wraps
 *		the seed to the requesting subsystem's provisioning wrapping public key and signs
 *		the completed header with that subsystem's provisioning authentication private key.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the SHA2 instance used for ECIES and SHA2-384.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	SubSystemId	Requesting subsystem whose provisioning key pairs are used.
 * @param	ParamsPtr	Pointer to the caller's record-stream and output-buffer parameters.
 *
 * @return
 *	- XASUFW_SUCCESS, if the complete AEPI image is built and signed.
 *	- XASUFW_PROVISION_INVALID_PARAM, if an input is invalid, the buffers overlap or the output
 *	  buffer is too small, or shared scratch storage is unavailable.
 *	- XASUFW_PROVISION_SCRATCH_TOO_SMALL, if shared storage cannot hold the working set.
 *	- XASUFW_PROVISION_KEY_NOT_FOUND, if either provisioning key pair is unavailable, with the
 *		underlying slot lookup or key object error packed into the status.
 *	- XASUFW_OVERLAY_ACTIVATE_FAIL, if the initial key-vault overlay activation fails.
 *	- XASUFW_PROVISION_KEY_WRAP_FAIL, if wrap setup, random generation or ECIES seed wrapping
 *	  fails.
 *	- XASUFW_PROVISION_KDF_FAIL, if CMAC-KDF, its key-register clear or derived-key validation
 *	  fails.
 *	- XASUFW_PROVISION_KEKS_ENCRYPT_FAIL, if the secrets of any chunk cannot be wrapped.
 *	- XASUFW_PROVISION_INSTR_PROCESS_FAIL, if chunk-chain construction, record copying or header
 *	  publication fails.
 *	- XASUFW_PROVISION_HDR_SIGN_FAIL, if the header cannot be hashed or signed.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if clearing the working region fails.
 *	- Error code from XAes_KeyClear, if final key-register cleanup fails, even after image
 *	  construction succeeded.
 *
 * @note	On failure the output buffer may contain an incomplete image and must be ignored.
 *
 *************************************************************************************************/
s32 XProvision_WrapInstructions(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
				 XAes *AesInstancePtr, u32 SubSystemId,
				 const XAsu_WrapProvisionInstructionsParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihProv = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XProvision_WrapScratch *ScratchPtr = NULL;
	XAsu_EccKeyObject AuthPvtKey = {0U};
	u8 *HdrPtr = NULL;
	u32 BodySize = 0U;
	u32 AuthPvtKeyId = 0U;
	volatile u32 KeyIndex = 0U;
	u16 AuthPvtKeySlot = 0U;
	u64 RecordEnd = 0U;
	u64 ImageEnd = 0U;
	XAsu_EciesParams EciesParams = {0U};
	XAsu_KdfParams KdfParams = {0U};
	XAsu_ShaOperationCmd ShaCmd = {0U};
	XKeyManager_ExportParams ExportParams = {0U};
	XProvision_SecretsCtx SecretsCtx = {0U};
	XProvision_SourceCtx SourceCtx = {0U};

	/** Validate resources and caller parameters before resolving any key material. */
	if ((DmaPtr == NULL) || (ShaInstancePtr == NULL) || (AesInstancePtr == NULL) ||
	    (ParamsPtr == NULL) ||
	    (XAsu_ProvisionValidateWrapParams(ParamsPtr) != XST_SUCCESS)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	BodySize = XKeyManager_CalcBodySize(ParamsPtr->RecordBlobLen);

	/** Reject arithmetic wrap and overlapping input/output ranges. */
	RecordEnd = ParamsPtr->RecordBlobAddr + (u64)ParamsPtr->RecordBlobLen;
	ImageEnd = ParamsPtr->ImageAddr + (u64)XPROVISION_HDR_MIN_TOTAL_LEN + (u64)BodySize;
	if ((RecordEnd < ParamsPtr->RecordBlobAddr) || (ImageEnd < ParamsPtr->ImageAddr) ||
	    ((ParamsPtr->RecordBlobAddr < ImageEnd) && (ParamsPtr->ImageAddr < RecordEnd))) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/** Borrow and clear the shared working region before placing secrets in it. */
	Status = XProvision_GetWrapScratch(&ScratchPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(Xil_SecureZeroize, XFihProv, Status, (u8 *)ScratchPtr,
		  (u32)sizeof(XProvision_WrapScratch));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END_CLR;
	}

	/**
	 * Resolve both key pairs before writing output. Get DPC creates these pairs; wrap does not
	 * generate replacements when either one is absent.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GetPubKey(SubSystemId,
			XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE, ScratchPtr->WrapPubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_NOT_FOUND);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GetKeySlot(SubSystemId, &AuthPvtKeySlot,
			XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_NOT_FOUND);
		goto END_CLR;
	}

	AuthPvtKeyId = XASU_KM_COMPOSE_KEY_ID(XKEYMANAGER_ASU_EXPORTABLE_VAULT_ID,
					XASU_ECC_PVT_SUBVAULT_ID, AuthPvtKeySlot);
	AuthPvtKey.CurveType = XASU_ECC_NIST_P384;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_UpdateKeyObjFromVault(DmaPtr, AuthPvtKeyId,
			(u64)(UINTPTR)&AuthPvtKey, XKEYMANAGER_ASU_SUBSYSTEM_ID,
			XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE,
			XKEYMANAGER_RSA_OP_NONE, XASU_ECC_PVT_SUBVAULT_ID);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_NOT_FOUND);
		goto END_CLR;
	}

	/** Load the key-vault overlay before using its strong-random generator. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOverlay_Activate(XOVERLAY_ALGO_KEYVAULT, DmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		goto END_CLR;
	}

	/** Draw the transient seed and the base IV used by every chunk-secrets piece. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_GenerateStrongRandom(ScratchPtr->Seed, XPROVISION_HDR_SEED_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_WRAP_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_GenerateStrongRandom(ScratchPtr->BaseIv,
			XPROVISION_HDR_AES_IV_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_WRAP_FAIL);
		goto END_CLR;
	}

	/** Derive one AES-256 key for each piece of a chunk-secrets block. */
	KdfParams.KeyObject.KeyInAddr = (u64)(UINTPTR)ScratchPtr->Seed;
	KdfParams.KeyObject.KeyInLen = (u32)XASU_AES_KEY_SIZE_256_BITS;
	KdfParams.KeyOutAddr = (u64)(UINTPTR)ScratchPtr->KeyMaterial;
	KdfParams.KeyOutLen = XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE;
	KdfParams.ContextAddr = (u64)(UINTPTR)ProvHdrKdfContext;
	KdfParams.ContextLen = XPROVISION_HDR_KDF_CONTEXT_LEN;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKdf_CmacGenerate(DmaPtr, AesInstancePtr, &KdfParams, XASU_AES_USER_KEY_7);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KDF_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(XAes_KeyClear, XFihProv, Status, AesInstancePtr, XASU_AES_USER_KEY_7);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KDF_FAIL);
		goto END_CLR;
	}
	for (KeyIndex = 0U; KeyIndex < XKEYMANAGER_SECRETS_PIECE_COUNT; KeyIndex++) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_RejectWeakKey(
				&ScratchPtr->KeyMaterial[KeyIndex * XKEYMANAGER_AES_KEY_SIZE],
				XKEYMANAGER_AES_KEY_SIZE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KDF_FAIL);
			goto END_CLR;
		}
	}
	if (KeyIndex != XKEYMANAGER_SECRETS_PIECE_COUNT) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_UpdateErrorStatus(XASUFW_LOOP_INDEX_CMP_ERROR,
				XASUFW_PROVISION_KDF_FAIL);
		goto END_CLR;
	}

	/** Build the encrypted body last-to-first, publishing the caller source only for the walk. */
	SecretsCtx.DmaPtr = DmaPtr;
	SecretsCtx.AesInstancePtr = AesInstancePtr;
	SecretsCtx.KeyMaterial = ScratchPtr->KeyMaterial;
	SecretsCtx.BaseIvAddr = (u64)(UINTPTR)ScratchPtr->BaseIv;
	SourceCtx.DmaPtr = DmaPtr;
	SourceCtx.RecordBlobAddr = ParamsPtr->RecordBlobAddr;
	SourceCtx.RecordBlobLen = ParamsPtr->RecordBlobLen;
	ProvActiveSourceCtx = &SourceCtx;

	ExportParams.BodyAddr = ParamsPtr->ImageAddr + XPROVISION_HDR_MIN_TOTAL_LEN;
	ExportParams.BodySize = BodySize;
	ExportParams.PlaintextSize = ParamsPtr->RecordBlobLen;
	ExportParams.SourceFn = XProvision_BlobBlockSource;
	ExportParams.SecretsWrapFn = XProvision_ChunkSecretsWrap;
	ExportParams.SecretsArgPtr = &SecretsCtx;
	ExportParams.WorkPtr = &ScratchPtr->Work;
	ExportParams.Chunk0SecretsPtr = ScratchPtr->Chunk0Secrets;
	ExportParams.Chunk0HashPtr = ScratchPtr->HeaderHash;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ChunkBlobExport(DmaPtr, AesInstancePtr, ShaInstancePtr, &ExportParams);
	ProvActiveSourceCtx = NULL;
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_INSTR_PROCESS_FAIL);
		goto END_CLR;
	}

	/**
	 * The chunk export no longer needs Work.ChunkBuf. Reuse it to assemble the complete header
	 * in ASU-owned memory, preserving only the IV and chunk-0 hash that crossed the export.
	 */
	HdrPtr = ScratchPtr->Work.ChunkBuf;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(HdrPtr, XPROVISION_HDR_MIN_TOTAL_LEN, 0U,
			     XPROVISION_HDR_MIN_TOTAL_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END_CLR;
	}
	HdrPtr[XPROVISION_HDR_TAG_BYTE_0_OFFSET] = (u8)'A';
	HdrPtr[XPROVISION_HDR_TAG_BYTE_1_OFFSET] = (u8)'E';
	HdrPtr[XPROVISION_HDR_TAG_BYTE_2_OFFSET] = (u8)'P';
	HdrPtr[XPROVISION_HDR_TAG_BYTE_3_OFFSET] = (u8)'I';
	XAsu_ProvisionWriteU32Le(&HdrPtr[XPROVISION_HDR_VERSION_OFFSET],
			      XPROVISION_HDR_VERSION_EXPECTED);
	XAsu_ProvisionWriteU32Le(&HdrPtr[XPROVISION_HDR_ALGTYPE_OFFSET],
			      XPROVISION_HDR_ALGTYPE_ECIES);
	XAsu_ProvisionWriteU32Le(&HdrPtr[XPROVISION_HDR_PI_SIZE_OFFSET],
			      ParamsPtr->RecordBlobLen);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(&HdrPtr[XPROVISION_HDR_IV_OFFSET],
			XPROVISION_HDR_AES_IV_LEN_IN_BYTES, ScratchPtr->BaseIv,
			XPROVISION_HDR_AES_IV_LEN_IN_BYTES, XPROVISION_HDR_AES_IV_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(&HdrPtr[XPROVISION_HDR_CHUNK0_HASH_OFFSET],
			XPROVISION_HDR_CHUNK0_HASH_LEN, ScratchPtr->HeaderHash,
			XPROVISION_HDR_HASH_LEN, XPROVISION_HDR_CHUNK0_HASH_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END_CLR;
	}

	/** Wrap chunk 0 secrets into the header after the body has handed them back. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_ChunkSecretsWrap(&SecretsCtx, 0U, ScratchPtr->Chunk0Secrets,
			&HdrPtr[XPROVISION_HDR_KEKS_OFFSET]);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	/** Prepare to ECIES-encrypt the seed into the ASU-owned header. */
	EciesParams.RxKeyAddr = (u64)(UINTPTR)ScratchPtr->WrapPubKey;
	EciesParams.TxKeyAddr = (u64)(UINTPTR)&HdrPtr[XPROVISION_HDR_ECIES_PUB_OFFSET];
	EciesParams.InDataAddr = (u64)(UINTPTR)ScratchPtr->Seed;
	EciesParams.IvAddr = (u64)(UINTPTR)ProvHdrEciesIv;
	EciesParams.OutDataAddr = (u64)(UINTPTR)&HdrPtr[XPROVISION_HDR_CIPHERTEXT_OFFSET];
	EciesParams.MacAddr = (u64)(UINTPTR)&HdrPtr[XPROVISION_HDR_ECIES_TAG_OFFSET];
	EciesParams.DataLength = XPROVISION_HDR_SEED_LEN;
	EciesParams.EccCurveType = (u8)XASU_ECC_NIST_P384;
	EciesParams.AesKeySize = (u8)XASU_AES_KEY_SIZE_256_BITS;
	EciesParams.EccKeyLength = (u8)XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES;
	EciesParams.IvLength = (u8)XASU_AES_IV_SIZE_96BIT_IN_BYTES;
	EciesParams.MacLength = (u8)XASU_AES_MAX_TAG_LENGTH_IN_BYTES;

	/** Load ECC once for both ECIES encryption and the final header signature. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOverlay_Activate(XOVERLAY_ALGO_ECC, DmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcies_Encrypt(DmaPtr, ShaInstancePtr, AesInstancePtr, &EciesParams,
				XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_WRAP_FAIL);
		goto END_CLR;
	}

	/** Hash bytes 0 through 751 and sign that digest with KEY_AUTH_ENC. */
	ShaCmd.ShaMode = XASU_SHA_MODE_384;
	ShaCmd.DataAddr = (u64)(UINTPTR)HdrPtr;
	ShaCmd.DataSize = XPROVISION_HDR_SIGNATURE_OFFSET;
	ShaCmd.HashAddr = (u64)(UINTPTR)ScratchPtr->HeaderHash;
	ShaCmd.HashBufSize = XPROVISION_HDR_HASH_LEN;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaCmd);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_HDR_SIGN_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGenerateSignature(DmaPtr, XASU_ECC_NIST_P384,
			XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES,
			AuthPvtKey.KeyAddr, NULL,
			(u64)(UINTPTR)ScratchPtr->HeaderHash, XPROVISION_HDR_HASH_LEN,
			(u64)(UINTPTR)&HdrPtr[XPROVISION_HDR_SIGNATURE_OFFSET]);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_HDR_SIGN_FAIL);
		goto END_CLR;
	}

	/** Publish the complete signed header only after all authenticated bytes are immutable. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)HdrPtr, ParamsPtr->ImageAddr,
			       XPROVISION_HDR_MIN_TOTAL_LEN, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END_CLR;
	}

	Status = XASUFW_SUCCESS;

END_CLR:
	ProvActiveSourceCtx = NULL;
	XFIH_CALL(XAes_KeyClear, XFihProv, ClearStatus, AesInstancePtr, XASU_AES_USER_KEY_7);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if (ScratchPtr != NULL) {
		ASSIGN_VOLATILE(ClearStatus, XASUFW_FAILURE);
		XFIH_CALL(Xil_SecureZeroize, XFihProv, ClearStatus, (u8 *)ScratchPtr,
			  (u32)sizeof(XProvision_WrapScratch));
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function processes a wrapped provisioning image header entirely inside the ASU:
 *		it validates the header (tag/version/algorithm), verifies the ECC P-384 signature over
 *		the header using the subsystem's provisioning authentication public key from the
 *		ASU exportable vault, ECIES-decrypts the wrapped seed with the subsystem's provisioning
 *		private key, derives one AES-256 key per secrets piece via CMAC-KDF, and drives the
 *		chunk import.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the SHA2 XSha instance (SHA2-384).
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	SubSystemId	Requesting subsystem. Selects which provisioning private key is used
 *				and owns the vaults and keys created by the instruction records.
 * @param	IpiMask		IPI mask of the requesting channel.
 * @param	ParamsPtr	Caller-supplied provisioning-header parameters.
 *
 * @return
 *	- XASUFW_SUCCESS, if the header is processed and every instruction record executed.
 *	- XASUFW_PROVISION_INVALID_PARAM, if a resource or request field is invalid, shared scratch
 *	  is unavailable, or the signed image geometry is inconsistent.
 *	- XASUFW_PROVISION_SCRATCH_TOO_SMALL, if shared storage cannot hold the working buffers.
 *	- XASUFW_PROVISION_HDR_VERIFY_FAIL, if the tag, version or algorithm is invalid, or header
 *	  hashing or signature verification fails.
 *	- XASUFW_PROVISION_KEY_NOT_FOUND, if a required authentication or unwrap key is missing,
 *		with the underlying slot lookup or key object error packed into the status.
 *	- XASUFW_OVERLAY_ACTIVATE_FAIL, if the initial ECC overlay activation fails.
 *	- XASUFW_PROVISION_KEY_UNWRAP_FAIL, if the ECIES decrypt of the seed fails.
 *	- XASUFW_PROVISION_KDF_FAIL, if CMAC-KDF, its key-register clear or derived-key validation
 *	  fails.
 *	- XASUFW_PROVISION_KEKS_DECRYPT_FAIL, if the secrets of any chunk cannot be unwrapped.
 *	- XASUFW_PROVISION_INSTR_PROCESS_FAIL, if a caller copy, chunk import, record or completeness
 *	  check fails.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if transient key material or working storage cannot be wiped.
 *	- Error code from XAes_KeyClear, if final key-register cleanup fails, even after every
 *	  instruction executed successfully.
 *
 * @note	A partially provisioned image is left in place on failure: every record commits as
 *		it runs and the stream stops at the first failure.
 *
 *************************************************************************************************/
s32 XProvision_UnwrapInstructions(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, XAes *AesInstancePtr,
		u32 SubSystemId, u32 IpiMask, const XAsu_UnwrapProvisionInstructionsParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihProv = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u64 ImageAddr = 0U;
	const u8 *HdrPtr = NULL;
	XKeyManager_WorkBuf *WorkPtr = NULL;
	u8 *RecBuf = NULL;
	u32 PiSize = 0U;
	u32 BodySize = 0U;
	u32 TotalLen = 0U;
	u32 PlaintextBytes = 0U;
	u32 EccPvtKeyId = 0U;
	volatile u32 KeyIndex = 0U;
	u16 EccPvtKeySlot = 0U;
	XKeyManager_ImportParams ImportParams = {0U};
	XProvision_SecretsCtx SecretsCtx = {0U};
	XProvision_SinkCtx SinkCtx = {0U};
	XAsu_EccKeyObject EccPvtKey = {0U};
	XAsu_EciesParams EciesParams = {0U};
	XAsu_KdfParams KdfParams = {0U};
	XAsu_ShaOperationCmd ShaCmd = {0U};
	u8 Seed[XPROVISION_HDR_SEED_LEN] = {0U};
	u8 KeyMaterial[XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE] = {0U};
	u8 HeaderHash[XPROVISION_HDR_HASH_LEN] = {0U};
	u8 BaseIv[XPROVISION_HDR_AES_IV_LEN_IN_BYTES] = {0U};

	/** Validate the input parameters. */
	if ((DmaPtr == NULL) || (ShaInstancePtr == NULL) || (AesInstancePtr == NULL) ||
	    (XAsu_ProvisionValidateUnwrapParams(ParamsPtr) != XST_SUCCESS)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	ImageAddr = ParamsPtr->ImageAddr;
	TotalLen = ParamsPtr->TotalLen;
	/**
	 * Lay the working buffers over the key manager's shared session storage before any key
	 * material exists, so every cleanup path below has the pointer it must zeroize.
	 */
	Status = XProvision_GetScratch(&WorkPtr, &RecBuf);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Snapshot the complete header once and use only ASU-owned bytes from this point onward. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, ImageAddr, (u64)(UINTPTR)RecBuf,
			       XPROVISION_HDR_MIN_TOTAL_LEN, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END;
	}
	HdrPtr = RecBuf;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Step 1a: validate the 'AEPI' tag (four ASCII bytes at offset 0). */
	if ((HdrPtr[XPROVISION_HDR_TAG_BYTE_0_OFFSET] != (u8)'A') ||
	    (HdrPtr[XPROVISION_HDR_TAG_BYTE_1_OFFSET] != (u8)'E') ||
	    (HdrPtr[XPROVISION_HDR_TAG_BYTE_2_OFFSET] != (u8)'P') ||
	    (HdrPtr[XPROVISION_HDR_TAG_BYTE_3_OFFSET] != (u8)'I')) {
		Status = XASUFW_PROVISION_HDR_VERIFY_FAIL;
		goto END;
	}

	/** Step 1b: only header version 1 is accepted. */
	if (XAsu_ProvisionReadU32Le(&HdrPtr[XPROVISION_HDR_VERSION_OFFSET]) !=
			XPROVISION_HDR_VERSION_EXPECTED) {
		Status = XASUFW_PROVISION_HDR_VERIFY_FAIL;
		goto END;
	}

	/** Only the ECIES algorithm type is supported for now. */
	if (XAsu_ProvisionReadU32Le(&HdrPtr[XPROVISION_HDR_ALGTYPE_OFFSET]) !=
			XPROVISION_HDR_ALGTYPE_ECIES) {
		Status = XASUFW_PROVISION_HDR_VERIFY_FAIL;
		goto END;
	}

	/** Resolve the trusted authentication public key for this subsystem into scratch. */
	Status = XProvision_GetPubKey(SubSystemId,
			XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE, WorkPtr->ChunkBuf);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_NOT_FOUND);
		goto END;
	}

	/**
	 * Step 1c: SHA2-384 over the header (bytes 0 up to the signature offset), then verify the
	 * ECC P-384 signature using the internally resolved provisioning authentication public key.
	 */
	ShaCmd.ShaMode = XASU_SHA_MODE_384;
	ShaCmd.DataAddr = (u64)(UINTPTR)HdrPtr;
	ShaCmd.DataSize = XPROVISION_HDR_SIGNATURE_OFFSET;
	ShaCmd.HashAddr = (u64)(UINTPTR)HeaderHash;
	ShaCmd.HashBufSize = XPROVISION_HDR_HASH_LEN;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaCmd);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_HDR_VERIFY_FAIL);
		goto END;
	}

	/** Load ECC once for both header signature verification and ECIES seed decryption. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOverlay_Activate(XOVERLAY_ALGO_ECC, DmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		goto END;
	}

	/*
	 * TODO: Add the matching SLH-DSA header-verification branch when the AEPI wire format
	 * identifies the signature algorithm and carries the corresponding signature.
	 */
	ReturnStatus = XASUFW_FAILURE;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccVerifySignature(DmaPtr, XASU_ECC_NIST_P384,
			XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES,
			(u64)(UINTPTR)WorkPtr->ChunkBuf,
			(u64)(UINTPTR)HeaderHash, XPROVISION_HDR_HASH_LEN,
			(u64)(UINTPTR)&HdrPtr[XPROVISION_HDR_SIGNATURE_OFFSET]);
	if ((Status != XASUFW_SUCCESS) ||
	    (ReturnStatus != XASUFW_ECC_SIGNATURE_VERIFIED)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_HDR_VERIFY_FAIL);
	}
	ReturnStatus = XASUFW_FAILURE;
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Locate this subsystem's provisioning ECC private key in the ASU-owned exportable vault.
	 * The key is fetched with the provisioning-unwrap use case and used in place; it never
	 * leaves the ASU.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_GetKeySlot(SubSystemId, &EccPvtKeySlot,
			XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_NOT_FOUND);
		goto END;
	}

	EccPvtKeyId = XASU_KM_COMPOSE_KEY_ID(XKEYMANAGER_ASU_EXPORTABLE_VAULT_ID,
					XASU_ECC_PVT_SUBVAULT_ID, EccPvtKeySlot);
	EccPvtKey.CurveType = XASU_ECC_NIST_P384;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_UpdateKeyObjFromVault(DmaPtr, EccPvtKeyId,
			(u64)(UINTPTR)&EccPvtKey, XKEYMANAGER_ASU_SUBSYSTEM_ID,
			XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE,
			XKEYMANAGER_RSA_OP_NONE, XASU_ECC_PVT_SUBVAULT_ID);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_NOT_FOUND);
		goto END;
	}

	/**
	 * Step 2: ECIES-decrypt the wrapped seed into an on-chip transient buffer. The crypto
	 * profile is fixed (P-384 / direct CMAC-KDF / AES-256 / fixed 12-byte IV / 16-byte MAC).
	 * The ECIES IV is an ASU-internal constant, so the recovered seed never leaves the ASU.
	 */
	EciesParams.RxKeyAddr = EccPvtKey.KeyAddr;
	EciesParams.TxKeyAddr = (u64)(UINTPTR)&HdrPtr[XPROVISION_HDR_ECIES_PUB_OFFSET];
	EciesParams.InDataAddr = (u64)(UINTPTR)&HdrPtr[XPROVISION_HDR_CIPHERTEXT_OFFSET];
	EciesParams.IvAddr = (u64)(UINTPTR)ProvHdrEciesIv;
	EciesParams.OutDataAddr = (u64)(UINTPTR)Seed;
	EciesParams.MacAddr = (u64)(UINTPTR)&HdrPtr[XPROVISION_HDR_ECIES_TAG_OFFSET];
	EciesParams.DataLength = XPROVISION_HDR_SEED_LEN;
	EciesParams.EccCurveType = (u8)XASU_ECC_NIST_P384;
	EciesParams.AesKeySize = (u8)XASU_AES_KEY_SIZE_256_BITS;
	EciesParams.EccKeyLength = (u8)XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES;
	EciesParams.IvLength = (u8)XASU_AES_IV_SIZE_96BIT_IN_BYTES;
	EciesParams.MacLength = (u8)XASU_AES_MAX_TAG_LENGTH_IN_BYTES;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcies_Decrypt(DmaPtr, ShaInstancePtr, AesInstancePtr, &EciesParams,
				XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_UNWRAP_FAIL);
		goto END_CLR;
	}

	/**
	 * Step 3: derive the secrets key material from the seed using CMAC-KDF (AES-CMAC PRF,
	 * counter mode). The secrets of a chunk are wrapped piece by piece, each piece under a key
	 * of its own, so the KDF is asked for one AES-256 key per piece in a single call rather
	 * than for one key reused across the whole buffer. The seed is written to USER_KEY_7 by
	 * the KDF; KeyInLen carries the AES key-size selector.
	 */
	KdfParams.KeyObject.KeyInAddr = (u64)(UINTPTR)Seed;
	KdfParams.KeyObject.KeyInLen = (u32)XASU_AES_KEY_SIZE_256_BITS;
	KdfParams.KeyOutAddr = (u64)(UINTPTR)KeyMaterial;
	KdfParams.KeyOutLen = XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE;
	KdfParams.ContextAddr = (u64)(UINTPTR)ProvHdrKdfContext;
	KdfParams.ContextLen = XPROVISION_HDR_KDF_CONTEXT_LEN;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKdf_CmacGenerate(DmaPtr, AesInstancePtr, &KdfParams, XASU_AES_USER_KEY_7);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KDF_FAIL);
		goto END_CLR;
	}

	/**
	 * Step 4: drop the seed out of USER_KEY_7. The secrets unwrap loads each piece key into
	 * that same user key register for the one call that uses it and clears it again, so no
	 * part of the schedule is written here; the seed the KDF left behind would otherwise
	 * outlive its use in a key register.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(XAes_KeyClear, XFihProv, Status, AesInstancePtr, XASU_AES_USER_KEY_7);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KDF_FAIL);
		goto END_CLR;
	}

	/** Load the key-vault overlay before calling its key checks and chunk importer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOverlay_Activate(XOVERLAY_ALGO_KEYVAULT, DmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		goto END_CLR;
	}

	/**
	 * A derived key of all zeros or all ones means the seed or the KDF is broken. The piece
	 * wrap does not test the keys it is handed, so each one is checked here, before any of
	 * them decrypts image material.
	 */
	for (KeyIndex = 0U; KeyIndex < XKEYMANAGER_SECRETS_PIECE_COUNT; KeyIndex++) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XKeyManager_RejectWeakKey(&KeyMaterial[KeyIndex * XKEYMANAGER_AES_KEY_SIZE],
				XKEYMANAGER_AES_KEY_SIZE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KDF_FAIL);
			goto END_CLR;
		}
	}
	if (KeyIndex != XKEYMANAGER_SECRETS_PIECE_COUNT) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_UpdateErrorStatus(XASUFW_LOOP_INDEX_CMP_ERROR,
				XASUFW_PROVISION_KDF_FAIL);
		goto END_CLR;
	}

	/**
	 * Step 5: read PI_SIZE and derive the chunk body geometry from it. PI_SIZE is covered by the
	 * signature verified above, and the body it implies must exactly consume the image the
	 * caller supplied. This both keeps the chunk walk within the request and rejects trailing
	 * bytes that are not authenticated as part of the AEPI image.
	 */
	PiSize = XAsu_ProvisionReadU32Le(&HdrPtr[XPROVISION_HDR_PI_SIZE_OFFSET]);
	if ((PiSize == 0U) || (PiSize > XPROVISION_MAX_PI_SIZE)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END_CLR;
	}

	BodySize = XKeyManager_CalcBodySize(PiSize);
	if ((BodySize == 0U) ||
			(BodySize != (TotalLen - XPROVISION_HDR_MIN_TOTAL_LEN))) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END_CLR;
	}

	/**
	 * Step 6: seed the chunk chain. The chunk-0 secrets are unwrapped straight into the import
	 * working set, and the chunk-0 digest is copied from the header, where it travels in the
	 * clear under the signature. From here the chain carries itself: the trailer of each chunk
	 * holds the secrets and digest of the next.
	 */
	SecretsCtx.DmaPtr = DmaPtr;
	SecretsCtx.AesInstancePtr = AesInstancePtr;
	SecretsCtx.KeyMaterial = KeyMaterial;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(BaseIv, XPROVISION_HDR_AES_IV_LEN_IN_BYTES,
			&HdrPtr[XPROVISION_HDR_IV_OFFSET], XPROVISION_HDR_AES_IV_LEN_IN_BYTES,
			XPROVISION_HDR_AES_IV_LEN_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END_CLR;
	}
	SecretsCtx.BaseIvAddr = (u64)(UINTPTR)BaseIv;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_ChunkSecretsUnwrap(&SecretsCtx, 0U,
			&HdrPtr[XPROVISION_HDR_KEKS_OFFSET],
			WorkPtr->ChunkSecrets);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_INSTR_PROCESS_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(WorkPtr->ExpectedHash, XPROVISION_HDR_CHUNK0_HASH_LEN,
			&HdrPtr[XPROVISION_HDR_CHUNK0_HASH_OFFSET],
			XPROVISION_HDR_CHUNK0_HASH_LEN, XPROVISION_HDR_CHUNK0_HASH_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
		goto END_CLR;
	}

	/**
	 * Step 7: drive the chunk import. Each decrypted block is handed to the record assembler,
	 * which executes every record it completes, and each trailer's secrets come back through the
	 * same unwrap that seeded chunk 0. Nothing but one record is resident at any instant.
	 */
	SinkCtx.DmaPtr = DmaPtr;
	SinkCtx.AesInstancePtr = AesInstancePtr;
	SinkCtx.RecBuf = RecBuf;
	SinkCtx.SubSystemId = SubSystemId;
	SinkCtx.IpiMask = IpiMask;

	/** Publish the assembler state for the walk, as the sink signature carries no argument. */
	ProvActiveSinkCtx = &SinkCtx;

	ImportParams.BodyAddr = ImageAddr + XPROVISION_HDR_MIN_TOTAL_LEN;
	ImportParams.BodySize = BodySize;
	ImportParams.MaxPlaintextSize = PiSize;
	ImportParams.SinkFn = XProvision_BlobBlockSink;
	ImportParams.SecretsUnwrapFn = XProvision_ChunkSecretsUnwrap;
	ImportParams.SecretsArgPtr = &SecretsCtx;
	ImportParams.WorkPtr = WorkPtr;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKeyManager_ChunkBlobImport(DmaPtr, AesInstancePtr, ShaInstancePtr, &ImportParams,
			&PlaintextBytes);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	/**
	 * The whole plaintext must have reached the assembler and the last record must have ended
	 * on the last byte; a part-filled window is a trailing partial record. PI_SIZE is non-zero,
	 * so at least one complete record must also have executed.
	 */
	if ((PlaintextBytes != PiSize) || (SinkCtx.Consumed != PiSize) ||
	    (SinkCtx.RecFill != 0U) || (SinkCtx.Executed == 0U)) {
		Status = XASUFW_PROVISION_INSTR_PROCESS_FAIL;
	}

END_CLR:
	/** Withdraw the assembler state so no later sink call can reach this frame. */
	ProvActiveSinkCtx = NULL;

	/** Clear the AES key written to the AES user key source. */
	XFIH_CALL(XAes_KeyClear, XFihProv, ClearStatus, AesInstancePtr, XASU_AES_USER_KEY_7);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Zeroize all transient key material and the borrowed working buffers. */
	ASSIGN_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFIH_CALL(Xil_SecureZeroize, XFihProv, ClearStatus, Seed, XPROVISION_HDR_SEED_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ASSIGN_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFIH_CALL(Xil_SecureZeroize, XFihProv, ClearStatus, KeyMaterial,
			XKEYMANAGER_SECRETS_KEY_MATERIAL_SIZE);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	/*
	 * Wipe the whole borrow on every exit. It holds the header snapshot before import and the
	 * chunk-0 KEKs once processing begins.
	 */
	if (WorkPtr != NULL) {
		ASSIGN_VOLATILE(ClearStatus, XASUFW_FAILURE);
		XFIH_CALL(Xil_SecureZeroize, XFihProv, ClearStatus, (u8 *)WorkPtr,
				(u32)XPROVISION_SCRATCH_LEN);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}
	return Status;
}
#endif /* XASU_ECIES_ENABLE */

#endif /* XASU_PROVISION_ENABLE */
/** @} */
