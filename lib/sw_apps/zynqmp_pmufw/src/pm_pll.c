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

/* PLL states: */
/*
 * PLL is not used by any master, so it can be powered down and it's power
 * parent can be powered down as well.
 */
#define PM_PLL_STATE_UNUSED	0U
/* PLL is used by at least one master which is controlling state of PLL */
#define PM_PLL_STATE_USED	1U
#define PM_PLL_STATE_MAX	2U

/* Register offsets (in regard to PLL's base address of control registers) */
#define PM_PLL_CTRL_OFFSET	0x0U
#define PM_PLL_CFG_OFFSET	0x4U
#define PM_PLL_FRAC_OFFSET	0x8U

/* Masks of bitfields in PLL's control register */
#define PM_PLL_CTRL_RESET_MASK	0x1U
#define PM_PLL_CTRL_BYPASS_MASK	0x8U

/* Configurable: timeout period when waiting for PLL to lock */
#define PM_PLL_LOCK_TIMEOUT	0x10000U

/* Power consumptions for PLLs defined by its states */
#define DEFAULT_PLL_POWER_ON	100U
#define DEFAULT_PLL_POWER_OFF	0U

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
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_PLL_STATE_UNUSED,
		.toState = PM_PLL_STATE_USED,
		.latency = PM_DEFAULT_LATENCY,
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

	PmDbg("%s\r\n", PmStrNode(pll->slv.node.nodeId));

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

	PmDbg("%s\r\n", PmStrNode(pll->slv.node.nodeId));

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
		/* Suspend the PLL (cannot fail) */
		PmPllSuspend(pll);
		status = XST_SUCCESS;
		break;
	default:
		status = XST_PM_INTERNAL;
		break;
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

static u32 PmStdPllPowers[] = {
	DEFAULT_PLL_POWER_OFF,
	DEFAULT_PLL_POWER_ON,
};

PmSlavePll pmSlaveApll_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveApll_g,
			.nodeId = NODE_APLL,
			.typeId = PM_TYPE_PLL,
			.parent = &pmPowerDomainFpd_g,
			.clocks = NULL,
			.currState = PM_PLL_STATE_UNUSED,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmStdPllPowers,
			.powerInfoCnt = ARRAY_SIZE(PmStdPllPowers),
		},
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slavePllFsm,
		.flags = PM_SLAVE_FLAG_IS_SHAREABLE,
	},
	.context = {
		.saved = false,
	},
	.addr = CRF_APB_APLL_CTRL,
	.toCtrlAddr = CRF_APB_APLL_TO_LPD_CTRL,
	.statusAddr = CRF_APB_PLL_STATUS,
	.lockMask = CRF_APB_PLL_STATUS_APLL_LOCK_MASK,
	.useCount = 0U,
};

PmSlavePll pmSlaveVpll_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveVpll_g,
			.nodeId = NODE_VPLL,
			.typeId = PM_TYPE_PLL,
			.parent = &pmPowerDomainFpd_g,
			.clocks = NULL,
			.currState = PM_PLL_STATE_UNUSED,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmStdPllPowers,
			.powerInfoCnt = ARRAY_SIZE(PmStdPllPowers),
		},
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slavePllFsm,
		.flags = PM_SLAVE_FLAG_IS_SHAREABLE,
	},
	.context = {
		.saved = false,
	},
	.addr = CRF_APB_VPLL_CTRL,
	.toCtrlAddr = CRF_APB_VPLL_TO_LPD_CTRL,
	.statusAddr = CRF_APB_PLL_STATUS,
	.lockMask = CRF_APB_PLL_STATUS_VPLL_LOCK_MASK,
	.useCount = 0U,
};

PmSlavePll pmSlaveDpll_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveDpll_g,
			.nodeId = NODE_DPLL,
			.typeId = PM_TYPE_PLL,
			.parent = &pmPowerDomainFpd_g,
			.clocks = NULL,
			.currState = PM_PLL_STATE_UNUSED,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmStdPllPowers,
			.powerInfoCnt = ARRAY_SIZE(PmStdPllPowers),
		},
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slavePllFsm,
		.flags = PM_SLAVE_FLAG_IS_SHAREABLE,
	},
	.context = {
		.saved = false,
	},
	.addr = CRF_APB_DPLL_CTRL,
	.toCtrlAddr = CRF_APB_DPLL_TO_LPD_CTRL,
	.statusAddr = CRF_APB_PLL_STATUS,
	.lockMask = CRF_APB_PLL_STATUS_DPLL_LOCK_MASK,
	.useCount = 0U,
};

