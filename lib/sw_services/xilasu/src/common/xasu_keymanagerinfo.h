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
 *       kp   08/22/26 Added composite key ID and user-requested vault ID definitions.
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
#define XASU_KM_SET_EPOCH_TIME_CMD_ID		(11U) /**< Command ID for setting epoch time */
#define XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID	(12U) /**< Command ID for ML-DSA key pair
							generation */
#define XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID	(13U) /**< Command ID for ML-KEM key pair
							generation */
#define XASU_KM_XOR_KEY_CMD_ID			(14U) /**< Command ID for combining two AES key
							splits using XOR */
#define XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID	(15U) /**< Command ID for SLH-DSA key pair
							generation */
#define XASU_KM_MAX_CMDS			(16U) /**< Maximum number of commands
							supported by KeyManager module */

#define XASU_KM_OUTPUT_ID_SIZE_IN_BYTES		(4U) /**< Key vault output id size */
#define XASU_KM_EXPORT_SIZE_IN_BYTES		(4U) /**< Size in bytes of the field storing actual
							exported keyvault data size */

#define XASU_KM_USAGE_COUNT_NON_DEPLETING_VALUE	(0xFFFFFFFFU) /**< Marker for keys that
									never expire */

#define XASU_KM_KEYTYPE_WRAPPED_BIT_MASK	(0x80U) /**< Bit mask for wrapped key type */

#define XASU_KM_MAX_VAULTS			(64U) /**< Maximum number of key vaults supported */

/*
 * Composite key identifier layout, a single u32 addressing one key:
 *	- Bits [31:24]: Vault ID.
 *	- Bits [23:16]: Key type (XAsu_KeyManagerSubVaultType, optionally OR'ed with
 *			XASU_KM_KEYTYPE_WRAPPED_BIT_MASK on the Store Key API).
 *	- Bits [15:0] : 1-based slot in the sub-vault. 0 requests auto-assign.
 */
#define XASU_KM_VAULT_ID_SHIFT		(24U) /**< Vault ID bit position */
#define XASU_KM_KEY_TYPE_SHIFT		(16U) /**< Key type bit position */
#define XASU_KM_VAULT_ID_MASK		(0xFF000000U) /**< Vault ID mask */
#define XASU_KM_KEY_TYPE_MASK		(0x00FF0000U) /**< Key type mask */
#define XASU_KM_KEY_INDEX_MASK		(0x0000FFFFU) /**< Key index mask */

#define XASU_KM_KDF_HMAC_MAX_KEY_LENGTH		(136U) /**< Max key length for KDF/HMAC in
							    key vault. */

/*
 * Key usecases bitmask related macros.
 * To specify multiple usecases from client, use bitwise OR operation with desired usecases.
 */
/* Key usecases for AES key type. */
#define XASU_KEYMANAGER_AES_ENC_USE_CASE			(0x01U)	/**< Data encryption key use case */
#define XASU_KEYMANAGER_AES_DEC_USE_CASE			(0x02U)	/**< Data decryption key use case */
#define XASU_KEYMANAGER_AES_KEY_WRAP_USE_CASE		(0x04U)	/**< Data key wrap use case */
#define XASU_KEYMANAGER_AES_KEY_UNWRAP_USE_CASE		(0x08U)	/**< Data key unwrap use case */
#define XASU_KEYMANAGER_AES_AUTH_USE_CASE			(0x10U)	/**< Data authentication use case */
#define XASU_KEYMANAGER_AES_KEY_SPLIT_USE_CASE		(0x20U)	/**< AES key may be used only as an
								input to XOR key combine. Exclusive:
								cannot be combined with any other
								AES use case. */

/* Key usecases for RSA private key type. */
#define XASU_KEYMANAGER_RSA_PVT_SIGN_GEN_USE_CASE		(0x01U)	/**< RSA private key sign
								use case */
#define XASU_KEYMANAGER_RSA_PVT_DECRYPT_USE_CASE		(0x02U)	/**< RSA private key decryption
								use case */
