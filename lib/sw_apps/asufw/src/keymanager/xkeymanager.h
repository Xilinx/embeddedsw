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

#ifdef XASU_KEYMANAGER_ENABLE
/************************************ Constant Definitions ***************************************/
#define XKEYMANAGER_MAX_SUB_VAULTS	(9U) /**< Total number of supported sub-vaults */

#define XKEYMANAGER_MAJOR_VERSION	(1U) /**< Major version of the key manager */
#define XKEYMANAGER_MINOR_VERSION	(0U) /**< Minor version of the key manager */
#define XKEYMANAGER_IDENTIFICATION_STRING	(0x4C564B58U) /**< Key manager identification string */

#define XKEYMANAGER_X509_DIGITALSIGNATURE_USE_CASE	(0x01U)	/**< Digital signature use case */
#define XKEYMANAGER_X509_NONREPUDIATION_USE_CASE	(0x02U)	/**< Non-repudiation use case */
#define XKEYMANAGER_X509_KEYENCIPHERMENT_USE_CASE	(0x04U)	/**< Key encipherment use case */
#define XKEYMANAGER_X509_DATAENCIPHERMENT_USE_CASE	(0x08U)	/**< Data encipherment use case */
#define XKEYMANAGER_X509_KEYAGREEMENT_USE_CASE		(0x10U)	/**< Key agreement use case */
#define XKEYMANAGER_X509_KEYCERTSIGN_USE_CASE		(0x20U)	/**< Certificate signing use case */
#define XKEYMANAGER_X509_CRLSIGN_USE_CASE		(0x40U)	/**< CRL signing use case */
#define XKEYMANAGER_X509_ENCIPHERONLY_USE_CASE		(0x80U)	/**< Encipher only use case */
#define XKEYMANAGER_X509_DECIPHERONLY_USE_CASE		(0x100U) /**< Decipher only use case */

#define XKEYMANAGER_RSA_ALL_KEY_USE_CASES_VALUE	(7U)	/**< Value for all RSA key use cases*/

#define XKEYMANAGER_ASU_SUBSYSTEM_ID			(0x1C000002U)	/**< ASU subsystem ID */

#define XKEYMANAGER_ASU_VAULT_ID	(0U)	/**< Vault index for ASU internal vault */

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
	u8 Reserved1;	/**< Reserved byte. */
	u16 Reserved2;	/**< Reserved halfword. */
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
	u8 Modulus[XRSA_4096_KEY_SIZE];	/**< Modulus buffer. */
	u32 PubExp;			/**< Public exponent value. */
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
s32 XKeyManager_CreateKeyVault(const XAsu_KeyManagerSubVaultParams *ParamsPtr, u32 SubsystemId,
						u32 IpiMask, u32 *VaultIdPtr);
s32 XKeyManager_DeleteKeyVault(u32 SubsystemId, u32 VaultId);
s32 XKeyManager_GenerateKeyIv(XAsufw_Dma *DmaPtr,
			const XAsu_KeyManagerParams *ParamsPtr, u32 *KeyIdPtr, u32 SubSystemId,
			XAsu_KeyManagerSubVaultType KeyType);
u8* XKeyManager_GetKeyObjectPtr(u32 KeyId, u32 SubSystemId, u8 KeyUsecase);
s32 XKeyManager_GenerateRsaKeyPair(XAsufw_Dma *DmaPtr, const XAsu_KeyManagerParams *ParamsPtr,
			u32 *KeyIdPtr, u32 SubSystemId, XAsu_KeyManagerSubVaultType KeyType);
s32 XKeyManager_GenerateEccKeyPair(XAsufw_Dma *DmaPtr, const XAsu_KeyManagerParams *ParamsPtr,
			u32 *KeyIdPtr, u32 SubSystemId);
s32 XKeyManager_DeleteKey(u32 KeyId, u32 SubSystemId);
u16 XKeyManager_GetAsuRsaActiveKeyCount(XAsu_KeyManagerSubVaultType KeyType);
s32 XKeyManager_StoreRsaKeyPairInAsuVault(const u8 *KeyData, u32 KeyLen, u8 VaultId);
s32 XKeyManager_FetchRsaKeyPairFromAsuVault(XAsufw_Dma *DmaPtr, u8 *DestBuf, u8 VaultId, u64 KeyAddr);
s32 XKeyManager_StoreKeyInVault(XAsufw_Dma *DmaPtr,  XAes *AesInstancePtr, const XAsu_KeyManagerParams *KeyParams,
				u32 *KeyIdPtr, u32 SubSystemId);
s32 XKeyManager_UpdateKeyObjFromVault(XAsufw_Dma *DmaPtr, u32 KeyId, u64 KeyObjectAddr,
				      u32 SubSystemId, u8 KeyUsecase,
				      XKeyManager_RsaOpType RsaOpType);
s32 XKeyManager_ExportKeyVault(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u64 ExportAddr,
			       u32 ExportBufSize, u32 ActualVaultSizeAddr);
s32 XKeyManager_ImportKeyVault(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u64 ImportAddr,
			       u32 ImportSize);
#endif /* XASU_KEYMANAGER_ENABLE */

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XKEYMANAGER_H_ */
/** @} */
