/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file versal_2vp/server/xplmi_wdt.h
*
* This file contains declarations related to WDT in versal_2vp
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 2.3   tvp  07/07/25 Initial release
*
* </pre>

***************************************************************************************************/

#ifndef XPLMI_WDT_H
#define XPLMI_WDT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xil_types.h"

/************************************ Constant Definitions ****************************************/
/**
 * @{
 * @cond xplmi_internal
 */

#define XPLMI_WDT_EXTERNAL		(0x14104001U)

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/
void XPlmi_SetPlmLiveStatus(void);
void XPlmi_ClearPlmLiveStatus(void);
/**
 * @}
 * @endcond
 */
int XPlmi_EnableWdt(u32 NodeId, u32 Periodicity);
/**
 * @{
 * @cond xplmi_internal
 */
void XPlmi_DisableWdt(u32 NodeId);
void XPlmi_WdtHandler(void);

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_WDT_H */

/**
 * @}
 * @endcond
 */
