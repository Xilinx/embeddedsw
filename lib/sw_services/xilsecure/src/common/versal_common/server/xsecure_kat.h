/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_kat.h
* @addtogroup xsecure_kat_common_apis XilSecure KAT common APIs
* @{
* @cond xsecure_internal
*
* This file contains KAT interface APIs for versal and versalnet
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 5.0   kpt  07/15/2022 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSECURE_KAT_H_
#define XSECURE_KAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_aes.h"
#include "xsecure_sha.h"
#include "xsecure_elliptic.h"

/************************** Constant Definitions *****************************/

#define XSECURE_KAT_MSG_LEN_IN_BYTES		(32U)
#define XSECURE_KAT_AAD_SIZE_IN_BYTES		(16U)
#define XSECURE_KAT_IV_SIZE_IN_BYTES		(16U)
#define XSECURE_KAT_KEY_SIZE_IN_BYTES		(32U)
#define XSECURE_KAT_MSG_LEN_IN_WORDS		(8U)

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
u8* XSecure_GetKatMessage(void);
u8* XSecure_GetKatAesKey(void);
u8* XSecure_GetKatAesIv(void);
u8* XSecure_GetKatAesCt(void);
u8* XSecure_GetKatAesGcmTag(void);
u8* XSecure_GetKatAesAad(void);
u8* XSecure_GetKatSha3ExpHash(void);
u8* XSecure_GetKatEccSha3ExpHash(void);
u32* XSecure_GetKatRsaModulus(void);
u32 XSecure_GetKatRsaPubExponent(void);
u32* XSecure_GetKatRsaModExt(void);
u32* XSecure_GetKatRsaData(void);
u32* XSecure_GetKatRsaExpCt(void);
XSecure_EllipticKey* XSecure_GetKatEccPublicKey(XSecure_EllipticCrvClass CrvClass);
XSecure_EllipticSign* XSecure_GetKatEccExpSign(XSecure_EllipticCrvClass CrvClass);
u8* XSecure_GetKatEccPrivateKey(XSecure_EllipticCrvClass CrvClass);
u8* XSecure_GetKatEccEphimeralKey(XSecure_EllipticCrvTyp Curvetype);
int XSecure_AesDecryptKat(XSecure_Aes *AesInstance);
int XSecure_Sha3Kat(XSecure_Sha3 *SecureSha3);
int XSecure_RsaPublicEncryptKat(void);
int XSecure_EllipticVerifySignKat(XSecure_EllipticCrvClass CrvClass);
int XSecure_EllipticSignGenerateKat(XSecure_EllipticCrvClass CrvClass);
int XSecure_EllipticPwct(XSecure_EllipticCrvTyp Curvetype, u8 *D, XSecure_EllipticKey *PubKey);
int XSecure_AesEncryptKat(XSecure_Aes *AesInstance);
int XSecure_RsaPrivateDecryptKat(void);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_KAT_H_ *//* @} */
