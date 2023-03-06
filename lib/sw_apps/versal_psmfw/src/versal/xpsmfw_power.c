/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_power.c
*
* This file contains power handler functions for PS Power islands and FPD
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ---- ---- -------- ------------------------------
* 1.00	rp	07/13/2018 	Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#include "xpsmfw_api.h"
#include "xpsmfw_default.h"
#include "xpsmfw_power.h"
#include "xpsmfw_init.h"
#include "fpd_apu.h"
#include "psm_global.h"
#include "rpu.h"
#include "crl.h"
#include "crf.h"
#include "pmc_global.h"
#include <assert.h>
#define CHECK_BIT(reg, mask)	(((reg) & (mask)) == (mask))

/**
 * NOTE: Older PsmToPlmEvent version (0x1U) only consists Event array
 *       while new version (0x2U) adds CpuIdleFlag and ResumeAddress in it.
 */
#define PSM_TO_PLM_EVENT_VERSION		(0x2U)
#define PWR_UP_EVT				(0x1U)
#define PWR_DWN_EVT				(0x100U)
#define PROC_DATA_LEN				(32*1024)

static u8 ProcData[PROC_DATA_LEN];

static volatile struct PsmToPlmEvent_t PsmToPlmEvent = {
	.Version	= PSM_TO_PLM_EVENT_VERSION,
	.Event		= {0x0},
	.CpuIdleFlag 	= {0x0},
	.ResumeAddress 	= {0x0},
	.ProcDataAddress = (u32)ProcData,
	.ProcDataLen = PROC_DATA_LEN,
};

static u32 LocalPwrState;

static struct XPsmFwPwrCtrl_t Acpu0PwrCtrl = {
	.Id = ACPU_0,
	.ResetCfgAddr = FPD_APU_RVBARADDR0L,
	.PwrStateMask = PSM_LOCAL_PWR_STATE_ACPU0_MASK,
	.PwrCtrlAddr = PSM_LOCAL_ACPU0_PWR_CNTRL,
	.PwrStatusAddr = PSM_LOCAL_ACPU0_PWR_STATUS,
	.PwrUpAckTimeout = {
		XPSMFW_PWRUP_ACPU0_CHN0_TO,
		XPSMFW_PWRUP_ACPU0_CHN1_TO,
		XPSMFW_PWRUP_ACPU0_CHN2_TO,
		XPSMFW_PWRUP_ACPU0_CHN3_TO },
	.PwrUpWaitTime = {
		XPSMFW_PWRUP_ACPU0_CHN0_WAIT_TM,
		XPSMFW_PWRUP_ACPU0_CHN1_WAIT_TM,
		XPSMFW_PWRUP_ACPU0_CHN2_WAIT_TM,
		XPSMFW_PWRUP_ACPU0_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = XPSMFW_PWRDWN_ACPU0_TO,
	.ClkCtrlAddr = CRF_ACPU_CTRL,
	.ClkCtrlMask = CRF_ACPU_CTRL_CLKACT_MASK,
	.ClkPropTime = XPSMFW_ACPU_CTRL_CLK_PROP_TIME,
	.RstCtrlMask = CRF_RST_APU_ACPU0_MASK,
	.MbistBitMask = PSM_GLOBAL_ACPU0_MBIST_BIT_MASK,
};

static struct XPsmFwPwrCtrl_t Acpu1PwrCtrl = {
	.Id = ACPU_1,
	.ResetCfgAddr = FPD_APU_RVBARADDR1L,
	.PwrStateMask = PSM_LOCAL_PWR_STATE_ACPU1_MASK,
	.PwrCtrlAddr = PSM_LOCAL_ACPU1_PWR_CNTRL,
	.PwrStatusAddr = PSM_LOCAL_ACPU1_PWR_STATUS,
	.PwrUpAckTimeout = {
		XPSMFW_PWRUP_ACPU1_CHN0_TO,
		XPSMFW_PWRUP_ACPU1_CHN1_TO,
		XPSMFW_PWRUP_ACPU1_CHN2_TO,
		XPSMFW_PWRUP_ACPU1_CHN3_TO },
	.PwrUpWaitTime = {
		XPSMFW_PWRUP_ACPU1_CHN0_WAIT_TM,
		XPSMFW_PWRUP_ACPU1_CHN1_WAIT_TM,
		XPSMFW_PWRUP_ACPU1_CHN2_WAIT_TM,
		XPSMFW_PWRUP_ACPU1_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = XPSMFW_PWRDWN_ACPU1_TO,
	.ClkCtrlAddr = CRF_ACPU_CTRL,
	.ClkCtrlMask = CRF_ACPU_CTRL_CLKACT_MASK,
	.ClkPropTime = XPSMFW_ACPU_CTRL_CLK_PROP_TIME,
	.RstCtrlMask = CRF_RST_APU_ACPU1_MASK,
	.MbistBitMask = PSM_GLOBAL_ACPU1_MBIST_BIT_MASK,
};

static struct XPsmFwPwrCtrl_t Rpu0PwrCtrl = {
	.Id = RPU0_0,
	.ResetCfgAddr = RPU_RPU_0_CFG,
	.PwrStateMask = PSM_LOCAL_PWR_STATE_R5_0_MASK,
	.PwrCtrlAddr = PSM_LOCAL_RPU_PWR_CNTRL,
	.PwrStatusAddr = PSM_LOCAL_RPU_PWR_STATUS,
	.PwrUpAckTimeout = {
		XPSMFW_PWRUP_RPU_CHN0_TO,
		XPSMFW_PWRUP_RPU_CHN1_TO,
		XPSMFW_PWRUP_RPU_CHN2_TO,
		XPSMFW_PWRUP_RPU_CHN3_TO },
	.PwrUpWaitTime = {
		XPSMFW_PWRUP_RPU_CHN0_WAIT_TM,
		XPSMFW_PWRUP_RPU_CHN1_WAIT_TM,
		XPSMFW_PWRUP_RPU_CHN2_WAIT_TM,
		XPSMFW_PWRUP_RPU_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = XPSMFW_PWRDWN_RPU_TO,
	.ClkCtrlAddr = CRL_CPU_R5_CTRL,
	.ClkCtrlMask = CRL_CPU_R5_CTRL_CLKACT_CORE_MASK,
	.ClkPropTime = XPSMFW_RPU_CTRL_CLK_PROP_TIME,
	.RstCtrlMask = CRL_RST_CPU_R5_RESET_CPU0_MASK,
	.MbistBitMask = PSM_GLOBAL_RPU_MBIST_BIT_MASK,
};

static struct XPsmFwPwrCtrl_t Rpu1PwrCtrl = {
	.Id = RPU0_1,
	.ResetCfgAddr = RPU_RPU_1_CFG,
	.PwrStateMask = PSM_LOCAL_PWR_STATE_R5_1_MASK,
	.PwrCtrlAddr = PSM_LOCAL_RPU_PWR_CNTRL,
	.PwrStatusAddr = PSM_LOCAL_RPU_PWR_STATUS,
	.PwrUpAckTimeout = {
		XPSMFW_PWRUP_RPU_CHN0_TO,
		XPSMFW_PWRUP_RPU_CHN1_TO,
		XPSMFW_PWRUP_RPU_CHN2_TO,
		XPSMFW_PWRUP_RPU_CHN3_TO },
	.PwrUpWaitTime = {
		XPSMFW_PWRUP_RPU_CHN0_WAIT_TM,
		XPSMFW_PWRUP_RPU_CHN1_WAIT_TM,
		XPSMFW_PWRUP_RPU_CHN2_WAIT_TM,
		XPSMFW_PWRUP_RPU_CHN3_WAIT_TM },
	.PwrDwnAckTimeout = XPSMFW_PWRDWN_RPU_TO,
	.ClkCtrlAddr = CRL_CPU_R5_CTRL,
	.ClkCtrlMask = CRL_CPU_R5_CTRL_CLKACT_CORE_MASK,
	.ClkPropTime = XPSMFW_RPU_CTRL_CLK_PROP_TIME,
	.RstCtrlMask = CRL_RST_CPU_R5_RESET_CPU1_MASK,
	.MbistBitMask = PSM_GLOBAL_RPU_MBIST_BIT_MASK,
};

static struct XPsmFwMemPwrCtrl_t Ocm0PwrCtrl = {
	.PwrStateMask = PSM_LOCAL_PWR_STATE_OCM_BANK0_MASK,
	.ChipEnAddr = PSM_LOCAL_OCM_CE_CNTRL,
	.ChipEnMask = PSM_LOCAL_OCM_CE_CNTRL_BANK0_MASK,
	.PwrCtrlAddr = PSM_LOCAL_OCM_PWR_CNTRL,
	.PwrCtrlMask = PSM_LOCAL_OCM_PWR_CNTRL_BANK0_MASK,
	.PwrStatusAddr = PSM_LOCAL_OCM_PWR_STATUS,
	.PwrStatusMask = PSM_LOCAL_OCM_PWR_STATUS_BANK0_MASK,
	.PwrStateAckTimeout = XPSMFW_OCM0_PWR_STATE_ACK_TIMEOUT,
	.PwrUpWaitTime = XPSMFW_OCM0_PWR_UP_WAIT_TIME,
};

static struct XPsmFwMemPwrCtrl_t Ocm1PwrCtrl = {
	.PwrStateMask = PSM_LOCAL_PWR_STATE_OCM_BANK1_MASK,
	.ChipEnAddr = PSM_LOCAL_OCM_CE_CNTRL,
	.ChipEnMask = PSM_LOCAL_OCM_CE_CNTRL_BANK1_MASK,
	.PwrCtrlAddr = PSM_LOCAL_OCM_PWR_CNTRL,
	.PwrCtrlMask = PSM_LOCAL_OCM_PWR_CNTRL_BANK1_MASK,
	.PwrStatusAddr = PSM_LOCAL_OCM_PWR_STATUS,
	.PwrStatusMask = PSM_LOCAL_OCM_PWR_STATUS_BANK1_MASK,
	.PwrStateAckTimeout = XPSMFW_OCM1_PWR_STATE_ACK_TIMEOUT,
	.PwrUpWaitTime = XPSMFW_OCM1_PWR_UP_WAIT_TIME,
};

static struct XPsmFwMemPwrCtrl_t Ocm2PwrCtrl = {
	.PwrStateMask = PSM_LOCAL_PWR_STATE_OCM_BANK2_MASK,
	.ChipEnAddr = PSM_LOCAL_OCM_CE_CNTRL,
	.ChipEnMask = PSM_LOCAL_OCM_CE_CNTRL_BANK2_MASK,
	.PwrCtrlAddr = PSM_LOCAL_OCM_PWR_CNTRL,
	.PwrCtrlMask = PSM_LOCAL_OCM_PWR_CNTRL_BANK2_MASK,
	.PwrStatusAddr = PSM_LOCAL_OCM_PWR_STATUS,
	.PwrStatusMask = PSM_LOCAL_OCM_PWR_STATUS_BANK2_MASK,
	.PwrStateAckTimeout = XPSMFW_OCM2_PWR_STATE_ACK_TIMEOUT,
	.PwrUpWaitTime = XPSMFW_OCM2_PWR_UP_WAIT_TIME,
};

static struct XPsmFwMemPwrCtrl_t Ocm3PwrCtrl = {
	.PwrStateMask = PSM_LOCAL_PWR_STATE_OCM_BANK3_MASK,
	.ChipEnAddr = PSM_LOCAL_OCM_CE_CNTRL,
	.ChipEnMask = PSM_LOCAL_OCM_CE_CNTRL_BANK3_MASK,
	.PwrCtrlAddr = PSM_LOCAL_OCM_PWR_CNTRL,
	.PwrCtrlMask = PSM_LOCAL_OCM_PWR_CNTRL_BANK3_MASK,
	.PwrStatusAddr = PSM_LOCAL_OCM_PWR_STATUS,
	.PwrStatusMask = PSM_LOCAL_OCM_PWR_STATUS_BANK3_MASK,
	.PwrStateAckTimeout = XPSMFW_OCM3_PWR_STATE_ACK_TIMEOUT,
	.PwrUpWaitTime = XPSMFW_OCM3_PWR_UP_WAIT_TIME,
};

static struct XPsmTcmPwrCtrl_t Tcm0APwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PSM_LOCAL_PWR_STATE_TCM0A_MASK,
		.ChipEnAddr = PSM_LOCAL_TCM_CE_CNTRL,
		.ChipEnMask = PSM_LOCAL_TCM_CE_CNTRL_TCMA0_MASK,
		.PwrCtrlAddr = PSM_LOCAL_TCM_PWR_CNTRL,
		.PwrCtrlMask = PSM_LOCAL_TCM_PWR_CNTRL_TCMA0_MASK,
		.PwrStatusAddr = PSM_LOCAL_TCM_PWR_STATUS,
		.PwrStatusMask = PSM_LOCAL_TCM_PWR_STATUS_TCMA0_MASK,
		.PwrStateAckTimeout = XPSMFW_TCM0A_PWR_STATE_ACK_TIMEOUT,
		.PwrUpWaitTime = XPSMFW_TCM0A_PWR_UP_WAIT_TIME,
	},

