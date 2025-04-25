/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal_2ve_2vm/xplmi_err.c
*
* This file contains the PLMI versal_net platform specific code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  sk   08/26/2024 Initial release, Updated Error Table
*       pre  01/09/2025 Added PCIE error handling
*       sk   02/20/2025 Added XMPU/XPPU error handlers
*       ma   03/14/2025 Moved a print statement in XPlmi_ErrPrintToLog
*       sk   04/07/2025 Updated error actions for apll1 and rpu fatal error
*       sk   04/07/2025 Added redundant call to enable EAM interrupt
*       sk   04/09/2025 Updated LPD SLCR EAM Disable error logic
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xilplmi_server_apis XilPlmi server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xplmi_err_common.h"
#include "xplmi_err.h"
#include "xil_error_node.h"
#include "xplmi_plat.h"
#include "xplmi_util.h"
#include "xplmi_generic.h"
#include "xplmi.h"

/************************** Constant Definitions *****************************/
#define XPLMI_ERROR_TABLE_DS_VER	(1U) /**< Error table data structure version */
#define XPLMI_ERROR_TABLE_DS_LCVER	(1U) /**< Error table data structure LC version */

#define XPLMI_NUM_ERROUTS_VERSION	(1U) /**< ERROUTS version */
#define XPLMI_NUM_ERROUTS_LCVERSION	(1U) /**< ERROUTS LC version */

/* Proc IDs for PCIe Reset CDOs */
#define PCIE0_LINK_DOWN_PROC_ID					(0x1U) /**< PCIE0 link down proc Id*/
#define PCIE1_LINK_DOWN_PROC_ID					(0x2U) /**< PCIE1 link down proc Id*/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlmi_HandleLPDSlcrError(u32 ErrorNodeId, u32 RegMask);
static void XPlmi_ErrLPDSlcrIntrHandler(u32 ErrorNodeId, u32 RegMask);
static void XPlmi_PcieErrHandler(u32 ErrorNodeId, u32 RegMask);
static void XPlmi_HandleLinkDownError(u32 DeviceIrStatusReg, u32 GenralStsReg, u32 ProcId);
static void XPlmi_XppuErrHandler(u32 BaseAddr, const char *ProtUnitStr);
static void XPlmi_XmpuErrHandler(u32 BaseAddr, const char *ProtUnitStr);
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
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_GSW_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_CFU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CFRAME] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RSVD_0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_RSVD_1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_DDRMB_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DDRMB_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOCTYPE1_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOCTYPE1_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOCUSER] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_MMCM] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_AIE_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_AIE_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DDRMC_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DDRMC_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_GT_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_GT_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PLSMON_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PLSMON_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
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
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MB_FATAL1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON2] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON3] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON4] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON8] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMCSMON9] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_CFI] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SEUCRC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SEUECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMX_WWDT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_SRST, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_RTCALARM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NPLL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PPLL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CLKMON] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_PMX_CORR_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_PMX_UNCORR_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_IOU_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_IOU_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DFX_UXPT_ACT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DICE_CDI_PAR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVIK_PRIV] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NXTSW_CDI_PAR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVAK_PRIV] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DME_PUB_X] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DME_PUB_Y] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVAK_PUB_X] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVAK_PUB_Y] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVIK_PUB_X] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVIK_PUB_Y] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PCR_PAR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_EAM_E0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_SRST, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_EAM_E1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_POR, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_EAM_E2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_ERROUT, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_EAM_E3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_ASU_EAM_GD] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_EAM_GD] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_EAM_SMIRQ0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_EAM_SMIRQ1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_EAM_PRAM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_EAM_AGERR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_EAM_UFSFE] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PS_SW_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PS_SW_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_USB_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_DFX] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_UFSHC_FE_IRQ] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_APLL1_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_APLL2_LOCK] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPLL_LOCK] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FLXPLL_LOCK] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_LPXASILB_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_LPXASILB_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_LPXASILD_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_LPXASILD_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_FPXASILD_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_FPXASILD_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_FPXASILB_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_FPXASILB_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_SPLIT_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_SPLIT_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RSVD_2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RSVD_3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RSVD_4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RSVD_5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOC_NMU_FIREWALL_WR_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOC_NMU_FIREWALL_RD_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NOC_NSU_FIREWALL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_GIC_FMU_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_GIC_FMU_FAULT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RSVD_6] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RSVD_7] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_IPI_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_CPI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT2] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT3] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSXC_SPLITTER0_NON_FATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSXC_SPLITTER1_NON_FATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSXC_SPLITTER2_NON_FATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSXC_SPLITTER3_NON_FATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSXC_SPLITTER_FATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_GIC_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_GIC_FAULT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_CMN_FAULT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_CMN_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_ACP_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_APU0_ERI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_APU0_FHI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_APU1_ERI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_APU1_FHI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_APU2_ERI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_APU2_FHI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_APU3_ERI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_APU3_FHI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_MMU_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_MMU_FAULT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_SLCR_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_SLCR_SECURE_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPX_AFIFM0_NONFATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPX_AFIFM1_NONFATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPX_AFIFM2_NONFATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPX_AFIFM3_NONFATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPX_AFIFS_CORR_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPX_AFIFS_UNCORR_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUA_CORE_CLUSTER_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUA_CORE0_NON_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUA_CORE1_NON_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUB_CORE_CLUSTER_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUB_CORE0_NON_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUB_CORE1_NON_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUC_CORE_CLUSTER_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUC_CORE0_NON_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUC_CORE1_NON_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUD_CORE_CLUSTER_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUD_CORE0_NON_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUD_CORE1_NON_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUE_CORE_CLUSTER_FATAL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUE_CORE0_NON_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUE_CORE1_NON_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPU_PCIL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM0_NONFATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM0_FATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM1_NONFATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM1_FATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM2_NONFATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM2_FATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM3_NONFATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM3_FATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_WWDT0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_WWDT1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_WWDT2] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_WWDT3] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_WWDT4] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_ADMA_LS_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_GLITCH_DET0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_GLITCH_DET1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_CRF] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_MON_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_AFIFM_FATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_AFIFM_NONFATAL_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_ASU_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_ASU_NON_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_AFIFS_CORR_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_AFIFS_UNCORR_ERR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MMI_CORR_EVENT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MMI_UNCORR_EVENT] =
	{ .Handler = XPlmi_PcieErrHandler, .Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_MMI_GPU_COR_EVENT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MMI_PCIE0_COR_EVENT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MMI_PCIE1_COR_EVENT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MMI_GEM_COR_EVENT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MMI_DC_COR_EVENT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MMI_UDH_COR_EVENT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_ADMA_ERR1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_ADMA_ERR2] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_ADMA_ERR3] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_ADMA_ERR4] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_ADMA_ERR5] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_ADMA_ERR6] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_ADMA_ERR7] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_ADMA_ERR8] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_SDMA_ERR1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_SDMA_ERR2] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_SDMA_ERR3] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_SDMA_ERR4] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_SDMA_ERR5] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_SDMA_ERR6] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_SDMA_ERR7] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_SDMA_ERR8] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
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
	[XPLMI_ERROR_DEV_STATE_CHANGE] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PCR_LOG_UPDATE] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CRAM_CE] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CRAM_UE] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NPI_UE] =
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
	EXPORT_GENERIC_DS(ErrorTable, XPLMI_ERROR_TABLE_DS_ID, XPLMI_ERROR_TABLE_DS_VER,
	XPLMI_ERROR_TABLE_DS_LCVER, sizeof(ErrorTable), (u32)(UINTPTR)ErrorTable);

	return ErrorTable;
}

