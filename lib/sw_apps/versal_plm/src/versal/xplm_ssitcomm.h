/**************************************************************************************************
* Copyright (c) 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/


/*************************************************************************************************/
/**
*
* @file versal/xplm_ssitcomm.h
*
* This file contains PLMI ssit secure communication specific declarations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- ---------------------------------------------------------------------------
* 1.00  pre  07/11/2024 Initial release
*       pre  07/30/2024 Fixed misrac and coverity violations
*
* </pre>
*
* @note
*
**************************************************************************************************/

#ifndef XPLM_SSITCOMM_H
#define XPLM_SSITCOMM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *****************************************************/
#include "xplmi_ssit.h"

/************************** Constant Definitions *************************************************/

/**************************** Type Definitions ***************************************************/


/***************** Macros (Inline Functions) Definitions *****************************************/

/************************** Function Prototypes **************************************************/
#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
XPlmi_SsitCommFunctions *XPlm_SsitCommGetFuncsPtr(void);
#endif

/************************** Variable Definitions *************************************************/

#ifdef __cplusplus
}
#endif
#endif  /* XPLM_SSITCOMM_H */
