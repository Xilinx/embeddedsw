/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_powerdomain.h"
#include "xpm_power_plat.h"
#include "xpm_core.h"

#define CHECK_BIT(reg, mask)	(((reg) & (mask)) == (mask))
#define RPU_TCMBOOT_MASK (0x00000010U)

#define NUM_CLUSTER 4U
static u8 ApuClusterState[NUM_CLUSTER] = {0U};

/* TODO: see if this data can be passed via topology */
static struct XPmFwPwrCtrl_t Acpu0_Core0PwrCtrl = {
	.Id = ACPU_0,
	.ResetCfgAddr = APU_CLUSTER0_RVBARADDR0L,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_APU0_CORE0_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_APU0_CORE0_PWR_CNTRL_REG0,
	.ClkCtrlAddr = PSXC_CRF_ACPU0_CLK_CTRL,
	.ClkPropTime = ACPU_CTRL_CLK_PROP_TIME,
	.RstAddr = PSXC_CRF_RST_APU0,
	.WarmRstMask = PSXC_CRF_RST_APU_CORE0_WARM_RST_MASK,
	.ClusterPcilAddr = APU_PCIL_CLUSTER_0_BASEADDR,
	.CorePcilAddr = APU_PCIL_CORE_0_BASEADDR,
	.ClusterId = CLUSTER_0,
	.PwrUpAckTimeout = {
		PWRUP_ACPU_CHN0_TO,
		PWRUP_ACPU_CHN1_TO,
		PWRUP_ACPU_CHN2_TO,
		PWRUP_ACPU_CHN3_TO },
	.PwrUpWaitTime = {
		PWRUP_ACPU_CHN0_WAIT_TM,
		PWRUP_ACPU_CHN1_WAIT_TM,
		PWRUP_ACPU_CHN2_WAIT_TM,
		PWRUP_ACPU_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = PWRDWN_ACPU_TO,
};

static struct XPmFwPwrCtrl_t Acpu0_Core1PwrCtrl = {
	.Id = ACPU_1,
	.ResetCfgAddr = APU_CLUSTER0_RVBARADDR1L,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_APU0_CORE1_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_APU0_CORE1_PWR_CNTRL_REG0,
	.ClkCtrlAddr = PSXC_CRF_ACPU0_CLK_CTRL,
	.ClkPropTime = ACPU_CTRL_CLK_PROP_TIME,
	.RstAddr = PSXC_CRF_RST_APU0,
	.WarmRstMask = PSXC_CRF_RST_APU_CORE1_WARM_RST_MASK,
	.ClusterPcilAddr = APU_PCIL_CLUSTER_0_BASEADDR,
	.CorePcilAddr = APU_PCIL_CORE_1_BASEADDR,
	.ClusterId = CLUSTER_0,
	.PwrUpAckTimeout = {
		PWRUP_ACPU_CHN0_TO,
		PWRUP_ACPU_CHN1_TO,
		PWRUP_ACPU_CHN2_TO,
		PWRUP_ACPU_CHN3_TO },
	.PwrUpWaitTime = {
		PWRUP_ACPU_CHN0_WAIT_TM,
		PWRUP_ACPU_CHN1_WAIT_TM,
		PWRUP_ACPU_CHN2_WAIT_TM,
		PWRUP_ACPU_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = PWRDWN_ACPU_TO,
};

static struct XPmFwPwrCtrl_t Acpu1_Core0PwrCtrl = {
	.Id = ACPU_2,
	.ResetCfgAddr = APU_CLUSTER1_RVBARADDR0L,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_APU1_CORE0_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_APU1_CORE0_PWR_CNTRL_REG0,
	.ClkCtrlAddr = PSXC_CRF_ACPU1_CLK_CTRL,
	.ClkPropTime = ACPU_CTRL_CLK_PROP_TIME,
	.RstAddr = PSXC_CRF_RST_APU1,
	.WarmRstMask = PSXC_CRF_RST_APU_CORE0_WARM_RST_MASK,
	.ClusterPcilAddr = APU_PCIL_CLUSTER_1_BASEADDR,
	.CorePcilAddr = APU_PCIL_CORE_4_BASEADDR,
	.ClusterId = CLUSTER_1,
	.PwrUpAckTimeout = {
		PWRUP_ACPU_CHN0_TO,
		PWRUP_ACPU_CHN1_TO,
		PWRUP_ACPU_CHN2_TO,
		PWRUP_ACPU_CHN3_TO },
	.PwrUpWaitTime = {
		PWRUP_ACPU_CHN0_WAIT_TM,
		PWRUP_ACPU_CHN1_WAIT_TM,
		PWRUP_ACPU_CHN2_WAIT_TM,
		PWRUP_ACPU_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = PWRDWN_ACPU_TO,
};

static struct XPmFwPwrCtrl_t Acpu1_Core1PwrCtrl = {
	.Id = ACPU_3,
	.ResetCfgAddr = APU_CLUSTER1_RVBARADDR1L,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_APU1_CORE1_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_APU1_CORE1_PWR_CNTRL_REG0,
	.ClkCtrlAddr = PSXC_CRF_ACPU1_CLK_CTRL,
	.ClkPropTime = ACPU_CTRL_CLK_PROP_TIME,
	.RstAddr = PSXC_CRF_RST_APU1,
	.WarmRstMask = PSXC_CRF_RST_APU_CORE1_WARM_RST_MASK,
	.ClusterPcilAddr = APU_PCIL_CLUSTER_1_BASEADDR,
	.CorePcilAddr = APU_PCIL_CORE_5_BASEADDR,
	.ClusterId = CLUSTER_1,
	.PwrUpAckTimeout = {
		PWRUP_ACPU_CHN0_TO,
		PWRUP_ACPU_CHN1_TO,
		PWRUP_ACPU_CHN2_TO,
		PWRUP_ACPU_CHN3_TO },
	.PwrUpWaitTime = {
		PWRUP_ACPU_CHN0_WAIT_TM,
		PWRUP_ACPU_CHN1_WAIT_TM,
		PWRUP_ACPU_CHN2_WAIT_TM,
		PWRUP_ACPU_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = PWRDWN_ACPU_TO,
};

static struct XPmFwPwrCtrl_t Acpu2_Core0PwrCtrl = {
	.Id = ACPU_4,
	.ResetCfgAddr = APU_CLUSTER2_RVBARADDR0L,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_APU2_CORE0_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_APU2_CORE0_PWR_CNTRL_REG0,
	.ClkCtrlAddr = PSXC_CRF_ACPU2_CLK_CTRL,
	.ClkPropTime = ACPU_CTRL_CLK_PROP_TIME,
	.RstAddr = PSXC_CRF_RST_APU2,
	.WarmRstMask = PSXC_CRF_RST_APU_CORE0_WARM_RST_MASK,
	.ClusterPcilAddr = APU_PCIL_CLUSTER_2_BASEADDR,
	.CorePcilAddr = APU_PCIL_CORE_8_BASEADDR,
	.ClusterId = CLUSTER_2,
	.PwrUpAckTimeout = {
		PWRUP_ACPU_CHN0_TO,
		PWRUP_ACPU_CHN1_TO,
		PWRUP_ACPU_CHN2_TO,
		PWRUP_ACPU_CHN3_TO },
	.PwrUpWaitTime = {
		PWRUP_ACPU_CHN0_WAIT_TM,
		PWRUP_ACPU_CHN1_WAIT_TM,
		PWRUP_ACPU_CHN2_WAIT_TM,
		PWRUP_ACPU_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = PWRDWN_ACPU_TO,
};

static struct XPmFwPwrCtrl_t Acpu2_Core1PwrCtrl = {
	.Id = ACPU_5,
	.ResetCfgAddr = APU_CLUSTER2_RVBARADDR1L,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_APU2_CORE1_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_APU2_CORE1_PWR_CNTRL_REG0,
	.ClkCtrlAddr = PSXC_CRF_ACPU2_CLK_CTRL,
	.ClkPropTime = ACPU_CTRL_CLK_PROP_TIME,
	.RstAddr = PSXC_CRF_RST_APU2,
	.WarmRstMask = PSXC_CRF_RST_APU_CORE1_WARM_RST_MASK,
	.ClusterPcilAddr = APU_PCIL_CLUSTER_2_BASEADDR,
	.CorePcilAddr = APU_PCIL_CORE_9_BASEADDR,
	.ClusterId = CLUSTER_2,
	.PwrUpAckTimeout = {
		PWRUP_ACPU_CHN0_TO,
		PWRUP_ACPU_CHN1_TO,
		PWRUP_ACPU_CHN2_TO,
		PWRUP_ACPU_CHN3_TO },
	.PwrUpWaitTime = {
		PWRUP_ACPU_CHN0_WAIT_TM,
		PWRUP_ACPU_CHN1_WAIT_TM,
		PWRUP_ACPU_CHN2_WAIT_TM,
		PWRUP_ACPU_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = PWRDWN_ACPU_TO,
};

static struct XPmFwPwrCtrl_t Acpu3_Core0PwrCtrl = {
	.Id = ACPU_6,
	.ResetCfgAddr = APU_CLUSTER3_RVBARADDR0L,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_APU3_CORE0_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_APU3_CORE0_PWR_CNTRL_REG0,
	.ClkCtrlAddr = PSXC_CRF_ACPU3_CLK_CTRL,
	.ClkPropTime = ACPU_CTRL_CLK_PROP_TIME,
	.RstAddr = PSXC_CRF_RST_APU3,
	.WarmRstMask = PSXC_CRF_RST_APU_CORE0_WARM_RST_MASK,
	.ClusterPcilAddr = APU_PCIL_CLUSTER_3_BASEADDR,
	.CorePcilAddr = APU_PCIL_CORE_12_BASEADDR,
	.ClusterId = CLUSTER_3,
	.PwrUpAckTimeout = {
		PWRUP_ACPU_CHN0_TO,
		PWRUP_ACPU_CHN1_TO,
		PWRUP_ACPU_CHN2_TO,
		PWRUP_ACPU_CHN3_TO },
	.PwrUpWaitTime = {
		PWRUP_ACPU_CHN0_WAIT_TM,
		PWRUP_ACPU_CHN1_WAIT_TM,
		PWRUP_ACPU_CHN2_WAIT_TM,
		PWRUP_ACPU_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = PWRDWN_ACPU_TO,
};

static struct XPmFwPwrCtrl_t Acpu3_Core1PwrCtrl = {
	.Id = ACPU_7,
	.ResetCfgAddr = APU_CLUSTER3_RVBARADDR1L,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_APU3_CORE1_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_APU3_CORE1_PWR_CNTRL_REG0,
	.ClkCtrlAddr = PSXC_CRF_ACPU3_CLK_CTRL,
	.ClkPropTime = ACPU_CTRL_CLK_PROP_TIME,
	.RstAddr = PSXC_CRF_RST_APU3,
	.WarmRstMask = PSXC_CRF_RST_APU_CORE1_WARM_RST_MASK,
	.ClusterPcilAddr = APU_PCIL_CLUSTER_3_BASEADDR,
	.CorePcilAddr = APU_PCIL_CORE_13_BASEADDR,
	.ClusterId = CLUSTER_3,
	.PwrUpAckTimeout = {
		PWRUP_ACPU_CHN0_TO,
		PWRUP_ACPU_CHN1_TO,
		PWRUP_ACPU_CHN2_TO,
		PWRUP_ACPU_CHN3_TO },
	.PwrUpWaitTime = {
		PWRUP_ACPU_CHN0_WAIT_TM,
		PWRUP_ACPU_CHN1_WAIT_TM,
		PWRUP_ACPU_CHN2_WAIT_TM,
		PWRUP_ACPU_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = PWRDWN_ACPU_TO,
};

static struct XPmFwPwrCtrl_t Rpu0_Core0PwrCtrl = {
	.Id = RPU0_0,
	.ResetCfgAddr = PSX_RPU_CLUSTER_A0_CORE0_CFG0,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_RPUA_CORE0_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_RPU0_CORE0_PWR_CNTRL_REG0,
	.RstCtrlAddr = PSX_CRL_RST_RPU_A,
	.RstCtrlMask = PSXC_CRL_RST_RPU_A_CORE0_RESET_MASK,
	.CorePcilIdsAddr = PSXC_LPX_SLCR_RPU_PCIL_A0_IDS,
	.CorePcilIsrAddr = LPD_SLCR_RPU_PCIL_A0_ISR,
	.CorePcilIenAddr = PSXC_LPX_SLCR_RPU_PCIL_A0_IEN,
	.CorePcilPsAddr = LPD_SLCR_RPU_PCIL_A0_PS,
	.CorePcilPrAddr = LPD_SLCR_RPU_PCIL_A0_PR,
	.CorePcilPaAddr = LPD_SLCR_RPU_PCIL_A0_PA,
	.CorePcilPwrdwnAddr = PSXC_LPX_SLCR_RPU_PCIL_A0_PWRDWN,
	.CorePactiveMask = LPD_SLCR_RPU_PCIL_PA_PACTIVE_MASK,
	.WakeupIrqMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUA_CORE0_MASK,
	.CacheCntrlMask = PSXC_LPX_SLCR_RPU_CACHE_CNTRL_A0_MASK,
	.ClusterId = CLUSTER_0,
	.VectTableAddr = PSX_RPU_CLUSTER_A0_CORE_0_VECTABLE,
};

static struct XPmFwPwrCtrl_t Rpu0_Core1PwrCtrl = {
	.Id = RPU0_1,
	.ResetCfgAddr = PSX_RPU_CLUSTER_A1_CORE1_CFG0,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_RPUA_CORE1_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_RPU0_CORE1_PWR_CNTRL_REG0,
	.RstCtrlAddr = PSX_CRL_RST_RPU_A,
	.RstCtrlMask = PSXC_CRL_RST_RPU_A_CORE1_RESET_MASK,
	.CorePcilIdsAddr = PSXC_LPX_SLCR_RPU_PCIL_A1_IDS,
	.CorePcilIsrAddr = LPD_SLCR_RPU_PCIL_A1_ISR,
	.CorePcilIenAddr = PSXC_LPX_SLCR_RPU_PCIL_A1_IEN,
	.CorePcilPsAddr = LPD_SLCR_RPU_PCIL_A1_PS,
	.CorePcilPrAddr = LPD_SLCR_RPU_PCIL_A1_PR,
	.CorePcilPaAddr = LPD_SLCR_RPU_PCIL_A1_PA,
	.CorePcilPwrdwnAddr = PSXC_LPX_SLCR_RPU_PCIL_A1_PWRDWN,
	.CorePactiveMask = LPD_SLCR_RPU_PCIL_PA_PACTIVE_MASK,
	.WakeupIrqMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUA_CORE1_MASK,
	.CacheCntrlMask = PSXC_LPX_SLCR_RPU_CACHE_CNTRL_A1_MASK,
	.ClusterId = CLUSTER_0,
	.VectTableAddr = PSX_RPU_CLUSTER_A1_CORE_1_VECTABLE,
};

static struct XPmFwPwrCtrl_t Rpu1_Core0PwrCtrl = {
	.Id = RPU1_0,
	.ResetCfgAddr = PSX_RPU_CLUSTER_B0_CORE0_CFG0,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_RPUB_CORE0_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_RPU1_CORE0_PWR_CNTRL_REG0,
	.RstCtrlAddr = PSX_CRL_RST_RPU_B,
	.RstCtrlMask = PSXC_CRL_RST_RPU_B_CORE0_RESET_MASK,
	.CorePcilIdsAddr = PSXC_LPX_SLCR_RPU_PCIL_B0_IDS,
	.CorePcilIsrAddr = LPD_SLCR_RPU_PCIL_B0_ISR,
	.CorePcilIenAddr = PSXC_LPX_SLCR_RPU_PCIL_B0_IEN,
	.CorePcilPsAddr = LPD_SLCR_RPU_PCIL_B0_PS,
	.CorePcilPrAddr = LPD_SLCR_RPU_PCIL_B0_PR,
	.CorePcilPaAddr = LPD_SLCR_RPU_PCIL_B0_PA,
	.CorePcilPwrdwnAddr = PSXC_LPX_SLCR_RPU_PCIL_B0_PWRDWN,
	.CorePactiveMask = LPD_SLCR_RPU_PCIL_PA_PACTIVE_MASK,
	.WakeupIrqMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUB_CORE0_MASK,
	.CacheCntrlMask = PSXC_LPX_SLCR_RPU_CACHE_CNTRL_B0_MASK,
	.ClusterId = CLUSTER_1,
	.VectTableAddr = PSX_RPU_CLUSTER_B0_CORE_0_VECTABLE,
};

static struct XPmFwPwrCtrl_t Rpu1_Core1PwrCtrl = {
	.Id = RPU1_1,
	.ResetCfgAddr = PSX_RPU_CLUSTER_B1_CORE1_CFG0,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_RPUB_CORE1_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_RPU1_CORE1_PWR_CNTRL_REG0,
	.RstCtrlAddr = PSX_CRL_RST_RPU_B,
	.RstCtrlMask = PSXC_CRL_RST_RPU_B_CORE1_RESET_MASK,
	.CorePcilIdsAddr = PSXC_LPX_SLCR_RPU_PCIL_B1_IDS,
	.CorePcilIsrAddr = LPD_SLCR_RPU_PCIL_B1_ISR,
	.CorePcilIenAddr = PSXC_LPX_SLCR_RPU_PCIL_B1_IEN,
	.CorePcilPsAddr = LPD_SLCR_RPU_PCIL_B1_PS,
	.CorePcilPrAddr = LPD_SLCR_RPU_PCIL_B1_PR,
	.CorePcilPaAddr = LPD_SLCR_RPU_PCIL_B1_PA,
	.CorePcilPwrdwnAddr = PSXC_LPX_SLCR_RPU_PCIL_B1_PWRDWN,
	.CorePactiveMask = LPD_SLCR_RPU_PCIL_PA_PACTIVE_MASK,
	.WakeupIrqMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUB_CORE1_MASK,
	.CacheCntrlMask = PSXC_LPX_SLCR_RPU_CACHE_CNTRL_B1_MASK,
	.ClusterId = CLUSTER_1,
	.VectTableAddr = PSX_RPU_CLUSTER_B1_CORE_1_VECTABLE,
};

static struct XPmFwPwrCtrl_t Rpu2_Core0PwrCtrl = {
	.Id = RPU2_0,
	.ResetCfgAddr = PSX_RPU_CLUSTER_C0_CORE0_CFG0,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_RPUC_CORE0_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_RPU2_CORE0_PWR_CNTRL_REG0,
	.RstCtrlAddr = PSX_CRL_RST_RPU_C,
	.RstCtrlMask = PSXC_CRL_RST_RPU_C_CORE0_RESET_MASK,
	.CorePcilIdsAddr = PSXC_LPX_SLCR_RPU_PCIL_C0_IDS,
	.CorePcilIsrAddr = LPD_SLCR_RPU_PCIL_C0_ISR,
	.CorePcilIenAddr = PSXC_LPX_SLCR_RPU_PCIL_C0_IEN,
	.CorePcilPsAddr = LPD_SLCR_RPU_PCIL_C0_PS,
	.CorePcilPrAddr = LPD_SLCR_RPU_PCIL_C0_PR,
	.CorePcilPaAddr = LPD_SLCR_RPU_PCIL_C0_PA,
	.CorePcilPwrdwnAddr = PSXC_LPX_SLCR_RPU_PCIL_C0_PWRDWN,
	.CorePactiveMask = LPD_SLCR_RPU_PCIL_PA_PACTIVE_MASK,
	.WakeupIrqMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUC_CORE0_MASK,
	.CacheCntrlMask = PSXC_LPX_SLCR_RPU_CACHE_CNTRL_C0_MASK,
	.ClusterId = CLUSTER_2,
	.VectTableAddr = PSX_RPU_CLUSTER_C0_CORE_0_VECTABLE,
};

static struct XPmFwPwrCtrl_t Rpu2_Core1PwrCtrl = {
	.Id = RPU2_1,
	.ResetCfgAddr = PSX_RPU_CLUSTER_C1_CORE1_CFG0,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_RPUC_CORE1_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_RPU2_CORE1_PWR_CNTRL_REG0,
	.RstCtrlAddr = PSX_CRL_RST_RPU_C,
	.RstCtrlMask = PSXC_CRL_RST_RPU_C_CORE1_RESET_MASK,
	.CorePcilIdsAddr = PSXC_LPX_SLCR_RPU_PCIL_C1_IDS,
	.CorePcilIsrAddr = LPD_SLCR_RPU_PCIL_C1_ISR,
	.CorePcilIenAddr = PSXC_LPX_SLCR_RPU_PCIL_C1_IEN,
	.CorePcilPsAddr = LPD_SLCR_RPU_PCIL_C1_PS,
	.CorePcilPrAddr = LPD_SLCR_RPU_PCIL_C1_PR,
	.CorePcilPaAddr = LPD_SLCR_RPU_PCIL_C1_PA,
	.CorePcilPwrdwnAddr = PSXC_LPX_SLCR_RPU_PCIL_C1_PWRDWN,
	.CorePactiveMask = LPD_SLCR_RPU_PCIL_PA_PACTIVE_MASK,
	.WakeupIrqMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUC_CORE1_MASK,
	.CacheCntrlMask = PSXC_LPX_SLCR_RPU_CACHE_CNTRL_C1_MASK,
	.ClusterId = CLUSTER_2,
	.VectTableAddr = PSX_RPU_CLUSTER_C1_CORE_1_VECTABLE,
};

static struct XPmFwPwrCtrl_t Rpu3_Core0PwrCtrl = {
	.Id = RPU3_0,
	.ResetCfgAddr = PSX_RPU_CLUSTER_D0_CORE0_CFG0,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_RPUD_CORE0_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_RPU3_CORE0_PWR_CNTRL_REG0,
	.RstCtrlAddr = PSX_CRL_RST_RPU_D,
	.RstCtrlMask = PSXC_CRL_RST_RPU_D_CORE0_RESET_MASK,
	.CorePcilIdsAddr = PSXC_LPX_SLCR_RPU_PCIL_D0_IDS,
	.CorePcilIsrAddr = LPD_SLCR_RPU_PCIL_D0_ISR,
	.CorePcilIenAddr = PSXC_LPX_SLCR_RPU_PCIL_D0_IEN,
	.CorePcilPsAddr = LPD_SLCR_RPU_PCIL_D0_PS,
	.CorePcilPrAddr = LPD_SLCR_RPU_PCIL_D0_PR,
	.CorePcilPaAddr = LPD_SLCR_RPU_PCIL_D0_PA,
	.CorePcilPwrdwnAddr = PSXC_LPX_SLCR_RPU_PCIL_D0_PWRDWN,
	.CorePactiveMask = LPD_SLCR_RPU_PCIL_PA_PACTIVE_MASK,
	.WakeupIrqMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUD_CORE0_MASK,
	.CacheCntrlMask = PSXC_LPX_SLCR_RPU_CACHE_CNTRL_D0_MASK,
	.ClusterId = CLUSTER_3,
	.VectTableAddr = PSX_RPU_CLUSTER_D0_CORE_0_VECTABLE,
};

static struct XPmFwPwrCtrl_t Rpu3_Core1PwrCtrl = {
	.Id = RPU3_1,
	.ResetCfgAddr = PSX_RPU_CLUSTER_D1_CORE1_CFG0,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_RPUD_CORE1_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_RPU3_CORE1_PWR_CNTRL_REG0,
	.RstCtrlAddr = PSX_CRL_RST_RPU_D,
	.RstCtrlMask = PSXC_CRL_RST_RPU_D_CORE1_RESET_MASK,
	.CorePcilIdsAddr = PSXC_LPX_SLCR_RPU_PCIL_D1_IDS,
	.CorePcilIsrAddr = LPD_SLCR_RPU_PCIL_D1_ISR,
	.CorePcilIenAddr = PSXC_LPX_SLCR_RPU_PCIL_D1_IEN,
	.CorePcilPsAddr = LPD_SLCR_RPU_PCIL_D1_PS,
	.CorePcilPrAddr = LPD_SLCR_RPU_PCIL_D1_PR,
	.CorePcilPaAddr = LPD_SLCR_RPU_PCIL_D1_PA,
	.CorePcilPwrdwnAddr = PSXC_LPX_SLCR_RPU_PCIL_D1_PWRDWN,
	.CorePactiveMask = LPD_SLCR_RPU_PCIL_PA_PACTIVE_MASK,
	.WakeupIrqMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUD_CORE1_MASK,
	.CacheCntrlMask = PSXC_LPX_SLCR_RPU_CACHE_CNTRL_D1_MASK,
	.ClusterId = CLUSTER_3,
	.VectTableAddr = PSX_RPU_CLUSTER_D1_CORE_1_VECTABLE,
};

static struct XPmFwPwrCtrl_t Rpu4_Core0PwrCtrl = {
	.Id = RPU4_0,
	.ResetCfgAddr = PSX_RPU_CLUSTER_E0_CORE0_CFG0,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_RPUE_CORE0_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_RPU4_CORE0_PWR_CNTRL_REG0,
	.RstCtrlAddr = PSX_CRL_RST_RPU_E,
	.RstCtrlMask = PSXC_CRL_RST_RPU_E_CORE0_RESET_MASK,
	.CorePcilIdsAddr = PSXC_LPX_SLCR_RPU_PCIL_E0_IDS,
	.CorePcilIsrAddr = LPD_SLCR_RPU_PCIL_E0_ISR,
	.CorePcilIenAddr = PSXC_LPX_SLCR_RPU_PCIL_E0_IEN,
	.CorePcilPsAddr = LPD_SLCR_RPU_PCIL_E0_PS,
	.CorePcilPrAddr = LPD_SLCR_RPU_PCIL_E0_PR,
	.CorePcilPaAddr = LPD_SLCR_RPU_PCIL_E0_PA,
	.CorePcilPwrdwnAddr = PSXC_LPX_SLCR_RPU_PCIL_E0_PWRDWN,
	.CorePactiveMask = LPD_SLCR_RPU_PCIL_PA_PACTIVE_MASK,
	.WakeupIrqMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUE_CORE0_MASK,
	.CacheCntrlMask = PSXC_LPX_SLCR_RPU_CACHE_CNTRL_E0_MASK,
	.ClusterId = CLUSTER_4,
	.VectTableAddr = PSX_RPU_CLUSTER_E0_CORE_0_VECTABLE,
};

static struct XPmFwPwrCtrl_t Rpu4_Core1PwrCtrl = {
	.Id = RPU4_1,
	.ResetCfgAddr = PSX_RPU_CLUSTER_E1_CORE1_CFG0,
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE0_RPUE_CORE1_MASK,
	.PwrCtrlAddr = PSXC_LPX_SLCR_RPU4_CORE1_PWR_CNTRL_REG0,
	.RstCtrlAddr = PSX_CRL_RST_RPU_E,
	.RstCtrlMask = PSXC_CRL_RST_RPU_E_CORE1_RESET_MASK,
	.CorePcilIdsAddr = PSXC_LPX_SLCR_RPU_PCIL_E1_IDS,
	.CorePcilIsrAddr = LPD_SLCR_RPU_PCIL_E1_ISR,
	.CorePcilIenAddr = PSXC_LPX_SLCR_RPU_PCIL_E1_IEN,
	.CorePcilPsAddr = LPD_SLCR_RPU_PCIL_E1_PS,
	.CorePcilPrAddr = LPD_SLCR_RPU_PCIL_E1_PR,
	.CorePcilPaAddr = LPD_SLCR_RPU_PCIL_E1_PA,
	.CorePcilPwrdwnAddr = PSXC_LPX_SLCR_RPU_PCIL_E1_PWRDWN,
	.CorePactiveMask = LPD_SLCR_RPU_PCIL_PA_PACTIVE_MASK,
	.WakeupIrqMask = PSXC_LPX_SLCR_WAKEUP1_IRQ_RPUE_CORE1_MASK,
	.CacheCntrlMask = PSXC_LPX_SLCR_RPU_CACHE_CNTRL_E1_MASK,
	.ClusterId = CLUSTER_4,
	.VectTableAddr = PSX_RPU_CLUSTER_E1_CORE_1_VECTABLE,
};

static struct XPmFwMemPwrCtrl_t Ocm_B0_I0_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B0_I0_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B0_I0_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B0_I0_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B0_I0_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND0_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND0_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B0_I0_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B0_I1_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B0_I1_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B0_I1_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B0_I1_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B0_I1_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND1_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND1_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B0_I1_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B0_I2_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B0_I2_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B0_I2_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B0_I2_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B0_I2_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND2_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND2_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B0_I2_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B0_I3_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B0_I3_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B0_I3_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B0_I3_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B0_I3_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND3_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND3_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B0_I3_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B1_I0_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B1_I0_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B1_I0_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B1_I0_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B1_I0_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND4_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND4_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B1_I0_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B1_I1_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B1_I1_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B1_I1_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B1_I1_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B1_I1_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND5_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND5_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B1_I1_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B1_I2_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B1_I2_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B1_I2_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B1_I2_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B1_I2_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND6_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND6_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B1_I2_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B1_I3_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B1_I3_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B1_I3_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B1_I3_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B1_I3_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND7_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND7_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B1_I3_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B2_I0_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B2_I0_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B2_I0_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B2_I0_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B2_I0_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND8_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND8_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B2_I0_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B2_I1_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B2_I1_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B2_I1_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B2_I1_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B2_I1_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND9_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND9_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B2_I1_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B2_I2_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B2_I2_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B2_I2_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B2_I2_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B2_I2_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND10_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND10_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B2_I2_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B2_I3_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B2_I3_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B2_I3_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B2_I3_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B2_I3_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND11_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND11_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B2_I3_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B3_I0_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B3_I0_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B3_I0_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B3_I0_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B3_I0_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND12_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND12_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B3_I0_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B3_I1_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B3_I1_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B3_I1_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B3_I1_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B3_I1_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND13_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND13_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B3_I1_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B3_I2_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B3_I2_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B3_I2_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B3_I2_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B3_I2_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND14_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND14_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B3_I2_MASK,
};

static struct XPmFwMemPwrCtrl_t Ocm_B3_I3_PwrCtrl = {
	.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_OCM_B3_I3_MASK,
	.ChipEnMask = PSXC_LPX_SLCR_OCM_CE_CNTRL_B3_I3_MASK,
	.PwrCtrlMask = PSXC_LPX_SLCR_OCM_PWR_CNTRL_B3_I3_MASK,
	.PwrStatusMask = PSXC_LPX_SLCR_OCM_PWR_STATUS_B3_I3_MASK,
	.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP2_STATUS_OCM_ISLAND15_MASK,
	.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS_OCM_ISLAND15_RET_MASK,
	.RetCtrlMask = PSXC_LPX_SLCR_OCM_RET_CNTRL_B3_I3_MASK,
};

static struct XPmTcmPwrCtrl_t TcmA0PwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_RPU_TCM_A0_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL_TCMA0_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL_TCMA0_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS_TCMA0_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_TCM0A_MASK,
		.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS_TCM0A_RET_MASK,
	},

