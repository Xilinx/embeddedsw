/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/xplmi_error_node.h
*
* This is the file which contains node IDs information for versal error events.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/12/2019 Initial release
* 1.01  ma   02/28/2020 Error actions related changes
*       bsv  04/04/2020 Code clean up
* 1.02  bm   10/14/2020 Code clean up
* 1.03  pj   03/24/2021 Added Software Error node for Healthy boot monitoring
* 1.04  bsv  07/16/2021 Fix doxygen warnings
*       ma   08/19/2021 Renamed error related macros and added event ID macros
* 1.05  bsv  12/24/2021 Move common defines from xilplmi and xilpm to common
*                       folder
*       ma   01/17/2022 Add exceptions to SW errors list
* 1.06  rama 06/28/2022 Added XilSem error IDs to support software event
*                       notification
*       bm   07/06/2022 Refactor versal and versal_net code
* 1.07  ng   11/11/2022 Fixed doxygen file name error
* 1.08  rama 07/19/2023 Added STL error ID to support software event
*                       notification
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

/**@cond xplmi_internal
 * @{
 */

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/* Error Event Node Ids */
#define XPLMI_NODE_TYPE_MASK		(0xFC000U)
#define XPLMI_NODE_TYPE_SHIFT		(0xEU)

typedef enum {
	/* Event error types */
	XPLMI_NODETYPE_EVENT_PMC_ERR1 = 0x0,
	XPLMI_NODETYPE_EVENT_PMC_ERR2, /**< 0x1 */
	XPLMI_NODETYPE_EVENT_PSM_ERR1, /**< 0x2 */
	XPLMI_NODETYPE_EVENT_PSM_ERR2, /**< 0x3 */
	XPLMI_NODETYPE_EVENT_SW_ERR,   /**< 0x4 */
} XPlmi_EventType;

/* PMC ERR1 errors */
#define XPLMI_ERROR_BOOT_CR			(0x0U)
#define XPLMI_ERROR_BOOT_NCR		(0x1U)
#define XPLMI_ERROR_FW_CR			(0x2U)
#define XPLMI_ERROR_FW_NCR			(0x3U)
#define XPLMI_ERROR_GSW_CR			(0x4U)
#define XPLMI_ERROR_GSW_NCR			(0x5U)
#define XPLMI_ERROR_CFU				(0x6U)
#define XPLMI_ERROR_CFRAME			(0x7U)
#define XPLMI_ERROR_PMC_PSM_CR		(0x8U)
#define XPLMI_ERROR_PMC_PSM_NCR		(0x9U)
#define XPLMI_ERROR_DDRMB_CR		(0xAU)
#define XPLMI_ERROR_DDRMB_NCR		(0xBU)
#define XPLMI_ERROR_NOCTYPE1_CR		(0xCU)
#define XPLMI_ERROR_NOCTYPE1_NCR	(0xDU)
#define XPLMI_ERROR_NOCUSER			(0xEU)
#define XPLMI_ERROR_MMCM			(0xFU)
#define XPLMI_ERROR_AIE_CR			(0x10U)
#define XPLMI_ERROR_AIE_NCR			(0x11U)
#define XPLMI_ERROR_DDRMC_CR		(0x12U)
#define XPLMI_ERROR_DDRMC_NCR		(0x13U)
#define XPLMI_ERROR_GT_CR			(0x14U)
#define XPLMI_ERROR_GT_NCR			(0x15U)
#define XPLMI_ERROR_PLSMON_CR		(0x16U)
#define XPLMI_ERROR_PLSMON_NCR		(0x17U)
#define XPLMI_ERROR_PL0				(0x18U)
#define XPLMI_ERROR_PL1				(0x19U)
#define XPLMI_ERROR_PL2				(0x1AU)
#define XPLMI_ERROR_PL3				(0x1BU)
#define XPLMI_ERROR_NPIROOT			(0x1CU)
#define XPLMI_ERROR_SSIT3			(0x1DU)
#define XPLMI_ERROR_SSIT4			(0x1EU)
#define XPLMI_ERROR_SSIT5			(0x1FU)
#define XPLMI_ERROR_PMCERR1_MAX		(0x20U)

