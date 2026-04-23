/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keymanagerinfo.h
 *
 * This file contains the Keymanager definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   11/25/25 Initial release
 *       yog  01/28/26 Added RSA key pair generation command and structure definitions.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_KEYMANAGERINFO_H_
#define XASU_KEYMANAGERINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xil_util.h"
#include "xasu_rsainfo.h"
#include "xasu_eccinfo.h"
#include "xasu_aesinfo.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
/* KeyManager module command IDs */
#define XASU_KM_CREATE_KEYVAULT_CMD_ID		(0U) /**< Command ID for key vault creation */
#define XASU_KM_DELETE_KEYVAULT_CMD_ID		(1U) /**< Command ID for key vault deletion */
#define XASU_KM_DELETE_KEY_CMD_ID		(2U) /**< Command ID for key deletion */
#define XASU_KM_GEN_AES_KEY_CMD_ID		(3U) /**< Command ID for AES key generation */
#define XASU_KM_GEN_AES_IV_CMD_ID		(4U) /**< Command ID for AES IV generation */
#define XASU_KM_GEN_RAW_KEY_CMD_ID		(5U) /**< Command ID for raw key generation */
#define XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID		(6U) /**< Command ID for RSA key pair generation */
#define XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID		(7U) /**< Command ID for ECC key pair generation */
#define XASU_KM_STORE_KEY_CMD_ID		(8U) /**< Command ID for storing key in vault */
#define XASU_KM_EXPORT_KEYVAULT_CMD_ID		(9U) /**< Command ID for exporting key vault */
#define XASU_KM_IMPORT_KEYVAULT_CMD_ID		(10U) /**< Command ID for importing key vault */
#define XASU_KM_MAX_CMDS			(11U) /**< Maximum number of commands
							supported by KeyManager module */

#define XASU_KM_OUTPUT_ID_SIZE_IN_BYTES		(4U) /**< Key vault output id size */
#define XASU_KM_EXPORT_SIZE_IN_BYTES		(4U) /**< Size in bytes of the field storing actual
							exported keyvault data size */

#define XASU_KM_USAGE_COUNT_NON_DEPLETING_VALUE	(0xFFFFFFFFU) /**< Marker for keys that
									never expire */

#define XASU_KM_KEYTYPE_WRAPPED_BIT_MASK	(0x80U) /**< Bit mask for wrapped key type */

#define XASU_KM_MAX_VAULTS			(64U) /**< Maximum number of key vaults supported */

#define XASU_KM_KDF_HMAC_MAX_KEY_LENGTH		(136U) /**< Max key length for KDF/HMAC in
							    key vault. */
/** @} */
/************************************** Type Definitions *****************************************/
/** This enum contains sub vault ID related information. */
typedef enum {
	XASU_AES_SUBVAULT_ID,		/**< AES key sub-vault ID. */
	XASU_IV_SUBVAULT_ID,		/**< IV sub-vault ID. */
	XASU_KDF_HMAC_SUBVAULT_ID,	/**< KDF/HMAC key sub-vault ID. */
	XASU_RSA_PVT_SUBVAULT_ID,	/**< RSA private key sub-vault ID. */
	XASU_RSA_PUB_SUBVAULT_ID,	/**< RSA public key sub-vault ID. */
	XASU_ECC_PVT_SUBVAULT_ID,	/**< ECC private key sub-vault ID. */
	XASU_ECC_PUB_SUBVAULT_ID,	/**< ECC public key sub-vault ID. */
	XASU_LMS_SUBVAULT_ID,		/**< LMS key sub-vault ID. */
	XASU_X509_SUBVAULT_ID,		/**< X509 certificate sub-vault ID. */
	XASU_INVALID_SUBVAULT_ID,	/**< Invalid sub-vault identifier. */
} XAsu_KeyManagerSubVaultType;

