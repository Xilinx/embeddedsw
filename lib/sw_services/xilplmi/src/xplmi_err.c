/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_err.c
*
* This file contains error management for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   02/12/2019 Initial release
* 1.01  ma   05/02/2020 Remove SRST error action for PSM errors as it is
*                       de-featured
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_err.h"
#include "xplmi.h"
#include "xplmi_sysmon.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
s32 (* PmSystemShutdown)(u32 SubsystemId, const u32 Type, const u32 SubType);
static void XPlmi_HandlePsmError(u32 ErrorNodeId, u32 ErrorIndex);
static void XPlmi_ErrPSMIntrHandler(u32 ErrorNodeId, u32 ErrorMask);
static void XPlmi_ErrIntrSubTypeHandler(u32 ErrorNodeId, u32 ErrorMask);
static void XPlmi_EmClearError(u32 ErrorNodeId, u32 ErrorMask);

/************************** Variable Definitions *****************************/
u32 EmSubsystemId = 0U;

/*****************************************************************************/
/**
 * @brief	This function is called in PLM error cases.
 *
 * @param	ErrStatus is the error code written to the FW_ERR register
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_ErrMgr(int ErrStatus)
{
	u32 RegVal;

	/* Print the PLM error */
	XPlmi_Printf(DEBUG_GENERAL, "PLM Error Status: 0x%08lx\n\r", ErrStatus);
	XPlmi_Out32(PMC_GLOBAL_PMC_FW_ERR, ErrStatus);

	/*
	 * Fallback if boot PDI is not done
	 * else just return, so that we receive next requests
	 */
	if (XPlmi_IsLoadBootPdiDone() == FALSE) {
		XPlmi_DumpRegisters();
		/*
		 * If boot mode is jtag, donot reset. This is to keep
		 * the system state intact for further debug.
		 */
#ifndef PLM_DEBUG_MODE
		if((XPlmi_In32(CRP_BOOT_MODE_USER) &
			CRP_BOOT_MODE_USER_BOOT_MODE_MASK) == 0U)
#endif
		{
			while(1U);
		}

		/* Update Multiboot register */
		RegVal = XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT);
		XPlmi_Out32(PMC_GLOBAL_PMC_MULTI_BOOT, ++RegVal);

		/* Make sure every thing completes */
		DATA_SYNC;
		INST_SYNC;

		RegVal = XPlmi_In32(CRP_RST_PS);
		XPlmi_Out32(CRP_RST_PS, RegVal |
			CRP_RST_PS_PMC_SRST_MASK);

		while(1U);
	}
}

/*
 * Structure to define error action type and handler if error to be handled
 * by PLM
 */
static struct XPlmi_Error_t ErrorTable[] = {
	[XPLMI_NODEIDX_ERROR_BOOT_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_BOOT_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_FW_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_FW_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_GSW_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_GSW_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_CFU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_CFRAME] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMC_PSM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_ERROUT, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMC_PSM_NCR] =
	{ .Handler = XPlmi_ErrPSMIntrHandler,
			.Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_DDRMB_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_DDRMB_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_NOCTYPE1_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_NOCTYPE1_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_NOCUSER] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_MMCM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_ME_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_ME_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_DDRMC_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_DDRMC_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_GT_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_GT_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PLSMON_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PLSMON_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PL0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PL1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PL2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PL3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_NPIROOT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_SSIT3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_SSIT4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_SSIT5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCAPB] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCROM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_MB_FATAL0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_MB_FATAL1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCPAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMC_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMC_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCSMON0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCSMON1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCSMON2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCSMON3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCSMON4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMC_RSRV1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMC_RSRV2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMC_RSRV3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCSMON8] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCSMON9] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_CFI] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_SEUCRC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_SEUECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMC_RSRV4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMC_RSRV5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_RTCALARM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_NPLL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PPLL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_CLKMON] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCTO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCXMPU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PMCXPPU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_SSIT0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_SSIT1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_SSIT2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PS_SW_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PS_SW_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_B_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_B_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_MB_FATAL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_OCM_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_L2_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_RPU_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_RPU_LS] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_RPU_CCF] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_GIC_AXI] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_GIC_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_APLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_RPLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_CPM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_CPM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_LPD_APB] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_FPD_APB] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_LPD_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_FPD_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_IOU_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_LPD_TO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_FPD_TO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_TO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_XRAM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_XRAM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_LPD_SWDT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_FPD_SWDT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV6] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV7] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV8] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV9] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV10] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV11] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV12] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV13] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV14] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV15] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV16] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV17] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV18] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_PSM_RSRV19] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_LPD_XMPU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_LPD_XPPU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_NODEIDX_ERROR_FPD_XMPU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
};