#define XASU_KEYMANAGER_RSA_PVT_KEY_TRANSPORT_USE_CASE	(0x04U)	/**< RSA private key key transport
								use case */

/* Key usecases for RSA public key type. */
#define XASU_KEYMANAGER_RSA_PUB_SIGN_VER_USE_CASE		(0x01U)	/**< RSA public key sign
								verification use case */
#define XASU_KEYMANAGER_RSA_PUB_ENCRYPT_USE_CASE		(0x02U)	/**< RSA public key encryption
								use case */
#define XASU_KEYMANAGER_RSA_PUB_KEY_TRANSPORT_USE_CASE	(0x04U)	/**< RSA public key key transport
								use case */

/* Key usecases for ECC public key type. */
#define XASU_KEYMANAGER_ECC_PUB_SIGN_VER_USE_CASE		(0x01U)	/**< ECC public key sign verification use case */
#define XASU_KEYMANAGER_ECC_PUB_KEY_AGREEMENT_USE_CASE	(0x02U)	/**< ECC public key key agreement use case */
#define XASU_KEYMANAGER_ECC_PUB_KEY_AUTH_ENC_USE_CASE	(0x04U)	/**< ECC public key key authentication and encryption use case */

/* Key usecases for ECC private key type. */
#define XASU_KEYMANAGER_ECC_PVT_SIGN_GEN_USE_CASE		(0x01U)	/**< ECC private key sign use case */
#define XASU_KEYMANAGER_ECC_PVT_KEY_AGREEMENT_USE_CASE	(0x02U)	/**< ECC private key key agreement use case */
#define XASU_KEYMANAGER_ECC_PVT_KEY_AUTH_ENC_USE_CASE	(0x04U)	/**< ECC private key key authentication and encryption use case */
#define XASU_KEYMANAGER_ECC_PVT_PUB_KEY_GEN_USE_CASE	(0x08U)	/**< ECC private key for public key generation */

/* Key usecases for KDF/HMAC key type. */
#define XASU_KEYMANAGER_KDF_HMAC_KDF_USE_CASE	(0x01U)	/**< Key derivation key use case */
#define XASU_KEYMANAGER_KDF_HMAC_HMAC_USE_CASE	(0x02U)	/**< HMAC secret key use case */

/* Key usecases for LMS public key type. */
#define XASU_KEYMANAGER_LMS_PUB_SIGN_VER_USE_CASE	(0x01U)	/**< LMS public key signature verification use case */

/* Key usecases for ML-DSA private key type. */
#define XASU_KEYMANAGER_MLDSA_PVT_SIGN_GEN_USE_CASE	(0x01U)	/**< ML-DSA private key sign
								use case */

/* Key usecases for ML-DSA public key type. */
#define XASU_KEYMANAGER_MLDSA_PUB_SIGN_VER_USE_CASE	(0x01U)	/**< ML-DSA public key signature
								verification use case */

/* Key usecases for ML-KEM decapsulation (private) key type. */
#define XASU_KEYMANAGER_MLKEM_PVT_DECAPS_USE_CASE	(0x01U)	/**< ML-KEM private key
								decapsulation use case */

/* Key usecases for ML-KEM encapsulation (public) key type. */
#define XASU_KEYMANAGER_MLKEM_PUB_ENCAPS_USE_CASE	(0x01U)	/**< ML-KEM public key
								encapsulation use case */

/* Key attribute for RSA private decrypt operation. */
#define XASU_KM_RSA_PVT_ATTR_MAX	(2U) /**< Maximum allowed value for RSA private key attribute */

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
	XASU_MLDSA_PVT_SUBVAULT_ID,	/**< ML-DSA private key sub-vault ID. */
	XASU_MLDSA_PUB_SUBVAULT_ID,	/**< ML-DSA public key sub-vault ID. */
	XASU_MLKEM_PVT_SUBVAULT_ID,	/**< ML-KEM decapsulation key sub-vault ID. */
	XASU_MLKEM_PUB_SUBVAULT_ID,	/**< ML-KEM encapsulation key sub-vault ID. */
	XASU_INVALID_SUBVAULT_ID,	/**< Invalid sub-vault identifier. */
} XAsu_KeyManagerSubVaultType;

