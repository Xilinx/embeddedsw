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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */
#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * Contains:
 * PLL management implementation based on the use count (the number of
 * nodes whose clocks are driven by the PLL)
 *********************************************************************/

#include "pm_pll.h"
#include "pm_power.h"
#include "crf_apb.h"
#include "crl_apb.h"
#include "xpfw_util.h"

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
#define DEFAULT_PLL_POWER_LOCKED	100U
#define DEFAULT_PLL_POWER_RESET		0U

/* Period of time needed to lock the PLL (to measure) */
#define PM_PLL_LOCKING_TIME	1U

/**
 * PmPllBypassAndReset() - Bypass and reset/power down a PLL
 * @pll Pointer to a Pll to be bypassed/reset
 */
static void PmPllBypassAndReset(PmPll* const pll)
{
	/* Bypass PLL before putting it into the reset */
	XPfw_RMW32(pll->addr + PM_PLL_CTRL_OFFSET, PM_PLL_CTRL_BYPASS_MASK,
		   PM_PLL_CTRL_BYPASS_MASK);

	/* Power down PLL (= reset PLL) */
	XPfw_RMW32(pll->addr + PM_PLL_CTRL_OFFSET, PM_PLL_CTRL_RESET_MASK,
		   PM_PLL_CTRL_RESET_MASK);
}

/**
 * PmPllSaveContext() - Save the context of the PLL
 * @pll		PLL whose context should be saved
 */
static void PmPllSaveContext(PmPll* const pll)
{
	/* Save register setting */
	pll->context.ctrl = XPfw_Read32(pll->addr + PM_PLL_CTRL_OFFSET);
	pll->context.cfg = XPfw_Read32(pll->addr + PM_PLL_CFG_OFFSET);
	pll->context.frac = XPfw_Read32(pll->addr + PM_PLL_FRAC_OFFSET);
	pll->context.toCtrl = XPfw_Read32(pll->toCtrlAddr);
	pll->context.saved = true;
}

/**
 * PmPllRestoreContext() - Restore the context of the PLL
 * @pll		PLL whose context should be restored
 */
static void PmPllRestoreContext(PmPll* const pll)
{
	/* Bypass PLL */
	XPfw_RMW32(pll->addr + PM_PLL_CTRL_OFFSET, PM_PLL_CTRL_BYPASS_MASK,
			PM_PLL_CTRL_BYPASS_MASK);
	/* Assert PLL reset */
	XPfw_RMW32(pll->addr + PM_PLL_CTRL_OFFSET, PM_PLL_CTRL_RESET_MASK,
			PM_PLL_CTRL_RESET_MASK);
	/* Restore register values with reset and bypass asserted */
	XPfw_Write32(pll->addr + PM_PLL_CTRL_OFFSET, pll->context.ctrl |
			PM_PLL_CTRL_RESET_MASK | PM_PLL_CTRL_BYPASS_MASK);
	XPfw_Write32(pll->addr + PM_PLL_CFG_OFFSET, pll->context.cfg);
	XPfw_Write32(pll->addr + PM_PLL_FRAC_OFFSET, pll->context.frac);
	XPfw_Write32(pll->toCtrlAddr, pll->context.toCtrl);
	pll->context.saved = false;
}

/**
 * PmPllSuspend() - Save context of PLL and power it down (reset)
 * @pll Pointer to a Pll to be suspended
 */
