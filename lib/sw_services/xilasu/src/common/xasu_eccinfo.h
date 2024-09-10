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
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASU_ECCINFO_H
#define XASU_ECCINFO_H

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
/************************************** Type Definitions *****************************************/
typedef struct {
	u32 CurveType; /**< Type of curve */
	u32 KeyLen; /**< Length of the key */
	u32 DigestLen; /**< Length of the digest provided */
	u32 Reserved; /**< Reserved */
	u64 KeyAddr; /**< Key address */
	u64 DigestAddr; /**< Digest address */
	u64 SignAddr; /**< Signature address */
} XAsu_EccParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/*************************************************************************************************/
/**
 * @brief	This function is to get the double curve length.
 *
 * @param	CurveLength	Length of the curve
 *
 * @return
 *	-	Double Curve length
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

#endif  /* XASU_ECCINFO_H */
