/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal_net/xplmi_err.h
*
* This file contains declarations versal_net PLMI module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       ma   07/20/2022 Rename PMC_PSM_ERR_REG_OFFSET macro
*       bm   07/20/2022 Update EAM logic for In-Place PLM Update
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*       bm   01/03/2023 Remove Triggering of SSIT ERR2 from Slave SLR to
*                       Master SLR
*       bm   01/03/2023 Notify Other SLRs about Secure Lockdown
*       sk   01/13/2023 CPM5N Link UP Event handler declaration
*       dd   03/28/2023 Updated doxygen comments
*       dd   09/12/2023 MISRA-C violation Rule 10.3 fixed
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_ERR_H
#define XPLMI_ERR_H

#ifdef __cplusplus
extern "C" {
#endif

/************************** Include Files *****************************/
#include "xplmi_error_node.h"
#include "xplmi_hw.h"
#include "xplmi_err_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define GET_PMC_ERR_ACTION_OFFSET(Index)	((Index == 2) ? \
			((0xFU) * XPLMI_PMC_PSM_ERR2_REG_OFFSET) : \
			(Index * XPLMI_PMC_PSM_ERR2_REG_OFFSET))  /**< PMC error action offset */

#define GET_PMC_ERR_ACTION_ADDR(PmcMask, Index)	((Index == 2) ? \
			(PmcMask + ((0xFU) * XPLMI_PMC_PSM_ERR2_REG_OFFSET - \
			 ((PmcMask - PMC_GLOBAL_PMC_ERR_OUT1_MASK) / 2U))) :\
			(PmcMask + (Index * XPLMI_PMC_PSM_ERR2_REG_OFFSET))) /**< PMC error action address */


#define GET_PMC_ERR_OUT_MASK(RegOffset)	(RegOffset == 0xF0U) ? (PMC_GLOBAL_PMC_ERR_OUT3_MASK) :\
					(PMC_GLOBAL_PMC_ERR_OUT1_MASK + RegOffset) /**< PMC error out mask */

#define GET_PMC_POR_MASK(RegOffset) (RegOffset == 0xF0U) ? (PMC_GLOBAL_PMC_POR3_MASK) :\
					(PMC_GLOBAL_PMC_POR1_MASK + RegOffset) /**< PMC POR mask */

#define GET_PMC_IRQ_MASK(RegOffset) (RegOffset == 0xF0U) ? (PMC_GLOBAL_PMC_IRQ3_MASK) :\
					(PMC_GLOBAL_PMC_IRQ1_MASK + RegOffset) /**< PMC IRQ mask */

#define GET_PMC_SRST_MASK(RegOffset) (RegOffset == 0xF0U) ? (PMC_GLOBAL_PMC_SRST3_MASK) :\
					(PMC_GLOBAL_PMC_SRST1_MASK + RegOffset) /**< PMC SRST mask */

/*****************************************************************************/
/**
 * @brief	This function sets the sysmon clock to IRO for ES1 silicon
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XPlmi_SysmonClkSetIro(void) {
	/* This workaround is not applicable for versal net */
	return;
}

/*****************************************************************************/
/**
 * @brief	This function clears Ssit errors for ES1 silicon
 *
 * @param	PmcErrStatus is the pointer to the error status array
 * @param	Index is the PMC Error register Index
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XPlmi_ClearSsitErrors(u32 *PmcErrStatus, u32 Index)
{
	(void)PmcErrStatus;
	(void)Index;
	/* Not applicable for versal net */
	return;
}

/************************** Function Prototypes ******************************/
XPlmi_Error_t *XPlmi_GetErrorTable(void);
u8 XPlmi_GetEventIndex(XPlmi_EventType ErrorNodeType);
int XPlmi_RestrictErrActions(XPlmi_EventType NodeType, u32 RegMask, u32 ErrorAction);
void XPlmi_DumpErrNGicStatus(void);
void XPlmi_ReconfigErrActions(void);
u32 *XPlmi_GetNumErrOuts(void);
u32 *XPlmi_GetPsmCrState(void);
void XPlmi_HandleLinkUpEvent(u32 Cpm5NPcieCdxIrStatusReg, u32 Cpm5NCdxPcieBReg, u32 ProcId);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ERR_H */
