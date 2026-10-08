/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xkeymanager.h
 *
 * This file contains the function prototypes, defines and macros for the key manager functionality.
 *
 * AEKV stands for Authenticated Encrypted Key Vault, the portable blob a key vault export
 * produces and a key vault import consumes.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   11/25/25 Initial release
 *       yog  01/28/26 Added key management support for RSA.
 *       ss   03/18/26 Added vault query, store, and fetch APIs for scheduler-based RSA key gen.
 *       kp   03/24/26 Moved KDF/HMAC key info structure to xasu_kdfinfo.h
 *       yog  04/23/26 Added support for LMS key management.
 *       vm   09/15/26 Added the vault side interfaces of key vault export and import, the ASU
 *                     internal exportable vault identifier, its key capacity and creation
 *                     interface, and the resident key vault export and import sequencers
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xkeymanager_server_apis Key Manager Server APIs
* @{
*/
#ifndef XKEYMANAGER_H_
#define XKEYMANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xaes.h"
#include "xasu_aesinfo.h"
#include "xasu_keymanagerinfo.h"
#include "xasu_kdfinfo.h"
#include "xasufw_dma.h"
#include "xasu_rsainfo.h"
#include "xasu_ocpinfo.h"
#include "xasu_eccinfo.h"
#include "xasu_lmsinfo.h"
#include "xasu_mldsainfo.h"
#include "xasu_mlkeminfo.h"
#include "xasu_slhdsainfo.h"
#include "xsha.h"
#include "xkeymanager_aekv.h"

#ifdef XASU_KEYMANAGER_ENABLE
/************************************ Constant Definitions ***************************************/
#define XKEYMANAGER_MAX_SUB_VAULTS	(13U) /**< Total number of supported sub-vaults */

#define XKEYMANAGER_MAJOR_VERSION	(1U) /**< Major version of the key manager */
#define XKEYMANAGER_MINOR_VERSION	(0U) /**< Minor version of the key manager */
#define XKEYMANAGER_IDENTIFICATION_STRING	(0x4C564B58U) /**< Key manager identification string */

#define XKEYMANAGER_RSA_ALL_KEY_USE_CASES_VALUE	(7U)	/**< Value for all RSA key use cases*/

#define XKEYMANAGER_ASU_SUBSYSTEM_ID			(0x1C000002U)	/**< ASU subsystem ID */

#define XKEYMANAGER_ASU_VAULT_ID	(0U)	/**< Vault index for ASU internal vault */
#define XKEYMANAGER_ASU_EXPORTABLE_VAULT_ID	(1U)	/**< Vault index for the ASU-owned
								exportable vault holding the ECIES
								provisioning key (created right after
								vault 0) */

#define XKEYMANAGER_ASU_RSA_KEY_VAULT_CAPACITY	(2U)	/**< Number of RSA key pairs that can be
								 stored in ASU vault */
#define XKEYMANAGER_ASU_EXPORTABLE_ECC_KEY_CAPACITY	(2U * XASU_MAX_CHANNELS_SUPPORTED)
						/**< Number of ECC key pairs the ASU internal
						     exportable vault holds. The boot creates
						     that vault and the import recreates it for
						     a blob which carries none, so both use this
						     capacity: a vault of another size would not
						     fit the snapshot the import keeps of it. */

/*
 * UDE key type encoding in the key vault main header. Each UDE slot occupies two bits, so the byte
 * holds up to XKEYMANAGER_UDE_MAX_SLOTS slots (UDE_0 in bits 1 and 0, up to UDE_2 in bits 5 and 4).
 * The top two bits (7 and 6) are unused.
 */
#define XKEYMANAGER_UDE_KEY_TYPE_BITS_PER_SLOT	(2U)	/**< UDE key type bits per slot */
#define XKEYMANAGER_UDE_KEY_TYPE_SLOT_MASK	(0x3U)	/**< 2-bit mask for a single UDE slot */
#define XKEYMANAGER_UDE_MAX_SLOTS		(3U)	/**< Maximum number of UDE slots in the byte */

#define XKEYMANAGER_KEY_INDEX_OFFSET		(1U) /**< Offset for key index */
#define XKEYMANAGER_KEY_PAIR_ID_COUNT	(2U) /**< Number of key IDs written by a key pair
							generation: public key ID followed by
							private key ID */
#define XKEYMANAGER_ASYM_KEY_TYPE_COUNT	(2U)	/**< Number of asymmetric key types
                                                        (private and public) */

/************************************** Type Definitions *****************************************/
/** This enum contains RSA operation types for private key retrieval. */
typedef enum {
	XKEYMANAGER_RSA_OP_NONE = 0U,		/**< No RSA operation */
	XKEYMANAGER_RSA_OP_NONCRT = 1U,		/**< RSA Non-CRT operation */
	XKEYMANAGER_RSA_OP_CRT = 2U		/**< RSA CRT operation */
} XKeyManager_RsaOpType;

/** This structure contains key vault header related information. */
typedef struct {
	u32 KeyVaultHeader;	/**< Vault header word. */
	u16 MinorVersion;	/**< Key vault minor version. */
	u16 MajorVersion;	/**< Key vault major version. */
	u8 RevokeId;	/**< Revocation identifier. */
	u8 UdeKeyType;	/**< UDE key type (two bits per UDE slot). */
	u16 Reserved;	/**< Reserved halfword. */
} XKeyManager_VaultMainHeader;

/** This structure contains key vault header related information. */
typedef struct {
	u32 KeyVaultId;	/**< Vault ID. */
	u16 AccessRights;	/**< Access permissions for the vault. */
	u8 Restrictions;	/**< Key vault restrictions. */
	u8 Reserved;	/**< Reserved byte. */
	u32 VaultSize;	/**< Vault size in bytes. */
	u32 SubSystemId;	/**< Subsystem ID */
} XKeyManager_VaultHeader;

/** This structure contains sub-vault header related information. */
typedef struct {
	u16 Capacity;	/**< Entries the sub-vault can hold. */
	u16 ActiveKeys;	/**< Number of active keys. */
	u16 DeactivatedKeys;	/**< Number of deactivated keys. */
	u16 Reserved;	/**< Reserved field. */
	u32 Offset;	/**< Offset to sub-vault in a key vault. */
} XKeyManager_SubVaultHeader;

/** This structure contains AES key object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	u8 Content[XASU_AES_KEY_SIZE_256BIT_IN_BYTES];	/**< Key data. */
} XKeyManager_AesKeyObject;