	.Id = TCM_A_0,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPmTcmPwrCtrl_t TcmA1PwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_RPU_TCM_A1_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL_TCMA1_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL_TCMA1_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS_TCMA1_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_TCM1A_MASK,
		.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS_TCM1A_RET_MASK,
	},

	.Id = TCM_A_1,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPmTcmPwrCtrl_t TcmB0PwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_RPU_TCM_B0_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL_TCMB0_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL_TCMB0_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS_TCMB0_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_TCM0B_MASK,
		.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS_TCM0B_RET_MASK,
	},

	.Id = TCM_B_0,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPmTcmPwrCtrl_t TcmB1PwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_RPU_TCM_B1_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL_TCMB1_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL_TCMB1_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS_TCMB1_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_TCM1B_MASK,
		.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS_TCM1B_RET_MASK,
	},

	.Id = TCM_B_1,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPmTcmPwrCtrl_t TcmC0PwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_RPU_TCM_C0_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL_TCMC0_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL_TCMC0_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS_TCMC0_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_TCM0C_MASK,
		.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS_TCM0C_RET_MASK,
	},

	.Id = TCM_C_0,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPmTcmPwrCtrl_t TcmC1PwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_RPU_TCM_C1_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL_TCMC1_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL_TCMC1_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS_TCMC1_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_TCM1C_MASK,
		.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS_TCM1C_RET_MASK,
	},