	.Id = TCM_0_A,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPsmTcmPwrCtrl_t Tcm0BPwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PSM_LOCAL_PWR_STATE_TCM0B_MASK,
		.ChipEnAddr = PSM_LOCAL_TCM_CE_CNTRL,
		.ChipEnMask = PSM_LOCAL_TCM_CE_CNTRL_TCMB0_MASK,
		.PwrCtrlAddr = PSM_LOCAL_TCM_PWR_CNTRL,
		.PwrCtrlMask = PSM_LOCAL_TCM_PWR_CNTRL_TCMB0_MASK,
		.PwrStatusAddr = PSM_LOCAL_TCM_PWR_STATUS,
		.PwrStatusMask = PSM_LOCAL_TCM_PWR_STATUS_TCMB0_MASK,
		.PwrStateAckTimeout = XPSMFW_TCM0B_PWR_STATE_ACK_TIMEOUT,
		.PwrUpWaitTime = XPSMFW_TCM0B_PWR_UP_WAIT_TIME,
	},

	.Id = TCM_0_B,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPsmTcmPwrCtrl_t Tcm1APwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PSM_LOCAL_PWR_STATE_TCM1A_MASK,
		.ChipEnAddr = PSM_LOCAL_TCM_CE_CNTRL,
		.ChipEnMask = PSM_LOCAL_TCM_CE_CNTRL_TCMA1_MASK,
		.PwrCtrlAddr = PSM_LOCAL_TCM_PWR_CNTRL,
		.PwrCtrlMask = PSM_LOCAL_TCM_PWR_CNTRL_TCMA1_MASK,
		.PwrStatusAddr = PSM_LOCAL_TCM_PWR_STATUS,
		.PwrStatusMask = PSM_LOCAL_TCM_PWR_STATUS_TCMA1_MASK,
		.PwrStateAckTimeout = XPSMFW_TCM1A_PWR_STATE_ACK_TIMEOUT,
		.PwrUpWaitTime = XPSMFW_TCM1A_PWR_UP_WAIT_TIME,
	},

	.Id = TCM_1_A,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPsmTcmPwrCtrl_t Tcm1BPwrCtrl = {
	.TcmMemPwrCtrl = {
		.PwrStateMask = PSM_LOCAL_PWR_STATE_TCM1B_MASK,
		.ChipEnAddr = PSM_LOCAL_TCM_CE_CNTRL,
		.ChipEnMask = PSM_LOCAL_TCM_CE_CNTRL_TCMB1_MASK,
		.PwrCtrlAddr = PSM_LOCAL_TCM_PWR_CNTRL,
		.PwrCtrlMask = PSM_LOCAL_TCM_PWR_CNTRL_TCMB1_MASK,
		.PwrStatusAddr = PSM_LOCAL_TCM_PWR_STATUS,
		.PwrStatusMask = PSM_LOCAL_TCM_PWR_STATUS_TCMB1_MASK,
		.PwrStateAckTimeout = XPSMFW_TCM1B_PWR_STATE_ACK_TIMEOUT,
		.PwrUpWaitTime = XPSMFW_TCM1B_PWR_UP_WAIT_TIME,
	},

	.Id = TCM_1_B,
	.PowerState = STATE_POWER_DEFAULT,
};

static struct XPsmFwGemPwrCtrl_t Gem0PwrCtrl = {
	.GemMemPwrCtrl = {
		.PwrStateMask = PSM_LOCAL_PWR_STATE_GEM0_MASK,
		.ChipEnAddr = PSM_LOCAL_GEM_CE_CNTRL,
		.ChipEnMask = PSM_LOCAL_GEM_CE_CNTRL_GEM0_MASK,
		.PwrCtrlAddr = PSM_LOCAL_GEM_PWR_CNTRL,
		.PwrCtrlMask = PSM_LOCAL_GEM_PWR_CNTRL_GEM0_MASK,
		.PwrStatusAddr = PSM_LOCAL_GEM_PWR_STATUS,
		.PwrStatusMask = PSM_LOCAL_GEM_PWR_STATUS_GEM0_MASK,
		.PwrStateAckTimeout = XPSMFW_GEM0_PWR_STATE_ACK_TIMEOUT,
		.PwrUpWaitTime = XPSMFW_GEM0_PWR_UP_WAIT_TIME,
	},
        .ClkCtrlAddr = CRL_GEM0_REF_CTRL,
        .ClkCtrlMask = CRL_GEM0_REF_CTRL_CLKACT_MASK,
        .RstCtrlAddr = CRL_RST_GEM0,
        .RstCtrlMask = CRL_RST_GEM0_RESET_MASK,
};

static struct XPsmFwGemPwrCtrl_t Gem1PwrCtrl = {
	.GemMemPwrCtrl = {
		.PwrStateMask = PSM_LOCAL_PWR_STATE_GEM1_MASK,
		.ChipEnAddr = PSM_LOCAL_GEM_CE_CNTRL,
		.ChipEnMask = PSM_LOCAL_GEM_CE_CNTRL_GEM1_MASK,
		.PwrCtrlAddr = PSM_LOCAL_GEM_PWR_CNTRL,
		.PwrCtrlMask = PSM_LOCAL_GEM_PWR_CNTRL_GEM1_MASK,
		.PwrStatusAddr = PSM_LOCAL_GEM_PWR_STATUS,
		.PwrStatusMask = PSM_LOCAL_GEM_PWR_STATUS_GEM1_MASK,
		.PwrStateAckTimeout = XPSMFW_GEM1_PWR_STATE_ACK_TIMEOUT,
		.PwrUpWaitTime = XPSMFW_GEM1_PWR_UP_WAIT_TIME,
	},
        .ClkCtrlAddr = CRL_GEM1_REF_CTRL,
        .ClkCtrlMask = CRL_GEM1_REF_CTRL_CLKACT_MASK,
        .RstCtrlAddr = CRL_RST_GEM1,
        .RstCtrlMask = CRL_RST_GEM1_RESET_MASK,
};

