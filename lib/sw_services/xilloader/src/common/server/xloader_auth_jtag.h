/******************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_auth_jtag.h
*
* This is the header file which contains JTAG interface declarations
* for the xilloader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.00  har  09/23/2025 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XLOADER_AUTH_JTAG_H
#define XLOADER_AUTH_JTAG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_AUTH_JTAG
#include "xloader_auth_enc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XLoader_AddAuthJtagToScheduler(void);
int XLoader_CheckAuthJtagIntStatus(void *Arg);

#endif	/**< PLM_AUTH_JTAG */

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_AUTH_JTAG_H */