/** This structure contains key meta data information. */
typedef struct {
	/*
	 * Client input on all Store and Generate Key APIs, so callers must zero-initialise this
	 * structure: stale bits are read as a request.
	 */
	u32 CompositeKeyId;	/**< Composite key identifier, built with
				XASU_KM_COMPOSE_KEY_ID():
				- Bits [31:24]: vault ID. Always required.
				- Bits [23:16]: key type, an XAsu_KeyManagerSubVaultType value
				  optionally OR'ed with XASU_KM_KEYTYPE_WRAPPED_BIT_MASK. Required
				  by the Store Key API only; the Generate Key APIs derive it.
				- Bits [15:0]: 1-based key slot within the sub-vault. 0 requests
				  an auto-assigned slot; other values request a specific slot,
				  which may be rejected if already in use. Key pair APIs apply
				  the requested slot to both the private and public
				  sub-vaults. */
	u8 KeyUseCase;	/**< Usage scenario stored alongside the key. */
	u8 KeyAttributes;	/**< Additional attribute for the key (applicable for ECC keys
				and RSA private keys to represent prime/totient presence information).
				For ML-DSA and ML-KEM key pair generation this field carries the
				parameter set, as XAsu_MldsaParamSet or XAsu_MlkemParamSet. */
	u16 Length;	/**< For ECC module: this parameter refers to Curve Length.
				For ML-DSA and ML-KEM key pair generation: the public or
				encapsulation key length of the parameter set named by
				KeyAttributes, which the request is cross-checked against.
				For other modules: this parameter refers to Key length. */
	u32 EpochTime;	/**< Time stamp expiry for the key. */
	u32 UsageCount;	/**< Number of times the key can be used. */
} XAsu_KeyManagerKeyMetadata;

/** This structure contains info for key manager parameters. */
typedef struct {
	XAsu_KeyManagerKeyMetadata KeyMetadata; /**< Key metadata. */
	XAsu_AesKeyObject AesKeyObj; /**< AES key object to be filled if input is wrapped key type.
						This field is applicable only for Store Key API. */
	u64 KeyObjectAddr; /**< Address of the key buffer.
						For Store Key API: This field is mandatory and it is an input buffer.
						For Generate Key APIs: This field is optional and it is output buffer
						if the key needs to be returned to user. */
	u64 KeyIdAddr; /**< Address where generated key ID is stored.
						For Store Key API: This field is mandatory.
						For Generate Key APIs: This field is optional and should be given only
						when the key to be stored in key vault. */
	u32 WrappedInputLen; /**< Length of the wrapped key input data (applicable only for Store Key API).
						This needs to be filled if input is wrapped key type. */
	u8 Reserved[4]; /**< Explicit padding to ensure consistent struct size across architectures */
} XAsu_KeyManagerParams;

/**
 * This structure contains info for the ML-DSA and ML-KEM key pair generation parameters.
 * Those two APIs are the only ones that can derive a key pair from a caller supplied seed, so the
 * seed travels in this structure rather than in XAsu_KeyManagerParams, which every other key
 * manager API shares and whose command length would otherwise change for all of them.
 */
typedef struct {
	XAsu_KeyManagerParams KmParams; /**< Key manager parameters common to every key type. */
	u64 SeedAddr; /**< Address of the seed the key pair is derived from.
						Zero draws the seed from the TRNG, which is what a caller
						other than an algorithm validation harness does. */
	u32 SeedLen; /**< Length of the seed in bytes: zero when SeedAddr is zero, otherwise
						XASU_MLDSA_KEYGEN_SEED_LEN for the ML-DSA API (xi) and
						XASU_MLKEM_KEYGEN_SEED_LEN for the ML-KEM API (d || z). */
	u8 Reserved[4]; /**< Explicit padding to ensure consistent struct size across architectures */
} XAsu_KeyManagerPqcKeyGenParams;