static void PmPllSuspend(PmPll* const pll)
{
	PmDbg(DEBUG_DETAILED,"%s\r\n", PmStrNode(pll->node.nodeId));

	PmPllSaveContext(pll);

	/* If PLL is not already in reset, bypass it and put in reset/pwrdn */
	if (0U == (pll->context.ctrl & PM_PLL_CTRL_RESET_MASK)) {
		PmPllBypassAndReset(pll);
	}

	PmNodeUpdateCurrState(&pll->node, PM_PLL_STATE_RESET);
	if (NULL != pll->node.parent) {
		PmPowerReleaseParent(&pll->node);
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
static int PmPllResume(PmPll* const pll)
{
	int status = XST_SUCCESS;

	PmDbg(DEBUG_DETAILED,"%s\r\n", PmStrNode(pll->node.nodeId));

	if (true == pll->context.saved) {
		PmPllRestoreContext(pll);
	}

	if (0U != (PM_PLL_CTRL_RESET_MASK & pll->context.ctrl)) {
		/* By saved/init configuration PLL is in reset, leave it as is */
		goto done;
	}
	if (NULL != pll->node.parent) {
		status = PmPowerRequestParent(&pll->node);
		if (XST_SUCCESS != status) {
			goto done;
		}
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
	PmNodeUpdateCurrState(&pll->node, PM_PLL_STATE_LOCKED);

done:
	return status;
}

/**
 * PmPllClearConfig() - Clear configuration of the PLL
 * @node	PLL node
 */
static void PmPllClearConfig(PmNode* const node)
{
	PmPll* pll = (PmPll*)node->derived;

	pll->useCount = 0U;
}

/**
 * PmPllGetWakeUpLatency() - Get wake-up latency of a PLL
 * @node	PLL node
 * @lat		Pointer to the location where the latency value should be stored
 *
 * @return	XST_SUCCESS if latency value is stored in *lat, XST_NO_FEATURE
 *		if the latency depends on power parent which has no method
 *		(getWakeUpLatency) to provide latency information.
 */
static int PmPllGetWakeUpLatency(const PmNode* const node, u32* const lat)
{
	int status = XST_SUCCESS;
	PmPll* pll = (PmPll*)node->derived;
	PmNode* const powerNode = &node->parent->node;
	u32 latency;

	*lat = 0U;
	if (PM_PLL_STATE_LOCKED == pll->node.currState) {
		goto done;
	}

	*lat += PM_PLL_LOCKING_TIME;
	if (NULL == powerNode->class->getWakeUpLatency) {
		status = XST_NO_FEATURE;
		goto done;
	}

	status = powerNode->class->getWakeUpLatency(powerNode, &latency);
	if (XST_SUCCESS == status) {
		*lat += latency;
	}

done:
	return status;
}

/**
 * PmPllForceDown() - Force down a PLL node
 * @node	PLL node
 *
 * @return	XST_SUCCESS always (operation cannot fail)
 */
static int PmPllForceDown(PmNode* const node)
{
	PmPll* pll = (PmPll*)node->derived;

	if (PM_PLL_STATE_LOCKED == node->currState) {
		pll->useCount = 0U;
		PmPllSuspend(pll);
	}

	return XST_SUCCESS;
}

/**
 * PmPllInit() - Initialize the PLL
 * @node	PLL node
 *
 * @note	This function does not affect the PLL configuration in hardware.
 */
static int PmPllInit(PmNode* const node)
{
	PmPll* pll = (PmPll*)node->derived;
	u32 ctrl = XPfw_Read32(pll->addr + PM_PLL_CTRL_OFFSET);
	int status = XST_SUCCESS;

	if (0U == (ctrl & PM_PLL_CTRL_RESET_MASK)) {
		node->currState = PM_PLL_STATE_LOCKED;
		if (NULL != node->parent) {
			status = PmPowerRequestParent(node);
		}
	} else {
		node->currState = PM_PLL_STATE_RESET;
	}

	return status;
}

/**
 * PmPllIsUsable() - Check if the PLL is used according to the set configuration
 * @node	PLL node
 *
 * @return	True if PLL is used, false otherwise
 */
static bool PmPllIsUsable(PmNode* const node)
{
	PmPll* pll = (PmPll*)node->derived;

	return 0U != pll->useCount;
}

/* Collection of PLL nodes */
static PmNode* pmNodePllBucket[] = {
	&pmApll_g.node,
	&pmVpll_g.node,
	&pmDpll_g.node,
	&pmRpll_g.node,
	&pmIOpll_g.node,
};

PmNodeClass pmNodeClassPll_g = {
	DEFINE_NODE_BUCKET(pmNodePllBucket),
	.id = NODE_CLASS_PLL,
	.clearConfig = PmPllClearConfig,
	.construct = NULL,
	.getWakeUpLatency = PmPllGetWakeUpLatency,
	.getPowerData = PmNodeGetPowerInfo,
	.forceDown = PmPllForceDown,
	.init = PmPllInit,
	.isUsable = PmPllIsUsable,
};

static u32 PmStdPllPowers[] = {
	DEFAULT_PLL_POWER_RESET,
	DEFAULT_PLL_POWER_LOCKED,
};

PmPll pmApll_g = {
	.node = {
		.derived = &pmApll_g,
		.nodeId = NODE_APLL,
		.class = &pmNodeClassPll_g,
		.parent = &pmPowerDomainFpd_g.power,
		.clocks = NULL,
		.currState = PM_PLL_STATE_RESET,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmStdPllPowers),
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

PmPll pmVpll_g = {
	.node = {
		.derived = &pmVpll_g,
		.nodeId = NODE_VPLL,
		.class = &pmNodeClassPll_g,
		.parent = &pmPowerDomainFpd_g.power,
		.clocks = NULL,
		.currState = PM_PLL_STATE_RESET,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmStdPllPowers),
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

PmPll pmDpll_g __attribute__((__section__(".srdata"))) = {
	.node = {
		.derived = &pmDpll_g,
		.nodeId = NODE_DPLL,
		.class = &pmNodeClassPll_g,
		.parent = &pmPowerDomainFpd_g.power,
		.clocks = NULL,
		.currState = PM_PLL_STATE_RESET,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmStdPllPowers),
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

PmPll pmRpll_g = {
	.node = {
		.derived = &pmRpll_g,
		.nodeId = NODE_RPLL,
		.class = &pmNodeClassPll_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_PLL_STATE_RESET,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmStdPllPowers),
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

PmPll pmIOpll_g = {
	.node = {
		.derived = &pmIOpll_g,
		.nodeId = NODE_IOPLL,
		.class = &pmNodeClassPll_g,
		.parent = &pmPowerDomainLpd_g.power,
		.clocks = NULL,
		.currState = PM_PLL_STATE_RESET,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmStdPllPowers),
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
int PmPllRequest(PmPll* const pll)
{
	int status = XST_SUCCESS;

#ifdef DEBUG_CLK
	PmDbg(DEBUG_DETAILED,"%s #%lu\r\n", PmStrNode(pll->node.nodeId),
			1 + pll->useCount);
#endif
	/* If the PLL is suspended it needs to be resumed first */
	if (true == pll->context.saved) {
		status = PmPllResume(pll);
	}

	pll->useCount++;

	return status;
}

/**
 * PmPllRequest() - Release the PLL (if PLL becomes unused, it will be reset)
 * @pll		The released PLL
 */
void PmPllRelease(PmPll* const pll)
{
	if (pll->useCount > 0U) {
		pll->useCount--;
#ifdef DEBUG_CLK
		PmDbg(DEBUG_DETAILED,"%s #%lu\r\n", PmStrNode(pll->node.nodeId),
				pll->useCount);
#endif
		if (0U == pll->useCount) {
			PmPllSuspend(pll);
		}
	}
}

#endif
