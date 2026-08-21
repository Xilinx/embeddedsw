/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_err.h
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
* 1.02  ma   02/29/2024 Change protection unit error actions to PRINT_TO_LOG
*                       to handle restoring of the error actions after IPU
* 1.02  sk   08/26/2024 Updated EAM support for Versal 2VE and 2VM Devices
*       sk   02/20/2025 Added defines for Versal 2VE and 2VM Devices EAM handler
* 2.4   abh  10/17/2025 Fixed MISRA-C violations
*       ias  08/07/2026 Added error link table accessor prototypes
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
#define GET_PMC_ERR_ACTION_OFFSET(Index)	(((Index) == 2U) ? \
			(0xFU * XPLMI_PMC_PSM_ERR2_REG_OFFSET) : \
			((Index) * XPLMI_PMC_PSM_ERR2_REG_OFFSET))  /**< PMC error action offset */

#define GET_PMC_ERR_ACTION_ADDR(PmcMask, Index)	(((Index) == 2U) ? \
			((PmcMask) + ((0xFU * XPLMI_PMC_PSM_ERR2_REG_OFFSET) - \
			 (((PmcMask) - PMC_GLOBAL_PMC_ERR_OUT1_MASK) / 2U))) :\
			((PmcMask) + ((Index) * XPLMI_PMC_PSM_ERR2_REG_OFFSET))) /**< PMC error action address */


#define GET_PMC_ERR_OUT_MASK(RegOffset)	(RegOffset == 0xF0U) ? (PMC_GLOBAL_PMC_ERR_OUT3_MASK) :\
					(PMC_GLOBAL_PMC_ERR_OUT1_MASK + RegOffset) /**< PMC error out mask */

#define GET_PMC_POR_MASK(RegOffset) (RegOffset == 0xF0U) ? (PMC_GLOBAL_PMC_POR3_MASK) :\
					(PMC_GLOBAL_PMC_POR1_MASK + RegOffset) /**< PMC POR mask */

/*
 * link_nodes payload fixed word offsets. Word 0 = EventNodeId, word 1 = Mask,
 * words 2..N = node IDs (>= 1). Minimum payload = 3 words.
 */
#define XPLMI_EM_LINK_NODES_EVENTID_OFFSET	(0U)	/**< EventNodeId offset */
#define XPLMI_EM_LINK_NODES_MASK_OFFSET		(1U)	/**< Mask offset */
#define XPLMI_EM_LINK_NODES_NODEID_OFFSET	(2U)	/**< First node-id offset */

/*
 * NODE_ALL sentinel reject for link_nodes. The node index field is the low
 * 14 bits of a PM node id. We reject an all-ones index because link_nodes
 * stores node IDs verbatim (no NODE_ALL expansion), so a NODE_ALL sentinel
 * would resolve to nothing. Both the 14-bit (0x3FFF) and 10-bit (0x3FF)
 * all-ones patterns are rejected so either sentinel encoding is refused.
 */
#define XPLMI_EM_LINK_NODE_INDEX_MASK		(0x3FFFU)	/**< Node index bits */
#define XPLMI_EM_LINK_NODE_ALL_14		(0x3FFFU)	/**< NODE_ALL (14-bit) */
#define XPLMI_EM_LINK_NODE_ALL_10		(0x3FFU)	/**< NODE_ALL (10-bit) */

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
XPlmi_ErrLink *XPlmi_GetErrLinkTable(void);
u32 *XPlmi_GetErrLinkPool(void);
u8 XPlmi_GetEventIndex(XPlmi_EventType ErrorNodeType);
int XPlmi_RestrictErrActions(XPlmi_EventType NodeType, u32 RegMask, u32 ErrorAction);
void XPlmi_DumpErrNGicStatus(void);
void XPlmi_ReconfigErrActions(void);
u32 *XPlmi_GetNumErrOuts(void);
u32 *XPlmi_GetPsmCrState(void);
void XPlmi_HandleLinkUpEvent(u32 Cpm5NPcieCdxIrStatusReg, u32 Cpm5NCdxPcieBReg, u32 ProcId);
void XPlmi_ErrPrintToLog(u32 ErrorNodeId, u32 RegMask);
int XPlmi_EmDisableLpdSlcrErrors(u32 RegMaskAddr, u32 RegMask);
int EmEnableLpdSlcrErrAction(u32 ErrMaskRegAddr, u32 RegMask);
int XPlmi_LpdSlcrEmInit(void);
int XPlmi_Versal2Ve2VmSetAction(XPlmi_Cmd * Cmd);
int XPlmi_Versal2Ve2VmLinkNodes(XPlmi_Cmd * Cmd);
/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ERR_H */