/**
 * This structure contains info for the SLH-DSA key pair generation parameters, sent over IPI with
 * XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID. The FIPS 205 seeds are always drawn from the TRNG, so a
 * caller cannot supply one and no seed travels here. The key vault does not hold SLH-DSA keys yet,
 * so the pair is returned to the caller buffers named below.
 */
typedef struct {
	u64 SkOutAddr;	/**< DMA address for secret key output (size per ParamSet) */
	u64 PkOutAddr;	/**< DMA address for public key output (size per ParamSet) */
	u8 ParamSet;	/**< Parameter set selector (XASU_SLHDSA_PARAM_*) */
	u8 Reserved[7];	/**< Reserved to pad the structure to 24 bytes */
} XAsu_SlhdsaKeyGenParams;

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
	u16 MldsaPvtKeyVaultCapacity; /**< Count for ML-DSA private keys. */
	u16 MldsaPubKeyVaultCapacity; /**< Count for ML-DSA public keys. */
	u16 MlkemPvtKeyVaultCapacity; /**< Count for ML-KEM decapsulation keys. */
	u16 MlkemPubKeyVaultCapacity; /**< Count for ML-KEM encapsulation keys. */
	u16 AccessRights; /**< Access permissions for the key vault. */
	u8 Restrictions;	/**< Key vault restrictions. */
	u8 VaultId;	/**< Requested vault ID. 0: auto-assign, non-zero: use that ID. */
	u8 Reserved[2]; /**< Explicit padding to ensure consistent struct size across architectures */
} XAsu_KeyManagerSubVaultParams;

/** This structure contains the RSA key pair object with modulus, exponents, and CRT parameters. */
typedef struct {
	u32 PubExp;			/**< Public exponent value. */
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

/** This structure contains info for combining two AES key splits using XOR. */
typedef struct {
	XAsu_KeyManagerKeyMetadata OutKeyMetadata; /**< Metadata of the combined key. Its
							CompositeKeyId gives the destination vault
							and the AES sub-vault, and the slot within
							it, or 0 to auto-assign the first free
							slot. Length shall match the length of
							the input key splits. */
	u64 OutKeyIdAddr; /**< Address where the composite key ID of the combined key is stored.
				Required when the destination slot in CompositeKeyId is 0, which
				requests an auto-assigned slot. Shall be 0 when a slot is
				selected, as the composite key ID is then already known. */
	u32 SplitKeyId1; /**< Key ID of the first input AES key split. */
	u32 SplitKeyId2; /**< Key ID of the second input AES key split. */
} XAsu_KmXorKeyParams;

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_KM_COMPOSE_KEY_ID(VaultId, KeyType, KeyIndex) \
	(((((u32)(VaultId)) << XASU_KM_VAULT_ID_SHIFT) & XASU_KM_VAULT_ID_MASK) | \
	 ((((u32)(KeyType)) << XASU_KM_KEY_TYPE_SHIFT) & XASU_KM_KEY_TYPE_MASK) | \
	 (((u32)(KeyIndex)) & XASU_KM_KEY_INDEX_MASK))	/**< Fuse vault ID, key type and key
					index into a composite key ID. Pass key type 0 on the
					Generate Key APIs, which derive it themselves. */
#define XASU_KM_GET_VAULT_ID(KeyId) \
	((u8)((((u32)(KeyId)) & XASU_KM_VAULT_ID_MASK) >> XASU_KM_VAULT_ID_SHIFT))
					/**< Extract vault ID from a composite key ID */
#define XASU_KM_GET_KEY_TYPE(KeyId) \
	((u8)((((u32)(KeyId)) & XASU_KM_KEY_TYPE_MASK) >> XASU_KM_KEY_TYPE_SHIFT))
					/**< Extract key type, wrapped-key bit included */
#define XASU_KM_GET_KEY_INDEX(KeyId)	((u16)(((u32)(KeyId)) & XASU_KM_KEY_INDEX_MASK))
					/**< Extract key index, 0 if auto-assign requested */

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYMANAGERINFO_H_ */
