/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_slhdsahandler.h
*
* This file contains declarations for xasufw_slhdsahandler.c.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   07/10/26 Initial release
*
* </pre>
*
*************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/

#ifndef XASUFW_SLHDSAHANDLER_H_
#define XASUFW_SLHDSAHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_resourcemanager.h"

#ifdef XASU_SLHDSA_ENABLE
/************************************ Function Prototypes ****************************************/
s32 XAsufw_SlhdsaInit(void);
XAsufw_Resource XAsufw_GetSlhdsaShaMaskResourceId(const XAsu_ReqBuf *ReqBuf);
#endif /* XASU_SLHDSA_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_SLHDSAHANDLER_H_ */
/** @} */
