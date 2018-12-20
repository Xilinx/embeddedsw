/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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

#include "xpsmfw_default.h"
#include "xpsmfw_power.h"
#include "fpd_apu.h"
#include "psm_global.h"
#include "rpu.h"
#include "crl.h"
#include "crf.h"
#include "pmc_global.h"

#define CHECK_BIT(reg, mask)	((reg & mask) == mask)

static struct XPsmFwPwrCtrl_t Acpu0PwrCtrl = {
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
	.ClkCtrlMask = CRL_CPU_R5_CTRL_CLKACT_MASK,
	.ClkPropTime = XPSMFW_RPU_CTRL_CLK_PROP_TIME,
	.RstCtrlMask = CRL_RST_CPU_R5_RESET_CPU0_MASK,
	.MbistBitMask = PSM_GLOBAL_RPU_MBIST_BIT_MASK,
};

static struct XPsmFwPwrCtrl_t Rpu1PwrCtrl = {
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
	.ClkCtrlMask = CRL_CPU_R5_CTRL_CLKACT_MASK,
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

static struct XPsmFwMemPwrCtrl_t Tcm0APwrCtrl = {
	.PwrStateMask = PSM_LOCAL_PWR_STATE_TCM0A_MASK,
	.ChipEnAddr = PSM_LOCAL_TCM_CE_CNTRL,
	.ChipEnMask = PSM_LOCAL_TCM_CE_CNTRL_TCMA0_MASK,
	.PwrCtrlAddr = PSM_LOCAL_TCM_PWR_CNTRL,
	.PwrCtrlMask = PSM_LOCAL_TCM_PWR_CNTRL_TCMA0_MASK,
	.PwrStatusAddr = PSM_LOCAL_TCM_PWR_STATUS,
	.PwrStatusMask = PSM_LOCAL_TCM_PWR_STATUS_TCMA0_MASK,
	.PwrStateAckTimeout = XPSMFW_TCM0A_PWR_STATE_ACK_TIMEOUT,
	.PwrUpWaitTime = XPSMFW_TCM0A_PWR_UP_WAIT_TIME,
};

static struct XPsmFwMemPwrCtrl_t Tcm0BPwrCtrl = {
	.PwrStateMask = PSM_LOCAL_PWR_STATE_TCM0B_MASK,
	.ChipEnAddr = PSM_LOCAL_TCM_CE_CNTRL,
	.ChipEnMask = PSM_LOCAL_TCM_CE_CNTRL_TCMB0_MASK,
	.PwrCtrlAddr = PSM_LOCAL_TCM_PWR_CNTRL,
	.PwrCtrlMask = PSM_LOCAL_TCM_PWR_CNTRL_TCMB0_MASK,
	.PwrStatusAddr = PSM_LOCAL_TCM_PWR_STATUS,
	.PwrStatusMask = PSM_LOCAL_TCM_PWR_STATUS_TCMB0_MASK,
	.PwrStateAckTimeout = XPSMFW_TCM0B_PWR_STATE_ACK_TIMEOUT,
	.PwrUpWaitTime = XPSMFW_TCM0B_PWR_UP_WAIT_TIME,
};

static struct XPsmFwMemPwrCtrl_t Tcm1APwrCtrl = {
	.PwrStateMask = PSM_LOCAL_PWR_STATE_TCM1A_MASK,
	.ChipEnAddr = PSM_LOCAL_TCM_CE_CNTRL,
	.ChipEnMask = PSM_LOCAL_TCM_CE_CNTRL_TCMA1_MASK,
	.PwrCtrlAddr = PSM_LOCAL_TCM_PWR_CNTRL,
	.PwrCtrlMask = PSM_LOCAL_TCM_PWR_CNTRL_TCMA1_MASK,
	.PwrStatusAddr = PSM_LOCAL_TCM_PWR_STATUS,
	.PwrStatusMask = PSM_LOCAL_TCM_PWR_STATUS_TCMA1_MASK,
	.PwrStateAckTimeout = XPSMFW_TCM1A_PWR_STATE_ACK_TIMEOUT,
	.PwrUpWaitTime = XPSMFW_TCM1A_PWR_UP_WAIT_TIME,
};

static struct XPsmFwMemPwrCtrl_t Tcm1BPwrCtrl = {
	.PwrStateMask = PSM_LOCAL_PWR_STATE_TCM1B_MASK,
	.ChipEnAddr = PSM_LOCAL_TCM_CE_CNTRL,
	.ChipEnMask = PSM_LOCAL_TCM_CE_CNTRL_TCMB1_MASK,
	.PwrCtrlAddr = PSM_LOCAL_TCM_PWR_CNTRL,
	.PwrCtrlMask = PSM_LOCAL_TCM_PWR_CNTRL_TCMB1_MASK,
	.PwrStatusAddr = PSM_LOCAL_TCM_PWR_STATUS,
	.PwrStatusMask = PSM_LOCAL_TCM_PWR_STATUS_TCMB1_MASK,
	.PwrStateAckTimeout = XPSMFW_TCM1B_PWR_STATE_ACK_TIMEOUT,
	.PwrUpWaitTime = XPSMFW_TCM1B_PWR_UP_WAIT_TIME,
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

static XStatus XPsmFwIslandPwrUp(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_SUCCESS;
	u32 Index;
	u32 Bit = PSM_LOCAL_PWR_CTRL_GATES_SHIFT;

	/* Power up island */
	for (Index = 0; Index < PSM_LOCAL_PWR_CTRL_GATES_WIDTH; Index++) {
		/* Enable this power stage */
		XPsmFw_RMW32(Args->PwrCtrlAddr, (1 << Bit), (1 << Bit));

		/* Poll the power stage status */
		Status = XPsmFw_UtilPollForMask(Args->PwrStatusAddr, (1 << Bit), Args->PwrUpAckTimeout[Index]);
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
	XStatus Status = XST_SUCCESS;
#ifdef SPP_HACK
	u32 ErrorCnt = 0;
#endif

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

	/* NOTE: SPP doesn't emulate BISR/BIST */
#ifdef SPP_HACK
	/* Run BISR on ACPUx */

	/* Applying bISR trigger */
	XPsmFw_Write32(FPD_SLCR_WPROT0, 0x0);
	XPsmFw_Write32(FPD_SLCR_BISR_CACHE_CTRL_0, 0x1);

	/* Polling for BISR done and pass status */
	Status = XPsmFw_UtilPollForMask(FPD_SLCR_BISR_CACHE_STATUS, 0x3FF, FPD_SLCR_BISR_CACHE_STATUS_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Initiating odm for ACPUx */
	XPsmFw_Write32(PSM_GLOBAL_REG_MBIST_RSTN, Args->MbistBitMask);
	XPsmFw_Write32(PSM_GLOBAL_REG_MBIST_SETUP, Args->MbistBitMask);
	XPsmFw_Write32(PSM_GLOBAL_REG_MBIST_PG_EN, Args->MbistBitMask);

	/* Polling for DONE status */
	Status = XPsmFw_UtilPollForMask(PSM_GLOBAL_REG_MBIST_DONE, Args->MbistBitMask, PSM_GLOBAL_MBIST_DONE_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Checking for GO status after DONE */
	if (Args->MbistBitMask != XPsmFw_Read32(PSM_GLOBAL_REG_MBIST_GO)) {
		ErrorCnt = ErrorCnt + 1;
		XPsmFw_Write32(LPD_SLCR_PERSISTENT0, ErrorCnt);
	}
	XPsmFw_UtilWait(20);
#endif // SPP_HACK

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
	XStatus Status = XST_SUCCESS;
	u32 RegVal;

	Status = XPsmFwACPUxPwrUp(Args, XPSMFW_PWR_UPDWN_DIRECT);
	if (XST_SUCCESS != Status) {
		goto done;
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
	/* This is already handled by common handler so no need to handle here */

	/* Enable Direct Power-down Request of ACPUx */
	XPsmFw_Write32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_EN, Args->PwrStateMask);

	/*
	 * Unmask interrupt for all Power-up Requests and Reset Requests that
	 * are triggered but have their interrupt masked.
	 */
	XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRUP_INT_EN, XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRDWN_STATUS));
	XPsmFw_Write32(PSM_GLOBAL_REG_REQ_SWRST_INT_EN, XPsmFw_Read32(PSM_GLOBAL_REG_REQ_SWRST_STATUS));
done:
	return Status;
}

static XStatus XPsmFwACPUxReqPwrUp(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_SUCCESS;
	u32 RegVal;

	/* Check if already power up */
	RegVal = XPsmFw_Read32(PSM_LOCAL_PWR_STATE);
	if (CHECK_BIT(RegVal, Args->PwrStateMask)) {
		goto done;
	}

	Status = XPsmFwACPUxPwrUp(Args, XPSMFW_PWR_UPDWN_REQUEST);
done:
	return Status;
}

static XStatus XPsmFwIslandPwrDwn(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status;

	/* Enable isolation */
	XPsmFw_RMW32(Args->PwrCtrlAddr, PSM_LOCAL_PWR_CTRL_ISO_MASK, PSM_LOCAL_PWR_CTRL_ISO_MASK);

	/* Disable all power stages */
	XPsmFw_RMW32(Args->PwrCtrlAddr, PSM_LOCAL_PWR_CTRL_GATES_MASK, ~PSM_LOCAL_PWR_CTRL_GATES_MASK);

	/* Poll the power stage status */
	Status = XPsmFw_UtilPollForZero(Args->PwrStatusAddr, PSM_LOCAL_PWR_CTRL_GATES_MASK, Args->PwrDwnAckTimeout);

	return Status;
}

static XStatus XPsmFwACPUxPwrDwn(struct XPsmFwPwrCtrl_t *Args, enum XPsmFWPwrUpDwnType Type)
{
	XStatus Status = XST_SUCCESS;
	u32 RegVal;

	/* Mark ACPUx powered down in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, Args->PwrStateMask, ~Args->PwrStateMask);

	/* Check for DEBUGNOPWRDWN bit */
	RegVal = XPsmFw_Read32(FPD_APU_PWRSTAT);
	if ((RegVal & FPD_APU_PWRSTAT_DBGNOPWRDWN_MASK) != 0) {
		/* Set Emulation bit in the LOC_AUX_PWR_STATE */
		XPsmFw_RMW32(PSM_LOCAL_AUX_PWR_STATE,
				(Args->PwrStateMask << PSM_LOCAL_AUX_PWR_STATE_ACPU0_EMUL_SHIFT),
				(Args->PwrStateMask << PSM_LOCAL_AUX_PWR_STATE_ACPU0_EMUL_SHIFT));
		goto done;
	}

	if (Type == XPSMFW_PWR_UPDWN_DIRECT) {
		/* Clear CPUPWRDWNREQ field of APU PWRCTL corresponding to ACPUx */
		XPsmFw_RMW32(FPD_APU_PWRCTL, Args->PwrStateMask, ~Args->PwrStateMask);
	}

	/* Disable the clock to the APU core */
	XPsmFw_RMW32(Args->ClkCtrlAddr, Args->ClkCtrlMask, ~Args->ClkCtrlMask);

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
	XStatus Status = XST_SUCCESS;
	u32 RegVal;

	/* NOTE: As per sequence specs, global power state need to be checked.
	 * But FPD bit in global register shows its isolation status instead of
	 * power status. So keeping check of local reg instead of global reg.
	 */
	/* Check for FPD bit in local PWR_STATE register */
	RegVal = XPsmFw_Read32(PSM_LOCAL_PWR_STATE);
	if (!CHECK_BIT(RegVal, PSM_LOCAL_PWR_STATE_FP_MASK)) {
		goto done;
	}

	/* Set the bit in the ACU_PWR_STATUS_INIT register */
	XPsmFw_RMW32(PSM_GLOBAL_REG_APU_PWR_STATUS_INIT, Args->PwrStateMask, Args->PwrStateMask);

	Status = XPsmFwACPUxPwrDwn(Args, XPSMFW_PWR_UPDWN_DIRECT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Disable and clear ACPUx direct power-down interrupt request */
	/* This is already handled by common handler so no need to handle here */

	/* Enable wake interrupt by GIC for ACPUx */
	XPsmFw_Write32(PSM_GLOBAL_REG_WAKEUP_IRQ_EN, Args->PwrStateMask);

	/*
	 * If FPD bit is set in the REQ_PWRDWN_STATUS register unmask the FPD
	 * interrupt in REQ_PWRDWN_INT_MASK register to allow power-down of
	 * the FPD that follows the APU Core power down.
	 */
	RegVal = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRDWN_STATUS);
	if (CHECK_BIT(RegVal, PSM_GLOBAL_REG_REQ_PWRDWN_STATUS_FP_MASK)) {
		XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRDWN_INT_EN, PSM_GLOBAL_REG_REQ_PWRDWN_INT_EN_FP_MASK);
	}
done:
	return Status;
}

static XStatus XPsmFwACPUxReqPwrDwn(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_SUCCESS;
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
	XStatus Status = XST_SUCCESS;
#ifdef SPP_HACK
	u32 ErrorCnt = 0;
#endif

	Status = XPsmFwIslandPwrUp(Args);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (Type == XPSMFW_PWR_UPDWN_REQUEST) {
		/* Enable the clock to the R5 and RPU core */
		XPsmFw_RMW32(Args->ClkCtrlAddr, Args->ClkCtrlMask | CRL_CPU_R5_CTRL_CLKACT_CORE_MASK, Args->ClkCtrlMask | CRL_CPU_R5_CTRL_CLKACT_CORE_MASK);
	} else {
		/* Enable clocks to the RPU */
		XPsmFw_RMW32(Args->ClkCtrlAddr, Args->ClkCtrlMask, Args->ClkCtrlMask);
	}

	/* Allow the clock to propagate */
	XPsmFw_UtilWait(Args->ClkPropTime);

	/* Disable isolation */
	XPsmFw_RMW32(Args->PwrCtrlAddr, PSM_LOCAL_PWR_CTRL_ISO_MASK, ~PSM_LOCAL_PWR_CTRL_ISO_MASK);

	/* NOTE: SPP doesn't emulate BISR/BIST */
#ifdef SPP_HACK
	/* Run BISR on RPU */

	/* Applying BISR trigger */
	XPsmFw_Write32(LPD_SLCR_BISR_CACHE_CTRL_0, 0x1);

	/* Polling for BISR done and pass status */
	Status = XPsmFw_UtilPollForMask(LPD_SLCR_BISR_CACHE_STATUS, 0xC000000F, LPD_SLCR_BISR_CACHE_STATUS_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Initiating odm for RPU */
	XPsmFw_Write32(PMC_ANALOG_OD_MBIST_RST, Args->MbistBitMask);
	XPsmFw_Write32(PMC_ANALOG_OD_MBIST_SETUP, Args->MbistBitMask);
	XPsmFw_Write32(PMC_ANALOG_OD_MBIST_PG_EN, Args->MbistBitMask);

	/* Polling for DONE status */
	Status = XPsmFw_UtilPollForMask(PMC_ANALOG_OD_MBIST_DONE, Args->MbistBitMask, PMC_ANALOG_MBIST_DONE_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Checking for GO status after DONE */
	if (Args->MbistBitMask != XPsmFw_Read32(PMC_ANALOG_OD_MBIST_GOOD)) {
		ErrorCnt = ErrorCnt + 1;
		XPsmFw_Write32(LPD_SLCR_PERSISTENT0, ErrorCnt);
	}
	XPsmFw_UtilWait(20);
#endif // SPP_HACK
done:
	return Status;
}

static XStatus XPsmFwRPUxDirectPwrUp(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_SUCCESS;
	u32 PwrStateMask, IsLockStep;

	/* Assert reset to RPU cores */
	XPsmFw_RMW32(CRL_RST_CPU_R5, Args->RstCtrlMask, Args->RstCtrlMask);

	Status = XPsmFwRPUxPwrUp(Args, XPSMFW_PWR_UPDWN_DIRECT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*
	 * Clear the bit in RPU_PWRDWN_EN register to transfer the interrupts
	 * back to the R5.
	 */
	if (Args == &Rpu0PwrCtrl) {
		XPsmFw_RMW32(RPU_RPU_0_PWRDWN, RPU_RPU_0_PWRDWN_EN_MASK, ~RPU_RPU_0_PWRDWN_EN_MASK);
	} else {
		XPsmFw_RMW32(RPU_RPU_1_PWRDWN, RPU_RPU_1_PWRDWN_EN_MASK, ~RPU_RPU_1_PWRDWN_EN_MASK);
	}

	IsLockStep = !(XPsmFw_Read32(RPU_RPU_GLBL_CNTL) & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK);
	if (IsLockStep) {
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

	/* Release RPU_LS_RESET in the PSM LOCAL_RESET register */
	/* As per EDT-978644, this is already handled in below 2 steps */

	/* Release reset to RPU island */
	XPsmFw_RMW32(CRL_RST_CPU_R5, CRL_RST_CPU_R5_RESET_PGE_MASK, ~CRL_RST_CPU_R5_RESET_PGE_MASK);

	/* Release reset to RPU core */
	XPsmFw_RMW32(CRL_RST_CPU_R5, Args->RstCtrlMask, ~Args->RstCtrlMask);

	/* Disable wake interrupt by GIC for RPUx */
	/* This is already handled by common handler so no need to handle here */

	/* Enable Direct Power-down Request of RPUx */
	XPsmFw_Write32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_EN, Args->PwrStateMask >> 6);

	/*
	 * Unmask interrupt for all Power-up Requests and Reset Requests that
	 * are triggered but have their interrupt masked.
	 */
	XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRUP_INT_EN, XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRDWN_STATUS));
	XPsmFw_Write32(PSM_GLOBAL_REG_REQ_SWRST_INT_EN, XPsmFw_Read32(PSM_GLOBAL_REG_REQ_SWRST_STATUS));
done:
	return Status;
}

static XStatus XPsmFwRPUxReqPwrUp(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_SUCCESS;
	u32 RegVal;

	/* Check if already power up */
	RegVal = XPsmFw_Read32(PSM_LOCAL_PWR_STATE);
	if (CHECK_BIT(RegVal, Args->PwrStateMask)) {
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
	XStatus Status = XST_SUCCESS;
	u32 RegVal;

	/* Check for DEBUGNOPWRDWN bit */
	RegVal = XPsmFw_Read32(RPU_RPU_GLBL_STATUS);
	if (RegVal & RPU_RPU_GLBL_STATUS_DBGNOPWRDWN_MASK) {
		/* Set Emulation bit in the LOC_AUX_PWR_STATE */
		XPsmFw_RMW32(PSM_LOCAL_AUX_PWR_STATE,
				PSM_LOCAL_AUX_PWR_STATE_RPU_EMUL_MASK,
				PSM_LOCAL_AUX_PWR_STATE_RPU_EMUL_MASK);
		goto done;
	}

	if (Type == XPSMFW_PWR_UPDWN_REQUEST) {
		/* Disable the clock to the R5 and RPU core */
		XPsmFw_RMW32(Args->ClkCtrlAddr, Args->ClkCtrlMask | CRL_CPU_R5_CTRL_CLKACT_CORE_MASK, ~(Args->ClkCtrlMask | CRL_CPU_R5_CTRL_CLKACT_CORE_MASK));
	} else {
		/* Disable clocks to the RPU */
		XPsmFw_RMW32(Args->ClkCtrlAddr, Args->ClkCtrlMask, ~Args->ClkCtrlMask);
	}

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
	XStatus Status = XST_SUCCESS;
	u32 PwrStateMask, OtherCorePowerState, OtherCorePowerStateMask;
	u32 IsLockStep;

	IsLockStep = !(XPsmFw_Read32(RPU_RPU_GLBL_CNTL) & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK);
	if (IsLockStep) {
		PwrStateMask = Rpu0PwrCtrl.PwrStateMask | Rpu1PwrCtrl.PwrStateMask;
	} else {
		PwrStateMask = Args->PwrStateMask;
	}
	/* Mark RPU powered down in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, PwrStateMask, ~PwrStateMask);

	/* Disable and clear RPU direct power-down interrupt request */
	/* This is already handled by common handler so no need to handle here */

	/* Enable wake interrupt by GIC for RPUx */
	XPsmFw_Write32(PSM_GLOBAL_REG_WAKEUP_IRQ_EN, Args->PwrStateMask >> 6);

	/* If R5 is in split mode and other R5 core is powered on, do not power off R5 */
	OtherCorePowerStateMask = Args->PwrStateMask ^ (PSM_LOCAL_PWR_STATE_R5_0_MASK | PSM_LOCAL_PWR_STATE_R5_1_MASK);
	OtherCorePowerState = XPsmFw_Read32(PSM_LOCAL_PWR_STATE) & OtherCorePowerStateMask;
	if (!IsLockStep && OtherCorePowerState) {
		goto done;
	}

	Status = XPsmFwRPUxPwrDwn(Args, XPSMFW_PWR_UPDWN_DIRECT);
done:
	return Status;
}

static XStatus XPsmFwRPUxReqPwrDwn(struct XPsmFwPwrCtrl_t *Args)
{
	XStatus Status = XST_SUCCESS;

	/* Mask and clear RPU requested power-down interrupt request */
	/* This is already handled by common handler so no need to handle here */

	/* Mark RPU powered down in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, Rpu0PwrCtrl.PwrStateMask | Rpu1PwrCtrl.PwrStateMask, ~(Rpu0PwrCtrl.PwrStateMask | Rpu1PwrCtrl.PwrStateMask));

	Status = XPsmFwRPUxPwrDwn(Args, XPSMFW_PWR_UPDWN_REQUEST);

	return Status;
}

static XStatus XPsmFwMemPwrUp(struct XPsmFwMemPwrCtrl_t *Args)
{
	XStatus Status = XST_SUCCESS;
	u32 RegVal;

	/* Check if already power up */
	RegVal = XPsmFw_Read32(PSM_LOCAL_PWR_STATE);
	if (CHECK_BIT(RegVal, Args->PwrStateMask)) {
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
	XStatus Status = XST_SUCCESS;

	/* Mask and clear memory power-down interrupt request */
	/* This is already handled by common handler so no need to handle here */

	/* Mark memory bank powered down in LOCAL_PWR_STATUS register */
	XPsmFw_RMW32(PSM_LOCAL_PWR_STATE, Args->PwrStateMask, ~Args->PwrStateMask);

	/* Clear chip enable bit */
	XPsmFw_RMW32(Args->ChipEnAddr, Args->ChipEnMask, ~Args->ChipEnMask);

	/* Disable power state for selected bank */
	XPsmFw_RMW32(Args->PwrCtrlAddr, Args->PwrCtrlMask, ~Args->PwrCtrlMask);

	/* Poll for power status to clear */
	Status = XPsmFw_UtilPollForZero(Args->PwrStatusAddr, Args->PwrStatusMask, Args->PwrStateAckTimeout);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

/**
 * XPsmFwGemDisable() - Disable GEM
 *
 * @return    XST_SUCCESS or error code
 */
static void XPsmFwGemDisable(struct XPsmFwGemPwrCtrl_t *Args)
{
	/* Assert reset to selected GEM core */
        XPsmFw_RMW32(Args->RstCtrlAddr, Args->RstCtrlMask, Args->RstCtrlMask);

	/* Disable clock for selected GEM core */
        XPsmFw_RMW32(Args->ClkCtrlAddr, Args->ClkCtrlMask, ~Args->ClkCtrlMask);
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
	return XPsmFwMemPwrUp(&Tcm0APwrCtrl);
}

/**
 * PowerUp_TCM0B() - Power up TCM0B memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_TCM0B(void)
{
	return XPsmFwMemPwrUp(&Tcm0BPwrCtrl);
}

/**
 * PowerUp_TCM1A() - Power up TCM1A memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_TCM1A(void)
{
	return XPsmFwMemPwrUp(&Tcm1APwrCtrl);
}

/**
 * PowerUp_TCM1B() - Power up TCM1B memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerUp_TCM1B(void)
{
	return XPsmFwMemPwrUp(&Tcm1BPwrCtrl);
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
	XStatus Status = XST_SUCCESS;
	u32 RegVal, PwrState;

	/* NOTE: As per sequence specs, global power state need to be checked.
	 * But FPD bit in global register shows its isolation status instead of
	 * power status. So keeping check of local reg instead of global reg.
	 */
	/* Check if FPD is already powered up */
	RegVal = XPsmFw_Read32(PSM_LOCAL_PWR_STATE);
	if (!CHECK_BIT(RegVal, PSM_LOCAL_PWR_STATE_FP_MASK)) {

		/* TODO: Disable PSM interrupts */

		/*
		 * Capture the current Power State
		 * Power up all ACPU Cores and reflect their PWR_STATE
		 */
		PwrState = XPsmFw_Read32(PSM_LOCAL_PWR_STATE);

		/*
		 * Power up ACPUx power islands.
		 * Remove physical isolation.
		 */
		XPsmFw_Write32(PSM_LOCAL_ACPU0_PWR_CNTRL, 0x0000000F);
		XPsmFw_Write32(PSM_LOCAL_ACPU1_PWR_CNTRL, 0x0000000F);

		/* Update PWR_STATE register to reflect that all 4 ACPUs are powerd up */
		XPsmFw_Write32(PSM_LOCAL_PWR_STATE,
			       (PwrState | PSM_LOCAL_PWR_STATE_ACPU0_MASK |
			       PSM_LOCAL_PWR_STATE_ACPU1_MASK));

		/* TODO: Request PMC to power up VCCINT_FP rail and wait for the acknowledgement.*/

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

		/* Wait until reset is propogated in FPD */
		XPsmFw_UtilWait(XPSMFW_PWRON_RST_FPD_WAIT_TIME);

		/* TODO: Run FPD and ACPU BISR engines */

		/* Check the Saved PWR_STATE in Stage 3 and power off any ACPU cores that were off */
		if ((PwrState & PSM_GLOBAL_REG_PWR_STATE_ACPU0_MASK))
			XPsmFw_Write32(PSM_LOCAL_ACPU0_PWR_CNTRL,
				       PSM_LOCAL_ACPU0_PWR_CNTRL_ISOLATION_MASK);
		if ((PwrState & PSM_GLOBAL_REG_PWR_STATE_ACPU1_MASK))
			XPsmFw_Write32(PSM_LOCAL_ACPU1_PWR_CNTRL,
				       PSM_LOCAL_ACPU1_PWR_CNTRL_ISOLATION_MASK);

		/* Update the PWR_STATE to reflect the ACPU cores that were powered off */
		XPsmFw_Write32(PSM_LOCAL_PWR_STATE, PwrState);

		/* Disable Remaining LP-FP isolation */
		XPsmFw_RMW32(PSM_LOCAL_DOMAIN_ISO_CNTRL,
			     PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK,
			     ~PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK);

		/* Disable FP-PL isolation if PL is on */
		RegVal = XPsmFw_Read32(PMC_GLOBAL_PWR_SUPPLY_STATUS);
		if (CHECK_BIT(RegVal, PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK)) {
			XPsmFw_RMW32(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
				     PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_MASK,
				     ~PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_MASK);

			XPsmFw_RMW32(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
				     PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_TEST_MASK,
				     ~PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_TEST_MASK);
		}

		/* Release FPD reset */
		XPsmFw_RMW32(CRL_RST_FPD,
			     CRL_RST_FPD_SRST_MASK,
			     ~CRL_RST_FPD_SRST_MASK);


		/* Clear Request Status bit in the REQ_PWRUP_STATUS register */
		/* This is already handled by common handler so no need to handle here */

		/* TODO: Enable PSM Interrupts */
	}
	return Status;
}

/**
 * PowerDwn_OCM_BANK0() - Power down OCM BANK0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_OCM_BANK0(void)
{
	return XPsmFwMemPwrDown(&Ocm0PwrCtrl);
}

/**
 * PowerDwn_OCM_BANK1() - Power down OCM BANK1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_OCM_BANK1(void)
{
	return XPsmFwMemPwrDown(&Ocm1PwrCtrl);
}

/**
 * PowerDwn_OCM_BANK2() - Power down OCM BANK2
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_OCM_BANK2(void)
{
	return XPsmFwMemPwrDown(&Ocm2PwrCtrl);
}

/**
 * PowerDwn_OCM_BANK3() - Power down OCM BANK3
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_OCM_BANK3(void)
{
	return XPsmFwMemPwrDown(&Ocm3PwrCtrl);
}

/**
 * PowerDwn_TCM0A() - Power down TCM0A memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_TCM0A(void)
{
	return XPsmFwMemPwrDown(&Tcm0APwrCtrl);
}

/**
 * PowerDwn_TCM0B() - Power down TCM0B memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_TCM0B(void)
{
	return XPsmFwMemPwrDown(&Tcm0BPwrCtrl);
}

/**
 * PowerDwn_TCM1A() - Power down TCM1A memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_TCM1A(void)
{
	return XPsmFwMemPwrDown(&Tcm1APwrCtrl);
}

/**
 * PowerDwn_TCM1B() - Power down TCM1B memory
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_TCM1B(void)
{
	return XPsmFwMemPwrDown(&Tcm1BPwrCtrl);
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
	XStatus Status = XST_SUCCESS;

	Status = XPsmFwMemPwrDown(&Gem0PwrCtrl.GemMemPwrCtrl);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XPsmFwGemDisable(&Gem0PwrCtrl);

done:
	return Status;
}

/**
 * PowerDwn_GEM1() - Power down GEM1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_GEM1(void)
{
	XStatus Status = XST_SUCCESS;

	Status = XPsmFwMemPwrDown(&Gem1PwrCtrl.GemMemPwrCtrl);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XPsmFwGemDisable(&Gem1PwrCtrl);

done:
	return Status;
}

/**
 * PowerDwn_FP() - Power down FPD
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus PowerDwn_FP(void)
{
	XStatus Status = XST_SUCCESS;
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
	return Status;
}

/* Structure for power up/down handler table */
static struct PwrHandlerTable_t PwrUpDwnHandlerTable[] = {
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
	XStatus Status = XST_SUCCESS;
	u32 Index;

	for (Index = 0U; Index < ARRAYSIZE(PwrUpDwnHandlerTable); Index++) {
		if ( (CHECK_BIT(PwrUpStatus, PwrUpDwnHandlerTable[Index].PwrUpMask)) &
				(!CHECK_BIT(PwrUpIntMask, PwrUpDwnHandlerTable[Index].PwrUpMask)) ) {
			/* Call power up handler */
			Status = PwrUpDwnHandlerTable[Index].PwrUpHandler();
		}

		/* Ack the service */
		XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRUP_STATUS, PwrUpDwnHandlerTable[Index].PwrUpMask);
		XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRUP_INT_DIS, PwrUpDwnHandlerTable[Index].PwrUpMask);
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
	XStatus Status = XST_SUCCESS;
	u32 Index;

	for (Index = 0U; Index < ARRAYSIZE(PwrUpDwnHandlerTable); Index++) {
		if ( (CHECK_BIT(PwrDwnStatus, PwrUpDwnHandlerTable[Index].PwrDwnMask)) &
				(!CHECK_BIT(pwrDwnIntMask, PwrUpDwnHandlerTable[Index].PwrDwnMask)) &
				(!CHECK_BIT(PwrUpStatus, PwrUpDwnHandlerTable[Index].PwrUpMask)) &
				(CHECK_BIT(PwrUpIntMask, PwrUpDwnHandlerTable[Index].PwrUpMask)) ) {
			/* Call power down handler */
			Status = PwrUpDwnHandlerTable[Index].PwrDwnHandler();
		}

		/* Ack the service */
		XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRDWN_STATUS, PwrUpDwnHandlerTable[Index].PwrDwnMask);
		XPsmFw_Write32(PSM_GLOBAL_REG_REQ_PWRDWN_INT_DIS, PwrUpDwnHandlerTable[Index].PwrDwnMask);
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
	return XPsmFwACPUxDirectPwrUp(&Acpu0PwrCtrl);
}

/**
 * ACPU0Sleep() - Direct power down ACPU0
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus ACPU0Sleep(void)
{
	return XPsmFwACPUxDirectPwrDwn(&Acpu0PwrCtrl);
}

/**
 * ACPU1Wakeup() - Wake up ACPU1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus ACPU1Wakeup(void)
{
	return XPsmFwACPUxDirectPwrUp(&Acpu1PwrCtrl);
}

/**
 * ACPU1Sleep() - Direct power down ACPU1
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus ACPU1Sleep(void)
{
	return XPsmFwACPUxDirectPwrDwn(&Acpu1PwrCtrl);
}

/**
 * R50Wakeup() - Wake up R50
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus R50Wakeup(void)
{
	return XPsmFwRPUxDirectPwrUp(&Rpu0PwrCtrl);
}

/**
 * R50Sleep() - Direct power down R50
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus R50Sleep(void)
{
	return XPsmFwRPUxDirectPwrDwn(&Rpu0PwrCtrl);
}

/**
 * R51Wakeup() - Wake up R51
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus R51Wakeup(void)
{
	return XPsmFwRPUxDirectPwrUp(&Rpu1PwrCtrl);
}

/**
 * R51Sleep() - Direct power down R51
 *
 * @return    XST_SUCCESS or error code
 */
static XStatus R51Sleep(void)
{
	return XPsmFwRPUxDirectPwrDwn(&Rpu1PwrCtrl);
}

static struct PwrCtlWakeupHandlerTable_t WakeupHandlerTable[] = {
	{ PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS_ACPU0_MASK, ACPU0Wakeup},
	{ PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS_ACPU1_MASK, ACPU1Wakeup},
	{ PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS_R50_MASK, R50Wakeup},
	{ PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS_R51_MASK, R51Wakeup},
};

static struct PwrCtlWakeupHandlerTable_t SleepHandlerTable[] = {
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
	XStatus Status = XST_SUCCESS;
	u32 Index;

	for (Index = 0U; Index < ARRAYSIZE(WakeupHandlerTable); Index++) {
		if ( (CHECK_BIT(WakeupStatus, WakeupHandlerTable[Index].Mask)) &
				(!CHECK_BIT(WakeupIntMask, WakeupHandlerTable[Index].Mask)) ) {
			/* Call power up handler */
			Status = WakeupHandlerTable[Index].Handler();
		}

		/* Ack the service */
		XPsmFw_Write32(PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS, WakeupHandlerTable[Index].Mask);
		XPsmFw_Write32(PSM_GLOBAL_REG_WAKEUP_IRQ_DIS, WakeupHandlerTable[Index].Mask);
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
	XStatus Status = XST_SUCCESS;
	u32 Index;

	for (Index = 0U; Index < ARRAYSIZE(SleepHandlerTable); Index++) {
		if ( (CHECK_BIT(PwrCtlStatus, SleepHandlerTable[Index].Mask)) &
				(!CHECK_BIT(PwrCtlIntMask, SleepHandlerTable[Index].Mask)) ) {
			/* Call power up handler */
			Status = SleepHandlerTable[Index].Handler();
		}

		/* Ack the service */
		XPsmFw_Write32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_STATUS, SleepHandlerTable[Index].Mask);
		XPsmFw_Write32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_DIS, SleepHandlerTable[Index].Mask);
	}

	return Status;
}
