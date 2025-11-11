/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file versal_2vp_p/server/xplmi_err.h
*
* This file contains declarations for versal_2vp EAM code
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 2.3   sd  10/13/25 Initial release
*
* </pre>
*
* @note
*
***************************************************************************************************/

#ifndef XPLMI_ERR_H
#define XPLMI_ERR_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xplmi_error_node.h"
#include "xplmi_hw.h"
#include "xplmi_err_common.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/
#define GET_PMC_ERR_ACTION_OFFSET(Index)	(Index * XPLMI_PMC_PSM_ERR2_REG_OFFSET)
							/**< PMC error action offset */
#define GET_PMC_ERR_ACTION_ADDR(PmcMask, Index)	(PmcMask + (Index * XPLMI_PMC_PSM_ERR2_REG_OFFSET))
							/**< PMC error action address */

#define GET_PMC_ERR_OUT_MASK(RegOffset)		(PMC_GLOBAL_PMC_ERR_OUT1_MASK + RegOffset)
							/**< PMC error out mask */
#define GET_PMC_POR_MASK(RegOffset)		(PMC_GLOBAL_PMC_POR1_MASK + RegOffset)
							/**< PMC POR mask */
#define GET_PMC_IRQ_MASK(RegOffset)		(PMC_GLOBAL_PMC_IRQ1_MASK + RegOffset)
							/**< PMC IRQ mask */
#define GET_PMC_SRST_MASK(RegOffset)		(PMC_GLOBAL_PMC_SRST1_MASK + RegOffset)
							/**< PMC SRST mask */

/**************************************************************************************************/
/**
 * @brief	This function reconfigures error actions after the update.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static inline void XPlmi_ReconfigErrActions(void)
{
	/* Not applicable for versal_2vp */
	return;
}

/************************************ Function Prototypes *****************************************/
XPlmi_Error_t *XPlmi_GetErrorTable(void);
void XPlmi_SysmonClkSetIro(void);
void XPlmi_HandleLinkDownError(u32 Cpm5PcieIrStatusReg, u32 Cpm5DmaCsrIntDecReg, u32 ProcId);
void XPlmi_DumpErrNGicStatus(void);
u8 XPlmi_GetEventIndex(XPlmi_EventType ErrorNodeType);
void XPlmi_ClearSsitErrors(u32 *PmcErrStatus, u32 Index);
u32 *XPlmi_GetNumErrOuts(void);
int XPlmi_RestrictErrActions(XPlmi_EventType NodeType, u32 RegMask, u32 ErrorAction);
u32 *XPlmi_GetPsmCrState(void);
void XPlmi_ErrPrintToLog(u32 ErrorNodeId, u32 RegMask);

/************************************ Variable Definitions ****************************************/

/**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ERR_H */