	.Id = TCM_C_1,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPmTcmPwrCtrl_t TcmD0PwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_RPU_TCM_D0_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL_TCMD0_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL_TCMD0_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS_TCMD0_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_TCM0D_MASK,
		.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS_TCM0D_RET_MASK,
	},

	.Id = TCM_D_0,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPmTcmPwrCtrl_t TcmD1PwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_RPU_TCM_D1_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL_TCMD1_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL_TCMD1_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS_TCMD1_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_TCM1D_MASK,
		.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS_TCM1D_RET_MASK,
	},

	.Id = TCM_D_1,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPmTcmPwrCtrl_t TcmE0PwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_RPU_TCM_E0_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL_TCME0_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL_TCME0_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS_TCME0_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_TCM0E_MASK,
		.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS_TCM0E_RET_MASK,
	},

	.Id = TCM_E_0,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPmTcmPwrCtrl_t TcmE1PwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE1_RPU_TCM_E1_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL_TCME1_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL_TCME1_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS_TCME1_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_TCM1E_MASK,
		.RetMask = PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS_TCM1E_RET_MASK,
	},

	.Id = TCM_E_1,
	.PowerState = STATE_POWER_DEFAULT,
};


static struct XPmFwGemPwrCtrl_t Gem0PwrCtrl = {
	.GemMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE2_GEM0_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_GEM_CE_CNTRL_GEM0_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_GEM_PWR_CNTRL_GEM0_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_GEM_PWR_STATUS_GEM0_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_GEM0_MASK,
	},
	.ClkCtrlAddr = PSXC_CRL_GEM0_REF_CTRL,
	.RstCtrlAddr = PSXC_CRL_RST_GEM0,
	.RstCtrlMask = PSXC_CRL_RST_GEM0_RESET_MASK,
	.PwrStateAckTimeout = GEM0_PWR_STATE_ACK_TIMEOUT,
	.PwrUpWaitTime = GEM0_PWR_UP_WAIT_TIME,
};