/*****************************************************************************/
/**
 * @brief	This function reconfigures error actions after the update
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_ReconfigErrActions(void)
{
	u8 ErrIndex;
	u32 ErrorNodeId;
	XPlmi_EventType NodeType;

	for (ErrIndex = 0U; ErrIndex < XPLMI_ARRAY_SIZE(ErrorTable); ErrIndex++) {
		ErrorTable[ErrIndex].Handler = NULL;
		if ((ErrorTable[ErrIndex].Action == XPLMI_EM_ACTION_CUSTOM)) {
				/* Set error action to NONE for all other errors */
				ErrorTable[ErrIndex].Action = XPLMI_EM_ACTION_NONE;
		}
		else if (ErrorTable[ErrIndex].Action == XPLMI_EM_ACTION_INVALID) {
			continue;
		}
		ErrorNodeId = XIL_NODETYPE_EVENT_ERROR_PMC_ERR1 +
			((ErrIndex / XPLMI_MAX_ERR_BITS) * XPLMI_EVENT_ERROR_OFFSET);
		NodeType = (XPlmi_EventType)XPlmi_EventNodeType(ErrorNodeId);
		if (XPlmi_EmConfig(NodeType, ErrIndex, ErrorTable[ErrIndex].Action,
			ErrorTable[ErrIndex].Handler, ErrorTable[ErrIndex].SubsystemId) != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL,
				"Warning: XPlmi_ReconfigErrActions: Failed to "
				"restore action for ERR index %d\n\r",
				ErrIndex);
		}
	}
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
		case XPLMI_NODETYPE_EVENT_PMC_ERR3:
			Index = XPLMI_NODETYPE_EVENT_PMC_INDEX;
			break;
		case XPLMI_NODETYPE_EVENT_LPDSLCR_ERR0:
		case XPLMI_NODETYPE_EVENT_LPDSLCR_ERR1:
		case XPLMI_NODETYPE_EVENT_LPDSLCR_ERR2:
		case XPLMI_NODETYPE_EVENT_LPDSLCR_ERR3:
			Index = XPLMI_NODETYPE_EVENT_LPDSLCR_INDEX;
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
* @brief    This function is the interrupt handler for Error action "Print
*           to Log". This function prints detailed information if the error is
*           due to XMPU/XPPU protection units.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_ErrPrintToLog(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrorId = XPlmi_GetErrorId(ErrorNodeId, RegMask);
	u32 RegVal;

	/** Print NodeId, Mask and Error ID information */
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Received EAM error. ErrorNodeId: 0x%x,"
				" Register Mask: 0x%x. The corresponding Error ID: 0x%x\r\n",
				ErrorNodeId, RegMask, ErrorId);

	/*
	 * The nature of XPPU/XMPU errors is such that they often occur consecutively
	 * due to the stream of transactions. This may result in flooding the UART with
	 * prints and/or starvation of user tasks due to PLM's constant handling of these
	 * error events. Therefore, such events are handled only the first time error
	 * occurs and these errors are not enabled again. The source is also not cleared
	 * because doing so may interfere with user's handling of these errors.
	 */
	switch (ErrorId) {
	case XPLMI_ERROR_INT_PMX_UNCORR_ERR:
		RegVal = XPlmi_In32(PMC_TOP_LVL_UNCORR_ERR_SRC_REGS_ADDR);
		if ((RegVal & PMC_UNCORR_ERR_SRC_REGS_1_MASK) == PMC_UNCORR_ERR_SRC_REGS_1_MASK) {
			RegVal = XPlmi_In32(PMC_UNCORR_ERR_SRC_REGS_1_ADDR);
			if ((RegVal & XPPU_NPI_FIREWALL_MASK) == XPPU_NPI_FIREWALL_MASK) {
				XPlmi_XppuErrHandler(PMC_XPPU_NPI_BASEADDR, "PMC_XPPU_NPI");
			}

			if ((RegVal & XPPU_FIREWALL_MASK) == XPPU_FIREWALL_MASK) {
				XPlmi_XppuErrHandler(PMC_XPPU_BASEADDR, "PMC_XPPU");
			}

			if ((RegVal & XMPU_SBI_FIREWALL_MASK)== XMPU_SBI_FIREWALL_MASK) {
				XPlmi_XmpuErrHandler(PMC_XMPU_SBI_BASEADDR, "PMC_SBI_XMPU");
			}

			if ((RegVal & XMPU_PRAM_FIREWALL_MASK)== XMPU_PRAM_FIREWALL_MASK) {
				XPlmi_XmpuErrHandler(PMC_XMPU_BASEADDR, "PMC_PRAM_XMPU");
			}

			if ((RegVal & XMPU_CFU_FIREWALL_MASK)== XMPU_CFU_FIREWALL_MASK) {
				XPlmi_XmpuErrHandler(PMC_XMPU_CFU_BASEADDR, "PMC_CFU_XMPU");
			}
		}
		break;

	case XPLMI_ERROR_INT_LPXASILD_NCR:
		RegVal = XPlmi_In32(LPD_SLCR_INT_LPXASILD_UNCORR_ERR_ISR);

		if((RegVal & INTLPD_ASILD_UNCORR_ERR_MASK) == INTLPD_ASILD_UNCORR_ERR_MASK) {
			RegVal = XPlmi_In32(INTLPD_ASILD_CONFIG_TOP_LVL_UNCORR_ERR_SRC);
			if ((RegVal & LPD_INT_UNCORR_ERR_SRC_REGS_0_MASK) == LPD_INT_UNCORR_ERR_SRC_REGS_0_MASK) {
				RegVal = XPlmi_In32(INTLPD_ASILD_CONFIG_UNCORR_ERR_SRC_0);
				if ((RegVal & INTLPX_XPPU_FIREWALL_MASK) == INTLPX_XPPU_FIREWALL_MASK) {
					XPlmi_XppuErrHandler(LPD_XPPU_BASEADDR, "LPD_XPPU");
				}
			}
		} else if((RegVal & INTOCM_ASILD_UNCORR_ERR_MASK) == INTOCM_ASILD_UNCORR_ERR_MASK) {
			RegVal = XPlmi_In32(INTOCM_ASILD_CONFIG_TOP_LVL_UNCORR_ERR_SRC);
			if ((RegVal & OCM_INT_UNCORR_ERR_SRC_REGS_0_MASK) == OCM_INT_UNCORR_ERR_SRC_REGS_0_MASK) {
				RegVal = XPlmi_In32(INTOCM_ASILD_CONFIG_UNCORR_ERR_SRC_0);

				if ((RegVal & INTOCM_XMPU_SLAVES_FIREWALL_MASK) == INTOCM_XMPU_SLAVES_FIREWALL_MASK) {
					XPlmi_XmpuErrHandler(OCM_SLVS_XMPU_BASEADDR, "OCM_SLAVE_XMPU");
				}
				if ((RegVal & INTOCM_XMPU3_FIREWALL_MASK) == INTOCM_XMPU3_FIREWALL_MASK) {
					XPlmi_XmpuErrHandler(OCM3_XMPU_BASEADDR, "OCM3_XMPU");
				}
				if ((RegVal & INTOCM_XMPU2_FIREWALL_MASK) == INTOCM_XMPU2_FIREWALL_MASK) {
					XPlmi_XmpuErrHandler(OCM2_XMPU_BASEADDR, "OCM2_XMPU");
				}
				if ((RegVal & INTOCM_XMPU1_FIREWALL_MASK) == INTOCM_XMPU1_FIREWALL_MASK) {
					XPlmi_XmpuErrHandler(OCM1_XMPU_BASEADDR, "OCM1_XMPU");
				}
				if ((RegVal & INTOCM_XMPU0_FIREWALL_MASK) == INTOCM_XMPU0_FIREWALL_MASK) {
					XPlmi_XmpuErrHandler(OCM0_XMPU_BASEADDR, "OCM0_XMPU");
				}
				if ((RegVal & INTOCM_XMPU_TCM_FIREWALL_MASK) == INTOCM_XMPU_TCM_FIREWALL_MASK) {
					XPlmi_XmpuErrHandler(OCM_TCM_XMPU_BASEADDR, "OCM_TCM_XMPU");
				}
			}
		} else {
			/* Misra-C */
		}
		break;

	case XPLMI_ERROR_INT_FPXASILD_NCR:
		RegVal = XPlmi_In32(FPD_INT_TOP_LVL_UNCORR_ERR_SRC_REGS_ADDR);
		if ((RegVal & FPD_INT_UNCORR_ERR_SRC_REGS_0_MASK) == FPD_INT_UNCORR_ERR_SRC_REGS_0_MASK) {
			RegVal = XPlmi_In32(FPD_INT_UNCORR_ERR_SRC_REGS_0_ADDR);

			if ((RegVal & FPX_XMPU_MMU_FIREWALL_MASK) == FPX_XMPU_MMU_FIREWALL_MASK) {
				XPlmi_XmpuErrHandler(FPD_MMU_XMPU_BASEADDR, "FPX_MMU_XMPU");
			}

			if ((RegVal & FPX_XMPU_FIREWALL_MASK) == FPX_XMPU_FIREWALL_MASK) {
				XPlmi_XmpuErrHandler(FPD_SLAVE_XMPU_BAESADDR, "FPX_XMPU");
			}

			if ((RegVal & FPX_XMPU_CMN_FIREWALL_MASK) == FPX_XMPU_CMN_FIREWALL_MASK) {
				XPlmi_XmpuErrHandler(FPD_CMN_XMPU_BASEADDR, "FPX_CMN_XMPU");
			}
		}
		break;

	case XPLMI_ERROR_LPX_AFIFS_UNCORR_ERR:
		XPlmi_XmpuErrHandler(LPD_AFIFS_XMPU_BASEADDR, "LPD_AFIFS_XMPU");
		break;

	case XPLMI_ERROR_FPX_AFIFS_UNCORR_ERR:
		XPlmi_XmpuErrHandler(FPD_AFIFS_XMPU_BASEADDR, "FPD_AFIFS_XMPU");
		break;

	default:
		break;
	}
}