/** This structure contains IV object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< IV metadata. */
	u8 Content[XASU_AES_IV_SIZE_128BIT_IN_BYTES];	/**< IV data. */
} XKeyManager_IvObject;

/** This structure contains KDF/HMAC key object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	u8 Content[XASU_KM_KDF_HMAC_MAX_KEY_LENGTH];	/**< Key data. */
} XKeyManager_KdfHmacKeyObject;

/** This structure contains RSA private key object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	XAsu_RsaKeyPairObject RsaPvtKeyPair;	/**< RSA private key pair structure. */
} XKeyManager_RsaPvtKeyObject;

/** This structure contains RSA public key object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	u32 PubExp;			/**< Public exponent value. */
	u8 Modulus[XRSA_4096_KEY_SIZE];	/**< Modulus buffer. */
} XKeyManager_RsaPubKeyObject;

/** This structure contains ECC private key object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	u8 PrivateKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES];	/**< ECC private key. */
} XKeyManager_EccPvtKeyObject;

/** This structure contains ECC public key object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	u8 PublicKey[XASU_ECC_P521_PUB_KEY_SIZE_IN_BYTES];	/**< ECC public key. */
} XKeyManager_EccPubKeyObject;

/** This structure contains X.509 certificate object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	u8 Content[XASU_X509_MAX_SIZE_IN_BYTES];	/**< X.509 certificate data. */
	u32 RawKeyId;	/**< KeyId of raw public key within the	certificate. */
} XKeyManager_X509KeyObject;

/** This structure contains LMS public key object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	u8 PublicKey[XASU_LMS_MAX_PUB_KEY_SIZE];	/**< LMS public key data. */
} XKeyManager_LmsPubKeyObject;

/*
 * The PQC key objects are sized to the largest parameter set and are defined unconditionally,
 * independent of XASU_MLDSA_ENABLE / XASU_MLKEM_ENABLE. Gating them on the algorithm build
 * options would make the persisted vault layout differ between builds, which would break
 * export/import between two firmware images that only differ in enabled algorithms.
 */

