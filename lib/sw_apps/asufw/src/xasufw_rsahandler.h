/**************************************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xasufw_rsahandler.h
* This file contains declarations for xasufw_rsahandler.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   08/20/24 Initial Release
*       ss   09/26/24 Fixed doxygen comments
*
* </pre>
*
**************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/

#ifndef XASUFW_RSAHANDLER_H_
#define XASUFW_RSAHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
#ifdef XASU_RSA_ECC_ENABLE
/* RSA Command Structure Initialization function */
s32 XAsufw_RsaInit(void);

#ifdef XASU_KEYMANAGER_ENABLE
/* Add RSA key pair generation task to the scheduler */
s32 XAsufw_CreateRsaKeyPairGenTask(void);
#endif /* XASU_KEYMANAGER_ENABLE */
#endif /* XASU_RSA_ECC_ENABLE */

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_RSAHANDLER_H_ */
/** @} */
