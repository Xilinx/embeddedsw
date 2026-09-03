/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_slhdsa.h
 *
 * This file contains the SLH-DSA (FIPS 205) client function prototypes for the ASU SLH-DSA
 * module. The client APIs marshal the request parameters into an IPI command and forward it to
 * ASUFW; the actual signing and verification are performed by the ASUFW SLH-DSA module. Key pair
 * generation is not an SLH-DSA module command: it is requested through the key manager, using
 * XAsu_KmGenerateSlhdsaKeyPair.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   kk   07/10/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_slhdsa_client_apis SLH-DSA Client APIs
 * @{
*/
#ifndef XASU_SLHDSA_H_
#define XASU_SLHDSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_client.h"
#include "xasu_slhdsainfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
/* Key pair generation lives in the key manager: see XAsu_KmGenerateSlhdsaKeyPair. */
s32 XAsu_SlhdsaSign(XAsu_ClientParams *ClientParamPtr, XAsu_SlhdsaSignParams *SlhdsaParamPtr);
s32 XAsu_SlhdsaSignVerify(XAsu_ClientParams *ClientParamPtr,
			  XAsu_SlhdsaSignVerifyParams *SlhdsaParamPtr);
s32 XAsu_SlhdsaKat(XAsu_ClientParams *ClientParamPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_SLHDSA_H_ */
/** @} */