/*************************************************************************************************/
/**
* @brief    This function handles the PCIE link down error.
*
* @param    DeviceIrStatusReg is the PCIE0/1 Device IR status register address
* @param    GenralStsReg is the PCIE0/1 General status register address
* @param    ProcId is the ProcId for PCIE0/1 link down error
*
**************************************************************************************************/
static void XPlmi_HandleLinkDownError(u32 DeviceIrStatusReg, u32 GenralStsReg, u32 ProcId)
{
	int Status = XST_FAILURE;
	u32 SmlhLinkSts;
	u32 SmlhLinkToggleEn = ((~XPlmi_In32(DeviceIrStatusReg + 4U)) & MMI_PCIE_DEVICE_IR_STATUS_SMLH_LINK_TOGGLE_MASK);
	u32 DeviceIrStsUncorrErr = XPlmi_In32(DeviceIrStatusReg);

	/* Checking PCIE linkup signal toggle status and its enable status */
	if ((SmlhLinkToggleEn != 0U) && ((DeviceIrStsUncorrErr &
	    MMI_PCIE_DEVICE_IR_STATUS_SMLH_LINK_TOGGLE_MASK) == MMI_PCIE_DEVICE_IR_STATUS_SMLH_LINK_TOGGLE_MASK)) {
		/* Read PCIE linkup status */
		SmlhLinkSts = (XPlmi_In32(GenralStsReg) & MMI_PCIE_GENERAL_STATUS_SMLH_LINK__MASK);
		if (SmlhLinkSts == 0U)
		{
			/* Execute PCIE0 link down proc */
			Status = XPlmi_ExecuteProc(ProcId);
			if (Status != XST_SUCCESS) {
				XPlmi_Printf(DEBUG_INFO, "Error in handling PCIE "
							"link down event: 0x%x\r\n", Status);
				/*
				 * Update error manager with error received
				 * while executing proc
				 */
				XPlmi_ErrMgr(Status);
			}
		}
		else {
			/* Received error is other than Link down error */
			XPlmi_Printf(DEBUG_GENERAL, "Received error is other than "
									"link down event");
		}
		XPlmi_Out32(DeviceIrStatusReg, MMI_PCIE_DEVICE_IR_STATUS_SMLH_LINK_TOGGLE_MASK);
	}
	else {
		/* Received error is other than PCIE local event */
		XPlmi_Printf(DEBUG_GENERAL, "Received error is other than "
				"PCIE local event: 0x%x\r\n", DeviceIrStsUncorrErr);
	}
}

