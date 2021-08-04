/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_modules.h
*
* This is the header file which contains definitions for the modules
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/20/2018 Initial release
* 1.01  bsv  04/04/2020 Code clean up
* 1.02  rama 08/12/2020 Added STL module ID
*       bm   10/14/2020 Code clean up
* 1.03  bm   02/17/2021 Added const to CmdAry
*       ma   03/04/2021 Added CheckIpiAccessHandler handler to XPlmi_Module
*       rama 03/22/2021 Added STL module ID to support STL execution
*       kal  03/30/2021 Added XilSecure module ID
* 1.04  bsv  07/16/2021 Fix doxygen warnings
*       kal  07/17/2021 Added XilNvm module ID
*       bsv  08/02/2021 Removed incorrect comment
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_MODULES_H
#define XPLMI_MODULES_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
#define XPLMI_MAX_MODULES			(12U)
#define XPLMI_MODULE_GENERIC_ID			(1U)
#define XPLMI_MODULE_XILPM_ID			(2U)
#define XPLMI_MODULE_SEM_ID			(3U)
#define XPLMI_MODULE_XILSECURE_ID		(5U)
#define XPLMI_MODULE_XILPSM_ID			(6U)
#define XPLMI_MODULE_LOADER_ID			(7U)
#define XPLMI_MODULE_ERROR_ID			(8U)
#define XPLMI_MODULE_STL_ID			(10U)
#define XPLMI_MODULE_XILNVM_ID			(11U)
#define XPLMI_MODULE_COMMAND(FUNC)		{ (FUNC) }

/**************************** Type Definitions *******************************/
typedef struct {
	int (*Handler)(XPlmi_Cmd *Cmd);
} XPlmi_ModuleCmd;

typedef struct {
	u32 Id;
	const XPlmi_ModuleCmd *CmdAry;
	u32 CmdCnt;
	int (*CheckIpiAccess)(u32 CmdId, u32 IpiReqType);
} XPlmi_Module;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XPlmi_ModuleRegister(XPlmi_Module *Module);

/************************** Variable Definitions *****************************/
extern XPlmi_Module *Modules[XPLMI_MAX_MODULES];

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_MODULES_H */
