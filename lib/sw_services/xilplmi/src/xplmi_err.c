/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
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
* 1.01  kc   08/01/2019 Added error management framework
*       ma   08/01/2019 Added LPD init code
*       sn   08/03/2019 Added code to wait until over-temperature condition
*						gets resolved before restart
*       bsv  08/29/2019 Added Multiboot and Fallback support
*       scs  08/29/2019 Added support for Extended IDCODE checks
* 1.02  ma   05/02/2020 Remove SRST error action for PSM errors as it is
*                       de-featured
*       ma   02/28/2020 Error actions related changes
*       bsv  04/04/2020 Code clean up
* 1.03  bsv  07/07/2020 Made functions used in single transaltion unit as
*						static
*       kc   08/11/2020 Added disabling and clearing of error which has actions
*                       selected as subsystem shutdown or restart or custom.
*                       They have to be re-enabled again using SetAction
*                       command.
*       bsv  09/21/2020 Set clock source to IRO before SRST for ES1 silicon
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.04  td   01/07/2021 Fix warning in PLM memory log regarding NULL handler for
*                       PMC_PSM_NCR error
*       bsv  01/29/2021 Added APIs for checking and clearing NPI errors
* 1.05  pj   03/24/2021 Added API to update Subystem Id of the error node
*       pj   03/24/2021 Added API to trigger error handling from software.
*                       Added SW Error event and Ids for Healthy Boot errors
*       bm   03/24/2021 Added logic to store error status in RTCA
*       bl   04/01/2021 Update function signature for PmSystemShutdown
*       ma   04/05/2021 Added support for error configuration using Error Mask
*                       instead of Error ID. Also, added support to configure
*                       multiple errors at once.
*       ma   05/03/2021 Minor updates related to PSM and FW errors, trigger
*                       FW_CR for PSM and FW errors which have error action
*                       set as ERROR_OUT
*       bsv  05/15/2021 Remove warning for AXI_WRSTRB NPI error
*       ma   05/17/2021 Update only data field when writing error code to FW_ERR
*                       register
*       bm   05/18/2021 Ignore printing and storing of ssit errors for ES1 silicon
*       td   05/20/2021 Fixed blind write on locking NPI address space in
*                       XPlmi_ClearNpiErrors
* 1.06  ma   06/28/2021 Added handler for CPM_NCR error
*       ma   07/08/2021 Fix logic in reading link down error mask value
*       td   07/08/2021 Fix doxygen warnings
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  07/18/2021 Debug enhancements
*       kc   07/22/2021 XPlmi_PorHandler scope updated to global
*       ma   07/27/2021 Updated print statements in CPM handlers
*       bm   08/02/2021 Change debug log level of PMC error status prints
*       ma   08/06/2021 Save PMC_FW_ERR register value to RTCA and clear it
*       bsv  08/15/2021 Removed unwanted goto statements
*       rv   08/19/2021 Updated XPLMI_EM_ACTION_SUBSYS_RESTART error action
*			handling
*       ma   08/19/2021 Renamed error related macros
*       ma   08/30/2021 Modified XPlmi_ErrMgr function to handle errors in
*                       SSIT Slave SLRs
*       bsv  10/11/2021 Added boundary check before incrementing NumErrOuts
* 1.07  ma   12/17/2021 Clear SSIT_ERR register during EM init
*       bsv  12/24/2021 Move common defines from xilplmi and xilpm to common
*                       folder
*       is   01/10/2022 Updated XPlmi_SysMonOTDetect API to pass wait time arg
*       is   01/10/2022 Updated Copyright Year to 2022
*       ma   01/17/2022 Add exceptions to SW Errors list
*       ma   01/24/2022 Check error mask registers after error action is
*                       enabled or disabled
*       ma   01/24/2022 Check if error action is enabled before executing the
*                       handler
*       ma   02/01/2022 Fix SW-BP-INIT-TO-FAILURE warnings
*       ma   03/10/2022 Fix bug in disabling the error actions for PSM errors
*       is   03/22/2022 Add custom handler for XPPU/XMPU error events
* 1.08  ma   05/10/2022 Added PLM to PLM communication feature
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
#include "xplmi_ssit.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
#define XPLMI_SYSMON_CLK_SRC_IRO_VAL	(0U)
#define XPLMI_UPDATE_TYPE_INCREMENT	(1U)
#define XPLMI_UPDATE_TYPE_DECREMENT	(2U)
#define XPLMI_MAX_ERR_OUTS		(0xFFFFFFFFU)
#define XPLMI_PMC_ERR1_SSIT_MASK	(0xE0000000U)
#define XPLMI_PMC_ERR2_SSIT_MASK	(0xE0000000U)

/* Proc IDs for CPM Proc CDOs */
#define CPM_NCR_PCIE0_LINK_DOWN_PROC_ID					(0x1U)
#define CPM_NCR_PCIE1_LINK_DOWN_PROC_ID					(0x2U)

/**
 * @}
 * @endcond
 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static s32 (* PmSystemShutdown)(u32 SubsystemId, const u32 Type, const u32 SubType,
				const u32 CmdType);
static s32 (* PmSubsysRestart)(const u32 SubsystemId);
static void XPlmi_HandlePsmError(u32 ErrorNodeId, u32 RegMask);
static void XPlmi_ErrPSMIntrHandler(u32 ErrorNodeId, u32 RegMask);
static void XPlmi_ErrIntrSubTypeHandler(u32 ErrorNodeId, u32 RegMask);
static void XPlmi_EmClearError(u32 ErrorNodeType, u32 ErrorId);
static void XPlmi_SoftResetHandler(void);
static void XPlmi_SysmonClkSetIro(void);
static void XPlmi_DumpRegisters(void);
static u32 XPlmi_UpdateNumErrOutsCount(u8 UpdateType);
static void XPlmi_HandleLinkDownError(u32 Cpm5PcieIrStatusReg,
		u32 Cpm5DmaCsrIntDecReg, u32 ProcId);
static void XPlmi_CpmErrHandler(u32 ErrorNodeId, u32 RegMask);
static void XPlmi_XppuErrHandler(u32 BaseAddr, const char *ProtUnitStr);
static void XPlmi_XmpuErrHandler(u32 BaseAddr, const char *ProtUnitStr);
static void XPlmi_ProtUnitErrHandler(u32 ErrorNodeId, u32 RegMask);

/************************** Variable Definitions *****************************/
static u32 EmSubsystemId = 0U;

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
#ifndef PLM_DEBUG_MODE
	u32 RegVal;
#endif
	u8 SlrType = (u8)(XPlmi_In32(PMC_TAP_SLR_TYPE) &
			PMC_TAP_SLR_TYPE_VAL_MASK);

	/* Print the PLM error */
	XPlmi_Printf(DEBUG_GENERAL, "PLM Error Status: 0x%08lx\n\r", ErrStatus);
	XPlmi_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_DATA_MASK,
			(u32)ErrStatus);

	/*
	 * Check if SLR Type is Master or Monolithic
	 * and take error action accordingly.
	 */
	if ((SlrType == XPLMI_SSIT_MASTER_SLR) ||
		(SlrType == XPLMI_SSIT_MONOLITIC)) {
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
				while (TRUE) {
					;
				}
			}

#ifndef PLM_DEBUG_MODE
			/* Update Multiboot register */
			RegVal = XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT);
			XPlmi_Out32(PMC_GLOBAL_PMC_MULTI_BOOT, RegVal + 1U);

			XPlmi_TriggerFwNcrError();
#endif
		}
	} else {
		/*
		 * Trigger SSIT error to Master SLR if SLR Type is Slave SLR
		 * And clear it immediately
		 */
		XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK,
				PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK);
		XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK,
				0x0U);
	}
}

/*
 * Structure to define error action type and handler if error to be handled
 * by PLM
 */
