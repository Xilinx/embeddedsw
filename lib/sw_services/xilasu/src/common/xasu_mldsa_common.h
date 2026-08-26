/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_mldsa_common.h
 *
 * This file contains the ML-DSA function prototypes which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/21/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_mldsa_common_apis ML-DSA Common APIs
 * @{
*/

#ifndef XASU_MLDSA_COMMON_H_
#define XASU_MLDSA_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_mldsainfo.h"

/*********************************** Constant Definitions ****************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_MldsaValidateParams(const XAsu_MldsaSignVerifyParams *MldsaParamsPtr);
s32 XAsu_MldsaValidateSignDerandParams(const XAsu_MldsaSignDerandParams *MldsaParamsPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_MLDSA_COMMON_H_ */
/** @} */
