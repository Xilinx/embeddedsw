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
 *       yog  03/24/25 Added XRsa_EccGeneratePrivKey() prototype
 *       yog  07/11/25 Removed curve type macros.
 *       rmv  07/16/25 Update function prototype of XRsa_EccGeneratePvtKey() for ECC private key
 *                     generation
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

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XRSA_ECC_P521_RANDBUFLEN_IN_BYTES	(0x4C) /**< Length of the random number to be
							generated for NIST P-521 in bytes */
#define XRSA_ECC_KEY_PAIR_GEN_EXTRA_BYTES	(0xCU) /**< Extra bytes count used for key pair
							generation */
#define XRSA_ECC_RAND_NUM_GEN_EXTRA_BYTES	(0x8U) /**< Extra bytes count for random number
							generation */

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
s32 XRsa_EccGeneratePvtKey(u32 CurveType, u32 CurveLen, u8 *PvtKey, u8 *InputRandBuf,
			   u32 InputRandBufLen);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XRSA_ECC_H_ */
/** @} */
