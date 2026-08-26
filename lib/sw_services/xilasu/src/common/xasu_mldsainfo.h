/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_mldsainfo.h
 *
 * This file contains the ML-DSA (FIPS 204) definitions which are common across the
 * client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_MLDSAINFO_H_
#define XASU_MLDSAINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
/*
 * ML-DSA module command IDs. Key pair generation is not one of them: it is reached only through
 * the key manager's XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID, so that a generated private key can be
 * placed in the key vault rather than handed back to the caller.
 */
#define XASU_MLDSA_SIGN_CMD_ID		(0U) /**< Command ID for ML-DSA signature generation */
#define XASU_MLDSA_VERIFY_CMD_ID	(1U) /**< Command ID for ML-DSA signature verification */
#define XASU_MLDSA_KAT_CMD_ID		(2U) /**< Command ID for ML-DSA Known Answer Test */
/*
 * Signature generation from a caller-supplied rnd, for algorithm validation only. Hedged signing
 * draws rnd from an approved RBG, so reproducing the ACVP hedged signature vectors needs an rnd
 * that comes in with the request. ASUFW therefore serves this command only when built with
 * XASU_ENABLE_CAVP_SUPPORT; a production build rejects it with
 * XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED. Deterministic signing (rnd = 0) needs no such
 * command, being an approved mode reachable through XASU_MLDSA_SIGN_CMD_ID. The ID is defined
 * unconditionally so that both builds agree on XASU_MLDSA_MAX_CMDS and the module table layout.
 */
#define XASU_MLDSA_SIGN_DERAND_CMD_ID	(3U) /**< Command ID for signing with a supplied rnd */
#define XASU_MLDSA_MAX_CMDS		(4U) /**< Maximum commands supported by ML-DSA module */

/* Maximum ML-DSA context string length in bytes (FIPS 204) */
#define XASU_MLDSA_MAX_CTX_LEN		(255U) /**< Maximum context string length */

/* ML-DSA signing variants (FIPS 204) */
#define XASU_MLDSA_SIGN_MODE_PURE	(0U) /**< Pure ML-DSA (message signed directly) */
#define XASU_MLDSA_SIGN_MODE_PREHASH	(1U) /**< HashML-DSA (pre-hashed message) */

/* ML-DSA signing randomness selection (FIPS 204) */
#define XASU_MLDSA_HEDGED		(0U) /**< Hedged/randomized signing (default) */
#define XASU_MLDSA_DETERMINISTIC	(1U) /**< Deterministic signing (rnd = 0) */

/* KeyGen seed and signing randomness lengths (FIPS 204), same for all parameter sets */
#define XASU_MLDSA_KEYGEN_SEED_LEN	(32U) /**< KeyGen seed (xi) length in bytes */
#define XASU_MLDSA_SIGN_RND_LEN		(32U) /**< Signing randomness (rnd) length in bytes */

/* Per parameter-set sizes in bytes (FIPS 204) */
#define XASU_MLDSA_44_PUB_KEY_LEN	(1312U) /**< ML-DSA-44 public key length */
#define XASU_MLDSA_44_PRIV_KEY_LEN	(2560U) /**< ML-DSA-44 secret key length */
#define XASU_MLDSA_44_SIGN_LEN		(2420U) /**< ML-DSA-44 signature length */

#define XASU_MLDSA_65_PUB_KEY_LEN	(1952U) /**< ML-DSA-65 public key length */
#define XASU_MLDSA_65_PRIV_KEY_LEN	(4032U) /**< ML-DSA-65 secret key length */
#define XASU_MLDSA_65_SIGN_LEN		(3309U) /**< ML-DSA-65 signature length */

#define XASU_MLDSA_87_PUB_KEY_LEN	(2592U) /**< ML-DSA-87 public key length */
#define XASU_MLDSA_87_PRIV_KEY_LEN	(4896U) /**< ML-DSA-87 secret key length */
#define XASU_MLDSA_87_SIGN_LEN		(4627U) /**< ML-DSA-87 signature length */

/* Maximum sizes across all supported parameter sets (used for buffer sizing) */
#define XASU_MLDSA_MAX_PUB_KEY_LEN	(XASU_MLDSA_87_PUB_KEY_LEN)  /**< Max public key length */
#define XASU_MLDSA_MAX_PRIV_KEY_LEN	(XASU_MLDSA_87_PRIV_KEY_LEN) /**< Max secret key length */
#define XASU_MLDSA_MAX_SIGN_LEN		(XASU_MLDSA_87_SIGN_LEN)     /**< Max signature length */

/************************************** Type Definitions *****************************************/
/** This enum lists the supported ML-DSA parameter sets (FIPS 204). */
typedef enum {
	XASU_MLDSA_44 = 0,	/**< ML-DSA-44 (NIST security category 2) */
	XASU_MLDSA_65,		/**< ML-DSA-65 (NIST security category 3) */
	XASU_MLDSA_87,		/**< ML-DSA-87 (NIST security category 5) */
} XAsu_MldsaParamSet;

