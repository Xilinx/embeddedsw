/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xasu_slhdsainfo.h
*
* This file contains the SLH-DSA definitions which are common across the client and server.
*
* Supported: FIPS 205 pure SLH-DSA and HashSLH-DSA (pre-hash) for all four "s"
* parameter sets (SHAKE/SHA2 x 128s/256s). Pre-hash HashId must match the parameter
* set per FIPS 205 footnote 18 (PH(M) length >= 2n bytes):
* - 128s (n = 16): XASU_SLHDSA_HASHID_SHA256 (32-byte PH(M)).
* - 256s (n = 32): XASU_SLHDSA_HASHID_SHA512 (64-byte PH(M)).
*
* Message mode (PreHashMode in sign/verify parameters):
* - XASU_SLHDSA_MSG_PURE (0): pure SLH-DSA (default when fields are zero).
* - XASU_SLHDSA_MSG_PREHASH (1): HashSLH-DSA; H_msg/PRF_msg absorb
*   0x01 || ctx_len || ctx || OID || PH(M).
*
* Context length semantics (pure SLH-DSA, PreHashMode == PURE):
* - CtxLen == 0: FIPS empty context (hashes 0x00 || 0x00 before the message).
* - CtxLen 1..255: FIPS context string at CtxAddr.
* Sign and verify must use the same CtxLen.
*
* Context length semantics (HashSLH-DSA, PreHashMode == PREHASH):
* - CtxLen 0..255 only. Empty context uses 0x01 || 0x00 || OID.
* - MsgAddr/MsgLen must reference a client-computed PH(M) digest (32 or 64 bytes per HashId).
* Sign and verify must use matching PreHashMode, HashId, and CtxLen.
*
* Sign mode (SignMode in sign parameters):
* - XASU_SLHDSA_SIGN_MODE_HEDGED (0, default): ASUFW draws fresh add_rand (n bytes) from TRNG
*   for each sign (FIPS hedged signing, recommended).
* - XASU_SLHDSA_SIGN_MODE_DETERMINISTIC (1): ASUFW uses PK.seed as add_rand (deterministic).
*
* IPI parameter structures:
* - XAsu_SlhdsaSignParams: sent with XASU_SLHDSA_SIGN_CMD_ID.
* - XAsu_SlhdsaSignVerifyParams: sent with XASU_SLHDSA_SIGN_VERIFY_CMD_ID.
* All 64-bit address fields must reference DMA-visible shared memory.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   07/10/26 Initial release
*       kk   09/23/26 Added XASU_SLHDSA_MAX_SIG_SIZE
*
* </pre>
*
*************************************************************************************************/
/**
* @addtogroup xasu_common_defs Common Defs
* @{
*/
#ifndef XASU_SLHDSAINFO_H_
#define XASU_SLHDSAINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
/** @name SLH-DSA module command IDs
* Key pair generation is not one of them: it is reached only through the key manager's
* XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID.
* @{
*/
#define XASU_SLHDSA_SIGN_CMD_ID       	(0U)	/**< Sign message with SLH-DSA secret key */
#define XASU_SLHDSA_SIGN_VERIFY_CMD_ID	(1U)	/**< Verify SLH-DSA signature */
#define XASU_SLHDSA_KAT_CMD_ID        	(2U)	/**< Run SLH-DSA operation KAT */
#define XASU_SLHDSA_MAX_CMDS          	(3U)	/**< Maximum SLH-DSA commands */
/** @} */

/** Maximum key and signature sizes across supported parameter sets (256s). */
#define XASU_SLHDSA_MAX_PK_SIZE	(64U)	/**< Maximum public key size in bytes */
#define XASU_SLHDSA_MAX_SK_SIZE	(128U)	/**< Maximum secret key size in bytes */
#define XASU_SLHDSA_MAX_SIG_SIZE	(29792U)	/**< Maximum signature size in bytes */

