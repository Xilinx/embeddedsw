/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xrsa_ecc.h
 * This file contains implementation of the interface functions for RSA hardware engine.
 * In ASU, since there are two cores ECC(supports NIST curves P-256 & P-384) and
 * RSA(supports all other curves) which supports ECDSA, the functions and macros related to the
 * RSA core are named rsa_ecc.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------------------------------------
 * 1.0   yog  07/11/24 Initial release
 *       yog  08/19/24 Received Dma instance from handler
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       ss   12/02/24 Added support for ECDH
 *       yog  02/21/25 Added XRsa_EccValidateAndGetCrvInfo() prototype
 *       yog  03/21/25 Added PWCT support
 *
 * </pre>
 *
 **************************************************************************************************/
/**
* @addtogroup xrsa_ecc_server_apis RSA ECC Server APIs
* @{
*/

#ifndef XRSA_ECC_H_
#define XRSA_ECC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasufw_sss.h"
#include "xasufw_dma.h"
#include "xrsa_eccinfo.h"

/************************************ Constant Definitions ***************************************/
#define XRSA_ECC_CURVE_TYPE_NIST_P521		(0x5U) /**< Curve type value for P-521 curve*/
#define XRSA_ECC_CURVE_TYPE_NIST_P192		(0x1U) /**< Curve type value for P-192 curve*/
#define XRSA_ECC_CURVE_TYPE_NIST_P224		(0x2U) /**< Curve type value for P-224 curve*/
#define XRSA_ECC_CURVE_TYPE_NIST_P256		(0x3U) /**<  Curve type value for P-256 curve*/
#define XRSA_ECC_CURVE_TYPE_NIST_P384		(0x4U) /**<  Curve type value for P-384 curve*/
#define XRSA_ECC_CURVE_TYPE_BRAINPOOL_P256	(0x15U) /**< Curve type for brainpool P-256 curve */
#define XRSA_ECC_CURVE_TYPE_BRAINPOOL_P320	(0x16U) /**< Curve type for brainpool P-320 curve */
#define XRSA_ECC_CURVE_TYPE_BRAINPOOL_P384	(0x17U) /**< Curve type for brainpool P-384 curve */
#define XRSA_ECC_CURVE_TYPE_BRAINPOOL_P512	(0x18U) /**< Curve type for brainpool P-512 curve */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XRsa_EccGeneratePubKey(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
			   u64 PubKeyAddr);
s32 XRsa_EccValidatePubKey(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PubKeyAddr);
s32 XRsa_EccGenerateSignature(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
			      const u8 *EphemeralKeyPtr, u64 HashAddr, u32 HashBufLen, u64 SignAddr);
s32 XRsa_EccVerifySignature(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PubKeyAddr,
			    u64 HashAddr, u32 HashBufLen, u64 SignAddr);
s32 XRsa_EcdhGenSharedSecret(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
			    u64 PubKeyAddr, u64 SharedSecretAddr, u64 SharedSecretObjIdAddr);
EcdsaCrvInfo *XRsa_EccGetCrvData(u32 CurveType);
u32 XRsa_EccValidateAndGetCrvInfo(u32 CurveType, EcdsaCrvInfo **Crv);
s32 XRsa_EccPwct(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
	u64 PubKeyAddr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XRSA_ECC_H_ */
/** @} */
