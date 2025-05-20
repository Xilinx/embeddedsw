/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_keywraphandler.h
*
* This file contains declarations for xasufw_keywraphandler.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   02/24/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/

#ifndef XASUFW_KEYWRAPHANDLER_H_
#define XASUFW_KEYWRAPHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

#ifdef XASU_KEYWRAP_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************ Variable Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_KeyWrapInit(void);
#endif /* XASU_KEYWRAP_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_KEYWRAPHANDLER_H_ */
/** @} */