static struct XPmFwGemPwrCtrl_t Gem1PwrCtrl = {
	.GemMemPwrCtrl = {
		.PwrStateMask = PMXC_GLOBAL_PMC_MSTR_PWR_STATE2_GEM1_MASK,
		.ChipEnMask = PSXC_LPX_SLCR_GEM_CE_CNTRL_GEM1_MASK,
		.PwrCtrlMask = PSXC_LPX_SLCR_GEM_PWR_CNTRL_GEM1_MASK,
		.PwrStatusMask = PSXC_LPX_SLCR_GEM_PWR_STATUS_GEM1_MASK,
		.GlobPwrStatusMask = PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_GEM1_MASK,
	},
	.ClkCtrlAddr = PSXC_CRL_GEM1_REF_CTRL,
	.RstCtrlAddr = PSXC_CRL_RST_GEM1,
	.RstCtrlMask = PSXC_CRL_RST_GEM1_RESET_MASK,
	.PwrStateAckTimeout = GEM1_PWR_STATE_ACK_TIMEOUT,
	.PwrUpWaitTime = GEM1_PWR_UP_WAIT_TIME,
};

static XStatus XPmPower_IslandPwrUp(struct XPmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 Index;
	u32 Bit = PSXC_LPX_SLCR_PWR_CTRL_STS_PRDY_SHIFT;

	/* Power up island */
	for (Index = 0; Index < PSXC_LPX_SLCR_PWR_CTRL_STS_PRDY_WIDTH; Index++) {
		/* Enable this power stage */
		XPm_RMW32(Args->PwrCtrlAddr, ((u32)1U << Bit), ((u32)1U << Bit));

		/* Poll the power stage status */
		Status = XPm_PollForMask(Args->PwrCtrlAddr + PSXC_LPX_SLCR_APU_CORE_PWR_CNTRL_STS_OFFSET, ((u32)1U << Bit), Args->PwrUpAckTimeout[Index]);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/* Wait for power to ramp up */
		XPm_Wait(Args->PwrUpWaitTime[Index]);

		Bit++;
	}

done:
	return Status;
}

static XStatus XPmPower_ACpuPwrUp(struct XPmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;

	if (A78_CLUSTER_CONFIGURED != ApuClusterState[Args->ClusterId]) {
		/* APU PSTATE, PREQ configuration */
		XPm_Out32(Args->ClusterPcilAddr + APU_PCIL_CLUSTER_PSTATE_OFFSET, APU_PCIL_CLUSTER_PSTATE_VAL);
		XPm_Out32(Args->ClusterPcilAddr + APU_PCIL_CLUSTER_PREQ_OFFSET, APU_PCIL_CLUSTER_PREQ_MASK);

		/* ACPU clock config */
		XPm_RMW32(Args->ClkCtrlAddr, PSXC_CRF_ACPU_CTRL_CLKACT_MASK, PSXC_CRF_ACPU_CTRL_CLKACT_MASK);

		/* Allow the clock to propagate */
		XPm_Wait(Args->ClkPropTime);

		/* APU cluster release cold & warm reset */
		XPm_RMW32(Args->RstAddr, ACPU_CLUSTER_COLD_WARM_RST_MASK, 0U);

		Status = XPm_PollForMask(Args->ClusterPcilAddr + APU_PCIL_CLUSTER_PACTIVE_OFFSET,
					 APU_PCIL_CLUSTER_PACCEPT_MASK, ACPU_PACCEPT_TIMEOUT);
		if (Status != XST_SUCCESS) {
			PmErr("A78 Cluster PACCEPT timeout..\n");
			goto done;
		}
		/* Clear PREQ bit */
		XPm_Out32(Args->ClusterPcilAddr + APU_PCIL_CLUSTER_PREQ_OFFSET, 0U);
		/* Clear power down and wake interrupt status */
		XPm_Out32(Args->ClusterPcilAddr + APU_PCIL_CLUSTER_ISR_POWER_OFFSET,
			       APU_PCIL_CLUSTER_PREQ_MASK);
		XPm_Out32(Args->ClusterPcilAddr + APU_PCIL_CLUSTER_ISR_WAKE_OFFSET,
			       APU_PCIL_CLUSTER_PREQ_MASK);

		ApuClusterState[Args->ClusterId] = A78_CLUSTER_CONFIGURED;
	}

	/* TBD: ignore below 2 steps if it is powering up from emulated
	 * pwrdwn or debug recovery pwrdwn*/
	/*Enables Power to the Core*/
	Status = XPmPower_IslandPwrUp(Args);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*Removes Isolation to the APU*/
	XPm_RMW32(Args->PwrCtrlAddr, PSXC_LPX_SLCR_APU_CORE_PWR_CNTRL_STS_ISOLATION_MASK,
		     ~PSXC_LPX_SLCR_APU_CORE_PWR_CNTRL_STS_ISOLATION_MASK);

	XPm_Out32(Args->CorePcilAddr + APU_PCIL_CORE_PSTATE_OFFSET, APU_PCIL_CORE_PSTATE_VAL);
	XPm_Out32(Args->CorePcilAddr + APU_PCIL_CORE_PREQ_OFFSET, APU_PCIL_CORE_PREQ_MASK);

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPmPower_ACpuDirectPwrUp(struct XPmFwPwrCtrl_t *Args, u64 ResumeAddr)
{
	XStatus Status = XST_FAILURE;
	u32 LowAddress, HighAddress;

	Status = XPmPower_ACpuPwrUp(Args);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Set start address */
	LowAddress = (u32)(ResumeAddr & PSX_APU_CLUSTER_RVBARADDR0L_MASK);
	/* TODO: HighAddress only uses bits 15:0, check if this needs to be masked as well */
	HighAddress = (u32)(ResumeAddr >> 32ULL);
	XPm_Out32(Args->ResetCfgAddr, LowAddress);
	XPm_Out32(Args->ResetCfgAddr + 0x4, HighAddress);

	/* APU core release warm reset */
	XPm_RMW32(Args->RstAddr, Args->WarmRstMask, ~Args->WarmRstMask);
	Status = XPm_PollForMask(Args->CorePcilAddr + APU_PCIL_CORE_PACTIVE_OFFSET, APU_PCIL_CORE_PACCEPT_MASK, ACPU_PACCEPT_TIMEOUT);
	if (Status != XST_SUCCESS) {
		PmErr("A78 Cluster PACCEPT timeout..\n");
		goto done;
	}

	/* Clear power down and wake interrupt status */
	XPm_Out32(Args->CorePcilAddr + APU_PCIL_CORE_ISR_POWER_OFFSET, APU_PCIL_CORE_PREQ_MASK);
	XPm_Out32(Args->CorePcilAddr + APU_PCIL_CORE_ISR_WAKE_OFFSET, APU_PCIL_CORE_PREQ_MASK);

	/* Clear PREQ bit */
	XPm_Out32(Args->CorePcilAddr + APU_PCIL_CORE_PREQ_OFFSET, 0U);

	/* Disable and clear ACPUx direct wake-up interrupt request */
	XPm_Out32(PSXC_LPX_SLCR_WAKEUP0_IRQ_STATUS, Args->PwrStateMask);

	/*
	* Unmask interrupt for all Power-up Requests and Reset Requests that
	* are triggered but have their interrupt masked.
	*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP0_INT_EN, XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN0_STATUS));
	XPm_Out32(PSXC_LPX_SLCR_REQ_SWRST_INT_EN, XPm_In32(PSXC_LPX_SLCR_REQ_SWRST_STATUS));

	/* Mark ACPUx powered up in LOCAL_PWR_STATUS register */
	XPm_RMW32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_0, Args->PwrStateMask, Args->PwrStateMask);

done:
	return Status;
}