static struct XPsmFwMemPwrCtrl_t L2BankPwrCtrl = {
	.PwrStateMask = PSM_LOCAL_PWR_STATE_L2_BANK0_MASK,
	.ChipEnAddr = PSM_LOCAL_L2_CE_CNTRL,
	.ChipEnMask = PSM_LOCAL_L2_CE_CNTRL_BANK0_MASK,
	.PwrCtrlAddr = PSM_LOCAL_L2_PWR_CNTRL,
	.PwrCtrlMask = PSM_LOCAL_L2_PWR_CNTRL_BANK0_MASK,
	.PwrStatusAddr = PSM_LOCAL_L2_PWR_STATUS,
	.PwrStatusMask = PSM_LOCAL_L2_PWR_STATUS_BANK0_MASK,
	.PwrStateAckTimeout = XPSMFW_L2_BANK_PWR_STATE_ACK_TIMEOUT,
	.PwrUpWaitTime = XPSMFW_L2_BANK_PWR_UP_WAIT_TIME,
};

enum XPsmFWPwrUpDwnType {
	XPSMFW_PWR_UPDWN_DIRECT,
	XPSMFW_PWR_UPDWN_REQUEST,
};

void XPsmFw_FpdMbisr(void)
{
	/* Disable Remaining LP-FP isolation */
	XPsmFw_RMW32(PSM_LOCAL_DOMAIN_ISO_CNTRL,
			PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK,
			~PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK);

	/* Release FPD reset */
	XPsmFw_RMW32(CRL_RST_FPD, CRL_RST_FPD_SRST_MASK,
			     ~CRL_RST_FPD_SRST_MASK);

	/* Bisr will now be triggered by XilPM when this call returns */
}

void XPsmFw_FpdMbistClear(void)
{
	/* Disable Remaining LP-FP isolation */
	XPsmFw_RMW32(PSM_LOCAL_DOMAIN_ISO_CNTRL,
			PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK,
			~PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK);

	/* Release FPD reset */
	XPsmFw_RMW32(CRL_RST_FPD, CRL_RST_FPD_SRST_MASK,
			     ~CRL_RST_FPD_SRST_MASK);
}

void XPsmFw_SecLockDown(void)
{
	/* Reset all group isolation */
	XPsmFw_Write32(PSM_LOCAL_DOMAIN_ISO_CNTRL, 0x1FU);
	XPsmFw_Write32(PSM_LOCAL_MISC_CNTRL, 0xFC000000U);
}

XStatus XPsmFw_FpdPreHouseClean(void)
{
	XStatus Status = XST_FAILURE;

	/*
	 * Capture the current Power State
	 * Power up all ACPU Cores and reflect their PWR_STATE
	 */
	LocalPwrState = XPsmFw_Read32(PSM_LOCAL_PWR_STATE);

	/* Update PWR_STATE register to reflect that all ACPUs are powerd up */
	XPsmFw_Write32(PSM_LOCAL_PWR_STATE,
			   (LocalPwrState | PSM_LOCAL_PWR_STATE_ACPU0_MASK |
			   PSM_LOCAL_PWR_STATE_ACPU1_MASK));


	/* Enable alarm associated with VCCINT_FP */
	XPsmFw_Write32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_EN,
			   PSM_GLOBAL_REG_PWR_CTRL_IRQ_EN_FPD_SUPPLY_MASK);

	/* Assert FPD reset */
	XPsmFw_RMW32(CRL_RST_FPD,
			 CRL_RST_FPD_SRST_MASK,
			 CRL_RST_FPD_SRST_MASK);

	/* Disable LP-FP clocking group isolation */
	XPsmFw_RMW32(PSM_LOCAL_DOMAIN_ISO_CNTRL,
			 PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_DFX_MASK,
			 ~PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_DFX_MASK);

	/* Wait until reset is propagated in FPD */
	XPsmFw_UtilWait(XPSMFW_PWRON_RST_FPD_WAIT_TIME);

	Status = XST_SUCCESS;

	return Status;
}

void XPsmFw_FpdPostHouseClean(void)
{
	/*
	 * Disable isolation and release reset in case BISR and MBIST are
	 * skipped
	 */
	XPsmFw_RMW32(PSM_LOCAL_DOMAIN_ISO_CNTRL,
			PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK,
			~PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK);

	XPsmFw_RMW32(CRL_RST_FPD, CRL_RST_FPD_SRST_MASK,
			     ~CRL_RST_FPD_SRST_MASK);

	/* Disable Remaining LP-FP isolation - in case bisr and mbist was skipped */
	XPsmFw_RMW32(PSM_LOCAL_DOMAIN_ISO_CNTRL,
			PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK,
			~PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK);

	/* Check if FPD is already powered up */
	if (!CHECK_BIT(LocalPwrState, PSM_LOCAL_PWR_STATE_FP_MASK)) {
		/* Update the PWR_STATE to reflect the ACPU cores that were powered off */
		XPsmFw_Write32(PSM_LOCAL_PWR_STATE, LocalPwrState);

		/* Mark FPD as powered ON */
		XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, PSM_LOCAL_PWR_STATE_FP_MASK,
			     PSM_LOCAL_PWR_STATE_FP_MASK);
	}
}

static XStatus XPsmFwIslandPwrUp(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 Index;
	u32 Bit = PSM_LOCAL_PWR_CTRL_GATES_SHIFT;

	/* Power up island */
	for (Index = 0; Index < PSM_LOCAL_PWR_CTRL_GATES_WIDTH; Index++) {
		/* Enable this power stage */
		XPsmFw_RMW32(Args->PwrCtrlAddr, ((u32)1U << Bit), ((u32)1U << Bit));

		/* Poll the power stage status */
		Status = XPsmFw_UtilPollForMask(Args->PwrStatusAddr, ((u32)1U << Bit), Args->PwrUpAckTimeout[Index]);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/* Wait for power to ramp up */
		XPsmFw_UtilWait(Args->PwrUpWaitTime[Index]);

		Bit++;
	}

done:
	return Status;
}

static XStatus XPsmFwACPUxPwrUp(struct XPsmFwPwrCtrl_t *Args, enum XPsmFWPwrUpDwnType Type)
{
	XStatus Status = XST_FAILURE;

	/* Warning fix for unused parameter */
	(void)Type;

	Status = XPsmFwIslandPwrUp(Args);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Enable the clock */
	XPsmFw_RMW32(Args->ClkCtrlAddr, Args->ClkCtrlMask, Args->ClkCtrlMask);

	/* Allow the clock to propagate */
	XPsmFw_UtilWait(Args->ClkPropTime);

	/* Disable isolation */
	XPsmFw_RMW32(Args->PwrCtrlAddr, PSM_LOCAL_PWR_CTRL_ISO_MASK, ~PSM_LOCAL_PWR_CTRL_ISO_MASK);

	/* Mask and clear ACPUx requested power-up interrupt request */
	/* This is already handled by common handler so no need to handle here */

	/* Mark ACPUx powered up in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, Args->PwrStateMask, Args->PwrStateMask);

	/* Clear Emulation bit in the LOC_AUX_PWR_STATE */
	XPsmFw_RMW32(PSM_LOCAL_AUX_PWR_STATE,
			(Args->PwrStateMask << PSM_LOCAL_AUX_PWR_STATE_ACPU0_EMUL_SHIFT),
			~(Args->PwrStateMask << PSM_LOCAL_AUX_PWR_STATE_ACPU0_EMUL_SHIFT));

done:
	return Status;
}

