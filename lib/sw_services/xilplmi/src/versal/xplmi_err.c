/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal/xplmi_err.c
*
* This file contains the PLMI versal platform specific EAM code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*       bm   01/03/2023 Remove Triggering of SSIT ERR2 from Slave SLR to
*                       Master SLR
*       bm   01/03/2023 Handle SSIT Events from PPU1 IRQ directly
*       bm   01/03/2023 Notify Other SLRs about Secure Lockdown
*       dd   03/28/2023 Updated doxygen comments
* 1.02  rama 07/19/2023 Updated ErrorTable to support STL errors
*       sk   08/17/2023 Updated XPlmi_EmSetAction arguments
*       rama 08/30/2023 Changed XMPU & XPPU error prints to DEBUG_ALWAYS for
*                       debug level_0 option
*       dd   09/12/2023 MISRA-C violation Rule 10.3 fixed
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xplmi_err_common.h"
#include "xplmi_err.h"
#include "xplmi_ssit.h"
#include "xplmi_tamper.h"

/************************** Constant Definitions *****************************/
#define XPLMI_SYSMON_CLK_SRC_IRO_VAL	(0U) /**< Sysmon clock source IRO value */
#define XPLMI_PMC_ERR_SSIT_MASK	(0xE0000000U) /**< PMC error SSIT mask */

/* Proc IDs for CPM Proc CDOs */
#define CPM_NCR_PCIE0_LINK_DOWN_PROC_ID					(0x1U) /**< PCIE0 link down proc Id*/
#define CPM_NCR_PCIE1_LINK_DOWN_PROC_ID					(0x2U) /**< PCIE1 link down proc Id*/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlmi_CpmErrHandler(u32 ErrorNodeId, u32 RegMask);
static void XPlmi_XppuErrHandler(u32 BaseAddr, const char *ProtUnitStr);
static void XPlmi_XmpuErrHandler(u32 BaseAddr, const char *ProtUnitStr);
static void XPlmi_ProtUnitErrHandler(u32 ErrorNodeId, u32 RegMask);

/************************** Variable Definitions *****************************/
/*
 * Structure to define error action type and handler if error to be handled
 * by PLM
 */
static XPlmi_Error_t ErrorTable[XPLMI_ERROR_SW_ERR_MAX] = {
	[XPLMI_ERROR_BOOT_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_BOOT_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FW_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_ERROUT, .SubsystemId = 0U, },
	[XPLMI_ERROR_FW_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_SRST, .SubsystemId = 0U, },
	[XPLMI_ERROR_GSW_CR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_GSW_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
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
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DDRMB_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOCTYPE1_CR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOCTYPE1_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOCUSER] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_MMCM] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_AIE_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_AIE_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DDRMC_CR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DDRMC_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_GT_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_GT_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PLSMON_CR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PLSMON_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
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
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MB_FATAL1] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCPAR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_CR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON0] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON1] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON2] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON3] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON4] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON8] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON9] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
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
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCTO] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
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
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PS_SW_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_B_CR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_B_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MB_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_CR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM_ECC] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_L2_ECC] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPU_ECC] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPU_LS] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPU_CCF] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_GIC_AXI] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_GIC_ECC] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_APLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CPM_CR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_CPM_NCR] =
	{ .Handler = XPlmi_CpmErrHandler,
			.Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_APB] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_APB] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_PAR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_PAR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_IOU_PAR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_PAR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_TO] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_TO] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_TO] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_XRAM_CR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_XRAM_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_SWDT] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_SWDT] =
	{ .Handler = XPlmi_ErrPrintToLog,
			.Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
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
	[XPLMI_ERROR_CRAM_CE] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CRAM_UE] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NPI_UE] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_STL_UE] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
};

/*****************************************************************************/
/**
 * @brief	This function provides error table pointer
 *
 * @return	Pointer to Error Table
 *
 *****************************************************************************/
XPlmi_Error_t *XPlmi_GetErrorTable(void)
{
	return ErrorTable;
}

/*****************************************************************************/
/**
 * @brief	This function provides IsPsmChanged variable
 *
 * @return	Pointer to IsPsmCrChanged
 *
 *****************************************************************************/