/****************************************************************************/
/**
* @brief    This function handles the PCIE errors.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
****************************************************************************/
static void XPlmi_PcieErrHandler(u32 ErrorNodeId, u32 RegMask)
{
	/* Read PCIE0, PCIE1 errors */
	u32 PsUnCorrErrs = XPlmi_In32(MMI_SLCR_PS_UNCORR_IR_STATUS);
	u32 PcieErrEnable = ((~XPlmi_In32(MMI_SLCR_PS_UNCORR_IR_MASK)) &
			(MMI_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK |
				MMI_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK));

	(void)ErrorNodeId;
	(void)RegMask;

	if (PcieErrEnable != 0U) {
		if ((PsUnCorrErrs & MMI_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK) ==
		    MMI_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK) {
			/* Handle PCIE0 link down error */
			XPlmi_Printf(DEBUG_GENERAL, "Received PCIE0 interrupt\r\n");
			XPlmi_HandleLinkDownError(MMI_PCIE0_CTRL_SLCR_DEV_IR_STS_UNCOCRR,
			                  MMI_PCIE0_CTRL_SLCR_GENERAL_STATUS, PCIE0_LINK_DOWN_PROC_ID);
			/* Clear PCIE0 error */
			XPlmi_Out32(MMI_SLCR_PS_UNCORR_IR_STATUS, MMI_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK);
		}

		if ((PsUnCorrErrs & MMI_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK) ==
		     MMI_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK) {
			/* Handle PCIE1 link down error */
			XPlmi_Printf(DEBUG_GENERAL, "Received PCIE1 interrupt\r\n");
			XPlmi_HandleLinkDownError(MMI_PCIE1_CTRL_SLCR_DEV_IR_STS_UNCOCRR,
			                  MMI_PCIE1_CTRL_SLCR_GENERAL_STATUS, PCIE1_LINK_DOWN_PROC_ID);
			/* Clear PCIE1 error */
			XPlmi_Out32(MMI_SLCR_PS_UNCORR_IR_STATUS, MMI_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK);
		}
	}
	else {
		/* Only PCIE0/1 errors are handled */
		XPlmi_Printf(DEBUG_GENERAL, "Unhandled MMI UNCORR error: 0x%x\r\n",
				PsUnCorrErrs);
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
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_PMC_ERR3_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_PMC_ERR3_STATUS));
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
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_GICP5_IRQ_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP5_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_GICP6_IRQ_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP6_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_GICP7_IRQ_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP7_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC_GLOBAL_GICP_PMC_IRQ_STATUS: 0x%08x\n\r",
		XPlmi_In32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS));
}