static XStatus XPsmFwACPUxDirectPwrUp(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;
	u32 HighAddress, LowAddress;

	/* Power up ACPUx only if powered down */
	RegVal = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_STATE);
	if (!CHECK_BIT(RegVal, Args->PwrStateMask)) {
		Status = XPsmFwACPUxPwrUp(Args, XPSMFW_PWR_UPDWN_DIRECT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set the start address */
	LowAddress = (u32)(PsmToPlmEvent.ResumeAddress[Args->Id] & 0xfffffffeULL);
	HighAddress = (u32)(PsmToPlmEvent.ResumeAddress[Args->Id] >> 32ULL);
	if (0U != (PsmToPlmEvent.ResumeAddress[Args->Id] & 1ULL)) {
		XPsmFw_Write32(Args->ResetCfgAddr, LowAddress);
		XPsmFw_Write32((Args->ResetCfgAddr + 0x4U), HighAddress);
		PsmToPlmEvent.ResumeAddress[Args->Id] = 0U;
	}

	RegVal = XPsmFw_Read32(CRF_RST_APU);
	/* Release L2 cache reset if asserted */
	if (0U != (RegVal & CRF_RST_APU_L2_RESET_MASK)) {
		XPsmFw_RMW32(CRF_RST_APU, CRF_RST_APU_L2_RESET_MASK, 0);
	}

	/* Release POR reset of ACPUx if asserted */
	if ((Args == &Acpu0PwrCtrl) && (0U != (RegVal & CRF_RST_APU_ACPU0_PWRON_MASK))) {
		XPsmFw_RMW32(CRF_RST_APU, CRF_RST_APU_ACPU0_PWRON_MASK, 0);
	} else if ((Args == &Acpu1PwrCtrl) && (0U != (RegVal & CRF_RST_APU_ACPU1_PWRON_MASK))) {
		XPsmFw_RMW32(CRF_RST_APU, CRF_RST_APU_ACPU1_PWRON_MASK, 0);
	} else {
		/* Required by MISRA */
	}

	/* Release reset to ACPUx */
	XPsmFw_RMW32(CRF_RST_APU, Args->RstCtrlMask, ~Args->RstCtrlMask);

	/*
	 * Update the bit associated with ACPUx in the CPUPWRDWNREQ field of
	 * PWRCTL register based on the APU_PWR_STATUS_INIT Register bit.
	 */
	RegVal = XPsmFw_Read32(PSM_GLOBAL_REG_APU_PWR_STATUS_INIT);
	XPsmFw_RMW32(FPD_APU_PWRCTL, Args->PwrStateMask, RegVal);

	/* Clear the bit in the SCU_PWR_STATUS_INIT register */
	XPsmFw_RMW32(PSM_GLOBAL_REG_APU_PWR_STATUS_INIT, Args->PwrStateMask, ~Args->PwrStateMask);

	/* Disable and clear ACPUx direct wake-up interrupt request */
	XPsmFw_Write32(PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS, Args->PwrStateMask);

	/* Enable Direct Power-down Request of ACPUx */
	/* This is already handled in PLM during self-suspend */

	/*
	 * Unmask interrupt for all Power-up Requests and Reset Requests that
	 * are triggered but have their interrupt masked.
	 */
	XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRUP_INT_EN, XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRDWN_STATUS));
	XPsmFw_Write32(PSM_GLOBAL_REG_REQ_SWRST_INT_EN, XPsmFw_Read32(PSM_GLOBAL_REG_REQ_SWRST_STATUS));

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPsmFwACPUxReqPwrUp(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;

	/* Check if already power up */
	RegVal = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_STATE);
	if (CHECK_BIT(RegVal, Args->PwrStateMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	Status = XPsmFwACPUxPwrUp(Args, XPSMFW_PWR_UPDWN_REQUEST);

done:
	return Status;
}

static XStatus XPsmFwIslandPwrDwn(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 IdCodeSubFamily, PlatformType;

	/* Enable isolation */
	XPsmFw_RMW32(Args->PwrCtrlAddr, PSM_LOCAL_PWR_CTRL_ISO_MASK, PSM_LOCAL_PWR_CTRL_ISO_MASK);

	/* EDT-1005580/CR-1070320: De-feature power down of Power Islands */
	PlatformType = XPsmFw_GetPlatform();
	IdCodeSubFamily = XPsmFw_GetIdCode() & PMC_TAP_IDCODE_DEV_SBFMLY_MASK;

	if ((PLATFORM_VERSION_SILICON == PlatformType) &&
	    ((IDCODE_DEV_SBFMLY_VC1902 == IdCodeSubFamily) ||
	     (IDCODE_DEV_SBFMLY_VM1802 == IdCodeSubFamily))) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Disable all power stages */
	XPsmFw_RMW32(Args->PwrCtrlAddr, PSM_LOCAL_PWR_CTRL_GATES_MASK, ~PSM_LOCAL_PWR_CTRL_GATES_MASK);

	/* Poll the power stage status */
	Status = XPsmFw_UtilPollForZero(Args->PwrStatusAddr, PSM_LOCAL_PWR_CTRL_GATES_MASK, Args->PwrDwnAckTimeout);

done:
	return Status;
}

static XStatus XPsmFwACPUxPwrDwn(struct XPsmFwPwrCtrl_t *Args, enum XPsmFWPwrUpDwnType Type)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;

	/* Mark ACPUx powered down in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, Args->PwrStateMask, ~Args->PwrStateMask);

	/* Check for DEBUGNOPWRDWN bit */
	RegVal = XPsmFw_Read32(FPD_APU_PWRSTAT);
	if (0U != (RegVal & FPD_APU_PWRSTAT_DBGNOPWRDWN_MASK)) {
		/* Set Emulation bit in the LOC_AUX_PWR_STATE */
		XPsmFw_RMW32(PSM_LOCAL_AUX_PWR_STATE,
				(Args->PwrStateMask << PSM_LOCAL_AUX_PWR_STATE_ACPU0_EMUL_SHIFT),
				(Args->PwrStateMask << PSM_LOCAL_AUX_PWR_STATE_ACPU0_EMUL_SHIFT));
		Status = XST_SUCCESS;
		goto done;
	}

	if (Type == XPSMFW_PWR_UPDWN_DIRECT) {
		/* Clear CPUPWRDWNREQ field of APU PWRCTL corresponding to ACPUx */
		XPsmFw_RMW32(FPD_APU_PWRCTL, Args->PwrStateMask, ~Args->PwrStateMask);
	}

	/* Disable the clock to the APU core */
	/* As per spec,  if it is not possible to gate clock to individual cores, this step
	 should  be eliminated. This would not cause issues, since ARM core gates clock
	internally when reset is asserted or when it is in the WFI state */
	//XPsmFw_RMW32(Args->ClkCtrlAddr, Args->ClkCtrlMask, ~Args->ClkCtrlMask);

	/*
	 * Assert reset to ACPUx. ARM recommends the island to be put nto reset
	 * before powering down.
	 */
	XPsmFw_RMW32(CRF_RST_APU, Args->RstCtrlMask, Args->RstCtrlMask);

	Status = XPsmFwIslandPwrDwn(Args);
done:
	return Status;
}

