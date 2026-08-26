/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_mldsahandler.h
*
* This file contains declarations for xasufw_mldsahandler.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   08/20/26 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/

#ifndef XASUFW_MLDSAHANDLER_H_
#define XASUFW_MLDSAHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_mldsainfo.h"

#ifdef XASU_MLDSA_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************ Variable Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_MldsaInit(void);
#endif /* XASU_MLDSA_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_MLDSAHANDLER_H_ */
/** @} */