/****************************************************************************/
/**
* @brief	This function restricts error actions.
*
* @param	NodeType of Error
* @param	RegMask of Error
* @param	ErrorAction of the the Error
*
* @return
* 			- XST_SUCCESS if success.
* 			- XPLMI_INVALID_ERROR_ACTION on invalid error action.
*
****************************************************************************/
int XPlmi_RestrictErrActions(XPlmi_EventType NodeType, u32 RegMask, u32 ErrorAction)
{
	int Status = XPLMI_INVALID_ERROR_ACTION;

	if ((NodeType == XPLMI_NODETYPE_EVENT_PMC_ERR2) &&
		(RegMask == XIL_EVENT_ERROR_MASK_PMX_WWDT) &&
		(ErrorAction > XPLMI_EM_ACTION_ERROUT) &&
		(ErrorAction != XPLMI_EM_ACTION_NONE)) {
		XPlmi_Printf(DEBUG_GENERAL, "Only HW Error Actions are"
			" supported for PMC WDT\n\r");
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
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

	EXPORT_GENERIC_DS(NumErrOuts, XPLMI_NUM_ERROUTS_DS_ID,
		XPLMI_NUM_ERROUTS_VERSION, XPLMI_NUM_ERROUTS_LCVERSION,
		sizeof(NumErrOuts), (u32)(UINTPTR)&NumErrOuts);

	return &NumErrOuts;
}


/*****************************************************************************/
/**
 * @brief	This function enables the error action for the given error mask.
 *
 * @param	ErrMaskRegAddr is the error action mask register address
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERROR_ACTION_NOT_ENABLED if error action is not enabled.
 *
 *****************************************************************************/
int EmEnableLpdSlcrErrAction(u32 ErrMaskRegAddr, u32 RegMask)
{
	int Status = XPLMI_ERROR_ACTION_NOT_ENABLED;

	/* Enable the error action */
	XPlmi_Out32((ErrMaskRegAddr + LPDSLCR_EAM_PMC_MASK_EN_OFFSET), RegMask);
	/* Check if the error action is enabled */
	if ((XPlmi_In32(ErrMaskRegAddr) & RegMask) == 0x0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}
/*****************************************************************************/
/**
 * @brief	This function disables the LPD SCLR error actions for the given mask.
 *
 * @param	RegMaskAddr is the offset for the PMC ERR, POR ,IRQ mask,SRST mask
 * @param	RegMask is the register mask of the error to be disabled
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERROR_ACTION_NOT_DISABLED if error action is not disabled.
 *
 *****************************************************************************/
int XPlmi_EmDisableLpdSlcrErrors(u32 RegMaskAddr, u32 RegMask)
{
	u32 Status = (u32)XPLMI_ERROR_ACTION_NOT_DISABLED;

	/* Clear error in enable register */
	XPlmi_Out32((RegMaskAddr + LPDSLCR_EAM_PMC_MASK_EN_OFFSET),
			((~RegMask) & (~XPlmi_In32(RegMaskAddr))));

	/** - Disable LPD SLCR error actions. */
	XPlmi_Out32((RegMaskAddr + LPDSLCR_EAM_PMC_MASK_DIS_OFFSET), RegMask);
	/**
	 * - Check if the error action is disabled.
	 */
	if ((XPlmi_In32(RegMaskAddr) & RegMask) == RegMask) {
		Status = XST_SUCCESS;
	}

	return (int)Status;
}

/****************************************************************************/
/**
* @brief    This function is the interrupt handler for the EAM errors.
*
* @param    Data is presently passed as NULL
*
* @return
* 			- XST_SUCCESS always.
*
****************************************************************************/
int XPlmi_Versal2Ve2VmEAMHandler(void *Data)
{
	u32 ErrStatus[XPLMI_PMC_MAX_ERR_CNT];
	u32 ErrIrqMask[XPLMI_PMC_MAX_ERR_CNT];
	u32 ErrIndex;
	u32 Index;
	u32 RegMask;
	XPlmi_Error_t *ErrTable = XPlmi_GetErrorTable();

	XPlmi_Printf(DEBUG_INFO, "%s\n",__func__);

	(void)Data;


	for (Index = 0U; Index < XPLMI_PMC_MAX_ERR_CNT; Index++) {
		ErrStatus[Index] = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS +
					(Index * PMC_GLOBAL_REG_PMC_ERR_OFFSET));
		ErrIrqMask[Index] = XPlmi_In32(GET_PMC_IRQ_MASK(GET_PMC_ERR_ACTION_OFFSET(Index)));
		if ((ErrStatus[Index] & ~ErrIrqMask[Index]) != 0x0U) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PMC EAM ERR%d: 0x%0x\r\n", (Index + 1U),
					ErrStatus[Index]);
		}
	}

	/**
	 * - Interrupt is selected as response for Custom, subsystem shutdown
	 * and subsystem restart actions. For these actions, error will be
	 * disabled. Agent should clear the source and enable the error again
	 * using SetAction. In SetAction, error will be cleared and enabled.
	 * For subsystem cases, during subsystem restart, error will be enabled
	 * again.
	 */

	for (ErrIndex = 0U; ErrIndex < XPLMI_PMC_MAX_ERR_CNT; ErrIndex++) {
		if (ErrStatus[ErrIndex] == 0U) {
			continue;
		}
		for (Index = GET_PMC_ERR_START(ErrIndex); Index <
				GET_PMC_ERR_END(ErrIndex); Index++) {
			if (Index >= XPLMI_ERROR_PMCERR_MAX) {
				break;
			}
			RegMask = XPlmi_ErrRegMask(Index);
			if (((ErrStatus[ErrIndex] & RegMask) != (u32)FALSE) &&
				((ErrIrqMask[ErrIndex] & RegMask) == 0x0U) &&
					((ErrTable[Index].Handler != NULL) ||
					(Index == XPLMI_ERROR_PSX_EAM_E3)) &&
					(ErrTable[Index].Action != XPLMI_EM_ACTION_NONE)) {
				/* Errors in LPDSLCR errors are handled in LPDSLCRErrHandler */
				if (Index != XPLMI_ERROR_PSX_EAM_E3) {
					(void)XPlmi_EmDisable(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1 +
						(ErrIndex * XPLMI_EVENT_ERROR_OFFSET), RegMask);
					ErrTable[Index].Handler(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1 +
						(ErrIndex * XPLMI_EVENT_ERROR_OFFSET), RegMask);
					/*
					 * Reset Action in ErrorTable to XPLMI_EM_ACTION_NONE if previous action
					 * is not invalid or custom.
					 */
					if ((ErrTable[Index].Action != XPLMI_EM_ACTION_INVALID) &&
						(ErrTable[Index].Action != XPLMI_EM_ACTION_CUSTOM)) {
						ErrTable[Index].Action = XPLMI_EM_ACTION_NONE;
					}
				}
				else {
					XPlmi_ErrLPDSlcrIntrHandler(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1 +
						(ErrIndex * XPLMI_EVENT_ERROR_OFFSET), RegMask);
				}
				XPlmi_EmClearError((XPlmi_EventType)(XPLMI_NODETYPE_EVENT_PMC_ERR1 + ErrIndex),
						Index);
			}
		}
	}

	/**
	 * - Clear and enable EAM errors at IOMODULE level
	 */
	(void)XPlmi_PlmIntrClear(XPLMI_IOMODULE_ERR_IRQ);
	XSECURE_REDUNDANT_IMPL(XPlmi_PlmIntrEnable, XPLMI_IOMODULE_ERR_IRQ);

	return XST_SUCCESS;
}


