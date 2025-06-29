/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_modules.h
 *
 * This file contains function declarations, macro and structure defines related to modules code
 * in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   04/18/24 Initial release
 *       ma   05/14/24 Add macros for SHA2 and SHA3 module IDs
 *       ma   07/08/24 Add task based approach at queue level
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ma   12/12/24 Updated resource allocation logic
 *       yog  02/24/25 Updated Aes pointer in XAsufw_Module structure.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#ifndef XASUFW_MODULES_H_
#define XASUFW_MODULES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_queuescheduler.h"
#include "xasu_def.h"
#include "xasufw_dma.h"
#include "xsha.h"
#include "xaes.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_MAX_MODULES			    (10U) /**< Maximum supported modules in ASUFW */
#define XASUFW_MODULE_COMMAND(FUNC)		{ (FUNC) } /**< Module command define */

/************************************** Type Definitions *****************************************/
/** @brief This structure contains function pointer to command handler. */
typedef struct {
	s32 (*CmdHandler)(const XAsu_ReqBuf *ReqBuf, u32 ReqId); /**< Command handler */
} XAsufw_ModuleCmd;

typedef s32 (*XAsufw_ResourceHandler_t)(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

typedef u16 XAsufw_ResourcesRequired;

/** @brief This structure contains Module information. */
typedef struct {
	u32 Id; /**< Module ID */
	const XAsufw_ModuleCmd *Cmds; /**< Pointer to module command handlers */
	XAsufw_ResourcesRequired *ResourcesRequired; /**< Pointer to the required resources array */
	u32 CmdCnt; /**< Command count in module */
	XAsufw_ResourceHandler_t ResourceHandler; /**< Function pointer to the resource handler */
	XAsufw_Dma *AsuDmaPtr; /**< Pointer to the DMA instance */
	XSha *ShaPtr; /**< Pointer to the SHA instance */
	XAes *AesPtr; /**< Pointer to the AES instance */
} XAsufw_Module;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_ModuleRegister(XAsufw_Module *Module);
XAsufw_Module *XAsufw_GetModule(u32 ModuleId);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_MODULES_H_ */
/** @} */
