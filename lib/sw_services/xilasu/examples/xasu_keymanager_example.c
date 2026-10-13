/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keymanager_example.c
 * @addtogroup xasu_keymanager_client_example ASU KeyManager API Example Generic Usage
 * @{
 *
 * This example illustrates the usage of the ASU KeyManager client APIs to:
 *      1. Create a key vault for the calling subsystem.
 *      2. Store a raw ECC P-256 private key into the ECC private sub-vault of that vault,
 *         retrieving the server-assigned composite KeyId. This shows storing an asymmetric key;
 *         StoreKey selects the sub-vault via KeyType and copies the raw key bytes as-is.
 *      3. Generate a fresh AES-256 key directly inside the vault (GenerateAesKey), where the
 *         ASU produces the key internally and returns only its composite KeyId.
 *      4. Generate a fresh AES IV directly inside the vault (GenerateAesIv), where the ASU
 *         produces the IV internally and returns only its composite KeyId.
 *      5. Use the generated AES key and IV (referenced by their vault IDs) to AES-GCM encrypt a
 *         plaintext and then decrypt the ciphertext, verifying the GCM tag and that the decrypted
 *         data matches the original plaintext. The key/IV bytes never leave the ASU.
 *      6. Generate a fresh raw (KDF/HMAC) key directly inside the vault (GenerateRawKey),
 *         provisioned for both HMAC and KDF use, returning only its composite KeyId.
 *      7. Use the generated raw key (referenced by its vault ID) to compute an HMAC over a
 *         message, recompute it, and verify both MACs match. The key bytes never leave the ASU.
 *      8. Use the same raw key (referenced by its vault ID) to derive keying material with the
 *         KDF, re-derive it, and verify both outputs match. The key bytes never leave the ASU.
 *      9. Generate the "Alice" and "Bob" ECC P-256 key pairs directly inside the vault, backing the
 *         two ECC use cases demonstrated below: Alice is provisioned for both sign/verify and key
 *         agreement, while Bob is provisioned for key agreement only to act as the ECDH peer. Each
 *         GenerateEccKeyPair returns a public and a private composite KeyId.
 *     10. Use Alice's key pair (referenced by their vault IDs) to sign a digest and verify the
 *         signature, a self-checking sign-then-verify round-trip. Key bytes never leave the ASU.
 *     11. Derive the ECDH shared secret from both sides (Alice priv + Bob pub, and Bob priv +
 *         Alice pub) and verify the two secrets match (ECDH symmetry). The key bytes never leave
 *         the ASU.
 *     12. Store a raw RSA-4096 public key into the vault's RSA public sub-vault, returning a
 *         composite KeyId. It backs the key-wrap workflow below (the public key wraps the KEK).
 *     13. Run a key-wrap workflow: a plaintext is AES-GCM-encrypted with a client-held data-
 *         encryption key (DEK); XAsu_KeyWrap wraps it with the vaulted RSA public key, using the
 *         vaulted generated AES key as the key-encryption key (KEK), yielding the hybrid blob
 *         RSA-OAEP(KEK) || AES-KWP(DEK); StoreKey then resolves that KEK from the vault by KeyId,
 *         AES-KWP-unwraps the DEK and stores it in the AES sub-vault (no client RSA-OAEP peel);
 *         the ciphertext is AES-GCM-decrypted with the stored DEK referenced by its KeyId and
 *         the result is compared against the original plaintext (the GCM tag also verifies the
 *         vaulted DEK is byte-identical to the original). The DEK plaintext never reaches long-term
 *         storage - only its wrapped form does - which is the reason key wrapping exists.
 *     14. Export the whole populated vault into a single signed and encrypted AEKV blob
 *         (ExportKeyVault), reporting the blob size the server produced and whether the vault
 *         changed since the previous export. The blob is built as chunks of encrypted blocks
 *         chained by their digests and signed with the device UDE key, its keys are derived from
 *         a context the caller supplies, and it is bound to an off-chip revocation eFuse, so
 *         blowing that eFuse is what retires it. This is the secure off-chip backup of the vault;
 *         the key bytes never leave the ASU in the clear.
 *     15. Import the vault back from that blob (ImportKeyVault), handing back the same context the
 *         export used. Import restores into an empty region and never discards a key vault on the
 *         caller's behalf, so the vault is deleted first; the blob then rebuilds it - preserving
 *         VaultIds and KeyIds. The round-trip is then self-checked on content: the step-13
 *         ciphertext is decrypted with the imported DEK by KeyId and the plaintext is confirmed to
 *         match (the GCM tag check plus the byte-for-byte compare prove the actual key bytes
 *         survived, not just the vault layout). The signature and chunk digest checks import
 *         performs already guarantee the blob was restored intact; this check is an extra
 *         end-to-end confirmation that the key is usable under its original KeyId.
 *     16. Delete the stored ECC key (DeleteKey).
 *     17. Delete the key vault (DeleteKeyVault), releasing the DDR space it occupied. Deleting
 *         the vault also frees the generated keys, IV, ECC key pairs, the stored RSA public key and
 *         the stored DEK still resident in it.
 *
 * Key material held in the vault is never exposed to the client: each store/generate operation
 * returns only an opaque composite KeyId (or, for an IV, an IvId), and every subsequent crypto
 * operation references that KeyId/IvId via KeyObject.KeyId / IvId so the ASU resolves the raw bytes
 * internally. This is exercised by the AES-GCM round-trip (step 5), HMAC (step 7), KDF (step 8),
 * ECC sign/verify (step 10), ECDH (step 11), RSA wrap (step 13, which references the vaulted RSA
 * public key and the vaulted AES KEK by KeyId) and the key-wrap workflow's decrypt (step 13). The
 * only raw key bytes the client ever supplies are the ones it must own itself: the stored ECC
 * private key (step 2), the stored RSA public key (step 12) and the workflow's client-held
 * data-encryption key (step 13).
 *
 * Steps 16 and 17 (DeleteKey and DeleteKeyVault) run as best-effort cleanup from a common teardown
 * path: they execute even if an earlier step fails - provided the key/vault were created - so the
 * example never leaks vault resources, and a cleanup failure can only downgrade the final status.
 *
 * To build this application, the xilmailbox library must be included in the BSP and xilasu must be
 * in client mode.
 *
 * Procedure to link and compile the example for the default DDR-less designs
 * ------------------------------------------------------------------------------------------------
 * The default linker settings place a software stack, heap and data in DDR memory. For this example
 * to work, any data shared between the client (running on A78/R52/PL) and the server (running on
 * ASU) must be placed in a memory region accessible to both client and server.
 *
 * Following is the procedure to compile the example on OCM or any memory region which can be
 * accessed by the server.
 *
 *      1. Open the example linker script(lscript.ld) in the Vitis project and the section to
 *         memory mapping should be updated to point all the required sections to shared
 *         memory(OCM or TCM) using a memory region drop down selection.
 *
 *                                      OR
 *
 *      1. In the linker script(lscript.ld) the user can add a new memory section in the source tab
 *         as shown below
 *              .sharedmemory : {
 *                      . = ALIGN(4);
 *                      __sharedmemory_start = .;
 *                      *(.sharedmemory)
 *                      *(.sharedmemory.*)
 *                      *(.gnu.linkonce.d.*)
 *                      __sharedmemory_end = .;
 *              } > Shared_memory_area
 *
 *      2. In this example, the ".data" section elements that are passed by reference to the
 *         server-side should be stored in the above shared memory section. To make it happen in the
 *         below example, replace ".data" in the attribute section with ".sharedmemory".
 *
 * To keep things simple, by default the cache is disabled for this example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   pks  07/22/26 Initial release
 *       kp   08/22/26 Used composite key ID in key metadata.
 *       vm   09/15/26 Updated key vault export and import steps for the AEKV blob format
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_client.h"
#include "xasu_keymanager.h"
#include "xasu_keymanager_common.h"
#include "xasu_aes.h"
#include "xasu_aesinfo.h"
#include "xasu_hmac.h"
#include "xasu_ecc.h"
#include "xasu_eccinfo.h"
#include "xasu_kdf.h"
#include "xasu_shainfo.h"
#include "xasu_keywrap.h"
#include "xasu_rsa.h"

/************************************ Constant Definitions ***************************************/

/************************************ Type Definitions *******************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_CACHE_DISABLE

#define XASU_KM_EXAMPLE_AES_KEY_LEN_IN_BYTES	(32U) /**< Generated AES-256 key length in bytes */
#define XASU_KM_EXAMPLE_AES_IV_LEN_IN_BYTES	(16U) /**< Generated AES IV length (128-bit) */
#define XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES	(64U) /**< Round-trip plaintext/ciphertext bytes */
#define XASU_KM_EXAMPLE_AES_AAD_LEN_IN_BYTES	(16U) /**< AES-GCM additional authenticated data */
#define XASU_KM_EXAMPLE_AES_TAG_LEN_IN_BYTES	(16U) /**< AES-GCM authentication tag length */
#define XASU_KM_EXAMPLE_RAW_KEY_LEN_IN_BYTES	(32U) /**< Generated KDF/HMAC raw key length */
#define XASU_KM_EXAMPLE_HMAC_MSG_LEN_IN_BYTES	(32U) /**< Message length for the HMAC operation */
#define XASU_KM_EXAMPLE_HMAC_LEN_IN_BYTES	(32U) /**< HMAC output length (SHA2-256) in bytes */
#define XASU_KM_EXAMPLE_AES_VAULT_CAPACITY	(4U)  /**< Number of AES keys the vault can hold */
#define XASU_KM_EXAMPLE_IV_VAULT_CAPACITY	(4U)  /**< Number of IVs the vault can hold */
#define XASU_KM_EXAMPLE_KDF_HMAC_VAULT_CAPACITY	(4U)  /**< KDF/HMAC keys the vault can hold */
#define XASU_KM_EXAMPLE_ACCESS_RIGHTS		(0xFFFFU) /**< Vault access rights (all channels) */
#define XASU_KM_EXAMPLE_USAGE_COUNT		(1U) /**< Times the stored key may be used */

/* Usage count for each "Alice" ECC key, resolved twice (sign/verify in step 10 + one ECDH in
 * step 11). */
#define XASU_KM_EXAMPLE_GEN_USAGE_COUNT		(2U) /**< Uses for keys resolved twice */

/* Usage count for the generated IV: resolved twice by the AES-GCM round-trip (step 5), three times
 * by key-wrap workflow (step 13) - the raw-DEK encrypt, the stored-DEK verify re-encrypt and the
 * stored-DEK decrypt - and once more by the post-import content check (step 15). Usage counts are
 * decremented per resolution and preserved across export/import, so the sixth use must be
 * provisioned up front. */
#define XASU_KM_EXAMPLE_IV_USAGE_COUNT		(6U) /**< Total IV resolutions in the example */

/* The generated key/IV are used for both encryption and decryption. The server resolves them with
 * the ENC use case on encrypt and the DEC use case on decrypt, rejecting a mismatch with
 * KEY_USE_CASE_ERROR, so both use-case bits must be provisioned. */
#define XASU_KM_EXAMPLE_AES_USE_CASE		(XASU_KEYMANAGER_AES_ENC_USE_CASE | \
						XASU_KEYMANAGER_AES_DEC_USE_CASE)

/* The generated AES key plays two roles: the data key for the AES-GCM round-trip (step 5, resolved
 * with ENC then DEC) and the key-encryption key (KEK) for the key-wrap workflow (step 13). As the
 * KEK it is resolved with KEY_WRAP during XAsu_KeyWrap and with KEY_UNWRAP when StoreKey AES-KWP-
 * unwraps the DEK, so all four use-case bits must be provisioned. */
#define XASU_KM_EXAMPLE_AES_KEK_USE_CASE	(XASU_KEYMANAGER_AES_ENC_USE_CASE | \
						XASU_KEYMANAGER_AES_DEC_USE_CASE | \
						XASU_KEYMANAGER_AES_KEY_WRAP_USE_CASE | \
						XASU_KEYMANAGER_AES_KEY_UNWRAP_USE_CASE)

/* The generated AES key is resolved four times across the example: AES-GCM encrypt + decrypt
 * (step 5) and KEY_WRAP (wrap) + KEY_UNWRAP (StoreKey) in the key-wrap workflow (step 13). */
#define XASU_KM_EXAMPLE_AES_KEK_USAGE_COUNT	(4U) /**< GCM enc+dec (2) + wrap (1) + unwrap (1) */

/* ECC demo uses the NIST P-256 curve. The private/public key length, signature length (r||s) and
 * digest length are all derived from the curve. The vault holds a couple of ECC key pairs. */
#define XASU_KM_EXAMPLE_ECC_CURVE		(XASU_ECC_NIST_P256) /**< Curve used for the ECC demo */
#define XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES	(XASU_ECC_P256_PVT_KEY_SIZE_IN_BYTES)
#define XASU_KM_EXAMPLE_ECC_SIG_LEN_IN_BYTES	(2U * XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES)
#define XASU_KM_EXAMPLE_ECC_DIGEST_LEN_IN_BYTES	(XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES)
#define XASU_KM_EXAMPLE_ECC_NUM_KEY_IDS		(2U)  /**< Key pair generation returns two IDs */
#define XASU_KM_EXAMPLE_ECC_VAULT_CAPACITY	(4U)  /**< ECC keys the vault can hold */

/* GenerateEccKeyPair applies a single KeyUseCase to both keys of the pair; each key interprets the
 * bits per its own type (0x01 = SIGN_GEN for the private key / SIGN_VER for the public key;
 * 0x02 = KEY_AGREEMENT for both). "Alice" is provisioned for sign, verify AND key agreement so the
 * same key pair backs both the sign/verify round-trip and the ECDH exchange. "Bob" (the ECDH peer)
 * only needs key agreement. */
#define XASU_KM_EXAMPLE_ECC_SIGN_AGREE_USE_CASE	(XASU_KEYMANAGER_ECC_PVT_SIGN_GEN_USE_CASE | \
						XASU_KEYMANAGER_ECC_PUB_SIGN_VER_USE_CASE | \
						XASU_KEYMANAGER_ECC_PVT_KEY_AGREEMENT_USE_CASE | \
						XASU_KEYMANAGER_ECC_PUB_KEY_AGREEMENT_USE_CASE)
#define XASU_KM_EXAMPLE_ECC_KEY_AGREE_USE_CASE	(XASU_KEYMANAGER_ECC_PVT_KEY_AGREEMENT_USE_CASE | \
						XASU_KEYMANAGER_ECC_PUB_KEY_AGREEMENT_USE_CASE)

/* Each ECC key is resolved (usage count decremented) once per operation. Alice's private key
 * is resolved by sign + one ECDH, and public key by verify + the other ECDH, so each Alice key is
 * provisioned for two uses (XASU_KM_EXAMPLE_GEN_USAGE_COUNT). Bob is the ECDH peer: his private
 * key is resolved by one ECDH and his public key by the other, so each Bob key needs one use. */
#define XASU_KM_EXAMPLE_ECC_PEER_USAGE_COUNT	(1U) /**< Uses per Bob (ECDH peer) key */

/* The single raw (KDF/HMAC) key is exercised by both HMAC and KDF. The server resolves it with the
 * HMAC use case on an HMAC compute and the KDF use case on a KDF derivation, so both use-case bits
 * must be provisioned. It is resolved (usage count decremented) once per operation: two HMAC
 * computes + two KDF derivations = four uses. */
#define XASU_KM_EXAMPLE_RAW_KEY_USE_CASE	(XASU_KEYMANAGER_KDF_HMAC_KDF_USE_CASE | \
						XASU_KEYMANAGER_KDF_HMAC_HMAC_USE_CASE)
#define XASU_KM_EXAMPLE_RAW_USAGE_COUNT		(4U) /**< Raw key uses: 2 HMAC + 2 KDF */

/* RSA key-transport material for the key-wrap workflow. Only the 4096-bit public key is stored
 * (into the RSA public sub-vault): it wraps the key-encryption key (KEK) during XAsu_KeyWrap. The
 * private key is not needed - because the KEK is a vaulted AES key, StoreKey recovers it from the
 * vault by KeyId. The vault forces a fixed public exponent (XASU_KM_EXAMPLE_RSA_PUB_EXP) on every
 * stored/resolved RSA key, so the stored public key must use that same exponent. */
#define XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES	(512U) /**< RSA-4096 modulus/exponent bytes */
#define XASU_KM_EXAMPLE_RSA_PUB_EXP		(0x1000100U) /**< Fixed RSA public exponent enforced by
								the vault (byte-swapped 0x00010001,
								i.e. 65537) for stored RSA keys */
#define XASU_KM_EXAMPLE_RSA_VAULT_CAPACITY	(1U) /**< RSA public keys the vault can hold
							(one public key is stored) */

/* The public key is resolved by XAsu_KeyWrap with the key-transport use case, so it is provisioned
 * with that use case. */
#define XASU_KM_EXAMPLE_RSA_PUB_USE_CASE	(XASU_KEYMANAGER_RSA_PUB_KEY_TRANSPORT_USE_CASE)

/* RSA key-transport key is long-lived: provision it as non-depleting so the wrap workflow (and any
 * repeat run after the export/import round-trip) never exhausts its usage budget. */
#define XASU_KM_EXAMPLE_RSA_USAGE_COUNT		(XASU_KM_USAGE_COUNT_NON_DEPLETING_VALUE)

