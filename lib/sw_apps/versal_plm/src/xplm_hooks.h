/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplm_hooks.h
*
* This file contains the declarations for the hooks provided in PLM
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/08/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_HOOK_H
#define XPLM_HOOK_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/
#include "xplm_default.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

int XPlm_HookBeforePlmCdo();
int XPlm_HookAfterPlmCdo();
int XPlm_HookAfterBootPdi();

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_HOOK_H */
