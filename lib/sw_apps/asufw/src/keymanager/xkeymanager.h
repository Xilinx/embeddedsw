/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xasu_aesinfo.h"
#include "xasu_keymanagerinfo.h"
#include "xasufw_dma.h"

/************************************ Constant Definitions ***************************************/
#define XKEYMANAGER_MAX_SUB_VAULTS	(9U) /**< Total number of supported sub-vaults */

#define XKEYMANAGER_MAJOR_VERSION	(1U) /**< Major version of the key manager */
#define XKEYMANAGER_MINOR_VERSION	(0U) /**< Minor version of the key manager */
#define XKEYMANAGER_IDENTIFICATION_STRING	(0x4C564B58U) /**< Key manager identification string */

#define XKEYMANAGER_MAX_VAULTS		(8U) /**< Maximum number of key vaults supported */

#define XKEYMANAGER_AES_ENC_USE_CASE		(0U)	/**< Data encryption key use case */
#define XKEYMANAGER_AES_DEC_USE_CASE		(1U)	/**< Data decryption key use case */
#define XKEYMANAGER_AES_KEY_WRAP_USE_CASE	(2U)	/**< Data key wrap use case */
#define XKEYMANAGER_AES_KEY_UNWRAP_USE_CASE	(3U)	/**< Data key unwrap use case */
#define XKEYMANAGER_AES_AUTH_USE_CASE		(4U)	/**< Data key unwrap use case */

#define XKEYMANAGER_LENGTH_AND_KEY_CONVERSION_OFFSET	(16U) /**< Offset for length to key size conversion */
#define XKEYMANAGER_LENGTH_AND_KEY_CONVERSION_SHIFT	(3U)  /**< Shift for length to key size conversion */

/************************************** Type Definitions *****************************************/

/** This enum contains sub vault ID related information. */
typedef enum {
	XKEYMANAGER_AES_SUBVAULT_ID,		/**< AES key sub-vault ID. */
	XKEYMANAGER_IV_SUBVAULT_ID,		/**< IV sub-vault ID. */
	XKEYMANAGER_RSA_PVT_SUBVAULT_ID,	/**< RSA private key sub-vault ID. */
	XKEYMANAGER_RSA_PUB_SUBVAULT_ID,	/**< RSA public key sub-vault ID. */
	XKEYMANAGER_ECC_PVT_SUBVAULT_ID,	/**< ECC private key sub-vault ID. */
	XKEYMANAGER_ECC_PUB_SUBVAULT_ID,	/**< ECC public key sub-vault ID. */
	XKEYMANAGER_KDF_SUBVAULT_ID,		/**< KDF key sub-vault ID. */
	XKEYMANAGER_LMS_SUBVAULT_ID,		/**< LMS key sub-vault ID. */
	XKEYMANAGER_X509_SUBVAULT_ID,		/**< X509 certificate sub-vault ID. */
	XKEYMANAGER_INVALID_SUBVAULT_ID,	/**< Invalid sub-vault identifier. */
} XKeyManager_SubVaultType;

/** This structure contains key vault header related information. */
typedef struct {
	u32 KeyVaultHeader;	/**< Vault header word. */
	u16 MinorVersion;	/**< Key vault minor version. */
	u16 MajorVersion;	/**< Key vault major version. */
	u8 RevokeId;	/**< Revocation identifier. */
	u8 Reserved1;	/**< Reserved byte. */
	u16 Reserved2;	/**< Reserved halfword. */
} XKeyManager_VaultHeader;

/** This structure contains sub-vault header related information. */
typedef struct {
	u16 Capacity;	/**< Entries the sub-vault can hold. */
	u16 ActiveKeys;	/**< Number of active keys. */
	u16 Deactivatedkeys;	/**< Number of deactivated keys. */
	u16 Reserved;	/**< Reserved field. */
	u32 Offset;	/**< Offset to sub-vault in a key vault. */
} XKeyManager_SubVaultHeader;

/** This structure contains key metadata related information. */
typedef struct {
	u8 KeyType; /**< Type of key stored. */
	u8 VaultId;	/**< Identifier of the vault. */
	u16 KeyId;	/**< Key identifier within sub-vault. */
	u32 EpochTime;	/**< Time stamp expiry for the key. */
	u32 UsageCount;	/**< Number of times the key can be used. */
	u8 AccessRights;	/**< Access permissions for the key. */
	u8 KeyMode;	/**< Mode of the key stored. */
	u8 KeyUseCase;	/**< Usage scenario stored alongside the key. */
	u8 KeyAttributes;	/**< Additional attribute for the key. */
	u16 Length;	/**< Key length. */
	u16 Reserved1;	/**< Reserved half-word. */
	u32 Reserved2;	/**< Reserved word. */
} XKeyManager_KeyMetadata;

/** This structure contains AES key object information. */
typedef struct {
	XKeyManager_KeyMetadata Metadata;	/**< Key metadata. */
	u8 Content[XASU_AES_KEY_SIZE_256BIT_IN_BYTES];	/**< Key data. */
} XKeyManager_AesKeyObject;

/** This structure contains IV object information. */
typedef struct {
	XKeyManager_KeyMetadata Metadata;	/**< IV metadata. */
	u8 Content[XASU_AES_IV_SIZE_128BIT_IN_BYTES];	/**< IV data. */
} XKeyManager_IvObject;

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
	XKeyManager_VaultInfo VaultInfo[XKEYMANAGER_MAX_VAULTS];	/**< Array of vault
								information for all subsystems. */
	u32 RemainingVaultSize;	/**< Remaining vault space available. */
	u32 TotalVaultSize;	/**< Total vault space allocated. */
} XKeyManager_VaultRegistry;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XKeyManager_CfgInitialize(void);
s32 XKeyManager_CreateKeyVault(const XAsu_KeyManagerSubVaultParams *ParamsPtr, u32 SubsystemId);
s32 XKeyManager_DeleteKeyVault(const XAsu_KeyManagerSubVaultParams *ParamsPtr, u32 SubsystemId);
s32 XKeyManager_GenerateKeyIv(XAsufw_Dma *DmaPtr,
			const XAsu_KeyManagerParams *ParamsPtr, u32 *KeyIdPtr, u32 SubSystemId,
			XKeyManager_SubVaultType KeyType);
u8* XKeyManager_GetKeyObjectPtr(u32 KeyId, u32 SubSystemId, u8 KeyUsecase);
s32 XKeyManager_GetVaultId(u32 SubSystemId, u8 *VaultIdPtr);
s32 XKeyManager_UpdateAesKeyObjectFromVault(XAsu_AesKeyObject *KeyObject, u32 SubSystemId,
			u8 KeyUsecase);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XKEYMANAGER_H_ */
/** @} */