/* RSA key-wrap workflow. A client-held data-encryption key (DEK) AES-GCM-encrypts the plaintext;
 * XAsu_KeyWrap then wraps the DEK with the vaulted RSA public key, using the vaulted generated AES
 * key as the key-encryption key (KEK), producing a hybrid blob:
 *   [ RSA-OAEP(KEK) : RsaKeySize bytes ] || [ AES-KWP(DEK) : KWP bytes ]
 * Because the KEK is itself a vaulted key, StoreKey AES-KWP-unwraps the DEK directly by resolving
 * that KEK from the vault by KeyId, so the workflow hands StoreKey only the AES-KWP portion
 * (blob + RsaKeySize) plus the KEK's KeyId. The ciphertext is finally decrypted by the stored DEK's
 * KeyId (AES-GCM, tag-verified) and compared against the original plaintext - the
 * DEK plaintext never reaches long-term storage, only its wrapped form. */
#define XASU_KM_EXAMPLE_DEK_LEN_IN_BYTES	(32U) /**< Data-encryption key (AES-256) bytes */

/* The stored DEK is resolved three times: in the key-wrap workflow (step 13) to re-encrypt the
 * plaintext for the stored-key-equals-DEK check and to decrypt the ciphertext, and again after the
 * export/import round-trip (step 15) to decrypt once more as the import content-integrity check.
 * Usage counts are preserved across export/import, so the third use must be provisioned. */
#define XASU_KM_EXAMPLE_DEK_USAGE_COUNT		(3U) /**< Total stored-DEK uses in the example */
#define XASU_KM_EXAMPLE_OAEP_LABEL_SIZE_IN_BYTES	(32U) /**< RSA-OAEP optional label bytes */

/* AES Key Wrap with Padding output for an 8-byte-aligned key is the key length plus one 8-byte
 * semi-block (no padding needed for a 32-byte DEK): 32 + 8 = 40 bytes. The full wrap blob prepends
 * the RSA-OAEP-encoded KEK (RsaKeySize = 512 bytes): 512 + 40 = 552 bytes. StoreKey consumes only
 * the trailing 40-byte AES-KWP portion; the 512-byte RSA-OAEP prefix is produced by the wrap but
 * unused during StoreKey because the KEK is recovered from the vault by KeyId instead. */
#define XASU_KM_EXAMPLE_AES_KWP_SEMIBLOCK_IN_BYTES	(8U) /**< AES-KWP semi-block in bytes */
#define XASU_KM_EXAMPLE_AES_KWP_BLOB_SIZE_IN_BYTES	(XASU_KM_EXAMPLE_DEK_LEN_IN_BYTES + \
							XASU_KM_EXAMPLE_AES_KWP_SEMIBLOCK_IN_BYTES)
#define XASU_KM_EXAMPLE_WRAPPED_BLOB_SIZE_IN_BYTES	(XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES + \
							XASU_KM_EXAMPLE_AES_KWP_BLOB_SIZE_IN_BYTES)

#define XASU_KM_EXAMPLE_KDF_CONTEXT_LEN_IN_BYTES	(32U) /**< KDF fixed-input/context length */
#define XASU_KM_EXAMPLE_KDF_OUT_LEN_IN_BYTES	(32U) /**< KDF derived keying-material length */

/* Key-vault export/import demo. ExportKeyVault serialises the whole user-vault region into a
 * single signed and encrypted AEKV blob and reports its actual byte count. ImportKeyVault
 * reverses this: it verifies and decrypts the blob and rebuilds the vault(s) - preserving
 * VaultIds and KeyIds - so a vault can be backed up off-chip and later restored. Export and
 * import operate on the whole vault and do not consume key usage counts.
 *
 * A blob is the AEKV header followed by whole chunks. A chunk carries fifteen 528-byte data
 * blocks and a trailer of the chunk secrets and the digest of the chunk after it, so it is
 * larger on the wire than the plaintext it holds, and a vault is never smaller than one chunk.
 * The buffer holds two chunks, which is more than the vault this example populates needs. */
#define XASU_KM_EXAMPLE_AEKV_HDR_SIZE		(704U) /**< AEKV V1 header size */
#define XASU_KM_EXAMPLE_AEKV_CHUNK_SIZE		(8496U) /**< Chunk size on the wire */
#define XASU_KM_EXAMPLE_EXPORT_BUF_SIZE		(XASU_KM_EXAMPLE_AEKV_HDR_SIZE + \
						(2U * XASU_KM_EXAMPLE_AEKV_CHUNK_SIZE))
					/**< Export/import scratch-buffer capacity */
#define XASU_KM_EXAMPLE_EXPORT_MIN_SIZE		(XASU_KM_EXAMPLE_AEKV_HDR_SIZE + \
						XASU_KM_EXAMPLE_AEKV_CHUNK_SIZE)
					/**< Header plus the one chunk every blob carries */
#define XASU_KM_EXAMPLE_EXPORT_PREVIEW_LEN	(64U)    /**< Blob bytes logged for inspection */

/** Off-chip revocation eFuse the blob is bound to. Blowing it is what retires the blob. */
#define XASU_KM_EXAMPLE_REVOKE_EFUSE		(1U)

/************************************ Function Prototypes ****************************************/
static s32 XAsu_KeyManagerExample(void);
/* Producer steps. */
static s32 XAsu_KmDemo_CreateVault(void);
static s32 XAsu_KmDemo_StoreEccKey(void);
static s32 XAsu_KmDemo_StoreRsaPubKey(void);
static s32 XAsu_KmDemo_GenerateAesKey(void);
static s32 XAsu_KmDemo_GenerateAesIv(void);
static s32 XAsu_KmDemo_GenerateRawKey(void);
static s32 XAsu_KmDemo_UseAesKey(void);
static s32 XAsu_KmDemo_UseRawKeyHmac(void);
static s32 XAsu_KmDemo_UseRawKeyKdf(void);
static s32 XAsu_KmDemo_GenerateEccKeyPair(void);
static s32 XAsu_KmDemo_UseEccSignVerify(void);
static s32 XAsu_KmDemo_UseEcdh(void);
static s32 XAsu_KmDemo_KeyWrapWorkflow(void);
static s32 XAsu_KmDemo_ExportKeyVault(void);
static s32 XAsu_KmDemo_ImportKeyVault(void);
/* Teardown steps. */
static s32 XAsu_KmDemo_DeleteKey(u32 KeyId);
static s32 XAsu_KmDemo_DeleteVault(u32 VaultId);
/* Common infrastructure. */
static void XAsu_KmDemo_InitClientParams(XAsu_ClientParams *ClientParamPtr);
/* Clears async completion state before a request. */
static void XAsu_KmDemo_ResetResponseState(void);
static void XAsu_KmDemo_WaitForResponse(void);
static s32 XAsu_KmDemo_AesGcmOp(u8 OperationType, u64 KeyAddr, u32 KeyId, u64 InAddr, u64 OutAddr,
				u32 Len, u64 TagAddr);
static s32 XAsu_KmDemo_RsaWrapDek(void);
static s32 XAsu_KmDemo_StoreWrappedDek(void);
static s32 XAsu_KmDemo_HmacOp(u64 OutAddr);
static s32 XAsu_KmDemo_KdfOp(u64 OutAddr);
static s32 XAsu_KmDemo_EccSignOp(u32 PvtKeyId);
static s32 XAsu_KmDemo_EccVerifyOp(u32 PubKeyId);
static s32 XAsu_KmDemo_EcdhOp(u32 PvtKeyId, u32 PeerPubKeyId, u64 SharedSecretAddr);
static void XAsu_KmDemo_PrintBuffer(const char *Label, const u8 *Buf, u32 Len);
static void XAsu_KmDemo_CallBackRef(void *CallBackRef, u32 Status);

/************************************ Variable Definitions ***************************************/
/* Raw ECC P-256 private key (the curve-length private scalar) to be stored into the ECC private
 * sub-vault via StoreKey. It copies these raw bytes into the vault as-is, so any 32-byte value
 * is accepted; this step demonstrates the asymmetric-key store path and is not used in a crypto
 * operation. Placed in shared memory so the ASU server can read it via the KeyObjectAddr pointer.
 * NOTE: all key material embedded in this example is non-secret test data, for demonstration
 * only - never embed real keys in production source. */
static u8 KmEccPvtKey[XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmEccPvtKey"))) = {
	0xC9U, 0xAFU, 0xA9U, 0xD8U, 0x45U, 0xBAU, 0x75U, 0x16U,
	0x6BU, 0x5CU, 0x21U, 0x57U, 0x67U, 0xB1U, 0xD6U, 0x93U,
	0x4EU, 0x50U, 0xC3U, 0xDBU, 0x36U, 0xE8U, 0x9BU, 0x12U,
	0x7BU, 0x8AU, 0x62U, 0x2BU, 0x12U, 0x0FU, 0x67U, 0x21U
};

/* Output: vault ID assigned by the server during CreateKeyVault. Shared so the server can write it
 * back via VaultIdAddr. */
static u32 KmVaultId __attribute__ ((section (".data.KmVaultId")));

/* Output: composite key ID assigned by the server during StoreKey (the stored ECC private key).
 * Shared so the server can write it back via KeyIdAddr. */
static u32 KmEccPvtKeyId __attribute__ ((section (".data.KmEccPvtKeyId")));

/* Output: composite key ID of the AES-256 key generated inside the vault during GenerateAesKey
 * (step 3); its raw bytes never leave the ASU. It is used as the data key for the AES-GCM
 * round-trip (step 5) and as the key-encryption key (KEK) for the key-wrap workflow (step 13).
 * Shared so the server can write it back via KeyIdAddr. */
static u32 KmGenAesKeyId __attribute__ ((section (".data.KmGenAesKeyId")));

/* Output: composite key ID of the AES IV GENERATED INSIDE THE VAULT during GenerateAesIv (step 4).
 * It is passed as IvId (never as raw IV bytes) to every AES-GCM operation in the example: both legs
 * of the round-trip (step 5) and both legs of the key-wrap workflow (step 13). Shared so the server
 * can write it back via KeyIdAddr.
 * CAUTION (demo-only): this single IV is deliberately reused across multiple GCM encrypts so that a
 * byte-identical re-encrypt can prove the wrapped/stored key equals the original DEK. Reusing a
 * (key, IV) pair for multiple GCM encryptions breaks GCM's security in production - do not copy
 * this pattern; use a fresh, unique IV per encryption instead. */
static u32 KmGenAesIvId __attribute__ ((section (".data.KmGenAesIvId")));

/* Plaintext input for the AES round-trip. Shared so the ASU can read it via InputDataAddr. */
static u8 KmPlaintext[XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmPlaintext"))) = {
	0x00U, 0x11U, 0x22U, 0x33U, 0x44U, 0x55U, 0x66U, 0x77U,
	0x88U, 0x99U, 0xAAU, 0xBBU, 0xCCU, 0xDDU, 0xEEU, 0xFFU,
	0x0FU, 0x1EU, 0x2DU, 0x3CU, 0x4BU, 0x5AU, 0x69U, 0x78U,
	0x87U, 0x96U, 0xA5U, 0xB4U, 0xC3U, 0xD2U, 0xE1U, 0xF0U,
	0x13U, 0x57U, 0x9BU, 0xDFU, 0x24U, 0x68U, 0xACU, 0xE0U,
	0xFEU, 0xDCU, 0xBAU, 0x98U, 0x76U, 0x54U, 0x32U, 0x10U,
	0xA1U, 0xB2U, 0xC3U, 0xD4U, 0xE5U, 0xF6U, 0x07U, 0x18U,
	0x29U, 0x3AU, 0x4BU, 0x5CU, 0x6DU, 0x7EU, 0x8FU, 0x90U
};