/** This structure contains ML-DSA private key object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	u8 PrivateKey[XASU_MLDSA_MAX_PRIV_KEY_LEN];	/**< ML-DSA private key (ML-DSA-87 sized). */
} XKeyManager_MldsaPvtKeyObject;

/** This structure contains ML-DSA public key object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	u8 PublicKey[XASU_MLDSA_MAX_PUB_KEY_LEN];	/**< ML-DSA public key (ML-DSA-87 sized). */
} XKeyManager_MldsaPubKeyObject;

/** This structure contains ML-KEM decapsulation key object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	u8 PrivateKey[XASU_MLKEM_MAX_PRIV_KEY_LEN];	/**< ML-KEM decapsulation key
								(ML-KEM-1024 sized). */
} XKeyManager_MlkemPvtKeyObject;

/** This structure contains ML-KEM encapsulation key object information. */
typedef struct {
	XAsu_KeyManagerKeyMetadata Metadata;	/**< Key metadata. */
	u8 PublicKey[XASU_MLKEM_MAX_PUB_KEY_LEN];	/**< ML-KEM encapsulation key
								(ML-KEM-1024 sized). */
} XKeyManager_MlkemPubKeyObject;

/** This structure contains key manager vault and sub vault header information. */
typedef struct {
	XKeyManager_VaultHeader Header;	/**< Vault header. */
	XKeyManager_SubVaultHeader SubVaultHeaders[XKEYMANAGER_MAX_SUB_VAULTS];	/**< Sub-vault
										information. */
} XKeyManager;

/** This structure contains vault info related information. */
typedef struct {
	u32 SubSystemId;	/**< Subsystem identifier. */
	u32 VaultSize;	/**< Size of the key vault in bytes. */
	u32 *VaultBasePtr;	/**< Pointer to the key vault base address. */
} XKeyManager_VaultInfo;

/** This structure contains each vault info and complete vault size related information. */
typedef struct {
	XKeyManager_VaultInfo VaultInfo[XASU_KM_MAX_VAULTS];	/**< Array of vault
								information for all subsystems. */
	u32 RemainingVaultSize;	/**< Remaining vault space available. */
	u32 TotalVaultSize;	/**< Total vault space allocated. */
	u16 Reserved;	/**< Reserved field for alignment. */
} XKeyManager_VaultRegistry;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XKeyManager_CfgInitialize(void);
s32 XKeyManager_UpdateUdeKeyType(u8 UdeSlot, u8 UdeKeyType);
s32 XKeyManager_CreateKeyVault(const XAsu_KeyManagerSubVaultParams *ParamsPtr, u32 SubsystemId,
			u32 IpiMask, u32 *VaultIdPtr, u8 IsProvisioning);
s32 XKeyManager_DeleteKeyVault(u32 SubsystemId, u32 VaultId);
s32 XKeyManager_GenerateKeyIv(XAsufw_Dma *DmaPtr,
			const XAsu_KeyManagerParams *ParamsPtr, u32 *KeyIdPtr, u32 SubSystemId,
			XAsu_KeyManagerSubVaultType KeyType);
s32 XKeyManager_GetKeyObjectPtr(u32 KeyId, u32 SubSystemId, u8 KeyUsecase, u8 IsKeyUsed,
			const u8 **KeyObjectPtr);
#ifdef XASU_RSA_ECC_ENABLE
s32 XKeyManager_GenerateRsaKeyPair(XAsufw_Dma *DmaPtr, const XAsu_KeyManagerParams *ParamsPtr,
			u32 *KeyIdPtr, u32 SubSystemId);
s32 XKeyManager_GenerateEccKeyPair(XAsufw_Dma *DmaPtr, const XAsu_KeyManagerParams *ParamsPtr,
			u32 *KeyIdPtr, u32 SubSystemId);
#endif
#ifdef XASU_MLDSA_ENABLE
s32 XKeyManager_GenerateMldsaKeyPair(XAsufw_Dma *DmaPtr, XSha *Sha3Ptr,
			const XAsu_KeyManagerPqcKeyGenParams *PqcParamsPtr, u32 *KeyIdPtr,
			u32 SubSystemId);
#endif
#ifdef XASU_MLKEM_ENABLE
s32 XKeyManager_GenerateMlkemKeyPair(XAsufw_Dma *DmaPtr,
			const XAsu_KeyManagerPqcKeyGenParams *PqcParamsPtr, u32 *KeyIdPtr,
			u32 SubSystemId);
