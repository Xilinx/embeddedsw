/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_kdf.h
 *
 * This file Contains the KDF client function prototypes.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/21/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_kdf_client_apis KDF Client APIs
 * @{
*/

#ifndef XASU_KDF_H_
#define XASU_KDF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasu_client.h"
#include "xasu_kdfinfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_KdfKat(XAsu_ClientParams *ClientParamsPtr);
s32 XAsu_KdfGenerate(XAsu_ClientParams *ClientParamsPtr, XAsu_KdfParams *KdfParamsPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KDF_H_ */
/** @} */