/** This structure contains key meta data information. */
typedef struct {
	u16 KeyId;	/**< Key identifier within sub-vault. */
	u8 KeyType;	/**< Type of key stored. */
	u8 VaultId;	/**< Identifier of the vault. */
	u8 KeyUseCase;	/**< Usage scenario stored alongside the key. */
	u8 KeyAttributes;	/**< Additional attribute for the key. */
	u16 Length;	/**< Key length. */
	u32 EpochTime;	/**< Time stamp expiry for the key. */
	u32 UsageCount;	/**< Number of times the key can be used. */
} XAsu_KeyManagerKeyMetadata;

/** This structure contains info for key manager parameters. */
typedef struct {
	XAsu_KeyManagerKeyMetadata KeyMetadata; /**< Key metadata. */
	XAsu_AesKeyObject AesKeyObj; /**< AES key object to be filled if input is wrapped key type. */
	u64 KeyObjectAddr; /**< Address of the key buffer. */
	u64 KeyIdAddr; /**< Address where generated key ID is stored. */
	u32 WrappedInputLen; /**< Length of the wrapped key input data, to be filled if input is wrapped key type. */
	u8 Reserved[4]; /**< Explicit padding to ensure consistent struct size across architectures */
} XAsu_KeyManagerParams;

/** This structure contains info for different sub vault capacities. */
typedef struct {
	u64 VaultIdAddr; /**< Address where generated vault ID is stored. */
	u16 AESKeyVaultCapacity; /**< Count for the AES key vault. */
	u16 IVVaultCapacity; /**< Count for the IV vault. */
	u16 KDFHmacKeyVaultCapacity; /**< Count for KDF/HMAC key material. */
	u16 RSAPvtKeyVaultCapacity; /**< Count for RSA private keys. */
	u16 RSAPubKeyVaultCapacity; /**< Count for RSA public keys. */
	u16 ECCPvtKeyVaultCapacity; /**< Count for ECC private keys. */
	u16 ECCPubKeyVaultCapacity; /**< Count for ECC public keys. */
	u16 LMSKeyVaultCapacity; /**< Count for LMS keys. */
	u16 X509KeyVaultCapacity; /**< Count for X.509 certificates. */
	u16 AccessRights; /**< Access permissions for the key vault. */
	u8 Restrictions;	/**< Key vault restrictions. */
	u8 Reserved[3]; /**< Explicit padding to ensure consistent struct size across architectures */
} XAsu_KeyManagerSubVaultParams;

/** This structure contains the RSA key pair object with modulus, exponents, and CRT parameters. */
typedef struct {
	u32 Modulus[XRSA_MAX_KEY_SIZE_IN_WORDS]; /**< Modulus buffer. */
	u32 PvtExp[XRSA_MAX_KEY_SIZE_IN_WORDS]; /**< Private exponent buffer. */
	u32 Prime1[XRSA_MAX_PRIME_SIZE_IN_WORDS]; /**< Prime1 buffer. */
	u32 Prime2[XRSA_MAX_PRIME_SIZE_IN_WORDS]; /**< Prime2 buffer. */
	u32 DP[XRSA_MAX_PRIME_SIZE_IN_WORDS]; /**< DP buffer. */
	u32 DQ[XRSA_MAX_PRIME_SIZE_IN_WORDS]; /**< DQ buffer. */
	u32 QInv[XRSA_MAX_PRIME_SIZE_IN_WORDS]; /**< QInv buffer. */
} XAsu_RsaKeyPairObject;

/** This structure contains the ECC key pair object with public and private keys. */
typedef struct {
	u8 PublicKey[XASU_ECC_P521_PUB_KEY_SIZE_IN_BYTES]; /**< Public key buffer. */
	u8 PrivateKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES]; /**< Private key buffer. */
} XAsu_EccKeyPairObject;

/** This structure contains info for exporting and importing key vault. */
typedef struct {
	u64 DataAddr;		/**< Address for vault data (export destination or import source). */
	u64 ActualSizeAddr;	/**< Address where actual vault size will be stored (used only for export). */
	u32 BufSize;		/**< Buffer size for export or actual data size for import. */
	u8 Reserved[4]; /**< Explicit padding to ensure consistent struct size across architectures */
} XAsu_KeyVaultTransferParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYMANAGERINFO_H_ */
