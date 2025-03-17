/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

/************************************ Constant Definitions ***************************************/
#define XASU_DOUBLE_CURVE_LENGTH_SHIFT		(0x1U) /**< Shift value to double the curve length */

/* ECC module command IDs */
#define XASU_ECC_GEN_SIGNATURE_CMD_ID		0U /**< Command ID for ECC sign generation */
#define XASU_ECC_VERIFY_SIGNATURE_CMD_ID	1U /**< Command ID for ECC sign verification */
#define XASU_ECC_KAT_CMD_ID			2U /**< Command ID for ECC KAT command */
#define XASU_ECC_GET_INFO_CMD_ID		3U /**< Command ID for ECC Get Info command */
#define XASU_ECDH_SHARED_SECRET_CMD_ID		4U /**< Command ID for ECC Get Info command */
#define XASU_ECDH_KAT_CMD_ID			5U /**< Command ID for ECC KAT command */

/* ECC curve Type values */
#define XASU_ECC_NIST_P192			1U /**< NIST P-192 curve */
#define XASU_ECC_NIST_P224			2U /**< NIST P-224 curve */
#define XASU_ECC_NIST_P256			3U /**< NIST P-256 curve */
#define XASU_ECC_NIST_P384			4U /**< NIST P-384 curve */
#define XASU_ECC_NIST_P521			5U /**< NIST P-521 curve */
#define XASU_ECC_BRAINPOOL_P256			21U /**< Brainpool P-256 curve */
#define XASU_ECC_BRAINPOOL_P320			22U /**< Brainpool P-320 curve */
#define XASU_ECC_BRAINPOOL_P384			23U /**< Brainpool P-384 curve */
#define XASU_ECC_BRAINPOOL_P512			24U /**< Brainpool P-512 curve */

#define XASU_ECC_P192_SIZE_IN_BYTES		(24U) /**< Size of NIST P-192 curve in bytes */
#define XASU_ECC_P224_SIZE_IN_BYTES		(28U) /**< Size of NIST P-192 curve in bytes */
#define XASU_ECC_P256_SIZE_IN_BYTES		(32U) /**< P256 Curve size in bytes */
#define XASU_ECC_P320_SIZE_IN_BYTES		(40U) /**< Size of NIST P-521 curve in bytes */
#define XASU_ECC_P384_SIZE_IN_BYTES		(48U) /**< P384 Curve size in bytes */
#define XASU_ECC_P512_SIZE_IN_BYTES		(64U) /**< Size of NIST P-521 curve in bytes */
#define XASU_ECC_P521_SIZE_IN_BYTES		(66U) /**< Size of NIST P-521 curve in bytes */

/************************************** Type Definitions *****************************************/
/**
 * @brief This structure contains ECC params info
 */
typedef struct {
	u32 CurveType; /**< Type of curve */
	u32 KeyLen; /**< Length of the key */
	u32 DigestLen; /**< Length of the digest provided */
	u32 Reserved; /**< Reserved */
	u64 KeyAddr; /**< Key address */
	u64 DigestAddr; /**< Digest address */
	u64 SignAddr; /**< Signature address */
} XAsu_EccParams;

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
/** @} */