static XStatus XPmPower_ACpuPwrDwn(struct XPmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;

	/* Disable CORE_x_POWER interrupt */
	XPm_Out32(Args->CorePcilAddr + APU_PCIL_CORE_IDS_POWER_OFFSET, APU_PCIL_CORE_PREQ_MASK);

	/*TBD: check for emulated power down/debug recovery pwrdwn*/
	XPm_RMW32(Args->RstAddr, Args->WarmRstMask & PSX_CRF_RST_APU_WARM_RST_MASK,
		     Args->WarmRstMask);

	/* TBD: for emulation and debug recovery pwrdwn modes
	 * no need to enable isolation and no need to disable power
	 */
	/* enable isolation */
	XPm_RMW32(Args->PwrCtrlAddr, PSXC_LPX_SLCR_APU_CORE_PWR_CNTRL_STS_ISOLATION_MASK,
		  PSXC_LPX_SLCR_APU_CORE_PWR_CNTRL_STS_ISOLATION_MASK);

	/* disable power to the core */
	XPm_RMW32(Args->PwrCtrlAddr, PSXC_LPX_SLCR_APU_CORE_PWR_CNTRL_PWRUP_GATES_MASK, ~PSXC_LPX_SLCR_APU_CORE_PWR_CNTRL_PWRUP_GATES_MASK);

	/* Poll the power stage status */
	Status = XPm_PollForZero(Args->PwrCtrlAddr + PSXC_LPX_SLCR_APU_CORE_PWR_CNTRL_STS_OFFSET, PSXC_LPX_SLCR_APU_CORE_PWR_CNTRL_PWRUP_GATES_MASK, Args->PwrDwnAckTimeout);
	if (XST_SUCCESS != Status) {
		PmErr("A78 core island power down ack timeout..\n");
		goto done;
	}

done:
	return Status;
}

static XStatus XPmPower_ACpuDirectPwrDwn(struct XPmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;

	/*Disable the Scan Clear and Mem Clear triggers*/
	XPm_RMW32(PSXC_LPX_SLCR_SCAN_CLEAR_TRIGGER, Args->PwrStateMask, ~Args->PwrStateMask);
	XPm_RMW32(PSXC_LPX_SLCR_MEM_CLEAR_TRIGGER, Args->PwrStateMask, ~Args->PwrStateMask);

	/* Set the PSTATE field to power off the core */
	XPm_Out32(Args->CorePcilAddr + APU_PCIL_CORE_PSTATE_OFFSET, 0U);

	/* Set PREQ field */
	XPm_Out32(Args->CorePcilAddr + APU_PCIL_CORE_PREQ_OFFSET, APU_PCIL_CORE_PREQ_MASK);

	/* poll for power state change */
	Status = XPm_PollForMask(Args->CorePcilAddr + APU_PCIL_CORE_PACTIVE_OFFSET, APU_PCIL_CORE_PACCEPT_MASK, ACPU_PACCEPT_TIMEOUT);
	if (XST_SUCCESS != Status) {
		PmErr("A78 Core PACCEPT timeout..\n");
		goto done;
	}

	/* Clear PREQ bit */
	XPm_Out32(Args->CorePcilAddr + APU_PCIL_CORE_PREQ_OFFSET, 0U);

	Status = XPmPower_ACpuPwrDwn(Args);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Clear the Interrupt */
	XPm_Out32(PSXC_LPX_SLCR_WAKEUP0_IRQ_STATUS, Args->PwrStateMask);
	u32 PwrState = XPm_In32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_0) & ((u32)0xFU << ((u32)Args->ClusterId * 4U));

	/* Power down cluster if all cores in cluster are powered off */
	if (1 == __builtin_popcount(PwrState)) {
		XPm_Out32(Args->ClusterPcilAddr + APU_PCIL_CLUSTER_PSTATE_OFFSET, 0U);
		XPm_Out32(Args->ClusterPcilAddr + APU_PCIL_CLUSTER_PREQ_OFFSET, APU_PCIL_CLUSTER_PREQ_MASK);

		Status =  XPm_PollForMask(Args->ClusterPcilAddr + APU_PCIL_CLUSTER_PACTIVE_OFFSET, APU_PCIL_CLUSTER_PACCEPT_MASK, ACPU_PACCEPT_TIMEOUT);
		if (XST_SUCCESS != Status) {
			PmErr("A78 Cluster PACCEPT timeout..\n");
			goto done;
		}
		/* Clear PREQ bit */
		XPm_Out32(Args->ClusterPcilAddr + APU_PCIL_CLUSTER_PREQ_OFFSET, 0U);
		ApuClusterState[Args->ClusterId] = 0U;
	}

	/*Mark ACPUx powered down in LOCAL_PWR_STATUS register */
	XPm_RMW32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_0, Args->PwrStateMask, ~Args->PwrStateMask);

	Status = XST_SUCCESS;

done:
	return Status;
}

maybe_unused static XStatus XPmPower_ACpuReqPwrDwn(struct XPmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;

	/*Disable the Scan Clear and Mem Clear triggers*/
	XPm_RMW32(PSXC_LPX_SLCR_SCAN_CLEAR_TRIGGER, Args->PwrStateMask, ~Args->PwrStateMask);
	XPm_RMW32(PSXC_LPX_SLCR_MEM_CLEAR_TRIGGER, Args->PwrStateMask, ~Args->PwrStateMask);

	Status = XPmPower_ACpuPwrDwn(Args);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Unmask the Power Up Interrupt */
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP0_INT_EN, Args->PwrStateMask);

	/* clear the Power dwn Interrupt */
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN0_STATUS, Args->PwrStateMask);

	/*Mark ACPUx powered down in LOCAL_PWR_STATUS register */
	XPm_RMW32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_0, Args->PwrStateMask, ~Args->PwrStateMask);

done:
	return Status;
}

static XStatus XPmPower_RpuPwrUp(struct XPmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;

	/*TBD: check if powering up from emulated pwr dwn state,if so skip below 4 instructions*/
	/* Restore Power to Core */
	XPm_RMW32(Args->PwrCtrlAddr, PSXC_LPX_SLCR_RPU_CORE_PWR_CNTRL_PWRUP_GATES_MASK,
		     PSXC_LPX_SLCR_RPU_CORE_PWR_CNTRL_PWRUP_GATES_MASK);

	/*Remove isolation */
	XPm_RMW32(Args->PwrCtrlAddr, PSXC_LPX_SLCR_RPU_CORE_PWR_CNTRL_STS_ISOLATION_MASK,
		     ~PSXC_LPX_SLCR_RPU_CORE_PWR_CNTRL_STS_ISOLATION_MASK);

	/* Restore Power to the RPU core cache RAMs */
	XPm_RMW32(PSXC_LPX_SLCR_RPU_CACHE_PWR_CNTRL, Args->CacheCntrlMask, Args->CacheCntrlMask);

	/* Enable the caches */
	XPm_RMW32(PSXC_LPX_SLCR_RPU_CACHE_CE_CNTRL, Args->CacheCntrlMask, Args->CacheCntrlMask);

	/*Mark RPUx powered up in LOCAL_PWR_STATE register */
	XPm_RMW32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_0, Args->PwrStateMask, Args->PwrStateMask);
	Status = XST_SUCCESS;

	return Status;
}

static XStatus XPmPower_RpuDirectPwrUp(struct XPmFwPwrCtrl_t *Args, u64 ResumeAddr)
{
	XStatus Status = XST_FAILURE;
	u32 LowAddress;

	/* reset assert */
	XPm_RMW32(Args->RstCtrlAddr, Args->RstCtrlMask, Args->RstCtrlMask);

	/* Set the resume address. */
	LowAddress = (u32)(ResumeAddr & PSX_RPU_CLUSTER_CORE_VECTABLE_MASK);
	if (0U != (ResumeAddr & 1ULL)) {
		u32 TcmBootFlag = (Xil_In32(Args->ResetCfgAddr) & RPU_TCMBOOT_MASK) >> 0x4;
		if(0U == TcmBootFlag){
			XPm_Out32(Args->VectTableAddr, LowAddress);
		}
	}

	/* Mask wake interrupt */
	XPm_Out32(PSXC_LPX_SLCR_WAKEUP1_IRQ_DIS, Args->WakeupIrqMask);

	/* Mask RPU PCIL Interrupts */
	XPm_RMW32(Args->CorePcilIdsAddr, LPD_SLCR_RPU_PCIL_ISR_PACTIVE1_MASK, LPD_SLCR_RPU_PCIL_ISR_PACTIVE1_MASK);

	Status = XPmPower_RpuPwrUp(Args);
	if(XST_SUCCESS != Status){
		goto done;
	}

	/* set pstate field */
	XPm_Out32(Args->CorePcilPsAddr, ~PSXC_LPX_SLCR_RPU_PCIL_PS_PSTATE_MASK);

	/* set preq field to request power state change */
	XPm_Out32(Args->CorePcilPrAddr, PSXC_LPX_SLCR_RPU_PCIL_PR_PREQ_MASK);

	/* release reset */
	XPm_RMW32(Args->RstCtrlAddr, Args->RstCtrlMask, ~Args->RstCtrlMask);


	/* TODO: Skip for now due to SPP issue */
	/*Status = XPm_PollForMask(Args->CorePcilPaAddr,
				 PSXC_LPX_SLCR_RPU_PCIL_PA_PACCEPT_MASK, RPU_PACTIVE_TIMEOUT);
	if (XST_SUCCESS != Status) {
		PmErr("R52 Core PACCEPT timeout..\n");
		goto done;
	}*/

	/* Clear PREQ bit */
	XPm_Out32(Args->CorePcilPrAddr, 0U);

	/* Disable and clear RPUx direct wake-up interrupt request */
	XPm_Out32(PSXC_LPX_SLCR_WAKEUP1_IRQ_STATUS, Args->WakeupIrqMask);

	/*
	 * Unmask interrupt for all Power-up Requests and Reset Requests that
	 * are triggered but have their interrupt masked.
	 */
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP1_INT_EN, XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS));
	XPm_Out32(PSXC_LPX_SLCR_REQ_SWRST_INT_EN, XPm_In32(PSXC_LPX_SLCR_REQ_SWRST_STATUS));

done:
	return Status;
}