static struct XPlmi_Error_t ErrorTable[XPLMI_ERROR_SW_ERR_MAX] = {
	[XPLMI_ERROR_BOOT_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_BOOT_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FW_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_ERROUT, .SubsystemId = 0U, },
	[XPLMI_ERROR_FW_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_SRST, .SubsystemId = 0U, },
	[XPLMI_ERROR_GSW_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_GSW_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CFU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CFRAME] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_PSM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_SRST, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_PSM_NCR] =
	{ .Handler = NULL,
			.Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_DDRMB_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DDRMB_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOCTYPE1_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOCTYPE1_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOCUSER] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_MMCM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_AIE_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_AIE_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DDRMC_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DDRMC_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_GT_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_GT_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PLSMON_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PLSMON_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PL0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PL1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PL2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PL3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NPIROOT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCAPB] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCROM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_MB_FATAL0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_MB_FATAL1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCPAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON8] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON9] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CFI] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SEUCRC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SEUECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_RTCALARM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NPLL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PPLL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CLKMON] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCTO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCXMPU] =
	{ .Handler = XPlmi_ProtUnitErrHandler,
			.Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCXPPU] =
	{ .Handler = XPlmi_ProtUnitErrHandler,
			.Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PS_SW_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PS_SW_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_B_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_B_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_MB_FATAL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_L2_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPU_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPU_LS] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPU_CCF] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_GIC_AXI] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_GIC_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_APLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CPM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CPM_NCR] =
	{ .Handler = XPlmi_CpmErrHandler,
			.Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_APB] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_APB] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_IOU_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_TO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_TO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_TO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_XRAM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_XRAM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_SWDT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_SWDT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV6] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV7] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV8] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV9] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV10] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV11] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV12] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV13] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV14] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV15] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV16] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV17] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV18] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV19] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_XMPU] =
	{ .Handler = XPlmi_ProtUnitErrHandler,
			.Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_XPPU] =
	{ .Handler = XPlmi_ProtUnitErrHandler,
			.Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_XMPU] =
	{ .Handler = XPlmi_ProtUnitErrHandler,
			.Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_HB_MON_0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_HB_MON_1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_HB_MON_2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_HB_MON_3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PLM_EXCEPTION] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
};

/****************************************************************************/
/**
* @brief    This function updates the SubystemId for the given error index.
*
* @param    ErrorNodeId is the node Id for the error event
* @param    ErrorMasks is the Register mask of the Errors
* @param    SubsystemId is the Subsystem ID for the error node.
*
* @return   None
*
****************************************************************************/
void XPlmi_UpdateErrorSubsystemId(u32 ErrorNodeId,
		u32 ErrorMasks, u32 SubsystemId)
{
	u32 ErrorId = XPlmi_EventNodeType(ErrorNodeId) * (u32)XPLMI_MAX_ERR_BITS;
	u32 ErrMasks = ErrorMasks;

	for (; ErrMasks != 0U; ErrMasks >>= 1U) {
		if (((ErrMasks & 0x1U) != 0U) &&
			(ErrorId < XPLMI_ERROR_SW_ERR_MAX)) {
			ErrorTable[ErrorId].SubsystemId = SubsystemId;
		}
		ErrorId++;
	}
}

/*****************************************************************************/
/**
 * @brief	This function triggers Power on Reset
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_PORHandler(void) {
	XPlmi_SysmonClkSetIro();
	XPlmi_UtilRMW(CRP_RST_PS, CRP_RST_PS_PMC_POR_MASK,
		CRP_RST_PS_PMC_POR_MASK);
	while(TRUE) {
		;
	}
}

/*****************************************************************************/
/**
 * @brief	This function returns Error Id for the given error node type and
 * error mask.
 *
 * @param	ErrorNodeId is the error node Id.
 * @param	RegMask  is register mask of the error.
 *
 * @return	ErrorId value.
 *
 *****************************************************************************/
static u32 XPlmi_GetErrorId(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrorId = 0U;
	u32 Mask = RegMask;
	u32 ErrorNodeType = XPlmi_EventNodeType(ErrorNodeId);

	while (Mask != (u32)0U) {
		if ((Mask & 0x1U) == 0x1U) {
			break;
		}
		ErrorId++;
		Mask >>= 1U;
	}
	ErrorId += (ErrorNodeType * XPLMI_MAX_ERR_BITS);

	return ErrorId;
}

/****************************************************************************/
/**
* @brief    This function handles the PSM error routed to PLM.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return   None
*
****************************************************************************/
static void XPlmi_HandlePsmError(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrorId;
	u32 ErrorNodeType = XPlmi_EventNodeType(ErrorNodeId);

	ErrorId = XPlmi_GetErrorId(ErrorNodeId, RegMask);

	switch (ErrorTable[ErrorId].Action) {
	case XPLMI_EM_ACTION_POR:
		XPlmi_PORHandler();
		break;
	case XPLMI_EM_ACTION_SRST:
		XPlmi_SoftResetHandler();
		break;
	case XPLMI_EM_ACTION_ERROUT:
		/*
		 * Clear PSM error and trigger error out using PMC FW_CR error
		 */
		(void)XPlmi_UpdateNumErrOutsCount(XPLMI_UPDATE_TYPE_INCREMENT);
		XPlmi_EmClearError(ErrorNodeType, ErrorId);
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_TRIG,
				PMC_GLOBAL_PMC_ERR1_TRIG_FW_CR_MASK);
		XPlmi_Printf(DEBUG_GENERAL, "FW_CR error out is triggered due to "
				"Error ID: 0x%x\r\n", ErrorId);
		break;
	case XPLMI_EM_ACTION_CUSTOM:
	case XPLMI_EM_ACTION_SUBSYS_SHUTDN:
	case XPLMI_EM_ACTION_SUBSYS_RESTART:
		(void)XPlmi_EmDisable(ErrorNodeId, RegMask);
		if (ErrorTable[ErrorId].Handler != NULL) {
			ErrorTable[ErrorId].Handler(ErrorNodeId, RegMask);
		}
		XPlmi_EmClearError(ErrorNodeType, ErrorId);
		break;
	default:
		XPlmi_EmClearError(ErrorNodeType, ErrorId);
		XPlmi_Printf(DEBUG_GENERAL, "Invalid Error Action "
				"for PSM errors. Error ID: 0x%x\r\n", ErrorId);
		break;
	}
}

/****************************************************************************/
/**
* @brief    This function handles the Software error triggered from within PLM.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return   None
*
****************************************************************************/
void XPlmi_HandleSwError(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrorId = XPlmi_GetErrorId(ErrorNodeId, RegMask);

	if ((ErrorNodeId == XIL_NODETYPE_EVENT_ERROR_SW_ERR) &&
			(ErrorId < XPLMI_ERROR_SW_ERR_MAX) &&
			(ErrorId >= XPLMI_ERROR_HB_MON_0)) {
		switch (ErrorTable[ErrorId].Action) {
		case XPLMI_EM_ACTION_POR:
			XPlmi_PORHandler();
			break;
		case XPLMI_EM_ACTION_SRST:
			XPlmi_SoftResetHandler();
			break;
		case XPLMI_EM_ACTION_ERROUT:
			/*
			 * Trigger error out using PMC FW_CR error
			 */
			(void)XPlmi_UpdateNumErrOutsCount(XPLMI_UPDATE_TYPE_INCREMENT);
			XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_TRIG,
					PMC_GLOBAL_PMC_ERR1_TRIG_FW_CR_MASK);
			XPlmi_Printf(DEBUG_GENERAL, "FW_CR error out is triggered due to "
					"Error ID: 0x%x\r\n", ErrorId);
			break;
		case XPLMI_EM_ACTION_CUSTOM:
		case XPLMI_EM_ACTION_SUBSYS_SHUTDN:
		case XPLMI_EM_ACTION_SUBSYS_RESTART:
			(void)XPlmi_EmDisable(ErrorNodeId, RegMask);
			if (ErrorTable[ErrorId].Handler != NULL) {
				ErrorTable[ErrorId].Handler(ErrorNodeId, RegMask);
			}
			break;
		case XPLMI_EM_ACTION_NONE:
			XPlmi_Printf(DEBUG_GENERAL, "No action is enabled for "
					"Error ID: 0x%x\r\n", ErrorId);
			break;
		default:
			XPlmi_Printf(DEBUG_GENERAL, "Invalid Error Action "
					"for software errors. Error ID: 0x%x\r\n", ErrorId);
			break;
		}
	} else {
		XPlmi_Printf(DEBUG_GENERAL, "Invalid SW Error Node: 0x%x and ErrorId: 0x%x\r\n",
									ErrorNodeId, ErrorId);
	}
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
static void XPlmi_HandleLinkDownError(u32 Cpm5PcieIrStatusReg,
		u32 Cpm5DmaCsrIntDecReg, u32 ProcId)
{
	int Status = XST_FAILURE;
	u32 PcieLocalErr = XPlmi_In32(Cpm5PcieIrStatusReg);
	u8 PcieLocalErrEnable = (u8)((~XPlmi_In32(Cpm5PcieIrStatusReg + 4U)) &
			CPM5_SLCR_PCIE_IR_STATUS_PCIE_LOCAL_ERR_MASK);
	u32 LinkDownErr = XPlmi_In32(Cpm5DmaCsrIntDecReg);
	u8 LinkDownErrEnable = (u8)(XPlmi_In32(Cpm5DmaCsrIntDecReg + 4U) &
			CPM5_DMA_CSR_LINK_DOWN_MASK);

	/*
	 * Check if PCIE local error is enabled and
	 * Check if received error is PCIE local error
	 */
	if ((PcieLocalErrEnable != 0U) &&
			((PcieLocalErr & CPM5_SLCR_PCIE_IR_STATUS_PCIE_LOCAL_ERR_MASK) ==
					CPM5_SLCR_PCIE_IR_STATUS_PCIE_LOCAL_ERR_MASK)) {
		/*
		 * Check if link down error is enabled and
		 * Check if received error is link down error
		 */
		if ((LinkDownErrEnable != 0U) &&
				((LinkDownErr & CPM5_DMA_CSR_LINK_DOWN_MASK) ==
						CPM5_DMA_CSR_LINK_DOWN_MASK)) {
			/* Execute proc for PCIE link down */
			Status = XPlmi_ExecuteProc(ProcId);
			if (Status != XST_SUCCESS) {
				XPlmi_Printf(DEBUG_GENERAL, "Error in handling PCIE "
						"link down event: 0x%x\r\n", Status);
				/*
				 * Update error manager with error received
				 * while executing proc
				 */
				XPlmi_ErrMgr(Status);
			}
			/* Clear PCIE link down error */
			XPlmi_Out32(Cpm5DmaCsrIntDecReg, CPM5_DMA_CSR_LINK_DOWN_MASK);

			/* Clear PCIE local event */
			XPlmi_Out32(Cpm5PcieIrStatusReg,
					CPM5_SLCR_PCIE_IR_STATUS_PCIE_LOCAL_ERR_MASK);
		} else {
			/* Received error is other than Link down error */
			XPlmi_Printf(DEBUG_GENERAL, "Received error is other than "
					"link down event: 0x%x\r\n", LinkDownErr);
		}
	} else {
		/* Received error is other than PCIE local event */
		XPlmi_Printf(DEBUG_GENERAL, "Received error is other than "
				"PCIE local event: 0x%x\r\n", PcieLocalErr);
	}
}

