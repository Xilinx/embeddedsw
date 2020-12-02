/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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

#define PLL_FRAC_CFG_ENABLED_MASK	CRF_APB_VPLL_FRAC_CFG_ENABLED_MASK

/* Configurable: timeout period when waiting for PLL to lock */
#define PM_PLL_LOCK_TIMEOUT	0x10000U

/* Power consumptions for PLLs defined by its states */
#define DEFAULT_PLL_POWER_LOCKED	100U
#define DEFAULT_PLL_POWER_RESET		0U

/* Period of time needed to lock the PLL (to measure) */
#define PM_PLL_LOCKING_TIME	1U

/* PLL flags */
#define PM_PLL_REQUESTED	(1U << 0U)
#define PM_PLL_CONTEXT_SAVED	(1U << 1U)

typedef struct PmPllParam {
	u8 regOffset;
	u8 shift;
	u8 bits;
} PmPllParam;

static PmPllParam pllParams[PM_PLL_MAX_PARAM] = {
	[PM_PLL_PARAM_DIV2] = {
		.regOffset = PM_PLL_CTRL_OFFSET,
		.shift = 16U,
		.bits = 1U,
	},
	[PM_PLL_PARAM_FBDIV] = {
		.regOffset = PM_PLL_CTRL_OFFSET,
		.shift = 8U,
		.bits = 7U,
	},
	[PM_PLL_PARAM_DATA] = {
		.regOffset = PM_PLL_FRAC_OFFSET,
		.shift = 0U,
		.bits = 16U,
	},
	[PM_PLL_PARAM_PRE_SRC] = {
		.regOffset = PM_PLL_CTRL_OFFSET,
		.shift = 20U,
		.bits = 3U,
	},
	[PM_PLL_PARAM_POST_SRC] = {
		.regOffset = PM_PLL_CTRL_OFFSET,
		.shift = 24U,
		.bits = 3U,
	},
	[PM_PLL_PARAM_LOCK_DLY] = {
		.regOffset = PM_PLL_CFG_OFFSET,
		.shift = 25U,
		.bits = 7U,
	},
	[PM_PLL_PARAM_LOCK_CNT] = {
		.regOffset = PM_PLL_CFG_OFFSET,
		.shift = 13U,
		.bits = 10U,
	},
	[PM_PLL_PARAM_LFHF] = {
		.regOffset = PM_PLL_CFG_OFFSET,
		.shift = 10U,
		.bits = 3U,
	},
	[PM_PLL_PARAM_CP] = {
		.regOffset = PM_PLL_CFG_OFFSET,
		.shift = 5U,
		.bits = 4U,
	},
	[PM_PLL_PARAM_RES] = {
		.regOffset = PM_PLL_CFG_OFFSET,
		.shift = 0U,
		.bits = 4U,
	},
};

/**
 * PmPllBypassAndReset() - Bypass and reset/power down a PLL
 * @pll Pointer to a Pll to be bypassed/reset
 */
static void PmPllBypassAndReset(PmPll* const pll)
{
	u32 pllCtrl = pll->addr + PM_PLL_CTRL_OFFSET;
	u32 r;
#ifdef ENABLE_EM
	u32 pllErrMask = 1 << pll->errShift;
	pll->errValue = 0;
	/*
	 * Store PLL lock error interrupt mask and error enable
	 * before disabling it
	 */
	pll->errValue |= (~XPfw_Read32(PMU_GLOBAL_ERROR_INT_MASK_2) &
			   pllErrMask) >> pll->errShift;
	pll->errValue |= ((~XPfw_Read32(PMU_GLOBAL_ERROR_POR_MASK_2) &
			   pllErrMask) >> pll->errShift) << 1;
	pll->errValue |= ((~XPfw_Read32(PMU_GLOBAL_ERROR_SRST_MASK_2) &
			   pllErrMask) >> pll->errShift) << 2;
	pll->errValue |= ((~XPfw_Read32(PMU_GLOBAL_ERROR_SIG_MASK_2) &
			   pllErrMask) >> pll->errShift) << 3;
	pll->errValue |= ((~XPfw_Read32(PMU_GLOBAL_ERROR_EN_2) &
			   pllErrMask) >> pll->errShift) << 4;
	/* Disable PLL lock error interrupts before powering down PLL */
	XPfw_Write32(PMU_GLOBAL_ERROR_INT_DIS_2, pllErrMask);
	XPfw_Write32(PMU_GLOBAL_ERROR_POR_DIS_2, pllErrMask);
	XPfw_Write32(PMU_GLOBAL_ERROR_SRST_DIS_2, pllErrMask);
	XPfw_Write32(PMU_GLOBAL_ERROR_SIG_DIS_2, pllErrMask);
	XPfw_RMW32(PMU_GLOBAL_ERROR_EN_2, pllErrMask, 0x0U);
#endif

	/* Bypass PLL before putting it into the reset */
	r = Xil_In32(pllCtrl);
	r |= PM_PLL_CTRL_BYPASS_MASK;
	Xil_Out32(pllCtrl, r);

	/* Power down PLL (= reset PLL) */
	r = Xil_In32(pllCtrl);
	r |= PM_PLL_CTRL_RESET_MASK;
	Xil_Out32(pllCtrl, r);
}

