/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
#define XSECURE_ECC_P521_SIZE_IN_BYTES	(66U)
#define XSECURE_SHA3_LEN_BYTES		(48U)
#define XSECURE_ECC_P384_DATA_SIZE_WORDS	\
					(XSECURE_ECC_P384_SIZE_IN_BYTES / XSECURE_WORD_SIZE)

/***************************** Type Definitions ******************************/
typedef struct {
	u8 *Qx;
	u8 *Qy;
} XSecure_EllipticKey;

typedef struct {
	u8 *SignR;
	u8 *SignS;
} XSecure_EllipticSign;

/***************************** Function Prototypes ***************************/
int XSecure_EllipticGenerateKey(XSecure_EllipticCrvTyp CrvType, const u8* D,
	XSecure_EllipticKey *Key);
int XSecure_EllipticGenerateSignature(XSecure_EllipticCrvTyp CrvType, const u8* Hash,
	const u32 HashLen, const u8* D, const u8* K, XSecure_EllipticSign *Sign);
int XSecure_EllipticValidateKey(XSecure_EllipticCrvTyp CrvType,
	XSecure_EllipticKey *Key);
int XSecure_EllipticVerifySign(XSecure_EllipticCrvTyp CrvType, const u8 *Hash,
	const u32 HashLen, XSecure_EllipticKey *Key, XSecure_EllipticSign *Sign);
int XSecure_EllipticKat(void);

#ifdef __cplusplus
}
#endif

#endif
/* @} */