/****************************************************************************/
/**
* @brief    This function handles the CPM_NCR error.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return   None
*
****************************************************************************/
static void XPlmi_CpmErrHandler(u32 ErrorNodeId, u32 RegMask)
{
	u32 CpmErrors = XPlmi_In32(CPM5_SLCR_PS_UNCORR_IR_STATUS);
	u8 PciexErrEnable = (u8)((~XPlmi_In32(CPM5_SLCR_PS_UNCORR_IR_MASK)) &
			(CPM5_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK |
				CPM5_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK));

	(void)ErrorNodeId;
	(void)RegMask;

	/* Check if PCIE0/1 errors are enabled */
	if (PciexErrEnable != 0U) {
		/* Check if CPM_NCR error is PCIE0 error */
		if ((CpmErrors & CPM5_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK) ==
				CPM5_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK) {
			/* Handle PCIE0 link down error */
			XPlmi_Printf(DEBUG_GENERAL, "Received CPM PCIE0 interrupt\r\n");
			XPlmi_HandleLinkDownError(CPM5_SLCR_PCIE0_IR_STATUS,
					CPM5_DMA0_CSR_INT_DEC, CPM_NCR_PCIE0_LINK_DOWN_PROC_ID);
			/* Clear PCIE0 error */
			XPlmi_Out32(CPM5_SLCR_PS_UNCORR_IR_STATUS,
					CPM5_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK);
		}

		/* Check if CPM_NCR error is PCIE1 error */
		if ((CpmErrors & CPM5_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK) ==
				CPM5_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK) {
			/* Handle PCIE1 link down error */
			XPlmi_Printf(DEBUG_GENERAL, "Received CPM PCIE1 interrupt\r\n");
			XPlmi_HandleLinkDownError(CPM5_SLCR_PCIE1_IR_STATUS,
					CPM5_DMA1_CSR_INT_DEC, CPM_NCR_PCIE1_LINK_DOWN_PROC_ID);
			/* Clear PCIE1 error */
			XPlmi_Out32(CPM5_SLCR_PS_UNCORR_IR_STATUS,
					CPM5_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK);
		}
	} else {
		/* Only PCIE0/1 errors are handled */
		XPlmi_Printf(DEBUG_GENERAL, "Unhandled CPM_NCR error: 0x%x\r\n",
				CpmErrors);
	}
}

/****************************************************************************/
/**
* @brief    This function handles the XPPU errors
*
* @param    BaseAddr is the base address of the XPPU
* @param    ProtUnitStr is string prefix to be used while printing event info
*
* @return   None
*
****************************************************************************/
static void XPlmi_XppuErrHandler(u32 BaseAddr, const char *ProtUnitStr)
{
	u32 XppuErrStatus1 = XPlmi_In32(BaseAddr + XPPU_ERR_STATUS1);
	u32 XppuErrStatus2 = XPlmi_In32(BaseAddr + XPPU_ERR_STATUS2);
	u32 XppuErrors = XPlmi_In32(BaseAddr + XPPU_ISR);

	if (NULL == ProtUnitStr) {
		ProtUnitStr = "";
	}

	/*
	 * ERR_ST1 is the upper 20 bits of violated transaction address
	 * ERR_ST2 is the Master ID (i.e. SMID) of the violated transaction
	 * ISR is the interrupt status and clear for access violations
	 */
	XPlmi_Printf(DEBUG_GENERAL, "%s: ERR_ST1: 0x%08x, ERR_ST2: 0x%08x, ISR: 0x%08x\r\n",
			ProtUnitStr, XppuErrStatus1, XppuErrStatus2, XppuErrors);
}

/****************************************************************************/
/**
* @brief    This function handles the XMPU errors
*
* @param    BaseAddr is the base address of the XMPU
* @param    ProtUnitStr is string prefix to be used while printing event info
*
* @return   None
*
****************************************************************************/
static void XPlmi_XmpuErrHandler(u32 BaseAddr, const char *ProtUnitStr)
{
	u32 XmpuErr1lo = XPlmi_In32(BaseAddr + XMPU_ERR_STATUS1_LO);
	u32 XmpuErr1hi = XPlmi_In32(BaseAddr + XMPU_ERR_STATUS1_HI);
	u32 XmpuErrStatus2 = XPlmi_In32(BaseAddr + XMPU_ERR_STATUS2);
	u32 XmpuErrors = XPlmi_In32(BaseAddr + XMPU_ISR);

	if (NULL == ProtUnitStr) {
		ProtUnitStr = "";
	}

	/*
	 * ERR_ST1_LO is the lower bits of failed transaction address
	 * ERR_ST1_HI is the higher bits of failed transaction address
	 * ERR_ST2 is Master ID (i.e. SMID) of the failed transaction
	 * ISR is the interrupt status and clear for access violations
	 */
	XPlmi_Printf(DEBUG_GENERAL,
			"%s: ERR_ST1_LO: 0x%08x, ERR_ST1_HI: 0x%08x, ERR_ST2: 0x%08x, ISR: 0x%08x\r\n",
			ProtUnitStr, XmpuErr1lo, XmpuErr1hi, XmpuErrStatus2, XmpuErrors);
}

/****************************************************************************/
/**
* @brief    Top level action handler for the XPPU/XMPU errors
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return   None
*
****************************************************************************/
static void XPlmi_ProtUnitErrHandler(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrorId = XPlmi_GetErrorId(ErrorNodeId, RegMask);

	/*
	 * The nature of XPPU/XMPU errors is such that they often occur consecutively
	 * due to the stream of transactions. This may result in flooding the UART with
	 * prints and/or starvation of user tasks due to PLM's constant handling of these
	 * error events. Therefore, such events are handled only the first time error
	 * occurs and these errors are not enabled again. The source is also not cleared
	 * because doing so may interfere with user's handling of these errors.
	 */
	switch (ErrorId) {
	case XPLMI_ERROR_PMCXPPU:
		XPlmi_XppuErrHandler(PMC_XPPU_BASEADDR, "PMC_XPPU");
		XPlmi_XppuErrHandler(PMC_XPPU_NPI_BASEADDR, "PMC_XPPU_NPI");
		break;
	case XPLMI_ERROR_LPD_XPPU:
		XPlmi_XppuErrHandler(LPD_XPPU_BASEADDR, "LPD_XPPU");
		break;
	case XPLMI_ERROR_PMCXMPU:
		XPlmi_XmpuErrHandler(PMC_XMPU_BASEADDR, "PMC_XMPU");
		break;
	case XPLMI_ERROR_LPD_XMPU:
		XPlmi_XmpuErrHandler(LPD_XMPU_BASEADDR, "LPD_XMPU");
		break;
	case XPLMI_ERROR_FPD_XMPU:
		XPlmi_XmpuErrHandler(FPD_XMPU_BASEADDR, "FPD_XMPU");
		break;
	default:
		/* Only XPPU/XMPU errors are handled */
		XPlmi_Printf(DEBUG_GENERAL, "Unhandled Error: Node: 0x%x, Mask: 0x%x\r\n",
				ErrorNodeId, RegMask);
		break;
	}
}

