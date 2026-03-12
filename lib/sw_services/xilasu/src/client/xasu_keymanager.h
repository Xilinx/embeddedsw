/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keymanager.h
 *
 * This file Contains the KeyManager client function prototypes, defines and macros for
 * the KeyManager module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   11/25/25 Initial release
 *       yog  01/28/26 Added RSA key pair generation client API.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_keymanager_client_apis KeyManager Client APIs
 * @{
*/
#ifndef XASU_KEYMANAGER_H_
#define XASU_KEYMANAGER_H_
#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_keymanagerinfo.h"
#include "xasu_client.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_KmCreateKeyVault(XAsu_ClientParams *ClientParamPtr,
			XAsu_KeyManagerSubVaultParams *KmVaultParamPtr);
s32 XAsu_KmGenerateAesKey(XAsu_ClientParams *ClientParamPtr,
				XAsu_KeyManagerParams *KmSubVaultParamPtr);
s32 XAsu_KmGenerateAesIv(XAsu_ClientParams *ClientParamPtr,
				XAsu_KeyManagerParams *KmSubVaultParamPtr);
s32 XAsu_KmDeleteKeyVault(XAsu_ClientParams *ClientParamPtr, u32 VaultId);
s32 XAsu_KmDeleteKey(XAsu_ClientParams *ClientParamPtr, u32 KeyId);
s32 XAsu_KmGenerateRsaKeyPair(XAsu_ClientParams *ClientParamPtr,
				XAsu_KeyManagerParams *KmSubVaultParamPtr);

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYMANAGER_H_ */
/** @} */
