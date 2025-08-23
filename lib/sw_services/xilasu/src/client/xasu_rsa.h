/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_rsa.h
 *
 * This file Contains the RSA client function prototypes, defines and macros for
 * the RSA hardware module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/24 Initial release
 *       ss   09/26/24 Fixed doxygen comments
 *       ss   02/04/25 Added client API's for RSA padding scheme
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_rsa_client_apis RSA Client APIs
 * @{
*/
#ifndef XASU_RSA_H_
#define XASU_RSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_rsainfo.h"
#include "xasu_client.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_RsaEnc(XAsu_ClientParams *ClientParamPtr, XAsu_RsaParams *RsaClientParamPtr);
s32 XAsu_RsaDec(XAsu_ClientParams *ClientParamPtr, XAsu_RsaParams *RsaClientParamPtr);
s32 XAsu_RsaCrtDec(XAsu_ClientParams *ClientParamPtr, XAsu_RsaParams *RsaClientParamPtr);
s32 XAsu_RsaOaepEnc(XAsu_ClientParams *ClientParamPtr,
	XAsu_RsaOaepPaddingParams *RsaClientParamPtr);
s32 XAsu_RsaOaepDec(XAsu_ClientParams *ClientParamPtr,
	XAsu_RsaOaepPaddingParams *RsaClientParamPtr);
s32 XAsu_RsaPssSignGen(XAsu_ClientParams *ClientParamPtr,
	XAsu_RsaPaddingParams *RsaClientParamPtr);
s32 XAsu_RsaPssSignVer(XAsu_ClientParams *ClientParamPtr,
	XAsu_RsaPaddingParams *RsaClientParamPtr);
s32 XAsu_RsaOaepEncDecKat(XAsu_ClientParams *ClientParamPtr);
s32 XAsu_RsaPssSignGenAndVerKat(XAsu_ClientParams *ClientParamPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_RSA_H_ */
/** @} */