/****************************************************************************/
/**
* @brief    This function is the interrupt handler for PSM Errors.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return   None
*
****************************************************************************/
static void XPlmi_ErrPSMIntrHandler(u32 ErrorNodeId, u32 RegMask)
{
	u32 Err1Status;
	u32 Err2Status;
	u32 Err1CrMask;
	u32 Err1NcrMask;
	u32 Err2CrMask;
	u32 Err2NcrMask;
	u32 Err1Mask;
	u32 Err2Mask;
	u32 Index;
	u32 ErrRegMask;

	(void)ErrorNodeId;
	(void)RegMask;

	Err1Status = XPlmi_In32(PSM_GLOBAL_REG_PSM_ERR1_STATUS);
	Err2Status = XPlmi_In32(PSM_GLOBAL_REG_PSM_ERR2_STATUS);
	Err1CrMask = XPlmi_In32(PSM_GLOBAL_REG_PSM_CR_ERR1_MASK);
	Err2CrMask = XPlmi_In32(PSM_GLOBAL_REG_PSM_CR_ERR2_MASK);
	Err1NcrMask = XPlmi_In32(PSM_GLOBAL_REG_PSM_NCR_ERR1_MASK);
	Err2NcrMask = XPlmi_In32(PSM_GLOBAL_REG_PSM_NCR_ERR2_MASK);
	Err1Mask = Err1CrMask & Err1NcrMask;
	Err2Mask = Err2CrMask & Err2NcrMask;

	if (Err1Status != 0U) {
		for (Index = XPLMI_ERROR_PS_SW_CR;
				Index < XPLMI_ERROR_PSMERR1_MAX; Index++) {
			ErrRegMask = XPlmi_ErrRegMask(Index);
			if (((Err1Status & ErrRegMask) != (u32)FALSE) &&
				((Err1Mask & ErrRegMask) == 0x0U) &&
				(ErrorTable[Index].Action != XPLMI_EM_ACTION_NONE)) {
				XPlmi_HandlePsmError(
					XIL_NODETYPE_EVENT_ERROR_PSM_ERR1, ErrRegMask);
			}
		}
	}
	if (Err2Status != 0U) {
		for (Index = XPLMI_ERROR_LPD_SWDT;
				Index < XPLMI_ERROR_PSMERR2_MAX; Index++) {
			ErrRegMask = XPlmi_ErrRegMask(Index);
			if (((Err2Status & ErrRegMask) != (u32)FALSE) &&
				((Err2Mask & ErrRegMask) == 0x0U) &&
				 (ErrorTable[Index].Action != XPLMI_EM_ACTION_NONE)) {
				XPlmi_HandlePsmError(
					XIL_NODETYPE_EVENT_ERROR_PSM_ERR2, ErrRegMask);
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
* @param    RegMask is the register mask of the error received
*
* @return   None
*
****************************************************************************/
static void XPlmi_ErrIntrSubTypeHandler(u32 ErrorNodeId, u32 RegMask)
{
	int Status = XST_FAILURE;
	u32 ActionId;
	u32 ErrorId = XPlmi_GetErrorId(ErrorNodeId, RegMask);

	ActionId = ErrorTable[ErrorId].Action;

	switch (ActionId) {
	case XPLMI_EM_ACTION_SUBSYS_SHUTDN:
		XPlmi_Printf(DEBUG_GENERAL, "System shutdown 0x%x\r\n", ErrorTable[ErrorId].SubsystemId);
		Status = (*PmSystemShutdown)(ErrorTable[ErrorId].SubsystemId,
				XPLMI_SUBSYS_SHUTDN_TYPE_SHUTDN, 0U,
				XPLMI_CMD_SECURE);
		break;
	case XPLMI_EM_ACTION_SUBSYS_RESTART:
		XPlmi_Printf(DEBUG_GENERAL, "Subsystem Restart 0x%x\r\n", ErrorTable[ErrorId].SubsystemId);
		Status = (*PmSubsysRestart)(ErrorTable[ErrorId].SubsystemId);
		break;
	default:
		Status = XPLMI_INVALID_ERROR_ACTION;
		break;
	}

	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_GENERAL, "Error action 0x%x failed for "
				"Error: 0x%x\r\n", ActionId, ErrorId);
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
	u32 Err1IrqMask;
	u32 Err2IrqMask;
	u32 Index;
	u32 RegMask;

	(void)CallbackRef;

	Err1Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
	Err2Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
	Err1IrqMask = XPlmi_In32(PMC_GLOBAL_PMC_IRQ1_MASK);
	Err2IrqMask = XPlmi_In32(PMC_GLOBAL_PMC_IRQ2_MASK);

	XPlmi_Printf(DEBUG_GENERAL,
		"PMC EAM Interrupt: ERR1: 0x%0x, ERR2: 0x%0x\n\r",
			Err1Status, Err2Status);
	/*
	 * Interrupt is selected as response for Custom, subsystem shutdown
	 * and subsystem restart actions. For these actions, error will be
	 * disabled. Agent should clear the source and enable the error again
	 * using SetAction. In SetAction, error will be cleared and enabled.
	 * For subsystem cases, during subsystem restart, error will be enabled
	 * again.
	 */

	if (Err1Status != 0U) {
		for (Index = XPLMI_ERROR_BOOT_CR;
				Index < XPLMI_ERROR_PMCERR1_MAX; Index++) {
			RegMask = XPlmi_ErrRegMask(Index);
			if (((Err1Status & RegMask) != (u32)FALSE) &&
				((Err1IrqMask & RegMask) == 0x0U) &&
					((ErrorTable[Index].Handler != NULL) ||
					(Index == XPLMI_ERROR_PMC_PSM_NCR)) &&
					(ErrorTable[Index].Action != XPLMI_EM_ACTION_NONE)) {
				/* PSM errors are handled in PsmErrHandler */
				if (Index != XPLMI_ERROR_PMC_PSM_NCR) {
					(void)XPlmi_EmDisable(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1,
							RegMask);
					ErrorTable[Index].Handler(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1,
							RegMask);
				}
				else {
					XPlmi_ErrPSMIntrHandler(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1,
							RegMask);
				}
				XPlmi_EmClearError((u32)XPLMI_NODETYPE_EVENT_PMC_ERR1, Index);
			}
		}
	}

	if (Err2Status != 0U) {
		for (Index = XPLMI_ERROR_PMCAPB;
				Index < XPLMI_ERROR_PMCERR2_MAX; Index++) {
			RegMask = XPlmi_ErrRegMask(Index);
			if (((Err2Status & RegMask) != (u32)FALSE) &&
				((Err2IrqMask & RegMask) == 0x0U) &&
				(ErrorTable[Index].Handler != NULL) &&
				(ErrorTable[Index].Action != XPLMI_EM_ACTION_NONE)) {
				if ((RegMask & PMC_GLOBAL_SSIT_ERR_MASK) == 0x0U) {
					(void)XPlmi_EmDisable(XIL_NODETYPE_EVENT_ERROR_PMC_ERR2,
							RegMask);
				}
				ErrorTable[Index].Handler(
					XIL_NODETYPE_EVENT_ERROR_PMC_ERR2, RegMask);
				XPlmi_EmClearError((u32)XPLMI_NODETYPE_EVENT_PMC_ERR2, Index);
			}
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function clears any previous errors before enabling them.
 *
 * @param	ErrorNodeType is the node type for the error event
 * @param	ErrorId is the index of the error to be cleared
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_EmClearError(u32 ErrorNodeType, u32 ErrorId)
{
	u32 RegMask = XPlmi_ErrRegMask(ErrorId);
	u32 NumErrOuts = 0U;

	switch ((XPlmi_EventType)ErrorNodeType) {
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
		/* If action is error out, clear PMC FW_CR error */
		if ((ErrorTable[ErrorId].Action == XPLMI_EM_ACTION_ERROUT) &&
			(ErrorTable[XPLMI_ERROR_PMC_PSM_CR].Action !=
					XPLMI_EM_ACTION_ERROUT)) {
			NumErrOuts =
					XPlmi_UpdateNumErrOutsCount(XPLMI_UPDATE_TYPE_DECREMENT);
			if (NumErrOuts == 0U) {
				XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS,
						PMC_GLOBAL_PMC_ERR1_TRIG_FW_CR_MASK);
			}
		}
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		/* Clear previous errors */
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_ERR2_STATUS, RegMask);
		/* If action is error out, clear PMC FW_CR error */
		if ((ErrorTable[ErrorId].Action == XPLMI_EM_ACTION_ERROUT) &&
			(ErrorTable[XPLMI_ERROR_PMC_PSM_CR].Action !=
					XPLMI_EM_ACTION_ERROUT)) {
			NumErrOuts =
					XPlmi_UpdateNumErrOutsCount(XPLMI_UPDATE_TYPE_DECREMENT);
			if (NumErrOuts == 0U) {
				XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS,
						PMC_GLOBAL_PMC_ERR1_TRIG_FW_CR_MASK);
			}
		}
		break;
	case XPLMI_NODETYPE_EVENT_SW_ERR:
		/* Clear previous erros */
		if (ErrorTable[ErrorId].Action == XPLMI_EM_ACTION_ERROUT) {
			NumErrOuts =
					XPlmi_UpdateNumErrOutsCount(XPLMI_UPDATE_TYPE_DECREMENT);
			if (NumErrOuts == 0U) {
				XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS,
						PMC_GLOBAL_PMC_ERR1_TRIG_FW_CR_MASK);
			}
		}
		break;
	default:
		/* Invalid Error Type */
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType 0x%x for Error Mask: 0x%0x\n\r",
			ErrorNodeType, RegMask);
		break;
	}
}

/*****************************************************************************/
/**
 * @brief	This function disables the error action for the given error mask.
 *
 * @param	ErrMaskRegAddr is the error action mask register address
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static u32 EmDisableErrAction(u32 ErrMaskRegAddr, u32 RegMask)
{
	int Status = XPLMI_ERROR_ACTION_NOT_DISABLED;

	/* Disable error action */
	XPlmi_Out32((ErrMaskRegAddr + PMC_PSM_DIS_REG_OFFSET), RegMask);
	/* Check if the error action is disabled */
	if ((XPlmi_In32(ErrMaskRegAddr) & RegMask) == RegMask) {
		Status = XST_SUCCESS;
	}

	return (u32)Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables the error action for the given error mask.
 *
 * @param	ErrMaskRegAddr is the error action mask register address
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int EmEnableErrAction(u32 ErrMaskRegAddr, u32 RegMask)
{
	int Status = XPLMI_ERROR_ACTION_NOT_ENABLED;

	/* Enable the error action */
	XPlmi_Out32((ErrMaskRegAddr + PMC_PSM_EN_REG_OFFSET), RegMask);
	/* Check if the error action is enabled */
	if ((XPlmi_In32(ErrMaskRegAddr) & RegMask) == 0x0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function disables the PMC error actions for the given mask.
 *
 * @param	PmcErrRegOffset is the offset for the PMC ERR1 and ERR2 errors
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_EmDisablePmcErrors(u32 RegOffset, u32 RegMask)
{
	u32 Status = (u32)XPLMI_ERROR_ACTION_NOT_DISABLED;

	/* Disable all PMC error actions */
	Status = EmDisableErrAction((PMC_GLOBAL_PMC_ERR_OUT1_MASK + RegOffset),
			RegMask);
	Status |= EmDisableErrAction((PMC_GLOBAL_PMC_POR1_MASK + RegOffset),
			RegMask);
	Status |= EmDisableErrAction((PMC_GLOBAL_PMC_IRQ1_MASK + RegOffset),
			RegMask);
	Status |= EmDisableErrAction((PMC_GLOBAL_PMC_SRST1_MASK + RegOffset),
			RegMask);

	return (int)Status;
}

/*****************************************************************************/
/**
 * @brief	This function disables the PSM error actions for the given mask.
 *
 * @param	PsmErrRegOffset is the offset for the PSM ERR1 and ERR2 errors
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_EmDisablePsmErrors(u32 RegOffset, u32 RegMask)
{
	u32 Status = (u32)XPLMI_ERROR_ACTION_NOT_DISABLED;

	/* Disable all PSM error actions */
	Status = EmDisableErrAction((PSM_GLOBAL_REG_PSM_CR_ERR1_MASK + RegOffset),
			RegMask);
	Status |= EmDisableErrAction((PSM_GLOBAL_REG_PSM_NCR_ERR1_MASK + RegOffset),
			RegMask);
	Status |= EmDisableErrAction((PSM_GLOBAL_REG_PSM_IRQ1_MASK + RegOffset),
			RegMask);

	return (int)Status;
}

/*****************************************************************************/
/**
 * @brief	This function disables the responses for the given error.
 *
 * @param	ErrorNodeId is the node Id for the error event
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_EmDisable(u32 ErrorNodeId, u32 RegMask)
{
	int Status = XPLMI_ERROR_ACTION_NOT_DISABLED;

	switch ((XPlmi_EventType)XPlmi_EventNodeType(ErrorNodeId)) {
	case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		/* Disable POR, SRST, Interrupt and PS Error Out */
		Status = XPlmi_EmDisablePmcErrors(0x0U, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PMC_ERR2:
		/* Disable POR, SRST, Interrupt and PS Error Out */
		Status = XPlmi_EmDisablePmcErrors(PMC_PSM_ERR2_REG_OFFSET, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		/* Disable CR / NCR to PMC, Interrupt */
		Status = XPlmi_EmDisablePsmErrors(0x0U, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		/* Disable CR / NCR to PMC, Interrupt */
		Status = XPlmi_EmDisablePsmErrors(PMC_PSM_ERR2_REG_OFFSET, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_SW_ERR:
		/* Do nothing */
		Status = XST_SUCCESS;
		break;
	default:
		/* Invalid Error Type */
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType 0x%x for Error Mask: 0x%0x\n\r",
			ErrorNodeId, RegMask);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables the POR response for the given Error.
 *
 * @param	ErrorNodeType is the node type for the error event
 * @param	RegMask is the register mask of the error to be enabled
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_EmEnablePOR(u32 ErrorNodeType, u32 RegMask)
{
	int Status = XPLMI_ERROR_ACTION_NOT_ENABLED;

	switch ((XPlmi_EventType)ErrorNodeType) {
	case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		/* Enable POR error action for given error mask */
		Status = EmEnableErrAction(PMC_GLOBAL_PMC_POR1_MASK, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PMC_ERR2:
		/* Enable POR error action for given error mask */
		Status = EmEnableErrAction(PMC_GLOBAL_PMC_POR2_MASK, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		/*
		 * If PMC PSM CR error action is POR, set the error action for the
		 * given error as PSM CR to handle the action by HW.
		 * Otherwise, set it as PSM NCR to handle the action by PLM.
		 */
		if (ErrorTable[XPLMI_ERROR_PMC_PSM_CR].Action ==
				XPLMI_EM_ACTION_POR) {
			/* Enable PSM CR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_CR_ERR1_MASK, RegMask);
		} else {
			/* Enable PSM NCR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_NCR_ERR1_MASK, RegMask);
		}
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		/*
		 * If PMC PSM CR error action is POR, set the error action for the
		 * given error as PSM CR to handle the action by HW.
		 * Otherwise, set it as PSM NCR to handle the action by PLM.
		 */
		if (ErrorTable[XPLMI_ERROR_PMC_PSM_CR].Action ==
				XPLMI_EM_ACTION_POR) {
			/* Enable PSM CR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_CR_ERR2_MASK, RegMask);
		} else {
			/* Enable PSM NCR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_NCR_ERR2_MASK, RegMask);
		}
		break;
	case XPLMI_NODETYPE_EVENT_SW_ERR:
		/* Do nothing */
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType 0x%x for Error Mask: 0x%0x\n\r",
			ErrorNodeType, RegMask);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables the SRST response for the given Error ID.
 *
 * @param	ErrorNodeType is the node type for the error event
 * @param	RegMask is the register mask of the error to be enabled
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_EmEnableSRST(u32 ErrorNodeType, u32 RegMask)
{
	int Status = XPLMI_ERROR_ACTION_NOT_ENABLED;

	switch ((XPlmi_EventType)ErrorNodeType) {
	case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		/* Enable SRST error action for given error mask */
		Status = EmEnableErrAction(PMC_GLOBAL_PMC_SRST1_MASK, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PMC_ERR2:
		/* Enable SRST error action for given error mask */
		Status = EmEnableErrAction(PMC_GLOBAL_PMC_SRST2_MASK, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		/*
		 * If PMC PSM CR error action is SRST, set the error action for the
		 * given error as PSM CR to handle the action by HW.
		 * Otherwise, set it as PSM NCR to handle the action by PLM.
		 */
		if (ErrorTable[XPLMI_ERROR_PMC_PSM_CR].Action ==
				XPLMI_EM_ACTION_SRST) {
			/* Enable PSM CR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_CR_ERR1_MASK, RegMask);
		} else {
			/* Enable PSM NCR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_NCR_ERR1_MASK, RegMask);
		}
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		/*
		 * If PMC PSM CR error action is SRST, set the error action for the
		 * given error as PSM CR to handle the action by HW.
		 * Otherwise, set it as PSM NCR to handle the action by PLM.
		 */
		if (ErrorTable[XPLMI_ERROR_PMC_PSM_CR].Action ==
				XPLMI_EM_ACTION_SRST) {
			/* Enable PSM CR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_CR_ERR2_MASK, RegMask);
		} else {
			/* Enable PSM NCR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_NCR_ERR2_MASK, RegMask);
		}
		break;
	case XPLMI_NODETYPE_EVENT_SW_ERR:
		/* Do nothing */
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType 0x%x for Error Mask: 0x%0x\n\r",
			ErrorNodeType, RegMask);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables the ERR OUT response for the given Error ID.
 *
 * @param	ErrorNodeType is the node type for the error event
 * @param	RegMask is the register mask of the error to be enabled
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_EmEnablePSError(u32 ErrorNodeType, u32 RegMask)
{
	int Status = XPLMI_ERROR_ACTION_NOT_ENABLED;

	/* Enable the specified Error to propagate to ERROUT pin	*/
	switch ((XPlmi_EventType)ErrorNodeType) {
	case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		/* Enable ERROUT error action for given error mask */
		Status = EmEnableErrAction(PMC_GLOBAL_PMC_ERR_OUT1_MASK, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PMC_ERR2:
		/* Enable ERROUT error action for given error mask */
		Status = EmEnableErrAction(PMC_GLOBAL_PMC_ERR_OUT2_MASK, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		/*
		 * If PMC PSM CR error action is ERROUT, set the error action for the
		 * given error as PSM CR to handle the action by HW.
		 * Otherwise, set it as PSM NCR to handle the action by PLM.
		 */
		if (ErrorTable[XPLMI_ERROR_PMC_PSM_CR].Action ==
				XPLMI_EM_ACTION_ERROUT) {
			/* Enable PSM CR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_CR_ERR1_MASK, RegMask);
		} else {
			/* Enable PSM NCR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_NCR_ERR1_MASK, RegMask);
		}
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		/*
		 * If PMC PSM CR error action is ERROUT, set the error action for the
		 * given error as PSM CR to handle the action by HW.
		 * Otherwise, set it as PSM NCR to handle the action by PLM.
		 */
		if (ErrorTable[XPLMI_ERROR_PMC_PSM_CR].Action ==
				XPLMI_EM_ACTION_ERROUT) {
			/* Enable PSM CR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_CR_ERR2_MASK, RegMask);
		} else {
			/* Enable PSM NCR error action for given error mask */
			Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_NCR_ERR2_MASK, RegMask);
		}
		break;
	case XPLMI_NODETYPE_EVENT_SW_ERR:
		/*
		 * Do nothing
		 * Error out will be trigerred through software using
		 * PMC_CR error trigger bit. PMC_CR is configured
		 * to error out by default and is not configurable.
		 */
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType 0x%x for Error Mask: 0x%0x\n\r",
			ErrorNodeType, RegMask);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables the interrupt to PMC for the given Error ID.
 *
 * @param	ErrorNodeType is the node type for the error event
 * @param	RegMask is the register mask of the error to be enabled
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_EmEnableInt(u32 ErrorNodeType, u32 RegMask)
{
	int Status = XPLMI_ERROR_ACTION_NOT_ENABLED;

	switch ((XPlmi_EventType)ErrorNodeType) {
	case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		/* Enable IRQ error action for given error mask */
		Status = EmEnableErrAction(PMC_GLOBAL_PMC_IRQ1_MASK, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PMC_ERR2:
		/* Enable IRQ error action for given error mask */
		Status = EmEnableErrAction(PMC_GLOBAL_PMC_IRQ2_MASK, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		/* Enable PSM NCR error action for given error mask */
		Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_NCR_ERR1_MASK, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		/* Enable PSM NCR error action for given error mask */
		Status = EmEnableErrAction(PSM_GLOBAL_REG_PSM_NCR_ERR2_MASK, RegMask);
		break;
	case XPLMI_NODETYPE_EVENT_SW_ERR:
		/* Do nothing */
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType 0x%x for Error Mask: 0x%0x\n\r",
			ErrorNodeType, RegMask);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures the Action specified for a given Error ID.
 *
 * @param	NodeType is the error node type
 * @param	ErrorId is the index of the error to which given action to be set
 * @param	ActionId is the action that need to be set for ErrorId. Action
 * 		  	can be SRST/POR/ERR OUT/INT
 * @param	ErrorHandler If INT is defined as response, handler should be
 * 		  	defined.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_EmConfig(u32 NodeType, u32 ErrorId, u8 ActionId,
		XPlmi_ErrorHandler_t ErrorHandler)
{
	int Status = XST_FAILURE;
	u32 RegMask = XPlmi_ErrRegMask(ErrorId);

	/* Set error action for given error Id */
	switch (ActionId) {
		case XPLMI_EM_ACTION_NONE:
			/* No Action */
			ErrorTable[ErrorId].Action = ActionId;
			Status = XST_SUCCESS;
			break;
		case XPLMI_EM_ACTION_POR:
			/* Set the error action and enable it */
			ErrorTable[ErrorId].Action = ActionId;
			Status = XPlmi_EmEnablePOR(NodeType, RegMask);
			break;
		case XPLMI_EM_ACTION_SRST:
			/* Set error action SRST for the errorId */
			ErrorTable[ErrorId].Action = ActionId;
			Status = XPlmi_EmEnableSRST(NodeType, RegMask);
			break;
		case XPLMI_EM_ACTION_CUSTOM:
			/* Set custom handler as error action for the errorId */
			ErrorTable[ErrorId].Action = ActionId;
			ErrorTable[ErrorId].Handler = ErrorHandler;
			Status = XPlmi_EmEnableInt(NodeType, RegMask);
			break;
		case XPLMI_EM_ACTION_ERROUT:
			ErrorTable[ErrorId].Action = ActionId;
			/* Set error action ERROUT signal for the errorId */
			Status = XPlmi_EmEnablePSError(NodeType, RegMask);
			break;
		case XPLMI_EM_ACTION_SUBSYS_SHUTDN:
		case XPLMI_EM_ACTION_SUBSYS_RESTART:
			/* Set handler and error action for the errorId */
			ErrorTable[ErrorId].Action = ActionId;
			ErrorTable[ErrorId].Handler = XPlmi_ErrIntrSubTypeHandler;
			ErrorTable[ErrorId].SubsystemId = EmSubsystemId;
			Status = XPlmi_EmEnableInt(NodeType, RegMask);
			break;
		default:
			/* Invalid Action Id */
			Status = XPLMI_INVALID_ERROR_ACTION;
			XPlmi_Printf(DEBUG_GENERAL,
					"Invalid ActionId for Error: 0x%0x\n\r", ErrorId);
			break;
		}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets the Action specified for a given Error Masks.
 *
 * @param	ErrorNodeId is the node ID for the error event
 * @param	ErrorMasks is the error masks to which specified action to be set
 * @param	ActionId is the action that need to be set for ErrorMasks. Action
 * 		  	can be SRST/POR/ERR OUT/INT
 * @param	ErrorHandler If INT is defined as response, handler should be
 * 		  	defined.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_EmSetAction(u32 ErrorNodeId, u32 ErrorMasks, u8 ActionId,
		XPlmi_ErrorHandler_t ErrorHandler)
{
	int Status = XST_FAILURE;
	u32 NodeType =  XPlmi_EventNodeType(ErrorNodeId);
	u32 ErrorId = NodeType * (u32)XPLMI_MAX_ERR_BITS;
	u32 RegMask;
	u32 ErrMasks = ErrorMasks;

	for ( ; ErrMasks != 0U; ErrMasks >>= 1U) {
		if ((ErrMasks & 0x1U) == 0U) {
			goto END;
		}
		RegMask = XPlmi_ErrRegMask(ErrorId);

		/* Check for Valid Error ID */
		if ((ErrorId >= XPLMI_ERROR_SW_ERR_MAX) ||
				(ErrorTable[ErrorId].Action == XPLMI_EM_ACTION_INVALID)) {
			/* Invalid Error Id */
			Status = XPLMI_INVALID_ERROR_ID;
			XPlmi_Printf(DEBUG_GENERAL,
					"Invalid Error: 0x%0x\n\r", ErrorId);
			goto END;
		}

		if((XPLMI_EM_ACTION_CUSTOM == ActionId) && (NULL == ErrorHandler) &&
				(XPLMI_ERROR_PMC_PSM_NCR != ErrorId)) {
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
			Status = XPlmi_EmDisable(ErrorNodeId, RegMask);
			if (XST_SUCCESS != Status) {
				/* Error action disabling failure */
				goto END;
			}
			/* Clear any previous errors */
			XPlmi_EmClearError(NodeType, ErrorId);
		}

		/* Configure the Error Action to given Error Id */
		Status = XPlmi_EmConfig(NodeType, ErrorId, ActionId, ErrorHandler);

END:
		++ErrorId;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the error module. Disables all the error
 * actions and registers default action.
 *
 * @param	SystemShutdown is the pointer to the PM system shutdown
 *		callback handler for action subtype system shutdown
 * @param	SubsystemRestart is pointer to the PM subsystem restart
 *		with CPU idle support handler for action subtype restart
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_EmInit(XPlmi_ShutdownHandler_t SystemShutdown,
		  XPlmi_RestartHandler_t SubsystemRestart)
{
	u32 Index;
	u32 PmcErr1Status;
	u32 PmcErr2Status;
	u32 FwErr;
	u32 RegMask;
	u32 SiliconVal = XPlmi_In32(PMC_TAP_VERSION) &
			PMC_TAP_VERSION_PMC_VERSION_MASK;

	/* Register Error module commands */
	XPlmi_ErrModuleInit();

	/* Clear SSIT_ERR register to stop error propagation to other SLRs */
	XPlmi_Out32(PMC_GLOBAL_SSIT_ERR, 0x0U);

	/* Save FW_ERR register value to RTCA and clear it */
	FwErr = XPlmi_In32(PMC_GLOBAL_PMC_FW_ERR);
	XPlmi_Out32(XPLMI_RTCFG_PMC_FW_ERR_VAL_ADDR, FwErr);
	XPlmi_Out32(PMC_GLOBAL_PMC_FW_ERR, 0x0U);

	/* Disable all the Error Actions */
	XPlmi_Out32(PMC_GLOBAL_PMC_POR1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR_OUT1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_IRQ1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_SRST1_DIS, MASK32_ALL_HIGH);

	PmcErr1Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
	PmcErr2Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);

	/* Ignore SSIT Errors on ES1 */
	if (SiliconVal == XPLMI_SILICON_ES1_VAL) {
		PmcErr1Status &= ~(XPLMI_PMC_ERR1_SSIT_MASK);
		PmcErr2Status &= ~(XPLMI_PMC_ERR2_SSIT_MASK);
	}

	XPlmi_Out32(XPLMI_RTCFG_PMC_ERR1_STATUS_ADDR, PmcErr1Status);
	XPlmi_Out32(XPLMI_RTCFG_PMC_ERR2_STATUS_ADDR, PmcErr2Status);
	if (PmcErr1Status != 0U) {
		XPlmi_Printf(DEBUG_INFO, "PMC_GLOBAL_PMC_ERR1_STATUS: "
			"0x%08x\n\r", PmcErr1Status);
	}
	if (PmcErr2Status != 0U) {
		XPlmi_Printf(DEBUG_INFO, "PMC_GLOBAL_PMC_ERR2_STATUS: "
			"0x%08x\n\r", PmcErr2Status);
	}

	/* Clear the error status registers */
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS, MASK32_ALL_HIGH);

	/* Detect if we are in over-temperature condition */
	XPlmi_SysMonOTDetect(0U);

	PmSystemShutdown = SystemShutdown;
	PmSubsysRestart = SubsystemRestart;

	/* Set the default actions as defined in the Error table */
	for (Index = XPLMI_ERROR_BOOT_CR;
		Index < XPLMI_ERROR_PMCERR1_MAX; Index++) {
		if (ErrorTable[Index].Action != XPLMI_EM_ACTION_INVALID) {
			RegMask = XPlmi_ErrRegMask(Index);
			if (XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1, RegMask,
						ErrorTable[Index].Action,
						ErrorTable[Index].Handler) != XST_SUCCESS) {
				XPlmi_Printf(DEBUG_GENERAL,
						"Warning: XPlmi_EmInit: Failed to "
						"set action for PMC ERR1: %u\r\n", Index);
			}
		}
	}

	for (Index = XPLMI_ERROR_PMCAPB;
		Index < XPLMI_ERROR_PMCERR2_MAX; Index++) {
		if (ErrorTable[Index].Action != XPLMI_EM_ACTION_INVALID) {
			RegMask = XPlmi_ErrRegMask(Index);
			if (XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR2, RegMask,
						ErrorTable[Index].Action,
						ErrorTable[Index].Handler) != XST_SUCCESS) {
				XPlmi_Printf(DEBUG_GENERAL,
						"Warning: XPlmi_EmInit: Failed to "
						"set action for PMC ERR2: %u\r\n", Index);
			}
		}
	}
}

/****************************************************************************/
/**
* @brief    This function enables the SSIT interrupts
*
* @return   None
*
****************************************************************************/
void XPlmi_EnableSsitErrors(void)
{
	u8 SlrIndex;
	/*
	 * Set Custom action for SSIT Errors
	 * For Master SLR:
	 *   - SSIT_ERR0 is for the events from Slave SLR0
	 *   - SSIT_ERR1 is for the events from Slave SLR1
	 *   - SSIT_ERR2 is for the events from Slave SLR2
	 * For Slave SLRs:
	 *   - SSIT_ERR0 in Slave SLR0 is for the events from Master SLR
	 *   - SSIT_ERR1 in Slave SLR1 is for the events from Master SLR
	 *   - SSIT_ERR2 in Slave SLR2 is for the events from Master SLR
	 *
	 * Other SSIT errors in Master/Slave SLRs are not configured
	 */
	if (XPlmi_SsitIsIntrEnabled() == (u8)TRUE) {
		/* Interrupts are enabled already. No need to enable again. */
		goto END;
	}

	SlrIndex = XPlmi_GetSlrIndex();
	if (SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) {
		(void)XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR2,
				XIL_EVENT_ERROR_MASK_SSIT0, XPLMI_EM_ACTION_CUSTOM,
				XPlmi_SsitErrHandler);
		(void)XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR2,
				XIL_EVENT_ERROR_MASK_SSIT1, XPLMI_EM_ACTION_CUSTOM,
				XPlmi_SsitErrHandler);
		(void)XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR2,
				XIL_EVENT_ERROR_MASK_SSIT2, XPLMI_EM_ACTION_CUSTOM,
				XPlmi_SsitErrHandler);
	} else if (SlrIndex == XPLMI_SSIT_SLAVE0_SLR_INDEX) {
		(void)XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR2,
				XIL_EVENT_ERROR_MASK_SSIT0, XPLMI_EM_ACTION_CUSTOM,
				XPlmi_SsitErrHandler);
	} else if (SlrIndex == XPLMI_SSIT_SLAVE1_SLR_INDEX) {
		(void)XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR2,
				XIL_EVENT_ERROR_MASK_SSIT1, XPLMI_EM_ACTION_CUSTOM,
				XPlmi_SsitErrHandler);
	} else if (SlrIndex == XPLMI_SSIT_SLAVE2_SLR_INDEX) {
		(void)XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR2,
				XIL_EVENT_ERROR_MASK_SSIT2, XPLMI_EM_ACTION_CUSTOM,
				XPlmi_SsitErrHandler);
	} else {
		goto END;
	}
	XPlmi_Printf(DEBUG_GENERAL, "Enabled SSIT Interrupts\r\n");
	XPlmi_SsitSetIsIntrEnabled((u8)TRUE);

END:
	return;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the PSM error actions. Disables all the
 * PSM error actions and registers default action.
 *
 * @return	XST_SUCCESS
 *
*****************************************************************************/
int XPlmi_PsEmInit(void)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 PsmErr1Status;
	u32 PsmErr2Status;
	u32 RegMask;

	/* Disable all the Error Actions */
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_CR_ERR1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_IRQ1_DIS, MASK32_ALL_HIGH);

	PsmErr1Status = XPlmi_In32(PSM_GLOBAL_REG_PSM_ERR1_STATUS);
	PsmErr2Status = XPlmi_In32(PSM_GLOBAL_REG_PSM_ERR2_STATUS);
	XPlmi_Out32(XPLMI_RTCFG_PSM_ERR1_STATUS_ADDR, PsmErr1Status);
	XPlmi_Out32(XPLMI_RTCFG_PSM_ERR2_STATUS_ADDR, PsmErr2Status);
	if (PsmErr1Status != 0U) {
		XPlmi_Printf(DEBUG_GENERAL, "PSM_GLOBAL_REG_PSM_ERR1_STATUS: "
			"0x%08x\n\r", PsmErr1Status);
	}
	if (PsmErr2Status != 0U) {
		XPlmi_Printf(DEBUG_GENERAL, "PSM_GLOBAL_REG_PSM_ERR2_STATUS: "
			"0x%08x\n\r", PsmErr2Status);
	}

	/* Clear the error status registers */
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_ERR1_STATUS, MASK32_ALL_HIGH);
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_ERR2_STATUS, MASK32_ALL_HIGH);

	/* Set the default actions as defined in the Error table */
	for (Index = XPLMI_ERROR_PS_SW_CR;
		Index < XPLMI_ERROR_PSMERR1_MAX; Index++) {
		if (ErrorTable[Index].Action != XPLMI_EM_ACTION_INVALID) {
			RegMask = XPlmi_ErrRegMask(Index);
			if (XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PSM_ERR1, RegMask,
						ErrorTable[Index].Action,
						ErrorTable[Index].Handler) != XST_SUCCESS) {
				XPlmi_Printf(DEBUG_GENERAL,
						"Warning: XPlmi_PsEmInit: Failed to "
						"set action for PSM ERR1: %u\r\n", Index);
			}
		}
	}

	for (Index = XPLMI_ERROR_LPD_SWDT;
		Index < XPLMI_ERROR_PSMERR2_MAX; Index++) {
		if (ErrorTable[Index].Action != XPLMI_EM_ACTION_INVALID) {
			RegMask = XPlmi_ErrRegMask(Index);
			if (XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PSM_ERR2, RegMask,
						ErrorTable[Index].Action,
						ErrorTable[Index].Handler) != XST_SUCCESS) {
				XPlmi_Printf(DEBUG_GENERAL,
						"Warning: XPlmi_PsEmInit: Failed to "
						"set action for PSM ERR2: %u\r\n", Index);
			}
		}
	}
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function dumps the registers which can help debugging.
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_DumpRegisters(void)
{
	XPlmi_Printf(DEBUG_GENERAL, "============Register Dump============\n\r");

	XPlmi_Printf(DEBUG_GENERAL, "PMC_TAP_IDCODE: 0x%08x\n\r",
		XPlmi_In32(PMC_TAP_IDCODE));
	XPlmi_Printf(DEBUG_GENERAL, "EFUSE_CACHE_IP_DISABLE_0(EXTENDED IDCODE): "
			"0x%08x\n\r",
		XPlmi_In32(EFUSE_CACHE_IP_DISABLE_0));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_TAP_VERSION: 0x%08x\n\r",
		XPlmi_In32(PMC_TAP_VERSION));
	XPlmi_Printf(DEBUG_GENERAL, "CRP_BOOT_MODE_USER: 0x%08x\n\r",
		XPlmi_In32(CRP_BOOT_MODE_USER));
	XPlmi_Printf(DEBUG_GENERAL, "CRP_BOOT_MODE_POR: 0x%08x\n\r",
		XPlmi_In32(CRP_BOOT_MODE_POR));
	XPlmi_Printf(DEBUG_GENERAL, "CRP_RESET_REASON: 0x%08x\n\r",
		XPlmi_In32(CRP_RESET_REASON));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_PMC_MULTI_BOOT: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_PWR_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PWR_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_PMC_GSW_ERR: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PMC_GSW_ERR));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_PLM_ERR: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PLM_ERR));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_PMC_ERR1_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_PMC_ERR2_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_GICP0_IRQ_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP0_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_GICP1_IRQ_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP1_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_GICP2_IRQ_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP2_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_GICP3_IRQ_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP3_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_GICP4_IRQ_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP4_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_GICP_PMC_IRQ_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS));

	XPlmi_Printf(DEBUG_GENERAL, "============Register Dump============\n\r");
}

/*****************************************************************************/
/**
 * @brief	This function sets clock source to IRO for ES1 silicon and resets
 * the device.
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_SoftResetHandler(void)
{
	XPlmi_SysmonClkSetIro();
	/* Make sure every thing completes */
	DATA_SYNC;
	INST_SYNC;
	XPlmi_Out32(CRP_RST_PS, CRP_RST_PS_PMC_SRST_MASK);
	while (TRUE) {
		;
	}
}

/*****************************************************************************/
/**
 * @brief	This function sets clock source to IRO for ES1 silicon and triggers
 * FW NCR error.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_TriggerFwNcrError(void)
{
	if ((ErrorTable[XPLMI_ERROR_FW_NCR].Action == XPLMI_EM_ACTION_SRST) ||
		(ErrorTable[XPLMI_ERROR_FW_NCR].Action == XPLMI_EM_ACTION_POR)) {
		XPlmi_SysmonClkSetIro();
	}

	/* Trigger FW NCR error by setting NCR_Flag in FW_ERR register */
	XPlmi_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_NCR_FLAG_MASK,
			PMC_GLOBAL_PMC_FW_ERR_NCR_FLAG_MASK);
}

