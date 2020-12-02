/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_hooks.h
*
* This is the header file which contains definitions for the FSBL hooks
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XFSBL_HOOKS_H
#define XFSBL_HOOKS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

u32 XFsbl_HookBeforeBSDownload(void );

u32 XFsbl_HookAfterBSDownload(void );

u32 XFsbl_HookBeforeHandoff(u32 EarlyHandoff);

u32 XFsbl_HookBeforeFallback(void);

u32 XFsbl_HookPsuInit(void);

u32 XFsbl_HookGetPosBootType(void);

#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_HOOKS_H */