/****************************************************************************/
/**
* @brief    This function handles the PSM error routed to PLM.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    ErrorMask is the error received
*
* @return   None
*
****************************************************************************/
static void XPlmi_HandlePsmError(u32 ErrorNodeId, u32 ErrorIndex)
{
	u32 RegVal;

	switch (ErrorTable[ErrorIndex].Action) {
	case XPLMI_EM_ACTION_POR:
		RegVal = XPlmi_In32(CRP_RST_PS);
		XPlmi_Out32(CRP_RST_PS, RegVal | CRP_RST_PS_PMC_POR_MASK);
		while(1U);
		break;
	case XPLMI_EM_ACTION_SRST:
		RegVal = XPlmi_In32(CRP_RST_PS);
		XPlmi_Out32(CRP_RST_PS, RegVal | CRP_RST_PS_PMC_SRST_MASK);
		while(1U);
		break;
	case XPLMI_EM_ACTION_CUSTOM:
	case XPLMI_EM_ACTION_SUBSYS_SHUTDN:
	case XPLMI_EM_ACTION_SUBSYS_RESTART:
		if (ErrorTable[ErrorIndex].Handler != NULL) {
			ErrorTable[ErrorIndex].Handler(ErrorNodeId, ErrorIndex);
		}
		break;
	default:
		XPlmi_Printf(DEBUG_GENERAL, "Invalid Error Action "
				"for PSM errors. Error ID: 0x%x\r\n", ErrorIndex);
		break;
	}
}

/****************************************************************************/
/**
* @brief    This function is the interrupt handler for PSM Errors.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    ErrorMask is the error received
*
* @return   None
*
****************************************************************************/
static void XPlmi_ErrPSMIntrHandler(u32 ErrorNodeId, u32 ErrorMask)
{
	u32 Err1Status;
	u32 Err2Status;
	u32 Index;

	(void)ErrorNodeId;
	(void)ErrorMask;

	Err1Status = XPlmi_In32(PSM_GLOBAL_REG_PSM_ERR1_STATUS);
	Err2Status = XPlmi_In32(PSM_GLOBAL_REG_PSM_ERR2_STATUS);

	if (Err1Status != 0U) {
		for (Index = XPLMI_NODEIDX_ERROR_PS_SW_CR;
				Index < XPLMI_NODEIDX_ERROR_PSMERR1_MAX; Index++) {
			if ((Err1Status & (1U << (Index - XPLMI_NODEIDX_ERROR_PS_SW_CR)))
				&& (ErrorTable[Index].Action != XPLMI_EM_ACTION_NONE)
				&& (ErrorTable[Index].Action != XPLMI_EM_ACTION_ERROUT)) {
				XPlmi_HandlePsmError(XPLMI_EVENT_ERROR_PSM_ERR1, Index);

				/* Do not clear error status for register notifier
				 * error action
				 */
				if (XPLMI_EM_ACTION_CUSTOM != ErrorTable[Index].Action) {
					XPlmi_Out32(PSM_GLOBAL_REG_PSM_ERR1_STATUS,
						(1U << (Index - XPLMI_NODEIDX_ERROR_PS_SW_CR)));
				}
			}
		}
	}
	if (Err2Status != 0U) {
		for (Index = XPLMI_NODEIDX_ERROR_LPD_SWDT;
				Index < XPLMI_NODEIDX_ERROR_PSMERR2_MAX; Index++) {
			if ((Err2Status & (1U << (Index - XPLMI_NODEIDX_ERROR_LPD_SWDT)))
				&& (ErrorTable[Index].Action != XPLMI_EM_ACTION_NONE)
				&& (ErrorTable[Index].Action != XPLMI_EM_ACTION_ERROUT)) {
				XPlmi_HandlePsmError(XPLMI_EVENT_ERROR_PSM_ERR2, Index);

				/* Do not clear error status for register notifier
				 * error action
				 */
				if (XPLMI_EM_ACTION_CUSTOM != ErrorTable[Index].Action) {
					XPlmi_Out32(PSM_GLOBAL_REG_PSM_ERR2_STATUS,
					(1U << (Index - XPLMI_NODEIDX_ERROR_LPD_SWDT)));
				}
			}
		}
	}
}

