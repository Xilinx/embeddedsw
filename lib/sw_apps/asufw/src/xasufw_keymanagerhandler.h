/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_keymanagerhandler.h
*
* This file contains declarations for xasufw_keymanagerhandler.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   11/25/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/

#ifndef XASUFW_KEYMANAGER_H_
#define XASUFW_KEYMANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************ Variable Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_KeyManagerInit(void);
u32 XKeyManager_IsAsuVaultCreated(void);

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_KEYMANAGERHANDLER_H_ */
/** @} */