static XStatus XPmPower_RpuPwrDwn(struct XPmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;

	/*TBD: check if it emulated pwr dwn, if so skip below 4 instructions*/
	/* reset assert */
	XPm_RMW32(Args->RstCtrlAddr, Args->RstCtrlMask, Args->RstCtrlMask);

	/* Enable isolation */
	XPm_RMW32(Args->PwrCtrlAddr, PSXC_LPX_SLCR_RPU_CORE_PWR_CNTRL_STS_ISOLATION_MASK,
		     PSXC_LPX_SLCR_RPU_CORE_PWR_CNTRL_STS_ISOLATION_MASK);

	/* disable power to rpu core */
	XPm_RMW32(Args->PwrCtrlAddr, PSXC_LPX_SLCR_RPU_CORE_PWR_CNTRL_PWRUP_GATES_MASK,
		     ~PSXC_LPX_SLCR_RPU_CORE_PWR_CNTRL_PWRUP_GATES_MASK);

	/* Disable the RPU core caches */
	XPm_RMW32(PSXC_LPX_SLCR_RPU_CACHE_CE_CNTRL, Args->PwrStateMask >> 16, ~(Args->PwrStateMask >> 16));

	/* Power gate the RPU core cache RAMs */
	XPm_RMW32(PSXC_LPX_SLCR_RPU_CACHE_PWR_CNTRL, Args->PwrStateMask >> 16, ~(Args->PwrStateMask >> 16));

	/*mask pwr down interrupt*/
	/* TODO: Check if PSXC_LPX_SLCR_POWER_DWN_IRQ_DIS_RPU_SHIFT is correct */
	XPm_Out32(PSXC_LPX_SLCR_POWER_DWN_IRQ_DIS, Args->PwrStateMask <<
		       PSXC_LPX_SLCR_POWER_DWN_IRQ_DIS_RPU_SHIFT);

	/*clear pwr ctrl ISR*/
	/* TODO: Check if PSXC_GLOBAL_REG_PWR_CTRL1_IRQ_RPU_X_COREX_SHIFT is correct */
	XPm_Out32(PSXC_LPX_SLCR_WAKEUP0_IRQ_STATUS, Args->PwrStateMask <<
		       PSXC_GLOBAL_REG_PWR_CTRL1_IRQ_RPU_X_COREX_SHIFT);

	/*Mark RPUx powered down in LOCAL_PWR_STATE register */
	XPm_RMW32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_0, Args->PwrStateMask,~Args->PwrStateMask);
	Status = XST_SUCCESS;

	return Status;
}

static XStatus XPmPower_RpuDirectPwrDwn(struct XPmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;

	/*clear PwrDwn En bit*/
	XPm_Out32(Args->CorePcilPwrdwnAddr, 0U);

	/*poll for PACTIVE to go low*/
	Status = XPm_PollForZero(Args->CorePcilPaAddr, Args->CorePactiveMask, RPU_PACTIVE_TIMEOUT);
	if(XST_SUCCESS != Status){
		PmErr("Pactive bit is low\n");
		goto done;
	}

	/* set pstate bit */
	XPm_Out32(Args->CorePcilPsAddr, PSXC_LPX_SLCR_RPU_PCIL_PS_PSTATE_MASK);

	/* set preq field to request power state change */
	XPm_Out32(Args->CorePcilPrAddr, PSXC_LPX_SLCR_RPU_PCIL_PR_PREQ_MASK);

	/*poll for PACCEPT*/
	Status = XPm_PollForMask(Args->CorePcilPaAddr, PSXC_LPX_SLCR_RPU_PCIL_PA_PACCEPT_MASK, RPU_PACTIVE_TIMEOUT);
	if(XST_SUCCESS != Status){
		PmErr("Paccept bit is not set\n");
		goto done;
	}

	Status = XPmPower_RpuPwrDwn(Args);
	if(XST_SUCCESS != Status){
		goto done;
	}

	/*unmask wakeup interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_WAKEUP1_IRQ_EN, Args->PwrStateMask);

	/*clear ISR*/
	XPm_Out32(Args->CorePcilIsrAddr, LPD_SLCR_RPU_PCIL_ISR_PACTIVE1_MASK);

	/* Unmask the RPU PCIL Interrupt */
	XPm_Out32(Args->CorePcilIenAddr, LPD_SLCR_RPU_PCIL_ISR_PACTIVE1_MASK);

done:
	return Status;
}

static XStatus XPmPower_RpuReqPwrUp(struct XPmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;

	/* Check if already power up */
	RegVal = XPm_In32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_0);
	if (CHECK_BIT(RegVal, Args->PwrStateMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* mask powerup interrupt */
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP1_INT_DIS, Args->PwrStateMask);

	/* reset assert */
	XPm_RMW32(Args->RstCtrlAddr, Args->RstCtrlMask, Args->RstCtrlMask);

	Status = XPmPower_RpuPwrUp(Args);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

static XStatus XPmPower_MemPwrDwn(struct XPmFwMemPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;

	/*clear the interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS, Args->GlobPwrStatusMask);
	u32 Retention = XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS) & Args->RetMask;

	/*Clear the retention bit*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS, Args->RetMask);
	if(0U != Retention){
		/*Set the retention bit*/
		XPm_RMW32(PSXC_LPX_SLCR_OCM_RET_CNTRL, Args->PwrStatusMask, Args->PwrStatusMask);
		/*Check the retention mode is enabled or not*/
		if((XPm_In32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_1)&Args->PwrStateMask) != Args->PwrStateMask){
			PmErr("Retention mode is not set\n");
			/*TBD: PSXC_LOCAL_REG_LOC_AUX_PWR_STATE bit is not setting to 1,uncomment once it is fixed*/

	/* TODO: Check value of PSXC_GLOBAL_REG_REQ_PWRUP1_INT_DIS_RPU_X_COREX_SHIFT *///goto done;
		}
	}else{
		/*power down the OCM RAMs without Retention*/
		XPm_RMW32(PSXC_LPX_SLCR_OCM_RET_CNTRL, Args->RetCtrlMask, ~Args->RetCtrlMask);

		/*poll for disable retention*/
		Status = XPm_PollForZero(PMXC_GLOBAL_PMC_AUX_PWR_STATE_1, Args->PwrStateMask, OCM_PWR_STATE_ACK_TIMEOUT);
		if (Status != XST_SUCCESS) {
			PmErr("Retenstion is not disabled\n");
			goto done;
		}
	}

	/*Disable power to ocm banks*/
	XPm_RMW32(PSXC_LPX_SLCR_OCM_PWR_CNTRL, Args->PwrCtrlMask, ~Args->PwrCtrlMask);

	/*Disable chip enable signal*/
	XPm_RMW32(PSXC_LPX_SLCR_OCM_CE_CNTRL, Args->ChipEnMask, ~Args->ChipEnMask);

	/*reset bit in local reg*/
	XPm_RMW32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_1, Args->PwrStateMask, ~Args->PwrStateMask);

	/*Read the OCM Power Status register*/
	Status = XPm_PollForZero(PSXC_LPX_SLCR_OCM_PWR_STATUS, Args->PwrStatusMask, OCM_PWR_STATE_ACK_TIMEOUT);
	if (Status != XST_SUCCESS) {
		PmErr("bit is not set\n");
		goto done;
	}

	/*Unmask the OCM Power Up Interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP2_INT_EN, Args->GlobPwrStatusMask);

	/*Clear the interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN2_STATUS, Args->GlobPwrStatusMask);

done:
	return Status;
}

static XStatus XPmPower_MemPwrUp(struct XPmFwMemPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;

	/*set chip enable*/
	XPm_RMW32(PSXC_LPX_SLCR_OCM_CE_CNTRL, Args->ChipEnMask, Args->ChipEnMask);

	/* enable power*/
	XPm_RMW32(PSXC_LPX_SLCR_OCM_PWR_CNTRL, Args->PwrCtrlMask, Args->PwrCtrlMask);

	/*set bit in local reg*/
	XPm_RMW32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_1, Args->PwrStateMask, Args->PwrStateMask);

	Status = XPm_PollForMask(PSXC_LPX_SLCR_OCM_PWR_STATUS, Args->PwrStatusMask, OCM_PWR_STATE_ACK_TIMEOUT);
	if (Status != XST_SUCCESS) {
		PmErr("bit is not set\n");
		goto done;
	}

	/* Unmask the OCM Power Down Interrupt	and retention mask*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN2_INT_EN, Args->GlobPwrStatusMask);
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN2_INT_EN, Args->RetMask);

	/*Clear the interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP2_STATUS, Args->GlobPwrStatusMask);

done:
	return Status;
}

static XStatus XPmPower_TcmRpuPwrUp(struct XPmTcmPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 Mode;
	u32 RegVal;

	/* Check if already power up */
	RegVal = XPm_In32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_1);

	if ((TCM_A_0 == Args->Id) &&
	     !CHECK_BIT(RegVal, Rpu0_Core0PwrCtrl.PwrStateMask)) {
		Status = XPmPower_RpuReqPwrUp(&Rpu0_Core0PwrCtrl);
	} else if ((TCM_A_1 == Args->Id) &&
		   !CHECK_BIT(RegVal, Rpu0_Core1PwrCtrl.PwrStateMask)) {
		/* Power up core 1 if RPU cluster A is in split mode */
		Mode = XPm_In32(PSX_RPU_CLUSTER_A_CLUSTER_CFG) & PSX_RPU_CLUSTER_CFG_SLSPLIT_MASK;
		if (PSX_RPU_CLUSTER_CFG_SLSPLIT_MASK == Mode) {
			Status = XPmPower_RpuReqPwrUp(&Rpu0_Core1PwrCtrl);
		} else {
			Status = XST_SUCCESS;
		}
	} else if ((TCM_B_0 == Args->Id) &&
		   !CHECK_BIT(RegVal, Rpu1_Core0PwrCtrl.PwrStateMask)) {
		Status = XPmPower_RpuReqPwrUp(&Rpu1_Core0PwrCtrl);
	} else if ((TCM_B_1 == Args->Id) &&
		   !CHECK_BIT(RegVal, Rpu1_Core1PwrCtrl.PwrStateMask)) {
		/* Power up core 1 if RPU cluster B is in split mode */
		Mode = XPm_In32(PSX_RPU_CLUSTER_B_CLUSTER_CFG) & PSX_RPU_CLUSTER_CFG_SLSPLIT_MASK;
		if (PSX_RPU_CLUSTER_CFG_SLSPLIT_MASK == Mode) {
			Status = XPmPower_RpuReqPwrUp(&Rpu1_Core1PwrCtrl);
		} else {
			Status = XST_SUCCESS;
		}
	} else if ((TCM_C_0 == Args->Id) &&
		   !CHECK_BIT(RegVal, Rpu2_Core0PwrCtrl.PwrStateMask)) {
		Status = XPmPower_RpuReqPwrUp(&Rpu2_Core0PwrCtrl);
	} else if ((TCM_C_1 == Args->Id) &&
		   !CHECK_BIT(RegVal, Rpu2_Core1PwrCtrl.PwrStateMask)) {
		/* Power up core 1 if RPU cluster C is in split mode */
		Mode = XPm_In32(PSX_RPU_CLUSTER_C_CLUSTER_CFG) & PSX_RPU_CLUSTER_CFG_SLSPLIT_MASK;
		if (PSX_RPU_CLUSTER_CFG_SLSPLIT_MASK == Mode) {
			Status = XPmPower_RpuReqPwrUp(&Rpu2_Core1PwrCtrl);
		} else {
			Status = XST_SUCCESS;
		}
	} else if ((TCM_D_0 == Args->Id) &&
		   !CHECK_BIT(RegVal, Rpu3_Core0PwrCtrl.PwrStateMask)) {
		Status = XPmPower_RpuReqPwrUp(&Rpu3_Core0PwrCtrl);
	} else if ((TCM_D_1 == Args->Id) &&
		   !CHECK_BIT(RegVal, Rpu3_Core1PwrCtrl.PwrStateMask)) {
		/* Power up core 1 if RPU cluster D is in split mode */
		Mode = XPm_In32(PSX_RPU_CLUSTER_D_CLUSTER_CFG) & PSX_RPU_CLUSTER_CFG_SLSPLIT_MASK;
		if (PSX_RPU_CLUSTER_CFG_SLSPLIT_MASK == Mode) {
			Status = XPmPower_RpuReqPwrUp(&Rpu3_Core1PwrCtrl);
		} else {
			Status = XST_SUCCESS;
		}
	} else if ((TCM_E_0 == Args->Id) &&
		   !CHECK_BIT(RegVal, Rpu4_Core0PwrCtrl.PwrStateMask)) {
		Status = XPmPower_RpuReqPwrUp(&Rpu4_Core0PwrCtrl);
	} else if ((TCM_E_1 == Args->Id) &&
		   !CHECK_BIT(RegVal, Rpu4_Core1PwrCtrl.PwrStateMask)) {
		/* Power up core 1 if RPU cluster E is in split mode */
		Mode = XPm_In32(PSX_RPU_CLUSTER_E_CLUSTER_CFG) & PSX_RPU_CLUSTER_CFG_SLSPLIT_MASK;
		if (PSX_RPU_CLUSTER_CFG_SLSPLIT_MASK == Mode) {
			Status = XPmPower_RpuReqPwrUp(&Rpu1_Core1PwrCtrl);
		} else {
			Status = XST_SUCCESS;
		}
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}

