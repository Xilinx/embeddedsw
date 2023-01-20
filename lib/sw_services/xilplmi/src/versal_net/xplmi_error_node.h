/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/xplmi_error_node.h
*
* This is the file which contains node IDs information for versal_net error
* events.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       dc   07/12/2022 Added XPLMI_ERROR_DEV_STATE_CHANGE
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*       kal  01/05/2023 Added XPLMI_ERROR_PCR_LOG_UPDATE
*       rama 01/19/2023 Added XilSem error IDs to support software event
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
	XPLMI_NODETYPE_EVENT_PMC_ERR3, /**< 0x2 */
	XPLMI_NODETYPE_EVENT_PSM_ERR1, /**< 0x3 */
	XPLMI_NODETYPE_EVENT_PSM_ERR2, /**< 0x4 */
	XPLMI_NODETYPE_EVENT_PSM_ERR3, /**< 0x5 */
	XPLMI_NODETYPE_EVENT_PSM_ERR4, /**< 0x6 */
	XPLMI_NODETYPE_EVENT_SW_ERR,   /**< 0x7 */
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
#define XPLMI_ERROR_PMC_RSRV0			(0x24U)
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
#define XPLMI_ERROR_PMX_WWDT		(0x34U)
#define XPLMI_ERROR_PMC_RSRV4		(0x35U)
#define XPLMI_ERROR_RTCALARM		(0x36U)
#define XPLMI_ERROR_NPLL			(0x37U)
#define XPLMI_ERROR_PPLL			(0x38U)
#define XPLMI_ERROR_CLKMON			(0x39U)
#define XPLMI_ERROR_PMC_RSRV5			(0x3AU)
#define XPLMI_ERROR_INT_PMX_CORR_ERR		(0x3BU)
#define XPLMI_ERROR_INT_PMX_UNCORR_ERR		(0x3CU)
#define XPLMI_ERROR_SSIT0			(0x3DU)
#define XPLMI_ERROR_SSIT1			(0x3EU)
#define XPLMI_ERROR_SSIT2			(0x3FU)
#define XPLMI_ERROR_PMCERR2_MAX		(0x40U)

/* PMC ERR3 errors */
#define XPLMI_ERROR_IOU_CR		(0x40U)
#define XPLMI_ERROR_IOU_NCR		(0x41U)
#define XPLMI_ERROR_DFX_UXPT_ACT	(0x42U)
#define XPLMI_ERROR_DICE_CDI_PAR	(0x43U)
#define XPLMI_ERROR_DEVIK_PRIV		(0x44U)
#define XPLMI_ERROR_NXTSW_CDI_PAR	(0x45U)
#define XPLMI_ERROR_DEVAK_PRIV		(0x46U)
#define XPLMI_ERROR_DME_PUB_X		(0x47U)
#define XPLMI_ERROR_DME_PUB_Y		(0x48U)
#define XPLMI_ERROR_DEVAK_PUB_X		(0x49U)
#define XPLMI_ERROR_DEVAK_PUB_Y		(0x4AU)
#define XPLMI_ERROR_DEVIK_PUB_X		(0x4BU)
#define XPLMI_ERROR_DEVIK_PUB_Y		(0x4CU)
#define XPLMI_ERROR_PCR_PAR		(0x4DU)
#define XPLMI_ERROR_PMCERR3_MAX		(0x4EU)

