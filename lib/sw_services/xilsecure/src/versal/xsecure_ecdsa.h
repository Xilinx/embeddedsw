/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ecdsa.h
* @addtogroup xsecure_ecdsa_apis XilSecure ECDSA APIs
* @{
* @cond xsecure_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0 	vns  03/27/19 First Release
* 4.0   vns  08/24/20 Updated file version to sync with library version
* 4.2   har  11/07/19 Typo correction to enable compilation in C++
*       rpo  04/02/20 Added crypto KAT APIs
* 4.3   har  08/24/20 Added function prototype for APIs to generate and verify
*                     ECDSA public key and signature
*
* </pre>
*
* @endcond
******************************************************************************/
#ifndef XSECURE_ECDSA_H_
#define XSECURE_ECDSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_ecdsacrvs.h"

/************************** Constant Definitions ****************************/
#define XSECURE_ECDSA_P384_SIZE_IN_BYTES	(48U)
#define XSECURE_ECDSA_P521_SIZE_IN_BYTES	(66U)
#define XSECURE_SHA3_LEN_BYTES				(48U)
#define XSECURE_ECC_P384_DATA_SIZE_WORDS	\
						(XSECURE_ECDSA_P384_SIZE_IN_BYTES / XSECURE_WORD_SIZE)

/***************************** Type Definitions ******************************/
typedef struct {
	u8 *Qx;
	u8 *Qy;
} XSecure_EcdsaKey;

typedef struct {
	u8 *SignR;
	u8 *SignS;
} XSecure_EcdsaSign;

/***************************** Function Prototypes ***************************/
int XSecure_EcdsaGenerateKey(XSecure_EcdsaCrvTyp CrvType, const u8* D,
	XSecure_EcdsaKey *Key);
int XSecure_EcdsaGenerateSign(XSecure_EcdsaCrvTyp CrvType, const u8* Hash,
	const u32 HashLen, const u8* D, const u8* K, XSecure_EcdsaSign *Sign);
int XSecure_EcdsaValidateKey(XSecure_EcdsaCrvTyp CrvType, XSecure_EcdsaKey *Key);
int XSecure_EcdsaVerifySign(XSecure_EcdsaCrvTyp CrvType, const u8 *Hash,
	const u32 HashLen, XSecure_EcdsaKey *Key, XSecure_EcdsaSign *Sign);
u32 XSecure_EcdsaKat(void);

#ifdef __cplusplus
}
#endif

#endif
/* @} */