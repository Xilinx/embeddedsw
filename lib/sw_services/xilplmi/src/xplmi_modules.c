/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
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
XPlmi_Module * Modules[XPLMI_MAX_MODULES];

/*****************************************************************************/
/**
 * @brief	This function registers the module passed to Modules variable.
 *
 * @param	Module is pointer to XPlmi Module
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_ModuleRegister(XPlmi_Module *Module)
{
	u32 ModuleId = Module->Id;

	Xil_AssertVoid(ModuleId < XPLMI_MAX_MODULES);
	Xil_AssertVoid(Modules[ModuleId] == NULL);
	Modules[ModuleId] = Module;
}
