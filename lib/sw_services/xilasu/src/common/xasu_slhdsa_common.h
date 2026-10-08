/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xasu_slhdsa_common.h
*
* This file contains the SLH-DSA function prototypes which are common across the client
* and server.
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
* @addtogroup xasu_slhdsa_common_apis SLH-DSA Common APIs
* @{
*/
#ifndef XASU_SLHDSA_COMMON_H_
#define XASU_SLHDSA_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_slhdsainfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
u32 XAsu_SlhdsaGetPkSize(u8 ParamSet);
u32 XAsu_SlhdsaGetSkSize(u8 ParamSet);
u32 XAsu_SlhdsaGetSigSize(u8 ParamSet);
u32 XAsu_SlhdsaGetPreHashDigestLen(u8 HashId);
s32 XAsu_SlhdsaValidateSignParams(const XAsu_SlhdsaSignParams *ParamsPtr);
s32 XAsu_SlhdsaValidateSignVerifyParams(const XAsu_SlhdsaSignVerifyParams *ParamsPtr);

/************************************ Variable Definitions ***************************************/


#ifdef __cplusplus
}
#endif

#endif /* XASU_SLHDSA_COMMON_H_ */
/** @} */
