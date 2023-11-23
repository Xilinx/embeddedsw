/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_modules.c
*
* This is the file which contains module registration code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/23/2018 Initial release
* 1.01  bsv  04/04/2020 Code clean up
* 1.02  bm   10/14/2020 Code clean up
* 1.03  td   07/08/2021 Fix doxygen warnings
* 2.0   ng   11/11/2023 Implemented user modules
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_modules.h"
#include "xil_assert.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
XPlmi_Module * Modules[XPLMI_ALL_MODULES_MAX];

/*****************************************************************************/
/**
 * @brief	This function registers the module.
 *
 * @param	Module is pointer to XPlmi Module
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_ModuleRegister(XPlmi_Module *Module)
{
	u32 ModuleId = Module->Id;

#if ( XPAR_MAX_USER_MODULES > 0U )
	/* Update the Module ID if it's user module. */
	if ( ModuleId < XPLMI_USER_MODULE_MASK ) {
		Xil_AssertVoid(ModuleId < XPLMI_MAX_MODULES);
	}
	else {
		ModuleId = ModuleId - XPLMI_USER_MODULE_MASK + XPLMI_MAX_MODULES;
		Xil_AssertVoid(ModuleId < XPLMI_ALL_MODULES_MAX);
	}
#else
	Xil_AssertVoid(ModuleId < XPLMI_MAX_MODULES);
#endif

	Xil_AssertVoid(Modules[ModuleId] == NULL);
	Modules[ModuleId] = Module;
}

/*****************************************************************************/
/**
 * @brief	This function returns the Module pointer if registered.
 *
 * @param	ModuleId	Registered ID of the module.
 *
 * @return
 * 			- Module ptr if module is registered else NULL.
 *
 *****************************************************************************/
XPlmi_Module* XPlmi_GetModule(u32 ModuleId)
{
	XPlmi_Module *Module = NULL;
	u32 UserModuleId;

	if ( ModuleId < XPLMI_USER_MODULE_MASK ) {
		if ( ModuleId < XPLMI_MAX_MODULES ) {
			Module = Modules[ModuleId];
		}
	}
	else {
		UserModuleId = ModuleId - XPLMI_USER_MODULE_MASK + XPLMI_MAX_MODULES;
		if ( UserModuleId < XPLMI_ALL_MODULES_MAX ) {
			Module = Modules[UserModuleId];
		}
	}

	return Module;
}