/****************************************************************************/
/**
* @brief    This function is the interrupt handler for Error subtype subsystem
* shutdown and subsystem restart.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    ErrorMask is the error received
*
* @return   None
*
****************************************************************************/
static void XPlmi_ErrIntrSubTypeHandler(u32 ErrorNodeId, u32 ErrorMask)
{
	int Status = XST_FAILURE;
	u32 ActionId;

	(void)ErrorNodeId;

	ActionId = ErrorTable[ErrorMask].Action;

	switch (ActionId) {
	case XPLMI_EM_ACTION_SUBSYS_SHUTDN:
		XPlmi_Printf(DEBUG_GENERAL, "System shutdown 0x%x\r\n", ErrorTable[ErrorMask].SubsystemId);
		Status = (*PmSystemShutdown)(ErrorTable[ErrorMask].SubsystemId,
				XPLMI_SUBSYS_SHUTDN_TYPE_SHUTDN, 0U);
		break;
	case XPLMI_EM_ACTION_SUBSYS_RESTART:
		XPlmi_Printf(DEBUG_GENERAL, "System restart 0x%x\r\n", ErrorTable[ErrorMask].SubsystemId);
		Status = (*PmSystemShutdown)(ErrorTable[ErrorMask].SubsystemId,
				XPLMI_SUBSYS_SHUTDN_TYPE_RESTART,
				XPLMI_RESTART_SUBTYPE_SUBSYS);
		break;
	default:
		Status = XPLMI_INVALID_ERROR_ACTION;
		break;
	}

	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_GENERAL, "Error action 0x%x failed for "
				"Error: 0x%x\r\n", ActionId, ErrorMask);
	}
}

/****************************************************************************/
/**
* @brief    This function is default interrupt handler for the device.
*
* @param    CallbackRef is presently the interrupt number that is received
*
* @return   None
*
****************************************************************************/
void XPlmi_ErrIntrHandler(void *CallbackRef)
{
	u32 Err1Status;
	u32 Err2Status;
	u32 Index;

	Err1Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
	Err2Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);

	XPlmi_Printf(DEBUG_GENERAL,
			"Received Error Interrupt: 0x%0x\n\r", (u32) CallbackRef);

	if (Err1Status != 0U) {
		for (Index = XPLMI_NODEIDX_ERROR_BOOT_CR;
				Index < XPLMI_NODEIDX_ERROR_PMCERR1_MAX; Index++) {
			if ((Err1Status & (1U << Index)) &&
				(ErrorTable[Index].Handler != NULL) &&
				(ErrorTable[Index].Action != XPLMI_EM_ACTION_NONE)) {
				ErrorTable[Index].Handler(XPLMI_EVENT_ERROR_PMC_ERR1, Index);

				/* Do not clear error status for register notifier
				* error action
				*/
				if ((XPLMI_EM_ACTION_CUSTOM != ErrorTable[Index].Action) ||
					(Index == XPLMI_NODEIDX_ERROR_PMC_PSM_NCR)) {
					XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS, (1U << Index));
				}
			}
		}
	}

	if (Err2Status != 0U) {
		for (Index = XPLMI_NODEIDX_ERROR_PMCAPB;
				Index < XPLMI_NODEIDX_ERROR_PMCERR2_MAX; Index++) {
			if ((Err2Status &
				(1U << (Index - XPLMI_NODEIDX_ERROR_PMCERR1_MAX)))
				&& (ErrorTable[Index].Handler != NULL) &&
				(ErrorTable[Index].Action != XPLMI_EM_ACTION_NONE)) {
				ErrorTable[Index].Handler(XPLMI_EVENT_ERROR_PMC_ERR2, Index);

				/* Do not clear error status for register notifier
				 * error action
				 */
				if (XPLMI_EM_ACTION_CUSTOM != ErrorTable[Index].Action) {
					XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
						(1U << (Index - XPLMI_NODEIDX_ERROR_PMCERR1_MAX)));
				}
			}
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function clears any previous errors before enabling them.
 *
 * @param	ErrorNodeId is the node ID for the error event
 * @param	ErrorMask is the error to be cleared
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_EmClearError(u32 ErrorNodeId, u32 ErrorMask)
{
	u32 RegMask = XPlmi_ErrRegMask(ErrorMask);

	switch (XPlmi_EventNodeType(ErrorNodeId)) {
	case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		/* Clear previous errors */
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PMC_ERR2:
		/* Clear previous errors */
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		/* Clear previous errors */
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_ERR1_STATUS, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		/* Clear previous errors */
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_ERR2_STATUS, RegMask);
		break;
	default:
		/* Invalid Error Type */
		XPlmi_Printf(DEBUG_GENERAL,
				"Invalid ErrType for Error: 0x%0x\n\r", ErrorMask);
		break;
	}
}

