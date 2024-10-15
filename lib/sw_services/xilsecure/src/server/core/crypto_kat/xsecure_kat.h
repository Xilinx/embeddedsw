/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_kat.h
*
* This file contains KAT interface APIs for Versal and Versal Net
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 5.0   kpt  07/15/2022 Initial release
* 5.1   yog  05/03/2023 Fixed MISRA C violation of Rule 8.3
* 5.4   yog  04/29/2024 Fixed doxygen grouping.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_kat_server_apis XilSecure KAT Server APIs
* @{
*/
#ifndef XSECURE_KAT_H_
#define XSECURE_KAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xsecure_aes.h"
#include "xsecure_sha.h"
#ifndef PLM_ECDSA_EXCLUDE
#include "xsecure_elliptic.h"
#endif

/************************** Constant Definitions *****************************/

#define XSECURE_KAT_MSG_LEN_IN_BYTES				 (32U) /**<Message length of known answer test in bytes*/
#define XSECURE_KAT_AAD_SIZE_IN_BYTES				 (16U) /**<AAD size of known answer test in bytes*/
#define XSECURE_KAT_IV_SIZE_IN_BYTES				 (16U) /**<IV size of known answer test in bytes*/
#define XSECURE_KAT_KEY_SIZE_IN_BYTES				 (32U) /**<Key size of known answer test in bytes*/
#define XSECURE_KAT_MSG_LEN_IN_WORDS				 (8U) /**<Message length of known answer test in words*/
#define XSECURE_KAT_RSA_PUB_EXP						 (0x1000100U) /**<RSA public exponent of known answer test*/
#define XSECURE_KAT_ECC_P521_SHA3_HASH_SIZE_IN_BYTES (66U) /**<Hash size of ECC P521 and SHA3 in bytes*/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
u8* XSecure_GetKatMessage(void);
u8* XSecure_GetKatAesKey(void);
u8* XSecure_GetKatSha3ExpHash(void);
#ifndef PLM_RSA_EXCLUDE
u32* XSecure_GetKatRsaModulus(void);
u32* XSecure_GetKatRsaModExt(void);
u32* XSecure_GetKatRsaData(void);
u32* XSecure_GetKatRsaCtData(void);
u32* XSecure_GetKatRsaPrivateExp(void);
int XSecure_RsaPublicEncryptKat(void);
#endif

#ifndef PLM_ECDSA_EXCLUDE
XSecure_EllipticKey* XSecure_GetKatEccPublicKey(XSecure_EllipticCrvClass CrvClass);
XSecure_EllipticSign* XSecure_GetKatEccExpSign(XSecure_EllipticCrvClass CrvClass);
u8* XSecure_GetKatEccPrivateKey(XSecure_EllipticCrvClass CrvClass);
u8* XSecure_GetKatEccEphemeralKey(XSecure_EllipticCrvTyp CrvType);
int XSecure_EllipticVerifySignKat(XSecure_EllipticCrvClass CrvClass);
int XSecure_EllipticSignGenerateKat(XSecure_EllipticCrvClass CrvClass);
int XSecure_EllipticPwct(XSecure_EllipticCrvTyp Curvetype, u64 DAddr, XSecure_EllipticKeyAddr *PubKeyAddr);
#endif
int XSecure_AesDecryptCmKat(const XSecure_Aes *AesInstance);
int XSecure_AesDecryptKat(XSecure_Aes *AesInstance);
int XSecure_AesEncryptKat(XSecure_Aes *AesInstance);
int XSecure_Sha3Kat(XSecure_Sha3 *SecureSha3);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_KAT_H_ */
/** @} */