static XStatus XPsmFwACPUxDirectPwrDwn(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;

	/* NOTE: As per sequence specs, global power state need to be checked.
	 * But FPD bit in global register shows its isolation status instead of
	 * power status. So keeping check of local reg instead of global reg.
	 */
	/* Check for FPD bit in local PWR_STATE register */
	RegVal = XPsmFw_Read32(PSM_LOCAL_PWR_STATE);
	if (!CHECK_BIT(RegVal, PSM_LOCAL_PWR_STATE_FP_MASK)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Set the bit in the ACU_PWR_STATUS_INIT register */
	XPsmFw_RMW32(PSM_GLOBAL_REG_APU_PWR_STATUS_INIT, Args->PwrStateMask, Args->PwrStateMask);

	/* Power down ACPUx only if powered up */
	RegVal = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_STATE);
	if(CHECK_BIT(RegVal, Args->PwrStateMask)) {
		Status = XPsmFwACPUxPwrDwn(Args, XPSMFW_PWR_UPDWN_DIRECT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Disable and clear ACPUx direct power-down interrupt request */
	XPsmFw_Write32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_STATUS, Args->PwrStateMask);

	/* Clear pending wake interrupt for ACPUx */
	XPsmFw_Write32(PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS, Args->PwrStateMask);

	/* Enable wake interrupt by GIC for ACPUx */
	/* This is already handled in post processing of PLM */

	/*
	 * If FPD bit is set in the REQ_PWRDWN_STATUS register unmask the FPD
	 * interrupt in REQ_PWRDWN_INT_MASK register to allow power-down of
	 * the FPD that follows the APU Core power down.
	 */
	RegVal = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRDWN_STATUS);
	if (CHECK_BIT(RegVal, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_FP_MASK)) {
		XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRDWN_INT_EN, PSM_GLOBAL_REG_REQ_PWRDWN_INT_EN_FP_MASK);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPsmFwACPUxReqPwrDwn(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;

	/* Mask and clear ACPUx requested power-down interrupt request */
	/* This is already handled by common handler so no need to handle here */

	/* NOTE: As per sequence specs, global power state need to be checked.
	 * But FPD bit in global register shows its isolation status instead of
	 * power status. So keeping check of local reg instead of global reg.
	 */
	/* Check for FPD bit in local PWR_STATE register */
	RegVal = XPsmFw_Read32(PSM_LOCAL_PWR_STATE);
	if (!CHECK_BIT(RegVal, PSM_LOCAL_PWR_STATE_FP_MASK)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check if ACPUx is already power down */
	RegVal = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_STATE);
	if(!CHECK_BIT(RegVal, Args->PwrStateMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Clear the bit in the SCU_PWR_STATUS_INIT register */
	XPsmFw_RMW32(PSM_GLOBAL_REG_APU_PWR_STATUS_INIT, Args->PwrStateMask, ~Args->PwrStateMask);

	Status = XPsmFwACPUxPwrDwn(Args, XPSMFW_PWR_UPDWN_REQUEST);
done:
	return Status;
}

static XStatus XPsmFwRPUxPwrUp(struct XPsmFwPwrCtrl_t *Args, enum XPsmFWPwrUpDwnType Type)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;

	/* Warning fix for unused parameter */
	(void)Type;

	Status = XPsmFwIslandPwrUp(Args);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if RPU Comparators have been turned off */
	RegVal = XPsmFw_Read32(RPU_RPU_ERR_INJ);
	if(CHECK_BIT(RegVal, 1U << RPU_RPU_ERR_INJ_DCCMINP_SHIFT) ||
		CHECK_BIT(RegVal, (u32)1U << RPU_RPU_ERR_INJ_DCCMINP2_SHIFT)) {
		XPsmFw_Printf(DEBUG_DETAILED,
		"RPU Register Comparator is on. This may trigger lockstep error\r\n");
	}

	/* Enable the R5 main clock if not enabled */
	XPsmFw_RMW32(Args->ClkCtrlAddr, CRL_CPU_R5_CTRL_CLKACT_MASK,
		     CRL_CPU_R5_CTRL_CLKACT_MASK);

	/* Enable clocks to the RPU */
	XPsmFw_RMW32(Args->ClkCtrlAddr, Args->ClkCtrlMask, Args->ClkCtrlMask);

	/* Allow the clock to propagate */
	XPsmFw_UtilWait(Args->ClkPropTime);

	/* De-assert RPU AMBA RST for comparator logic gates to be operational */
	XPsmFw_RMW32(CRL_RST_CPU_R5, CRL_RST_CPU_R5_RESET_AMBA_MASK, ~CRL_RST_CPU_R5_RESET_AMBA_MASK);

	/* Disable isolation */
	XPsmFw_RMW32(Args->PwrCtrlAddr, PSM_LOCAL_PWR_CTRL_ISO_MASK, ~PSM_LOCAL_PWR_CTRL_ISO_MASK);

done:
	return Status;
}

static XStatus XPsmFwRPUxDirectPwrUp(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 PwrStateMask;
	u32 RegVal;
	u32 LowAddress;

	/* Assert reset to RPU cores */
	XPsmFw_RMW32(CRL_RST_CPU_R5, Args->RstCtrlMask, Args->RstCtrlMask);

	/* Power up RPUx only if powered down */
	RegVal = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_STATE);
	if(!CHECK_BIT(RegVal, Args->PwrStateMask)) {
		Status = XPsmFwRPUxPwrUp(Args, XPSMFW_PWR_UPDWN_DIRECT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (0U != (XPsmFw_Read32(RPU_RPU_GLBL_CNTL) & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK)) {
		PwrStateMask = Rpu0PwrCtrl.PwrStateMask | Rpu1PwrCtrl.PwrStateMask;
	} else {
		PwrStateMask = Args->PwrStateMask;
	}

	/* Mark RPU powered up in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, PwrStateMask, PwrStateMask);

	/* Clear Emulation bit in the LOC_AUX_PWR_STATE */
	XPsmFw_RMW32(PSM_LOCAL_AUX_PWR_STATE,
			PSM_LOCAL_AUX_PWR_STATE_RPU_EMUL_MASK,
			~PSM_LOCAL_AUX_PWR_STATE_RPU_EMUL_MASK);

	/* Set the start address */
	LowAddress = (u32)(PsmToPlmEvent.ResumeAddress[Args->Id] & 0xfffffffeULL);
	if (0U != (PsmToPlmEvent.ResumeAddress[Args->Id] & 1ULL)) {
		if (RPU_HIVEC_ADDR == (LowAddress & RPU_HIVEC_ADDR)) {
			XPsmFw_RMW32(Args->ResetCfgAddr, RPU_VINITHI_MASK,
				     RPU_VINITHI_MASK);
		} else {
			XPsmFw_RMW32(Args->ResetCfgAddr, RPU_VINITHI_MASK, 0U);
		}
		PsmToPlmEvent.ResumeAddress[Args->Id] = 0U;
	}

	/* Release RPU_LS_RESET in the PSM LOCAL_RESET register */
	/* As per EDT-978644, this is already handled in below 2 steps */

	/* Release reset to RPU island */
	XPsmFw_RMW32(CRL_RST_CPU_R5, CRL_RST_CPU_R5_RESET_PGE_MASK, ~CRL_RST_CPU_R5_RESET_PGE_MASK);

	/* Release reset to RPU core */
	XPsmFw_RMW32(CRL_RST_CPU_R5, Args->RstCtrlMask, ~Args->RstCtrlMask);

	/* Disable wake interrupt by GIC for RPUx */
	XPsmFw_Write32(PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS, Args->PwrStateMask >> 6);

	/* Clear pending direct power-down request of RPUx */
	XPsmFw_Write32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_STATUS, Args->PwrStateMask >> 6);
	/* Enable Direct Power-down Request of RPUx */
	XPsmFw_Write32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_EN, Args->PwrStateMask >> 6);

	/*
	 * Unmask interrupt for all Power-up Requests and Reset Requests that
	 * are triggered but have their interrupt masked.
	 */
	XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRUP_INT_EN, XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRDWN_STATUS));
	XPsmFw_Write32(PSM_GLOBAL_REG_REQ_SWRST_INT_EN, XPsmFw_Read32(PSM_GLOBAL_REG_REQ_SWRST_STATUS));

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPsmFwRPUxReqPwrUp(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;

	/* Check if already power up */
	RegVal = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_STATE);
	if (CHECK_BIT(RegVal, Args->PwrStateMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	Status = XPsmFwRPUxPwrUp(Args, XPSMFW_PWR_UPDWN_REQUEST);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Mask and clear RPU requested power-up interrupt request */
	/* This is already handled by common handler so no need to handle here */

	/* Mark RPU powered up in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, Rpu0PwrCtrl.PwrStateMask | Rpu1PwrCtrl.PwrStateMask, Rpu0PwrCtrl.PwrStateMask | Rpu1PwrCtrl.PwrStateMask);

	/* Clear Emulation bit in the LOC_AUX_PWR_STATE */
	XPsmFw_RMW32(PSM_LOCAL_AUX_PWR_STATE,
			PSM_LOCAL_AUX_PWR_STATE_RPU_EMUL_MASK,
			~PSM_LOCAL_AUX_PWR_STATE_RPU_EMUL_MASK);

done:
	return Status;
}

static XStatus XPsmFwRPUxPwrDwn(struct XPsmFwPwrCtrl_t *Args, enum XPsmFWPwrUpDwnType Type)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;

	/* Warning fix for unused parameter */
	(void)Type;

	/* Check for DEBUGNOPWRDWN bit */
	RegVal = XPsmFw_Read32(RPU_RPU_GLBL_STATUS);
	if (0U != (RegVal & RPU_RPU_GLBL_STATUS_DBGNOPWRDWN_MASK)) {
		/* Set Emulation bit in the LOC_AUX_PWR_STATE */
		XPsmFw_RMW32(PSM_LOCAL_AUX_PWR_STATE,
				PSM_LOCAL_AUX_PWR_STATE_RPU_EMUL_MASK,
				PSM_LOCAL_AUX_PWR_STATE_RPU_EMUL_MASK);
		Status = XST_SUCCESS;
		goto done;
	}

	/* Disable clocks to the RPU */
	XPsmFw_RMW32(Args->ClkCtrlAddr, Args->ClkCtrlMask, ~Args->ClkCtrlMask);

	Status = XPsmFwIslandPwrDwn(Args);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Assert reset to RPU island */
	XPsmFw_RMW32(CRL_RST_CPU_R5, CRL_RST_CPU_R5_RESET_PGE_MASK, CRL_RST_CPU_R5_RESET_PGE_MASK);

	/* Assert reset to RPU cores */
	XPsmFw_RMW32(CRL_RST_CPU_R5, Rpu0PwrCtrl.RstCtrlMask | Rpu1PwrCtrl.RstCtrlMask, Rpu0PwrCtrl.RstCtrlMask | Rpu1PwrCtrl.RstCtrlMask);
done:
	return Status;
}

