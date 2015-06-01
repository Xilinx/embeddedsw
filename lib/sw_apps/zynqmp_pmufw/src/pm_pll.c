/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

/*********************************************************************
 * Contains:
 * - PLL slave implementation
 * - PLL slave's FSM implementation - only tracks the PLL usage status
 *   (used/unused).
 * - Functions for saving and restoring PLLs' context
 *
 * Note: PMU does not control states of PLLs. When none of FPD PLLs
 * is used and FPD is going to be powered down, PMU saves context of
 * PLLs in FPD and asserts their reset. After powering up FPD, PMU
 * restores the state of PLL based on saved context only when PLL is
 * needed for the use.
 *********************************************************************/

#include "pm_pll.h"
#include "pm_master.h"
#include "pm_power.h"
#include "crf_apb.h"
#include "crl_apb.h"
#include "xpfw_util.h"

/* PLL states */
static const u32 pmPllStates[PM_PLL_STATE_MAX] = {
	[PM_PLL_STATE_UNUSED] = 0U,
	[PM_PLL_STATE_USED] = PM_CAP_ACCESS | PM_CAP_POWER,
};

/* PLL transition table (from which to which state PLL can transit) */
static const PmStateTran pmPllTransitions[] = {
	{
		.fromState = PM_PLL_STATE_USED,
		.toState = PM_PLL_STATE_UNUSED,
	}, {
		.fromState = PM_PLL_STATE_UNUSED,
		.toState = PM_PLL_STATE_USED,
	},
};

/**
 * PmPllBypassAndReset() - Bypass and reset/power down a PLL
 * @pll Pointer to a Pll to be bypassed/reset
 */
static void PmPllBypassAndReset(PmSlavePll* const pll)
{
	/* Bypass PLL before putting it into the reset */
	XPfw_RMW32(pll->addr + PM_PLL_CTRL_OFFSET, PM_PLL_CTRL_BYPASS_MASK,
		   PM_PLL_CTRL_BYPASS_MASK);

	/* Power down PLL (= reset PLL) */
	XPfw_RMW32(pll->addr + PM_PLL_CTRL_OFFSET, PM_PLL_CTRL_RESET_MASK,
		   PM_PLL_CTRL_RESET_MASK);
}

/**
 * PmPllSuspend() - Save context of PLL and power it down (reset)
 * @pll Pointer to a Pll to be suspended
 */
static void PmPllSuspend(PmSlavePll* const pll)
{
	u32 val;

	/* Save register setting */
	pll->context.ctrl = XPfw_Read32(pll->addr + PM_PLL_CTRL_OFFSET);
	pll->context.cfg = XPfw_Read32(pll->addr + PM_PLL_CFG_OFFSET);
	pll->context.frac = XPfw_Read32(pll->addr + PM_PLL_FRAC_OFFSET);
	pll->context.toCtrl = XPfw_Read32(pll->toCtrlAddr);
	pll->context.saved = true;

	val = XPfw_Read32(pll->addr + PM_PLL_CTRL_OFFSET);
	/* If PLL is not already in reset, bypass it and put in reset/pwrdn */
	if (0U == (val & PM_PLL_CTRL_RESET_MASK)) {
		PmPllBypassAndReset(pll);
	}
}

/**
 * PmPllResume() - Restore PLL context
 * @pll         Pll whose context should be restored
 *
 * @return      Status of resume:
 *              - XST_SUCCESS if resumed correctly
 *              - XST_FAILURE if resume failed (if PLL failed to lock)
 */
