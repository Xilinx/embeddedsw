/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_modules.c
 * @addtogroup Overview
 * @{
 *
 * This file contains code for module registration functionality in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   04/18/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasufw_modules.h"
#include "xil_types.h"
#include "xasufw_status.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
XAsufw_Module *Modules[XASUFW_MAX_MODULES]; /**< Array for Modules structures */

/*************************************************************************************************/
/**
 * @brief   This function registers the module passed to Modules variable.
 *
 * @param	Module  Pointer to XAsufw_Module
 *
 * @return
 * 			- On successful module registration, it returns XASUFW_SUCCESS.
 *            Otherwise, it returns XASUFW_MODULE_REGISTRATION_FAILED
 *
 *************************************************************************************************/
s32 XAsufw_ModuleRegister(XAsufw_Module *Module)
{
	s32 Status = XASUFW_FAILURE;
	u32 ModuleId = Module->Id;

	if ((ModuleId >= XASUFW_MAX_MODULES) || (Modules[ModuleId] != NULL)) {
		Status = XASUFW_MODULE_REGISTRATION_FAILED;
		XFIH_GOTO(END);
	}

	Modules[ModuleId] = Module;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function returns the Module pointer if registered.
 *
 * @param	ModuleId	Registered ID of the module
 *
 * @return
 * 			- Returns module pointer if registered. Otherwise, it returns NULL.
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