/*****************************************************************************/
/**
 * @brief	This function sets the sysmon clock to IRO for ES1 silicon
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_SysmonClkSetIro(void) {
	u32 SiliconVal = XPlmi_In32(PMC_TAP_VERSION) &
			PMC_TAP_VERSION_PMC_VERSION_MASK;

	if (SiliconVal == XPLMI_SILICON_ES1_VAL) {
		XPlmi_UtilRMW(CRP_SYSMON_REF_CTRL, CRP_SYSMON_REF_CTRL_SRCSEL_MASK,
			XPLMI_SYSMON_CLK_SRC_IRO_VAL);
	}
}

/*****************************************************************************/
/**
 * @brief	This function sets EmSubsystemId
 *
 * @param	Id pointer to set the EmSubsystemId
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_SetEmSubsystemId(const u32 *Id)
{
	EmSubsystemId = *Id;
}

/*****************************************************************************/
/**
 * @brief	This function updates NumErrOuts and returns number of error outs
 * count to the caller.
 *
 * @param	UpdateType is increment/decrement
 *
 * @return	Number of ErrOuts count
 *
 *****************************************************************************/
static u32 XPlmi_UpdateNumErrOutsCount(u8 UpdateType)
{
	static u32 NumErrOuts = 0U;

	if (UpdateType == XPLMI_UPDATE_TYPE_INCREMENT) {
		if (NumErrOuts < XPLMI_MAX_ERR_OUTS) {
			++NumErrOuts;
		}
	} else {
		if (NumErrOuts > 0U) {
			--NumErrOuts;
		}
	}

	return NumErrOuts;
}

