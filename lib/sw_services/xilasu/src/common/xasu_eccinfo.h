/**************************************************************************************************
* Copyright (c) 2024 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_eccinfo.h
 *
 * This file contains the ECC definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  07/11/24 Initial release
 *       yog  08/19/24 Added XAsu_EccParams structure, module command ID's and curve type values
 *       ss   12/02/24 Added support for ECDH
 *       yog  03/25/25 Added XAsu_EccKeyParams structure and XASU_ECC_GEN_PUBKEY_CMD_ID.
 *       yog  07/11/25 Added XASU_ECC_P448_SIZE_IN_BYTES macro and updated curve type macro values.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/

#ifndef XASU_ECCINFO_H_
#define XASU_ECCINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
#define XASU_DOUBLE_CURVE_LENGTH_SHIFT		(0x1U) /**< Shift value to double the curve length */

/* ECC module command IDs */
#define XASU_ECC_GEN_SIGNATURE_CMD_ID		(0U) /**< Command ID for ECC sign generation */
#define XASU_ECC_VERIFY_SIGNATURE_CMD_ID	(1U) /**< Command ID for ECC sign verification */
#define XASU_ECC_KAT_CMD_ID			(2U) /**< Command ID for ECC KAT */
#define XASU_ECDH_SHARED_SECRET_CMD_ID		(3U) /**< Command ID for ECDH generate shared secret */
#define XASU_ECDH_KAT_CMD_ID			(4U) /**< Command ID for ECDH KAT */
#define XASU_ECC_GEN_PUBKEY_CMD_ID		(5U) /**< Command ID for ECC public key generation */
#define XASU_ECC_VAL_PUBKEY_CMD_ID		(6U) /**< Command ID for ECC public key validation */
#define XASU_ECC_MAX_CMDS			(7U) /**< Maximum number of commands supported by ECC module */

/* ECC curve Type index values */
#define XASU_ECC_NIST_P256			(0U) /**< NIST P-256 curve */
#define XASU_ECC_NIST_P384			(1U) /**< NIST P-384 curve */
#define XASU_ECC_NIST_P192			(2U) /**< NIST P-192 curve */
#define XASU_ECC_NIST_P224			(3U) /**< NIST P-224 curve */
#define XASU_ECC_NIST_P521			(4U) /**< NIST P-521 curve */
#define XASU_ECC_BRAINPOOL_P256			(5U) /**< Brainpool P-256 curve */
#define XASU_ECC_BRAINPOOL_P320			(6U) /**< Brainpool P-320 curve */
#define XASU_ECC_BRAINPOOL_P384			(7U) /**< Brainpool P-384 curve */
#define XASU_ECC_BRAINPOOL_P512			(8U) /**< Brainpool P-512 curve */
#define XASU_ECC_NIST_ED25519			(9U) /**< NIST ED25519 curve */
#define XASU_ECC_NIST_ED448			(10U) /**< NIST ED448 curve */
#define XASU_ECC_NIST_ED25519_PH		(11U) /**< NIST Hash-ED25519 curve */
#define XASU_ECC_NIST_ED448_PH			(12U) /**< NIST Hash-ED448 curve */
#define XASU_ECC_CURVE25519			(13U) /**< Curve25519 curve */
#define XASU_ECC_CURVE448			(14U) /**< Curve448 curve */

#define XASU_ECC_P192_PVT_KEY_SIZE_IN_BYTES		(24U) /**< Size of NIST P-192 private key
								size in bytes */
#define XASU_ECC_P224_PVT_KEY_SIZE_IN_BYTES		(28U) /**< Size of NIST P-224 private key
								size in bytes */
#define XASU_ECC_P256_PVT_KEY_SIZE_IN_BYTES		(32U) /**< Size of NIST P-256 private key
								size in bytes */
#define XASU_ECC_P320_PVT_KEY_SIZE_IN_BYTES		(40U) /**< Size of NIST P-320 private key
								size in bytes */
#define XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES		(48U) /**< Size of NIST P-384 private key
								size in bytes */
#define XASU_ECC_P512_PVT_KEY_SIZE_IN_BYTES		(64U) /**< Size of Brainpool P-512 private
								key size in bytes */
#define XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES		(66U) /**< Size of NIST P-521 private key
								size in bytes */
#define XASU_ECC_ED448_PVT_KEY_SIZE_IN_BYTES		(57U) /**< Size of Edwards Ed448 private
								key size in bytes */
#define XASU_ECC_CURVE448_PVT_KEY_SIZE_IN_BYTES		(56U) /**< Size of Curve448 private
								key size in bytes */
#define XASU_ECC_P521_PUB_KEY_SIZE_IN_BYTES		(132U) /**< Size of NIST P-521 public key
								size in bytes */
#define XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES		(96U) /**< Size of NIST P-384 public key
								size in bytes */

/** @} */
/************************************** Type Definitions *****************************************/
/** Common key object structure used by all ECC operations. */
typedef struct {
	u64 KeyAddr;     /**< Address of the key data (private or public) */
	u32 KeyId;       /**< Key identifier from key management (used if KeyAddr == 0) */
	u32 KeyLen;      /**< Length of the key in bytes (curve length) */
} XAsu_EccKeyObject;

/** This structure contains ECC params info for signature operations. */
typedef struct {
	XAsu_EccKeyObject Key;  /**< Key object (private or public) */
	u64 DigestAddr;         /**< Address of digest */
	u64 SignAddr;           /**< Address of signature */
	u32 CurveType;          /**< Type of curve */
	u32 DigestLen;          /**< Length of the digest provided */
} XAsu_EccParams;

/** This structure contains ECC params info for public key generation. */
typedef struct {
	XAsu_EccKeyObject PvtKey; /**< Private key object */
	u64 PubKeyAddr;         /**< Address where public key will be written (size = 2 * PvtKey.KeyLen) */
	u32 CurveType;          /**< Type of curve */
	u8 Reserved[4]; /**< Explicit padding to ensure consistent struct size across architectures */
} XAsu_EccKeyParams;

/** This structure contains ECDH params info. */
typedef struct {
	XAsu_EccKeyObject PvtKey; /**< Private key object */
	XAsu_EccKeyObject PubKey; /**< Peer's public key object */
	u64 SharedSecretAddr;   /**< Address where shared secret will be written (size = PvtKey.KeyLen) */
	u64 SharedSecretObjIdAddr; /**< Optional object ID for shared secret storage */
	u32 CurveType;          /**< Type of curve */
	u8 Reserved[4]; /**< Explicit padding to ensure consistent struct size across architectures */
} XAsu_EcdhParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/*************************************************************************************************/
/**
 * @brief	This function is to get the double curve length.
 *
 * @param	CurveLength	Length of the curve.
 *
 * @return
 *	-	Double Curve length.
 *
 *************************************************************************************************/
static inline u32 XAsu_DoubleCurveLength(u32 CurveLength)
{
	return (CurveLength << XASU_DOUBLE_CURVE_LENGTH_SHIFT);
}

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_ECCINFO_H_ */
