/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_hooks.h
*
* @addtogroup xplm_apis XPlm Versal APIs
* @{
* @cond xplm_internal
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
*       ana  10/19/2020 Added doxygen comments
* 1.03  bm   01/08/2021 Updated PmcCdo hook function name
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XPLM_HOOK_H
#define XPLM_HOOK_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

int XPlm_HookBeforePmcCdo(void *Arg);
int XPlm_HookAfterPmcCdo(void *Arg);
int XPlm_HookAfterBootPdi(void *Arg);

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_HOOK_H */

/* @} */