/****************************************************************************/
/**
* @brief    This function is the interrupt handler for Errors in LPDSLCR
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return
* 			- None
*
****************************************************************************/
static void XPlmi_ErrLPDSlcrIntrHandler(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrStatus[XPLMI_LPDSLCR_MAX_ERR_CNT];
	u32 ErrMask[XPLMI_LPDSLCR_MAX_ERR_CNT];
	u32 Index;
	u32 ErrIndex;
	u32 ErrRegMask;
	XPlmi_Error_t *ErrTable = XPlmi_GetErrorTable();

	(void)ErrorNodeId;
	(void)RegMask;

	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "LPD SLCR EAM Interrupt: ");
	for (Index = 0U; Index < XPLMI_LPDSLCR_MAX_ERR_CNT; Index++) {
		ErrStatus[Index] = XPlmi_In32(LPD_SLCR_REG_EAM_ERR0_STATUS +
					(Index * LPD_SLCR_GLOBAL_REG_ERR_OFFSET));
		/* PMC3_EAMx .. is routed to IRQ */
		ErrMask[Index] = XPlmi_In32(LPD_SLCR_EAM_PMC3_ERR0_MASK +
					(Index * LPD_SLCR_GLOBAL_REG_ERR_OFFSET));
		XPlmi_Printf_WoTS(DEBUG_PRINT_ALWAYS, "ERR%d: 0x%0x ", (Index),
					ErrStatus[Index]);
	}
	XPlmi_Printf_WoTS(DEBUG_PRINT_ALWAYS, "\n\r");

	for (ErrIndex = 0U; ErrIndex < XPLMI_LPDSLCR_MAX_ERR_CNT; ErrIndex++) {
		if (ErrStatus[ErrIndex] == 0U) {
			continue;
		}
		for (Index = GET_LPDSLCR_ERR_START(ErrIndex); Index <
				GET_LPDSLCR_ERR_END(ErrIndex); Index++) {
			if (Index >= XPLMI_ERROR_LPDSLCR_ERR_MAX) {
				break;
			}
			ErrRegMask = XPlmi_ErrRegMask(Index);
			if (((ErrStatus[ErrIndex] & ErrRegMask) != (u32)FALSE) &&
				((ErrMask[ErrIndex] & ErrRegMask) == 0x0U) &&
				(ErrTable[Index].Action != XPLMI_EM_ACTION_NONE)) {
				XPlmi_HandleLPDSlcrError(XIL_NODETYPE_EVENT_ERROR_LPD_SLCR_ERR1 +
					(ErrIndex * XPLMI_EVENT_ERROR_OFFSET),
					ErrRegMask);
			}
		}
	}
}