/* PSM ERR1 errors */
#define XPLMI_ERROR_PS_SW_CR		(0x60U)
#define XPLMI_ERROR_PS_SW_NCR		(0x61U)
#define XPLMI_ERROR_PSM_B_CR		(0x62U)
#define XPLMI_ERROR_PSM_B_NCR		(0x63U)
#define XPLMI_ERROR_MB_FATAL		(0x64U)
#define XPLMI_ERROR_PSM_CR		(0x65U)
#define XPLMI_ERROR_PSM_NCR		(0x66U)
#define XPLMI_ERROR_PSMX_CHK		(0x67U)
#define XPLMI_ERROR_APLL1_LOCK		(0x68U)
#define XPLMI_ERROR_APLL2_LOCK		(0x69U)
#define XPLMI_ERROR_RPLL_LOCK		(0x6AU)
#define XPLMI_ERROR_FLXPLL_LOCK		(0x6BU)
#define XPLMI_ERROR_INT_PSM_CR		(0x6CU)
#define XPLMI_ERROR_INT_PSM_NCR		(0x6DU)
#define XPLMI_ERROR_USB2		(0x6EU)
#define XPLMI_ERROR_LPX_UXPT_ACT	(0x6FU)
#define XPLMI_ERROR_PSM_RSRV0		(0x70U)
#define XPLMI_ERROR_INT_LPD_CR		(0x71U)
#define XPLMI_ERROR_INT_LPD_NCR		(0x72U)
#define XPLMI_ERROR_INT_OCM_CR		(0x73U)
#define XPLMI_ERROR_INT_OCM_NCR		(0x74U)
#define XPLMI_ERROR_INT_FPD_CR		(0x75U)
#define XPLMI_ERROR_INT_FPD_NCR		(0x76U)
#define XPLMI_ERROR_INT_IOU_CR		(0x77U)
#define XPLMI_ERROR_INT_IOU_NCR		(0x78U)
#define XPLMI_ERROR_RPUA_LOCKSTEP	(0x79U)
#define XPLMI_ERROR_RPUB_LOCKSTEP	(0x7AU)
#define XPLMI_ERROR_APU_GIC_AXI		(0x7BU)
#define XPLMI_ERROR_APU_GIC_ECC		(0x7CU)
#define XPLMI_ERROR_CPM_CR		(0x7DU)
#define XPLMI_ERROR_CPM_NCR		(0x7EU)
#define XPLMI_ERROR_CPI			(0x7FU)
#define XPLMI_ERROR_PSMERR1_MAX		(0x80U)

/* PSM ERR2 errors */
#define XPLMI_ERROR_FPD_WDT0		(0x80U)
#define XPLMI_ERROR_FPD_WDT1		(0x81U)
#define XPLMI_ERROR_FPD_WDT2		(0x82U)
#define XPLMI_ERROR_FPD_WDT3		(0x83U)
#define XPLMI_ERROR_MEM_SPLITTER0	(0x84U)
#define XPLMI_ERROR_AXI_PAR_SPLITTER0	(0x85U)
#define XPLMI_ERROR_MEM_SPLITTER1	(0x86U)
#define XPLMI_ERROR_AXI_PAR_SPLITTER1	(0x87U)
#define XPLMI_ERROR_MEM_SPLITTER2	(0x88U)
#define XPLMI_ERROR_AXI_PAR_SPLITTER2	(0x89U)
#define XPLMI_ERROR_MEM_SPLITTER3    	(0x8AU)
#define XPLMI_ERROR_AXI_PAR_SPLITTER3	(0x8BU)
#define XPLMI_ERROR_APU_CLUSTER0 	(0x8CU)
#define XPLMI_ERROR_APU_CLUSTER1	(0x8DU)
#define XPLMI_ERROR_APU_CLUSTER2	(0x8EU)
#define XPLMI_ERROR_APU_CLUSTER3	(0x8FU)
#define XPLMI_ERROR_LPD_WWDT0		(0x90U)
#define XPLMI_ERROR_LPD_WWDT1		(0x91U)
#define XPLMI_ERROR_ADMA_LOCKSTEP	(0x92U)
#define XPLMI_ERROR_IPI			(0x93U)
#define XPLMI_ERROR_OCM_BANK0_CR	(0x94U)
#define XPLMI_ERROR_OCM_BANK1_CR	(0x95U)
#define XPLMI_ERROR_OCM_BANK0_NCR	(0x96U)
#define XPLMI_ERROR_OCM_BANK1_NCR	(0x97U)
#define XPLMI_ERROR_LPXAFIFS_CR		(0x98U)
#define XPLMI_ERROR_LPXAFIFS_NCR	(0x99U)
#define XPLMI_ERROR_LPX_GLITCH_DETECT0	(0x9AU)
#define XPLMI_ERROR_LPX_GLITCH_DETECT1	(0x9BU)
#define XPLMI_ERROR_FWALL_WR_NOC_NMU	(0x9CU)
#define XPLMI_ERROR_FWALL_RD_NOC_NMU	(0x9DU)
#define XPLMI_ERROR_FWALL_NOC_NSU	(0x9EU)
#define XPLMI_ERROR_B18_R52_A0		(0x9FU)
#define XPLMI_ERROR_PSMERR2_MAX		(0xA0U)

