/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_wdt.h
* @addtogroup xplmi_apis XilPlmi Versal APIs
* @{
* @cond xplmi_internal
* This file contains declarations related to WDT
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/28/2020 Initial release
*       ana  10/19/2020 Added doxygen comments
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XPLMI_WDT_H
#define XPLMI_WDT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/
/**
 * @{
 * @cond xplmi_internal
 */
#define XPLMI_MODE_OPERATIONAL		(1U)
#define XPLMI_MODE_CONFIGURATION	(2U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XPlmi_SetPlmMode(u8 Mode);
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
void XPlmi_DisableWdt(void);
void XPlmi_WdtHandler(void);

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_WDT_H */

/**
+ * @}
+ * @endcond
+ */

/** @} */