/****************************************************************************/
/**
* @brief    This function handles the error in LPD SLCR routed to PLM.
*
* @param    ErrorNodeId is the node ID for the error event
* @param    RegMask is the register mask of the error received
*
* @return
* 			- None
*
****************************************************************************/
static void XPlmi_HandleLPDSlcrError(u32 ErrorNodeId, u32 RegMask)
{
	u32 ErrorId;
	XPlmi_EventType ErrorNodeType = (XPlmi_EventType)XPlmi_EventNodeType(ErrorNodeId);
	XPlmi_Error_t *ErrTable = XPlmi_GetErrorTable();

	ErrorId = XPlmi_GetErrorId(ErrorNodeId, RegMask);

	(void)XPlmi_EmDisable(ErrorNodeId, RegMask);
	switch (ErrTable[ErrorId].Action) {
	case XPLMI_EM_ACTION_POR:
		XPlmi_PORHandler();
		break;
	case XPLMI_EM_ACTION_SRST:
		XPlmi_SoftResetHandler();
		break;
	case XPLMI_EM_ACTION_ERROUT:
		/*
		 * Clear LPD SLCR error and trigger error out using PMC FW_CR error
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
	case XPLMI_EM_ACTION_PRINT_TO_LOG:
	case XPLMI_EM_ACTION_SLD:
	case XPLMI_EM_ACTION_SLD_WITH_IO_TRI:
		if (ErrTable[ErrorId].Handler != NULL) {
			ErrTable[ErrorId].Handler(ErrorNodeId, RegMask);
		}
		XPlmi_EmClearError(ErrorNodeType, ErrorId);
		break;
	default:
		XPlmi_EmClearError(ErrorNodeType, ErrorId);
		XPlmi_Printf(DEBUG_GENERAL, "Invalid Error Action "
				"for errors in LPD SLCR. Error ID: 0x%x\r\n", ErrorId);
		break;
	}

	/*
	 * Reset Action in ErrorTable to XPLMI_EM_ACTION_NONE if previous action is
	 * not invalid or custom.
	 */
	if ((ErrTable[ErrorId].Action != XPLMI_EM_ACTION_INVALID) &&
		(ErrTable[ErrorId].Action != XPLMI_EM_ACTION_CUSTOM)) {
		ErrTable[ErrorId].Action = XPLMI_EM_ACTION_NONE;
	}
}


/*****************************************************************************/
/**
 * @brief	This function initializes the error actions in LPD Slcr Disables
 * 		all the LPD SLCR error actions and registers default action.
 *
 * @return
 * 			- XST_SUCCESS always.
 *
*****************************************************************************/
int XPlmi_LpdSlcrEmInit(void)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 ErrIndex;
	u32 LpdSlcrErrStatus[XPLMI_LPDSLCR_MAX_ERR_CNT];
	u32 RegMask;
	XPlmi_Error_t *ErrTable = XPlmi_GetErrorTable();

	for (ErrIndex = 0U; ErrIndex < XPLMI_LPDSLCR_MAX_ERR_CNT; ErrIndex++) {
		/**
		 * - Disable all the Error Actions
		 */
		(void)XPlmi_EmDisableLpdSlcrErrors(GET_LPDSLCR_PMC0_ERR_MASK(ErrIndex),
					MASK32_ALL_HIGH);
		(void)XPlmi_EmDisableLpdSlcrErrors(GET_LPDSLCR_PMC1_ERR_MASK(ErrIndex),
					MASK32_ALL_HIGH);
		(void)XPlmi_EmDisableLpdSlcrErrors(GET_LPDSLCR_PMC2_ERR_MASK(ErrIndex),
					MASK32_ALL_HIGH);
		(void)XPlmi_EmDisableLpdSlcrErrors(GET_LPDSLCR_PMC3_ERR_MASK(ErrIndex),
					MASK32_ALL_HIGH);

		LpdSlcrErrStatus[ErrIndex] = XPlmi_In32(LPD_SLCR_REG_EAM_ERR0_STATUS +
					(ErrIndex * LPD_SLCR_GLOBAL_REG_ERR_OFFSET));
		/* Saving to RTCA Area */
		XPlmi_Out32(GET_RTCFG_LPDSLCR_ERR_ADDR(ErrIndex), LpdSlcrErrStatus[ErrIndex]);
		if (LpdSlcrErrStatus[ErrIndex] != 0U) {
			XPlmi_Printf(DEBUG_GENERAL, "LPD_SLCR_EAM_ERR%d_STATUS: "
				"0x%08x\n\r", ErrIndex, LpdSlcrErrStatus[ErrIndex]);
		}
		/**
		 * - Clear the error status registers.
		 */
		XPlmi_Out32(LPD_SLCR_REG_EAM_ERR0_STATUS +
			(ErrIndex * LPD_SLCR_GLOBAL_REG_ERR_OFFSET), MASK32_ALL_HIGH);

		/**
		 * - Set the default actions as defined in the Error table.
		 */
		for (Index = GET_LPDSLCR_ERR_START(ErrIndex); Index <
				GET_LPDSLCR_ERR_END(ErrIndex); Index++) {
			if (Index >= XPLMI_ERROR_LPDSLCR_ERR_MAX) {
				break;
			}
			if (ErrTable[Index].Action != XPLMI_EM_ACTION_INVALID) {
				RegMask = XPlmi_ErrRegMask(Index);
				if (XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_LPD_SLCR_ERR1 +
					(ErrIndex * XPLMI_EVENT_ERROR_OFFSET),
					RegMask, ErrTable[Index].Action,
					ErrTable[Index].Handler, ErrTable[Index].SubsystemId) != XST_SUCCESS) {
					XPlmi_Printf(DEBUG_GENERAL,
						"Warning: XPlmi_LpdSlcrEmInit: Failed to "
						"set action for LPD SLCR ERR%d: %u\r\n",
						Index , Index);
				}
			}
		}
	}

	Status = XST_SUCCESS;

	return Status;
}



