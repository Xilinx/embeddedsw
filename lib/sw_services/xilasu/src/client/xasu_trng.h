/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_trng.h
 *
 * This file Contains the TRNG client function prototypes, defines and macros for
 * the ASU TRNG hardware module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  08/23/24 Initial release
 *       yog  09/26/24 Added doxygen groupings.
 * 1.1   ma   02/07/25 Added DRBG support in client
 *       kd   07/23/25 Fixed gcc warnings
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_trng_client_apis TRNG Client APIs
 * @{
*/
#ifndef XASU_TRNG_H_
#define XASU_TRNG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_client.h"
#include "xasu_trnginfo.h"

/************************************ Constant Definitions ***************************************/

#define XASU_TRNG_RANDOM_NUM_IN_BYTES			32U /**< Security strength in Bytes */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_TrngGetRandomNum(XAsu_ClientParams *ClientParamPtr, u8 *BufPtr, u32 Length);
s32 XAsu_TrngKat(XAsu_ClientParams *ClientParamPtr);

#ifdef XASU_TRNG_ENABLE_DRBG_MODE
s32 XAsu_TrngDrbgInstantiate(XAsu_ClientParams *ClientParamPtr,
        XAsu_DrbgInstantiateCmd *CmdParamsPtr);
s32 XAsu_TrngDrbgReseed(XAsu_ClientParams *ClientParamPtr, XAsu_DrbgReseedCmd *CmdParamsPtr);
s32 XAsu_TrngDrbgGenerate(XAsu_ClientParams *ClientParamPtr, XAsu_DrbgGenerateCmd *CmdParamsPtr);
#endif /* XASU_TRNG_ENABLE_DRBG_MODE */
/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_TRNG_H_ */
/** @} */