static int PmPllResume(PmSlavePll* const pll)
{
	int status = XST_SUCCESS;

	if (true == pll->context.saved) {
		/* Restore register values with reset and bypass asserted */
		XPfw_Write32(pll->addr + PM_PLL_CTRL_OFFSET, pll->context.ctrl |
			     PM_PLL_CTRL_RESET_MASK | PM_PLL_CTRL_BYPASS_MASK);
		XPfw_Write32(pll->addr + PM_PLL_CFG_OFFSET, pll->context.cfg);
		XPfw_Write32(pll->addr + PM_PLL_FRAC_OFFSET, pll->context.frac);
		XPfw_Write32(pll->toCtrlAddr, pll->context.toCtrl);
		pll->context.saved = false;
	}

	if (0U != (PM_PLL_CTRL_RESET_MASK & pll->context.ctrl)) {
		/* By saved/init configuration PLL is in reset, leave it as is */
		goto done;
	}

	/* Release reset */
	XPfw_RMW32(pll->addr + PM_PLL_CTRL_OFFSET, PM_PLL_CTRL_RESET_MASK,
		   ~PM_PLL_CTRL_RESET_MASK);
	/* Poll status register for the lock */
	status = XPfw_UtilPollForMask(pll->statusAddr, pll->lockMask,
				      PM_PLL_LOCK_TIMEOUT);
	if (XST_SUCCESS != status) {
		/* Failed to lock PLL - assert reset and return */
		XPfw_RMW32(pll->addr + PM_PLL_CTRL_OFFSET,
			   PM_PLL_CTRL_RESET_MASK, PM_PLL_CTRL_RESET_MASK);
		goto done;
	}

	/* PLL is bypassed here (done by the reset) */
	if (0U == (PM_PLL_CTRL_BYPASS_MASK & pll->context.ctrl)) {
		/* According to saved context PLL should not be bypassed */
		XPfw_RMW32(pll->addr + PM_PLL_CTRL_OFFSET,
			   PM_PLL_CTRL_BYPASS_MASK,
			  ~PM_PLL_CTRL_BYPASS_MASK);
	}

done:
	return status;
}

/**
 * PmPllFsmHandler() - PLL FSM handler
 * @slave       Slave whose state should be changed
 * @nextState   State the slave should enter
 *
 * @return      Always XST_SUCCESS if FSM is implemented correctly
 *
 * Note: PLL FSM basically updates currState variable and restores PLL state
 * if needed. FSM transitions cannot fail.
 */
static int PmPllFsmHandler(PmSlave* const slave, const PmStateId nextState)
{
	int status;
	PmSlavePll* pll = (PmSlavePll*)slave->node.derived;

	switch (nextState) {
	case PM_PLL_STATE_USED:
		/* Resume the PLL */
		status = PmPllResume(pll);
		break;
	case PM_PLL_STATE_UNUSED:
		/* Bypassing and reseting PLL cannot fail */
		PmPllBypassAndReset(pll);
		status = XST_SUCCESS;
		break;
	default:
		status = XST_PM_INTERNAL;
		break;
	}

	if (XST_SUCCESS == status) {
		slave->node.currState = nextState;
	}

	return status;
}

/* PLL FSM */
static const PmSlaveFsm slavePllFsm = {
	.states = pmPllStates,
	.statesCnt = ARRAY_SIZE(pmPllStates),
	.trans = pmPllTransitions,
	.transCnt = ARRAY_SIZE(pmPllTransitions),
	.enterState = PmPllFsmHandler,
};

/* APLL */
static PmRequirement* const pmApllReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_APLL],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_APLL],
};

PmSlavePll pmSlaveApll_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveApll_g,
			.nodeId = NODE_APLL,
			.typeId = PM_TYPE_PLL,
			.parent = &pmPowerDomainFpd_g,
			.ops = NULL,
		},
		.reqs = pmApllReqs,
		.reqsCnt = ARRAY_SIZE(pmApllReqs),
		.wake = NULL,
		.slvFsm = &slavePllFsm,
	},
	.context = {
		.saved = false,
	},
	.addr = CRF_APB_APLL_CTRL,
	.toCtrlAddr = CRF_APB_APLL_TO_LPD_CTRL,
	.statusAddr = CRF_APB_PLL_STATUS,
	.lockMask = CRF_APB_PLL_STATUS_APLL_LOCK_MASK,
};

/* VPLL */
static PmRequirement* const pmVpllReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_VPLL],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_VPLL],
};