static XStatus XPmPower_TcmPwrUp(struct XPmTcmPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	struct XPmFwMemPwrCtrl_t *Tcm = &Args->TcmMemPwrCtrl;

	/* RPU needs to be power up to access TCM since TCM is in RPU island */
	Status = XPmPower_TcmRpuPwrUp(Args);
	if (XST_SUCCESS != Status) {
		PmErr("RPU power up failed for TCM ID %d", Args->Id);
		goto done;
	}

	/*Clear the interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP1_STATUS, Tcm->GlobPwrStatusMask);

	/*enable the chip enable signal*/
	XPm_RMW32(PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL, Tcm->ChipEnMask, Tcm->ChipEnMask);
	/*Enable power for corresponding TCM bank*/
	XPm_RMW32(PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL, Tcm->PwrCtrlMask, Tcm->PwrCtrlMask);

	/* Mark tcm bank powered up in LOCAL_PWR_STATE0 register */
	XPm_RMW32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_1, Tcm->PwrStateMask, Tcm->PwrStateMask);
	Status = XPm_PollForMask(PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS, Tcm->PwrStatusMask, TCM_PWR_STATE_ACK_TIMEOUT);
	if (XST_SUCCESS != Status) {
		PmErr("TCM bit is not set\n");
		goto done;
	}

	/* Wait for power to ramp up */
	XPm_Wait(TCM_PWR_UP_WAIT_TIME);

	/* Unmask the TCM Power Down Interrupt	and retention mask*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_INT_EN, Tcm->GlobPwrStatusMask);
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_INT_EN, Tcm->RetMask);

done:
	return Status;
}

static XStatus XPmPower_TcmPwrDown(struct XPmTcmPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	struct XPmFwMemPwrCtrl_t *Tcm = &Args->TcmMemPwrCtrl;

	/*Clear the interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS, Tcm->GlobPwrStatusMask);
	u32 Retention = XPm_In32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS) & Tcm->RetMask;
	if(0U != Retention){
		XPm_RMW32(PSXC_LPX_SLCR_RPU_TCM_RET_CNTRL, Tcm->PwrStatusMask, Tcm->PwrStatusMask);
		XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS, Tcm->RetMask);
		/*Ensure for Retention Mode taken effect*/
		if((XPm_In32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_1) & Tcm->PwrStateMask) != Tcm->PwrStateMask){
			PmErr("Retention mode is not set\n");
			/*TBD: PSXC_LOCAL_REG_LOC_AUX_PWR_STATE bit is not setting to 1,uncomment below line once it is fixed*/
			//goto done;
		}
	}

	/* disable power gate*/
	XPm_RMW32(PSXC_LPX_SLCR_RPU_TCM_PWR_CNTRL, Tcm->PwrStateMask, ~Tcm->PwrStateMask);

	/*disable chip enable signal*/
	XPm_RMW32(PSXC_LPX_SLCR_RPU_TCM_CE_CNTRL, Tcm->ChipEnMask, ~Tcm->ChipEnMask);

	/* reset bit in local reg*/
	XPm_RMW32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_1, Tcm->PwrStateMask, ~Tcm->PwrStateMask);
	Status = XPm_PollForZero(PSXC_LPX_SLCR_RPU_TCM_PWR_STATUS, Tcm->PwrStatusMask, TCM_PWR_STATE_ACK_TIMEOUT);
	if (XST_SUCCESS != Status) {
		PmErr("TCM bit is not reset\n");
		goto done;
	}

	/* unmask tcm powerup interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP1_INT_EN, Tcm->GlobPwrStatusMask);

done:
	return Status;
}

static XStatus XPmPower_MemPwrUpGem(struct XPmFwGemPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	struct XPmFwMemPwrCtrl_t *Gem = &Args->GemMemPwrCtrl;

	/*set chip enable*/
	XPm_RMW32(PSXC_LPX_SLCR_GEM_CE_CNTRL, Gem->ChipEnMask, Gem->ChipEnMask);

	/* enable power*/
	XPm_RMW32(PSXC_LPX_SLCR_GEM_PWR_CNTRL, Gem->PwrCtrlMask, Gem->PwrCtrlMask);

	/*set bit in local reg*/
	XPm_RMW32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_2, Gem->PwrStateMask, Gem->PwrStateMask);

	Status = XPm_PollForMask(PSXC_LPX_SLCR_GEM_PWR_STATUS, Gem->PwrStatusMask, Args->PwrStateAckTimeout);
	if (XST_SUCCESS != Status) {
		PmErr("bit is not set\n");
		goto done;
	}

	/* Unmask the Power Down Interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_INT_EN, Gem->GlobPwrStatusMask);

	/*Clear the interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP1_STATUS, Gem->GlobPwrStatusMask);

done:
	return Status;
}

static XStatus XPmPower_MemPwrDwnGem(struct XPmFwGemPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	struct XPmFwMemPwrCtrl_t *Gem = &Args->GemMemPwrCtrl;

	/*Disable power to gem banks*/
	XPm_RMW32(PSXC_LPX_SLCR_GEM_PWR_CNTRL, Gem->PwrCtrlMask, ~Gem->PwrCtrlMask);

	/*Disable chip enable signal*/
	XPm_RMW32(PSXC_LPX_SLCR_GEM_CE_CNTRL, Gem->ChipEnMask, ~Gem->ChipEnMask);

	/*reset bit in local reg*/
	XPm_RMW32(PMXC_GLOBAL_PMC_AUX_PWR_STATE_2, Gem->PwrStateMask, ~Gem->PwrStateMask);

	/*Read the gem Power Status register*/
	Status = XPm_PollForZero(PSXC_LPX_SLCR_GEM_PWR_STATUS, Gem->PwrStatusMask, Args->PwrStateAckTimeout);
	if (XST_SUCCESS != Status) {
		PmErr("bit is not set\n");
		goto done;
	}

	/*Unmask the gem Power Up Interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRUP1_INT_EN, Gem->GlobPwrStatusMask);

	/*Clear the interrupt*/
	XPm_Out32(PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS, Gem->GlobPwrStatusMask);

done:
	return Status;
}

