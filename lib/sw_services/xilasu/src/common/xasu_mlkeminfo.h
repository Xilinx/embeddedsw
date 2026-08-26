/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_mlkeminfo.h
 *
 * This file contains the ML-KEM (FIPS 203) definitions which are common across the
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
#ifndef XASU_MLKEMINFO_H_
#define XASU_MLKEMINFO_H_

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
 * ML-KEM module command IDs. Key pair generation is not one of them: it is reached only through
 * the key manager's XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID, so that a generated decapsulation key can
 * be placed in the key vault rather than handed back to the caller.
 */
#define XASU_MLKEM_ENCAPS_CMD_ID	(0U) /**< Command ID for ML-KEM encapsulation */
#define XASU_MLKEM_DECAPS_CMD_ID	(1U) /**< Command ID for ML-KEM decapsulation */
#define XASU_MLKEM_KAT_CMD_ID		(2U) /**< Command ID for ML-KEM Known Answer Test */
/*
 * Derandomized encapsulation, for algorithm validation only. FIPS 203 draws the message m of an
 * approved ML-KEM.Encaps from an approved RBG, so encapsulating from a caller-supplied m is the
 * standard's internal testing function rather than an approved service. ASUFW therefore serves
 * this command only when built with XASU_ENABLE_CAVP_SUPPORT; a production build rejects it with
 * XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED. The ID is defined unconditionally so that both
 * builds agree on XASU_MLKEM_MAX_CMDS and on the module table layout.
 */
#define XASU_MLKEM_ENCAPS_DERAND_CMD_ID	(3U) /**< Command ID for derandomized encapsulation */
#define XASU_MLKEM_MAX_CMDS		(4U) /**< Maximum commands supported by ML-KEM module */

/* ML-KEM shared secret length is 32 bytes for all parameter sets (FIPS 203) */
#define XASU_MLKEM_SHARED_SECRET_LEN	(32U) /**< Shared secret length in bytes */

/* Derandomization seed lengths (FIPS 203), same for all parameter sets */
#define XASU_MLKEM_KEYGEN_SEED_LEN	(64U) /**< KeyGen seed length (d || z) in bytes */
#define XASU_MLKEM_ENCAPS_SEED_LEN	(32U) /**< Encaps message/randomness (m) length in bytes */

/* Per parameter-set sizes in bytes (FIPS 203) */
#define XASU_MLKEM_512_PUB_KEY_LEN	(800U)  /**< ML-KEM-512 public key length */
#define XASU_MLKEM_512_PRIV_KEY_LEN	(1632U) /**< ML-KEM-512 secret key length */
#define XASU_MLKEM_512_CT_LEN		(768U)  /**< ML-KEM-512 ciphertext length */

#define XASU_MLKEM_768_PUB_KEY_LEN	(1184U) /**< ML-KEM-768 public key length */
#define XASU_MLKEM_768_PRIV_KEY_LEN	(2400U) /**< ML-KEM-768 secret key length */
#define XASU_MLKEM_768_CT_LEN		(1088U) /**< ML-KEM-768 ciphertext length */

#define XASU_MLKEM_1024_PUB_KEY_LEN	(1568U) /**< ML-KEM-1024 public key length */
#define XASU_MLKEM_1024_PRIV_KEY_LEN	(3168U) /**< ML-KEM-1024 secret key length */
#define XASU_MLKEM_1024_CT_LEN		(1568U) /**< ML-KEM-1024 ciphertext length */

/* Maximum sizes across all supported parameter sets (used for buffer sizing) */
#define XASU_MLKEM_MAX_PUB_KEY_LEN	(XASU_MLKEM_1024_PUB_KEY_LEN)  /**< Max public key length */
#define XASU_MLKEM_MAX_PRIV_KEY_LEN	(XASU_MLKEM_1024_PRIV_KEY_LEN) /**< Max secret key length */
#define XASU_MLKEM_MAX_CT_LEN		(XASU_MLKEM_1024_CT_LEN)       /**< Max ciphertext length */

/************************************** Type Definitions *****************************************/
/** This enum lists the supported ML-KEM parameter sets (FIPS 203). */
typedef enum {
	XASU_MLKEM_512 = 0,	/**< ML-KEM-512 (NIST security category 1) */
	XASU_MLKEM_768,		/**< ML-KEM-768 (NIST security category 3) */
	XASU_MLKEM_1024,	/**< ML-KEM-1024 (NIST security category 5) */
} XAsu_MlkemParamSet;

/**
 * This structure contains ML-KEM encapsulation/decapsulation parameters. Both operations share
 * the same layout and are mutually exclusive, so a single generic parameter set is used:
 *  - For encapsulation, KeyAddr/KeyLen refer to the input public (encapsulation) key and CtAddr is
 *    the output ciphertext.
 *  - For decapsulation, KeyAddr/KeyLen refer to the input secret (decapsulation) key and CtAddr is
 *    the input ciphertext.
 * The key is supplied either directly through KeyAddr/KeyLen or from the ASU key vault through
 * KeyId. Exactly one of the two must be non-zero, as enforced by XAsu_KmValidateKeyAddrNdKeyId.
 */
typedef struct {
	u64 KeyAddr;		/**< Address of the input key (public key for encaps, secret key for
				  *  decaps); 0 when the key is taken from the key vault via KeyId */
	u64 CtAddr;		/**< Address of the ciphertext (output for encaps, input for decaps) */
	u64 SharedSecretAddr;	/**< Address of the output shared secret */
	u32 KeyLen;		/**< Length of the key in bytes */
	u32 CtLen;		/**< Length of the ciphertext in bytes */
	u32 SharedSecretLen;	/**< Length of the shared secret buffer in bytes */
	u32 KeyId;		/**< Composite key vault identifier of the key to operate with;
				  *  0 when the key is supplied directly through KeyAddr */
	u8  ParamSet;		/**< Parameter set (XAsu_MlkemParamSet) */
	u8  Reserved[7];	/**< Reserved to pad the structure to 48 bytes */
} XAsu_MlkemEncapsDecapsParams;

/**
 * This structure contains the parameters of the derandomized encapsulation command, which exists
 * for algorithm validation only (see XASU_MLKEM_ENCAPS_DERAND_CMD_ID). It carries the ordinary
 * encapsulation parameters plus the message m to encapsulate with, laid out the same way
 * XAsu_KeyManagerPqcKeyGenParams appends a seed to the parameters it shares. The encapsulation key
 * must be supplied through EncapsParams.KeyAddr: the key vault is not available to this command,
 * so EncapsParams.KeyId must be zero. Unlike key generation, a zero SeedAddr is not a request to
 * draw from the TRNG but an error, since supplying m is this command's only purpose.
 */
typedef struct {
	XAsu_MlkemEncapsDecapsParams EncapsParams; /**< Encapsulation parameters, as the
						     *  encapsulation command takes them */
	u64 SeedAddr;		/**< Address of the message m to encapsulate with; must be
				  *  non-zero */
	u32 SeedLen;		/**< Length of the message m in bytes; must equal
				  *  XASU_MLKEM_ENCAPS_SEED_LEN */
	u8  Reserved[4];	/**< Reserved to pad the structure to 64 bytes */
} XAsu_MlkemEncapsDerandParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_MLKEMINFO_H_ */
/** @} */