/* PSM ERR3 errors */
#define XPLMI_ERROR_B18_R52_A1		(0xA0U)
#define XPLMI_ERROR_B18_R52_B0		(0xA1U)
#define XPLMI_ERROR_B18_R52_B1		(0xA2U)
#define XPLMI_ERROR_R52_A0_CR		(0xA3U)
#define XPLMI_ERROR_R52_A0_TFATAL	(0xA4U)
#define XPLMI_ERROR_R52_A0_TIMEOUT	(0xA5U)
#define XPLMI_ERROR_B24_B20_RPUA0	(0xA6U)
#define XPLMI_ERROR_B25_RPUA0		(0xA7U)
#define XPLMI_ERROR_R52_A1_CR		(0xA8U)
#define XPLMI_ERROR_R52_A1_TFATAL	(0xA9U)
#define XPLMI_ERROR_R52_A1_TIMEOUT    	(0xAAU)
#define XPLMI_ERROR_B24_B20_RPUA1	(0xABU)
#define XPLMI_ERROR_B25_RPUA1		(0xACU)
#define XPLMI_ERROR_R52_B0_CR		(0xADU)
#define XPLMI_ERROR_R52_B0_TFATAL	(0xAEU)
#define XPLMI_ERROR_R52_B0_TIMEOUT    	(0xAFU)
#define XPLMI_ERROR_B24_B20_RPUB0	(0xB0U)
#define XPLMI_ERROR_B25_RPUB0		(0xB1U)
#define XPLMI_ERROR_R52_B1_CR		(0xB2U)
#define XPLMI_ERROR_R52_B1_TFATAL	(0xB3U)
#define XPLMI_ERROR_R52_B1_TIMEOUT    	(0xB4U)
#define XPLMI_ERROR_B24_B20_RPUB1	(0xB5U)
#define XPLMI_ERROR_B25_RPUB1		(0xB6U)
#define XPLMI_ERROR_PSM_RSRV1		(0xB7U)
#define XPLMI_ERROR_PCIL_RPU		(0xB8U)
#define XPLMI_ERROR_FPXAFIFS_CR		(0xB9U)
#define XPLMI_ERROR_FPXAFIFS_NCR	(0xBAU)
#define XPLMI_ERROR_PSX_CMN_1		(0xBBU)
#define XPLMI_ERROR_PSX_CMN_2		(0xBCU)
#define XPLMI_ERROR_PSX_CMN_3		(0xBDU)
#define XPLMI_ERROR_PSX_CML		(0xBEU)
#define XPLMI_ERROR_FPD_INT_WRAP	(0xBFU)
#define XPLMI_ERROR_PSMERR3_MAX		(0xC0U)

/* PSM ERR4 errors */
#define XPLMI_ERROR_FPD_RST_MON		(0xC0U)
#define XPLMI_ERROR_LPD_RST_CLK_MON	(0xC1U)
#define XPLMI_ERROR_FATAL_AFI_FM	(0xC2U)
#define XPLMI_ERROR_NFATAL_AFI_FM_LPX	(0xC3U)
#define XPLMI_ERROR_NFATAL_AFI_FM0_FPX	(0xC4U)
#define XPLMI_ERROR_NFATAL_AFI_FM1_FPX	(0xC5U)
#define XPLMI_ERROR_NFATAL_AFI_FM2_FPX	(0xC6U)
#define XPLMI_ERROR_NFATAL_AFI_FM3_FPX	(0xC7U)
#define XPLMI_ERROR_RPU_CLUSTERA	(0xC8U)
#define XPLMI_ERROR_RPU_CLUSTERB	(0xC9U)
#define XPLMI_ERROR_PSMERR4_MAX		(0xCAU)

/* Software Errors */
/* Health Boot Monitoring errors */
#define XPLMI_ERROR_HB_MON_0		(0xE0U)
#define XPLMI_ERROR_HB_MON_1		(0xE1U)
#define XPLMI_ERROR_HB_MON_2		(0xE2U)
#define XPLMI_ERROR_HB_MON_3		(0xE3U)
#define XPLMI_ERROR_PLM_EXCEPTION	(0xE4U)
#define XPLMI_ERROR_DEV_STATE_CHANGE 	(0xE5U)
#define XPLMI_ERROR_PCR_LOG_UPDATE	(0xE6U)
/* XilSem CE & UE errors */
#define XPLMI_ERROR_CRAM_CE			(0xE7U)
#define XPLMI_ERROR_CRAM_UE			(0xE8U)
#define XPLMI_ERROR_NPI_UE			(0xE9U)
#define XPLMI_ERROR_SW_ERR_MAX		(0xEAU)

#define XPLMI_ERROR_PSMERR_MAX		XPLMI_ERROR_PSMERR4_MAX
#define XPLMI_ERROR_PMCERR_MAX		XPLMI_ERROR_PMCERR3_MAX

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
