/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*
 * This is an automatically generated file from script.
 * Please do not modify this!
 */
#ifndef XPM_NODEID_H_
#define XPM_NODEID_H_

/**
 * @name Versal Power Nodes
 * @defgroup xilpmpwrnodes Power Nodes
 * @ingroup xilpmnodeids
 * @{
 */
#define PM_DEV_PMC_PROC				(0x18104001U)
#define PM_DEV_PSM_PROC				(0x18108002U)

#define PM_DEV_ACPU_0				(0x1810c003U)
#define PM_DEV_ACPU_1				(0x1810c004U)
#define PM_DEV_CLUSTER0_ACPU_0      (0x1810c003U)
#define PM_DEV_CLUSTER0_ACPU_1      (0x1810c004U)
#define PM_DEV_CLUSTER0_ACPU_2      (0x1810c005U)
#define PM_DEV_CLUSTER0_ACPU_3      (0x1810c006U)

#define PM_DEV_CLUSTER1_ACPU_0      (0x1810c007U)
#define PM_DEV_CLUSTER1_ACPU_1      (0x1810c008U)
#define PM_DEV_CLUSTER1_ACPU_2      (0x1810c009U)
#define PM_DEV_CLUSTER1_ACPU_3      (0x1810c00aU)

#define PM_DEV_CLUSTER2_ACPU_0      (0x1810c00bU)
#define PM_DEV_CLUSTER2_ACPU_1      (0x1810c00cU)
#define PM_DEV_CLUSTER2_ACPU_2      (0x1810c00dU)
#define PM_DEV_CLUSTER2_ACPU_3      (0x1810c00eU)

#define PM_DEV_CLUSTER3_ACPU_0      (0x1810c00fU)
#define PM_DEV_CLUSTER3_ACPU_1      (0x1810c010U)
#define PM_DEV_CLUSTER3_ACPU_2      (0x1810c011U)
#define PM_DEV_CLUSTER3_ACPU_3      (0x1810c012U)

#define PM_DEV_CLUSTER0_RPU0_0      (0x18110013U)
#define PM_DEV_CLUSTER0_RPU0_1      (0x18110014U)

#define PM_DEV_CLUSTER1_RPU0_0      (0x18110015U)
#define PM_DEV_CLUSTER1_RPU0_1      (0x18110016U)

#define PM_DEV_TCM_0_A				(0x1831800bU)
#define PM_DEV_TCM_0_B				(0x1831800cU)
#define PM_DEV_TCM_1_A				(0x1831800dU)
#define PM_DEV_TCM_1_B				(0x1831800eU)
#define PM_DEV_PLD_0				(0x18700000U)
#define PM_DEV_OSPI				(0x1822402aU)
#define PM_DEV_QSPI				(0x1822402bU)
#define PM_DEV_SDIO_0				(0x1822402eU)
#define PM_DEV_SDIO_1				(0x1822402fU)
#define PM_DEV_DDR_0				(0x18320010U)
#define PM_DEV_USB_0				(0x18224018U)
#define PM_DEV_DDRMC_0				(0x18520045U)
#define PM_DEV_DDRMC_1				(0x18520046U)
#define PM_DEV_DDRMC_2				(0x18520047U)
#define PM_DEV_DDRMC_3				(0x18520048U)

#define PM_SUBSYS_DEFAULT			(0x1c000000U)
#define PM_SUBSYS_PMC				(0x1c000001U)



/**
 * @cond INTERNAL
 * @defgroup xilpmisonodes Isolation Nodes
 * @ingroup xilpmnodeids
 * @{
 */
#define PM_ISO_FPD_PL_TEST			(0x20000000U)
#define PM_ISO_FPD_PL				(0x20000001U)
#define PM_ISO_FPD_SOC				(0x20000002U)
#define PM_ISO_LPD_CPM_DFX			(0x20000003U)
#define PM_ISO_LPD_CPM				(0x20000004U)
#define PM_ISO_LPD_PL_TEST			(0x20000005U)
#define PM_ISO_LPD_PL				(0x20000006U)
#define PM_ISO_LPD_SOC				(0x20000007U)
#define PM_ISO_PMC_LPD_DFX			(0x20000008U)
#define PM_ISO_PMC_LPD				(0x20000009U)
#define PM_ISO_PMC_PL_CFRAME			(0x2000000aU)
#define PM_ISO_PMC_PL_TEST			(0x2000000bU)
#define PM_ISO_PMC_PL				(0x2000000cU)
#define PM_ISO_PMC_SOC_NPI			(0x2000000dU)
#define PM_ISO_PMC_SOC				(0x2000000eU)
#define PM_ISO_PL_SOC				(0x2000000fU)
#define PM_ISO_VCCAUX_SOC			(0x20000010U)
#define PM_ISO_VCCRAM_SOC			(0x20000011U)
#define PM_ISO_VCCAUX_VCCRAM			(0x20000012U)
#define PM_ISO_PL_CPM_PCIEA0_ATTR		(0x20000013U)
#define PM_ISO_PL_CPM_PCIEA1_ATTR		(0x20000014U)
#define PM_ISO_PL_CPM_RST_CPI0			(0x20000015U)
#define PM_ISO_PL_CPM_RST_CPI1			(0x20000016U)
#define PM_ISO_GEM_TSU_CLK			(0x20000017U)
#define PM_ISO_GEM0_TXRX_CLK			(0x20000018U)
#define PM_ISO_GEM1_TXRX_CLK			(0x20000019U)
#define PM_ISO_CPM5_PL				(0x2000001aU)
#define PM_ISO_CPM5_PL_AXIMM			(0x2000001bU)
#define PM_ISO_CPM5_PL_CHI0			(0x2000001cU)
#define PM_ISO_CPM5_PL_CHI1			(0x2000001dU)
#define PM_ISO_CPM5_PL_TST			(0x2000001eU)
#define PM_ISO_CPM5_PL_PCIEA0_MPIO		(0x2000001fU)
#define PM_ISO_CPM5_PL_PCIEA1_MPIO		(0x20000020U)
#define PM_ISO_CPM5_RAM				(0x20000021U)
#define PM_ISO_LPD_CPM5				(0x20000022U)
#define PM_ISO_LPD_CPM5_DFX			(0x20000023U)
#define PM_ISO_XRAM_PL_AXI0			(0x20000024U)
#define PM_ISO_XRAM_PL_AXI1			(0x20000025U)
#define PM_ISO_XRAM_PL_AXI2			(0x20000026U)
#define PM_ISO_XRAM_PL_AXILITE			(0x20000027U)
#define PM_ISO_XRAM_PL_FABRIC			(0x20000028U)

#endif /* XPM_NODEID_H_ */