#endif
#ifdef XASU_SLHDSA_ENABLE
s32 XKeyManager_GenerateSlhdsaKeyPair(XAsufw_Dma *DmaPtr, XSha *ShaPtr,
			const XAsu_SlhdsaKeyGenParams *KeyGenParamsPtr);
#endif
s32 XKeyManager_DeleteKey(u32 KeyId, u32 SubSystemId);
u16 XKeyManager_GetAsuRsaActiveKeyCount(XAsu_KeyManagerSubVaultType KeyType);
s32 XKeyManager_StoreRsaKeyPairInAsuVault(const u8 *KeyData, u32 KeyLen, u8 VaultId);
s32 XKeyManager_StoreKeyInAsuVault(const XAsu_KeyManagerParams *ParamsPtr, u32 *KeyIdPtr);
s32 XKeyManager_FetchRsaKeyPairFromAsuVault(XAsufw_Dma *DmaPtr, u8 *DestBuf, u8 VaultId, u64 KeyAddr);
s32 XKeyManager_StoreKeyInVault(XAsufw_Dma *DmaPtr,  XAes *AesInstancePtr, const XAsu_KeyManagerParams *KeyParams,
				u32 *KeyIdPtr, u32 SubSystemId);
s32 XKeyManager_UpdateKeyObjFromVault(XAsufw_Dma *DmaPtr, u32 KeyId, u64 KeyObjectAddr,
				      u32 SubSystemId, u8 KeyUsecase,
				      XKeyManager_RsaOpType RsaOpType,
				      XAsu_KeyManagerSubVaultType ExpectedKeyType);
s32 XKeyManager_SetEpochTime(u32 NewEpochTime);
u8 XKeyManager_GetEccPubKeyUseCase(u32 KeyUsage);
s32 XKeyManager_CombineKeysXor(XAsufw_Dma *DmaPtr, u32 SubSystemId,
	const XAsu_KmXorKeyParams *ParamsPtr, u32 *KeyIdPtr);
u32 XKeyManager_IsVaultInRegistry(u32 VaultId);
s32 XKeyManager_CreateAsuExportableVault(void);

/**
 * Vault side of a blob export or import. A blob format reads and writes the vault only through
 * these, so it never reaches into the registry itself.
 */
s32 XKeyManager_GetExportablePlaintextSize(u32 *SizePtr);
s32 XKeyManager_HasExportableVault(void);
s32 XKeyManager_IsImportRegionEmpty(void);
s32 XKeyManager_ReadExportPlaintext(u32 Offset, u8 *Buf, u32 Len);
s32 XKeyManager_WriteImportPlaintext(u32 Offset, const u8 *Buf, u32 Len);
void XKeyManager_MarkVaultClean(void);
u8 XKeyManager_IsKeyVaultDirty(void);
s32 XKeyManager_RebuildVaultRegistryFromImport(void);
s32 XKeyManager_ValidateImportedKeyObjects(void);
s32 XKeyManager_PrepareImportRegion(void);
s32 XKeyManager_GetImportRestorableSize(u32 *SizePtr);
s32 XKeyManager_RevertImportRegion(void);
s32 XKeyManager_ValidateClientBuffer(u64 Addr, u32 Len);
u8 *XKeyManager_GetVaultBasePtr(void);

/**
 * Storage the key vault export and import sessions share, lent out as raw scratch to a caller
 * that runs while neither of them does. It is declared here rather than beside the blob format
 * so a borrower does not have to include the blob format header to reach it.
 */
u8 *XKeyManager_GetSharedScratch(u32 *SizePtr);

/**
 * One whole key vault export or import, the entry points the key manager handler calls. These
 * phase the overlay images the blob format and the signature engine are linked into, so they
 * stay resident and the format layer never switches images itself.
 */
s32 XKeyManager_ExportKeyVault(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, XSha *ShaInstancePtr,
			       const XKeyManager_AekvExportParams *ParamsPtr);
s32 XKeyManager_ImportKeyVault(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, XSha *ShaInstancePtr,
			       const XKeyManager_AekvImportParams *ParamsPtr);
#endif /* XASU_KEYMANAGER_ENABLE */

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XKEYMANAGER_H_ */
/** @} */