/*****************************************************************************/
/**
 * @brief	This function disables the responses for the given error.
 *
 * @param	ErrorNodeId is the node ID for the error event
 * @param	ErrorMask is the error to be disabled
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_EmDisable(u32 ErrorNodeId, u32 ErrorMask)
{
	int Status = XST_FAILURE;
	u32 RegMask;

	if (ErrorMask >= XPLMI_NODEIDX_ERROR_PSMERR2_MAX) {
		/* Invalid Error ID */
		Status = XPLMI_INVALID_ERROR_ID;
		goto END;
	}

	RegMask = XPlmi_ErrRegMask(ErrorMask);

	switch (XPlmi_EventNodeType(ErrorNodeId)) {
	case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		/* Disable POR, SRST, Interrupt and PS Error Out */
		XPlmi_Out32(PMC_GLOBAL_PMC_POR1_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR_OUT1_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_IRQ1_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_SRST1_DIS, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PMC_ERR2:
		/* Disable POR, SRST, Interrupt and PS Error Out */
		XPlmi_Out32(PMC_GLOBAL_PMC_POR2_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR_OUT2_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_IRQ2_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_SRST2_DIS, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		/* Disable CR / NCR to PMC, Interrupt */
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_CR_ERR1_DIS, RegMask);
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR1_DIS, RegMask);
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_IRQ1_DIS, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		/* Disable CR / NCR to PMC, Interrupt */
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_CR_ERR2_DIS, RegMask);
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR2_DIS, RegMask);
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_IRQ2_DIS, RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		/* Invalid Error Type */
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType for Error: 0x%0x\n\r", ErrorMask);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables the POR response for the given Error.
 *
 * @param	ErrorNodeId is the node ID for the error event
 * @param	ErrorMask is the error received
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_EmEnablePOR(u32 ErrorNodeId, u32 ErrorMask)
{
	int Status = XST_FAILURE;
	u32 RegMask;

	if (ErrorMask >= XPLMI_NODEIDX_ERROR_PSMERR2_MAX) {
		Status = XPLMI_INVALID_ERROR_ID;
		goto END;
	}

	RegMask = XPlmi_ErrRegMask(ErrorMask);

	switch (XPlmi_EventNodeType(ErrorNodeId)) {
	case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		XPlmi_Out32(PMC_GLOBAL_PMC_POR1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PMC_ERR2:
		XPlmi_Out32(PMC_GLOBAL_PMC_POR2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType for Error: 0x%0x\n\r", ErrorMask);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables the SRST response for the given Error ID.
 *
 * @param	ErrorNodeId is the node ID for the error event
 * @param	ErrorMask is the error received
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_EmEnableSRST(u32 ErrorNodeId, u32 ErrorMask)
{
	int Status = XST_FAILURE;
	u32 RegMask;

	if (ErrorMask >= XPLMI_NODEIDX_ERROR_PSMERR2_MAX) {
		Status = XPLMI_INVALID_ERROR_ID;
		goto END;
	}

	RegMask = XPlmi_ErrRegMask(ErrorMask);

	switch (XPlmi_EventNodeType(ErrorNodeId)) {
	case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		XPlmi_Out32(PMC_GLOBAL_PMC_SRST1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PMC_ERR2:
		XPlmi_Out32(PMC_GLOBAL_PMC_SRST2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType for Error: 0x%0x\n\r", ErrorMask);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables the ERR OUT response for the given Error ID.
 *
 * @param	ErrorNodeId is the node ID for the error event
 * @param	ErrorMask is the error received
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_EmEnablePSError(u32 ErrorNodeId, u32 ErrorMask)
{
	int Status = XST_FAILURE;
	u32 RegMask;

	/* If Error ID is not in range, fail */
	if (ErrorMask >= XPLMI_NODEIDX_ERROR_PSMERR2_MAX) {
		Status = XPLMI_INVALID_ERROR_ID;
		goto END;
	}

	RegMask = XPlmi_ErrRegMask(ErrorMask);

	/* Enable the specified Error to propagate to ERROUT pin	*/
	switch (XPlmi_EventNodeType(ErrorNodeId)) {
	case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR_OUT1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PMC_ERR2:
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR_OUT2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_CR_ERR1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_CR_ERR2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType for Error: 0x%0x\n\r", ErrorMask);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables the interrupt to PMC for the given Error ID.
 *
 * @param	ErrorNodeId is the node ID for the error event
 * @param	ErrorMask is the error received
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_EmEnableInt(u32 ErrorNodeId, u32 ErrorMask)
{
	int Status = XST_FAILURE;
	u32 RegMask;

	if (ErrorMask >= XPLMI_NODEIDX_ERROR_PSMERR2_MAX) {
		/* Invalid Error Id */
		Status = XPLMI_INVALID_ERROR_ID;
		goto END;
	}

	RegMask = XPlmi_ErrRegMask(ErrorMask);

	switch (XPlmi_EventNodeType(ErrorNodeId)) {
	case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		XPlmi_Out32(PMC_GLOBAL_PMC_IRQ1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PMC_ERR2:
		XPlmi_Out32(PMC_GLOBAL_PMC_IRQ2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType for Error: 0x%0x\n\r", ErrorMask);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets the Action specified for a given Error ID.
 *
 * @param	ErrorNodeId is the node ID for the error event
 * @param	ErrorMask is the error to which specified action to be set
 * @param	ActionId is the action that need to be set for ErrorMask. Action
 * 		  	can be SRST/POR/ERR OUT/INT
 * @param	ErrorHandler If INT is defined as response, handler should be
 * 		  	defined.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_EmSetAction(u32 ErrorNodeId, u32 ErrorMask, u8 ActionId,
		XPlmi_ErrorHandler_t ErrorHandler)
{
	int Status = XST_FAILURE;

	/* Check for Valid Error ID */
	if ((ErrorMask >= XPLMI_NODEIDX_ERROR_PSMERR2_MAX) ||
		(ErrorTable[ErrorMask].Action == XPLMI_EM_ACTION_INVALID)) {
		/* Invalid Error Id */
		Status = XPLMI_INVALID_ERROR_ID;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid Error:0x%0x\n\r", ErrorMask);
		goto END;
	}

	if((XPLMI_EM_ACTION_CUSTOM == ActionId) && (NULL == ErrorHandler)) {
		/* Null handler */
		Status = XPLMI_INVALID_ERROR_HANDLER;
		XPlmi_Printf(DEBUG_GENERAL, "Invalid Error Handler \n\r");
		goto END;
	}

	if((ActionId > XPLMI_EM_ACTION_INVALID) &&
		(ActionId < XPLMI_EM_ACTION_MAX)) {
		/* Disable the error actions for Error ID for configuring
		 * the requested error action
		 */
		Status = XPlmi_EmDisable(ErrorNodeId, ErrorMask);
		if (XST_SUCCESS != Status) {
			/* Error action disabling failure */
			goto END;
		}
		/* Clear any previous errors */
		XPlmi_EmClearError(ErrorNodeId, ErrorMask);
	}

	switch (ActionId) {
	case XPLMI_EM_ACTION_NONE:
		/* No Action */
		ErrorTable[ErrorMask].Action = ActionId;
		Status = XST_SUCCESS;
		break;
	case XPLMI_EM_ACTION_POR:
		/* Set the error action and enable it */
		ErrorTable[ErrorMask].Action = ActionId;
		Status = XPlmi_EmEnablePOR(ErrorNodeId, ErrorMask);
		break;
	case XPLMI_EM_ACTION_SRST:
		/* Set error action SRST for the errorId */
		ErrorTable[ErrorMask].Action = ActionId;
		Status = XPlmi_EmEnableSRST(ErrorNodeId, ErrorMask);
		break;
	case XPLMI_EM_ACTION_CUSTOM:
		/* Set custom handler as error action for the errorId */
		ErrorTable[ErrorMask].Action = ActionId;
		ErrorTable[ErrorMask].Handler = ErrorHandler;
		Status = XPlmi_EmEnableInt(ErrorNodeId, ErrorMask);
		break;
	case XPLMI_EM_ACTION_ERROUT:
		ErrorTable[ErrorMask].Action = ActionId;
		/* Set error action ERROUT signal for the errorId */
		Status = XPlmi_EmEnablePSError(ErrorNodeId, ErrorMask);
		break;
	case XPLMI_EM_ACTION_SUBSYS_SHUTDN:
	case XPLMI_EM_ACTION_SUBSYS_RESTART:
		/* Set handler and error action for the errorId */
		ErrorTable[ErrorMask].Action = ActionId;
		ErrorTable[ErrorMask].Handler = XPlmi_ErrIntrSubTypeHandler;
		ErrorTable[ErrorMask].SubsystemId = EmSubsystemId;
		Status = XPlmi_EmEnableInt(ErrorNodeId, ErrorMask);
		break;
	default:
		/* Invalid Action Id */
		Status = XPLMI_INVALID_ERROR_ACTION;
		XPlmi_Printf(DEBUG_GENERAL,
		"Invalid ActionId for Error: 0x%0x\n\r", ErrorMask);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the error module. Disables all the error
 * actions and registers default action.
 *
 * @param	SystemShutdown is the pointer to the PM system shutdown
 * 			callback handler for action subtype system shutdown/restart
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_EmInit(s32 (* SystemShutdown)(u32 SubsystemId,
		const u32 Type, const u32 SubType))
{
	u32 Index;

	/* Register Error module commands */
	XPlmi_ErrModuleInit();

	/* Disable all the Error Actions */
	XPlmi_Out32(PMC_GLOBAL_PMC_POR1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR_OUT1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_IRQ1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_SRST1_DIS, MASK32_ALL_HIGH);

	/* Clear the error status registers */
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS, MASK32_ALL_HIGH);

	/* Detect if we are in over-temperature condition */
	XPlmi_SysMonOTDetect();

	PmSystemShutdown = SystemShutdown;

	/* Set the default actions as defined in the Error table */
	for (Index = XPLMI_NODEIDX_ERROR_BOOT_CR;
		Index < XPLMI_NODEIDX_ERROR_PMCERR1_MAX; Index++) {
		if ((ErrorTable[Index].Action != XPLMI_EM_ACTION_INVALID) &&
			(XPlmi_EmSetAction(XPLMI_EVENT_ERROR_PMC_ERR1, Index,
			ErrorTable[Index].Action,
			ErrorTable[Index].Handler) != XST_SUCCESS)) {
			XPlmi_Printf(DEBUG_GENERAL,
				"Warning: XPlmi_EmInit: Failed to "
				"set action for PMC ERR1: %u\r\n", Index)
		}
	}

	for (Index = XPLMI_NODEIDX_ERROR_PMCAPB;
		Index < XPLMI_NODEIDX_ERROR_PMCERR2_MAX; Index++) {
		if ((ErrorTable[Index].Action != XPLMI_EM_ACTION_INVALID) &&
			(XPlmi_EmSetAction(XPLMI_EVENT_ERROR_PMC_ERR2, Index,
			ErrorTable[Index].Action,
			ErrorTable[Index].Handler) != XST_SUCCESS)) {
			XPlmi_Printf(DEBUG_GENERAL,
				"Warning: XPlmi_EmInit: Failed to "
				"set action for PMC ERR2: %u\r\n", Index)
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function initializes the PSM error actions. Disables all the
 * PSM error actions and registers default action.
 *
 * @param	None
 *
 * @return	XST_SUCCESS
 *
*****************************************************************************/
int XPlmi_PsEmInit(void)
{
	int Status = XST_FAILURE;
	u32 Index;

	/* Disable all the Error Actions */
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_CR_ERR1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_IRQ1_DIS, MASK32_ALL_HIGH);

	/* Clear the error status registers */
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_ERR1_STATUS, MASK32_ALL_HIGH);
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_ERR2_STATUS, MASK32_ALL_HIGH);

	/* Set the default actions as defined in the Error table */
	for (Index = XPLMI_NODEIDX_ERROR_PS_SW_CR;
		Index < XPLMI_NODEIDX_ERROR_PSMERR1_MAX; Index++) {
		if ((ErrorTable[Index].Action != XPLMI_EM_ACTION_INVALID) &&
			(XPlmi_EmSetAction(XPLMI_EVENT_ERROR_PSM_ERR1, Index,
			ErrorTable[Index].Action,
			ErrorTable[Index].Handler) != XST_SUCCESS)) {
			XPlmi_Printf(DEBUG_GENERAL,
				"Warning: XPlmi_PsEmInit: Failed to "
				"set action for PSM ERR1: %u\r\n", Index)
		}
	}

	for (Index = XPLMI_NODEIDX_ERROR_LPD_SWDT;
		Index < XPLMI_NODEIDX_ERROR_PSMERR2_MAX; Index++) {
		if ((ErrorTable[Index].Action != XPLMI_EM_ACTION_INVALID) &&
			(XPlmi_EmSetAction(XPLMI_EVENT_ERROR_PSM_ERR2, Index,
			ErrorTable[Index].Action,
			ErrorTable[Index].Handler) != XST_SUCCESS)) {
			XPlmi_Printf(DEBUG_GENERAL,
				"Warning: XPlmi_PsEmInit: Failed to "
				"set action for PSM ERR2: %u\r\n", Index)
		}
	}
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function dumps the registers which can help debugging.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_DumpRegisters()
{
	XPlmi_Printf(DEBUG_GENERAL, "====Register Dump============\n\r");

	XPlmi_Printf(DEBUG_GENERAL, "IDCODE: 0x%08x\n\r",
		XPlmi_In32(PMC_TAP_IDCODE));
	XPlmi_Printf(DEBUG_GENERAL, "EXTENDED IDCODE: 0x%08x\n\r",
		XPlmi_In32(EFUSE_CACHE_IP_DISABLE_0));
	XPlmi_Printf(DEBUG_GENERAL, "Version: 0x%08x\n\r",
		XPlmi_In32(PMC_TAP_VERSION));
	XPlmi_Printf(DEBUG_GENERAL, "Bootmode User: 0x%08x\n\r",
		XPlmi_In32(CRP_BOOT_MODE_USER));
	XPlmi_Printf(DEBUG_GENERAL, "Bootmode POR: 0x%08x\n\r",
		XPlmi_In32(CRP_BOOT_MODE_POR));
	XPlmi_Printf(DEBUG_GENERAL, "Reset Reason: 0x%08x\n\r",
		XPlmi_In32(CRP_RESET_REASON));
	XPlmi_Printf(DEBUG_GENERAL, "Multiboot: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT));
	XPlmi_Printf(DEBUG_GENERAL, "PMC PWR Status: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PWR_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC GSW Err: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PMC_GSW_ERR));
	XPlmi_Printf(DEBUG_GENERAL, "PLM Error: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PLM_ERR));
	XPlmi_Printf(DEBUG_GENERAL, "PMC ERR OUT1 Status: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC ERR OUT2 Status: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP0 IRQ Status: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP0_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP1 IRQ Status: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP1_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP2 IRQ Status: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP2_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP3 IRQ Status: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP3_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP4 IRQ Status: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP4_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICPPMC IRQ Status: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS));

	XPlmi_Printf(DEBUG_GENERAL, "====Register Dump============\n\r");
}
