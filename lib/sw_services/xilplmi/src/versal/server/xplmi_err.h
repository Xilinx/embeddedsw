/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal/xplmi_err.h
*
* This file contains declarations for versal EAM code
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
*       dd   03/28/2023 Updated doxygen comments
*       dd   09/12/2023 MISRA-C violation Rule 10.3 fixed
* 1.01  ma   02/29/2024 Change protection unit error actions to PRINT_TO_LOG
*                       to handle restoring of the error actions after IPU
*       kj   09/18/2024 Added support for SW Error Handling in secondary SLR
*                       and changed HBM CATTRIP SW Error Action in ErrorTable.
*                       Also restricted HBM Cattrip error action to HW Errors.
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
#define GET_PMC_ERR_ACTION_OFFSET(Index)	\
				(Index * XPLMI_PMC_PSM_ERR2_REG_OFFSET) /**< PMC error action offset */
#define GET_PMC_ERR_ACTION_ADDR(PmcMask, Index) \
				(PmcMask + (Index * XPLMI_PMC_PSM_ERR2_REG_OFFSET)) /**< PMC error action address */

#define GET_PMC_ERR_OUT_MASK(RegOffset)	(PMC_GLOBAL_PMC_ERR_OUT1_MASK + RegOffset) /**< PMC error out mask */
#define GET_PMC_POR_MASK(RegOffset)	(PMC_GLOBAL_PMC_POR1_MASK + RegOffset) /**< PMC POR mask */
#define GET_PMC_IRQ_MASK(RegOffset)	(PMC_GLOBAL_PMC_IRQ1_MASK + RegOffset) /**< PMC IRQ mask */
#define GET_PMC_SRST_MASK(RegOffset)	(PMC_GLOBAL_PMC_SRST1_MASK + RegOffset) /**< PMC SRST mask */

/*****************************************************************************/
/**
 * @brief	This function reconfigures error actions after the update
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XPlmi_ReconfigErrActions(void)
{
	/* Not applicable for versal */
	return;
}

/************************** Function Prototypes ******************************/
XPlmi_Error_t *XPlmi_GetErrorTable(void);
void XPlmi_SysmonClkSetIro(void);
void XPlmi_HandleLinkDownError(u32 Cpm5PcieIrStatusReg,
		u32 Cpm5DmaCsrIntDecReg, u32 ProcId);
void XPlmi_DumpErrNGicStatus(void);
u8 XPlmi_GetEventIndex(XPlmi_EventType ErrorNodeType);
void XPlmi_ClearSsitErrors(u32 *PmcErrStatus, u32 Index);
#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
void XPlmi_DetectSlaveSlrTamper(void);
void XPlmi_EnableSsitErrors(void);
#endif
u32 *XPlmi_GetNumErrOuts(void);
int XPlmi_RestrictErrActions(XPlmi_EventType NodeType, u32 RegMask, u32 ErrorAction);
u32 *XPlmi_GetPsmCrState(void);
void XPlmi_ErrPrintToLog(u32 ErrorNodeId, u32 RegMask);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ERR_H */