/** FIPS 205 security parameter n (hash width in bytes) for 128s / 256s sets. */
#define XASU_SLHDSA_128S_N	(16U)	/**< Hash width n for 128s parameter sets */
#define XASU_SLHDSA_256S_N	(32U)	/**< Hash width n for 256s parameter sets */

/** FIPS 205 key sizes for 128s parameter sets (n = 16). */
#define XASU_SLHDSA_128S_PK_SIZE	(32U)	/**< Public key size for 128s sets */
#define XASU_SLHDSA_128S_SK_SIZE	(64U)	/**< Secret key size for 128s sets */

/** FIPS 205 key sizes for 256s parameter sets (n = 32). */
#define XASU_SLHDSA_256S_PK_SIZE	(64U)	/**< Public key size for 256s sets */
#define XASU_SLHDSA_256S_SK_SIZE	(128U)	/**< Secret key size for 256s sets */

#define XASU_SLHDSA_SHAKE_128S_SIG_SIZE	(7856U)	/**< Signature size for SHAKE-128s */
#define XASU_SLHDSA_SHAKE_256S_SIG_SIZE	(29792U)	/**< Signature size for SHAKE-256s */
#define XASU_SLHDSA_SHA2_128S_SIG_SIZE	(7856U)	/**< Signature size for SHA2-128s */
#define XASU_SLHDSA_SHA2_256S_SIG_SIZE	(29792U)	/**< Signature size for SHA2-256s */

/** @name SLH-DSA parameter set selectors
 * @{
 */
#define XASU_SLHDSA_PARAM_SHAKE_256S	(0U)	/**< SHAKE-256s parameter set */
#define XASU_SLHDSA_PARAM_SHAKE_128S	(1U)	/**< SHAKE-128s parameter set */
#define XASU_SLHDSA_PARAM_SHA2_256S 	(2U)	/**< SHA2-256s parameter set */
#define XASU_SLHDSA_PARAM_SHA2_128S 	(3U)	/**< SHA2-128s parameter set */
/** @} */

/** @name SLH-DSA message mode selectors
 * @{
 */
#define XASU_SLHDSA_MSG_PURE		(0U)	/**< Pure SLH-DSA (default) */
#define XASU_SLHDSA_MSG_PREHASH		(1U)	/**< HashSLH-DSA pre-hash mode */
/** @} */

/** @name SLH-DSA pre-hash function identifiers
 * @{
 */
#define XASU_SLHDSA_HASHID_SHA256	(0U)	/**< SHA-256 pre-hash for 128s sets. */
#define XASU_SLHDSA_HASHID_SHA512	(1U)	/**< SHA-512 pre-hash for 256s sets. */
#define XASU_SLHDSA_HASHID_INVALID	(0xFFU)	/**< Invalid pre-hash HashId sentinel. */
#define XASU_SLHDSA_PREHASH_SHA256_LEN	(32U)	/**< SHA-256 digest length in bytes */
#define XASU_SLHDSA_PREHASH_SHA512_LEN	(64U)	/**< SHA-512 digest length in bytes */
#define XASU_SLHDSA_PREHASH_OID_LEN	(11U)	/**< DER OID length for supported HashIds */
/** @} */

/** @name FIPS 205 context prefix (M' = DS || ctx_len || ctx [|| OID || PH(M)])
 * @{
 */
#define XASU_SLHDSA_MAX_CTX_LEN			(255U)	/**< FIPS 205 maximum context length. */
#define XASU_SLHDSA_DS_PURE			(0x00U)	/**< Domain separator for pure SLH-DSA. */
#define XASU_SLHDSA_DS_PREHASH			(0x01U)	/**< Domain separator for HashSLH-DSA. */
#define XASU_SLHDSA_PREFIX_DS_OFFSET		(0U)	/**< Domain-separator octet offset. */
#define XASU_SLHDSA_PREFIX_CTXLEN_OFFSET	(1U)	/**< Context-length octet offset. */
#define XASU_SLHDSA_PREFIX_CTX_OFFSET		(2U)	/**< Context string byte offset. */
#define XASU_SLHDSA_PREFIX_HDR_LEN		(2U)	/**< DS plus context-length header. */
/** @} */