/**
 * PmPllLock() - Trigger locking of the PLL and wait for it to lock
 * @pll		Target PLL
 *
 * @status	Status of polling for the lock status as returned by
 *		XPfw_UtilPollForMask
 */
static s32 PmPllLock(const PmPll* const pll)
{
	s32 status;

	/* Deassert reset to trigger the PLL locking */
	XPfw_RMW32(pll->addr + PM_PLL_CTRL_OFFSET, PM_PLL_CTRL_RESET_MASK,
		   ~PM_PLL_CTRL_RESET_MASK);
	/* Poll status register for the lock */
	status = XPfw_UtilPollForMask(pll->statusAddr, (u32)1 << pll->lockShift,
				      PM_PLL_LOCK_TIMEOUT);

#ifdef ENABLE_EM
	/*
	 * Restore PLL lock error interrupts and error enable
	 * once PLL is locked
	 */
	XPfw_Write32(PMU_GLOBAL_ERROR_INT_EN_2,
		     ((pll->errValue & 1U) << pll->errShift));
	XPfw_Write32(PMU_GLOBAL_ERROR_POR_EN_2,
		     (((pll->errValue >> 1) & 1U) << pll->errShift));
	XPfw_Write32(PMU_GLOBAL_ERROR_SRST_EN_2,
		     (((pll->errValue >> 2) & 1U) << pll->errShift));
	XPfw_Write32(PMU_GLOBAL_ERROR_SIG_EN_2,
		     (((pll->errValue >> 3) & 1U) << pll->errShift));
	XPfw_RMW32(PMU_GLOBAL_ERROR_EN_2, (1U << pll->errShift),
			(((pll->errValue >> 4) & 1U) << pll->errShift));
#endif

	return status;
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
	pll->flags |= PM_PLL_CONTEXT_SAVED;
}

/**
 * PmPllRestoreContext() - Restore the context of the PLL
 * @pll		PLL whose context should be restored
 */
static void PmPllRestoreContext(PmPll* const pll)
{
	/* Bypass and reset PLL */
	PmPllBypassAndReset(pll);
	/* Restore register values with reset and bypass asserted */
	XPfw_Write32(pll->addr + PM_PLL_CTRL_OFFSET, pll->context.ctrl |
			PM_PLL_CTRL_RESET_MASK | PM_PLL_CTRL_BYPASS_MASK);
	XPfw_Write32(pll->addr + PM_PLL_CFG_OFFSET, pll->context.cfg);
	XPfw_Write32(pll->addr + PM_PLL_FRAC_OFFSET, pll->context.frac);
	pll->flags &= ~(u8)PM_PLL_CONTEXT_SAVED;
}

/**
 * PmPllSuspend() - Save context of PLL and power it down (reset)
 * @pll Pointer to a Pll to be suspended
 */
