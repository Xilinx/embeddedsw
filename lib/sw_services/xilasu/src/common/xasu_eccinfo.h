/**************************************************************************************************
* Copyright (c) 2024 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
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

#define XASU_ECC_P192_SIZE_IN_BYTES		(24U) /**< Size of NIST P-192 curve in bytes */
#define XASU_ECC_P224_SIZE_IN_BYTES		(28U) /**< Size of NIST P-224 curve in bytes */
#define XASU_ECC_P256_SIZE_IN_BYTES		(32U) /**< Size of NIST P-256 curve in bytes */
#define XASU_ECC_P320_SIZE_IN_BYTES		(40U) /**< Size of NIST P-320 curve in bytes */
#define XASU_ECC_P384_SIZE_IN_BYTES		(48U) /**< Size of NIST P-384 curve in bytes */
#define XASU_ECC_P512_SIZE_IN_BYTES		(64U) /**< Size of Brainpool P-512 curve in bytes */
#define XASU_ECC_P521_SIZE_IN_BYTES		(66U) /**< Size of NIST P-521 curve in bytes */
#define XASU_ECC_P448_SIZE_IN_BYTES		(57U) /**< Size of Edwards Ed448 curve in bytes */

/** @} */
/************************************** Type Definitions *****************************************/
/** This structure contains ECC params info. */
typedef struct {
	u32 CurveType; /**< Type of curve */
	u32 KeyLen; /**< Length of the key */
	u32 DigestLen; /**< Length of the digest provided */
	u32 Reserved; /**< Reserved */
	u64 KeyAddr; /**< Key address */
	u64 DigestAddr; /**< Digest address */
	u64 SignAddr; /**< Signature address */
} XAsu_EccParams;

/** This structure contains ECC params info for public key generation. */
typedef struct {
	u32 CurveType; /**< Type of curve */
	u32 KeyLen; /**< Length of the key */
	u64 PvtKeyAddr; /**< ECC Private Key buffer address of X party, of KeyLen size */
	u64 PubKeyAddr; /**< ECC Public Key buffer address of Y party, of double the KeyLen size */
} XAsu_EccKeyParams;

/** This structure contains ECDH params info. */
typedef struct {
	u32 CurveType; /**< Type of curve */
	u32 KeyLen; /**< Length of the key */
	u64 PvtKeyAddr; /**< Private Key address */
	u64 PubKeyAddr; /**< Public Key address */
	u64 SharedSecretAddr; /**< Shared Secret address */
	u64 SharedSecretObjIdAddr; /**< 0 : if SharedSecretAddr is not null
				non zero: if SharedSecretAddr is null */
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
