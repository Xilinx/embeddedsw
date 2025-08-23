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
* @addtogroup xasufw_application ASUFW Server Functionality
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
#define XASUFW_MODULE_COMMAND(FUNC)		{ (FUNC) } /**< Module command define */
/* Macros of IPI Access Permissions */
#define XASUFW_NO_IPI_ACCESS			(0x0U) /**< No IPI access */
#define XASUFW_SECURE_IPI_ACCESS		(0x1U) /**< Secure IPI access */
#define XASUFW_NON_SECURE_IPI_ACCESS	(0x2U) /**< Non-secure IPI access */
#define XASUFW_FULL_IPI_ACCESS			(0x3U) /**< Full IPI access */
#define XASUFW_ACCESS_PERM_MASK			(0x3U) /**< Access permission mask */
#define XASUFW_ACCESS_PERM_SHIFT		(0x2U) /**< Access permission shift */

/** Macro for calculating access permissions IPI mask based on the given mask. */
#define XASUFW_GET_ALL_IPI_MASK(Mask)	((Mask) | ((Mask) << 2U) | \
					((Mask) << 4U) | ((Mask) << 6U) | \
					((Mask) << 8U) | ((Mask) << 10U) | \
					((Mask) << 12U) | ((Mask) << 14U))

/** Macro for all IPI channel's access permissions with no access for a given command. */
#define XASUFW_ALL_IPI_NO_ACCESS(CmdId)		XASUFW_GET_ALL_IPI_MASK(XASUFW_NO_IPI_ACCESS)
/** Macro for all IPI channel's access permissions with secure access for a given command. */
#define XASUFW_ALL_IPI_SECURE_ACCESS(CmdId)	XASUFW_GET_ALL_IPI_MASK(XASUFW_SECURE_IPI_ACCESS)
/** Macro for all IPI channel's access permissions with non-secure access for a given command. */
#define XASUFW_ALL_IPI_NON_SECURE_ACCESS(CmdId)	XASUFW_GET_ALL_IPI_MASK(XASUFW_NON_SECURE_IPI_ACCESS)
/** Macro for all IPI channel's access permissions with full access for a given command. */
#define XASUFW_ALL_IPI_FULL_ACCESS(CmdId)	XASUFW_GET_ALL_IPI_MASK(XASUFW_FULL_IPI_ACCESS)

/************************************** Type Definitions *****************************************/
typedef s32 (*XAsufw_ResourceHandler_t)(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

typedef u16 XAsufw_ResourcesRequired;

/** @} */
/** This structure contains a function pointer to command handler. */
typedef struct {
	s32 (*CmdHandler)(const XAsu_ReqBuf *ReqBuf, u32 ReqId); /**< Command handler */
} XAsufw_ModuleCmd;

typedef u32 XAsufw_AccessPerm_t; /**< Access permission type */

/** This structure contains Module information. */
typedef struct {
	u32 Id; /**< Module ID */
	const XAsufw_ModuleCmd *Cmds; /**< Pointer to module command handlers */
	XAsufw_ResourcesRequired *ResourcesRequired; /**< Pointer to the required resources array */
	u32 CmdCnt; /**< Command count in module */
	XAsufw_ResourceHandler_t ResourceHandler; /**< Function pointer to the resource handler */
	XAsufw_Dma *AsuDmaPtr; /**< Pointer to the DMA instance */
	XSha *ShaPtr; /**< Pointer to the SHA instance */
	XAes *AesPtr; /**< Pointer to the AES instance */
	XAsufw_AccessPerm_t *AccessPermBufferPtr;	 /**< Pointer to the access permissions buffer */
} XAsufw_Module;

/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_ModuleRegister(XAsufw_Module *Module);
XAsufw_Module *XAsufw_GetModule(u32 ModuleId);
s32 XAsufw_UpdateAccessPermissions(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_MODULES_H_ */
/** @} */
