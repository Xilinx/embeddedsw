/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       kp   02/26/26 Added client-side KDF SHA3-256 KAT prototype
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
s32 XAsu_KdfSha3Kat(void);
s32 XAsu_CmacKdfGenerate(XAsu_ClientParams *ClientParamsPtr, XAsu_CmacKdfParams *CmacKdfParamsPtr);
s32 XAsu_HKdfGenerate(XAsu_ClientParams *ClientParamsPtr, XAsu_HkdfParams *HkdfParamsPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KDF_H_ */
/** @} */
