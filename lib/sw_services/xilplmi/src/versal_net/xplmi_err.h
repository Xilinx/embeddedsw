/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_err_plat.h
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
			(Index * XPLMI_PMC_PSM_ERR2_REG_OFFSET))

#define GET_PMC_ERR_ACTION_ADDR(PmcMask, Index)	((Index == 2) ? \
			(PmcMask + ((0xFU) * XPLMI_PMC_PSM_ERR2_REG_OFFSET - \
			 ((PmcMask - PMC_GLOBAL_PMC_ERR_OUT1_MASK) / 2U))) :\
			(PmcMask + (Index * XPLMI_PMC_PSM_ERR2_REG_OFFSET)))


#define GET_PMC_ERR_OUT_MASK(RegOffset)	(RegOffset == 0xF0U) ? (PMC_GLOBAL_PMC_ERR_OUT3_MASK) :\
					(PMC_GLOBAL_PMC_ERR_OUT1_MASK + RegOffset)

#define GET_PMC_POR_MASK(RegOffset) (RegOffset == 0xF0U) ? (PMC_GLOBAL_PMC_POR3_MASK) :\
					(PMC_GLOBAL_PMC_POR1_MASK + RegOffset)

#define GET_PMC_IRQ_MASK(RegOffset) (RegOffset == 0xF0U) ? (PMC_GLOBAL_PMC_IRQ3_MASK) :\
					(PMC_GLOBAL_PMC_IRQ1_MASK + RegOffset)

#define GET_PMC_SRST_MASK(RegOffset) (RegOffset == 0xF0U) ? (PMC_GLOBAL_PMC_SRST3_MASK) :\
					(PMC_GLOBAL_PMC_SRST1_MASK + RegOffset)

/*****************************************************************************/
/**
 * @brief	This function triggers SSIT_ERR to slave SLRS and is not
 *		applicable for versal_net
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XPlmi_TriggerSsitErrToMaster(void)
{
	/* Not applicable for versal net */
	return;
}

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

/****************************************************************************/
/**
* @brief    This function handles the CPM_NCR PCIE link down error.
*
* @param    Cpm5PcieIrStatusReg is the PCIE0/1 IR status register address
* @param    Cpm5DmaCsrIntDecReg is the DMA0/1 CSR INT DEC register address
* @param    ProcId is the ProcId for PCIE0/1 link down error
*
* @return   None
*
****************************************************************************/
static inline void XPlmi_HandleLinkDownError(u32 Cpm5PcieIrStatusReg,
		u32 Cpm5DmaCsrIntDecReg, u32 ProcId)
{
	(void)Cpm5PcieIrStatusReg;
	(void)Cpm5DmaCsrIntDecReg;
	(void)ProcId;
	/* Not applicable for versal net */
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
u8 XPlmi_GetEventIndex(u32 ErrorNodeType);
int XPlmi_RestrictErrActions(XPlmi_EventType NodeType, u32 RegMask, u32 ErrorAction);
void XPlmi_DisablePmcErrAction(u32 ErrIndex, u32 RegMask);
void XPlmi_DumpErrNGicStatus(void);
void XPlmi_ReconfigErrActions(void);
u32 *XPlmi_GetNumErrOuts(void);
u32 *XPlmi_GetPsmCrState(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ERR_H */
