/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_elliptic.h
* @addtogroup xsecure_elliptic_apis XilSecure Elliptic APIs
* @{
* @cond xsecure_internal
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
*
* </pre>
*
* @endcond
******************************************************************************/
#ifndef XSECURE_ELLIPTIC_H_
#define XSECURE_ELLIPTIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_ellipticcrvs.h"

/************************** Constant Definitions ****************************/
#define XSECURE_ECC_P384_SIZE_IN_BYTES	(48U)
									/**< Size of NIST P-384 curve in bytes */
#define XSECURE_ECC_P521_SIZE_IN_BYTES	(66U)
									/**< Size of NIST P-521 curve in bytes */
#define XSECURE_ECC_P384_DATA_SIZE_WORDS	\
					(XSECURE_ECC_P384_SIZE_IN_BYTES / XSECURE_WORD_SIZE)
									/**< Size of NIST P-384 curve in words */

/***************************** Type Definitions ******************************/
typedef struct {
	u8 *Qx;		/**< Public key curve point x */
	u8 *Qy;		/**< Public key curve point y */
} XSecure_EllipticKey;

typedef struct {
	u8 *SignR;		/**< The signature component R */
	u8 *SignS;		/**< The signature component S */
} XSecure_EllipticSign;

typedef struct {
	u64 Qx;		/**< Address of Public key curve point x */
	u64 Qy;		/**< Address of Public key curve point y */
} XSecure_EllipticKeyAddr;

typedef struct {
	u64 SignR;		/**< Address of the signature component R */
	u64 SignS;		/**< Address of the signature component S */
} XSecure_EllipticSignAddr;

typedef struct {
	u64 Addr;		/**< Address of the hash */
	u32 Len;		/**< Length of the hash */
} XSecure_EllipticHashData;

/***************************** Function Prototypes ***************************/
int XSecure_EllipticGenerateKey(XSecure_EllipticCrvTyp CrvType, const u8* D,
	XSecure_EllipticKey *Key);
int XSecure_EllipticGenerateSignature(XSecure_EllipticCrvTyp CrvType, const u8* Hash,
	const u32 HashLen, const u8* D, const u8* K, XSecure_EllipticSign *Sign);
int XSecure_EllipticValidateKey(XSecure_EllipticCrvTyp CrvType,
	XSecure_EllipticKey *Key);
int XSecure_EllipticVerifySign(XSecure_EllipticCrvTyp CrvType, const u8 *Hash,
	const u32 HashLen, XSecure_EllipticKey *Key, XSecure_EllipticSign *Sign);
int XSecure_EllipticKat(u32 AuthCurve);

/* 64 Bit address supported APIs */
int XSecure_EllipticGenerateKey_64Bit(XSecure_EllipticCrvTyp CrvType,
	const u64 DAddr, XSecure_EllipticKeyAddr *KeyAddr);
int XSecure_EllipticGenerateSignature_64Bit(XSecure_EllipticCrvTyp CrvType,
	XSecure_EllipticHashData *HashInfo, const u64 DAddr,
	const u64 KAddr, XSecure_EllipticSignAddr *SignAddr);
int XSecure_EllipticValidateKey_64Bit(XSecure_EllipticCrvTyp CrvType,
	XSecure_EllipticKeyAddr *KeyAddr);
int XSecure_EllipticVerifySign_64Bit(XSecure_EllipticCrvTyp CrvType,
	XSecure_EllipticHashData *HashInfo, XSecure_EllipticKeyAddr *KeyAddr,
	XSecure_EllipticSignAddr *SignAddr);

#ifdef __cplusplus
}
#endif

#endif
/* @} */