/* PMC ERR2 errors */
#define XPLMI_ERROR_PMCAPB			(0x20U)
#define XPLMI_ERROR_PMCROM			(0x21U)
#define XPLMI_ERROR_MB_FATAL0		(0x22U)
#define XPLMI_ERROR_MB_FATAL1		(0x23U)
#define XPLMI_ERROR_PMCPAR			(0x24U)
#define XPLMI_ERROR_PMC_CR			(0x25U)
#define XPLMI_ERROR_PMC_NCR			(0x26U)
#define XPLMI_ERROR_PMCSMON0		(0x27U)
#define XPLMI_ERROR_PMCSMON1		(0x28U)
#define XPLMI_ERROR_PMCSMON2		(0x29U)
#define XPLMI_ERROR_PMCSMON3		(0x2AU)
#define XPLMI_ERROR_PMCSMON4		(0x2BU)
#define XPLMI_ERROR_PMC_RSRV1		(0x2CU)
#define XPLMI_ERROR_PMC_RSRV2		(0x2DU)
#define XPLMI_ERROR_PMC_RSRV3		(0x2EU)
#define XPLMI_ERROR_PMCSMON8		(0x2FU)
#define XPLMI_ERROR_PMCSMON9		(0x30U)
#define XPLMI_ERROR_CFI				(0x31U)
#define XPLMI_ERROR_SEUCRC			(0x32U)
#define XPLMI_ERROR_SEUECC			(0x33U)
#define XPLMI_ERROR_PMC_RSRV4		(0x34U)
#define XPLMI_ERROR_PMC_RSRV5		(0x35U)
#define XPLMI_ERROR_RTCALARM		(0x36U)
#define XPLMI_ERROR_NPLL			(0x37U)
#define XPLMI_ERROR_PPLL			(0x38U)
#define XPLMI_ERROR_CLKMON			(0x39U)
#define XPLMI_ERROR_PMCTO			(0x3AU)
#define XPLMI_ERROR_PMCXMPU			(0x3BU)
#define XPLMI_ERROR_PMCXPPU			(0x3CU)
#define XPLMI_ERROR_SSIT0			(0x3DU)
#define XPLMI_ERROR_SSIT1			(0x3EU)
#define XPLMI_ERROR_SSIT2			(0x3FU)
#define XPLMI_ERROR_PMCERR2_MAX		(0x40U)

/* PSM ERR1 errors */
#define XPLMI_ERROR_PS_SW_CR		(0x40U)
#define XPLMI_ERROR_PS_SW_NCR		(0x41U)
#define XPLMI_ERROR_PSM_B_CR		(0x42U)
#define XPLMI_ERROR_PSM_B_NCR		(0x43U)
#define XPLMI_ERROR_MB_FATAL		(0x44U)
#define XPLMI_ERROR_PSM_CR			(0x45U)
#define XPLMI_ERROR_PSM_NCR			(0x46U)
#define XPLMI_ERROR_OCM_ECC			(0x47U)
#define XPLMI_ERROR_L2_ECC			(0x48U)
#define XPLMI_ERROR_RPU_ECC			(0x49U)
#define XPLMI_ERROR_RPU_LS			(0x4AU)
#define XPLMI_ERROR_RPU_CCF			(0x4BU)
#define XPLMI_ERROR_GIC_AXI			(0x4CU)
#define XPLMI_ERROR_GIC_ECC			(0x4DU)
#define XPLMI_ERROR_APLL_LOCK		(0x4EU)
#define XPLMI_ERROR_RPLL_LOCK		(0x4FU)
#define XPLMI_ERROR_CPM_CR			(0x50U)
#define XPLMI_ERROR_CPM_NCR			(0x51U)
#define XPLMI_ERROR_LPD_APB			(0x52U)
#define XPLMI_ERROR_FPD_APB			(0x53U)
#define XPLMI_ERROR_LPD_PAR			(0x54U)
#define XPLMI_ERROR_FPD_PAR			(0x55U)
#define XPLMI_ERROR_IOU_PAR			(0x56U)
#define XPLMI_ERROR_PSM_PAR			(0x57U)
#define XPLMI_ERROR_LPD_TO			(0x58U)
#define XPLMI_ERROR_FPD_TO			(0x59U)
#define XPLMI_ERROR_PSM_TO			(0x5AU)
#define XPLMI_ERROR_XRAM_CR			(0x5BU)
#define XPLMI_ERROR_XRAM_NCR		(0x5CU)
#define XPLMI_ERROR_PSM_RSRV1		(0x5DU)
#define XPLMI_ERROR_PSM_RSRV2		(0x5EU)
#define XPLMI_ERROR_PSM_RSRV3		(0x5FU)
#define XPLMI_ERROR_PSMERR1_MAX		(0x60U)