static XStatus XPsmFwRPUxDirectPwrDwn(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 PwrStateMask, OtherCorePowerState, OtherCorePowerStateMask;
	u32 IsLockStep;
	u32 RegVal;

	if (0U != (XPsmFw_Read32(RPU_RPU_GLBL_CNTL) & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK)) {
		IsLockStep = 1U;
		PwrStateMask = Rpu0PwrCtrl.PwrStateMask | Rpu1PwrCtrl.PwrStateMask;
	} else {
		IsLockStep = 0U;
		PwrStateMask = Args->PwrStateMask;
	}

	/* Check for the RPU_0_PWRDWN.EN or RPU_1_PWRDWN.EN */
	if ((0U != IsLockStep) && (RPU0_1 == Args->Id)) {
		RegVal = XPsmFw_Read32(RPU_RPU_1_PWRDWN);
		if (!CHECK_BIT(RegVal, RPU_RPU_1_PWRDWN_EN_MASK)) {
			Status = XST_SUCCESS;
			goto done;
		}
	} else {
		RegVal = XPsmFw_Read32(RPU_RPU_0_PWRDWN);
		if (!CHECK_BIT(RegVal, RPU_RPU_0_PWRDWN_EN_MASK)) {
			Status = XST_SUCCESS;
			goto done;
		}
	}

	/* Mark RPU powered down in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, PwrStateMask, ~PwrStateMask);

	/* Disable and clear RPU direct power-down interrupt request */
	XPsmFw_Write32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_STATUS, Args->PwrStateMask >> 6);

	/* Clear pending wake interrupt for RPUx */
	XPsmFw_Write32(PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS, Args->PwrStateMask >> 6);

	/* Enable wake interrupt by GIC for RPUx */
	XPsmFw_Write32(PSM_GLOBAL_REG_WAKEUP_IRQ_EN, Args->PwrStateMask >> 6);

	/* If R5 is in split mode and other R5 core is powered on, do not power off R5 */
	OtherCorePowerStateMask = Args->PwrStateMask ^ (PSM_LOCAL_PWR_STATE_R5_0_MASK | PSM_LOCAL_PWR_STATE_R5_1_MASK);
	OtherCorePowerState = XPsmFw_Read32(PSM_LOCAL_PWR_STATE) & OtherCorePowerStateMask;
	if ((0U == IsLockStep) && (0U != OtherCorePowerState)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Power down RPUx only if powered up */
	RegVal = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_STATE);
	if (CHECK_BIT(RegVal, Args->PwrStateMask)) {
		Status = XPsmFwRPUxPwrDwn(Args, XPSMFW_PWR_UPDWN_DIRECT);
	} else {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

static XStatus XPsmFwRPUxReqPwrDwn(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;

	/* Check if already power down */
	RegVal = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_STATE);
	if (!CHECK_BIT(RegVal, Args->PwrStateMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Mask and clear RPU requested power-down interrupt request */
	/* This is already handled by common handler so no need to handle here */

	/* Mark RPU powered down in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, Rpu0PwrCtrl.PwrStateMask | Rpu1PwrCtrl.PwrStateMask, ~(Rpu0PwrCtrl.PwrStateMask | Rpu1PwrCtrl.PwrStateMask));

	Status = XPsmFwRPUxPwrDwn(Args, XPSMFW_PWR_UPDWN_REQUEST);

done:
	return Status;
}

static XStatus XPsmFwMemPwrUp(struct XPsmFwMemPwrCtrl_t *Args)
{
	/* HERE */
	XStatus Status = XST_FAILURE;
	u32 RegVal;

	/* Check if already power up */
	RegVal = XPsmFw_Read32(PSM_LOCAL_PWR_STATE);
	if (CHECK_BIT(RegVal, Args->PwrStateMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Enable power state for selected bank */
	XPsmFw_RMW32(Args->PwrCtrlAddr, Args->PwrCtrlMask, Args->PwrCtrlMask);

	/* Poll for power status to set */
	Status = XPsmFw_UtilPollForMask(Args->PwrStatusAddr, Args->PwrStatusMask, Args->PwrStateAckTimeout);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Wait for power to ramp up */
	XPsmFw_UtilWait(Args->PwrUpWaitTime);

	/* Set chip enable bit */
	XPsmFw_RMW32(Args->ChipEnAddr, Args->ChipEnMask, Args->ChipEnMask);

	/* Mask and clear memory power-up interrupt request */
	/* This is already handled by common handler so no need to handle here */

	/* Mark memory bank powered up in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, Args->PwrStateMask, Args->PwrStateMask);

done:
	return Status;
}

static XStatus XPsmFwMemPwrDown(struct XPsmFwMemPwrCtrl_t *Args)
{
	XStatus Status = XST_FAILURE;

	/* Mask and clear memory power-down interrupt request */
	/* This is already handled by common handler so no need to handle here */

	/* Mark memory bank powered down in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, Args->PwrStateMask, ~Args->PwrStateMask);

	/* Clear chip enable bit */
	XPsmFw_RMW32(Args->ChipEnAddr, Args->ChipEnMask, ~Args->ChipEnMask);

	/* Disable power state for selected bank */
	XPsmFw_RMW32(Args->PwrCtrlAddr, Args->PwrCtrlMask, ~Args->PwrCtrlMask);

	if (PLATFORM_VERSION_SILICON != XPsmFw_GetPlatform()) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Poll for power status to clear */
	Status = XPsmFw_UtilPollForZero(Args->PwrStatusAddr, Args->PwrStatusMask, Args->PwrStateAckTimeout);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

static XStatus XTcmPwrUp(struct XPsmTcmPwrCtrl_t *Tcm)
{
	XStatus Status = XST_FAILURE;
	struct XPsmTcmPwrCtrl_t *OtherTcm;

	switch (Tcm->Id) {
	case TCM_0_A:
		OtherTcm = &Tcm0BPwrCtrl;
		break;
	case TCM_0_B:
		OtherTcm = &Tcm0APwrCtrl;
		break;
	case TCM_1_A:
		OtherTcm = &Tcm1BPwrCtrl;
		break;
	case TCM_1_B:
		OtherTcm = &Tcm1APwrCtrl;
		break;
	default:
		OtherTcm = NULL;
		break;
	}

	/* Set power state to on */
	Tcm->PowerState = STATE_POWER_ON;

	/* Check state of OtherTcm. If powered off, power it up */
	if ((NULL != OtherTcm) && (STATE_POWER_DOWN == OtherTcm->PowerState)) {
		OtherTcm->PowerState = STATE_POWER_ON;
		Status = XPsmFwMemPwrUp(&OtherTcm->TcmMemPwrCtrl);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XPsmFwMemPwrUp(&Tcm->TcmMemPwrCtrl);

done:
	return Status;
}

static XStatus XTcmPwrDown(struct XPsmTcmPwrCtrl_t *Tcm)
{
	XStatus Status = XST_FAILURE;
	struct XPsmTcmPwrCtrl_t *OtherTcm;

	switch (Tcm->Id) {
	case TCM_0_A:
		OtherTcm = &Tcm0BPwrCtrl;
		break;
	case TCM_0_B:
		OtherTcm = &Tcm0APwrCtrl;
		break;
	case TCM_1_A:
		OtherTcm = &Tcm1BPwrCtrl;
		break;
	case TCM_1_B:
		OtherTcm = &Tcm1APwrCtrl;
		break;
	default:
		OtherTcm = NULL;
		break;
	}

	Tcm->PowerState = STATE_POWER_DOWN;

	/*
	 * Check state of OtherTcm. If in power off state, power down both banks.
	 * If OtherTcm state is still powered on, do not power down banks and
	 * return success
	 */
	if ((NULL != OtherTcm) && (STATE_POWER_DOWN == OtherTcm->PowerState)) {
		Status = XPsmFwMemPwrDown(&Tcm->TcmMemPwrCtrl);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPsmFwMemPwrDown(&OtherTcm->TcmMemPwrCtrl);
	} else {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

/**
 * PowerUp_OCM_BANK0() - Power up OCM BANK0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_OCM_BANK0(void)
{
	return XPsmFwMemPwrUp(&Ocm0PwrCtrl);
}

/**
 * PowerUp_OCM_BANK1() - Power up OCM BANK1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_OCM_BANK1(void)
{
	return XPsmFwMemPwrUp(&Ocm1PwrCtrl);
}

/**
 * PowerUp_OCM_BANK2() - Power up OCM BANK2
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_OCM_BANK2(void)
{
	return XPsmFwMemPwrUp(&Ocm2PwrCtrl);
}

/**
 * PowerUp_OCM_BANK3() - Power up OCM BANK3
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_OCM_BANK3(void)
{
	return XPsmFwMemPwrUp(&Ocm3PwrCtrl);
}

/**
 * PowerUp_TCM0A() - Power up TCM0A memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_TCM0A(void)
{
	return XTcmPwrUp(&Tcm0APwrCtrl);
}

/**
 * PowerUp_TCM0B() - Power up TCM0B memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_TCM0B(void)
{
	return XTcmPwrUp(&Tcm0BPwrCtrl);
}

/**
 * PowerUp_TCM1A() - Power up TCM1A memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_TCM1A(void)
{
	return XTcmPwrUp(&Tcm1APwrCtrl);
}

/**
 * PowerUp_TCM1B() - Power up TCM1B memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_TCM1B(void)
{
	return XTcmPwrUp(&Tcm1BPwrCtrl);
}

/**
 * PowerUp_L2_BANK0() - Power up L2 BANK0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_L2_BANK0(void)
{
	return XPsmFwMemPwrUp(&L2BankPwrCtrl);
}

/**
 * PowerUp_ACPU0() - Power up ACPU0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_ACPU0(void)
{
	return XPsmFwACPUxReqPwrUp(&Acpu0PwrCtrl);
}

/**
 * PowerUp_ACPU1() - Power up ACPU1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_ACPU1(void)
{
	return XPsmFwACPUxReqPwrUp(&Acpu1PwrCtrl);
}

/**
 * PowerUp_RPU() - Power up RPU
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_RPU(void)
{
	return XPsmFwRPUxReqPwrUp(&Rpu0PwrCtrl);
}

/**
 * PowerUp_GEM0() - Power up GEM0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_GEM0(void)
{
	return XPsmFwMemPwrUp(&Gem0PwrCtrl.GemMemPwrCtrl);
}

/**
 * PowerUp_GEM1() - Power up GEM1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_GEM1(void)
{
	return XPsmFwMemPwrUp(&Gem1PwrCtrl.GemMemPwrCtrl);
}

/**
 * PowerUp_FP() - Power up FPD
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_FP(void)
{
	XStatus Status = XST_FAILURE;

	/* Instead of trigeering this interrupt
	FPD CDO should be reexecuted by XilPM */

	Status = XST_SUCCESS;

	return Status;
}

/**
 * PowerDwn_OCM_BANK0() - Power down OCM BANK0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_OCM_BANK0(void)
{
	/*
	 * As per EDT-995988, Getting the SLV error from power down
	 * island even when Dec error disabled
	 *
	 * OCM gives SLVERR response when a powered-down bank is
	 * accessed, even when Response Error is disabled. Error occurs
	 * only for a narrow access (< 64 bits). Skip OCM power down as
	 * workaround.
	 */
//	return XPsmFwMemPwrDown(&Ocm0PwrCtrl);
	return XST_SUCCESS;
}

/**
 * PowerDwn_OCM_BANK1() - Power down OCM BANK1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_OCM_BANK1(void)
{
	/*
	 * As per EDT-995988, Getting the SLV error from power down
	 * island even when Dec error disabled
	 *
	 * OCM gives SLVERR response when a powered-down bank is
	 * accessed, even when Response Error is disabled. Error occurs
	 * only for a narrow access (< 64 bits). Skip OCM power down as
	 * workaround.
	 */
//	return XPsmFwMemPwrDown(&Ocm1PwrCtrl);
	return XST_SUCCESS;
}

/**
 * PowerDwn_OCM_BANK2() - Power down OCM BANK2
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_OCM_BANK2(void)
{
	/*
	 * As per EDT-995988, Getting the SLV error from power down
	 * island even when Dec error disabled
	 *
	 * OCM gives SLVERR response when a powered-down bank is
	 * accessed, even when Response Error is disabled. Error occurs
	 * only for a narrow access (< 64 bits). Skip OCM power down as
	 * workaround.
	 */
//	return XPsmFwMemPwrDown(&Ocm2PwrCtrl);
	return XST_SUCCESS;
}

/**
 * PowerDwn_OCM_BANK3() - Power down OCM BANK3
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_OCM_BANK3(void)
{
	/*
	 * As per EDT-995988, Getting the SLV error from power down
	 * island even when Dec error disabled
	 *
	 * OCM gives SLVERR response when a powered-down bank is
	 * accessed, even when Response Error is disabled. Error occurs
	 * only for a narrow access (< 64 bits). Skip OCM power down as
	 * workaround.
	 */
//	return XPsmFwMemPwrDown(&Ocm3PwrCtrl);
	return XST_SUCCESS;
}

/**
 * PowerDwn_TCM0A() - Power down TCM0A memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_TCM0A(void)
{
	return XTcmPwrDown(&Tcm0APwrCtrl);
}

/**
 * PowerDwn_TCM0B() - Power down TCM0B memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_TCM0B(void)
{
	return XTcmPwrDown(&Tcm0BPwrCtrl);
}

/**
 * PowerDwn_TCM1A() - Power down TCM1A memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_TCM1A(void)
{
	return XTcmPwrDown(&Tcm1APwrCtrl);
}

/**
 * PowerDwn_TCM1B() - Power down TCM1B memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_TCM1B(void)
{
	return XTcmPwrDown(&Tcm1BPwrCtrl);
}

/**
 * PowerDwn_L2_BANK0() - Power down L2 BANK0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_L2_BANK0(void)
{
	return XPsmFwMemPwrDown(&L2BankPwrCtrl);
}

/**
 * PowerDwn_ACPU0() - Power down ACPU0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_ACPU0(void)
{
	return XPsmFwACPUxReqPwrDwn(&Acpu0PwrCtrl);
}

/**
 * PowerDwn_ACPU1() - Power down ACPU1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_ACPU1(void)
{
	return XPsmFwACPUxReqPwrDwn(&Acpu1PwrCtrl);
}

/**
 * PowerDwn_RPU() - Power down RPU
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_RPU(void)
{
	return XPsmFwRPUxReqPwrDwn(&Rpu0PwrCtrl);
}

/**
 * PowerDwn_GEM0() - Power down GEM0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_GEM0(void)
{
	return XPsmFwMemPwrDown(&Gem0PwrCtrl.GemMemPwrCtrl);
}

/**
 * PowerDwn_GEM1() - Power down GEM1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_GEM1(void)
{
	return XPsmFwMemPwrDown(&Gem1PwrCtrl.GemMemPwrCtrl);
}

/**
 * PowerDwn_FP() - Power down FPD
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_FP(void)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal;

	/* Check if already power down */
	RegVal = XPsmFw_Read32(PSM_LOCAL_PWR_STATE);
	if (CHECK_BIT(RegVal, PSM_LOCAL_PWR_STATE_FP_MASK)) {
		/* Enable isolation between FPD and LPD, PLD */
		XPsmFw_RMW32(PSM_LOCAL_DOMAIN_ISO_CNTRL,
			     PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_DFX_MASK | PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK,
			     PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_DFX_MASK | PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK);

		/* Disable alarms associated with FPD */
		XPsmFw_Write32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_DIS,
			       PSM_GLOBAL_REG_PWR_CTRL_IRQ_DIS_FPD_SUPPLY_MASK);

		/* Clear power down request status */
		/* This is already handled by common handler so no need to handle here */

		/* Mark the FP as Powered Down */
		XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, PSM_LOCAL_PWR_STATE_FP_MASK,
			     ~PSM_LOCAL_PWR_STATE_FP_MASK);
	}

	Status = XST_SUCCESS;

	return Status;
}

