/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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

/************************** Constant Definitions *****************************/
#define XPLMI_MAX_MODULES			(11U)

/* TODO add enum for declaring module ids */
#define XPLMI_MODULE_GENERIC_ID			(1U)
#define XPLMI_MODULE_XILPM_ID			(2U)
#define XPLMI_MODULE_SEM_ID			(3U)
#define XPLMI_MODULE_XILPSM_ID			(6U)
#define XPLMI_MODULE_LOADER_ID			(7U)
#define XPLMI_MODULE_ERROR_ID			(8U)

#define XPLMI_MODULE_COMMAND(FUNC)		{ (FUNC) }

/**************************** Type Definitions *******************************/
typedef struct {
	int (*Handler)(XPlmi_Cmd *Cmd);
} XPlmi_ModuleCmd;

typedef struct {
	u32 Id;
	XPlmi_ModuleCmd *CmdAry;
	u32 CmdCnt;
} XPlmi_Module;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XPlmi_ModuleRegister(XPlmi_Module *Module);

/************************** Variable Definitions *****************************/
extern XPlmi_Module *Modules[XPLMI_MAX_MODULES];

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_MODULES_H */