/*****************************************************************************/
/**
 * @brief	This function sets the error action as prescribed by the command.
 *			Command payload parameters are
 *				* Error Node ID
 *				* Error Action
 *					0 - Invalid
 *					1 - POR
 *					2 - SRST
 *					3 - Custom(Not supported)
 *					4 - ErrOut
 *					5 - Subsystem Shutdown
 *					6 - Subsystem Restart
 *					7 - Print to log
 *					8 - SLD
 *					9 - SLD with TRI
 *					10 - None
 *			* Error ID Mask
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_INVALID_NODE_ID on invalid node ID.
 * 			- XPLMI_INVALID_ERROR_ACTION on invalid error action.
 * 			- XPLMI_CANNOT_CHANGE_ACTION if failed to change error action.
 * 			- XPLMI_LPD_UNINITIALIZED if LPD failed to initialize.
 *
 *****************************************************************************/
int XPlmi_Versal2Ve2VmSetAction(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;

	XPlmi_EventType NodeType =
			(XPlmi_EventType)XPlmi_EventNodeType(Cmd->Payload[0U]);
	u32 ErrorAction = Cmd->Payload[1U];
	u32 ErrorMasks = Cmd->Payload[2U];
	u32 SubsystemId;

	/* Check if it is an IPI or CDO request*/
	if (Cmd->IpiMask != 0U) {
		SubsystemId = Cmd->SubsystemId;
	} else {
		SubsystemId = XPlmi_GetEmSubsystemId();
	}

	XPlmi_Printf(DEBUG_DETAILED,
		"%s: NodeId: 0x%0x,  ErrorAction: 0x%0x, ErrorMasks: 0x%0x, SubsystemId: 0x%0x\n\r",
		 __func__, Cmd->Payload[0U], ErrorAction, ErrorMasks, SubsystemId);

	/* Do not allow CUSTOM error action as it is not supported */
	if ((XPLMI_EM_ACTION_CUSTOM == ErrorAction) ||
		(ErrorAction >= XPLMI_EM_ACTION_MAX) ||
		(XPLMI_EM_ACTION_INVALID == ErrorAction)) {
		XPlmi_Printf(DEBUG_INFO,
			"Error: XPlmi_CmdEmSetAction: Invalid/unsupported error "
			"action 0x%x received for error mask 0x%x", ErrorAction, ErrorMasks);
		Status = XPLMI_INVALID_ERROR_ACTION;
		goto END;
	}

	/* Do not allow invalid node id */
	if (NodeType > XPLMI_NODETYPE_EVENT_SW_ERR) {
		Status = XPLMI_INVALID_NODE_ID;
		goto END;
	}

	if (NodeType == XPLMI_NODETYPE_EVENT_PMC_ERR3) {
		/* PSX_EAM_E0 - SRST
		 * PSX_EAM_E1 - POR
		 * PSX_EAM_E2 - Error Out
		 * PSX_EAM_E3 - IRQ
		 */
		if (ErrorMasks & XPLMI_BIT(PSX_EAM_E0_BIT_SHIFT)) {
			if (ErrorAction != XPLMI_EM_ACTION_SRST) {
				XPlmi_Printf(DEBUG_INFO, "Error: "
					"XPlmi_CmdEmSetAction: PSX_EAM_E0 error"
					" only action allowed is "
					"SRST\r\n");
				Status = XPLMI_CANNOT_CHANGE_ACTION;
				goto END;
			}
		} else if (ErrorMasks & XPLMI_BIT(PSX_EAM_E1_BIT_SHIFT)) {
			if (ErrorAction != XPLMI_EM_ACTION_POR) {
				XPlmi_Printf(DEBUG_INFO, "Error: "
					"XPlmi_CmdEmSetAction: PSX_EAM_E1 error"
					" only action allowed is "
					"POR\r\n");
				Status = XPLMI_CANNOT_CHANGE_ACTION;
				goto END;
			}

		} else if (ErrorMasks & XPLMI_BIT(PSX_EAM_E2_BIT_SHIFT)) {
			if (ErrorAction != XPLMI_EM_ACTION_ERROUT) {
				XPlmi_Printf(DEBUG_INFO, "Error: "
					"XPlmi_CmdEmSetAction: PSX_EAM_E2 error"
					" only action allowed is "
					"ERROUT\r\n");
				Status = XPLMI_CANNOT_CHANGE_ACTION;
				goto END;
			}
		} else if (ErrorMasks & XPLMI_BIT(PSX_EAM_E3_BIT_SHIFT)) {
			XPlmi_Printf(DEBUG_INFO, "Error: XPlmi_CmdEmSetAction: "
				"Error Action cannot be changed for  PSX_EAM_E3\r\n");
				Status = XPLMI_CANNOT_CHANGE_ACTION;
				goto END;
			}
	}

	/*
	 * Allow error action setting for LPD SLCR errors only if LPD is initialized
	 */
	if ((XPlmi_GetEventIndex(NodeType) ==  XPLMI_NODETYPE_EVENT_LPDSLCR_INDEX) &&
		(XPlmi_IsLpdInitialized() != (u8)TRUE)) {
		XPlmi_Printf(DEBUG_INFO, "LPD is not initialized to configure "
				"errors and actions in LPD SLCR\n\r");
		Status = XPLMI_LPD_UNINITIALIZED;
		goto END;
	}

	Status = XPlmi_RestrictErrActions(NodeType, ErrorMasks, ErrorAction);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_EmSetAction(Cmd->Payload[0U], ErrorMasks, (u8)ErrorAction, NULL, SubsystemId);
	if(Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}
