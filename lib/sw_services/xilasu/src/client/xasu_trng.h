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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_trng_client_apis TRNG Client APIs
 * @{
*/
#ifndef XASU_TRNG_H
#define XASU_TRNG_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_client.h"

/************************************ Constant Definitions ***************************************/

#define XASU_TRNG_RANDOM_NUM_IN_BYTES			32U /**< Security strength in Bytes */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_TrngGetRandomNum(XAsu_ClientParams *ClientParamPtr, u8 *RandomBuf, u32 Length);
s32 XAsu_TrngKat(void);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_TRNG_H */
/** @} */