PmSlavePll pmSlaveVpll_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveVpll_g,
			.nodeId = NODE_VPLL,
			.typeId = PM_TYPE_PLL,
			.parent = &pmPowerDomainFpd_g,
			.ops = NULL,
		},
		.reqs = pmVpllReqs,
		.reqsCnt = ARRAY_SIZE(pmVpllReqs),
		.wake = NULL,
		.slvFsm = &slavePllFsm,
	},
	.context = {
		.saved = false,
	},
	.addr = CRF_APB_VPLL_CTRL,
	.toCtrlAddr = CRF_APB_VPLL_TO_LPD_CTRL,
	.statusAddr = CRF_APB_PLL_STATUS,
	.lockMask = CRF_APB_PLL_STATUS_VPLL_LOCK_MASK,
};

/* DPLL */
static PmRequirement* const pmDpllReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_DPLL],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_DPLL],
};

PmSlavePll pmSlaveDpll_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveDpll_g,
			.nodeId = NODE_DPLL,
			.typeId = PM_TYPE_PLL,
			.parent = &pmPowerDomainFpd_g,
			.ops = NULL,
		},
		.reqs = pmDpllReqs,
		.reqsCnt = ARRAY_SIZE(pmDpllReqs),
		.wake = NULL,
		.slvFsm = &slavePllFsm,
	},
	.context = {
		.saved = false,
	},
	.addr = CRF_APB_DPLL_CTRL,
	.toCtrlAddr = CRF_APB_DPLL_TO_LPD_CTRL,
	.statusAddr = CRF_APB_PLL_STATUS,
	.lockMask = CRF_APB_PLL_STATUS_DPLL_LOCK_MASK,
};

/* RPLL */
static PmRequirement* const pmRpllReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_RPLL],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_RPLL],
};

PmSlavePll pmSlaveRpll_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveRpll_g,
			.nodeId = NODE_RPLL,
			.typeId = PM_TYPE_PLL,
			.parent = NULL,
			.ops = NULL,
		},
		.reqs = pmRpllReqs,
		.reqsCnt = ARRAY_SIZE(pmRpllReqs),
		.wake = NULL,
		.slvFsm = &slavePllFsm,
	},
	.context = {
		.saved = false,
	},
	.addr = CRL_APB_RPLL_CTRL,
	.toCtrlAddr = CRL_APB_RPLL_TO_FPD_CTRL,
	.statusAddr = CRL_APB_PLL_STATUS,
	.lockMask = CRL_APB_PLL_STATUS_RPLL_LOCK_MASK,
};

/* IOPLL */
static PmRequirement* const pmIOpllReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_IOPLL],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_IOPLL],
};

PmSlavePll pmSlaveIOpll_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveIOpll_g,
			.nodeId = NODE_IOPLL,
			.typeId = PM_TYPE_PLL,
			.parent = NULL,
			.ops = NULL,
		},
		.reqs = pmIOpllReqs,
		.reqsCnt = ARRAY_SIZE(pmIOpllReqs),
		.wake = NULL,
		.slvFsm = &slavePllFsm,
	},
	.context = {
		.saved = false,
	},
	.addr = CRL_APB_IOPLL_CTRL,
	.toCtrlAddr = CRL_APB_IOPLL_TO_FPD_CTRL,
	.statusAddr = CRL_APB_PLL_STATUS,
	.lockMask = CRL_APB_PLL_STATUS_IOPLL_LOCK_MASK,
};

static PmSlavePll* const pmPlls[] = {
	&pmSlaveApll_g,
	&pmSlaveVpll_g,
	&pmSlaveDpll_g,
	&pmSlaveRpll_g,
	&pmSlaveIOpll_g,
};

/**
 * PmPllSuspendAll() - Suspend all PLLs whose power parent is given as argument
 * @powerParent Power parent of PLLs to be suspended
 */
void PmPllSuspendAll(const PmPower* const powerParent)
{
	u8 i;

	for (i = 0U; i < ARRAY_SIZE(pmPlls); i++) {
		if (powerParent == pmPlls[i]->slv.node.parent) {
			PmPllSuspend(pmPlls[i]);
		}
	}
}