PmSlavePll pmSlaveRpll_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveRpll_g,
			.nodeId = NODE_RPLL,
			.typeId = PM_TYPE_PLL,
			.parent = &pmPowerDomainLpd_g,
			.clocks = NULL,
			.currState = PM_PLL_STATE_UNUSED,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmStdPllPowers,
			.powerInfoCnt = ARRAY_SIZE(PmStdPllPowers),
		},
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slavePllFsm,
		.flags = PM_SLAVE_FLAG_IS_SHAREABLE,
	},
	.context = {
		.saved = false,
	},
	.addr = CRL_APB_RPLL_CTRL,
	.toCtrlAddr = CRL_APB_RPLL_TO_FPD_CTRL,
	.statusAddr = CRL_APB_PLL_STATUS,
	.lockMask = CRL_APB_PLL_STATUS_RPLL_LOCK_MASK,
	.useCount = 0U,
};

PmSlavePll pmSlaveIOpll_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveIOpll_g,
			.nodeId = NODE_IOPLL,
			.typeId = PM_TYPE_PLL,
			.parent = &pmPowerDomainLpd_g,
			.clocks = NULL,
			.currState = PM_PLL_STATE_UNUSED,
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
			.powerInfo = PmStdPllPowers,
			.powerInfoCnt = ARRAY_SIZE(PmStdPllPowers),
		},
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slavePllFsm,
		.flags = PM_SLAVE_FLAG_IS_SHAREABLE,
	},
	.context = {
		.saved = false,
	},
	.addr = CRL_APB_IOPLL_CTRL,
	.toCtrlAddr = CRL_APB_IOPLL_TO_FPD_CTRL,
	.statusAddr = CRL_APB_PLL_STATUS,
	.lockMask = CRL_APB_PLL_STATUS_IOPLL_LOCK_MASK,
	.useCount = 0U,
};

static PmSlavePll* const pmPlls[] = {
	&pmSlaveApll_g,
	&pmSlaveVpll_g,
	&pmSlaveDpll_g,
	&pmSlaveRpll_g,
	&pmSlaveIOpll_g,
};

void PmPllClearUseCount(void)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmPlls); i++) {
		pmPlls[i]->useCount = 0U;
	}
}

/**
 * PmPllRequest() - Request the PLL
 * @pll		The requested PLL
 * @return	XST_SUCCESS if the request is processed ok, else XST_FAILURE
 *
 * @note	If the requested PLL is not locked and if it was never locked
 *		before, the PM framework will not lock it because the frequency
 *		related aspects are not handled by the PM framework. The PM
 *		framework only saves/restores the context of PLLs.
 */
int PmPllRequest(PmSlavePll* const pll)
{
	int status = XST_SUCCESS;

#ifdef DEBUG_CLK
	PmDbg("%s #%lu\r\n", PmStrNode(pll->slv.node.nodeId), 1 + pll->useCount);
#endif
	/* If the PLL is suspended it needs to be resumed first */
	if (true == pll->context.saved) {
		status = PmPllResume(pll);
		if (XST_SUCCESS == status) {
			PmNodeUpdateCurrState(&pll->slv.node, PM_PLL_STATE_USED);
		}
	}

	pll->useCount++;

	return status;
}

/**
 * PmPllRequest() - Release the PLL (if PLL becomes unused, it will be reset)
 * @pll		The released PLL
 */
void PmPllRelease(PmSlavePll* const pll)
{
	pll->useCount--;

#ifdef DEBUG_CLK
	PmDbg("%s #%lu\r\n", PmStrNode(pll->slv.node.nodeId), pll->useCount);
#endif
	if (0U == pll->useCount) {
		PmPllSuspend(pll);
		PmNodeUpdateCurrState(&pll->slv.node, PM_PLL_STATE_UNUSED);
	}
}