/**
 * This structure contains ML-DSA signature generation/verification parameters. Both operations
 * share the same layout and are mutually exclusive, so a single generic parameter set is used:
 *  - For signing, KeyAddr/KeyLen refer to the input secret key and SignatureAddr is the output
 *    signature; Deterministic selects the randomness mode.
 *  - For verification, KeyAddr/KeyLen refer to the input public key and SignatureAddr is the input
 *    signature; Deterministic is unused and must be left zero.
 * The key is supplied either directly through KeyAddr/KeyLen or from the ASU key vault through
 * KeyId. Exactly one of the two must be non-zero, as enforced by XAsu_KmValidateKeyAddrNdKeyId.
 * ShaType and ShaMode select the HashML-DSA pre-hash function and are used only when SignMode is
 * XASU_MLDSA_SIGN_MODE_PREHASH; they must be left zero for pure ML-DSA. The pair encodes 7 of the
 * 12 pre-hash functions FIPS 204 approves: SHA2-256, SHA2-384 and SHA2-512 with XASU_SHA2_TYPE,
 * and SHA3-256, SHA3-384, SHA3-512 and SHAKE256 with XASU_SHA3_TYPE. The remaining five,
 * SHA2-224, SHA2-512/224, SHA2-512/256, SHA3-224 and SHAKE128, cannot be requested: the SHA mode
 * encoding defines only XASU_SHA_MODE_256/_384/_512/_SHAKE256, and the ASU SHA hardware offers no
 * programmable initial value for the truncated SHA-2 variants nor a programmable rate for the
 * remaining Keccak ones.
 */
typedef struct {
	u64 KeyAddr;		/**< Address of the input key (secret key for sign, public key for
				  *  verify); 0 when the key is taken from the key vault via KeyId */
	u64 MsgAddr;		/**< Address of the message to be signed/verified; must be a valid
				  *  (non-zero) buffer pointer even when MsgLen is 0 (empty message) */
	u64 SignatureAddr;	/**< Address of the signature (output for sign, input for verify) */
	u64 ContextAddr;	/**< Address of the context string (0 if unused) */
	u32 KeyLen;		/**< Length of the key in bytes */
	u32 MsgLen;		/**< Length of the message in bytes */
	u32 SignatureLen;	/**< Length of the signature in bytes */
	u32 ContextLen;		/**< Length of the context string in bytes (0..255) */
	u32 KeyId;		/**< Composite key vault identifier of the key to operate with;
				  *  0 when the key is supplied directly through KeyAddr */
	u8  ParamSet;		/**< Parameter set (XAsu_MldsaParamSet) */
	u8  SignMode;		/**< Pure or pre-hash mode (XASU_MLDSA_SIGN_MODE_PURE/_PREHASH);
				  *  required for verify too, and must match how the signature was
				  *  produced */
	u8  ShaType;		/**< Pre-hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE), for
				  *  PREHASH mode */
	u8  ShaMode;		/**< Pre-hash SHA mode, where XASU_SHA_MODE_SHAKE256 is valid only
				  *  for SHA3 type, for PREHASH mode */
	u8  Deterministic;	/**< Randomness mode (XASU_MLDSA_HEDGED/_DETERMINISTIC); read on
				  *  sign only and must be left zero for verify */
	u8  Reserved[7];	/**< Reserved to pad the structure to 64 bytes */
} XAsu_MldsaSignVerifyParams;

/**
 * This structure contains the parameters of the signature generation command that takes its rnd
 * from the request, which exists for algorithm validation only (see
 * XASU_MLDSA_SIGN_DERAND_CMD_ID). It carries the ordinary signing parameters plus the rnd to sign
 * with, laid out the same way XAsu_MlkemEncapsDerandParams appends the message m it encapsulates
 * with. The secret key must be supplied through SignParams.KeyAddr: the key vault is not available
 * to this command, so SignParams.KeyId must be zero. SignParams.Deterministic is ignored, since
 * the supplied rnd decides the randomness outright, and RndAddr must be non-zero: an absent rnd is
 * an error rather than a request to fall back on the TRNG.
 */
typedef struct {
	XAsu_MldsaSignVerifyParams SignParams;	/**< Signing parameters, as the signature
						  *  generation command takes them */
	u64 RndAddr;		/**< Address of the signing randomness rnd; must be non-zero */
	u32 RndLen;		/**< Length of rnd in bytes; must equal XASU_MLDSA_SIGN_RND_LEN */
	u8  Reserved[4];	/**< Reserved to pad the structure to 80 bytes */
} XAsu_MldsaSignDerandParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_MLDSAINFO_H_ */
/** @} */
