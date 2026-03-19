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
#define XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID		(5U) /**< Command ID for RSA key pair generation */
#define XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID		(6U) /**< Command ID for ECC key pair generation */

#define XASU_KM_OUTPUT_ID_SIZE_IN_BYTES		(4U) /**< Key vault output id size */

#define XASU_KM_USAGE_COUNT_NON_DEPLETING_VALUE	(0xFFFFFFFFU) /**< Marker for keys that
									never expire */

#define XASU_KM_MAX_VAULTS			(64U) /**< Maximum number of key vaults supported */
/** @} */
/************************************** Type Definitions *****************************************/
/** This structure contains info for meta data of key. */
typedef struct {
	u16 Length;	/**< Key length. */
	u8 KeyAttributes;	/**< Additional attribute for the key. */
	u8 KeyUseCase;	/**< Usage scenario stored alongside the key. */
	u32 EpochTime;	/**< Time stamp expiry for the key. */
	u32 UsageCount;	/**< Number of times the key can be used. */
	u32 Reserved;	/**< Reserved field. */
} XAsu_KeyManagerKeyMetadata;

/** This structure contains info for key manager parameters. */
typedef struct {
	XAsu_KeyManagerKeyMetadata KeyMetadata; /**< Key metadata. */
	u64 KeyObjectAddr; /**< Address of the key buffer. */
	u64 KeyIdAddr; /**< Address where generated key ID is stored. */
	u32 VaultId; /**< Vault ID for key storage. */
} XAsu_KeyManagerParams;

/** This structure contains info for different sub vault capacities. */
typedef struct {
	u64 VaultIdAddr; /**< Address where generated vault ID is stored. */
	u16 AESKeyVaultCapacity; /**< Count for the AES key vault. */
	u16 IVVaultCapacity; /**< Count for the IV vault. */
	u16 RSAPvtKeyVaultCapacity; /**< Count for RSA private keys. */
	u16 RSAPubKeyVaultCapacity; /**< Count for RSA public keys. */
	u16 ECCPvtKeyVaultCapacity; /**< Count for ECC private keys. */
	u16 ECCPubKeyVaultCapacity; /**< Count for ECC public keys. */
	u16 KDFKeyVaultCapacity; /**< Count for KDF material. */
	u16 LMSKeyVaultCapacity; /**< Count for LMS keys. */
	u16 X509KeyVaultCapacity; /**< Count for X.509 certificates. */
	u16 AccessRights; /**< Access permissions for the key vault. */
	u8 Restrictions;	/**< Key vault restrictions. */
	u8 Reserved1; /**< Reserved byte. */
	u16 Reserved2; /**< Reserved halfword. */
} XAsu_KeyManagerSubVaultParams;

typedef struct {
	u32 Modulus[XRSA_MAX_KEY_SIZE_IN_WORDS]; /**< Pointer to modulus buffer. */
	u32 PvtExp[XRSA_MAX_KEY_SIZE_IN_WORDS]; /**< Pointer to private exponent buffer. */
	u32 Prime1[XRSA_MAX_PRIME_SIZE_IN_WORDS]; /**< Pointer to prime1 buffer. */
	u32 Prime2[XRSA_MAX_PRIME_SIZE_IN_WORDS]; /**< Pointer to prime2 buffer. */
	u32 DP[XRSA_MAX_PRIME_SIZE_IN_WORDS]; /**< Pointer to DP buffer. */
	u32 DQ[XRSA_MAX_PRIME_SIZE_IN_WORDS]; /**< Pointer to DQ buffer. */
	u32 QInv[XRSA_MAX_PRIME_SIZE_IN_WORDS]; /**< Pointer to QInv buffer. */
} XAsu_RsaKeyPairObject;

typedef struct {
	u8 PublicKey[XASU_ECC_P521_PUB_KEY_SIZE_IN_BYTES]; /**< Public key buffer. */
	u8 PrivateKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES]; /**< Private key buffer. */
} XAsu_EccKeyPairObject;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYMANAGERINFO_H_ */
