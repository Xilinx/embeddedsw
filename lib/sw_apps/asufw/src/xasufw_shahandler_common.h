/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_shahandler_common.h
 *
 * This file contains common function declarations for SHA2 and SHA3 handler modules
 * in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  04/01/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_SHAHANDLER_COMMON_H_
#define XASUFW_SHAHANDLER_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_sharedmem.h"
#include "xasufw_modules.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/
/**
 * This structure contains the SHA common context.
 */
typedef struct {
	XAsufw_Module Module;	/**< Module instance */
	u32 CmdStage;		/**< Command stage */
} XAsufw_ShaContext;

/*************************** Macros (Inline Functions) Definitions *******************************/
/** This macro derives the containing XAsufw_ShaContext from a member pointer. */
#define XAsufw_GetShaContext(Item, Type, Member)    \
	((Type *)((char *)(Item) - offsetof(Type, Member)))

/************************************ Function Prototypes ****************************************/
s32 XAsufw_ShaOperation(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
s32 XAsufw_ShaKatOperation(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_SHAHANDLER_COMMON_H_ */
/** @} */
