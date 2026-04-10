/**************************************************************************************************
* Copyright (c) 2024 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_modules.c
 *
 * This file contains code for module registration functionality in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   04/18/24 Initial release
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_modules.h"
#include "xil_types.h"
#include "xasufw_status.h"
#include "xasufw_memory.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
/** Mask for command count in access permissions buffer */
#define XASUFW_ACCESS_PERM_CMD_CNT_MASK			0xFFFF0000U
/** Shift for command count in access permissions buffer */
#define XASUFW_ACCESS_PERM_CMD_CNT_SHIFT		16U
/** Mask for module ID in access permissions buffer */
#define XASUFW_ACCESS_PERM_MODULE_ID_MASK		0x0000FFFFU

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
XAsufw_Module *Modules[XASU_MAX_MODULES]; /**< Array for Modules structures */

/*************************************************************************************************/
/**
 * @brief	This function registers the given module for ASUFW.
 *
 * @param	Module	Pointer to the XAsufw_Module structure.
 *
 * @return
 * 	- XASUFW_SUCCESS, if module registration is successful.
 * 	- XASUFW_FAILURE, if module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_ModuleRegister(XAsufw_Module *Module)
{
	s32 Status = XASUFW_FAILURE;
	u32 ModuleId = Module->Id;

	/**
	 * If the Module ID is within the supported range and the module is not already registered,
	 * register the module.
	 */
	if ((ModuleId < XASU_MAX_MODULES) && (Modules[ModuleId] == NULL)) {
		Modules[ModuleId] = Module;
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the Module pointer if registered.
 *
 * @param	ModuleId	Registered ID of the module
 *
 * @return
 * 	- Returns module pointer if registered.
 * 	- Otherwise, it returns NULL.
 *
 *************************************************************************************************/
XAsufw_Module *XAsufw_GetModule(u32 ModuleId)
{
	XAsufw_Module *Module = NULL;

	if (ModuleId < XASU_MAX_MODULES) {
		Module = Modules[ModuleId];
	}

	return Module;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the IPI access permission buffer for a module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if module permissions are updated successfully.
 * 	- XASUFW_UPDATE_ACCESS_PERM_INVALID_MODULE_INFO, if module information is invalid.
 * 	- XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, if access permissions are for invalid module.
 * 	- XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED, if command count is invalid.
 * 	- XASUFW_ERR_NO_ACCESS_PERMISSIONS, if access permissions buffer is not registered by module.
 * 	- XASUFW_ERR_UPDATE_ACCESS_PERM_FAILED, if updating access permissions fails.
 *
 *************************************************************************************************/
s32 XAsufw_UpdateAccessPermissions(void)
{
	s32 Status = XASUFW_FAILURE;
	XFih_Var FihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 CmdCnt;
	u32 ModuleId;
	const u32 *CmdPermBuf = (const u32 *)(UINTPTR)(XASUFW_RTCA_ACCESS_PERM_ADDR);
	const XAsufw_Module *Module;

	/** Check and update the access permissions for the provided modules. */
	while (*CmdPermBuf != 0U) {
		/** Get the command count and module ID from the access permissions buffer in RTCA. */
		CmdCnt = (*CmdPermBuf & XASUFW_ACCESS_PERM_CMD_CNT_MASK) >> XASUFW_ACCESS_PERM_CMD_CNT_SHIFT;
		ModuleId = *CmdPermBuf & XASUFW_ACCESS_PERM_MODULE_ID_MASK;
		CmdPermBuf++;

		/**
		 * - If the module ID is greater than maximum supported modules or command count is zero,
		 * then the module information is invalid.
		 */
		if ((ModuleId >= XASU_MAX_MODULES) || (CmdCnt == 0U)) {
			Status = XASUFW_UPDATE_ACCESS_PERM_INVALID_MODULE_INFO;
			goto END;
		}

		/** - Get the module from the module ID received in the access permissions buffer in RTCA. */
		Module = XAsufw_GetModule(ModuleId);
		if (Module == NULL) {
			Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
			goto END;
		}

		/** - Check if command count is greater than module's command count. */
		if (CmdCnt > Module->CmdCnt) {
			Status = XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED;
			goto END;
		}

		/** - Check if access permissions buffer is not registered by module. */
		if (Module->AccessPermBufferPtr == NULL) {
			Status = XASUFW_ERR_NO_ACCESS_PERMISSIONS;
			goto END;
		}

		/** - Copy the access permissions buffer to the module's access permissions buffer. */
		XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Xil_SMemCpy,
						   XASUFW_ERR_UPDATE_ACCESS_PERM_FAILED,
						   FihVar, Status, END, Module->AccessPermBufferPtr,
						   Module->CmdCnt * sizeof(XAsufw_AccessPerm_t),
						   CmdPermBuf, CmdCnt * sizeof(XAsufw_AccessPerm_t),
						   CmdCnt * sizeof(XAsufw_AccessPerm_t));

		CmdPermBuf += CmdCnt;
	}

END:
	return Status;
}

/** @} */
