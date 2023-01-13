/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/xplmi_wdt.h
* @addtogroup xplmi_apis XilPlmi Versal APIs
* @{
* @cond xplmi_internal
* This file contains declarations related to WDT in versal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/28/2020 Initial release
*       ana  10/19/2020 Added doxygen comments
* 1.01  bsv  08/13/2021 Code clean up
* 1.02  bm   07/06/2022 Refactor versal and versal_net code
* 1.03  ng   11/11/2022 Fixed doxygen file name error
*       bm   01/14/2023 Remove bypassing of PLM Set Alive during boot
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

#define XPLMI_WDT_EXTERNAL		(0x14104001U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
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
+ * @}
+ * @endcond
+ */

/** @} */
