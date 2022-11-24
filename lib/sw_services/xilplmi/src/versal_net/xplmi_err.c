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

/************************** Constant Definitions *****************************/
#define XPLMI_ERROR_TABLE_DS_VER	(1U)
#define XPLMI_ERROR_TABLE_DS_LCVER	(1U)

#define XPLMI_IS_PSMCR_CHANGED_VERSION		(1U)
#define XPLMI_IS_PSMCR_CHANGED_LCVERSION	(1U)

#define XPLMI_NUM_ERROUTS_VERSION	(1U)
#define XPLMI_NUM_ERROUTS_LCVERSION	(1U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

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
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_CUSTOM, .SubsystemId = 0U, },
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
	[XPLMI_ERROR_PMC_RSRV0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
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
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PMC_RSRV5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_PMX_CORR_ERR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_PMX_UNCORR_ERR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_SSIT2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_IOU_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_IOU_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DFX_UXPT_ACT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DICE_CDI_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVIK_PRIV] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NXTSW_CDI_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVAK_PRIV] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DME_PUB_X] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DME_PUB_Y] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVAK_PUB_X] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVAK_PUB_Y] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVIK_PUB_X] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_DEVIK_PUB_Y] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PCR_PAR] =
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
	[XPLMI_ERROR_PSMX_CHK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_APLL1_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_APLL2_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FLXPLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_PSM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_PSM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_USB2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_UXPT_ACT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_LPD_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_LPD_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_OCM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_OCM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_FPD_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_FPD_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_IOU_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_INT_IOU_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUA_LOCKSTEP] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPUB_LOCKSTEP] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_GIC_AXI] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_GIC_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CPM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CPM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_CPI] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_WDT3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_MEM_SPLITTER0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_AXI_PAR_SPLITTER0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_MEM_SPLITTER1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_AXI_PAR_SPLITTER1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_MEM_SPLITTER2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_AXI_PAR_SPLITTER2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_MEM_SPLITTER3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_AXI_PAR_SPLITTER3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_CLUSTER0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_CLUSTER1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_CLUSTER2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_APU_CLUSTER3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_WWDT0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_WWDT1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_ADMA_LOCKSTEP] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_IPI] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM_BANK0_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM_BANK1_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM_BANK0_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_OCM_BANK1_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPXAFIFS_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPXAFIFS_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_GLITCH_DETECT0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPX_GLITCH_DETECT1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FWALL_WR_NOC_NMU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FWALL_RD_NOC_NMU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FWALL_NOC_NSU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B18_R52_A0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B18_R52_A1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B18_R52_B0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B18_R52_B1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A0_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A0_TFATAL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A0_TIMEOUT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B24_B20_RPUA0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B25_RPUA0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A1_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A1_TFATAL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_A1_TIMEOUT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B24_B20_RPUA1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B25_RPUA1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B0_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B0_TFATAL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B0_TIMEOUT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B24_B20_RPUB0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B25_RPUB0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B1_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B1_TFATAL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_R52_B1_TIMEOUT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B24_B20_RPUB1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_B25_RPUB1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSM_RSRV1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_INVALID, .SubsystemId = 0U, },
	[XPLMI_ERROR_PCIL_RPU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPXAFIFS_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPXAFIFS_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_CMN_1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_CMN_2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_CMN_3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_PSX_CML] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_INT_WRAP] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FPD_RST_MON] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_LPD_RST_CLK_MON] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_FATAL_AFI_FM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NFATAL_AFI_FM_LPX] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NFATAL_AFI_FM0_FPX] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NFATAL_AFI_FM1_FPX] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NFATAL_AFI_FM2_FPX] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_NFATAL_AFI_FM3_FPX] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPU_CLUSTERA] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
	[XPLMI_ERROR_RPU_CLUSTERB] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, .SubsystemId = 0U, },
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
	u32 NodeType;

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
		NodeType = XPlmi_EventNodeType(ErrorNodeId);
		if (XPlmi_EmConfig(NodeType, ErrIndex, ErrorTable[ErrIndex].Action,
			ErrorTable[ErrIndex].Handler) != XST_SUCCESS) {
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
u8 XPlmi_GetEventIndex(u32 ErrorNodeType)
{
	u8 Index;

	switch ((XPlmi_EventType)ErrorNodeType) {
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

/*****************************************************************************/
/**
 * @brief	This function disables PMC EAM Errors
 *
 * @param	ErrIndex is the Index of PMC EAM regiser
 * @param	RegMask is the register mask of the Error

 * @return	None
 *
 *****************************************************************************/
void XPlmi_DisablePmcErrAction(u32 ErrIndex, u32 RegMask)
{
	(void)XPlmi_EmDisable(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1 +
		(ErrIndex * XPLMI_EVENT_ERROR_OFFSET), RegMask);
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
* @return	XST_SUCCESS on success and error code on failure
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
