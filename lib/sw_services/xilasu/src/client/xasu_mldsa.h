/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_mldsa.h
 *
 * This file contains the ML-DSA (FIPS 204) client function prototypes for the ASU ML-DSA module.
 * The client APIs marshal the request parameters into an IPI command and forward it to ASUFW;
 * the actual signing and verification are performed by the ASUFW ML-DSA module. Key pair
 * generation is not an ML-DSA module command: it is requested through the key manager, using
 * XAsu_KmGenerateMldsaKeyPair.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_mldsa_client_apis ML-DSA Client APIs
 * @{
*/
#ifndef XASU_MLDSA_H_
#define XASU_MLDSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_mldsainfo.h"
#include "xasu_client.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
/* Key pair generation lives in the key manager: see XAsu_KmGenerateMldsaKeyPair. */
s32 XAsu_MldsaSign(XAsu_ClientParams *ClientParamPtr, XAsu_MldsaSignVerifyParams *MldsaParamPtr);
s32 XAsu_MldsaVerify(XAsu_ClientParams *ClientParamPtr, XAsu_MldsaSignVerifyParams *MldsaParamPtr);
s32 XAsu_MldsaKat(XAsu_ClientParams *ClientParamPtr);
/*
 * Signing with a caller-supplied rnd, served only by an ASUFW built with CAVP support; see
 * XASU_MLDSA_SIGN_DERAND_CMD_ID. A production ASUFW rejects the command.
 */
s32 XAsu_MldsaSignDerand(XAsu_ClientParams *ClientParamPtr,
			 XAsu_MldsaSignDerandParams *MldsaParamPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_MLDSA_H_ */
/** @} */
