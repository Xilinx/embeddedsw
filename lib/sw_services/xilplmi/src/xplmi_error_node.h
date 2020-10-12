/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_error_node.h
*
* This is the file which contains node IDs information for error events.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/12/2019 Initial release
* 1.01  ma   02/28/2020 Error actions related changes
*       bsv  04/04/2020 Code clean up
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLMI_ERROR_NODE_H
#define XPLMI_ERROR_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/* Error Event Node Ids */
#define XPLMI_EVENT_ERROR_PMC_ERR1	(0x28100000U)
#define XPLMI_EVENT_ERROR_PMC_ERR2	(0x28104000U)
#define XPLMI_EVENT_ERROR_PSM_ERR1	(0x28108000U)
#define XPLMI_EVENT_ERROR_PSM_ERR2	(0x2810C000U)

#define XPLMI_NODE_TYPE_MASK		(0xFC000U)
#define XPLMI_NODE_TYPE_SHIFT		(0xEU)

#define XPLMI_NODEIDX_ERROR_PMCERR1_MAX	(0x20U)
#define XPLMI_NODEIDX_ERROR_PMCERR2_MAX	(0x40U)
#define XPLMI_NODEIDX_ERROR_PSMERR1_MAX	(0x60U)
#define XPLMI_NODEIDX_ERROR_PSMERR2_MAX	(0x75U)

typedef enum {
	/* Event subclasses */
	XPLMI_NODESUBCL_EVENT_ERROR = 0x1U,
	XPLMI_NODESUBCL_EVENT_INTERRUPT, /**< 0x2U */
} XPlmi_EventSubclass;

typedef enum {
	/* Event error types */
	XPLMI_NODETYPE_EVENT_PMC_ERR1 = 0x0U,
	XPLMI_NODETYPE_EVENT_PMC_ERR2, /**< 0x1U */
	XPLMI_NODETYPE_EVENT_PSM_ERR1, /**< 0x2U */
	XPLMI_NODETYPE_EVENT_PSM_ERR2, /**< 0x3U */
} XPlmi_EventType;

/*
 * Error Event IDs
 */