/*****************************************************************************/
/**
 * @brief	This function clears NPI errors.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
******************************************************************************/
int XPlmi_CheckNpiErrors(void)
{
	int Status = XST_FAILURE;
	u32 ErrVal;
	u32 IsrVal = XPlmi_In32(NPI_NIR_REG_ISR);
	u32 ErrTypeVal = XPlmi_In32(NPI_NIR_ERR_TYPE);
	u32 ErrLogP0Info0Val = XPlmi_In32(NPI_NIR_ERR_LOG_P0_INFO_0);
	u32 ErrLogP0Info1Val = XPlmi_In32(NPI_NIR_ERR_LOG_P0_INFO_1);

	ErrVal =  IsrVal & NPI_NIR_REG_ISR_ERR_MASK;
	ErrVal |= ErrTypeVal & NPI_NIR_ERR_TYPE_ERR_MASK;
	if (ErrVal != 0U) {
		XPlmi_Printf(DEBUG_GENERAL, "NPI_NIR_ISR: 0x%08x\n\r"
			"NPI_NIR_ERR_TYPE: 0x%08x\n\r"
			"NPI_NIR_ERR_LOG_P0_INFO_0: 0x%08x\n\r"
			"NPI_NIR_ERR_LOG_P0_INFO_1: 0x%08x\n\r",
			IsrVal, ErrTypeVal, ErrLogP0Info0Val, ErrLogP0Info1Val);
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function clears NPI errors.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
******************************************************************************/
int XPlmi_ClearNpiErrors(void)
{
	int Status = XST_FAILURE;

	/* Unlock NPI address space */
	XPlmi_Out32(NPI_NIR_REG_PCSR_LOCK, NPI_NIR_REG_PCSR_UNLOCK_VAL);
	/* Clear ISR */
	XPlmi_UtilRMW(NPI_NIR_REG_ISR, NPI_NIR_REG_ISR_ERR_MASK,
		NPI_NIR_REG_ISR_ERR_MASK);
	/* Clear error type registers */
	XPlmi_UtilRMW(NPI_NIR_ERR_TYPE, NPI_NIR_ERR_TYPE_ERR_MASK,
		~(NPI_NIR_ERR_TYPE_ERR_MASK));
	XPlmi_Out32(NPI_NIR_ERR_LOG_P0_INFO_0, 0U);
	XPlmi_Out32(NPI_NIR_ERR_LOG_P0_INFO_1, 0U);
	/* Lock NPI address space */
	Status = Xil_SecureOut32(NPI_NIR_REG_PCSR_LOCK, 1U);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_NPI_LOCK, Status);
	}

	return Status;
}