/* Structure for power up/down handler table */
static const struct PwrHandlerTable_t PwrUpDwnHandlerTable[] = {
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_FP_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_FP_MASK, PowerUp_FP, PowerDwn_FP},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_GEM0_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_GEM0_MASK, PowerUp_GEM0, PowerDwn_GEM0},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_GEM1_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_GEM1_MASK, PowerUp_GEM1, PowerDwn_GEM1},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_OCM_BANK0_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_OCM_BANK0_MASK, PowerUp_OCM_BANK0, PowerDwn_OCM_BANK0},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_OCM_BANK1_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_OCM_BANK1_MASK, PowerUp_OCM_BANK1, PowerDwn_OCM_BANK1},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_OCM_BANK2_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_OCM_BANK2_MASK, PowerUp_OCM_BANK2, PowerDwn_OCM_BANK2},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_OCM_BANK3_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_OCM_BANK3_MASK, PowerUp_OCM_BANK3, PowerDwn_OCM_BANK3},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_TCM0A_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_TCM0A_MASK, PowerUp_TCM0A, PowerDwn_TCM0A},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_TCM0B_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_TCM0B_MASK, PowerUp_TCM0B, PowerDwn_TCM0B},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_TCM1A_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_TCM1A_MASK, PowerUp_TCM1A, PowerDwn_TCM1A},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_TCM1B_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_TCM1B_MASK, PowerUp_TCM1B, PowerDwn_TCM1B},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_L2_BANK0_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_L2_BANK0_MASK, PowerUp_L2_BANK0, PowerDwn_L2_BANK0},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_RPU_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_RPU_MASK, PowerUp_RPU, PowerDwn_RPU},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_ACPU0_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_ACPU0_MASK, PowerUp_ACPU0, PowerDwn_ACPU0},
	{PSM_GLOBAL_REG_REQ_PWRUP_STATUS_ACPU1_MASK, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_ACPU1_MASK, PowerUp_ACPU1, PowerDwn_ACPU1},
};

/**
 * XPsmFw_DispatchPwrUpHandler() - Power-up interrupt handler
 *
 * @PwrUpStatus    Power Up status register value
 * @PwrUpIntMask   Power Up interrupt mask register value
 *
 * @return         XST_SUCCESS or error code
 */