static void PmPllSuspend(PmPll* const pll)
{
	PmInfo("%s 1->0\r\n", pll->node.name);

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
static s32 PmPllResume(PmPll* const pll)
{
	s32 status = XST_SUCCESS;

	PmInfo("%s 0->1\r\n", pll->node.name);

	if (0U != (pll->flags & PM_PLL_CONTEXT_SAVED)) {
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
	status = PmPllLock(pll);
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

	pll->flags = 0U;
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
static s32 PmPllGetWakeUpLatency(const PmNode* const node, u32* const lat)
{
	s32 status = XST_SUCCESS;
	PmPll* pll = (PmPll*)node->derived;
	PmNode* const powerNode = &node->parent->node;
	u32 latency = 0U;

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
static s32 PmPllForceDown(PmNode* const node)
{
	PmPll* pll = (PmPll*)node->derived;

	if (PM_PLL_STATE_LOCKED == node->currState) {
		pll->flags = 0U;
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
static s32 PmPllInit(PmNode* const node)
{
	PmPll* pll = (PmPll*)node->derived;
	u32 ctrl = XPfw_Read32(pll->addr + PM_PLL_CTRL_OFFSET);
	s32 status = XST_SUCCESS;

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
 * PmPllGetPerms() - Get permissions of masters to control clocks of PLLs
 * @node	Target PLL node
 *
 * @return	ORed IPI masks of masters allowed to directly control the PLL
 *
 * @note	Permissions to control clocks of PLLs is equivalent to
 *		permissions to directly configure the PLL.
 */
static u32 PmPllGetPerms(const PmNode* const node)
{
	const PmPll* pll = (PmPll*)node->derived;

	return pll->perms;
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

	return 0U != (PM_PLL_REQUESTED & pll->flags);
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
	.getPerms = PmPllGetPerms,
};

static u8 PmStdPllPowers[] = {
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
		DEFINE_NODE_NAME("apll"),
	},
	.context = { 0U },
	.addr = CRF_APB_APLL_CTRL,
	.statusAddr = CRF_APB_PLL_STATUS,
	.perms = 0U,
	.lockShift = CRF_APB_PLL_STATUS_APLL_LOCK_SHIFT,
	.flags = 0U,
	.childCount = 0U,
#ifdef ENABLE_EM
	.errShift = PMU_GLOBAL_ERROR_SIG_2_APLL_SHIFT,
	.errValue = 0,
#endif
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
		DEFINE_NODE_NAME("vpll"),
	},
	.context = { 0U },
	.addr = CRF_APB_VPLL_CTRL,
	.statusAddr = CRF_APB_PLL_STATUS,
	.perms = 0U,
	.lockShift = CRF_APB_PLL_STATUS_VPLL_LOCK_SHIFT,
	.flags = 0U,
	.childCount = 0U,
#ifdef ENABLE_EM
	.errShift = PMU_GLOBAL_ERROR_SIG_2_VPLL_SHIFT,
	.errValue = 0,
#endif
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
		DEFINE_NODE_NAME("dpll"),
	},
	.context = { 0U },
	.addr = CRF_APB_DPLL_CTRL,
	.statusAddr = CRF_APB_PLL_STATUS,
	.perms = 0U,
	.lockShift = CRF_APB_PLL_STATUS_DPLL_LOCK_SHIFT,
	.flags = 0U,
	.childCount = 0U,
#ifdef ENABLE_EM
	.errShift = PMU_GLOBAL_ERROR_SIG_2_DPLL_SHIFT,
	.errValue = 0,
#endif
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
		DEFINE_NODE_NAME("rpll"),
	},
	.context = { 0U },
	.addr = CRL_APB_RPLL_CTRL,
	.statusAddr = CRL_APB_PLL_STATUS,
	.perms = 0U,
	.lockShift = CRL_APB_PLL_STATUS_RPLL_LOCK_SHIFT,
	.flags = 0U,
	.childCount = 0U,
#ifdef ENABLE_EM
	.errShift = PMU_GLOBAL_ERROR_SIG_2_RPLL_SHIFT,
	.errValue = 0,
#endif
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
		DEFINE_NODE_NAME("iopll"),
	},
	.context = { 0U },
	.addr = CRL_APB_IOPLL_CTRL,
	.statusAddr = CRL_APB_PLL_STATUS,
	.perms = 0U,
	.lockShift = CRL_APB_PLL_STATUS_IOPLL_LOCK_SHIFT,
	.flags = 0U,
	.childCount = 0U,
#ifdef ENABLE_EM
	.errShift = PMU_GLOBAL_ERROR_SIG_2_IOPLL_SHIFT,
	.errValue = 0,
#endif
};

/**
 * PmPllRequest() - Request the PLL
 * @pll		The requested PLL
 * @note	If the requested PLL is not locked and if it was never locked
 *		before, the PM framework will not lock it because the frequency
 *		related aspects are not handled by the PM framework. The PM
 *		framework only saves/restores the context of PLLs.
 */
void PmPllRequest(PmPll* const pll)
{
	/* If the PLL is suspended it needs to be resumed first */
	if (0U != (PM_PLL_CONTEXT_SAVED & pll->flags)) {
		s32 status = PmPllResume(pll);
		if (XST_SUCCESS != status) {
			PmErr("Failed to lock %s", pll->node.name);
		}
	}
	pll->flags |= PM_PLL_REQUESTED;
}

/**
 * PmPllRelease() - Release the PLL (PLL will be suspended)
 * @pll		The released PLL
 */
void PmPllRelease(PmPll* const pll)
{
	pll->flags &= ~(u8)PM_PLL_REQUESTED;
	PmPllSuspend(pll);
}

/**
 * PmPllSetParameterInt() - Set PLL parameter
 * @pll		PLL whose parameter should be set
 * @paramId	Parameter ID
 * @val		Parameter value to be set
 *
 * @return	Status of setting the parameter:
 *		XST_INVALID_PARAM if one of the given arguments is invalid
 *		XST_SUCCESS if parameter is set
 */
s32 PmPllSetParameterInt(PmPll* const pll, const u32 paramId, const u32 val)
{
	s32 status = XST_INVALID_PARAM;
	PmPllParam* p;

	if (paramId >= ARRAY_SIZE(pllParams)) {
		goto done;
	}

	p = &pllParams[paramId];
	if (val > MASK_OF_BITS(p->bits)) {
		goto done;
	}

	/*
	 * We're running on a ZynqMP compatible machine, make sure the
	 * VPLL only has one child. Check only while changing PLL rate.
	 * This helps to remove the warn in cases where the expected clock
	 * is not using vpll and vpll is used for other stuff.
	 */
	if ((NODE_VPLL == pll->node.nodeId) && (PM_PLL_PARAM_FBDIV == paramId)) {
		if (pll->childCount > 1U) {
			PmErr("More than 1 devices are using VPLL which is forbidden\r\n");
			status = XST_PM_MULT_USER;
			goto done;
		}
	}

	XPfw_RMW32(pll->addr + p->regOffset, MASK_OF_BITS(p->bits) << p->shift,
		   val << p->shift);
	status = XST_SUCCESS;

done:
	return status;
}

/**
 * PmPllGetParameterInt() - Get the PLL parameter value
 * @pll		PLL whose parameter should be get
 * @paramId	Parameter ID
 * @val		Location to store parameter value
 *
 * @return	Status of setting the parameter:
 *		XST_INVALID_PARAM if one of the given arguments is invalid
 *		XST_SUCCESS if parameter is set
 */
s32 PmPllGetParameterInt(PmPll* const pll, const u32 paramId, u32* const val)
{
	s32 status = XST_SUCCESS;
	PmPllParam* p;

	if (paramId >= ARRAY_SIZE(pllParams)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	p = &pllParams[paramId];
	*val = XPfw_Read32(pll->addr + p->regOffset) >> p->shift;
	*val &= MASK_OF_BITS(p->bits);

done:
	return status;
}

/**
 * PmPllSetModeInt() - Set the mode for PLL
 * @pll		Target PLL
 * @mode	Identifier of the mode to be set
 *
 * @return	XST_SUCCESS if the mode is set
 *		XST_NO_DATA if the fractional mode is requested and configured
 *		fractional divider is zero
 */
s32 PmPllSetModeInt(PmPll* const pll, const u32 mode)
{
	s32 status = XST_SUCCESS;
	u32 val;

	/* Check whether all config parameters are known for frac/int mode */
	if (PM_PLL_MODE_FRACTIONAL == mode) {
		PmPllParam* p = &pllParams[PM_PLL_PARAM_DATA];

		val = XPfw_Read32(pll->addr + p->regOffset);
		val = (val >> p->shift) & MASK_OF_BITS(p->bits);
		/* Check if fractional divider has been set (data parameter) */
		if (0U == val) {
			status = XST_NO_DATA;
			goto done;
		}
	}

	PmPllBypassAndReset(pll);
	if (PM_PLL_MODE_RESET == mode) {
		goto done;
	}

	if (PM_PLL_MODE_FRACTIONAL == mode) {
		val = PLL_FRAC_CFG_ENABLED_MASK;
	} else {
		val = ~PLL_FRAC_CFG_ENABLED_MASK;
	}
	/* Enable/disable fractional mode */
	XPfw_RMW32(pll->addr + PM_PLL_FRAC_OFFSET, PLL_FRAC_CFG_ENABLED_MASK,
		   val);

	status = PmPllLock(pll);
	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Deassert bypass if the PLL has locked */
	XPfw_RMW32(pll->addr + PM_PLL_CTRL_OFFSET,
		   PM_PLL_CTRL_BYPASS_MASK, ~PM_PLL_CTRL_BYPASS_MASK);

done:
	return status;
}

/**
 * PmPllGetModeInt() - Get current PLL mode
 * @pll		Target PLL
 *
 * @return	Current mode of the PLL, i.e. one of the following:
 *		PM_PLL_MODE_FRACTIONAL
 *		PM_PLL_MODE_INTEGER
 *		PM_PLL_MODE_RESET
 */
u32 PmPllGetModeInt(PmPll* const pll)
{
	u32 val, mode;

	val = XPfw_Read32(pll->addr + PM_PLL_CTRL_OFFSET);
	if (0U != (val & PM_PLL_CTRL_RESET_MASK)) {
		mode = PM_PLL_MODE_RESET;
	} else {
		val = XPfw_Read32(pll->addr + PM_PLL_FRAC_OFFSET);
		if (0U != (val & PLL_FRAC_CFG_ENABLED_MASK)) {
			mode = PM_PLL_MODE_FRACTIONAL;
		} else {
			mode = PM_PLL_MODE_INTEGER;
		}
	}

	return mode;
}

/**
 * PmPllOpenAccess() - Allow direct access to the master with given IPI mask
 * @pll		Target PLL
 * @ipiMask	IPI mask of the master that will be allowed to directly control
 *		the target PLL
 */
void PmPllOpenAccess(PmPll* const pll, u32 ipiMask) {
	pll->perms = ipiMask;
};

#endif