typedef enum {
	/* PMC ERR1 errors */
	XPLMI_NODEIDX_ERROR_BOOT_CR = 0x0U,
	XPLMI_NODEIDX_ERROR_BOOT_NCR, /**< 0x1U */
	XPLMI_NODEIDX_ERROR_FW_CR, /**< 0x2U */
	XPLMI_NODEIDX_ERROR_FW_NCR, /**< 0x3U */
	XPLMI_NODEIDX_ERROR_GSW_CR, /**< 0x4U */
	XPLMI_NODEIDX_ERROR_GSW_NCR, /**< 0x5U */
	XPLMI_NODEIDX_ERROR_CFU, /**< 0x6U */
	XPLMI_NODEIDX_ERROR_CFRAME, /**< 0x7U */
	XPLMI_NODEIDX_ERROR_PMC_PSM_CR, /**< 0x8U */
	XPLMI_NODEIDX_ERROR_PMC_PSM_NCR, /**< 0x9U */
	XPLMI_NODEIDX_ERROR_DDRMB_CR, /**< 0xAU */
	XPLMI_NODEIDX_ERROR_DDRMB_NCR, /**< 0xBU */
	XPLMI_NODEIDX_ERROR_NOCTYPE1_CR, /**< 0xCU */
	XPLMI_NODEIDX_ERROR_NOCTYPE1_NCR, /**< 0xDU */
	XPLMI_NODEIDX_ERROR_NOCUSER, /**< 0xEU */
	XPLMI_NODEIDX_ERROR_MMCM, /**< 0xFU */
	XPLMI_NODEIDX_ERROR_ME_CR, /**< 0x10U */
	XPLMI_NODEIDX_ERROR_ME_NCR, /**< 0x11U */
	XPLMI_NODEIDX_ERROR_DDRMC_CR, /**< 0x12U */
	XPLMI_NODEIDX_ERROR_DDRMC_NCR, /**< 0x13U */
	XPLMI_NODEIDX_ERROR_GT_CR, /**< 0x14U */
	XPLMI_NODEIDX_ERROR_GT_NCR, /**< 0x15U */
	XPLMI_NODEIDX_ERROR_PLSMON_CR, /**< 0x16U */
	XPLMI_NODEIDX_ERROR_PLSMON_NCR, /**< 0x17U */
	XPLMI_NODEIDX_ERROR_PL0, /**< 0x18U */
	XPLMI_NODEIDX_ERROR_PL1, /**< 0x19U */
	XPLMI_NODEIDX_ERROR_PL2, /**< 0x1AU */
	XPLMI_NODEIDX_ERROR_PL3, /**< 0x1BU */
	XPLMI_NODEIDX_ERROR_NPIROOT, /**< 0x1CU */
	XPLMI_NODEIDX_ERROR_SSIT3, /**< 0x1DU */
	XPLMI_NODEIDX_ERROR_SSIT4, /**< 0x1EU */
	XPLMI_NODEIDX_ERROR_SSIT5, /**< 0x1FU */
	/* PMC ERR2 errors */
	XPLMI_NODEIDX_ERROR_PMCAPB = 32U, /**< 0x20U */
	XPLMI_NODEIDX_ERROR_PMCROM, /**< 0x21U */
	XPLMI_NODEIDX_ERROR_MB_FATAL0, /**< 0x22U */
	XPLMI_NODEIDX_ERROR_MB_FATAL1, /**< 0x23U */
	XPLMI_NODEIDX_ERROR_PMCPAR, /**< 0x24U */
	XPLMI_NODEIDX_ERROR_PMC_CR, /**< 0x25U */
	XPLMI_NODEIDX_ERROR_PMC_NCR, /**< 0x26U */
	XPLMI_NODEIDX_ERROR_PMCSMON0, /**< 0x27U */
	XPLMI_NODEIDX_ERROR_PMCSMON1, /**< 0x28U */
	XPLMI_NODEIDX_ERROR_PMCSMON2, /**< 0x29U */
	XPLMI_NODEIDX_ERROR_PMCSMON3, /**< 0x2AU */
	XPLMI_NODEIDX_ERROR_PMCSMON4, /**< 0x2BU */
	XPLMI_NODEIDX_ERROR_PMC_RSRV1, /**< 0x2CU */
	XPLMI_NODEIDX_ERROR_PMC_RSRV2, /**< 0x2DU */
	XPLMI_NODEIDX_ERROR_PMC_RSRV3, /**< 0x2EU */
	XPLMI_NODEIDX_ERROR_PMCSMON8, /**< 0x2FU */
	XPLMI_NODEIDX_ERROR_PMCSMON9, /**< 0x30U */
	XPLMI_NODEIDX_ERROR_CFI, /**< 0x31U */
	XPLMI_NODEIDX_ERROR_SEUCRC, /**< 0x32U */
	XPLMI_NODEIDX_ERROR_SEUECC, /**< 0x33U */
	XPLMI_NODEIDX_ERROR_PMC_RSRV4, /**< 0x34U */
	XPLMI_NODEIDX_ERROR_PMC_RSRV5, /**< 0x35U */
	XPLMI_NODEIDX_ERROR_RTCALARM, /**< 0x36U */
	XPLMI_NODEIDX_ERROR_NPLL, /**< 0x37U */
	XPLMI_NODEIDX_ERROR_PPLL, /**< 0x38U */
	XPLMI_NODEIDX_ERROR_CLKMON, /**< 0x39U */
	XPLMI_NODEIDX_ERROR_PMCTO, /**< 0x3AU */
	XPLMI_NODEIDX_ERROR_PMCXMPU, /**< 0x3BU */
	XPLMI_NODEIDX_ERROR_PMCXPPU, /**< 0x3CU */
	XPLMI_NODEIDX_ERROR_SSIT0, /**< 0x3DU */
	XPLMI_NODEIDX_ERROR_SSIT1, /**< 0x3EU */
	XPLMI_NODEIDX_ERROR_SSIT2, /**< 0x3FU */
	/* PSM ERR1 errors */
	XPLMI_NODEIDX_ERROR_PS_SW_CR = 64U, /**< 0x40U */
	XPLMI_NODEIDX_ERROR_PS_SW_NCR, /**< 0x41U */
	XPLMI_NODEIDX_ERROR_PSM_B_CR, /**< 0x42U */
	XPLMI_NODEIDX_ERROR_PSM_B_NCR, /**< 0x43U */
	XPLMI_NODEIDX_ERROR_MB_FATAL, /**< 0x44U */
	XPLMI_NODEIDX_ERROR_PSM_CR, /**< 0x45U */
	XPLMI_NODEIDX_ERROR_PSM_NCR, /**< 0x46U */
	XPLMI_NODEIDX_ERROR_OCM_ECC, /**< 0x47U */
	XPLMI_NODEIDX_ERROR_L2_ECC, /**< 0x48U */
	XPLMI_NODEIDX_ERROR_RPU_ECC, /**< 0x49U */
	XPLMI_NODEIDX_ERROR_RPU_LS, /**< 0x4AU */
	XPLMI_NODEIDX_ERROR_RPU_CCF, /**< 0x4BU */
	XPLMI_NODEIDX_ERROR_GIC_AXI, /**< 0x4CU */
	XPLMI_NODEIDX_ERROR_GIC_ECC, /**< 0x4DU */
	XPLMI_NODEIDX_ERROR_APLL_LOCK, /**< 0x4EU */
	XPLMI_NODEIDX_ERROR_RPLL_LOCK, /**< 0x4FU */
	XPLMI_NODEIDX_ERROR_CPM_CR, /**< 0x50U */
	XPLMI_NODEIDX_ERROR_CPM_NCR, /**< 0x51U */
	XPLMI_NODEIDX_ERROR_LPD_APB, /**< 0x52U */
	XPLMI_NODEIDX_ERROR_FPD_APB, /**< 0x53U */
	XPLMI_NODEIDX_ERROR_LPD_PAR, /**< 0x54U */
	XPLMI_NODEIDX_ERROR_FPD_PAR, /**< 0x55U */
	XPLMI_NODEIDX_ERROR_IOU_PAR, /**< 0x56U */
	XPLMI_NODEIDX_ERROR_PSM_PAR, /**< 0x57U */
	XPLMI_NODEIDX_ERROR_LPD_TO, /**< 0x58U */
	XPLMI_NODEIDX_ERROR_FPD_TO, /**< 0x59U */
	XPLMI_NODEIDX_ERROR_PSM_TO, /**< 0x5AU */
	XPLMI_NODEIDX_ERROR_XRAM_CR, /**< 0x5BU */
	XPLMI_NODEIDX_ERROR_XRAM_NCR, /**< 0x5CU */
	XPLMI_NODEIDX_ERROR_PSM_RSRV1, /**< 0x5DU */
	XPLMI_NODEIDX_ERROR_PSM_RSRV2, /**< 0x5EU */
	XPLMI_NODEIDX_ERROR_PSM_RSRV3, /**< 0x5FU */
	/* PSM ERR2 errors */
	XPLMI_NODEIDX_ERROR_LPD_SWDT = 96U, /**< 0x60U */
	XPLMI_NODEIDX_ERROR_FPD_SWDT, /**< 0x61U */
	XPLMI_NODEIDX_ERROR_PSM_RSRV4, /**< 0x62U */
	XPLMI_NODEIDX_ERROR_PSM_RSRV5, /**< 0x63U */
	XPLMI_NODEIDX_ERROR_PSM_RSRV6, /**< 0x64U */
	XPLMI_NODEIDX_ERROR_PSM_RSRV7, /**< 0x65U */
	XPLMI_NODEIDX_ERROR_PSM_RSRV8, /**< 0x66U */
	XPLMI_NODEIDX_ERROR_PSM_RSRV9, /**< 0x67U */
	XPLMI_NODEIDX_ERROR_PSM_RSRV10, /**< 0x68U */
	XPLMI_NODEIDX_ERROR_PSM_RSRV11, /**< 0x69U */
	XPLMI_NODEIDX_ERROR_PSM_RSRV12, /**< 0x6AU */
	XPLMI_NODEIDX_ERROR_PSM_RSRV13, /**< 0x6BU */
	XPLMI_NODEIDX_ERROR_PSM_RSRV14, /**< 0x6CU */
	XPLMI_NODEIDX_ERROR_PSM_RSRV15, /**< 0x6DU */
	XPLMI_NODEIDX_ERROR_PSM_RSRV16, /**< 0x6EU */
	XPLMI_NODEIDX_ERROR_PSM_RSRV17, /**< 0x6FU */
	XPLMI_NODEIDX_ERROR_PSM_RSRV18, /**< 0x70U */
	XPLMI_NODEIDX_ERROR_PSM_RSRV19, /**< 0x71U */
	XPLMI_NODEIDX_ERROR_LPD_XMPU, /**< 0x72U */
	XPLMI_NODEIDX_ERROR_LPD_XPPU, /**< 0x73U */
	XPLMI_NODEIDX_ERROR_FPD_XMPU, /**< 0x74U */
} XPlmi_ErrorId;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ERROR_NODE_H */
