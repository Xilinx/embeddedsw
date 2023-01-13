/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/xplmi_wdt.h
* @addtogroup xplmi_apis XilPlmi versal_net APIs
* @{
* @cond xplmi_internal
* This file contains declarations related to WDT in versal_net
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
* 1.01  ng   11/11/2022 Fixed doxygen file name error
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
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/
/**
 * @{
 * @cond xplmi_internal
 */

/* WDT Timeout (in ms) to be used in In-Place Update */
#define XPLMI_INPLACE_UPDATE_WDT_TIMEOUT	(100U)

#define XPLMI_PM_STMIC_LMIO_0		(0x14104001U)
#define XPLMI_PM_STMIC_LMIO_25		(0x1410401aU)
#define XPLMI_PM_STMIC_PMIO_0		(0x1410801bU)
#define XPLMI_PM_STMIC_PMIO_51		(0x1410804eU)
#define XPLMI_PM_DEV_PMC_WDT		(0x1821C035U)

#define XPLMI_WDT_EXTERNAL		XPLMI_PM_STMIC_LMIO_0
#define XPLMI_WDT_INTERNAL		XPLMI_PM_DEV_PMC_WDT

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
 * @brief	This function checks if ROM SWDT Usage is enabled in EFUSE
 *
 * @return	TRUE if Usage is enabled, else FALSE
 *
 *****************************************************************************/
static inline u8 XPlmi_RomSwdtUsage(void) {
	return (XPlmi_In32(EFUSE_CACHE_ROM_RSVD) &
			EFUSE_ROM_SWDT_USAGE_MASK) ?  (u8)TRUE : (u8)FALSE;
}

/************************** Function Prototypes ******************************/
void XPlmi_SetPlmLiveStatus(void);
void XPlmi_ClearPlmLiveStatus(void);
/**
 * @}
 * @endcond
 */
int XPlmi_EnableWdt(u32 NodeId, u32 Periodicity);
void XPlmi_KickWdt(u32 NodeId);
void XPlmi_RestoreWdt(void);
void XPlmi_StopWdt(u32 NodeId);
int XPlmi_DefaultSWdtConfig(void);
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