/** @name SLH-DSA sign mode selectors (FIPS 205 §9.2 / §10.2)
 * @{
 */
#define XASU_SLHDSA_SIGN_MODE_HEDGED		(0U)	/**< Default: fresh add_rand from TRNG */
#define XASU_SLHDSA_SIGN_MODE_DETERMINISTIC	(1U)	/**< Use PK.seed when add_rand omitted */
/** @} */

/************************************** Type Definitions *****************************************/

/** This structure represents an SLH-DSA secret key object. */
typedef struct {
	u64 SkAddr;	/**< Address of the secret key data */
	u32 SkId;	/**< Reserved for future key management support; ignored */
	u32 SkLen;	/**< Length of the secret key in bytes */
} XAsu_SlhdsaSkObject;

/** This structure represents an SLH-DSA public key object. */
typedef struct {
	u64 PubKeyAddr;	/**< Address of the public key data */
	u32 PubKeyId;	/**< Reserved for future key management support; ignored */
	u32 PubKeyLen;	/**< Length of the public key in bytes */
} XAsu_SlhdsaPkObject;

/** This structure contains SLH-DSA sign parameters. */
typedef struct {
	XAsu_SlhdsaSkObject SkObj;	/**< Secret key object */
	u64 MsgAddr;			/**< Address of the message to be signed */
	u64 SignatureAddr;		/**< Address of the signature */
	u64 CtxAddr;			/**< Address of the context string (0 if unused) */
	u32 MsgLen;			/**< Length of the message in bytes */
	u32 SignatureLen;		/**< Length of the signature in bytes */
	u32 CtxLen;			/**< Length of the context string in bytes (0..255) */
	u8 ParamSet;			/**< Parameter set (XASU_SLHDSA_PARAM_*) */
	u8 PreHashMode;			/**< Pure or pre-hash mode (XASU_SLHDSA_MSG_PURE /
					  *  XASU_SLHDSA_MSG_PREHASH) */
	u8 HashId;			/**< Pre-hash function ID (XASU_SLHDSA_HASHID_SHA256 /
					  *  XASU_SLHDSA_HASHID_SHA512) */
	u8 SignMode;			/**< Hedged or deterministic (XASU_SLHDSA_SIGN_MODE_HEDGED /
					  *  XASU_SLHDSA_SIGN_MODE_DETERMINISTIC) */
} XAsu_SlhdsaSignParams;

/** This structure contains SLH-DSA signature verification parameters. */
typedef struct {
	XAsu_SlhdsaPkObject PkObj;	/**< Public key object */
	u64 MsgAddr;			/**< Address of the message to be verified */
	u64 SignatureAddr;		/**< Address of the signature */
	u64 CtxAddr;			/**< Address of the context string (0 if unused) */
	u32 MsgLen;			/**< Length of the message in bytes */
	u32 SignatureLen;		/**< Length of the signature in bytes */
	u32 CtxLen;			/**< Length of the context string in bytes (0..255) */
	u8 ParamSet;			/**< Parameter set (XASU_SLHDSA_PARAM_*) */
	u8 PreHashMode;			/**< Pure or pre-hash mode (XASU_SLHDSA_MSG_PURE /
					  *  XASU_SLHDSA_MSG_PREHASH) */
	u8 HashId;			/**< Pre-hash function ID (XASU_SLHDSA_HASHID_SHA256 /
					  *  XASU_SLHDSA_HASHID_SHA512) */
	u8 Reserved;			/**< Reserved for alignment */
} XAsu_SlhdsaSignVerifyParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASU_SLHDSAINFO_H_ */
/** @} */