/* Ciphertext produced by the encrypt operation; the server writes it via OutputDataAddr. */
static u8 KmCiphertext[XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmCiphertext")));

/* Decrypted output; must match KmPlaintext after the encrypt/decrypt round-trip. */
static u8 KmDecrypted[XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmDecrypted")));

/* AES-GCM additional authenticated data, shared by the AES round-trip (step 5) and the key-wrap
 * workflow (step 13). It is authenticated but not encrypted; encrypt and decrypt must use the same
 * AAD or the tag check fails. Shared so the ASU server can read it via AadAddr. */
static const u8 KmAesAad[XASU_KM_EXAMPLE_AES_AAD_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmAesAad"))) = {
	0xA5U, 0x5AU, 0xC3U, 0x3CU, 0x0FU, 0xF0U, 0x12U, 0x21U,
	0x34U, 0x43U, 0x56U, 0x65U, 0x78U, 0x87U, 0x9AU, 0xA9U
};

/* AES-GCM authentication tag for the round-trip (step 5): the encrypt writes it, the decrypt reads
 * and verifies it. Shared so the server can write/read it via TagAddr. */
static u8 KmAesTag[XASU_KM_EXAMPLE_AES_TAG_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmAesTag")));

/* Output: composite key ID of the raw KDF/HMAC key GENERATED INSIDE THE VAULT during GenerateRawKey
 * (step 6, KDF/HMAC sub-vault); raw bytes never leave the ASU. It is referenced by KeyId for two
 * operations: the HMAC compute (step 7) and the KDF derivation (step 8). Shared so the server can
 * write it back via KeyIdAddr. */
static u32 KmGenRawKeyId __attribute__ ((section (".data.KmGenRawKeyId")));

/* Message authenticated by the HMAC step. Shared so the ASU can read it via MsgBufferAddr. */
static u8 KmHmacMessage[XASU_KM_EXAMPLE_HMAC_MSG_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmHmacMessage"))) = {
	0x54U, 0x68U, 0x65U, 0x20U, 0x71U, 0x75U, 0x69U, 0x63U,
	0x6BU, 0x20U, 0x62U, 0x72U, 0x6FU, 0x77U, 0x6EU, 0x20U,
	0x66U, 0x6FU, 0x78U, 0x20U, 0x41U, 0x53U, 0x55U, 0x20U,
	0x48U, 0x4DU, 0x41U, 0x43U, 0x20U, 0x6DU, 0x73U, 0x67U
};

/* Two HMAC outputs computed over the same vaulted key + message; they must be byte-identical. */
static u8 KmHmac_Output1[XASU_KM_EXAMPLE_HMAC_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmHmac_Output1")));
static u8 KmHmac_Output2[XASU_KM_EXAMPLE_HMAC_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmHmac_Output2")));

/* Fixed input/context for the KDF derivation. Shared so the ASU can read it via ContextAddr. */
static u8 KmKdfContext[XASU_KM_EXAMPLE_KDF_CONTEXT_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmKdfContext"))) = {
	0x4BU, 0x44U, 0x46U, 0x20U, 0x63U, 0x6FU, 0x6EU, 0x74U,
	0x65U, 0x78U, 0x74U, 0x20U, 0x66U, 0x6FU, 0x72U, 0x20U,
	0x41U, 0x53U, 0x55U, 0x20U, 0x6BU, 0x65U, 0x79U, 0x6DU,
	0x67U, 0x72U, 0x20U, 0x65U, 0x78U, 0x61U, 0x6DU, 0x70U
};

/* Two KDF outputs derived from the same vaulted key + context; they must be byte-identical. */
static u8 KmKdf1[XASU_KM_EXAMPLE_KDF_OUT_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmKdf1")));
static u8 KmKdf2[XASU_KM_EXAMPLE_KDF_OUT_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmKdf2")));

/* Output: ECC key pair IDs assigned by the server during GenerateEccKeyPair. Element [0] is the
 * public key ID, element [1] is the private key ID. "Alice" backs sign/verify + ECDH; "Bob" is the
 * ECDH peer. Shared so the server can write them back via KeyIdAddr (two IDs per pair). */
static u32 KmEccAliceIds[XASU_KM_EXAMPLE_ECC_NUM_KEY_IDS] __attribute__ ((
		section (".data.KmEccAliceIds")));
static u32 KmEccBobIds[XASU_KM_EXAMPLE_ECC_NUM_KEY_IDS] __attribute__ ((
		section (".data.KmEccBobIds")));

/* Output: composite KeyId the server assigns to the DEK after the key-wrap workflow AES-KWP-unwraps
 * it and stores it in the AES sub-vault (produced in XAsu_KmDemo_StoreWrappedDek, step 13).
 * The workflow then references the now-vaulted DEK by this id for the verify re-encrypt and the
 * decrypt. Shared so the server can write it back via KeyIdAddr. */
static u32 KmDekId __attribute__ ((section (".data.KmDekId")));

/* RSA-4096 test public-key modulus stored into the vault by XAsu_KmDemo_StoreRsaPubKey. It is read
 * by the server via the StoreKey KeyObjectAddr pointer, so it lives in shared memory. The matching
 * public exponent is fixed by the vault (XASU_KM_EXAMPLE_RSA_PUB_EXP) and so is not stored here. */
static const u8 KmRsaModulus[XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES] __attribute__ ((
		section (".data.KmRsaModulus"))) = {
	0xCCU, 0x90U, 0xB4U, 0xB1U, 0xF1U, 0xC1U, 0xD3U, 0x9DU, 0x98U, 0xB1U, 0x34U, 0x49U, 0x92U,
	0xCDU, 0xFDU, 0xD1U, 0xCFU, 0x87U, 0x43U, 0x52U, 0xAFU, 0x9CU, 0x5CU, 0xFBU, 0x62U, 0xC9U,
	0xAAU, 0x7EU, 0xCAU, 0xFDU, 0x0EU, 0x2AU, 0xF9U, 0xBFU, 0x86U, 0xBCU, 0xDEU, 0xFDU, 0x10U,
	0x42U, 0x79U, 0xF2U, 0x18U, 0x6BU, 0xB1U, 0xCFU, 0x8DU, 0x66U, 0xDEU, 0xB0U, 0x64U, 0x1EU,
	0xB1U, 0x45U, 0x45U, 0x20U, 0x80U, 0xCCU, 0xCBU, 0x6AU, 0xADU, 0x1AU, 0x0AU, 0x2AU, 0xA9U,
	0xE2U, 0xA6U, 0x41U, 0xABU, 0xD3U, 0x3AU, 0xEDU, 0x34U, 0x71U, 0x2EU, 0x2DU, 0x08U, 0xF5U,
	0x16U, 0x7BU, 0x89U, 0xEBU, 0xBAU, 0xD5U, 0x86U, 0x33U, 0x0FU, 0x26U, 0x80U, 0x9FU, 0x3CU,
	0x98U, 0x11U, 0x62U, 0xDCU, 0x12U, 0x41U, 0xE9U, 0x7AU, 0x3CU, 0xD9U, 0x1BU, 0x0CU, 0xE5U,
	0x33U, 0x55U, 0x25U, 0x51U, 0xEFU, 0x2DU, 0xF2U, 0x3EU, 0x6EU, 0x7FU, 0xF9U, 0x76U, 0x86U,
	0x6CU, 0xA9U, 0x3CU, 0x04U, 0xFFU, 0xADU, 0xD1U, 0x9DU, 0x93U, 0xC1U, 0x04U, 0x4CU, 0x55U,
	0xABU, 0xFDU, 0xE7U, 0x71U, 0xAFU, 0xF5U, 0xA6U, 0xE1U, 0x51U, 0xE7U, 0xBAU, 0x4DU, 0x94U,
	0x22U, 0xD2U, 0x07U, 0x7BU, 0x64U, 0x11U, 0xC2U, 0x56U, 0xFAU, 0xBEU, 0x7CU, 0x1DU, 0x5FU,
	0x57U, 0x66U, 0x4EU, 0x88U, 0xD4U, 0x58U, 0x1FU, 0x12U, 0x07U, 0x65U, 0x77U, 0xA3U, 0x2AU,
	0xF4U, 0x8DU, 0xDEU, 0xDAU, 0x3CU, 0xA1U, 0x0DU, 0x41U, 0x3EU, 0x58U, 0x77U, 0x1EU, 0x59U,
	0x71U, 0xDBU, 0xD2U, 0xDDU, 0x48U, 0x20U, 0xE8U, 0xB9U, 0x09U, 0xB0U, 0xD5U, 0x95U, 0x26U,
	0xCFU, 0xAEU, 0x4FU, 0x31U, 0xDFU, 0x10U, 0xE7U, 0x7BU, 0x34U, 0x44U, 0x3EU, 0x2DU, 0x4EU,
	0x7EU, 0x9BU, 0xBAU, 0x9CU, 0xBDU, 0xB4U, 0xC9U, 0xD9U, 0x8AU, 0x09U, 0x74U, 0x75U, 0x3DU,
	0xD2U, 0x9FU, 0xDFU, 0x1AU, 0xE8U, 0x6AU, 0xA0U, 0x09U, 0xD3U, 0x66U, 0x04U, 0x8DU, 0xA8U,
	0x7BU, 0x4BU, 0x2EU, 0x3AU, 0x42U, 0xACU, 0xD7U, 0x24U, 0xFFU, 0x5FU, 0x9CU, 0x09U, 0x7FU,
	0xEFU, 0x22U, 0xA5U, 0x5AU, 0x7EU, 0x3BU, 0x63U, 0x36U, 0x38U, 0xF8U, 0xEBU, 0xFFU, 0xAEU,
	0xFDU, 0xDEU, 0x19U, 0xB6U, 0x38U, 0xC2U, 0x91U, 0x83U, 0x75U, 0x5FU, 0x86U, 0x9EU, 0xA7U,
	0x99U, 0xA1U, 0xC0U, 0x75U, 0x7EU, 0x43U, 0x8CU, 0xE7U, 0x4BU, 0x66U, 0x4CU, 0x3BU, 0xBBU,
	0x11U, 0x3EU, 0xF2U, 0x9DU, 0x84U, 0x4AU, 0x2FU, 0xACU, 0xBFU, 0x2CU, 0xC2U, 0xA9U, 0x97U,
	0xDCU, 0xC4U, 0x0FU, 0x09U, 0x2CU, 0xCCU, 0x1DU, 0x57U, 0xD6U, 0x34U, 0xB0U, 0x43U, 0xB9U,
	0xFBU, 0x45U, 0xC9U, 0x51U, 0xE8U, 0x87U, 0x39U, 0x90U, 0x49U, 0x81U, 0x1FU, 0x0EU, 0x6DU,
	0x1AU, 0x4FU, 0xC2U, 0x04U, 0xEEU, 0x69U, 0xF0U, 0x67U, 0xCEU, 0xDCU, 0x92U, 0xC5U, 0x8CU,
	0xCEU, 0x91U, 0x89U, 0x2CU, 0x56U, 0xFBU, 0xF8U, 0x70U, 0x8FU, 0xB9U, 0xCFU, 0xA0U, 0xBDU,
	0x9FU, 0x8AU, 0xF7U, 0xC5U, 0xEDU, 0xA1U, 0x57U, 0xD4U, 0x27U, 0x70U, 0x34U, 0xD5U, 0x59U,
	0x6AU, 0x93U, 0x48U, 0x11U, 0x00U, 0x02U, 0xD0U, 0x92U, 0xE6U, 0x1BU, 0xA3U, 0xFCU, 0xF8U,
	0x04U, 0x64U, 0x08U, 0x0BU, 0xDBU, 0x49U, 0x93U, 0xE3U, 0xB4U, 0x4DU, 0x68U, 0xEBU, 0x9EU,
	0xEAU, 0xABU, 0x19U, 0x36U, 0xA3U, 0xCFU, 0x39U, 0x6AU, 0xDDU, 0x24U, 0x1AU, 0x4EU, 0x70U,
	0x64U, 0xF4U, 0x0CU, 0x2CU, 0x54U, 0xCEU, 0xB3U, 0x78U, 0x53U, 0xEAU, 0x2DU, 0xB0U, 0xDBU,
	0xEEU, 0x58U, 0x86U, 0x00U, 0x42U, 0x99U, 0x8FU, 0xCFU, 0x2EU, 0x1FU, 0xCBU, 0xA5U, 0x60U,
	0x44U, 0xD4U, 0xBEU, 0x05U, 0x2CU, 0xD4U, 0x62U, 0xD8U, 0xB4U, 0x38U, 0xCBU, 0x80U, 0x38U,
	0x78U, 0x42U, 0x43U, 0xACU, 0xEBU, 0xA5U, 0xBDU, 0x05U, 0xFCU, 0x13U, 0xA7U, 0x2BU, 0x67U,
	0x82U, 0x02U, 0x4CU, 0x74U, 0x8EU, 0xAAU, 0xD0U, 0x28U, 0xD1U, 0x39U, 0x8CU, 0xEFU, 0x61U,
	0xB2U, 0xFFU, 0x60U, 0x41U, 0x83U, 0x99U, 0x81U, 0x4DU, 0xCFU, 0x81U, 0x7CU, 0xCCU, 0xECU,
	0xE3U, 0xF5U, 0xF1U, 0x17U, 0x2BU, 0x32U, 0xF2U, 0x46U, 0x8AU, 0xC4U, 0x93U, 0x20U, 0xC1U,
	0xD8U, 0xFCU, 0xC8U, 0x23U, 0x71U, 0xBFU, 0x37U, 0x85U, 0x00U, 0x7CU, 0xEDU, 0xB6U, 0x84U,
	0xABU, 0x45U, 0x00U, 0x39U, 0xA5U
};

/* Working buffer the example fills at runtime and hands to StoreKey via KeyObjectAddr. The public
 * sub-vault expects an XAsu_RsaPubKeyComp (Keysize, Modulus, PubExp). It lives in shared memory
 * because the server reads it by DMA. */
static XAsu_RsaPubKeyComp KmRsaPubKeyComp __attribute__ ((
		section (".data.KmRsaPubKeyComp")));

/* Output: composite KeyId the server assigns to the stored RSA public key. Shared so the server
 * can write it back via KeyIdAddr. */
static u32 KmRsaPubKeyId __attribute__ ((section (".data.KmRsaPubKeyId")));

/* Client-held data-encryption key (DEK) for the key-wrap workflow. It encrypts the plaintext and is
 * the secret the workflow wraps, vaults (via StoreKey) and later uses by KeyId - so its raw bytes
 * stay client-side until wrapped. Shared so the server can read it via InputDataAddr. */
static const u8 KmDek[XASU_KM_EXAMPLE_DEK_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmDek"))) = {
	0x60U, 0x3DU, 0xEBU, 0x10U, 0x15U, 0xCAU, 0x71U, 0xBEU,
	0x2BU, 0x73U, 0xAEU, 0xF0U, 0x85U, 0x7DU, 0x77U, 0x81U,
	0x1FU, 0x35U, 0x2CU, 0x07U, 0x3BU, 0x61U, 0x08U, 0xD7U,
	0x2DU, 0x98U, 0x10U, 0xA3U, 0x09U, 0x14U, 0xDFU, 0xF4U
};

/* RSA-OAEP optional label for the wrap (a non-trivial 32-byte label to exercise a realistic label
 * length). Shared so the server can read it. The label is hashed into the OAEP encoding, so its
 * length does not affect the wrapped-blob size. */
static const u8 KmOaepLabel[XASU_KM_EXAMPLE_OAEP_LABEL_SIZE_IN_BYTES] __attribute__ ((
		section (".data.KmOaepLabel"))) = "ASU KeyManager OAEP label sample";

/* Workflow ciphertext (DEK-encrypted plaintext) and the plaintext recovered after the DEK is
 * wrapped, vaulted and reused by KeyId; the latter must match KmPlaintext. */
static u8 KmDekCiphertext[XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmDekCiphertext")));
static u8 KmDekDecrypted[XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmDekDecrypted")));

/* Recovered plaintext for the post-import content-integrity check (step 15): the imported DEK
 * (KmDekId) decrypts KmDekCiphertext here, which must equal KmPlaintext. This proves the
 * export/import preserved the actual DEK bytes in the vault, and that the key stays usable under
 * its original KeyId. */
static u8 KmDecrypted_ImportVerif[XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmDecrypted_ImportVerif")));

/* Re-encryption of KmPlaintext with the STORED DEK (referenced by KmDekId). Before the decrypt
 * round-trip, the workflow encrypts the same plaintext again through the vaulted DEK and compares
 * this ciphertext against KmDekCiphertext (produced with the raw DEK). Because the same key + IV +
 * AAD + plaintext must yield the same ciphertext, a byte-for-byte match proves the key that was
 * wrapped and stored is identical to the DEK that encrypted originally. */
static u8 KmCiphertext_ReVerif[XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmCiphertext_ReVerif")));

/* AES-GCM auth tag for the workflow (step 13): the encrypt with the raw DEK writes it, the
 * decrypt with the vaulted DEK reads and verifies it. Shared so the server can write/read it. */
static u8 KmDekTag[XASU_KM_EXAMPLE_AES_TAG_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmDekTag")));

/* GCM tag output for the verify re-encrypt (the stored-DEK encrypt that proves key equality). It is
 * a required output slot for that encrypt; only KmCiphertext_ReVerif is compared. */
static u8 KmDek_ReVerifGCMTag[XASU_KM_EXAMPLE_AES_TAG_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmDek_ReVerifGCMTag")));

/* Hybrid wrap blob produced by XAsu_KeyWrap in the key-wrap workflow (step 13). Layout:
 *   [ RSA-OAEP(KEK) : XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES bytes ] ||
 *   [ AES-KWP(DEK)  : XASU_KM_EXAMPLE_AES_KWP_BLOB_SIZE_IN_BYTES bytes ]
 * The leading RSA-OAEP portion holds the key-encryption key (KEK) wrapped with the vaulted RSA
 * public key; the trailing AES-KWP portion holds the DEK wrapped with that KEK. XAsu_KeyWrap writes
 * the whole blob; StoreKey later reads only the trailing AES-KWP portion (skipping the RSA-OAEP
 * prefix) to unwrap and vault the DEK. Shared so the server can DMA-write it during the wrap and
 * DMA-read it during StoreKey. */
static u8 KmWrappedBlob[XASU_KM_EXAMPLE_WRAPPED_BLOB_SIZE_IN_BYTES] __attribute__ ((
		section (".data.KmWrappedBlob")));

/* Actual output length the server reports for the wrap. Shared so the server can write it via
 * ActualOutputDataLenAddr. */
static u32 KmWrapOutLen __attribute__ ((section (".data.KmWrapOutLen")));

/* Key-vault export/import buffer and size. The server DMAs the exported blob into this buffer and
 * later DMAs it back in on import, so it lives in shared memory; it is 64-byte aligned to suit the
 * server's DMA. KmExportSize receives the actual blob size the export produced (server-written). */
static u8 KmExportBuffer[XASU_KM_EXAMPLE_EXPORT_BUF_SIZE] __attribute__ ((aligned(64),
		section (".data.KmExportBuffer")));
static u32 KmExportSize __attribute__ ((section (".data.KmExportSize")));

/* Result block an export writes back, carrying the blob size and the modified flag. The client
 * library fills it from the command response. */
static XAsu_KeyVaultExportResult KmExportResult __attribute__ ((aligned(64),
		section (".data.KmExportResult")));

/* Context the key vault keys are derived from. The import below hands back the same one, which
 * is what lets it recover the blob this export produced. */
static const u8 KmExportContext[XASU_KM_KV_CONTEXT_SIZE_IN_BYTES] = {
	0x4BU, 0x4DU, 0x5FU, 0x45U, 0x58U, 0x41U, 0x4DU, 0x50U,
	0x4CU, 0x45U, 0x5FU, 0x56U, 0x41U, 0x55U, 0x4CU, 0x54U,
	0x5FU, 0x43U, 0x4FU, 0x4EU, 0x54U, 0x45U, 0x58U, 0x54U,
	0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
};

/* Digest signed/verified by the ECC sign/verify step. Shared so the ASU server can read it. */
static u8 KmEccDigest[XASU_KM_EXAMPLE_ECC_DIGEST_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmEccDigest")));

/* Signature (r||s) produced by the sign operation and consumed by the verify operation. */
static u8 KmEccSignature[XASU_KM_EXAMPLE_ECC_SIG_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmEccSignature")));

/* Two ECDH shared secrets computed from opposite sides of the same key pairs; ECDH's symmetry
 * property requires them to be byte-identical. */
static u8 KmEcdhSecretA[XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmEcdhSecretA")));
static u8 KmEcdhSecretB[XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES] __attribute__ ((
		section (".data.KmEcdhSecretB")));

static volatile u8 Notify = 0U; /**< To notify the call back from the client library */
static volatile u32 ErrorStatus = XST_FAILURE; /**< Holds the status returned by the server and
						gets updated during the callback. */

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function that initializes the mailbox and ASU client and then calls
 *		XAsu_KeyManagerExample, which exercises the KeyManager lifecycle: it creates a key
 *		vault, stores a raw ECC P-256 private key, generates an AES-256 key, an AES IV and a
 *		raw KDF/HMAC key directly inside the vault, uses the generated AES key/IV for an
 *		AES-GCM encrypt/decrypt round-trip and the raw key for both an HMAC and a KDF
 *		derivation, generates two ECC P-256 key pairs and uses them for a sign/verify
 *		round-trip and an ECDH shared-secret exchange, stores an RSA-4096 public key and runs
 *		a key-wrap workflow, exports the whole vault to an encrypted blob and imports it back,
 *		and finally deletes the stored key and the vault.
 *
 * @return
 *		- XST_SUCCESS, if the KeyManager example completes successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;
	XMailbox MailboxInstance;

	xil_printf("ASU KeyManager Client Example: \r\n");

#ifdef XASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	/** Initialize mailbox instance. */
	Status = (s32)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Mailbox initialize failed: %08x \r\n", Status);
		goto END;
	}

	/** Initialize the client instance. */
	Status = XAsu_ClientInit(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ASU Client initialize failed: %08x \r\n", Status);
		goto END;
	}

	Status = XAsu_KeyManagerExample();

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\n Successfully ran KeyManager client example ");
	} else {
		xil_printf("\r\n KeyManager client example failed with Status = %08x", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function orchestrates the KeyManager example lifecycle by invoking the
 *		individual per-operation step functions: it creates a key vault, stores a raw ECC
 *		P-256 private key into its ECC private sub-vault, generates an AES-256 key, an IV and a raw
 *		KDF/HMAC key directly inside the vault, uses the generated key/IV for an AES-GCM
 *		encrypt/decrypt round-trip and the raw key for both an HMAC and a KDF derivation,
 *		generates two ECC P-256 key pairs and uses them for a sign/verify round-trip and an
 *		ECDH shared-secret exchange, stores an RSA-4096 public key and runs a key-wrap workflow
 *		(AES-GCM-encrypt a plaintext with a data key, RSA-wrap that data key with the vaulted RSA
 *		public key using the vaulted AES key as the KEK, StoreKey to resolve that KEK by KeyId and
 *		AES-KWP-unwrap the data key, then AES-GCM-decrypt by the stored data key's KeyId
 *		and verify), exports the whole vault to an encrypted blob and imports it back (with a
 *		content self-check that decrypts the step-13 ciphertext by the imported DEK's KeyId), and
 *		then tears everything down (DeleteKey followed by DeleteKeyVault).
 *
 * @return
 *		- XST_SUCCESS, if the create, store, generate, use and delete operations all succeed.
 *		- XST_FAILURE, otherwise.
 *
 * @note	The delete operations run as best-effort cleanup: the key/vault are deleted even if
 *		an earlier step fails, provided they were created. The generated keys and IV are not
 *		deleted explicitly; deleting the vault frees them along with any other keys it holds.
 *
 *************************************************************************************************/
/** //! [KeyManager example] */
static s32 XAsu_KeyManagerExample(void)
{
	s32 Status = XST_FAILURE;

#ifndef XASU_CACHE_DISABLE
	/* Prime the shared DMA buffers before use: flush the raw ECC key the server will read, and
	 * invalidate every server-written output/ID slot so no dirty CPU line later evicts over the
	 * values the server DMAs back. */
	Xil_DCacheFlushRange((UINTPTR)KmEccPvtKey, XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)&KmVaultId, sizeof(KmVaultId));
	Xil_DCacheInvalidateRange((UINTPTR)&KmEccPvtKeyId, sizeof(KmEccPvtKeyId));
	Xil_DCacheInvalidateRange((UINTPTR)&KmGenAesKeyId, sizeof(KmGenAesKeyId));
	Xil_DCacheInvalidateRange((UINTPTR)&KmGenAesIvId, sizeof(KmGenAesIvId));
	Xil_DCacheInvalidateRange((UINTPTR)&KmGenRawKeyId, sizeof(KmGenRawKeyId));
	Xil_DCacheInvalidateRange((UINTPTR)KmEccAliceIds, sizeof(KmEccAliceIds));
	Xil_DCacheInvalidateRange((UINTPTR)KmEccBobIds, sizeof(KmEccBobIds));
	Xil_DCacheInvalidateRange((UINTPTR)&KmDekId, sizeof(KmDekId));
	Xil_DCacheInvalidateRange((UINTPTR)&KmRsaPubKeyId, sizeof(KmRsaPubKeyId));
	Xil_DCacheInvalidateRange((UINTPTR)KmWrappedBlob, sizeof(KmWrappedBlob));
	Xil_DCacheInvalidateRange((UINTPTR)KmDekCiphertext, sizeof(KmDekCiphertext));
	Xil_DCacheInvalidateRange((UINTPTR)KmCiphertext_ReVerif, sizeof(KmCiphertext_ReVerif));
	Xil_DCacheInvalidateRange((UINTPTR)KmDekDecrypted, sizeof(KmDekDecrypted));
	Xil_DCacheInvalidateRange((UINTPTR)KmDecrypted_ImportVerif,
				sizeof(KmDecrypted_ImportVerif));
	Xil_DCacheInvalidateRange((UINTPTR)KmAesTag, sizeof(KmAesTag));
	Xil_DCacheInvalidateRange((UINTPTR)KmDekTag, sizeof(KmDekTag));
	Xil_DCacheInvalidateRange((UINTPTR)KmDek_ReVerifGCMTag, sizeof(KmDek_ReVerifGCMTag));
	Xil_DCacheInvalidateRange((UINTPTR)&KmWrapOutLen, sizeof(KmWrapOutLen));
	Xil_DCacheInvalidateRange((UINTPTR)&KmExportSize, sizeof(KmExportSize));
#endif

	/** Step 1: Create a key vault for this subsystem. */
	Status = XAsu_KmDemo_CreateVault();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 2: Store a raw ECC P-256 private key into the vault. */
	Status = XAsu_KmDemo_StoreEccKey();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 3: Generate an AES-256 key directly inside the vault. */
	Status = XAsu_KmDemo_GenerateAesKey();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 4: Generate an AES IV directly inside the vault. */
	Status = XAsu_KmDemo_GenerateAesIv();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 5: Use the generated AES key + IV (by ID) to AES-GCM encrypt then decrypt. */
	Status = XAsu_KmDemo_UseAesKey();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 6: Generate a raw (KDF/HMAC) key directly inside the vault. */
	Status = XAsu_KmDemo_GenerateRawKey();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 7: Use the generated raw key (by ID) to compute and verify an HMAC. */
	Status = XAsu_KmDemo_UseRawKeyHmac();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 8: Use the same raw key (by ID) to derive and verify keying material with the KDF. */
	Status = XAsu_KmDemo_UseRawKeyKdf();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 9: Generate the "Alice" and "Bob" ECC key pairs backing the two ECC use cases
	 *  (sign/verify and key agreement) exercised by the next two steps. */
	Status = XAsu_KmDemo_GenerateEccKeyPair();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 10: Use Alice's key pair (by ID) to sign a digest and verify the signature. */
	Status = XAsu_KmDemo_UseEccSignVerify();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 11: Derive the ECDH shared secret from both sides and verify they match. */
	Status = XAsu_KmDemo_UseEcdh();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 12: Store the RSA-4096 public key into the vault's RSA public sub-vault; the key-wrap
	 *  workflow below uses it to wrap the vaulted AES key-encryption key (KEK). */
	Status = XAsu_KmDemo_StoreRsaPubKey();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 13: Key-wrap workflow: AES-GCM encrypt with the raw DEK -> RSA-wrap the DEK with the
	 *  vaulted AES KEK -> StoreKey (AES-KWP unwrap by resolving the KEK from the vault) -> AES-GCM
	 *  decrypt by the stored DEK's KeyId, verifying the tag and that the recovered plaintext
	 *  matches. */
	Status = XAsu_KmDemo_KeyWrapWorkflow();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 14: Export the whole populated vault into a single encrypted, authenticated blob. */
	Status = XAsu_KmDemo_ExportKeyVault();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

	/** Step 15: Import the vault back from that blob, restoring its keys, then decrypt the step-13
	 *  ciphertext with the imported DEK by KeyId to confirm the key bytes survived. */
	Status = XAsu_KmDemo_ImportKeyVault();
	if (Status != XST_SUCCESS) {
		goto CLEANUP;
	}

CLEANUP:
	/**
	 * Step 16: Delete the stored ECC key (best-effort), only if it was created. DeleteKey is
	 * demonstrated on the stored key; the generated keys, IV, ECC key pairs, stored RSA public
	 * key and the stored DEK are reclaimed by the vault deletion below. A cleanup failure can only
	 * downgrade the overall status.
	 */
	if (KmEccPvtKeyId != 0U) {
		if (XAsu_KmDemo_DeleteKey(KmEccPvtKeyId) != XST_SUCCESS) {
			Status = XST_FAILURE;
		}
	}

	/**
	 * Step 17: Delete the key vault (best-effort), only if it was created. This also frees any
	 * keys still resident in the vault, including the generated keys, IV, ECC key pairs, stored
	 * RSA public key and DEK.
	 */
	if (KmVaultId != 0U) {
		if (XAsu_KmDemo_DeleteVault(KmVaultId) != XST_SUCCESS) {
			Status = XST_FAILURE;
		}
	}

	return Status;
}
/** //! [KeyManager example] */

/*************************************************************************************************/
/**
 * @brief	Creates a key vault for this subsystem and captures the assigned VaultId in
 *		KmVaultId. The AES, IV, KDF/HMAC, ECC (private/public) and RSA (private/public)
 *		sub-vault capacities are requested; other sub-vault capacities remain 0. The vault is
 *		marked exportable so the later ExportKeyVault/ImportKeyVault steps can operate on it.
 *
 * @return
 *		- XST_SUCCESS, if the vault is created successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_CreateVault(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KeyManagerSubVaultParams VaultParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	VaultParams.VaultIdAddr = (u64)(UINTPTR)&KmVaultId;
	VaultParams.AESKeyVaultCapacity = XASU_KM_EXAMPLE_AES_VAULT_CAPACITY;
	VaultParams.IVVaultCapacity = XASU_KM_EXAMPLE_IV_VAULT_CAPACITY;
	VaultParams.KDFHmacKeyVaultCapacity = XASU_KM_EXAMPLE_KDF_HMAC_VAULT_CAPACITY;
	VaultParams.ECCPvtKeyVaultCapacity = XASU_KM_EXAMPLE_ECC_VAULT_CAPACITY;
	VaultParams.ECCPubKeyVaultCapacity = XASU_KM_EXAMPLE_ECC_VAULT_CAPACITY;
	VaultParams.RSAPvtKeyVaultCapacity = 0U;
	VaultParams.RSAPubKeyVaultCapacity = XASU_KM_EXAMPLE_RSA_VAULT_CAPACITY;
	VaultParams.AccessRights = XASU_KM_EXAMPLE_ACCESS_RIGHTS;
	VaultParams.Restrictions = XASU_KEYMANAGER_EXPORTABLE_VAULT;

	XAsu_KmDemo_ResetResponseState();
	KmVaultId = 0U;

#ifndef XASU_CACHE_DISABLE
	/* Clean the VaultId output slot before the request so a dirty line can't clobber the
	 * server's write. */
	Xil_DCacheFlushRange((UINTPTR)&KmVaultId, sizeof(KmVaultId));
#endif

	Status = XAsu_KmCreateKeyVault(&ClientParams, &VaultParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("CreateKeyVault request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the VaultId the server wrote, not a stale cached copy. */
	Xil_DCacheInvalidateRange((UINTPTR)&KmVaultId, sizeof(KmVaultId));
#endif

	if ((ErrorStatus != (u32)XST_SUCCESS) || (KmVaultId == 0U)) {
		XilAsu_Printf("CreateKeyVault failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n Created vault, VaultId = %u \r\n", KmVaultId);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Stores a raw ECC P-256 private key into the ECC private sub-vault of the
 *		created vault and captures the assigned composite KeyId in KmEccPvtKeyId.
 *
 *		KeyObjectAddr : input buffer holding the raw private scalar (curve-length bytes).
 *		KeyIdAddr     : output buffer where the server writes the composite KeyId.
 *		KeyMetadata   : describes the key (vault/sub-vault, use case, curve, length, etc.).
 *
 *		KeyType selects the ECC private sub-vault and KeyAttributes carries the curve type;
 *		the server copies the raw key bytes into the vault as-is. This demonstrates storing an
 *		asymmetric key (vs. generating one in-vault), mirroring the lifecycle shown for AES.
 *		EpochTime is left 0 (no expiry); UsageCount limits how many times the key may be used.
 *		Use XASU_KM_USAGE_COUNT_NON_DEPLETING_VALUE for a key that never depletes.
 *
 * @return
 *		- XST_SUCCESS, if the key is stored successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_StoreEccKey(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KeyManagerParams KmParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	KmParams.KeyObjectAddr = (u64)(UINTPTR)KmEccPvtKey;
	KmParams.KeyObjectLen = (u32)sizeof(KmEccPvtKey);
	KmParams.KeyIdAddr = (u64)(UINTPTR)&KmEccPvtKeyId;
	/* Target the vault created in step 1 (KmVaultId, CreateVault). */
	KmParams.KeyMetadata.CompositeKeyId =
			XASU_KM_COMPOSE_KEY_ID((u8)KmVaultId, (u8)XASU_ECC_PVT_SUBVAULT_ID, 0U);
	KmParams.KeyMetadata.KeyAttributes = (u8)XASU_KM_EXAMPLE_ECC_CURVE;
	KmParams.KeyMetadata.KeyUseCase = (u8)XASU_KEYMANAGER_ECC_PVT_SIGN_GEN_USE_CASE;
	KmParams.KeyMetadata.UsageCount = XASU_KM_EXAMPLE_USAGE_COUNT;
	KmParams.KeyMetadata.EpochTime = 0U;
	KmParams.KeyMetadata.Length = XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES;

	XAsu_KmDemo_ResetResponseState();
	KmEccPvtKeyId = 0U;

#ifndef XASU_CACHE_DISABLE
	/* Clean the KeyId output slot before the request so a dirty line can't clobber the
	 * server's write. */
	Xil_DCacheFlushRange((UINTPTR)&KmEccPvtKeyId, sizeof(KmEccPvtKeyId));
#endif

	Status = XAsu_KmStoreKey(&ClientParams, &KmParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("StoreKey request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the KeyId the server wrote back. */
	Xil_DCacheInvalidateRange((UINTPTR)&KmEccPvtKeyId, sizeof(KmEccPvtKeyId));
#endif

	if ((ErrorStatus != (u32)XST_SUCCESS) || (KmEccPvtKeyId == 0U)) {
		XilAsu_Printf("StoreKey failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n Stored ECC P-256 private key, KeyId = 0x%08x \r\n", KmEccPvtKeyId);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Stores the RSA-4096 test public key into the vault's RSA public sub-vault, capturing
 *		its composite KeyId in KmRsaPubKeyId. The key-wrap workflow uses this public key to
 *		wrap the key-encryption key (KEK) during XAsu_KeyWrap, so it is provisioned with the
 *		key-transport use case. No private key is stored - StoreKey recovers the (vaulted) KEK
 *		by KeyId, so the workflow needs no client-side RSA-OAEP peel.
 *
 *		KeyObjectAddr points to an XAsu_RsaPubKeyComp (Keysize, Modulus, PubExp). The server
 *		stores only the modulus; the public exponent is fixed by the vault to
 *		XASU_KM_EXAMPLE_RSA_PUB_EXP, so PubExp is set to match. Length carries the RSA key size
 *		(512 bytes); the key is provisioned non-depleting as long-lived key-transport material.
 *
 * @return
 *		- XST_SUCCESS, if the public key is stored successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_StoreRsaPubKey(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KeyManagerParams KmParams = {0};

	/** Build the RSA public key component (modulus + fixed public exponent). */
	KmRsaPubKeyComp.Keysize = XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES;
	KmRsaPubKeyComp.PubExp = XASU_KM_EXAMPLE_RSA_PUB_EXP;
	Status = Xil_SMemCpy(KmRsaPubKeyComp.Modulus, XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES,
			KmRsaModulus, XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES,
			XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("RSA public key buffer prep failed: %08x \r\n", Status);
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	/* Flush the RSA public key component so the server reads the bytes the CPU just built. */
	Xil_DCacheFlushRange((UINTPTR)&KmRsaPubKeyComp, sizeof(KmRsaPubKeyComp));
#endif

	/** Store the RSA public key into the RSA public sub-vault. */
	XAsu_KmDemo_InitClientParams(&ClientParams);

	KmParams.KeyObjectAddr = (u64)(UINTPTR)&KmRsaPubKeyComp;
	KmParams.KeyObjectLen = (u32)sizeof(KmRsaPubKeyComp);
	KmParams.KeyIdAddr = (u64)(UINTPTR)&KmRsaPubKeyId;
	/* Target the vault created in step 1 (KmVaultId, CreateVault). */
	KmParams.KeyMetadata.CompositeKeyId =
			XASU_KM_COMPOSE_KEY_ID((u8)KmVaultId, (u8)XASU_RSA_PUB_SUBVAULT_ID, 0U);
	KmParams.KeyMetadata.KeyAttributes = 0U;
	KmParams.KeyMetadata.KeyUseCase = (u8)XASU_KM_EXAMPLE_RSA_PUB_USE_CASE;
	KmParams.KeyMetadata.UsageCount = XASU_KM_EXAMPLE_RSA_USAGE_COUNT;
	KmParams.KeyMetadata.EpochTime = 0U;
	KmParams.KeyMetadata.Length = XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES;

	XAsu_KmDemo_ResetResponseState();
	KmRsaPubKeyId = 0U;

#ifndef XASU_CACHE_DISABLE
	/* Clean the KeyId output slot before the request so a dirty line can't clobber the
	 * server's write. */
	Xil_DCacheFlushRange((UINTPTR)&KmRsaPubKeyId, sizeof(KmRsaPubKeyId));
#endif

	Status = XAsu_KmStoreKey(&ClientParams, &KmParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("StoreKey (RSA public) request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the KeyId the server wrote back. */
	Xil_DCacheInvalidateRange((UINTPTR)&KmRsaPubKeyId, sizeof(KmRsaPubKeyId));
#endif

	if ((ErrorStatus != (u32)XST_SUCCESS) || (KmRsaPubKeyId == 0U)) {
		XilAsu_Printf("StoreKey (RSA public) failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n Stored RSA-4096 public key, KeyId = 0x%08x \r\n", KmRsaPubKeyId);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Generates a fresh AES-256 key directly inside the vault and captures the assigned
 *		composite KeyId in KmGenAesKeyId.
 *
 *		Unlike StoreKey, no raw key bytes are supplied: the ASU generates the key internally
 *		(from its TRNG) and returns only a composite KeyId via KeyIdAddr. KeyObjectAddr is left
 *		0 so the key is kept in the vault; a non-zero KeyObjectAddr would instead make the
 *		server DMA the generated key out to that buffer, bypassing vault storage.
 *
 *		This key serves two roles later: the data key for the AES-GCM round-trip (step 5) and
 *		the key-encryption key (KEK) for the key-wrap workflow (step 13). It is therefore
 *		provisioned with all four AES use cases (ENC/DEC for the round-trip, KEY_WRAP/KEY_UNWRAP
 *		for the KEK) and a usage count covering all four resolutions.
 *
 * @return
 *		- XST_SUCCESS, if the key is generated successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_GenerateAesKey(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KeyManagerParams GenKmParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* Producer site: the server writes the generated AES key's composite id back into
	 * KmGenAesKeyId here (step 3). It is later consumed as the AES-GCM data key (step 5) and as
	 * the key-encryption key (KEK) that wraps/unwraps the client DEK (step 13). */
	GenKmParams.KeyIdAddr = (u64)(UINTPTR)&KmGenAesKeyId;
	/* Target the vault created in step 1 (KmVaultId, CreateVault). */
	GenKmParams.KeyMetadata.CompositeKeyId = XASU_KM_COMPOSE_KEY_ID((u8)KmVaultId,
			(u8)XASU_AES_SUBVAULT_ID, 0U);
	GenKmParams.KeyMetadata.KeyUseCase = (u8)XASU_KM_EXAMPLE_AES_KEK_USE_CASE;
	GenKmParams.KeyMetadata.UsageCount = XASU_KM_EXAMPLE_AES_KEK_USAGE_COUNT;
	GenKmParams.KeyMetadata.Length = XASU_KM_EXAMPLE_AES_KEY_LEN_IN_BYTES;

	XAsu_KmDemo_ResetResponseState();
	KmGenAesKeyId = 0U;

#ifndef XASU_CACHE_DISABLE
	/* Clean the KeyId output slot before the request so a dirty line can't clobber the
	 * server's write. */
	Xil_DCacheFlushRange((UINTPTR)&KmGenAesKeyId, sizeof(KmGenAesKeyId));
#endif

	Status = XAsu_KmGenerateAesKey(&ClientParams, &GenKmParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("GenerateAesKey request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the KeyId the server wrote back. */
	Xil_DCacheInvalidateRange((UINTPTR)&KmGenAesKeyId, sizeof(KmGenAesKeyId));
#endif

	if ((ErrorStatus != (u32)XST_SUCCESS) || (KmGenAesKeyId == 0U)) {
		XilAsu_Printf("GenerateAesKey failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n Generated AES key, KeyId = 0x%08x \r\n", KmGenAesKeyId);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Generates a fresh AES IV directly inside the vault and captures the assigned composite
 *		KeyId in KmGenAesIvId.
 *
 *		Like GenerateAesKey, no raw bytes are supplied: the ASU generates the IV internally
 *		(from its TRNG) and returns only a composite KeyId via KeyIdAddr. KeyObjectAddr is left
 *		0 so the IV is kept in the IV sub-vault; a non-zero KeyObjectAddr would instead make the
 *		server DMA the generated IV out to that buffer, bypassing vault storage.
 *
 * @return
 *		- XST_SUCCESS, if the IV is generated successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_GenerateAesIv(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KeyManagerParams GenIvParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* Producer site: the server writes the generated IV's composite id back into KmGenAesIvId
	 * here (step 4). It is later consumed as IvId by every AES-GCM operation (steps 5 and 13). */
	GenIvParams.KeyIdAddr = (u64)(UINTPTR)&KmGenAesIvId;
	/* Target the vault created in step 1 (KmVaultId, CreateVault). */
	GenIvParams.KeyMetadata.CompositeKeyId = XASU_KM_COMPOSE_KEY_ID((u8)KmVaultId,
			(u8)XASU_IV_SUBVAULT_ID, 0U);
	GenIvParams.KeyMetadata.KeyUseCase = (u8)XASU_KM_EXAMPLE_AES_USE_CASE;
	GenIvParams.KeyMetadata.UsageCount = XASU_KM_EXAMPLE_IV_USAGE_COUNT;
	GenIvParams.KeyMetadata.Length = XASU_KM_EXAMPLE_AES_IV_LEN_IN_BYTES;

	XAsu_KmDemo_ResetResponseState();
	KmGenAesIvId = 0U;

#ifndef XASU_CACHE_DISABLE
	/* Clean the IvId output slot before the request so a dirty line can't clobber the
	 * server's write. */
	Xil_DCacheFlushRange((UINTPTR)&KmGenAesIvId, sizeof(KmGenAesIvId));
#endif

	Status = XAsu_KmGenerateAesIv(&ClientParams, &GenIvParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("GenerateAesIv request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the IvId the server wrote back. */
	Xil_DCacheInvalidateRange((UINTPTR)&KmGenAesIvId, sizeof(KmGenAesIvId));
#endif

	if ((ErrorStatus != (u32)XST_SUCCESS) || (KmGenAesIvId == 0U)) {
		XilAsu_Printf("GenerateAesIv failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n Generated AES IV, KeyId = 0x%08x \r\n", KmGenAesIvId);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Generates a fresh raw (KDF/HMAC) key directly inside the vault and captures the
 *		assigned composite KeyId in KmGenRawKeyId.
 *
 *		A raw key is generic random key material stored in the KDF/HMAC sub-vault; it is not
 *		intrinsically an HMAC or a KDF key. Its permitted purpose is set by KeyUseCase: here it
 *		is provisioned for both HMAC and KDF use, so the same key backs the HMAC recompute and
 *		the KDF derivation steps. KeyObjectAddr is left 0 so the key stays in the vault.
 *
 * @return
 *		- XST_SUCCESS, if the raw key is generated successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_GenerateRawKey(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KeyManagerParams GenRawParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* Producer site: the server writes the generated raw key's composite id back into
	 * KmGenRawKeyId here (step 6). It is later consumed by KeyId for the HMAC compute (step 7)
	 * and the KDF derivation (step 8). */
	GenRawParams.KeyIdAddr = (u64)(UINTPTR)&KmGenRawKeyId;
	/* Target the vault created in step 1 (KmVaultId, CreateVault). */
	GenRawParams.KeyMetadata.CompositeKeyId = XASU_KM_COMPOSE_KEY_ID((u8)KmVaultId,
			(u8)XASU_KDF_HMAC_SUBVAULT_ID, 0U);
	GenRawParams.KeyMetadata.KeyUseCase = (u8)XASU_KM_EXAMPLE_RAW_KEY_USE_CASE;
	GenRawParams.KeyMetadata.UsageCount = XASU_KM_EXAMPLE_RAW_USAGE_COUNT;
	GenRawParams.KeyMetadata.Length = XASU_KM_EXAMPLE_RAW_KEY_LEN_IN_BYTES;

	XAsu_KmDemo_ResetResponseState();
	KmGenRawKeyId = 0U;

#ifndef XASU_CACHE_DISABLE
	/* Clean the KeyId output slot before the request so a dirty line can't clobber the
	 * server's write. */
	Xil_DCacheFlushRange((UINTPTR)&KmGenRawKeyId, sizeof(KmGenRawKeyId));
#endif

	Status = XAsu_KmGenerateRawKey(&ClientParams, &GenRawParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("GenerateRawKey request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the KeyId the server wrote back. */
	Xil_DCacheInvalidateRange((UINTPTR)&KmGenRawKeyId, sizeof(KmGenRawKeyId));
#endif

	if ((ErrorStatus != (u32)XST_SUCCESS) || (KmGenRawKeyId == 0U)) {
		XilAsu_Printf("GenerateRawKey failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n Generated raw (KDF/HMAC) key, KeyId = 0x%08x \r\n", KmGenRawKeyId);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Performs a single AES-GCM operation, resolving the IV from the vault by IvId and
 *		taking the key either by raw bytes (KeyAddr) or by vault KeyId. The same fixed AAD
 *		(KmAesAad) is authenticated on every call.
 *
 *		Exactly one of KeyAddr / KeyId must be non-zero: KeyAddr supplies raw key bytes
 *		(KeyId = 0), used when encrypting with a client-held data key; KeyId resolves a vaulted
 *		key (KeyAddr = 0), used when decrypting with a key that lives in the vault. The IV is
 *		always referenced by the generated IvId (IvAddr = 0). The key object is rebuilt on every
 *		call so the KeyAddress/IvAddr the server fills in for one operation never leaks into the
 *		next request's "exactly one of address/ID" validation.
 *
 *		On encrypt the server writes the authentication tag to TagAddr; on decrypt it reads the
 *		tag from TagAddr and verifies it, so a wrong key, IV, AAD or ciphertext makes the call
 *		fail rather than silently returning garbage - this is what gives the round-trips their
 *		strong correctness proof.
 *
 * @param	OperationType	XASU_AES_ENCRYPT_OPERATION or XASU_AES_DECRYPT_OPERATION.
 * @param	KeyAddr		Address of the raw AES-256 key bytes, or 0 to resolve by KeyId.
 * @param	KeyId		Composite KeyId of the vaulted key, or 0 to use KeyAddr.
 * @param	InAddr		Address of the input buffer.
 * @param	OutAddr		Address of the output buffer.
 * @param	Len		Length of the input/output data in bytes.
 * @param	TagAddr		Address of the GCM tag buffer (written on encrypt, read on decrypt).
 *
 * @return
 *		- XST_SUCCESS, if the operation completes successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_AesGcmOp(u8 OperationType, u64 KeyAddr, u32 KeyId, u64 InAddr, u64 OutAddr,
			u32 Len, u64 TagAddr)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_AesParams AesParams = {0};
	XAsu_AesKeyObject KeyObj = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* Seed AdditionalStatus with a failure value so a missing/garbled response cannot leave a
	 * stale TAG_READ/TAG_MATCHED behind; the library overwrites it from the server response. */
	ClientParams.AdditionalStatus = (u32)XST_FAILURE;

	/* Take the key by raw bytes (KeyAddr) or by vault KeyId; the other selector stays 0. */
	KeyObj.KeyAddress = KeyAddr;
	KeyObj.KeySize = XASU_AES_KEY_SIZE_256_BITS;
	KeyObj.KeySrc = XASU_AES_USER_KEY_0;
	KeyObj.KeyId = KeyId;

	AesParams.InputDataAddr = InAddr;
	AesParams.OutputDataAddr = OutAddr;
	AesParams.KeyObjectAddr = (u64)(UINTPTR)&KeyObj;
	/* Reference the generated IV by IvId; IvAddr = 0 selects vault resolution. KmGenAesIvId is the
	 * IV generated in step 4. This helper is shared by the AES-GCM round-trip (step 5) and both
	 * legs of the key-wrap workflow (step 13), so the same vaulted IV backs every AES-GCM op. */
	AesParams.IvObj.IvAddr = 0U;
	AesParams.IvObj.IvId = KmGenAesIvId;
	AesParams.IvObj.IvLen = 0U;
	AesParams.DataLen = Len;
	AesParams.AadAddr = (u64)(UINTPTR)KmAesAad;
	AesParams.AadLen = XASU_KM_EXAMPLE_AES_AAD_LEN_IN_BYTES;
	AesParams.TagAddr = TagAddr;
	AesParams.TagLen = XASU_KM_EXAMPLE_AES_TAG_LEN_IN_BYTES;
	AesParams.EngineMode = XASU_AES_GCM_MODE;
	AesParams.OperationFlags = (u8)(XASU_INIT | XASU_UPDATE | XASU_FINISH);
	AesParams.OperationType = OperationType;
	AesParams.IsLast = TRUE;

#ifndef XASU_CACHE_DISABLE
	/* The server DMAs the key object from KeyObjectAddr into its own copy, so flush this
	 * stack-resident struct out to memory before the request or the server reads stale bytes. */
	Xil_DCacheFlushRange((UINTPTR)&KeyObj, sizeof(KeyObj));
	/* Flush the AES inputs the server reads: the raw key bytes (when passed by address), the
	 * input data and the AAD. */
	if (KeyAddr != 0U) {
		Xil_DCacheFlushRange((UINTPTR)KeyAddr, XASU_KM_EXAMPLE_AES_KEY_LEN_IN_BYTES);
	}
	Xil_DCacheFlushRange((UINTPTR)InAddr, Len);
	Xil_DCacheFlushRange((UINTPTR)KmAesAad, XASU_KM_EXAMPLE_AES_AAD_LEN_IN_BYTES);
	/* DMA destination buffers: invalidate upfront to avoid dirty cache lines
	 * overwriting DMA output. */
	Xil_DCacheInvalidateRange((UINTPTR)OutAddr, Len);
	if (OperationType == XASU_AES_ENCRYPT_OPERATION) {
		Xil_DCacheInvalidateRange((UINTPTR)TagAddr, XASU_KM_EXAMPLE_AES_TAG_LEN_IN_BYTES);
	} else {
		/* On decrypt the server reads the tag to verify it, so flush the caller's tag bytes out. */
		Xil_DCacheFlushRange((UINTPTR)TagAddr, XASU_KM_EXAMPLE_AES_TAG_LEN_IN_BYTES);
	}
#endif

	XAsu_KmDemo_ResetResponseState();

	Status = XAsu_AesOperation(&ClientParams, &AesParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();
	if (ErrorStatus != (u32)XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	/* AES-GCM FINISH contract (see XAsu_AesOperation): the server reports the tag result via
	 * AdditionalStatus - TAG_READ when the encrypt wrote the tag, TAG_MATCHED when the decrypt
	 * verified it. Anything else is a failure. This is a redundant, anti-glitch confirmation on
	 * top of the main status, so an authenticated decrypt is never accepted on a tag mismatch. */
	if (OperationType == XASU_AES_ENCRYPT_OPERATION) {
		if (ClientParams.AdditionalStatus != (u32)XASU_AES_TAG_READ) {
			Status = XST_FAILURE;
			goto END;
		}
	} else if (ClientParams.AdditionalStatus != (u32)XASU_AES_TAG_MATCHED) {
		Status = XST_FAILURE;
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the ciphertext/plaintext the server wrote. */
	Xil_DCacheInvalidateRange((UINTPTR)OutAddr, Len);
	/* On encrypt the server produced the tag; invalidate so the CPU reads the fresh bytes. */
	if (OperationType == XASU_AES_ENCRYPT_OPERATION) {
		Xil_DCacheInvalidateRange((UINTPTR)TagAddr, XASU_KM_EXAMPLE_AES_TAG_LEN_IN_BYTES);
	}
#endif

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Uses the generated AES key and IV (referenced by their vault IDs) to AES-GCM encrypt
 *		a plaintext and then decrypt the resulting ciphertext, verifying the GCM tag and that
 *		the decrypted output matches the original plaintext.
 *
 *		This demonstrates "using" a vaulted key: the key/IV bytes never leave the ASU; the
 *		client only references them by KeyId/IvId. The encrypt produces an authentication tag
 *		over the ciphertext + AAD; the decrypt re-resolves the same IvId and verifies that tag,
 *		so the round-trip fails loudly on any tampering. The key and IV are provisioned with
 *		both ENC and DEC use cases, since the server resolves each once per operation (encrypt +
 *		decrypt).
 *
 * @return
 *		- XST_SUCCESS, if the encrypt/decrypt round-trip matches the plaintext.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_UseAesKey(void)
{
	s32 Status = XST_FAILURE;
	u32 Index;

	XAsu_KmDemo_PrintBuffer("Plaintext ", KmPlaintext, XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES);

	/* Encrypt: KmGenAesKeyId (the AES key generated in step 3) is used here in its DATA-KEY role -
	 * it directly AES-GCM-encrypts the plaintext, referenced by KeyId (KeyAddr = 0). This is the
	 * ordinary "encrypt data with a vaulted key" case; its second, KEK role is exercised later by
	 * the step-13 key-wrap workflow. The IV is KmGenAesIvId, resolved from the vault. */
	Status = XAsu_KmDemo_AesGcmOp(XASU_AES_ENCRYPT_OPERATION, 0U, KmGenAesKeyId,
				(u64)(UINTPTR)KmPlaintext, (u64)(UINTPTR)KmCiphertext,
				XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES, (u64)(UINTPTR)KmAesTag);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("AES encrypt with vaulted key failed: "
			"server status=%08x \r\n", ErrorStatus);
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("Ciphertext", KmCiphertext, XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES);
	XAsu_KmDemo_PrintBuffer("GCM tag   ", KmAesTag, XASU_KM_EXAMPLE_AES_TAG_LEN_IN_BYTES);

	/* Decrypt: the same KmGenAesKeyId data key (by KeyId) + KmGenAesIvId reverse the round-trip,
	 * and the GCM tag written by the encrypt is verified. */
	Status = XAsu_KmDemo_AesGcmOp(XASU_AES_DECRYPT_OPERATION, 0U, KmGenAesKeyId,
				(u64)(UINTPTR)KmCiphertext, (u64)(UINTPTR)KmDecrypted,
				XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES, (u64)(UINTPTR)KmAesTag);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("AES decrypt with vaulted key failed: "
			"server status=%08x \r\n", ErrorStatus);
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("Decrypted ", KmDecrypted, XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES);

	/* The round-trip is correct only if the decrypted output equals the original plaintext. */
	for (Index = 0U; Index < XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES; Index++) {
		if (KmDecrypted[Index] != KmPlaintext[Index]) {
			XilAsu_Printf("AES round-trip mismatch at byte %u: "
				"got 0x%02x expected 0x%02x \r\n",
				Index, KmDecrypted[Index], KmPlaintext[Index]);
			Status = XST_FAILURE;
			goto END;
		}
	}

	xil_printf("\r\n AES-GCM encrypt/decrypt round-trip with the vaulted key/IV succeeded: "
		"the decrypted data matches the plaintext \r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Computes an HMAC (SHA2-256) over the example message using the generated raw key
 *		referenced from the vault by its KeyId, writing the MAC to the given output buffer.
 *
 *		The key object sets KeyId (with KeyInAddr/KeyInLen = 0), so the ASU server resolves the
 *		raw key from the KDF/HMAC sub-vault; the key bytes never leave the ASU.
 *
 * @param	OutAddr		Address of the buffer that receives the computed HMAC.
 *
 * @return
 *		- XST_SUCCESS, if the HMAC is computed successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_HmacOp(u64 OutAddr)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_HmacParams HmacParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* Reference the generated raw key by KeyId; KeyInAddr = 0 selects vault resolution.
	 * KmGenRawKeyId is the KDF/HMAC key generated in step 6; here (step 7) it is resolved with the
	 * HMAC use case so the server keys the HMAC with it - the raw key bytes never leave the ASU. */
	HmacParams.KeyObject.KeyInAddr = 0U;
	HmacParams.KeyObject.KeyInLen = 0U;
	HmacParams.KeyObject.KeyId = KmGenRawKeyId;
	HmacParams.MsgBufferAddr = (u64)(UINTPTR)KmHmacMessage;
	HmacParams.HmacAddr = OutAddr;
	HmacParams.MsgLen = XASU_KM_EXAMPLE_HMAC_MSG_LEN_IN_BYTES;
	HmacParams.HmacLen = XASU_KM_EXAMPLE_HMAC_LEN_IN_BYTES;
	HmacParams.ShaType = XASU_SHA2_TYPE;
	HmacParams.ShaMode = XASU_SHA_MODE_256;
	HmacParams.IsLast = TRUE;
	HmacParams.OperationFlags = (u8)(XASU_INIT | XASU_UPDATE | XASU_FINISH);

#ifndef XASU_CACHE_DISABLE
	/* Flush the message the server reads; invalidate the HMAC destination up front so no dirty
	 * line evicts over the server's output. */
	Xil_DCacheFlushRange((UINTPTR)KmHmacMessage, XASU_KM_EXAMPLE_HMAC_MSG_LEN_IN_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)OutAddr, XASU_KM_EXAMPLE_HMAC_LEN_IN_BYTES);
#endif

	XAsu_KmDemo_ResetResponseState();

	Status = XAsu_HmacCompute(&ClientParams, &HmacParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();
	if (ErrorStatus != (u32)XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the computed HMAC the server wrote. */
	Xil_DCacheInvalidateRange((UINTPTR)OutAddr, XASU_KM_EXAMPLE_HMAC_LEN_IN_BYTES);
#endif

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Uses the generated raw key (referenced by its vault KeyId) to compute an HMAC over a
 *		message, and verifies correctness by recomputing the HMAC and checking the two MACs are
 *		identical.
 *
 *		The raw key is TRNG-generated, so its bytes are unknown to the client and a fixed
 *		known-answer is not possible; instead, correctness is shown via determinism: the same
 *		vaulted key + message must yield the same MAC on both computes. The server resolves the
 *		key once per compute (two resolves here), out of the four uses it is provisioned for
 *		(the remaining two are consumed by the KDF step).
 *
 * @return
 *		- XST_SUCCESS, if both HMACs are computed and match.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_UseRawKeyHmac(void)
{
	s32 Status = XST_FAILURE;
	u32 Index;

	XAsu_KmDemo_PrintBuffer("HMAC msg  ", KmHmacMessage, XASU_KM_EXAMPLE_HMAC_MSG_LEN_IN_BYTES);

	/* First HMAC over the message using the raw key resolved from the vault. */
	Status = XAsu_KmDemo_HmacOp((u64)(UINTPTR)KmHmac_Output1);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("HMAC compute #1 with vaulted key failed: "
				"server status=%08x \r\n", ErrorStatus);
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("HMAC #1   ", KmHmac_Output1, XASU_KM_EXAMPLE_HMAC_LEN_IN_BYTES);

	/* Second HMAC over the same key + message; must reproduce the same MAC. */
	Status = XAsu_KmDemo_HmacOp((u64)(UINTPTR)KmHmac_Output2);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("HMAC compute #2 with vaulted key failed: "
				"server status=%08x \r\n", ErrorStatus);
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("HMAC #2   ", KmHmac_Output2, XASU_KM_EXAMPLE_HMAC_LEN_IN_BYTES);

	/* The vaulted-key HMAC is correct only if both computes produced the same MAC. */
	for (Index = 0U; Index < XASU_KM_EXAMPLE_HMAC_LEN_IN_BYTES; Index++) {
		if (KmHmac_Output1[Index] != KmHmac_Output2[Index]) {
			XilAsu_Printf("HMAC mismatch at byte %u: 0x%02x vs 0x%02x \r\n",
					Index, KmHmac_Output1[Index], KmHmac_Output2[Index]);
			Status = XST_FAILURE;
			goto END;
		}
	}

	xil_printf("\r\n HMAC with the vaulted raw key succeeded as both the MACs match \r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Derives keying material with the KDF (SP800-108 counter mode, SHA2-256) over the fixed
 *		context using the generated raw key referenced from the vault by its KeyId, writing the
 *		derived key to the given output buffer.
 *
 *		The key object sets KeyId (with KeyInAddr/KeyInLen = 0), so the ASU server resolves the
 *		raw key from the KDF/HMAC sub-vault with the KDF use case; the key bytes never leave the
 *		ASU.
 *
 * @param	OutAddr		Address of the buffer that receives the derived keying material.
 *
 * @return
 *		- XST_SUCCESS, if the KDF derivation completes successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_KdfOp(u64 OutAddr)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KdfParams KdfParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* Reference the generated raw key by KeyId; KeyInAddr = 0 selects vault resolution.
	 * KmGenRawKeyId is the same KDF/HMAC key generated in step 6; here (step 8) it is resolved with
	 * the KDF use case so the server derives keying material from it - the raw key bytes never
	 * leave the ASU. */
	KdfParams.KeyObject.KeyInAddr = 0U;
	KdfParams.KeyObject.KeyInLen = 0U;
	KdfParams.KeyObject.KeyId = KmGenRawKeyId;
	KdfParams.ContextAddr = (u64)(UINTPTR)KmKdfContext;
	KdfParams.KeyOutAddr = OutAddr;
	KdfParams.ContextLen = XASU_KM_EXAMPLE_KDF_CONTEXT_LEN_IN_BYTES;
	KdfParams.KeyOutLen = XASU_KM_EXAMPLE_KDF_OUT_LEN_IN_BYTES;
	KdfParams.ShaType = XASU_SHA2_TYPE;
	KdfParams.ShaMode = XASU_SHA_MODE_256;

#ifndef XASU_CACHE_DISABLE
	/* Flush the KDF context the server reads; invalidate the output up front so no dirty line
	 * evicts over the server's derived bytes. */
	Xil_DCacheFlushRange((UINTPTR)KmKdfContext, XASU_KM_EXAMPLE_KDF_CONTEXT_LEN_IN_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)OutAddr, XASU_KM_EXAMPLE_KDF_OUT_LEN_IN_BYTES);
#endif

	XAsu_KmDemo_ResetResponseState();

	Status = XAsu_KdfGenerate(&ClientParams, &KdfParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();
	if (ErrorStatus != (u32)XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the derived KDF output the server wrote. */
	Xil_DCacheInvalidateRange((UINTPTR)OutAddr, XASU_KM_EXAMPLE_KDF_OUT_LEN_IN_BYTES);
#endif

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Uses the generated raw key (referenced by its vault KeyId) to derive keying material
 *		with the KDF, and verifies correctness by repeating the derivation and checking the two
 *		outputs are identical.
 *
 *		As with the HMAC step, the raw key is TRNG-generated so a fixed known-answer is not
 *		possible; correctness is shown via determinism: the same vaulted key + context must
 *		yield the same derived key on both derivations. The server resolves the key once per
 *		derivation (two resolves here), consuming the remaining two of the key's four uses.
 *
 * @return
 *		- XST_SUCCESS, if both derivations complete and match.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_UseRawKeyKdf(void)
{
	s32 Status = XST_FAILURE;
	u32 Index;

	XAsu_KmDemo_PrintBuffer("KDF ctx   ", KmKdfContext,
			XASU_KM_EXAMPLE_KDF_CONTEXT_LEN_IN_BYTES);

	/* First derivation over the context using the raw key resolved from the vault. */
	Status = XAsu_KmDemo_KdfOp((u64)(UINTPTR)KmKdf1);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("KDF derive #1 with vaulted key failed: "
				"server status=%08x \r\n", ErrorStatus);
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("KDF #1    ", KmKdf1, XASU_KM_EXAMPLE_KDF_OUT_LEN_IN_BYTES);

	/* Second derivation over the same key + context; must reproduce the same derived key. */
	Status = XAsu_KmDemo_KdfOp((u64)(UINTPTR)KmKdf2);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("KDF derive #2 with vaulted key failed: "
				"server status=%08x \r\n", ErrorStatus);
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("KDF #2    ", KmKdf2, XASU_KM_EXAMPLE_KDF_OUT_LEN_IN_BYTES);

	/* The vaulted-key KDF is correct only if both derivations produced the same output. */
	for (Index = 0U; Index < XASU_KM_EXAMPLE_KDF_OUT_LEN_IN_BYTES; Index++) {
		if (KmKdf1[Index] != KmKdf2[Index]) {
			XilAsu_Printf("KDF mismatch at byte %u: 0x%02x vs 0x%02x \r\n",
				Index, KmKdf1[Index], KmKdf2[Index]);
			Status = XST_FAILURE;
			goto END;
		}
	}

	xil_printf("\r\n KDF derivation with the vaulted raw key succeeded "
		"(both derivations match) \r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Generates the two ECC key pairs that back the two ECC use cases this example
 *		demonstrates - signing/verification and key agreement (ECDH) - directly inside the
 *		vault, in a single step.
 *
 *		Each GenerateEccKeyPair call returns two composite KeyIds via KeyIdAddr: element [0] is
 *		the public key ID and element [1] is the private key ID, so every destination buffer
 *		holds XASU_KM_EXAMPLE_ECC_NUM_KEY_IDS IDs. KeyObjectAddr is left 0 so both keys of a
 *		pair stay in the vault. The curve type is carried in KeyAttributes and the curve length
 *		in Length. The KeyUseCase bitmask is applied to both keys of a pair; each key interprets
 *		the bits per its own type.
 *
 *		Two pairs are generated by two successive GenerateEccKeyPair calls:
 *		- "Alice" is provisioned for both sign/verify and key agreement
 *		  (XASU_KM_EXAMPLE_ECC_SIGN_AGREE_USE_CASE). Her pair backs the sign/verify round-trip
 *		  (use case 1) and acts as one side of the ECDH exchange (use case 2).
 *		- "Bob" is provisioned for key agreement only
 *		  (XASU_KM_EXAMPLE_ECC_KEY_AGREE_USE_CASE) and serves as the ECDH peer, providing the
 *		  other side of the shared-secret derivation (use case 2).
 *
 *		The two use cases themselves are exercised by the subsequent steps
 *		(XAsu_KmDemo_UseEccSignVerify and XAsu_KmDemo_UseEcdh) using the IDs captured here.
 *
 * @return
 *		- XST_SUCCESS, if both key pairs are generated successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_GenerateEccKeyPair(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KeyManagerParams GenEccParams = {0};

	/* Curve type and length are common to both pairs. */
	/* Target the vault created in step 1 (KmVaultId, CreateVault). */
	GenEccParams.KeyMetadata.CompositeKeyId = XASU_KM_COMPOSE_KEY_ID((u8)KmVaultId,
			(u8)XASU_ECC_PVT_SUBVAULT_ID, 0U);
	GenEccParams.KeyMetadata.KeyAttributes = (u8)XASU_KM_EXAMPLE_ECC_CURVE;
	GenEccParams.KeyMetadata.Length = XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES;

	/* Generate the "Alice" pair, provisioned for both sign/verify and key agreement. */
	XAsu_KmDemo_InitClientParams(&ClientParams);
	GenEccParams.KeyIdAddr = (u64)(UINTPTR)KmEccAliceIds;
	GenEccParams.KeyMetadata.KeyUseCase = (u8)XASU_KM_EXAMPLE_ECC_SIGN_AGREE_USE_CASE;
	GenEccParams.KeyMetadata.UsageCount = XASU_KM_EXAMPLE_GEN_USAGE_COUNT;

	XAsu_KmDemo_ResetResponseState();
	KmEccAliceIds[0] = 0U;
	KmEccAliceIds[1] = 0U;

#ifndef XASU_CACHE_DISABLE
	/* Clean the key-pair ID slots before the request so dirty lines can't clobber the
	 * server's write. */
	Xil_DCacheFlushRange((UINTPTR)KmEccAliceIds, sizeof(KmEccAliceIds));
#endif

	Status = XAsu_KmGenerateEccKeyPair(&ClientParams, &GenEccParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("GenerateEccKeyPair request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the Alice key-pair IDs the server wrote back. */
	Xil_DCacheInvalidateRange((UINTPTR)KmEccAliceIds, sizeof(KmEccAliceIds));
#endif

	if ((ErrorStatus != (u32)XST_SUCCESS) || (KmEccAliceIds[0] == 0U) ||
	(KmEccAliceIds[1] == 0U)) {
		XilAsu_Printf("GenerateEccKeyPair failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n Generated Alice ECC key pair, PubKeyId = 0x%08x PvtKeyId = 0x%08x \r\n",
		KmEccAliceIds[0], KmEccAliceIds[1]);

	/* Generate the "Bob" pair, provisioned for key agreement only (the ECDH peer). */
	XAsu_KmDemo_InitClientParams(&ClientParams);
	GenEccParams.KeyIdAddr = (u64)(UINTPTR)KmEccBobIds;
	GenEccParams.KeyMetadata.KeyUseCase = (u8)XASU_KM_EXAMPLE_ECC_KEY_AGREE_USE_CASE;
	GenEccParams.KeyMetadata.UsageCount = XASU_KM_EXAMPLE_ECC_PEER_USAGE_COUNT;

	XAsu_KmDemo_ResetResponseState();
	KmEccBobIds[0] = 0U;
	KmEccBobIds[1] = 0U;

#ifndef XASU_CACHE_DISABLE
	/* Clean the key-pair ID slots before the request so dirty lines can't clobber the
	 * server's write. */
	Xil_DCacheFlushRange((UINTPTR)KmEccBobIds, sizeof(KmEccBobIds));
#endif

	Status = XAsu_KmGenerateEccKeyPair(&ClientParams, &GenEccParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("GenerateEccKeyPair request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the Bob key-pair IDs the server wrote back. */
	Xil_DCacheInvalidateRange((UINTPTR)KmEccBobIds, sizeof(KmEccBobIds));
#endif

	if ((ErrorStatus != (u32)XST_SUCCESS) || (KmEccBobIds[0] == 0U) || (KmEccBobIds[1] == 0U)) {
		XilAsu_Printf("GenerateEccKeyPair failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n Generated Bob ECC key pair, PubKeyId = 0x%08x PvtKeyId = 0x%08x \r\n",
		KmEccBobIds[0], KmEccBobIds[1]);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Signs the example digest using the vaulted ECC private key referenced by its KeyId,
 *		writing the signature (r||s) to KmEccSignature.
 *
 *		Key.KeyAddr = 0 with a non-zero Key.KeyId selects vault resolution, so the private key
 *		bytes never leave the ASU.
 *
 * @param	PvtKeyId	Composite KeyId of the vaulted ECC private key.
 *
 * @return
 *		- XST_SUCCESS, if the signature is generated successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_EccSignOp(u32 PvtKeyId)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_EccParams EccParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* Reference the vaulted private key by KeyId; KeyAddr = 0 selects vault resolution. */
	EccParams.Key.KeyAddr = 0U;
	EccParams.Key.KeyId = PvtKeyId;
	EccParams.Key.KeyLen = 0U;
	EccParams.DigestAddr = (u64)(UINTPTR)KmEccDigest;
	EccParams.SignAddr = (u64)(UINTPTR)KmEccSignature;
	EccParams.Key.CurveType = XASU_KM_EXAMPLE_ECC_CURVE;
	EccParams.DigestLen = XASU_KM_EXAMPLE_ECC_DIGEST_LEN_IN_BYTES;

#ifndef XASU_CACHE_DISABLE
	/* Flush the digest the server signs; invalidate the signature buffer up front so no dirty
	 * line evicts over the server's output. */
	Xil_DCacheFlushRange((UINTPTR)KmEccDigest, XASU_KM_EXAMPLE_ECC_DIGEST_LEN_IN_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)KmEccSignature, XASU_KM_EXAMPLE_ECC_SIG_LEN_IN_BYTES);
#endif

	XAsu_KmDemo_ResetResponseState();

	Status = XAsu_EccGenSign(&ClientParams, &EccParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();
	if (ErrorStatus != (u32)XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the signature the server wrote. */
	Xil_DCacheInvalidateRange((UINTPTR)KmEccSignature, XASU_KM_EXAMPLE_ECC_SIG_LEN_IN_BYTES);
#endif

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Verifies the signature in KmEccSignature over the example digest using the vaulted
 *		ECC public key referenced by its KeyId.
 *
 * @param	PubKeyId	Composite KeyId of the vaulted ECC public key.
 *
 * @return
 *		- XST_SUCCESS, if the signature verifies successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_EccVerifyOp(u32 PubKeyId)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_EccParams EccParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* Reference the vaulted public key by KeyId; KeyAddr = 0 selects vault resolution. */
	EccParams.Key.KeyAddr = 0U;
	EccParams.Key.KeyId = PubKeyId;
	EccParams.Key.KeyLen = 0U;
	EccParams.DigestAddr = (u64)(UINTPTR)KmEccDigest;
	EccParams.SignAddr = (u64)(UINTPTR)KmEccSignature;
	EccParams.Key.CurveType = XASU_KM_EXAMPLE_ECC_CURVE;
	EccParams.DigestLen = XASU_KM_EXAMPLE_ECC_DIGEST_LEN_IN_BYTES;

#ifndef XASU_CACHE_DISABLE
	/* Flush the digest and signature the server reads to verify. */
	Xil_DCacheFlushRange((UINTPTR)KmEccDigest, XASU_KM_EXAMPLE_ECC_DIGEST_LEN_IN_BYTES);
	Xil_DCacheFlushRange((UINTPTR)KmEccSignature, XASU_KM_EXAMPLE_ECC_SIG_LEN_IN_BYTES);
#endif

	XAsu_KmDemo_ResetResponseState();

	Status = XAsu_EccVerifySign(&ClientParams, &EccParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();
	if (ErrorStatus != (u32)XST_SUCCESS) {
		Status = XST_FAILURE;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Uses Alice's vaulted ECC key pair (referenced by their KeyIds) to sign a digest and
 *		then verify the resulting signature, demonstrating a self-checking sign-then-verify
 *		round-trip. The private and public key bytes never leave the ASU.
 *
 *		Alice's keys are provisioned with UsageCount = 2 because each is resolved once here
 *		(sign for the private key, verify for the public key) and once more during ECDH.
 *
 * @return
 *		- XST_SUCCESS, if the signature is generated and verifies.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_UseEccSignVerify(void)
{
	s32 Status = XST_FAILURE;
	u32 Index;

	/*
	 * ECDSA signs a message digest, not the message itself. In a real application the digest is
	 * the output of a cryptographic hash (e.g. SHA-256 over the message), which for P-256 is the
	 * 32-byte value placed at DigestAddr. Here, to keep the example focused on the vaulted-key
	 * sign/verify mechanics, we skip the hashing and just populate the digest buffer with a fixed,
	 * deterministic, non-zero byte pattern (0x01, 0x02, ... ) that stands in for a real hash. The
	 * round-trip succeeds as long as sign and verify use the same digest bytes; their actual value
	 * is irrelevant for self-consistency. (Non-zero avoids a degenerate all-zero scalar.)
	 */
	for (Index = 0U; Index < XASU_KM_EXAMPLE_ECC_DIGEST_LEN_IN_BYTES; Index++) {
		KmEccDigest[Index] = (u8)(Index + 1U);
	}

	XAsu_KmDemo_PrintBuffer("ECC digest", KmEccDigest, XASU_KM_EXAMPLE_ECC_DIGEST_LEN_IN_BYTES);

	/* Sign the digest with Alice's vaulted private key (KmEccAliceIds[1], generated in step 9;
	 * element [1] is the private key, [0] the public key). */
	Status = XAsu_KmDemo_EccSignOp(KmEccAliceIds[1]);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ECC sign with vaulted key failed: "
			"server status=%08x \r\n", ErrorStatus);
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("ECC sig   ", KmEccSignature, XASU_KM_EXAMPLE_ECC_SIG_LEN_IN_BYTES);

	/* Verify the signature with Alice's vaulted public key (KmEccAliceIds[0], from step 9). */
	Status = XAsu_KmDemo_EccVerifyOp(KmEccAliceIds[0]);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ECC verify with vaulted key failed: "
			"server status=%08x \r\n", ErrorStatus);
		goto END;
	}

	xil_printf("\r\n ECC sign/verify round-trip with the vaulted key pair succeeded \r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Performs a single ECDH operation that references the local private key and the peer's
 *		public key from the vault by their IDs, writing the shared secret to the given buffer.
 *
 *		Both PvtKey and PubKey set KeyAddr = 0 with a non-zero KeyId, so the ASU server
 *		resolves both keys from the vault; no key bytes are supplied by the client.
 *
 * @param	PvtKeyId	Composite KeyId of the local vaulted ECC private key.
 * @param	PeerPubKeyId	Composite KeyId of the peer's vaulted ECC public key.
 * @param	SharedSecretAddr	Address of the buffer that receives the shared secret.
 *
 * @return
 *		- XST_SUCCESS, if the shared secret is derived successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_EcdhOp(u32 PvtKeyId, u32 PeerPubKeyId, u64 SharedSecretAddr)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_EcdhParams EcdhParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* Local private key, resolved from the vault by KeyId. */
	EcdhParams.PvtKey.KeyAddr = 0U;
	EcdhParams.PvtKey.KeyId = PvtKeyId;
	EcdhParams.PvtKey.KeyLen = 0U;
	/* Peer public key, resolved from the vault by KeyId. */
	EcdhParams.PubKey.KeyAddr = 0U;
	EcdhParams.PubKey.KeyId = PeerPubKeyId;
	EcdhParams.PubKey.KeyLen = 0U;
	EcdhParams.SharedSecretAddr = SharedSecretAddr;
	EcdhParams.SharedSecretLen = XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES;
	EcdhParams.SharedSecretObjIdAddr = 0U;
	EcdhParams.PvtKey.CurveType = XASU_KM_EXAMPLE_ECC_CURVE;

#ifndef XASU_CACHE_DISABLE
	/* Invalidate the shared-secret destination before the request so no dirty line evicts
	 * over the server's output. */
	Xil_DCacheInvalidateRange((UINTPTR)SharedSecretAddr,
				  XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES);
#endif

	XAsu_KmDemo_ResetResponseState();

	Status = XAsu_EcdhGenSharedSecret(&ClientParams, &EcdhParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();
	if (ErrorStatus != (u32)XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the shared secret the server wrote. */
	Xil_DCacheInvalidateRange((UINTPTR)SharedSecretAddr,
				  XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES);
#endif

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Derives the ECDH shared secret from both sides of the exchange using the vaulted ECC
 *		key pairs, and verifies correctness via ECDH's symmetry property: the secret computed
 *		from Alice's private + Bob's public key must equal the secret computed from Bob's
 *		private + Alice's public key. All key bytes stay inside the ASU.
 *
 *		This needs no known-answer vector: the two independently derived secrets are expected to
 *		be byte-identical. Each participating key is resolved once per ECDH call.
 *
 * @return
 *		- XST_SUCCESS, if both shared secrets are derived and match.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_UseEcdh(void)
{
	s32 Status = XST_FAILURE;
	u32 Index;

	/* Side A: Alice's private key + Bob's public key (KmEccAliceIds[1] + KmEccBobIds[0], both
	 * generated in step 9). */
	Status = XAsu_KmDemo_EcdhOp(KmEccAliceIds[1], KmEccBobIds[0],
			(u64)(UINTPTR)KmEcdhSecretA);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ECDH (Alice priv + Bob pub) failed: "
			"server status=%08x \r\n", ErrorStatus);
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("ECDH A    ", KmEcdhSecretA, XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES);

	/* Side B: Bob's private key + Alice's public key (KmEccBobIds[1] + KmEccAliceIds[0], both
	 * generated in step 9). */
	Status = XAsu_KmDemo_EcdhOp(KmEccBobIds[1], KmEccAliceIds[0],
			(u64)(UINTPTR)KmEcdhSecretB);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ECDH (Bob priv + Alice pub) failed: "
			"server status=%08x \r\n", ErrorStatus);
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("ECDH B    ", KmEcdhSecretB, XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES);

	/* ECDH is correct only if both sides derived the same shared secret. */
	for (Index = 0U; Index < XASU_KM_EXAMPLE_ECC_KEY_LEN_IN_BYTES; Index++) {
		if (KmEcdhSecretA[Index] != KmEcdhSecretB[Index]) {
			XilAsu_Printf("ECDH shared-secret mismatch at byte %u: "
				"0x%02x vs 0x%02x \r\n",
				Index, KmEcdhSecretA[Index], KmEcdhSecretB[Index]);
			Status = XST_FAILURE;
			goto END;
		}
	}

	xil_printf("\r\n ECDH exchange with the vaulted key pairs succeeded "
		"(both the shared secrets match) \r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Wraps the client-held DEK with the vaulted RSA public key using XAsu_KeyWrap, using the
 *		vaulted generated AES key as the key-encryption key (KEK).
 *
 *		The wrap service RSA-OAEP-encodes the KEK with the RSA public key resolved by RsaKeyId and
 *		AES-KWP-wraps the DEK with that KEK. The output is the hybrid blob RSA-OAEP(KEK) ||
 *		AES-KWP(DEK). AesKeyId selects the vaulted KEK (resolved with the KEY_WRAP use case)
 *		instead of an ephemeral key; KeyCompAddr = 0 so the RSA key is taken from the vault by
 *		RsaKeyId. Because the KEK lives in the vault, StoreKey can later recover it by KeyId to
 *		AES-KWP-unwrap the DEK, so no client-side RSA-OAEP peel is needed.
 *
 * @return
 *		- XST_SUCCESS, if the DEK is wrapped successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_RsaWrapDek(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KeyWrapParams WrapParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* Wrap input: KmDek is the client-held DEK, the payload being secured. The server AES-KWP-wraps
	 * these raw DEK bytes with the vaulted KEK to form the trailing AES-KWP portion of the output
	 * blob. */
	WrapParams.InputDataAddr = (u64)(UINTPTR)KmDek;
	/* Wrap output: the server writes the full hybrid blob RSA-OAEP(KEK) || AES-KWP(DEK) into
	 * KmWrappedBlob here; KmWrapOutLen reports the actual byte count written. */
	WrapParams.OutputDataAddr = (u64)(UINTPTR)KmWrappedBlob;
	WrapParams.ExpoCompAddr = 0U;
	WrapParams.KeyCompAddr = 0U;
	WrapParams.OptionalLabelAddr = (u64)(UINTPTR)KmOaepLabel;
	WrapParams.ActualOutputDataLenAddr = (u64)(UINTPTR)&KmWrapOutLen;
	WrapParams.InputDataLen = XASU_KM_EXAMPLE_DEK_LEN_IN_BYTES;
	WrapParams.RsaKeySize = XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES;
	WrapParams.OptionalLabelSize = XASU_KM_EXAMPLE_OAEP_LABEL_SIZE_IN_BYTES;
	WrapParams.OutputDataLen = XASU_KM_EXAMPLE_WRAPPED_BLOB_SIZE_IN_BYTES;
	/* KEK role (step 13): KmGenAesKeyId - the same vaulted AES key generated in step 3 and used as
	 * the plain data key in step 5 - is supplied here as the key-ENCRYPTION key. XAsu_KeyWrap
	 * resolves it (KEY_WRAP use case) and AES-KWP-wraps the client DEK (KmDek) with it, while the
	 * RSA public key (KmRsaPubKeyId) RSA-OAEP-wraps the KEK itself. It is NOT the key that gets
	 * imported - KmDek is - which is exactly why a separate DEK exists here rather than reusing
	 * KmGenAesKeyId as the payload. */
	WrapParams.AesKeyId = KmGenAesKeyId;
	WrapParams.RsaKeyId = KmRsaPubKeyId;
	WrapParams.AesKeySize = XASU_AES_KEY_SIZE_256_BITS;
	WrapParams.ShaType = XASU_SHA2_TYPE;
	WrapParams.ShaMode = XASU_SHA_MODE_256;

#ifndef XASU_CACHE_DISABLE
	/* Flush the DEK and OAEP label the server reads to build the wrapped blob. */
	Xil_DCacheFlushRange((UINTPTR)KmDek, sizeof(KmDek));
	Xil_DCacheFlushRange((UINTPTR)KmOaepLabel, sizeof(KmOaepLabel));
#endif

	XAsu_KmDemo_ResetResponseState();
	KmWrapOutLen = 0U;

#ifndef XASU_CACHE_DISABLE
	/* Clean the output-length slot before the request so a dirty line can't clobber the
	 * length the server writes. */
	Xil_DCacheFlushRange((UINTPTR)&KmWrapOutLen, sizeof(KmWrapOutLen));
	/* KmWrappedBlob is a DMA destination: invalidate before submitting so dirty lines
	 * can't evict over the wrapped blob the server produces. */
	Xil_DCacheInvalidateRange((UINTPTR)KmWrappedBlob, sizeof(KmWrappedBlob));
#endif

	Status = XAsu_KeyWrap(&ClientParams, &WrapParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("KeyWrap request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the actual wrapped length the server wrote. */
	Xil_DCacheInvalidateRange((UINTPTR)&KmWrapOutLen, sizeof(KmWrapOutLen));
#endif

	if (ErrorStatus != (u32)XST_SUCCESS) {
		XilAsu_Printf("KeyWrap failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	if (KmWrapOutLen != XASU_KM_EXAMPLE_WRAPPED_BLOB_SIZE_IN_BYTES) {
		XilAsu_Printf("KeyWrap produced unexpected length: %u \r\n", KmWrapOutLen);
		Status = XST_FAILURE;
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the wrapped blob the server produced. */
	Xil_DCacheInvalidateRange((UINTPTR)KmWrappedBlob, sizeof(KmWrappedBlob));
#endif

	xil_printf("\r\n Wrapped DEK with vaulted RSA public key (KeyId = 0x%08x) + vaulted AES "
		"KEK (KeyId = 0x%08x): hybrid blob = %u bytes \r\n", KmRsaPubKeyId,
		KmGenAesKeyId, KmWrapOutLen);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Stores the wrapped DEK into the AES sub-vault via StoreKey, capturing the assigned
 *		KeyId in KmDekId.
 *
 *		StoreKey AES-KWP-unwraps the DEK, so KeyObjectAddr points past the RSA-OAEP prefix to the
 *		AES-KWP portion of the blob, and the KEK is supplied by KeyId (AesKeyObj.KeyId, with
 *		KeyAddress = 0) so the server resolves it from the vault with the KEY_UNWRAP use case.
 *		The wrapped-key bit in KeyType tells the server to AES-KWP-unwrap
 *		the input with that KEK and store the recovered DEK; the unwrapped length must equal
 *		KeyMetadata.Length. The DEK is provisioned for ENC/DEC use so it can be referenced by KeyId
 *		for both the verify re-encrypt and the decrypt below.
 *
 * @return
 *		- XST_SUCCESS, if the DEK is unwrapped and stored successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_StoreWrappedDek(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KeyManagerParams KmParams = {0};

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* Consumer: hand StoreKey only the trailing AES-KWP(DEK) portion of KmWrappedBlob by advancing
	 * the pointer past the RSA-OAEP(KEK) prefix (XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES bytes). The
	 * server AES-KWP-unwraps the DEK from this portion using the KEK it resolves from the vault. */
	KmParams.KeyObjectAddr = (u64)(UINTPTR)(KmWrappedBlob +
						XASU_KM_EXAMPLE_RSA_KEY_SIZE_IN_BYTES);
	KmParams.KeyObjectLen = XASU_KM_EXAMPLE_AES_KWP_BLOB_SIZE_IN_BYTES;
	/* Producer site: StoreKey writes the composite KeyId it assigns to the stored (unwrapped) DEK
	 * back into KmDekId here; the workflow later references the vaulted DEK by this id. */
	KmParams.KeyIdAddr = (u64)(UINTPTR)&KmDekId;
	KmParams.WrappedInputLen = XASU_KM_EXAMPLE_AES_KWP_BLOB_SIZE_IN_BYTES;
	/* KEK role (step 13, unwrap): KmGenAesKeyId is handed to StoreKey by KeyId (KeyAddress = 0)
	 * so the server resolves the same vaulted AES KEK (KEY_UNWRAP use case) and AES-KWP-unwraps the
	 * DEK from the blob. This mirrors the wrap above; the recovered DEK - not this KEK - is what
	 * ends up stored in the AES sub-vault (its id is returned in KmDekId). */
	KmParams.AesKeyObj.KeyAddress = 0U;
	KmParams.AesKeyObj.KeySize = XASU_AES_KEY_SIZE_256_BITS;
	KmParams.AesKeyObj.KeySrc = XASU_AES_USER_KEY_0;
	KmParams.AesKeyObj.KeyId = KmGenAesKeyId;
	/* Target the vault created in step 1 (KmVaultId, CreateVault). */
	KmParams.KeyMetadata.CompositeKeyId = XASU_KM_COMPOSE_KEY_ID((u8)KmVaultId,
			(u8)(XASU_AES_SUBVAULT_ID | XASU_KM_KEYTYPE_WRAPPED_BIT_MASK), 0U);
	KmParams.KeyMetadata.KeyUseCase = (u8)XASU_KM_EXAMPLE_AES_USE_CASE;
	/* The stored DEK is resolved three times overall: the verify re-encrypt and the decrypt below
	 * (step 13), plus one decrypt after the export/import round-trip (step 15 content check). */
	KmParams.KeyMetadata.UsageCount = XASU_KM_EXAMPLE_DEK_USAGE_COUNT;
	KmParams.KeyMetadata.EpochTime = 0U;
	KmParams.KeyMetadata.Length = XASU_KM_EXAMPLE_DEK_LEN_IN_BYTES;

#ifndef XASU_CACHE_DISABLE
	/* Flush the wrapped blob the server reads to unwrap the DEK. */
	Xil_DCacheFlushRange((UINTPTR)KmWrappedBlob, sizeof(KmWrappedBlob));
#endif

	XAsu_KmDemo_ResetResponseState();
	KmDekId = 0U;

#ifndef XASU_CACHE_DISABLE
	/* Clean the KeyId output slot before the request so a dirty line can't clobber the
	 * server's write. */
	Xil_DCacheFlushRange((UINTPTR)&KmDekId, sizeof(KmDekId));
#endif

	Status = XAsu_KmStoreKey(&ClientParams, &KmParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("StoreKey (wrapped DEK) request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the DEK KeyId the server wrote back. */
	Xil_DCacheInvalidateRange((UINTPTR)&KmDekId, sizeof(KmDekId));
#endif

	if ((ErrorStatus != (u32)XST_SUCCESS) || (KmDekId == 0U)) {
		XilAsu_Printf("StoreKey (wrapped DEK) failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n Stored unwrapped DEK in AES sub-vault, KeyId = 0x%08x \r\n", KmDekId);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Executes the complete key-wrap workflow in six steps:
 *		(1) the client-held DEK AES-GCM-encrypts the plaintext;
 *		(2) the DEK is wrapped with the vaulted RSA public key, using the vaulted AES key as the
 *		    key-encryption key (KEK);
 *		(3) StoreKey resolves that KEK from the vault by its KeyId, AES-KWP-unwraps the DEK and
 *		    stores it in the vault;
 *		(4) the stored key is proven to be the DEK by re-encrypting the plaintext with the vaulted
 *		    DEK (by KeyId) and confirming the ciphertext is byte-for-byte identical to the one the
 *		    raw DEK produced;
 *		(5) the ciphertext is AES-GCM-decrypted with the now-vaulted DEK, referenced only by its
 *		    KeyId; and
 *		(6) the recovered plaintext is compared against the original.
 *
 *		This function illustrates secure key transport: the DEK exists in the clear on the
 *		client only until it is wrapped, after which it resides in the vault and is used
 *		exclusively by KeyId. The workflow is self-checking on three levels - the re-encrypt
 *		ciphertext must match the raw-DEK ciphertext (proving the stored key equals the
 *		DEK), the AES-GCM tag must verify on decrypt, and the recovered plaintext must equal
 *		the original.
 *
 * @return
 *		- XST_SUCCESS, if the round-trip succeeds and the recovered plaintext matches.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_KeyWrapWorkflow(void)
{
	s32 Status = XST_FAILURE;
	u32 Index;

	/** 1. AES-GCM-encrypt the plaintext with the client-held DEK (raw key bytes, KeyId = 0). */
	XAsu_KmDemo_PrintBuffer("Workflow plaintext ", KmPlaintext,
			XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES);

	Status = XAsu_KmDemo_AesGcmOp(XASU_AES_ENCRYPT_OPERATION, (u64)(UINTPTR)KmDek, 0U,
				(u64)(UINTPTR)KmPlaintext, (u64)(UINTPTR)KmDekCiphertext,
				XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES, (u64)(UINTPTR)KmDekTag);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Key-wrap workflow: DEK encrypt failed \r\n");
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("Workflow ciphertext", KmDekCiphertext,
			XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES);
	XAsu_KmDemo_PrintBuffer("Workflow GCM tag   ", KmDekTag,
			XASU_KM_EXAMPLE_AES_TAG_LEN_IN_BYTES);

	/** 2. Wrap the DEK with the vaulted RSA public key + vaulted AES KEK (RSA-OAEP + AES-KWP). */
	Status = XAsu_KmDemo_RsaWrapDek();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** 3. Hand the AES-KWP portion to StoreKey, which resolves the KEK by KeyId, unwraps and
	 *  vaults the DEK */
	Status = XAsu_KmDemo_StoreWrappedDek();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** 4. Prove the stored key is the DEK before trusting the decrypt: AES-GCM-encrypt the same
	 *  plaintext again, this time with the now-vaulted DEK referenced only by its KeyId (KmDekId,
	 *  produced by StoreKey in step 3), and check the ciphertext is byte-for-byte identical to
	 *  KmDekCiphertext (produced with the raw DEK).
	 *  The two encrypts share the same key candidate, IV (KmGenAesIvId), AAD and plaintext, so an
	 *  identical ciphertext can only mean the wrapped/stored key equals the DEK. */
	Status = XAsu_KmDemo_AesGcmOp(XASU_AES_ENCRYPT_OPERATION, 0U, KmDekId,
				(u64)(UINTPTR)KmPlaintext, (u64)(UINTPTR)KmCiphertext_ReVerif,
				XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES,
				(u64)(UINTPTR)KmDek_ReVerifGCMTag);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Key-wrap workflow: verify re-encrypt by KeyId failed \r\n");
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("Verify ciphertext  ", KmCiphertext_ReVerif,
			XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES);

	for (Index = 0U; Index < XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES; Index++) {
		if (KmCiphertext_ReVerif[Index] != KmDekCiphertext[Index]) {
			XilAsu_Printf("Key-wrap workflow: stored key != DEK (ciphertext mismatch "
					"at byte %u: 0x%02x vs 0x%02x) \r\n", Index,
					KmCiphertext_ReVerif[Index], KmDekCiphertext[Index]);
			Status = XST_FAILURE;
			goto END;
		}
	}

	xil_printf("\r\n Verified the stored key is the DEK "
		"(re-encrypt ciphertext identical) \r\n");

	/** 5. AES-GCM-decrypt the ciphertext with the now-vaulted DEK referenced only by its KeyId
	 *  (KmDekId); the GCM tag verifies the vaulted DEK matches the raw DEK that produced the
	 *  ciphertext. */
	Status = XAsu_KmDemo_AesGcmOp(XASU_AES_DECRYPT_OPERATION, 0U, KmDekId,
				(u64)(UINTPTR)KmDekCiphertext, (u64)(UINTPTR)KmDekDecrypted,
				XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES, (u64)(UINTPTR)KmDekTag);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Key-wrap workflow: DEK decrypt by KeyId failed \r\n");
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("Workflow decrypted ", KmDekDecrypted,
			XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES);

	/** 6. The workflow is correct only if the recovered plaintext matches the original. */
	for (Index = 0U; Index < XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES; Index++) {
		if (KmDekDecrypted[Index] != KmPlaintext[Index]) {
			XilAsu_Printf("Key-wrap workflow plaintext mismatch at byte %u: 0x%02x vs "
					"0x%02x \r\n", Index, KmDekDecrypted[Index],
					KmPlaintext[Index]);
			Status = XST_FAILURE;
			goto END;
		}
	}

	xil_printf("\r\n Key-wrap workflow succeeded (DEK unwrapped, stored, "
		"and reused by KeyId) \r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Exports the calling subsystem's key vault into a single encrypted, authenticated blob.
 *
 *		ExportKeyVault serialises the whole user-vault region (its registry and every key it
 *		holds) and protects it with AES-GCM: the server emits a 16-byte IV, the encrypted vault
 *		image and a 16-byte GCM tag into the client-supplied buffer, and reports the actual blob
 *		size. The blob can then be persisted off-chip and restored later with ImportKeyVault. The
 *		key bytes never leave the ASU in the clear; export does not consume any key usage counts.
 *
 * @return
 *		- XST_SUCCESS, if the vault is exported and the reported size is within range.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_ExportKeyVault(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KeyVaultTransferParams ExportParams = {0};
	u32 PreviewLen;

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* DataAddr is the destination buffer and BufSize its capacity. ExportResultAddr points at
	 * the result block the blob size and the modified flag come back in, RevokeEfuseNum is the
	 * eFuse the blob is bound to, and Context is what its keys are derived from. */
	ExportParams.DataAddr = (u64)(UINTPTR)KmExportBuffer;
	ExportParams.BufSize = XASU_KM_EXAMPLE_EXPORT_BUF_SIZE;
	ExportParams.ExportResultAddr = (u64)(UINTPTR)&KmExportResult;
	ExportParams.RevokeEfuseNum = XASU_KM_EXAMPLE_REVOKE_EFUSE;

	Status = Xil_SMemCpy(ExportParams.Context, sizeof(ExportParams.Context), KmExportContext,
			sizeof(KmExportContext), sizeof(KmExportContext));
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ExportKeyVault could not set the context: %08x \r\n", Status);
		goto END;
	}

	KmExportSize = 0U;

#ifndef XASU_CACHE_DISABLE
	/* Invalidate the export buffer up front so no dirty line clobbers the blob the server
	 * DMAs back. The result block is not invalidated: the client library copies it out of the
	 * response buffer with the CPU, so it is already coherent. */
	Xil_DCacheInvalidateRange((UINTPTR)KmExportBuffer, XASU_KM_EXAMPLE_EXPORT_BUF_SIZE);
#endif

	XAsu_KmDemo_ResetResponseState();

	Status = XAsu_KmExportKeyVault(&ClientParams, &ExportParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ExportKeyVault request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();
	if (ErrorStatus != (u32)XST_SUCCESS) {
		XilAsu_Printf("ExportKeyVault failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	KmExportSize = KmExportResult.ActualSize;

	/* A valid blob is at least the AEKV header plus one chunk, and must fit the buffer the
	 * server was given. */
	if ((KmExportSize < XASU_KM_EXAMPLE_EXPORT_MIN_SIZE) ||
	(KmExportSize > XASU_KM_EXAMPLE_EXPORT_BUF_SIZE)) {
		XilAsu_Printf("ExportKeyVault returned an out-of-range size: %u \r\n",
					KmExportSize);
		Status = XST_FAILURE;
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	/* Invalidate so the CPU reads the exported vault blob the server wrote. */
	Xil_DCacheInvalidateRange((UINTPTR)KmExportBuffer, KmExportSize);
#endif

	/* The blob opens with the AEKV tag, so a buffer the server never wrote is caught here. */
	if (Xil_In32((UINTPTR)KmExportBuffer) != XASU_KM_AEKV_TAG) {
		XilAsu_Printf("ExportKeyVault produced a blob with tag %08x \r\n",
					Xil_In32((UINTPTR)KmExportBuffer));
		Status = XST_FAILURE;
		goto END;
	}

	/* Never read past the bytes the server actually wrote: cap the preview at the blob size. */
	PreviewLen = (KmExportSize < XASU_KM_EXAMPLE_EXPORT_PREVIEW_LEN) ?
			KmExportSize : XASU_KM_EXAMPLE_EXPORT_PREVIEW_LEN;

	/* Modified tells whether the vault changed since the export before this one, which is what
	 * a backup scheme uses to decide it has to keep the new blob. */
	xil_printf("\r\n Exported vault, size = %u bytes, key vault size = %u bytes, "
			"modified = %u \r\n", KmExportSize,
			Xil_In32((UINTPTR)KmExportBuffer + XASU_KM_AEKV_HDR_OFFSET_KV_SIZE),
			KmExportResult.Modified);
	XAsu_KmDemo_PrintBuffer("Exported vault (preview)", KmExportBuffer, PreviewLen);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Imports a key vault from a previously exported blob, then verifies the round-trip.
 *
 *		ImportKeyVault verifies and decrypts the blob produced by ExportKeyVault and
 *		rebuilds the vault(s) it contains - preserving VaultIds and KeyIds - so the keys
 *		provisioned earlier remain usable under their original IDs (the teardown deletes
 *		the stored ECC key and the vault by those same IDs after this step).
 *
 *		Import restores into an empty region and never discards a key vault on the
 *		caller's behalf, so the vault this example created is deleted first. Importing
 *		over it would be refused with XASUFW_KEYMANAGER_AEKV_VAULT_PRESENT.
 *
 *		The round-trip is self-checked on content: a vaulted key is exercised through its preserved
 *		KeyId - KmDekCiphertext (produced in step 13) is decrypted with the imported DEK (KmDekId)
 *		and the recovered plaintext is compared against KmPlaintext. The AES-GCM tag check plus a
 *		byte-for-byte match proves import restored the actual key BYTES (not merely the vault
 *		layout) and that the key is usable under its original KeyId.
 *
 * @return
 *		- XST_SUCCESS, if the vault is imported and the imported DEK still decrypts the ciphertext
 *		  to the original plaintext.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_ImportKeyVault(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_KeyVaultTransferParams ImportParams = {0};
	u32 Index;

	/* Nothing valid to import unless a prior export succeeded. */
	if ((KmExportSize < XASU_KM_EXAMPLE_EXPORT_MIN_SIZE) ||
	(KmExportSize > XASU_KM_EXAMPLE_EXPORT_BUF_SIZE)) {
		XilAsu_Printf("ImportKeyVault has no valid exported blob to import \r\n");
		goto END;
	}

	XAsu_KmDemo_InitClientParams(&ClientParams);

	/* On import, DataAddr is the source blob and BufSize its actual length. ExportResultAddr
	 * is unused, and Context has to be the one the export ran with or the blob does not
	 * decrypt. */
	ImportParams.DataAddr = (u64)(UINTPTR)KmExportBuffer;
	ImportParams.BufSize = KmExportSize;
	ImportParams.ExportResultAddr = 0U;

	Status = Xil_SMemCpy(ImportParams.Context, sizeof(ImportParams.Context), KmExportContext,
			sizeof(KmExportContext), sizeof(KmExportContext));
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ImportKeyVault could not set the context: %08x \r\n", Status);
		goto END;
	}

	/* Free the region the import restores into. The blob rebuilds this vault under the same
	 * VaultId, so the teardown still deletes it by KmVaultId afterwards. */
	Status = XAsu_KmDemo_DeleteVault(KmVaultId);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ImportKeyVault could not free the vault region: %08x \r\n", Status);
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	/* Flush the vault blob the server reads to import it. */
	Xil_DCacheFlushRange((UINTPTR)KmExportBuffer, KmExportSize);
#endif

	XAsu_KmDemo_ResetResponseState();

	Status = XAsu_KmImportKeyVault(&ClientParams, &ImportParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ImportKeyVault request failed: %08x \r\n", Status);
		goto END;
	}

	XAsu_KmDemo_WaitForResponse();
	if (ErrorStatus != (u32)XST_SUCCESS) {
		XilAsu_Printf("ImportKeyVault failed from server: %08x \r\n", ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n Imported vault from the %u-byte blob \r\n", KmExportSize);

	/**
	 * Content-integrity check: import succeeding proves the blob authenticated and the vault was
	 * rebuilt, but not that the key BYTES are correct. Exercise the actual content through a
	 * preserved KeyId - decrypt KmDekCiphertext (produced back in step 13 by the raw DEK) with the
	 * imported DEK (KmDekId) and confirm the recovered plaintext equals KmPlaintext. The AES-GCM
	 * tag check (against KmDekTag) plus a byte-for-byte match proves the imported DEK is identical
	 * to the one stored before export, i.e. import faithfully restored the key material, not just
	 * the vault layout.
	 */
	Status = XAsu_KmDemo_AesGcmOp(XASU_AES_DECRYPT_OPERATION, 0U, KmDekId,
					(u64)(UINTPTR)KmDekCiphertext,
					(u64)(UINTPTR)KmDecrypted_ImportVerif,
					XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES, (u64)(UINTPTR)KmDekTag);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Import content check: decrypt with imported DEK KeyId failed \r\n");
		goto END;
	}

	XAsu_KmDemo_PrintBuffer("Import-check decrypt", KmDecrypted_ImportVerif,
			XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES);

	for (Index = 0U; Index < XASU_KM_EXAMPLE_AES_DATA_LEN_IN_BYTES; Index++) {
		if (KmDecrypted_ImportVerif[Index] != KmPlaintext[Index]) {
			XilAsu_Printf("Import content mismatch at byte %u: 0x%02x vs 0x%02x \r\n",
					Index, KmDecrypted_ImportVerif[Index], KmPlaintext[Index]);
			Status = XST_FAILURE;
			goto END;
		}
	}

	xil_printf("\r\n Import content check OK: the vaulted DEK still decrypts to the original "
		"plaintext \r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Prints a byte buffer as hex over UART, for visual inspection of the AES round-trip.
 *
 * @param	Label	Short label printed before the bytes.
 * @param	Buf	Pointer to the buffer to print.
 * @param	Len	Number of bytes to print.
 *
 *************************************************************************************************/
static void XAsu_KmDemo_PrintBuffer(const char *Label, const u8 *Buf, u32 Len)
{
	u32 Index;

	xil_printf("\r\n %s : ", Label);
	for (Index = 0U; Index < Len; Index++) {
		xil_printf("%02x", Buf[Index]);
	}
	xil_printf(" \r\n");
}

/*************************************************************************************************/
/**
 * @brief	Deletes a single key from the vault by its composite KeyId.
 *
 * @param	KeyId	Composite KeyId of the key to delete.
 *
 * @return
 *		- XST_SUCCESS, if the key is deleted successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_DeleteKey(u32 KeyId)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;

	XAsu_KmDemo_InitClientParams(&ClientParams);
	XAsu_KmDemo_ResetResponseState();

	Status = XAsu_KmDeleteKey(&ClientParams, KeyId);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("DeleteKey request failed: %08x \r\n", Status);
	} else {
		XAsu_KmDemo_WaitForResponse();
		if (ErrorStatus != (u32)XST_SUCCESS) {
			XilAsu_Printf("DeleteKey failed from server: %08x \r\n", ErrorStatus);
			Status = XST_FAILURE;
		} else {
			xil_printf("\r\n Deleted key, KeyId = 0x%08x \r\n", KeyId);
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Deletes a key vault by its VaultId, releasing the DDR space it occupied and freeing
 *		any keys still resident in it.
 *
 * @param	VaultId	VaultId of the vault to delete.
 *
 * @return
 *		- XST_SUCCESS, if the vault is deleted successfully.
 *		- XST_FAILURE, otherwise.
 *
 *************************************************************************************************/
static s32 XAsu_KmDemo_DeleteVault(u32 VaultId)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;

	XAsu_KmDemo_InitClientParams(&ClientParams);
	XAsu_KmDemo_ResetResponseState();

	Status = XAsu_KmDeleteKeyVault(&ClientParams, VaultId);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("DeleteKeyVault request failed: %08x \r\n", Status);
	} else {
		XAsu_KmDemo_WaitForResponse();
		if (ErrorStatus != (u32)XST_SUCCESS) {
			XilAsu_Printf("DeleteKeyVault failed from server: %08x \r\n", ErrorStatus);
			Status = XST_FAILURE;
		} else {
			xil_printf("\r\n Deleted vault, VaultId = %u \r\n", VaultId);
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes the common client parameters used for the KeyManager
 *		requests in this example.
 *
 * @param	ClientParamPtr	Pointer to the client parameters structure to initialize.
 *
 *************************************************************************************************/
static void XAsu_KmDemo_InitClientParams(XAsu_ClientParams *ClientParamPtr)
{
	ClientParamPtr->Priority = XASU_PRIORITY_HIGH;
	ClientParamPtr->SecureFlag = XASU_CMD_SECURE;
	ClientParamPtr->CallBackFuncPtr =
		(XAsuClient_ResponseHandler)((void *)XAsu_KmDemo_CallBackRef);
	ClientParamPtr->CallBackRefPtr = (void *)ClientParamPtr;
}

/*************************************************************************************************/
/**
 * @brief	Clears the asynchronous completion state (the notify flag and the server status)
 *		before a request is submitted, so a stale value left by a previous request cannot be
 *		mistaken for this request's response.
 *
 *************************************************************************************************/
static void XAsu_KmDemo_ResetResponseState(void)
{
	ErrorStatus = XST_FAILURE;
	Notify = 0U;
}

/*************************************************************************************************/
/**
 * @brief	Blocks until the client library signals (via the callback) that the server has
 *		responded, then clears the notify flag for the next request. The server-side outcome is
 *		left in ErrorStatus for the caller to validate. Must be called only after a request has
 *		been submitted successfully, otherwise no callback will arrive.
 *
 *************************************************************************************************/
static void XAsu_KmDemo_WaitForResponse(void)
{
	while (!Notify);
	Notify = 0U;
}

/*************************************************************************************************/
/**
 * @brief	Call back function which will be registered with the library to notify the completion
 *		of the request.
 *
 * @param	CallBackRef	Pointer to the call back reference.
 * @param	Status		Status of the request will be passed as an argument during the callback
 *				- 0 upon success
 *				- Error code from the ASUFW application upon any error
 *
 *************************************************************************************************/
static void XAsu_KmDemo_CallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	ErrorStatus = Status;

	/* Update the variable to notify the callback */
	Notify = 1U;
}
/** @} */
