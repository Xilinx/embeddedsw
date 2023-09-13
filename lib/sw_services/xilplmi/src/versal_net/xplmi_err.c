/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal_net/xplmi_err.c
*
* This file contains the PLMI versal_net platform specific code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       dc   07/12/2022 Added support to device state change
*       bm   07/13/2022 Update EAM logic for In-Place PLM Update
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*       kal  01/05/2022 Added support to PCR log update
*       bm   01/05/2023 Notify Other SLRs about Secure Lockdown
*       sk   01/13/2023 Updated Error Handler and Action table
*                       CPM5N,XMPU,XPPU error/event handling
*       rama 01/19/2023 Updated ErrorTable to support XilSem errors
*       dd   03/28/2023 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.02  sk   08/17/2023 Updated XPlmi_EmConfig arguments
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
#include "xil_error_node.h"
#include "xplmi_plat.h"
#include "xplmi_util.h"
#include "xplmi_generic.h"

/************************** Constant Definitions *****************************/
#define XPLMI_ERROR_TABLE_DS_VER	(1U) /**< Error table data structure version */
#define XPLMI_ERROR_TABLE_DS_LCVER	(1U) /**< Error table data structure LC version */

#define XPLMI_IS_PSMCR_CHANGED_VERSION		(1U) /**< PSMCR changed version status */
#define XPLMI_IS_PSMCR_CHANGED_LCVERSION	(1U) /**< PSMCR changed LC version status */

#define XPLMI_NUM_ERROUTS_VERSION	(1U) /**< ERROUTS version */
#define XPLMI_NUM_ERROUTS_LCVERSION	(1U) /**< ERROUTS LC version */

#define CPM5N_NCR_PCIE0_LINK_UP_EVENT_PROC_ID					(0x1U) /**< CPM5N_NCR_PCIE0 linkup event procedure Id */
#define CPM5N_NCR_PCIE1_LINK_UP_EVENT_PROC_ID					(0x2U) /**< CPM5N_NCR_PCIE1 linkup event procedure Id */
#define CPM5N_NCR_PCIE2_LINK_UP_EVENT_PROC_ID					(0x3U) /**< CPM5N_NCR_PCIE2 linkup event procedure Id */
#define CPM5N_NCR_PCIE3_LINK_UP_EVENT_PROC_ID					(0x4U) /**< CPM5N_NCR_PCIE3 linkup event procedure Id */

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
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_GSW_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_CFU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CFRAME] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_PSM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_SRST, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_PSM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
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
	{ .Handler = XPlmi_ProtUnitErrHandler, .Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
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
	[XPLMI_ERROR_PS_SW_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PS_SW_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_B_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_B_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MB_FATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSMX_CHK] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_APLL1_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_APLL2_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FLXPLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_PSM_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_PSM_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_USB2] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_UXPT_ACT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_LPD_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_LPD_NCR] =
	{ .Handler = XPlmi_ProtUnitErrHandler, .Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_OCM_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_OCM_NCR] =
	{ .Handler = XPlmi_ProtUnitErrHandler, .Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_FPD_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_FPD_NCR] =
	{ .Handler = XPlmi_ProtUnitErrHandler, .Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_IOU_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_IOU_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUA_LOCKSTEP] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUB_LOCKSTEP] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_GIC_AXI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_GIC_ECC] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_CPM_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_CPM_NCR] =
	{ .Handler = XPlmi_CpmErrHandler, .Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_CPI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT2] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT3] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MEM_SPLITTER0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_AXI_PAR_SPLITTER0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MEM_SPLITTER1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_AXI_PAR_SPLITTER1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MEM_SPLITTER2] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_AXI_PAR_SPLITTER2] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_MEM_SPLITTER3] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_AXI_PAR_SPLITTER3] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_CLUSTER0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_CLUSTER1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_CLUSTER2] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_CLUSTER3] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_WWDT0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_WWDT1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_ADMA_LOCKSTEP] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_IPI] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM_BANK0_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM_BANK1_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM_BANK0_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM_BANK1_NCR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPXAFIFS_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPXAFIFS_NCR] =
	{ .Handler = XPlmi_ProtUnitErrHandler, .Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_GLITCH_DETECT0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_GLITCH_DETECT1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FWALL_WR_NOC_NMU] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FWALL_RD_NOC_NMU] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FWALL_NOC_NSU] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B18_R52_A0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B18_R52_A1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B18_R52_B0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B18_R52_B1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A0_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A0_TFATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A0_TIMEOUT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B24_B20_RPUA0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B25_RPUA0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A1_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A1_TFATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A1_TIMEOUT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B24_B20_RPUA1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B25_RPUA1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B0_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B0_TFATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B0_TIMEOUT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B24_B20_RPUB0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B25_RPUB0] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B1_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B1_TFATAL] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B1_TIMEOUT] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B24_B20_RPUB1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_B25_RPUB1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PCIL_RPU] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPXAFIFS_CR] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPXAFIFS_NCR] =
	{ .Handler = XPlmi_ProtUnitErrHandler, .Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_CMN_1] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_CMN_2] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_CMN_3] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_CML] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_INT_WRAP] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_RST_MON] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_RST_CLK_MON] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_FATAL_AFI_FM] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NFATAL_AFI_FM_LPX] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NFATAL_AFI_FM0_FPX] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NFATAL_AFI_FM1_FPX] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NFATAL_AFI_FM2_FPX] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_NFATAL_AFI_FM3_FPX] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPU_CLUSTERA] =
	{ .Handler = XPlmi_ErrPrintToLog, .Action = XPLMI_EM_ACTION_PRINT_TO_LOG, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPU_CLUSTERB] =
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
 * @brief	This function provides IsPsmChanged variable
 *
 * @return	Pointer to IsPsmCrChanged
 *
 *****************************************************************************/
