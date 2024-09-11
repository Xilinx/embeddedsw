/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_aes.h
 * @addtogroup Overview
 * @{
 * This file Contains the AES client function prototypes, defines and macros for the AES module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   08/01/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASU_AES_H
#define XASU_AES_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_client.h"
#include "xasu_aesinfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_AesEncrypt(XAsu_ClientParams *ClientParamsPtr, Asu_AesParams *AesParamsPtr);
s32 XAsu_AesDecrypt(XAsu_ClientParams *ClientParamsPtr, Asu_AesParams *AesParamsPtr);
s32 XAsu_AesKat(void);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_AES_H */
/** @} */