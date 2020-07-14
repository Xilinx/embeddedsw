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
* 1.01  ma   08/01/2019 Removed LPD module init related code from PLM app
*       kc   08/29/2019 Added xilpm hook to be called after plm cdo
* 1.02  kc   02/19/2020 Moved PLM banner print to XilPlmi
*       kc   03/23/2020 Minor code cleanup
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

int XPlm_HookBeforePlmCdo(void);
int XPlm_HookAfterPlmCdo(void);
int XPlm_HookAfterBootPdi(void);

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_HOOK_H */