u32 *XPlmi_GetPsmCrState(void)
{
	static u32 IsPsmCrChanged __attribute__ ((aligned(4U))) = (u32)FALSE;

	return &IsPsmCrChanged;
}

/*****************************************************************************/
/**
 * @brief	This function provides event index
 *
 * @param	ErrorNodeType is the Node Type of the EAM register

 * @return	Event Index
 *
 *****************************************************************************/
u8 XPlmi_GetEventIndex(XPlmi_EventType ErrorNodeType)
{
	u8 Index;

	switch (ErrorNodeType) {
		case XPLMI_NODETYPE_EVENT_PMC_ERR1:
		case XPLMI_NODETYPE_EVENT_PMC_ERR2:
			Index = XPLMI_NODETYPE_EVENT_PMC_INDEX;
			break;
		case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		case XPLMI_NODETYPE_EVENT_PSM_ERR2:
			Index = XPLMI_NODETYPE_EVENT_PSM_INDEX;
			break;
		case XPLMI_NODETYPE_EVENT_SW_ERR:
			Index = XPLMI_NODETYPE_EVENT_SW_INDEX;
			break;
		default:
			Index = XPLMI_NODETYPE_EVENT_INVALID_INDEX;
			break;
	}

	return Index;
}

/*****************************************************************************/
/**
 * @brief	This function sets the sysmon clock to IRO for ES1 silicon
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_SysmonClkSetIro(void) {
	u32 SiliconVal = XPlmi_In32(PMC_TAP_VERSION) &
			PMC_TAP_VERSION_PMC_VERSION_MASK;

	if (SiliconVal == XPLMI_SILICON_ES1_VAL) {
		XPlmi_UtilRMW(CRP_SYSMON_REF_CTRL, CRP_SYSMON_REF_CTRL_SRCSEL_MASK,
			XPLMI_SYSMON_CLK_SRC_IRO_VAL);
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
void XPlmi_HandleLinkDownError(u32 Cpm5PcieIrStatusReg,
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
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "%s: ERR_ST1: 0x%08x, ERR_ST2: 0x%08x, ISR: 0x%08x\r\n",
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
	XPlmi_Printf(DEBUG_PRINT_ALWAYS,
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
* @brief    This function dumps EAM Error status registers and Gic Status
* registers
*
* @return   None
*
****************************************************************************/
void XPlmi_DumpErrNGicStatus(void)
{
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
void XPlmi_ClearSsitErrors(u32 *PmcErrStatus, u32 Index)
{
	u32 SiliconVal = XPlmi_In32(PMC_TAP_VERSION) &
			PMC_TAP_VERSION_PMC_VERSION_MASK;
	/* Ignore SSIT Errors on ES1 */
	if (SiliconVal == XPLMI_SILICON_ES1_VAL) {
		PmcErrStatus[Index] &= ~(XPLMI_PMC_ERR_SSIT_MASK);
	}
}

/*****************************************************************************/
/**
 * @brief	This function provides pointer to NumErrOuts
 *
 * @return	Pointer to NumErrOuts
 *
 *****************************************************************************/
u32 *XPlmi_GetNumErrOuts(void)
{
	static u32 NumErrOuts = 0U;

	return &NumErrOuts;
}

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
/*****************************************************************************/
/**
 * @brief	This function registers SSIT Err handlers and also enables
 *		the interrupts.
 * @brief	This function detects and handles tamper condition occured on
 *		slave SLRs
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_DetectSlaveSlrTamper(void)
{
	u32 PmcErr1Status;

	if ((XPlmi_GetSlrIndex() == XPLMI_SSIT_MASTER_SLR_INDEX) &&
			(XPlmi_SsitIsIntrEnabled() == (u8)TRUE)) {
		PmcErr1Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
		PmcErr1Status &= XPLMI_PMC_ERR_SSIT_MASK;
		/** Detect if SSIT ERR is set in PMC_ERR1_STATUS */
		if (PmcErr1Status) {
			/** Disable SSIT Error */
			(void)XPlmi_EmDisable(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1,
				PmcErr1Status);
			/** Trigger Tamper Response as a task */
			XPlmi_TriggerTamperResponse(XPLMI_RTCFG_TAMPER_RESP_SLD_1_MASK,
				XPLMI_TRIGGER_TAMPER_TASK);
			/** Clear SSIT Errors */
			XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS, PmcErr1Status);
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function registers SSIT Err handlers and also Enables the
 *		interrupts.
 *
 * @param	Id is the IoModule Intr ID
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_RegisterSsitErrHandlers(u32 Id)
{
	/** Register XPlmi_SsitErrHandler to IoModule */
	(void)XPlmi_RegisterHandler(Id,
		(GicIntHandler_t)(void *)XPlmi_SsitErrHandler, (void *)Id);
	/** Enable SSIT Irq on IoModule */
	XPlmi_PlmIntrEnable(Id);
}

/*****************************************************************************/
/**
 * @brief	This function handles the SSIT ERR2 IRQs coming from the slave
 *
 * @param	ErrorNodeId is the node ID for the error event
 * @param	RegMask is the register mask of the error received
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_HandleSsitErr2(u32 ErrorNodeId, u32 RegMask)
{
	(void)ErrorNodeId;
	(void)RegMask;

	/** Trigger SLD1 Tamper Response as a task */
	XPlmi_TriggerTamperResponse(XPLMI_RTCFG_TAMPER_RESP_SLD_1_MASK,
		XPLMI_TRIGGER_TAMPER_TASK);
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
	 *   - SSIT_ERR3 is for the SLD notification from Slave SLR0
	 *   - SSIT_ERR4 is for the SLD notification from Slave SLR1
	 *   - SSIT_ERR5 is for the SLD notification from Slave SLR2
	 * For Slave SLRs:
	 *   - SSIT_ERR0 in Slave SLR0 is for the events from Master SLR
	 *   - SSIT_ERR1 in Slave SLR1 is for the events from Master SLR
	 *   - SSIT_ERR2 in Slave SLR2 is for the events from Master SLR
	 *
	 * Other SSIT errors in Slave SLRs are not configured
	 */
	if (XPlmi_SsitIsIntrEnabled() == (u8)TRUE) {
		/* Interrupts are enabled already. No need to enable again. */
		goto END;
	}

	/* Clear the SSIT IRQ bits */
	XPlmi_Out32(PMC_PMC_MB_IO_IRQ_ACK, PMC_PMC_MB_IO_SSIT_IRQ_MASK);

	SlrIndex = XPlmi_GetSlrIndex();
	if (SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) {
		/* Register Handlers for SSIT ERR IRQ in master */
		XPlmi_RegisterSsitErrHandlers(XPLMI_IOMODULE_SSIT_ERR0);
		XPlmi_RegisterSsitErrHandlers(XPLMI_IOMODULE_SSIT_ERR1);
		XPlmi_RegisterSsitErrHandlers(XPLMI_IOMODULE_SSIT_ERR2);
		(void)XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1,
			XIL_EVENT_ERROR_MASK_SSIT3, XPLMI_EM_ACTION_CUSTOM,
			XPlmi_HandleSsitErr2, XPLMI_INVALID_SUBSYSTEM_ID);
		(void)XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1,
			XIL_EVENT_ERROR_MASK_SSIT4, XPLMI_EM_ACTION_CUSTOM,
			XPlmi_HandleSsitErr2, XPLMI_INVALID_SUBSYSTEM_ID);
		(void)XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1,
			XIL_EVENT_ERROR_MASK_SSIT5, XPLMI_EM_ACTION_CUSTOM,
			XPlmi_HandleSsitErr2, XPLMI_INVALID_SUBSYSTEM_ID);
	} else if (SlrIndex == XPLMI_SSIT_SLAVE0_SLR_INDEX) {
		XPlmi_RegisterSsitErrHandlers(XPLMI_IOMODULE_SSIT_ERR0);
	} else if (SlrIndex == XPLMI_SSIT_SLAVE1_SLR_INDEX) {
		XPlmi_RegisterSsitErrHandlers(XPLMI_IOMODULE_SSIT_ERR1);
	} else if (SlrIndex == XPLMI_SSIT_SLAVE2_SLR_INDEX) {
		XPlmi_RegisterSsitErrHandlers(XPLMI_IOMODULE_SSIT_ERR2);
	} else {
		goto END;
	}
	XPlmi_Printf(DEBUG_GENERAL, "Enabled SSIT Interrupts\r\n");
	XPlmi_SsitSetIsIntrEnabled((u8)TRUE);

END:
	return;
}
#endif
