/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_elliptic.h
* This file contains the interface functions for ECC engine.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   vns  03/27/19 First Release
* 4.0   vns  08/24/20 Updated file version to sync with library version
* 4.2   har  11/07/19 Typo correction to enable compilation in C++
*       rpo  04/02/20 Added crypto KAT APIs
* 4.3   har  08/24/20 Added function prototype for APIs to generate and verify
*                     ECDSA public key and signature
*       am   09/25/20 Resolved MISRA C violations
*       har  10/12/20 Addressed security review comments
*       har  10/14/20 Replaced ecdsa with elliptic in names of function and
*                     macros
* 4.5   har  01/18/21 Updated prototype for XSecure_EllipticKat
* 4.6   har  07/14/21 Fixed doxygen warnings
*       gm   07/16/21 Added support for 64-bit address
* 5.0   kpt  07/24/22 Moved XSecure_EllipticKat into xsecure_kat.c
* 5.2   yog  05/18/23 Updated the flow for Big Endian ECC Mode setting
*       yog  06/07/23 Added support for P-256 Curve
*       mmd  07/09/23 Included header file for crypto algorithm information
*       am   08/18/23 Added XSecure_EllipticGetCrvSize() prototype
*	vss  09/11/2023 Fixed MISRA-C Rule 8.13 violation
* 5.4   yog  04/29/24 Fixed doxygen grouping and doxygen warnings.
*       mb   05/23/24 Added support for P-192 Curve
*       mb   05/23/24 Added support for P-224 Curve
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_ecdsa_server_apis XilSecure ECDSA Server APIs
* @{
*/
#ifndef XSECURE_ELLIPTIC_H_
#define XSECURE_ELLIPTIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"

#ifndef PLM_ECDSA_EXCLUDE
#include "xil_types.h"
#include "xsecure_ellipticcrvs.h"
#include "xsecure_ellipticalginfo.h"

/************************** Constant Definitions ****************************/
#define XSECURE_ECC_P384_SIZE_IN_BYTES	(48U)
									/**< Size of NIST P-384 curve in bytes */
#define XSECURE_ECC_P521_SIZE_IN_BYTES	(66U)
									/**< Size of NIST P-521 curve in bytes */
#define XSECURE_ECC_P256_SIZE_IN_BYTES	(32U)
									/**< Size of NIST P-256 curve in bytes */
#define XSECURE_ECC_P192_SIZE_IN_BYTES	(24U)
									/**< Size of NIST P-192 curve in bytes */
#define XSECURE_ECC_P224_SIZE_IN_BYTES	(28U)
									/**< Size of NIST P-224 curve in bytes */
#define XSECURE_ECDSA_P521_ALIGN_BYTES	(2U)
				/**< Size of NIST P-521 curve is 66 bytes. This macro is used
				to make the address word aligned */

#define XSECURE_ELLIPTIC_LITTLE_ENDIAN	(0U)
									/**< Operates APIs on little endian format */

/***************************** Type Definitions ******************************/
/** Structure for pointers of public key curve points. */
typedef struct {
	u8 *Qx;		/**< Public key curve point x */
	u8 *Qy;		/**< Public key curve point y */
} XSecure_EllipticKey;

/** Structure for pointers of signature components. */
typedef struct {
	u8 *SignR;		/**< The signature component R */
	u8 *SignS;		/**< The signature component S */
} XSecure_EllipticSign;

/** Structure for addresses of public key curve points. */
typedef struct {
	u64 Qx;		/**< Address of Public key curve point x */
	u64 Qy;		/**< Address of Public key curve point y */
} XSecure_EllipticKeyAddr;

/** Structure for addresses of signature components */
typedef struct {
	u64 SignR;		/**< Address of the signature component R */
	u64 SignS;		/**< Address of the signature component S */
} XSecure_EllipticSignAddr;

/**
 * Structure for input parameters Hash and Length
 * for Elliptic Sign generation.
 */
typedef struct {
	u64 Addr;		/**< Address of the hash */
	u32 Len;		/**< Length of the hash */
} XSecure_EllipticHashData;

/***************************** Function Prototypes ***************************/
int XSecure_EllipticGenerateKey(XSecure_EllipticCrvTyp CrvType, const u8* D,
	const XSecure_EllipticKey *Key);
int XSecure_EllipticGenerateSignature(XSecure_EllipticCrvTyp CrvType, const u8* Hash,
	const u32 HashLen, const u8* D, const u8* K, const XSecure_EllipticSign *Sign);
int XSecure_EllipticValidateKey(XSecure_EllipticCrvTyp CrvType,
	const XSecure_EllipticKey *Key);
int XSecure_EllipticVerifySign(XSecure_EllipticCrvTyp CrvType, const u8 *Hash,
	const u32 HashLen, const XSecure_EllipticKey *Key, const XSecure_EllipticSign *Sign);

/* 64 Bit address supported APIs */
int XSecure_EllipticGenerateKey_64Bit(XSecure_EllipticCrvTyp CrvType,
	const u64 DAddr, const XSecure_EllipticKeyAddr *KeyAddr);
int XSecure_EllipticGenerateSignature_64Bit(XSecure_EllipticCrvTyp CrvType,
	const XSecure_EllipticHashData *HashInfo, const u64 DAddr,
	const u64 KAddr, const XSecure_EllipticSignAddr *SignAddr);
int XSecure_EllipticValidateKey_64Bit(XSecure_EllipticCrvTyp CrvType,
	const XSecure_EllipticKeyAddr *KeyAddr);
int XSecure_EllipticVerifySign_64Bit(XSecure_EllipticCrvTyp CrvType,
	const XSecure_EllipticHashData *HashInfo, const XSecure_EllipticKeyAddr *KeyAddr,
	const XSecure_EllipticSignAddr *SignAddr);
/**
 * @cond xsecure_internal
 * @{
 */
void XSecure_PutData(const u32 Size, u8 *Dst, const u64 SrcAddr);
void XSecure_GetData(const u32 Size, const u8 *Src, const u64 DstAddr);
void XSecure_FixEndiannessNCopy(const u32 Size, u64 DstAddr, const u64 SrcAddr);
u32 XSecure_EllipticGetCrvSize(const XSecure_EllipticCrvTyp CrvType);
/**
 * @}
 * @endcond
 */
#endif

#ifdef __cplusplus
}
#endif

#endif
/** @} */
