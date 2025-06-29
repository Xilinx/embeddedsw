/**************************************************************************************************
* Copyright (c) 2024 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
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
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_modules.h"
#include "xil_types.h"
#include "xasufw_status.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
XAsufw_Module *Modules[XASUFW_MAX_MODULES]; /**< Array for Modules structures */

/*************************************************************************************************/
/**
 * @brief	This function registers the given module for ASUFW.
 *
 * @param	Module	Pointer to the XAsufw_Module structure.
 *
 * @return
 * 	- XASUFW_SUCCESS, if module registration is successful.
 * 	- XASUFW_MODULE_REGISTRATION_FAILED, if module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_ModuleRegister(XAsufw_Module *Module)
{
	s32 Status = XASUFW_FAILURE;
	u32 ModuleId = Module->Id;

	/**
	 * If the Module ID is greater than maximum supported modules or the module is already registered,
	 * return failure. Otherwise, register the module.
	 */
	if ((ModuleId >= XASUFW_MAX_MODULES) || (Modules[ModuleId] != NULL)) {
		Status = XASUFW_MODULE_REGISTRATION_FAILED;
		goto END;
	}

	Modules[ModuleId] = Module;
	Status = XASUFW_SUCCESS;

END:
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

	if (ModuleId < XASUFW_MAX_MODULES) {
		Module = Modules[ModuleId];
	}

	return Module;
}
/** @} */