XStatus XPm_DirectPwrUp(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;

	XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
	u64 ResumeAddr = Core->ResumeAddr;

	switch (DeviceId) {
		case PM_DEV_ACPU_0_0:
			Status = XPmPower_ACpuDirectPwrUp(&Acpu0_Core0PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_ACPU_0_1:
			Status = XPmPower_ACpuDirectPwrUp(&Acpu0_Core1PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_ACPU_1_0:
			Status = XPmPower_ACpuDirectPwrUp(&Acpu1_Core0PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_ACPU_1_1:
			Status = XPmPower_ACpuDirectPwrUp(&Acpu1_Core1PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_ACPU_2_0:
			Status = XPmPower_ACpuDirectPwrUp(&Acpu2_Core0PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_ACPU_2_1:
			Status = XPmPower_ACpuDirectPwrUp(&Acpu2_Core1PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_ACPU_3_0:
			Status = XPmPower_ACpuDirectPwrUp(&Acpu3_Core0PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_ACPU_3_1:
			Status = XPmPower_ACpuDirectPwrUp(&Acpu3_Core1PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_RPU_A_0:
			Status = XPmPower_RpuDirectPwrUp(&Rpu0_Core0PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_RPU_A_1:
			Status = XPmPower_RpuDirectPwrUp(&Rpu0_Core1PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_RPU_B_0:
			Status = XPmPower_RpuDirectPwrUp(&Rpu1_Core0PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_RPU_B_1:
			Status = XPmPower_RpuDirectPwrUp(&Rpu1_Core1PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_RPU_C_0:
			Status = XPmPower_RpuDirectPwrUp(&Rpu2_Core0PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_RPU_C_1:
			Status = XPmPower_RpuDirectPwrUp(&Rpu2_Core1PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_RPU_D_0:
			Status = XPmPower_RpuDirectPwrUp(&Rpu3_Core0PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_RPU_D_1:
			Status = XPmPower_RpuDirectPwrUp(&Rpu3_Core1PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_RPU_E_0:
			Status = XPmPower_RpuDirectPwrUp(&Rpu4_Core0PwrCtrl, ResumeAddr);
			break;
		case PM_DEV_RPU_E_1:
			Status = XPmPower_RpuDirectPwrUp(&Rpu4_Core1PwrCtrl, ResumeAddr);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

XStatus XPm_DirectPwrDwn(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;

	switch (DeviceId) {
		case PM_DEV_ACPU_0_0:
			Status = XPmPower_ACpuDirectPwrDwn(&Acpu0_Core0PwrCtrl);
			break;
		case PM_DEV_ACPU_0_1:
			Status = XPmPower_ACpuDirectPwrDwn(&Acpu0_Core1PwrCtrl);
			break;
		case PM_DEV_ACPU_1_0:
			Status = XPmPower_ACpuDirectPwrDwn(&Acpu1_Core0PwrCtrl);
			break;
		case PM_DEV_ACPU_1_1:
			Status = XPmPower_ACpuDirectPwrDwn(&Acpu1_Core1PwrCtrl);
			break;
		case PM_DEV_ACPU_2_0:
			Status = XPmPower_ACpuDirectPwrDwn(&Acpu2_Core0PwrCtrl);
			break;
		case PM_DEV_ACPU_2_1:
			Status = XPmPower_ACpuDirectPwrDwn(&Acpu2_Core1PwrCtrl);
			break;
		case PM_DEV_ACPU_3_0:
			Status = XPmPower_ACpuDirectPwrDwn(&Acpu3_Core0PwrCtrl);
			break;
		case PM_DEV_ACPU_3_1:
			Status = XPmPower_ACpuDirectPwrDwn(&Acpu3_Core1PwrCtrl);
			break;
		case PM_DEV_RPU_A_0:
			Status = XPmPower_RpuDirectPwrDwn(&Rpu0_Core0PwrCtrl);
			break;
		case PM_DEV_RPU_A_1:
			Status = XPmPower_RpuDirectPwrDwn(&Rpu0_Core1PwrCtrl);
			break;
		case PM_DEV_RPU_B_0:
			Status = XPmPower_RpuDirectPwrDwn(&Rpu1_Core0PwrCtrl);
			break;
		case PM_DEV_RPU_B_1:
			Status = XPmPower_RpuDirectPwrDwn(&Rpu1_Core1PwrCtrl);
			break;
		case PM_DEV_RPU_C_0:
			Status = XPmPower_RpuDirectPwrDwn(&Rpu2_Core0PwrCtrl);
			break;
		case PM_DEV_RPU_C_1:
			Status = XPmPower_RpuDirectPwrDwn(&Rpu2_Core1PwrCtrl);
			break;
		case PM_DEV_RPU_D_0:
			Status = XPmPower_RpuDirectPwrDwn(&Rpu3_Core0PwrCtrl);
			break;
		case PM_DEV_RPU_D_1:
			Status = XPmPower_RpuDirectPwrDwn(&Rpu3_Core1PwrCtrl);
			break;
		case PM_DEV_RPU_E_0:
			Status = XPmPower_RpuDirectPwrDwn(&Rpu4_Core0PwrCtrl);
			break;
		case PM_DEV_RPU_E_1:
			Status = XPmPower_RpuDirectPwrDwn(&Rpu4_Core1PwrCtrl);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;

}

XStatus XPmPower_PlatSendPowerUpReq(XPm_Node *Node)
{
	(void)Node;
	return XST_SUCCESS;
}

XStatus XPmPower_PlatSendPowerDownReq(const XPm_Node *Node)
{
	(void)Node;
	return XST_SUCCESS;
}

XStatus XPmPower_SendIslandPowerUpReq(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;
	/* TODO: Add interrupt handling */

	switch (Node->Id) {
		case PM_POWER_TCM_0_A:
			Status = XPmPower_TcmPwrUp(&TcmA0PwrCtrl);
			break;
		case PM_POWER_TCM_0_B:
			Status = XPmPower_TcmPwrUp(&TcmB0PwrCtrl);
			break;
		case PM_POWER_TCM_1_A:
			Status = XPmPower_TcmPwrUp(&TcmA1PwrCtrl);
			break;
		case PM_POWER_TCM_1_B:
			Status = XPmPower_TcmPwrUp(&TcmB1PwrCtrl);
			break;
		case PM_POWER_TCM_0_C:
			Status = XPmPower_TcmPwrUp(&TcmC0PwrCtrl);
			break;
		case PM_POWER_TCM_1_C:
			Status = XPmPower_TcmPwrUp(&TcmC1PwrCtrl);
			break;
		case PM_POWER_TCM_0_D:
			Status = XPmPower_TcmPwrUp(&TcmD0PwrCtrl);
			break;
		case PM_POWER_TCM_1_D:
			Status = XPmPower_TcmPwrUp(&TcmD1PwrCtrl);
			break;
		case PM_POWER_TCM_0_E:
			Status = XPmPower_TcmPwrUp(&TcmE0PwrCtrl);
			break;
		case PM_POWER_TCM_1_E:
			Status = XPmPower_TcmPwrUp(&TcmE1PwrCtrl);
			break;
		case PM_POWER_OCM_0_0:
			Status = XPmPower_MemPwrUp(&Ocm_B0_I0_PwrCtrl);
			break;
		case PM_POWER_OCM_0_1:
			Status = XPmPower_MemPwrUp(&Ocm_B0_I1_PwrCtrl);
			break;
		case PM_POWER_OCM_0_2:
			Status = XPmPower_MemPwrUp(&Ocm_B0_I2_PwrCtrl);
			break;
		case PM_POWER_OCM_0_3:
			Status = XPmPower_MemPwrUp(&Ocm_B0_I3_PwrCtrl);
			break;
		case PM_POWER_OCM_1_0:
			Status = XPmPower_MemPwrUp(&Ocm_B1_I0_PwrCtrl);
			break;
		case PM_POWER_OCM_1_1:
			Status = XPmPower_MemPwrUp(&Ocm_B1_I1_PwrCtrl);
			break;
		case PM_POWER_OCM_1_2:
			Status = XPmPower_MemPwrUp(&Ocm_B1_I2_PwrCtrl);
			break;
		case PM_POWER_OCM_1_3:
			Status = XPmPower_MemPwrUp(&Ocm_B1_I3_PwrCtrl);
			break;
		case PM_POWER_OCM_2_0:
			Status = XPmPower_MemPwrUp(&Ocm_B2_I0_PwrCtrl);
			break;
		case PM_POWER_OCM_2_1:
			Status = XPmPower_MemPwrUp(&Ocm_B2_I1_PwrCtrl);
			break;
		case PM_POWER_OCM_2_2:
			Status = XPmPower_MemPwrUp(&Ocm_B2_I2_PwrCtrl);
			break;
		case PM_POWER_OCM_2_3:
			Status = XPmPower_MemPwrUp(&Ocm_B2_I3_PwrCtrl);
			break;
		case PM_POWER_OCM_3_0:
			Status = XPmPower_MemPwrUp(&Ocm_B3_I0_PwrCtrl);
			break;
		case PM_POWER_OCM_3_1:
			Status = XPmPower_MemPwrUp(&Ocm_B3_I1_PwrCtrl);
			break;
		case PM_POWER_OCM_3_2:
			Status = XPmPower_MemPwrUp(&Ocm_B3_I2_PwrCtrl);
			break;
		case PM_POWER_OCM_3_3:
			Status = XPmPower_MemPwrUp(&Ocm_B3_I3_PwrCtrl);
			break;
		case PM_POWER_GEM0:
			Status = XPmPower_MemPwrUpGem(&Gem0PwrCtrl);
			break;
		case PM_POWER_GEM1:
			Status = XPmPower_MemPwrUpGem(&Gem1PwrCtrl);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

XStatus XPmPower_SendIslandPowerDwnReq(const XPm_Node *Node)
{
	XStatus Status = XST_FAILURE;

	switch (Node->Id) {
		case PM_POWER_TCM_0_A:
			Status = XPmPower_TcmPwrDown(&TcmA0PwrCtrl);
			break;
		case PM_POWER_TCM_0_B:
			Status = XPmPower_TcmPwrDown(&TcmB0PwrCtrl);
			break;
		case PM_POWER_TCM_1_A:
			Status = XPmPower_TcmPwrDown(&TcmA1PwrCtrl);
			break;
		case PM_POWER_TCM_1_B:
			Status = XPmPower_TcmPwrDown(&TcmB1PwrCtrl);
			break;
		case PM_POWER_TCM_0_C:
			Status = XPmPower_TcmPwrDown(&TcmC0PwrCtrl);
			break;
		case PM_POWER_TCM_1_C:
			Status = XPmPower_TcmPwrDown(&TcmC1PwrCtrl);
			break;
		case PM_POWER_TCM_0_D:
			Status = XPmPower_TcmPwrDown(&TcmD0PwrCtrl);
			break;
		case PM_POWER_TCM_1_D:
			Status = XPmPower_TcmPwrDown(&TcmD1PwrCtrl);
			break;
		case PM_POWER_TCM_0_E:
			Status = XPmPower_TcmPwrDown(&TcmE0PwrCtrl);
			break;
		case PM_POWER_TCM_1_E:
			Status = XPmPower_TcmPwrDown(&TcmE1PwrCtrl);
			break;
		case PM_POWER_OCM_0_0:
			Status = XPmPower_MemPwrDwn(&Ocm_B0_I0_PwrCtrl);
			break;
		case PM_POWER_OCM_0_1:
			Status = XPmPower_MemPwrDwn(&Ocm_B0_I1_PwrCtrl);
			break;
		case PM_POWER_OCM_0_2:
			Status = XPmPower_MemPwrDwn(&Ocm_B0_I2_PwrCtrl);
			break;
		case PM_POWER_OCM_0_3:
			Status = XPmPower_MemPwrDwn(&Ocm_B0_I3_PwrCtrl);
			break;
		case PM_POWER_OCM_1_0:
			Status = XPmPower_MemPwrDwn(&Ocm_B1_I0_PwrCtrl);
			break;
		case PM_POWER_OCM_1_1:
			Status = XPmPower_MemPwrDwn(&Ocm_B1_I1_PwrCtrl);
			break;
		case PM_POWER_OCM_1_2:
			Status = XPmPower_MemPwrDwn(&Ocm_B1_I2_PwrCtrl);
			break;
		case PM_POWER_OCM_1_3:
			Status = XPmPower_MemPwrDwn(&Ocm_B1_I3_PwrCtrl);
			break;
		case PM_POWER_OCM_2_0:
			Status = XPmPower_MemPwrDwn(&Ocm_B2_I0_PwrCtrl);
			break;
		case PM_POWER_OCM_2_1:
			Status = XPmPower_MemPwrDwn(&Ocm_B2_I1_PwrCtrl);
			break;
		case PM_POWER_OCM_2_2:
			Status = XPmPower_MemPwrDwn(&Ocm_B2_I2_PwrCtrl);
			break;
		case PM_POWER_OCM_2_3:
			Status = XPmPower_MemPwrDwn(&Ocm_B2_I3_PwrCtrl);
			break;
		case PM_POWER_OCM_3_0:
			Status = XPmPower_MemPwrDwn(&Ocm_B3_I0_PwrCtrl);
			break;
		case PM_POWER_OCM_3_1:
			Status = XPmPower_MemPwrDwn(&Ocm_B3_I1_PwrCtrl);
			break;
		case PM_POWER_OCM_3_2:
			Status = XPmPower_MemPwrDwn(&Ocm_B3_I2_PwrCtrl);
			break;
		case PM_POWER_OCM_3_3:
			Status = XPmPower_MemPwrDwn(&Ocm_B3_I3_PwrCtrl);
			break;
		case PM_POWER_GEM0:
			Status = XPmPower_MemPwrDwnGem(&Gem0PwrCtrl);
			break;
		case PM_POWER_GEM1:
			Status = XPmPower_MemPwrDwnGem(&Gem1PwrCtrl);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}
