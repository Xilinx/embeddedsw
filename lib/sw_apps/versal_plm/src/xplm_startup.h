/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplm_startup.h
*
* This file contains the headers for startup code of PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/28/2018 Initial release
* 1.01  ma   08/01/2019 Removed LPD module init related code from PLM app
*       rm   09/08/2019 Adding xilsem library in place of source code
* 1.02  kc   02/26/2020 Added XPLM_SEM macro to include/disable SEM
*						functionality
*       kc   03/23/2020 Minor code cleanup
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_STARTUP_H
#define XPLM_STARTUP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_default.h"
#include "xplm_hooks.h"
#include "xplmi_task.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlm_AddStartUpTasks(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_STARTUP_H */