u32 *XPlmi_GetPsmCrState(void)
{
	static u32 IsPsmCrChanged __attribute__ ((aligned(4U))) = (u32)FALSE;

	EXPORT_GENERIC_DS(IsPsmCrChanged, XPLMI_IS_PSMCR_CHANGED_DS_ID,
	XPLMI_IS_PSMCR_CHANGED_VERSION, XPLMI_IS_PSMCR_CHANGED_LCVERSION,
	sizeof(IsPsmCrChanged), (u32)(UINTPTR)&IsPsmCrChanged);

	return &IsPsmCrChanged;
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
		/* All custom actions except PSM NCR will be disabled */
		if ((ErrorTable[ErrIndex].Action == XPLMI_EM_ACTION_CUSTOM) &&
				(ErrIndex != XPLMI_ERROR_PMC_PSM_NCR)) {
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
		case XPLMI_NODETYPE_EVENT_PSM_ERR1:
		case XPLMI_NODETYPE_EVENT_PSM_ERR2:
		case XPLMI_NODETYPE_EVENT_PSM_ERR3:
		case XPLMI_NODETYPE_EVENT_PSM_ERR4:
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

/****************************************************************************/
/**
* @brief    This function handles the CPM_NCR PCIE link up event
*
* @param    Cpm5NPcieCdxIrStatusReg is the PCIE0/1/2/3 IR status register address
* @param    Cpm5NCdxPcieBReg is the Pcie Bridge Misc Status register address
* @param    ProcId is the ProcId for PCIE0/1/2/3 link up event
*
* @return   None
*
****************************************************************************/
void XPlmi_HandleLinkUpEvent(u32 Cpm5NPcieCdxIrStatusReg,
		u32 Cpm5NCdxPcieBReg, u32 ProcId)
{
	int Status = XST_FAILURE;
	u32 PcieCdxIrStatus = XPlmi_In32(Cpm5NPcieCdxIrStatusReg);
	u32 PcieCdxIrEnable = ((~XPlmi_In32(Cpm5NPcieCdxIrStatusReg + 4U)) &
			CPM5N_SLCR_CDX_INTERRUPT_3_MASK);
	u32 LinkUpEvent = XPlmi_In32(Cpm5NCdxPcieBReg);
	u32 LinkUpEventEnable = (XPlmi_In32(Cpm5NCdxPcieBReg + 4U) &
			CPM5N_LINK_UP_EVENT_MASK);

	/*
	 * Check if PCIE Cdx Interrupt is enabled and
	 * Check if received error is PCIE CDX Interrupt
	 */
	if ((PcieCdxIrEnable != 0U) &&
			((PcieCdxIrStatus & CPM5N_SLCR_CDX_INTERRUPT_3_MASK) ==
					CPM5N_SLCR_CDX_INTERRUPT_3_MASK)) {
		/*
		 * Check if link up event is enabled and
		 * Check if received event is link up event
		 */
		if ((LinkUpEventEnable != 0U) &&
				((LinkUpEvent & CPM5N_LINK_UP_EVENT_MASK) ==
						CPM5N_LINK_UP_EVENT_MASK)) {
			/* Execute proc for PCIE link up */
			Status = XPlmi_ExecuteProc(ProcId);
			if (Status != XST_SUCCESS) {
				XPlmi_Printf(DEBUG_GENERAL, "Error in handling PCIE "
						"link up event: 0x%x\r\n", Status);
				/*
				 * Update error manager with error received
				 * while executing proc
				 */
				XPlmi_ErrMgr(Status);
			}
			/* Clear PCIE Cdx Interrupt */
			XPlmi_Out32(Cpm5NPcieCdxIrStatusReg, CPM5N_SLCR_CDX_INTERRUPT_3_MASK);

			/* Clear PCIE Link UP event */
			XPlmi_Out32(Cpm5NCdxPcieBReg, CPM5N_LINK_UP_EVENT_MASK);
		} else {
			/* Received error is other than Link up error */
			XPlmi_Printf(DEBUG_GENERAL, "Received error is other than "
					"link Up event: 0x%x\r\n", LinkUpEvent);
		}
	} else {
		/* Received error is other than PCIE Cdx Interrupt */
		XPlmi_Printf(DEBUG_GENERAL, "Received error is other than "
				"PCIE Cdx:3 event: 0x%x\r\n", PcieCdxIrStatus);
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
	u32 CpmErrors = XPlmi_In32(CPM5N_SLCR_PS_UNCORR_IR_STATUS);
	u32 PciexErrEnable = ((~XPlmi_In32(CPM5N_SLCR_PS_UNCORR_IR_MASK)) &
			(CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK |
				CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK |
				CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE2_MASK |
				CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE3_MASK));

	(void)ErrorNodeId;
	(void)RegMask;

	if((CpmErrors & CPM5N_SLCR_PS_UNCORR_IR_STATUS_CDX_ERR_MASK) !=
			CPM5N_SLCR_PS_UNCORR_IR_STATUS_CDX_ERR_MASK) {
		goto END;
	}

	/* Check if PCIE0/1/2/3 errors are enabled */
	if (PciexErrEnable != 0U) {
		/* Check if CPM_NCR error is PCIE0 error */
		if ((CpmErrors & CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK) ==
				CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK) {
			/* Handle PCIE0 link up event */
			XPlmi_Printf(DEBUG_GENERAL, "Received CPM PCIE0 interrupt\r\n");
			XPlmi_HandleLinkUpEvent(CPM5N_SLCR_CDX_IR_STATUS,
					CPM5N_CDX_PCIEB_CSR_MISC_EVENT_STATUS, CPM5N_NCR_PCIE0_LINK_UP_EVENT_PROC_ID);
			/* Clear PCIE0 error */
			XPlmi_Out32(CPM5N_SLCR_PS_UNCORR_IR_STATUS,
					CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK);
		}

		/* Check if CPM_NCR error is PCIE1 error */
		if ((CpmErrors & CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK) ==
				CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK) {
			/* Handle PCIE1 link up event */
			XPlmi_Printf(DEBUG_GENERAL, "Received CPM PCIE1 interrupt\r\n");
			XPlmi_HandleLinkUpEvent(CPM5N_SLCR_CDX_IR_STATUS,
					CPM5N_CDX_PCIEB1_CSR_MISC_EVENT_STATUS, CPM5N_NCR_PCIE1_LINK_UP_EVENT_PROC_ID);
			/* Clear PCIE1 error */
			XPlmi_Out32(CPM5N_SLCR_PS_UNCORR_IR_STATUS,
					CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK);
		}
		/* Check if CPM_NCR error is PCIE2 error */
		if ((CpmErrors & CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE2_MASK) ==
				CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE2_MASK) {
			/* Handle PCIE2 link up event */
			XPlmi_Printf(DEBUG_GENERAL, "Received CPM PCIE2 interrupt\r\n");
			XPlmi_HandleLinkUpEvent(CPM5N_SLCR_CDX_IR_STATUS,
					CPM5N_CDX_PCIEB2_CSR_MISC_EVENT_STATUS, CPM5N_NCR_PCIE2_LINK_UP_EVENT_PROC_ID);
			/* Clear PCIE2 error */
			XPlmi_Out32(CPM5N_SLCR_PS_UNCORR_IR_STATUS,
					CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE2_MASK);
		}
		/* Check if CPM_NCR error is PCIE3 error */
		if ((CpmErrors & CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE3_MASK) ==
				CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE3_MASK) {
			/* Handle PCIE3 link up event */
			XPlmi_Printf(DEBUG_GENERAL, "Received CPM PCIE3 interrupt\r\n");
			XPlmi_HandleLinkUpEvent(CPM5N_SLCR_CDX_IR_STATUS,
					CPM5N_CDX_PCIEB3_CSR_MISC_EVENT_STATUS, CPM5N_NCR_PCIE3_LINK_UP_EVENT_PROC_ID);
			/* Clear PCIE3 error */
			XPlmi_Out32(CPM5N_SLCR_PS_UNCORR_IR_STATUS,
					CPM5N_SLCR_PS_UNCORR_IR_STATUS_PCIE3_MASK);
		}
	} else {
		/* Only PCIE0/1/2/3 errors are handled */
		XPlmi_Printf(DEBUG_GENERAL, "Unhandled CPM_NCR error: 0x%x\r\n",
				CpmErrors);
	}

END:
	return;

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
	u32 RegVal;

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

	case XPLMI_ERROR_INT_LPD_NCR:
		RegVal = XPlmi_In32(LDP_INT_TOP_LVL_UNCORR_ERR_SRC_REGS_ADDR);
		if ((RegVal & LPD_INT_UNCORR_ERR_SRC_REGS_0_MASK) == LPD_INT_UNCORR_ERR_SRC_REGS_0_MASK) {
			RegVal = XPlmi_In32(LPD_INT_UNCORR_ERR_SRC_REGS_0_ADDR);
			if ((RegVal & INTLPX_XPPU_FIREWALL_MASK) == INTLPX_XPPU_FIREWALL_MASK) {
				XPlmi_XppuErrHandler(LPD_XPPU_BASEADDR, "LPD_XPPU");
			}
		}
		break;

	case XPLMI_ERROR_INT_OCM_NCR:
		RegVal = XPlmi_In32(OCM_INT_TOP_LVL_UNCORR_ERR_SRC_REGS_ADDR);
		if ((RegVal & OCM_INT_UNCORR_ERR_SRC_REGS_0_MASK) == OCM_INT_UNCORR_ERR_SRC_REGS_0_MASK) {
			RegVal = XPlmi_In32(OCM_INT_UNCORR_ERR_SRC_REGS_0_ADDR);
			if ((RegVal & INTOCM_XMPU_SLAVES_FIREWALL_MASK) == INTOCM_XMPU_SLAVES_FIREWALL_MASK) {
				XPlmi_XmpuErrHandler(OCM_SLVS_XMPU_BASEADDR, "OCM_SLAVE_XMPU");
			}

			if ((RegVal & INTOCM_XMPU1_FIREWALL_MASK) == INTOCM_XMPU1_FIREWALL_MASK) {
				XPlmi_XmpuErrHandler(OCM1_XMPU_BASEADDR, "OCM1_XMPU");
			}

			if ((RegVal & INTOCM_XMPU0_FIREWALL_MASK) == INTOCM_XMPU0_FIREWALL_MASK) {
				XPlmi_XmpuErrHandler(OCM0_XMPU_BASEADDR, "OCM0_XMPU");
			}
		}
		break;

	case XPLMI_ERROR_INT_FPD_NCR:
		RegVal = XPlmi_In32(FPD_INT_TOP_LVL_UNCORR_ERR_SRC_REGS_ADDR);
		if ((RegVal & FPD_INT_UNCORR_ERR_SRC_REGS_1_MASK) == FPD_INT_UNCORR_ERR_SRC_REGS_1_MASK) {
			RegVal = XPlmi_In32(FPD_INT_UNCORR_ERR_SRC_REGS_1_ADDR);
			if ((RegVal & FPX_XMPU_PKI_FIREWALL_MASK) == FPX_XMPU_PKI_FIREWALL_MASK) {
				XPlmi_XmpuErrHandler(FPD_PKI_XMPU_BASEADDR, "FPX_PKI_XMPU");
			}

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

	case XPLMI_ERROR_LPXAFIFS_NCR:
		XPlmi_XmpuErrHandler(LPD_AFIFS_XMPU_BASEADDR, "LPD_AFIFS_XMPU");
		break;

	case XPLMI_ERROR_FPXAFIFS_NCR:
		XPlmi_XmpuErrHandler(FPD_AFIFS_XMPU_BASEADDR, "FPD_AFIFS_XMPU");
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