/* PSM ERR2 errors */
#define XPLMI_ERROR_LPD_SWDT		(0x60U)
#define XPLMI_ERROR_FPD_SWDT		(0x61U)
#define XPLMI_ERROR_PSM_RSRV4		(0x62U)
#define XPLMI_ERROR_PSM_RSRV5		(0x63U)
#define XPLMI_ERROR_PSM_RSRV6		(0x64U)
#define XPLMI_ERROR_PSM_RSRV7		(0x65U)
#define XPLMI_ERROR_PSM_RSRV8		(0x66U)
#define XPLMI_ERROR_PSM_RSRV9		(0x67U)
#define XPLMI_ERROR_PSM_RSRV10		(0x68U)
#define XPLMI_ERROR_PSM_RSRV11		(0x69U)
#define XPLMI_ERROR_PSM_RSRV12		(0x6AU)
#define XPLMI_ERROR_PSM_RSRV13		(0x6BU)
#define XPLMI_ERROR_PSM_RSRV14		(0x6CU)
#define XPLMI_ERROR_PSM_RSRV15		(0x6DU)
#define XPLMI_ERROR_PSM_RSRV16		(0x6EU)
#define XPLMI_ERROR_PSM_RSRV17		(0x6FU)
#define XPLMI_ERROR_PSM_RSRV18		(0x70U)
#define XPLMI_ERROR_PSM_RSRV19		(0x71U)
#define XPLMI_ERROR_LPD_XMPU		(0x72U)
#define XPLMI_ERROR_LPD_XPPU		(0x73U)
#define XPLMI_ERROR_FPD_XMPU		(0x74U)
#define XPLMI_ERROR_PSMERR2_MAX		(0x75U)

/* Software Errors */
/* Health Boot Monitoring errors */
#define XPLMI_ERROR_HB_MON_0		(0x80U)
#define XPLMI_ERROR_HB_MON_1		(0x81U)
#define XPLMI_ERROR_HB_MON_2		(0x82U)
#define XPLMI_ERROR_HB_MON_3		(0x83U)
#define XPLMI_ERROR_PLM_EXCEPTION	(0x84U)

/* XilSem CE & UE errors */
#define XPLMI_ERROR_CRAM_CE			(0x85U)
#define XPLMI_ERROR_CRAM_UE			(0x86U)
#define XPLMI_ERROR_NPI_UE			(0x87U)

/* STL UE errors */
#define XPLMI_ERROR_STL_UE			(0x88U)

#define XPLMI_ERROR_SW_ERR_MAX		(0x89U)

#define XPLMI_ERROR_PSMERR_MAX		XPLMI_ERROR_PSMERR2_MAX
#define XPLMI_ERROR_PMCERR_MAX		XPLMI_ERROR_PMCERR2_MAX

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ERROR_NODE_H */