XStatus XPsmFw_DispatchPwrUpHandler(u32 PwrUpStatus, u32 PwrUpIntMask)
{
	XStatus Status = XST_FAILURE;
	u32 Index;

	for (Index = 0U; Index < ARRAYSIZE(PwrUpDwnHandlerTable); Index++) {
		if ((CHECK_BIT(PwrUpStatus, PwrUpDwnHandlerTable[Index].PwrUpMask)) &&
		    !(CHECK_BIT(PwrUpIntMask, PwrUpDwnHandlerTable[Index].PwrUpMask))) {
			/* Call power up handler */
			Status = PwrUpDwnHandlerTable[Index].PwrUpHandler();

			/* Ack the service */
			XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRUP_STATUS, PwrUpDwnHandlerTable[Index].PwrUpMask);
			XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRUP_INT_DIS, PwrUpDwnHandlerTable[Index].PwrUpMask);
		} else if (CHECK_BIT(PwrUpStatus, PwrUpDwnHandlerTable[Index].PwrUpMask)){
			/* Ack the service if status is 1 but interrupt is not enabled */
			XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRUP_STATUS, PwrUpDwnHandlerTable[Index].PwrUpMask);
			XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRUP_INT_DIS, PwrUpDwnHandlerTable[Index].PwrUpMask);
			Status = XST_SUCCESS;
		} else {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/**
 * XPsmFw_DispatchPwrDwnHandler() - Power-down interrupt handler
 *
 * @PwrDwnStatus   Power Down status register value
 * @pwrDwnIntMask  Power Down interrupt mask register value
 * @PwrUpStatus    Power Up status register value
 * @PwrUpIntMask   Power Up interrupt mask register value
 *
 * @return         XST_SUCCESS or error code
 */
XStatus XPsmFw_DispatchPwrDwnHandler(u32 PwrDwnStatus, u32 pwrDwnIntMask,
		u32 PwrUpStatus, u32 PwrUpIntMask)
{
	XStatus Status = XST_FAILURE;
	u32 Index;

	for (Index = 0U; Index < ARRAYSIZE(PwrUpDwnHandlerTable); Index++) {
		if ((CHECK_BIT(PwrDwnStatus, PwrUpDwnHandlerTable[Index].PwrDwnMask)) &&
		    !(CHECK_BIT(pwrDwnIntMask, PwrUpDwnHandlerTable[Index].PwrDwnMask)) &&
		    !(CHECK_BIT(PwrUpStatus, PwrUpDwnHandlerTable[Index].PwrUpMask)) &&
		    (CHECK_BIT(PwrUpIntMask, PwrUpDwnHandlerTable[Index].PwrUpMask))) {
			/* Call power down handler */
			Status = PwrUpDwnHandlerTable[Index].PwrDwnHandler();

			/* Ack the service */
			XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRDWN_STATUS, PwrUpDwnHandlerTable[Index].PwrDwnMask);
			XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRDWN_INT_DIS, PwrUpDwnHandlerTable[Index].PwrDwnMask);
		} else if (CHECK_BIT(PwrDwnStatus, PwrUpDwnHandlerTable[Index].PwrDwnMask)) {
			/* Ack the service  if power up and power down interrupt arrives simultaneously */
			XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRDWN_STATUS, PwrUpDwnHandlerTable[Index].PwrDwnMask);
			XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRDWN_INT_DIS, PwrUpDwnHandlerTable[Index].PwrDwnMask);
			Status = XST_SUCCESS;
		} else {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/**
 * ACPU0Wakeup() - Wake up ACPU0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus ACPU0Wakeup(void)
{
	XStatus Status = XST_FAILURE;

	/* Check for any pending event */
	assert(PsmToPlmEvent.Event[ACPU_0] == 0U);

	if (1U == PsmToPlmEvent.CpuIdleFlag[ACPU_0]) {
		Status = XPsmFwACPUxDirectPwrUp(&Acpu0PwrCtrl);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set the event bit for PLM */
	PsmToPlmEvent.Event[ACPU_0] = PWR_UP_EVT;
	Status = XPsmFw_NotifyPlmEvent();

done:
	return Status;
}

/**
 * ACPU0Sleep() - Direct power down ACPU0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus ACPU0Sleep(void)
{
	XStatus Status = XST_FAILURE;

	/* Check for any pending event */
	assert(PsmToPlmEvent.Event[ACPU_0] == 0U);

	if (1U == PsmToPlmEvent.CpuIdleFlag[ACPU_0]) {
		Status = XPsmFwACPUxDirectPwrDwn(&Acpu0PwrCtrl);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set the event bit for PLM */
	PsmToPlmEvent.Event[ACPU_0] = PWR_DWN_EVT;
	Status = XPsmFw_NotifyPlmEvent();

done:
	return Status;
}

/**
 * ACPU1Wakeup() - Wake up ACPU1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus ACPU1Wakeup(void)
{
	XStatus Status = XST_FAILURE;

	/* Check for any pending event */
	assert(PsmToPlmEvent.Event[ACPU_1] == 0U);

	if (1U == PsmToPlmEvent.CpuIdleFlag[ACPU_1]) {
		Status = XPsmFwACPUxDirectPwrUp(&Acpu1PwrCtrl);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set the event bit for PLM */
	PsmToPlmEvent.Event[ACPU_1] = PWR_UP_EVT;
	Status = XPsmFw_NotifyPlmEvent();

done:
	return Status;
}

/**
 * ACPU1Sleep() - Direct power down ACPU1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus ACPU1Sleep(void)
{
	XStatus Status = XST_FAILURE;

	/* Check for any pending event */
	assert(PsmToPlmEvent.Event[ACPU_1] == 0U);

	if (1U == PsmToPlmEvent.CpuIdleFlag[ACPU_1]) {
		Status = XPsmFwACPUxDirectPwrDwn(&Acpu1PwrCtrl);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set the event bit for PLM */
	PsmToPlmEvent.Event[ACPU_1] = PWR_DWN_EVT;
	Status = XPsmFw_NotifyPlmEvent();

done:
	return Status;
}

/**
 * R50Wakeup() - Wake up R50
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus R50Wakeup(void)
{
	XStatus Status = XST_FAILURE;

	/* Check for any pending event */
	assert(PsmToPlmEvent.Event[RPU0_0] == 0U);

	if (1U == PsmToPlmEvent.CpuIdleFlag[RPU0_0]) {
		Status = XPsmFwRPUxDirectPwrUp(&Rpu0PwrCtrl);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set the event bit for PLM */
	PsmToPlmEvent.Event[RPU0_0] = PWR_UP_EVT;
	Status = XPsmFw_NotifyPlmEvent();

done:
	return Status;
}

/**
 * R50Sleep() - Direct power down R50
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus R50Sleep(void)
{
	XStatus Status = XST_FAILURE;

	/* Check for any pending event */
	assert(PsmToPlmEvent.Event[RPU0_0] == 0U);

	if (1U == PsmToPlmEvent.CpuIdleFlag[RPU0_0]) {
		Status = XPsmFwRPUxDirectPwrDwn(&Rpu0PwrCtrl);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set the event bit for PLM */
	PsmToPlmEvent.Event[RPU0_0] = PWR_DWN_EVT;
	Status = XPsmFw_NotifyPlmEvent();

done:
	return Status;
}

/**
 * R51Wakeup() - Wake up R51
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus R51Wakeup(void)
{
	XStatus Status = XST_FAILURE;

	/* Check for any pending event */
	assert(PsmToPlmEvent.Event[RPU0_1] == 0U);

	if (1U == PsmToPlmEvent.CpuIdleFlag[RPU0_1]) {
		Status = XPsmFwRPUxDirectPwrUp(&Rpu1PwrCtrl);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set the event bit for PLM */
	PsmToPlmEvent.Event[RPU0_1] = PWR_UP_EVT;
	Status = XPsmFw_NotifyPlmEvent();

done:
	return Status;
}

/**
 * R51Sleep() - Direct power down R51
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus R51Sleep(void)
{
	XStatus Status = XST_FAILURE;

	/* Check for any pending event */
	assert(PsmToPlmEvent.Event[RPU0_1] == 0U);

	if (1U == PsmToPlmEvent.CpuIdleFlag[RPU0_1]) {
		Status = XPsmFwRPUxDirectPwrDwn(&Rpu1PwrCtrl);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set the event bit for PLM */
	PsmToPlmEvent.Event[RPU0_1] = PWR_DWN_EVT;
	Status = XPsmFw_NotifyPlmEvent();

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Direct power down processor
 *
 * @param DeviceId	Device ID of processor
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_DirectPwrDwn(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;

	switch (DeviceId) {
		case XPSMFW_DEV_ACPU_0:
			Status = XPsmFwACPUxDirectPwrDwn(&Acpu0PwrCtrl);
			break;
		case XPSMFW_DEV_ACPU_1:
			Status = XPsmFwACPUxDirectPwrDwn(&Acpu1PwrCtrl);
			break;
		case XPSMFW_DEV_RPU0_0:
			Status = XPsmFwRPUxDirectPwrDwn(&Rpu0PwrCtrl);
			break;
		case XPSMFW_DEV_RPU0_1:
			Status = XPsmFwRPUxDirectPwrDwn(&Rpu1PwrCtrl);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief	Direct power up processor
 *
 * @param DeviceId	Device ID of processor
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_DirectPwrUp(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;

	switch (DeviceId) {
		case XPSMFW_DEV_ACPU_0:
			Status = XPsmFwACPUxDirectPwrUp(&Acpu0PwrCtrl);
			break;
		case XPSMFW_DEV_ACPU_1:
			Status = XPsmFwACPUxDirectPwrUp(&Acpu1PwrCtrl);
			break;
		case XPSMFW_DEV_RPU0_0:
			Status = XPsmFwRPUxDirectPwrUp(&Rpu0PwrCtrl);
			break;
		case XPSMFW_DEV_RPU0_1:
			Status = XPsmFwRPUxDirectPwrUp(&Rpu1PwrCtrl);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

static const struct PwrCtlWakeupHandlerTable_t WakeupHandlerTable[] = {
	{ PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS_ACPU0_MASK, ACPU0Wakeup},
	{ PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS_ACPU1_MASK, ACPU1Wakeup},
	{ PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS_R50_MASK, R50Wakeup},
	{ PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS_R51_MASK, R51Wakeup},
};

static const struct PwrCtlWakeupHandlerTable_t SleepHandlerTable[] = {
	{ PSM_GLOBAL_REG_PWR_CTRL_IRQ_STATUS_ACPU0_MASK, ACPU0Sleep},
	{ PSM_GLOBAL_REG_PWR_CTRL_IRQ_STATUS_ACPU1_MASK, ACPU1Sleep},
	{ PSM_GLOBAL_REG_PWR_CTRL_IRQ_STATUS_R50_MASK, R50Sleep},
	{ PSM_GLOBAL_REG_PWR_CTRL_IRQ_STATUS_R51_MASK, R51Sleep},
};

/**
 * XPsmFw_DispatchWakeupHandler() - Wakeup up interrupt handler
 *
 * @WakeupStatus    Wake Up status register value
 * @WakeupIntMask   Wake Up interrupt mask register value
 *
 * @return         XST_SUCCESS or error code
 */
XStatus XPsmFw_DispatchWakeupHandler(u32 WakeupStatus, u32 WakeupIntMask)
{
	XStatus Status = XST_FAILURE;
	u32 Index;

	for (Index = 0U; Index < ARRAYSIZE(WakeupHandlerTable); Index++) {
		if ((CHECK_BIT(WakeupStatus, WakeupHandlerTable[Index].Mask)) &&
		    !(CHECK_BIT(WakeupIntMask, WakeupHandlerTable[Index].Mask))) {
			/* Call power up handler */
			Status = WakeupHandlerTable[Index].Handler();

			/* Disable wake-up interrupt */
			XPsmFw_Write32(PSM_GLOBAL_REG_WAKEUP_IRQ_DIS, WakeupHandlerTable[Index].Mask);
		} else {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/**
 * XPsmFw_DispatchPwrCtlHandler() - PwrCtl interrupt handler
 *
 * @PwrCtlStatus   Power Down status register value
 * @PwrCtlIntMask  Power Down interrupt mask register value
 *
 * @return         XST_SUCCESS or error code
 */
XStatus XPsmFw_DispatchPwrCtlHandler(u32 PwrCtlStatus, u32 PwrCtlIntMask)
{
	XStatus Status = XST_FAILURE;
	u32 Index;

	for (Index = 0U; Index < ARRAYSIZE(SleepHandlerTable); Index++) {
		if ((CHECK_BIT(PwrCtlStatus, SleepHandlerTable[Index].Mask)) &&
		    !(CHECK_BIT(PwrCtlIntMask, SleepHandlerTable[Index].Mask))) {
			/* Call power up handler */
			Status = SleepHandlerTable[Index].Handler();

			/* Disable direct power-down interrupt */
			XPsmFw_Write32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_DIS, SleepHandlerTable[Index].Mask);
		} else {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/**
 * XPsmFw_GetPsmToPlmEventAddr() - Provides address of
 * PsmToPlmEvent
 *
 * @EventAddr      Buffer pointer to store PsmToPlmEvent
 */
void XPsmFw_GetPsmToPlmEventAddr(u32 *EventAddr)
{
	*EventAddr = (u32)(&PsmToPlmEvent);
}
