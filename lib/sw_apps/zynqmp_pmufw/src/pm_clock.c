/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM
#include "pm_common.h"
#include "pm_clock.h"
#include "pm_power.h"
#include "pm_usb.h"
#include "pm_periph.h"
#include "pm_ddr.h"
#include "pm_pll.h"
#include "crf_apb.h"
#include "crl_apb.h"
#include "afi.h"
#include "xpfw_default.h"

/*********************************************************************
 * Macros
 ********************************************************************/
#define CONNECT(clk, nd)	\
{ \
	.clock = &(clk), \
	.node = &(nd), \
	.nextClock = NULL, \
	.nextNode = NULL, \
}

#define IOU_SLCR_WDT_CLK_SEL		(IOU_SLCR_BASE + 0x300U)
#define IOU_SLCR_CAN_MIO_CTRL		(IOU_SLCR_BASE + 0x304U)
#define IOU_SLCR_GEM_CLK_CTRL		(IOU_SLCR_BASE + 0x308U)

#define FPD_SLCR_WDT_CLK_SEL		(FPD_SLCR_BASEADDR + 0x100U)

#define PM_CLOCK_TYPE_DIV0	(1U << 1U)	/* bits 13:8 */
#define PM_CLOCK_TYPE_DIV1	(1U << 2U)	/* bits 21:16 */
#define PM_CLOCK_TYPE_GATE24	(1U << 3U)	/* bit 24 */
#define PM_CLOCK_TYPE_GATE25	(1U << 4U)	/* bit 25 */
#define PM_CLOCK_TYPE_GATE26	(1U << 5U)	/* bit 26 */
#define PM_CLOCK_TYPE_SYSTEM	(1U << 6U)	/* system level clock */

#define PM_CLOCK_TYPE_GATES    (PM_CLOCK_TYPE_GATE24 | \
				PM_CLOCK_TYPE_GATE25 | \
				PM_CLOCK_TYPE_GATE26)

#define PM_CLOCK_HAS_DIV0(clk)	(0U != ((clk)->type & PM_CLOCK_TYPE_DIV0))
#define PM_CLOCK_HAS_DIV1(clk)	(0U != ((clk)->type & PM_CLOCK_TYPE_DIV1))

#define PM_DIV0_SHIFT	8U
#define PM_DIV1_SHIFT	16U
#define PM_DIV_MASK	0x3FU

/*********************************************************************
 * Structure definitions
 ********************************************************************/

/**
 * PmClockCtrlMethods - Structure that encapsulates clock control methods
 * @initParent	Called during the boot to discover initial mux configuration
 * @getParent	Get mux select of the current clock parent
 * @setParent	Set clock parent (configure clock's mux)
 * @getGate	Get state of the clock gate
 * @setGate	Configure gate of this clock (activate or gate the clock)
 * @getDivider	Get currently configured divider of the clock
 * @setDivider	Set clock divider value
 */
typedef struct PmClockCtrlMethods {
	void (*const initParent)(PmClock* const clock);
	s32 (*const getParent)(PmClock* const clock, u32 *const select);
	s32 (*const setParent)(PmClock* const clock, const u32 select);
	s32 (*const getGate)(PmClock* const clock, u8* const enable);
	s32 (*const setGate)(PmClock* const clock, const u8 enable);
	s32 (*const getDivider)(PmClock* const clock, const u32 divId,
				u32* const val);
	s32 (*const setDivider)(PmClock* const clock, const u32 divId,
				const u32 val);
} PmClockCtrlMethods;

/**
 * PmClockClass - Structure that encapsulates essential clock methods
 * @request	Pointer to the function that is used to request clock
 * @release	Pointer to the function that is used to release clock
 * @ctrl	Pointer to struct that encapsulates other clock specific methods
 * @getPerms	Get permissions (which master can control this clock)
 *
 * @note	A class of clocks for which the maintenance of use count is
 * important must implement request/release methods. Other clock control
 * methods are optional and depend on a particular clock class. If none of this
 * is relevant for certain clock, e.g. oscillator, the class doesn't have to
 * be defined.
 */
struct PmClockClass {
	PmClock* (*const request)(PmClock* const clock);
	PmClock* (*const release)(PmClock* const clock);
	u32 (*const getPerms)(const PmClock* const clock);
	const PmClockCtrlMethods* const ctrl;
};

/*
 * PmClockPll - Structure for PLL-output clock
 * @base	Base clock structure
 * @pll		Pointer to the PLL that generates this clock
 * @useCount	Number of requests towards this clock
 */
typedef struct PmClockPll {
	PmClock base;
	PmPll* const pll;
	u8 useCount;
} PmClockPll;

/**
 * PmClockCrossDom - Clock structure for PLL cross-domain clocks
 * @base	Base clock structure
 * @parent	Pointer to the parent that drives this clock
 * @ctrlAddr	Address of the control register of the clock
 * @useCount	Number of requests towards this clock
 */
typedef struct PmClockCrossDom {
	PmClock base;
	PmClockPll* parent;
	const u32 ctrlAddr;
	u8 useCount;
} PmClockCrossDom;

/**
 * PmClockSel2ClkIn - Pair of multiplexer select value and selected clock input
 * @clkIn	Pointer to input clock that is selected with the 'select' value
 * @select	Select value of the clock multiplexer
 */
typedef struct {
	PmClock* const clkIn;
	const u8 select;
} PmClockSel2ClkIn;

/**
 * PmClockMux - Structure encapsulates MUX select values to clock input mapping
 * @inputs	Mux select to pll mapping at the input of the multiplexer
 * @size	Size of the inputs array
 * @bits	Number of bits of mux select
 * @shift	Number of bits to shift 'bits' in order to get mux select mask
 */
typedef struct {
	const PmClockSel2ClkIn* const inputs;
	const u8 size;
	const u8 bits;
	const u8 shift;
} PmClockMux;

/**
 * PmClockGen - Generic on-chip clock structure
 * @base	Base clock structure
 * @parent	Pointer to the current parent that drives this clock
 * @users	Pointer to the list of nodes that use this clock
 * @mux		Mux model for this clock (models possible parents and selects)
 * @ctrlAddr	Address of the control register of the clock
 * @ctrlVal	Value of control register found at boot
 * @type	Type of the clock (specifies available dividers and gate, and
 *		whether it's the system clock)
 * @useCount	Number of requests towards this clock
 */
typedef struct PmClockGen {
	PmClock base;
	PmClock* parent;
	PmClockHandle* users;
	PmClockMux* const mux;
	const u32 ctrlAddr;
	u32 ctrlVal;
	const u8 type;
	u8 useCount;
} PmClockGen;

/**
 * PmClockHandle - Models a clock/node pair (node using the clock)
 * @clock	Pointer to the clock used by the node
 * @node	Pointer to the node that uses the clock
 * @nextClock	Pointer to the next clock used by the node
 * @nextNode	Pointer to the next node that uses the clock
 */
struct PmClockHandle {
	PmClockGen* clock;
	PmNode* node;
	PmClockHandle* nextClock;
	PmClockHandle* nextNode;
};

/**
 * PmClockRequestInt() - Wrapper function for a chained requesting of a clock
 * @clockPtr	Pointer to the clock to be requested
 *
 * @note	This function implements non-recursive chained requesting of
 *		a clock and all its parents. Such an approach is required
 *		because recursion is not allowed due to the MISRA.
 */
static void PmClockRequestInt(PmClock* const clockPtr)
{
	PmClock* clk = clockPtr;

	while (NULL != clk) {
		if ((clk->class != NULL) && (clk->class->request != NULL)) {
			clk = clk->class->request(clk);
		} else {
			clk = NULL;
		}
	}
}

/**
 * PmClockReleaseInt() - Wrapper function for a chained releasing of a clock
 * @clockPtr	Pointer to the clock to be released
 *
 * @note	This function implements non-recursive chained releasing of
 *		a clock and all its parents. Such an approach is required
 *		because recursion is not allowed due to the MISRA.
 */
static void PmClockReleaseInt(PmClock* const clockPtr)
{
	PmClock* clk = clockPtr;

	while (NULL != clk) {
		if ((clk->class != NULL) && (clk->class->release != NULL)) {
			clk = clk->class->release(clk);
		} else {
			clk = NULL;
		}
	}
}

/******************************************************************************/
/* Pll output clock models */

/**
 * PmClockRequestPll() - PLL specific request clock method
 * @clockPtr	Pointer to a PLL clock
 *
 * @return	This function always returns NULL because the PLL has no clock
 *		parent (its parent is a PLL which is not modeled as a clock)
 */
static PmClock* PmClockRequestPll(PmClock* const clockPtr)
{
	PmClockPll* pclk = (PmClockPll*)clockPtr->derived;

	if (0U == pclk->useCount++) {
		PmPllRequest(pclk->pll);
	}

	return NULL;
}

/**
 * PmClockReleasePll() - PLL specific release clock method
 * @clockPtr	Pointer to a PLL clock
 *
 * @return	This function always returns NULL because the PLL has no clock
 *		parent (its parent is a PLL which is not modeled as a clock)
 */
static PmClock* PmClockReleasePll(PmClock* const clockPtr)
{
	PmClockPll* pclk = (PmClockPll*)clockPtr->derived;

	if (0U == --pclk->useCount) {
		PmPllRelease(pclk->pll);
	}

	return NULL;
}

/**
 * PmClockGetPllPerms() - Get permissions (which master can control this clock)
 * @clockPtr	Pointer to a PLL clock
 *
 * @return	This function ORed ipi masks of masters that are allowed to
 *		control this clock
 */
static u32 PmClockGetPllPerms(const PmClock* const clockPtr)
{
	const PmClockPll* pclk = (PmClockPll*)clockPtr->derived;

	return PmPllGetPermissions(pclk->pll);
}

static PmClockClass pmClockClassPll = {
	.request = PmClockRequestPll,
	.release = PmClockReleasePll,
	.getPerms = PmClockGetPllPerms,
	.ctrl = NULL,
};

static PmClockPll pmClockApll = {
	.base = {
		.derived = &pmClockApll,
		.class = &pmClockClassPll,
		.id = PM_CLOCK_APLL,
	},
	.pll = &pmApll_g,
	.useCount = 0U,
};

static PmClockPll pmClockDpll = {
	.base = {
		.derived = &pmClockDpll,
		.class = &pmClockClassPll,
		.id = PM_CLOCK_DPLL,
	},
	.pll = &pmDpll_g,
	.useCount = 0U,
};

static PmClockPll pmClockVpll = {
	.base = {
		.derived = &pmClockVpll,
		.class = &pmClockClassPll,
		.id = PM_CLOCK_VPLL,
	},
	.pll = &pmVpll_g,
	.useCount = 0U,
};

static PmClockPll pmClockRpll = {
	.base = {
		.derived = &pmClockRpll,
		.class = &pmClockClassPll,
		.id = PM_CLOCK_RPLL,
	},
	.pll = &pmRpll_g,
	.useCount = 0U,
};

static PmClockPll pmClockIOpll = {
	.base = {
		.derived = &pmClockIOpll,
		.class = &pmClockClassPll,
		.id = PM_CLOCK_IOPLL,
	},
	.pll = &pmIOpll_g,
	.useCount = 0U,
};

/******************************************************************************/
/* On-chip/generic clocks that can drive PM nodes */

/**
 * PmClockRequestGen() - Request clock method for generic clocks
 * @clockPtr	Pointer to a generic clock
 *
 * @return	Pointer to the parent clock
 */
static PmClock* PmClockRequestGen(PmClock* const clockPtr)
{
	PmClockGen* clk = (PmClockGen*)clockPtr->derived;
	PmClock* parent = NULL;

	if (0U == clk->useCount++) {
		parent = clk->parent;
	}

	return parent;
}

/**
 * PmClockReleaseGen() - Release clock method for generic clocks
 * @clockPtr	Pointer to a generic clock
 *
 * @return	Pointer to the parent clock
 */
static PmClock* PmClockReleaseGen(PmClock* const clockPtr)
{
	PmClockGen* clk = (PmClockGen*)clockPtr->derived;
	PmClock* parent = NULL;

	if (0U == --clk->useCount) {
		parent = clk->parent;
	}

	return parent;
}

/**
 * PmClockGenInitParent() - Initialize parent method for generic clocks
 * @clockPtr	Pointer to the target clock
 *
 * @note	After the the PMU-FW is loaded the only way to change the
 *		parent is using set parent method, which updates the parent
 *		pointer. Therefore this function just returns the parent
 *		pointer. The get parent function should not be called before the
 *		clocks are initialized.
 */
static void PmClockGenInitParent(PmClock* const clockPtr)
{
	PmClockGen* clk = (PmClockGen*)clockPtr->derived;
	u32 select, i;

	if (NULL == clk->mux) {
		goto done;
	}
	select = XPfw_Read32(clk->ctrlAddr);
	select = (select >> clk->mux->shift) & MASK_OF_BITS(clk->mux->bits);
	for (i = 0U; i < clk->mux->size; i++) {
		if (select == clk->mux->inputs[i].select) {
			clk->parent = clk->mux->inputs[i].clkIn;
			break;
		}
	}

	if (NULL != clk->parent) {
		if (clk->parent->class == &pmClockClassPll) {
			PmClockPll* pclk = (PmClockPll*)clk->parent->derived;
			pclk->pll->childCount++;
		}
	}

done:
	return;
}

/**
 * PmClockGenSetParent() - Set parent method for generic clocks
 * @clockPtr	Pointer to the target clock
 * @select	Mux select value
 *
 * @return	Status of performing the operation:
 *		XST_SUCCESS if parent is set
 *		XST_NO_FEATURE if clock has no multiplexer
 *		XST_INVALID_PARAM if given parent is invalid/cannot be set
 */
static s32 PmClockGenSetParent(PmClock* const clockPtr, const u32 select)
{
	s32 status;
	u32 i;
	PmClockGen* clk = (PmClockGen*)clockPtr->derived;
	PmClock* new_parent = NULL;

	if (NULL == clk->mux) {
		status = XST_NO_FEATURE;
		goto done;
	}
	if (select > MASK_OF_BITS(clk->mux->bits)) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	/* Check if mux inputs are modeled (if not just configure the select) */
	if (NULL == clk->mux->inputs) {
		XPfw_RMW32(clk->ctrlAddr,
			   MASK_OF_BITS(clk->mux->bits) << clk->mux->shift,
			   select << clk->mux->shift);
		status = XST_SUCCESS;
		goto done;
	}
	/* Figure out what is the newly selected parent (if select is valid) */
	status = XST_INVALID_PARAM;
	for (i = 0U; i < clk->mux->size; i++) {
		if (select == clk->mux->inputs[i].select) {
			new_parent = clk->mux->inputs[i].clkIn;
			status = XST_SUCCESS;
			break;
		}
	}
	if (XST_SUCCESS != status) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	if (new_parent == clk->parent) {
		goto done;
	}
	if (NULL != new_parent) {
		PmClockRequestInt(new_parent);
	}
	if (new_parent->class == &pmClockClassPll) {
		PmClockPll* pclk = (PmClockPll*)new_parent->derived;
		pclk->pll->childCount++;
	}

	XPfw_RMW32(clk->ctrlAddr,
		   MASK_OF_BITS(clk->mux->bits) << clk->mux->shift,
		   select << clk->mux->shift);
	if (NULL != clk->parent) {
		PmClockReleaseInt(clk->parent);
		if (clk->parent->class == &pmClockClassPll) {
			PmClockPll* pclk = (PmClockPll*)clk->parent->derived;
			pclk->pll->childCount--;
		}
	}
	clk->parent = new_parent;

done:
	return status;
}

/**
 * PmClockGenGetParent() - Get parent method for generic clocks
 * @clockPtr	Pointer to the target clock
 * @select	Location to store clock select value
 *
 * @return	Status of getting the mux select value
 */
static s32 PmClockGenGetParent(PmClock* const clockPtr, u32 *const select)
{
	PmClockGen* clk = (PmClockGen*)clockPtr->derived;
	s32 status = XST_NO_FEATURE;
	u32 val;

	if (NULL == clk->mux) {
		goto done;
	}
	val = XPfw_Read32(clk->ctrlAddr);
	val = (val >> clk->mux->shift) & MASK_OF_BITS(clk->mux->bits);
	*select = val;
	status = XST_SUCCESS;

done:
	return status;
}

/**
 * PmClockGateGetShift() - Get gate shift from gate flag
 * @clk		Generic clock
 * @shift	Location where the shift should be stored
 *
 * @return	Status of getting the gate
 */
static s32 PmClockGateGetShift(const PmClockGen* const clk, u8* const shift)
{
	s32 status = XST_SUCCESS;

	switch (clk->type & PM_CLOCK_TYPE_GATES) {
	case PM_CLOCK_TYPE_GATE24:
		*shift = 24U;
		break;
	case PM_CLOCK_TYPE_GATE25:
		*shift = 25U;
		break;
	case PM_CLOCK_TYPE_GATE26:
		*shift = 26U;
		break;
	default:
		status = XST_NO_FEATURE;
		break;
	}

	return status;
}

/**
 * PmClockGenSetGateState() - Set state of generic clock gate
 * @clockPtr	Generic clock
 * @enable	Gate flag to set: 0=disable, 1=enable
 *
 * @return	Status of setting the gate state:
 *		XST_SUCCESS if state is set
 *		XST_NO_FEATURE if the given clock has no gate
 */
static s32 PmClockGenSetGateState(PmClock* const clockPtr, const u8 enable)
{
	u8 shift = 0x0U;
	PmClockGen* clk = (PmClockGen*)clockPtr->derived;
	s32 status = PmClockGateGetShift(clk, &shift);

	if (XST_SUCCESS == status) {
		XPfw_RMW32(clk->ctrlAddr, (u32)1 << shift, (u32)enable << shift);
	}

	return status;
}

/**
 * PmClockGenGetGateState() - Get state of generic clock gate
 * @clockPtr	Generic clock
 * @enable	Location where the state should be stored
 *
 * @return	Status of getting the gate state:
 *		XST_SUCCESS if enable location is updated
 *		XST_NO_FEATURE if the given clock has no gate
 */
static s32 PmClockGenGetGateState(PmClock* const clockPtr, u8* const enable)
{
	u8 shift = 0x0U;
	PmClockGen* clk = (PmClockGen*)clockPtr->derived;
	s32 status = PmClockGateGetShift(clk, &shift);

	if (XST_SUCCESS == status) {
		*enable = (u8)((XPfw_Read32(clk->ctrlAddr) >> shift) & 1U);
	}

	return status;
}

/**
 * PmClockGenSetDivider() - Generic clock method to set clock divider
 * @clockPtr	Target clock
 * @divId	Identifier of the divider to be set
 * @val		Divider value to be set
 *
 * @return	Status of setting the divider:
 *		XST_SUCCESS the divider is configured as requested
 *		XST_NO_FEATURE if clock has no divider
 *		XST_INVALID_PARAM the requested value is out of physically
 *		configurable divider's scope
 */
static s32 PmClockGenSetDivider(PmClock* const clockPtr, const u32 divId,
				const u32 val)
{
	s32 status = XST_SUCCESS;
	PmClockGen* clk = (PmClockGen*)clockPtr->derived;
	u8 shift;

	if (((PM_CLOCK_DIV0_ID == divId) && !PM_CLOCK_HAS_DIV0(clk)) ||
	    ((PM_CLOCK_DIV1_ID == divId) && !PM_CLOCK_HAS_DIV1(clk))) {
		/* Clock has no divider with specified ID */
		status = XST_NO_FEATURE;
		goto done;
	}
	if (val > PM_DIV_MASK) {
		/* Given div value is out of scope */
		status = XST_INVALID_PARAM;
		goto done;
	}
	if (PM_CLOCK_DIV0_ID == divId) {
		shift = PM_DIV0_SHIFT;
	} else if (PM_CLOCK_DIV1_ID == divId) {
		shift = PM_DIV1_SHIFT;
	} else {
		status = XST_INVALID_PARAM;
		goto done;
	}
	XPfw_RMW32(clk->ctrlAddr, (u32)PM_DIV_MASK << shift, (u32)val << shift);

done:
	return status;
}

/**
 * PmClockGenGetDivider() - Generic clock method to get clock divider
 * @clockPtr	Target clock
 * @divId	Identifier of the divider whose value should be get
 * @val		Location where the divider value needs to be stored
 *
 * @return	Status of getting the divider:
 *		XST_SUCCESS the divider value is stored in 'div' location
 *		XST_NO_FEATURE if clock has no divider
 */
static s32 PmClockGenGetDivider(PmClock* const clockPtr, const u32 divId,
				u32* const val)
{
	s32 status = XST_SUCCESS;
	PmClockGen* clk = (PmClockGen*)clockPtr->derived;
	u32 reg;
	u8 shift;

	if (((PM_CLOCK_DIV0_ID == divId) && !PM_CLOCK_HAS_DIV0(clk)) ||
	    ((PM_CLOCK_DIV1_ID == divId) && !PM_CLOCK_HAS_DIV1(clk))) {
		/* Clock has no divider with specified ID */
		status = XST_NO_FEATURE;
		goto done;
	}
	if (PM_CLOCK_DIV0_ID == divId) {
		shift = PM_DIV0_SHIFT;
	} else if (PM_CLOCK_DIV1_ID == divId) {
		shift = PM_DIV1_SHIFT;
	} else {
		status = XST_INVALID_PARAM;
		goto done;
	}
	reg = XPfw_Read32(clk->ctrlAddr);
	*val = (reg >> shift) & PM_DIV_MASK;

done:
	return status;
}

/**
 * PmClockGenGetPerms() - Get permissions (which master can control this clock)
 * @clockPtr	Pointer to a PLL clock
 *
 * @return	This function ORed ipi masks of masters that are allowed to
 *		control this clock
 */
static u32 PmClockGenGetPerms(const PmClock* const clockPtr)
{
	PmClockHandle* ch;
	const PmClockGen* clk = (PmClockGen*)clockPtr->derived;
	u32 permissions = 0U;

	/* If this is a system clock no one has permission to control it */
	if (0U != (PM_CLOCK_TYPE_SYSTEM & clk->type)) {
		goto done;
	}

	ch = clk->users;
	while (NULL != ch) {
		permissions |= PmNodeGetPermissions(ch->node);
		ch = ch->nextNode;
	}

done:
	return permissions;
}

static PmClockCtrlMethods pmClockGenCtrlMethods = {
	.initParent = PmClockGenInitParent,
	.getParent = PmClockGenGetParent,
	.setParent = PmClockGenSetParent,
	.getGate = PmClockGenGetGateState,
	.setGate = PmClockGenSetGateState,
	.getDivider = PmClockGenGetDivider,
	.setDivider = PmClockGenSetDivider,
};

static PmClockClass pmClockClassGen = {
	.request = PmClockRequestGen,
	.release = PmClockReleaseGen,
	.getPerms = PmClockGenGetPerms,
	.ctrl = &pmClockGenCtrlMethods,
};


/******************************************************************************/
/* Pll output cross domain clock models */
/**
 * PmClockRequestCrossDom() - Request clock method for cross-domain clocks
 * @clockPtr	Pointer to a cross-domain clock
 *
 * @return	Pointer to the parent clock
 */
static PmClock* PmClockRequestCrossDom(PmClock* const clockPtr)
{
	PmClockCrossDom* clk = (PmClockCrossDom*)clockPtr->derived;
	PmClock* parent = NULL;

	if (0U == clk->useCount++) {
		parent = &clk->parent->base;
	}

	return parent;
}

/**
 * PmClockReleaseCrossDom() - Release clock method for cross-domain clocks
 * @clockPtr	Pointer to a cross-domain clock
 *
 * @return	Pointer to the parent clock
 */
static PmClock* PmClockReleaseCrossDom(PmClock* const clockPtr)
{
	PmClockCrossDom* clk = (PmClockCrossDom*)clockPtr->derived;
	PmClock* parent = NULL;

	if (0U == --clk->useCount) {
		parent = &clk->parent->base;
	}

	return parent;
}

/**
 * PmClockCrossDomSetDivider() - Cross-domain clock method to set clock divider
 * @clockPtr	Target clock
 * @divId	Identifier of the divider to be set
 * @val		Divider value to be set
 *
 * @return	Status of setting the divider:
 *		XST_SUCCESS the divider is configured as requested
 *		XST_NO_FEATURE if clock has no divider
 *		XST_INVALID_PARAM the requested value is out of physically
 *		configurable divider's scope
 */
static s32 PmClockCrossDomSetDivider(PmClock* const clockPtr, const u32 divId,
				     const u32 val)
{
	s32 status = XST_SUCCESS;
	PmClockCrossDom* clk = (PmClockCrossDom*)clockPtr->derived;

	if (PM_CLOCK_DIV0_ID != divId) {
		/* Cross-domain clocks have only one divisor */
		status = XST_NO_FEATURE;
		goto done;
	}
	if (val > PM_DIV_MASK) {
		/* Given div value is out of scope */
		status = XST_INVALID_PARAM;
		goto done;
	}
	XPfw_RMW32(clk->ctrlAddr, (u32)PM_DIV_MASK << PM_DIV0_SHIFT,
		   (u32)val << PM_DIV0_SHIFT);

done:
	return status;
}

/**
 * PmClockCrossDomGetDivider() - Cross-domain clock method to get clock divider
 * @clockPtr	Target clock
 * @divId	Identifier of the divider whose value should be get
 * @val		Location where the divider value needs to be stored
 *
 * @return	Status of getting the divider:
 *		XST_SUCCESS the divider value is stored in 'div' location
 *		XST_NO_FEATURE if clock has no divider
 */
static s32 PmClockCrossDomGetDivider(PmClock* const clockPtr, const u32 divId,
				     u32* const val)
{
	s32 status = XST_SUCCESS;
	PmClockCrossDom* clk = (PmClockCrossDom*)clockPtr->derived;

	if (PM_CLOCK_DIV0_ID != divId) {
		/* Cross-domain clocks have only one divisor */
		status = XST_NO_FEATURE;
		goto done;
	}
	*val = (XPfw_Read32(clk->ctrlAddr) >> PM_DIV0_SHIFT) & PM_DIV_MASK;

done:
	return status;
}

/**
 * PmClockCrossDomGetPerms() - Get permissions (which master can control clock)
 * @clockPtr	Pointer to a cross-domain clock
 *
 * @return	This function ORed ipi masks of masters that are allowed to
 *		control this clock
 */
static u32 PmClockCrossDomGetPerms(const PmClock* const clockPtr)
{
	const PmClockCrossDom* clk = (PmClockCrossDom*)clockPtr->derived;
	u32 permissions = 0U;

	/* Inherit permissions from PLL output clock (parent) */
	if ((NULL != clk->parent->base.class) &&
	    (NULL != clk->parent->base.class->getPerms)) {
		permissions =
		clk->parent->base.class->getPerms(&clk->parent->base);
	}

	return permissions;
}

static PmClockCtrlMethods pmClockCrossDomCtrlMethods = {
	.initParent = NULL,
	.getParent = NULL,
	.setParent = NULL,
	.getGate = NULL,
	.setGate = NULL,
	.getDivider = PmClockCrossDomGetDivider,
	.setDivider = PmClockCrossDomSetDivider,
};

static PmClockClass pmClockClassCrossDom = {
	.request = PmClockRequestCrossDom,
	.release = PmClockReleaseCrossDom,
	.getPerms = PmClockCrossDomGetPerms,
	.ctrl = &pmClockCrossDomCtrlMethods,
};

static PmClockCrossDom pmClockIOpllToFpd = {
	.base = {
		.derived = &pmClockIOpllToFpd,
		.class = &pmClockClassCrossDom,
		.id = PM_CLOCK_IOPLL_TO_FPD,
	},
	.parent = &pmClockIOpll,
	.ctrlAddr = CRL_APB_IOPLL_TO_FPD_CTRL,
	.useCount = 0U,
};

static PmClockCrossDom pmClockRpllToFpd = {
	.base = {
		.derived = &pmClockRpllToFpd,
		.class = &pmClockClassCrossDom,
		.id = PM_CLOCK_RPLL_TO_FPD,
	},
	.parent = &pmClockRpll,
	.ctrlAddr = CRL_APB_RPLL_TO_FPD_CTRL,
	.useCount = 0U,
};

static PmClockCrossDom pmClockDpllToLpd = {
	.base = {
		.derived = &pmClockDpllToLpd,
		.class = &pmClockClassCrossDom,
		.id = PM_CLOCK_DPLL_TO_LPD,
	},
	.parent = &pmClockDpll,
	.ctrlAddr = CRF_APB_DPLL_TO_LPD_CTRL,
	.useCount = 0U,
};

static PmClockCrossDom pmClockVpllToLpd = {
	.base = {
		.derived = &pmClockVpllToLpd,
		.class = &pmClockClassCrossDom,
		.id = PM_CLOCK_VPLL_TO_LPD,
	},
	.parent = &pmClockVpll,
	.ctrlAddr = CRF_APB_VPLL_TO_LPD_CTRL,
	.useCount = 0U,
};

static const PmClockSel2ClkIn advSel2ClkIn[] = {
	{
		.clkIn = &pmClockApll.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockDpll.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockVpll.base,
		.select = 3U,
	},
};

static PmClockMux advMux = {
	.inputs = advSel2ClkIn,
	.size = ARRAY_SIZE(advSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn avdSel2ClkIn[] = {
	{
		.clkIn = &pmClockApll.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockVpll.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockDpll.base,
		.select = 3U,
	},
};

static PmClockMux avdMux = {
	.inputs = avdSel2ClkIn,
	.size = ARRAY_SIZE(avdSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn aiodSel2ClkIn[] = {
	{
		.clkIn = &pmClockApll.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockIOpllToFpd.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockDpll.base,
		.select = 3U,
	},
};

static PmClockMux aiodMux = {
	.inputs = aiodSel2ClkIn,
	.size = ARRAY_SIZE(aiodSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn vdrSel2ClkIn[] = {
	{
		.clkIn = &pmClockVpll.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockDpll.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockRpllToFpd.base,
		.select = 3U,
	},
};

static PmClockMux vdrMux = {
	.inputs = vdrSel2ClkIn,
	.size = ARRAY_SIZE(vdrSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn dvSel2ClkIn[] = {
	{
		.clkIn = &pmClockDpll.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockVpll.base,
		.select = 1U,
	},
};

static PmClockMux dvMux = {
	.inputs = dvSel2ClkIn,
	.size = ARRAY_SIZE(dvSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn iovdSel2ClkIn[] = {
	{
		.clkIn = &pmClockIOpllToFpd.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockVpll.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockDpll.base,
		.select = 3U,
	},
};

static PmClockMux iovdMux = {
	.inputs = iovdSel2ClkIn,
	.size = ARRAY_SIZE(iovdSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn ioadSel2ClkIn[] = {
	{
		.clkIn = &pmClockIOpllToFpd.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockApll.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockDpll.base,
		.select = 3U,
	},
};

static PmClockMux ioadMux = {
	.inputs = ioadSel2ClkIn,
	.size = ARRAY_SIZE(ioadSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn iodaSel2ClkIn[] = {
	{
		.clkIn = &pmClockIOpllToFpd.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockDpll.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockApll.base,
		.select = 3U,
	},
};

static PmClockMux iodaMux = {
	.inputs = iodaSel2ClkIn,
	.size = ARRAY_SIZE(iodaSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn iorSel2ClkIn[] = {
	{
		.clkIn = &pmClockIOpll.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockRpll.base,
		.select = 2U,
	},
};

static PmClockMux iorMux = {
	.inputs = iorSel2ClkIn,
	.size = ARRAY_SIZE(iorSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn iordFpdSel2ClkIn[] = {
	{
		.clkIn = &pmClockIOpllToFpd.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockRpllToFpd.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockDpll.base,
		.select = 3U,
	},
};

static PmClockMux iordFpdMux = {
	.inputs = iordFpdSel2ClkIn,
	.size = ARRAY_SIZE(iordFpdSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn iordSel2ClkIn[] = {
	{
		.clkIn = &pmClockIOpll.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockRpll.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockDpllToLpd.base,
		.select = 3U,
	},
};

static PmClockMux iordMux = {
	.inputs = iordSel2ClkIn,
	.size = ARRAY_SIZE(iordSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn iorvSel2ClkIn[] = {
	{
		.clkIn = &pmClockIOpll.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockRpll.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockVpllToLpd.base,
		.select = 3U,
	},
};

static PmClockMux iorvMux = {
	.inputs = iorvSel2ClkIn,
	.size = ARRAY_SIZE(iorvSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const PmClockSel2ClkIn riodSel2ClkIn[] = {
	{
		.clkIn = &pmClockRpll.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockIOpll.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockDpllToLpd.base,
		.select = 3U,
	},
};

static PmClockMux riodMux = {
	.inputs = riodSel2ClkIn,
	.size = ARRAY_SIZE(riodSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

/* CRF_APB clocks */
static PmClockGen pmClockAcpu = {
	.base = {
		.derived = &pmClockAcpu,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_ACPU,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &advMux,
	.ctrlAddr = CRF_APB_ACPU_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0,
	.useCount = 0U,
};

static PmClockGen pmClockAcpuFull = {
	.base = {
		.derived = &pmClockAcpuFull,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_ACPU_FULL,
	},
	.parent = &pmClockAcpu.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRF_APB_ACPU_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockAcpuHalf = {
	.base = {
		.derived = &pmClockAcpuHalf,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_ACPU_HALF,
	},
	.parent = &pmClockAcpu.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRF_APB_ACPU_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockDbgTrace = {
	.base = {
		.derived = &pmClockDbgTrace,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_DBG_TRACE,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iodaMux,
	.ctrlAddr = CRF_APB_DBG_TRACE_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24 |
		PM_CLOCK_TYPE_SYSTEM,
	.useCount = 0U,
};

static PmClockGen pmClockDbgFpd = {
	.base = {
		.derived = &pmClockDbgFpd,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_DBG_FPD,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iodaMux,
	.ctrlAddr = CRF_APB_DBG_FPD_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24 |
		PM_CLOCK_TYPE_SYSTEM,
	.useCount = 0U,
};

static PmClockGen pmClockDpVideo = {
	.base = {
		.derived = &pmClockDpVideo,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_DP_VIDEO_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &vdrMux,
	.ctrlAddr = CRF_APB_DP_VIDEO_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockDpAudio = {
	.base = {
		.derived = &pmClockDpAudio,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_DP_AUDIO_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &vdrMux,
	.ctrlAddr = CRF_APB_DP_AUDIO_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockDpStc = {
	.base = {
		.derived = &pmClockDpStc,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_DP_STC_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &vdrMux,
	.ctrlAddr = CRF_APB_DP_STC_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockDdr __attribute__((__section__(".srdata"))) = {
	.base = {
		.derived = &pmClockDdr,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_DDR_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &dvMux,
	.ctrlAddr = CRF_APB_DDR_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0,
	.useCount = 0U,
};

static PmClockGen pmClockGpu = {
	.base = {
		.derived = &pmClockGpu,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GPU_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iovdMux,
	.ctrlAddr = CRF_APB_GPU_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockGpuPp0 = {
	.base = {
		.derived = &pmClockGpuPp0,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GPU_PP0_REF,
	},
	.parent = &pmClockGpu.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRF_APB_GPU_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockGpuPp1 = {
	.base = {
		.derived = &pmClockGpuPp1,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GPU_PP1_REF,
	},
	.parent = &pmClockGpu.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRF_APB_GPU_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE26,
	.useCount = 0U,
};

static PmClockGen pmClockSata = {
	.base = {
		.derived = &pmClockSata,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_SATA_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &ioadMux,
	.ctrlAddr = CRF_APB_SATA_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockPcie = {
	.base = {
		.derived = &pmClockPcie,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_PCIE_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordFpdMux,
	.ctrlAddr = CRF_APB_PCIE_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockGdma = {
	.base = {
		.derived = &pmClockGdma,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GDMA_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &avdMux,
	.ctrlAddr = CRF_APB_GDMA_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockDpDma = {
	.base = {
		.derived = &pmClockDpDma,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_DPDMA_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &avdMux,
	.ctrlAddr = CRF_APB_DPDMA_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockTopSwMain __attribute__((__section__(".srdata"))) = {
	.base = {
		.derived = &pmClockTopSwMain,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_TOPSW_MAIN,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &avdMux,
	.ctrlAddr = CRF_APB_TOPSW_MAIN_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24 |
		PM_CLOCK_TYPE_SYSTEM,
	.useCount = 0U,
};

static PmClockGen pmClockTopSwLsBus __attribute__((__section__(".srdata"))) = {
	.base = {
		.derived = &pmClockTopSwLsBus,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_TOPSW_LSBUS,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &aiodMux,
	.ctrlAddr = CRF_APB_TOPSW_LSBUS_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24 |
		PM_CLOCK_TYPE_SYSTEM,
	.useCount = 0U,
};

static PmClockGen pmClockDbgTstmp = {
	.base = {
		.derived = &pmClockDbgTstmp,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_DBG_TSTMP,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iodaMux,
	.ctrlAddr = CRF_APB_DBG_TSTMP_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_SYSTEM,
	.useCount = 0U,
};

/* CRL_APB clocks */
static PmClockGen pmClockUsb3Dual = {
	.base = {
		.derived = &pmClockUsb3Dual,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_USB3_DUAL_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_USB3_DUAL_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockGem0RefUngated = {
	.base = {
		.derived = &pmClockGem0RefUngated,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM0_REF_UNGATED,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM0_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.useCount = 0U,
};

static PmClockGen pmClockGem1RefUngated = {
	.base = {
		.derived = &pmClockGem1RefUngated,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM1_REF_UNGATED,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM1_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.useCount = 0U,
};

static PmClockGen pmClockGem2RefUngated = {
	.base = {
		.derived = &pmClockGem2RefUngated,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM2_REF_UNGATED,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM2_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.useCount = 0U,
};

static PmClockGen pmClockGem3RefUngated = {
	.base = {
		.derived = &pmClockGem3RefUngated,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM3_REF_UNGATED,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM3_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.useCount = 0U,
};

static PmClockGen pmClockUsb0Bus = {
	.base = {
		.derived = &pmClockUsb0Bus,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_USB0_BUS_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_USB0_BUS_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockUsb1Bus = {
	.base = {
		.derived = &pmClockUsb1Bus,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_USB1_BUS_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_USB1_BUS_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockQSpi = {
	.base = {
		.derived = &pmClockQSpi,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_QSPI_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_QSPI_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockSdio0 = {
	.base = {
		.derived = &pmClockSdio0,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_SDIO0_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iorvMux,
	.ctrlAddr = CRL_APB_SDIO0_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockSdio1 = {
	.base = {
		.derived = &pmClockSdio1,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_SDIO1_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iorvMux,
	.ctrlAddr = CRL_APB_SDIO1_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockUart0 = {
	.base = {
		.derived = &pmClockUart0,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_UART0_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_UART0_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockUart1 = {
	.base = {
		.derived = &pmClockUart1,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_UART1_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_UART1_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockSpi0 = {
	.base = {
		.derived = &pmClockSpi0,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_SPI0_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_SPI0_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockSpi1 = {
	.base = {
		.derived = &pmClockSpi1,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_SPI1_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_SPI1_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockCan0Ref = {
	.base = {
		.derived = &pmClockCan0Ref,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_CAN0_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_CAN0_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockCan1Ref = {
	.base = {
		.derived = &pmClockCan1Ref,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_CAN1_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_CAN1_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockCpuR5 = {
	.base = {
		.derived = &pmClockCpuR5,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_CPU_R5,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_CPU_R5_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.useCount = 0U,
};

static PmClockGen pmClockCpuR5Core = {
	.base = {
		.derived = &pmClockCpuR5Core,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_CPU_R5_CORE,
	},
	.parent = &pmClockCpuR5.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRL_APB_CPU_R5_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockIouSwitch = {
	.base = {
		.derived = &pmClockIouSwitch,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_IOU_SWITCH,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_IOU_SWITCH_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24 |
		PM_CLOCK_TYPE_SYSTEM,
	.useCount = 0U,
};

static PmClockGen pmClockCsuPll = {
	.base = {
		.derived = &pmClockCsuPll,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_CSU_PLL,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_CSU_PLL_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockPcap = {
	.base = {
		.derived = &pmClockPcap,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_PCAP,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_PCAP_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockLpdSwitch = {
	.base = {
		.derived = &pmClockLpdSwitch,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_LPD_SWITCH,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_LPD_SWITCH_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24 |
		PM_CLOCK_TYPE_SYSTEM,
	.useCount = 0U,
};

static PmClockGen pmClockLpdLsBus = {
	.base = {
		.derived = &pmClockLpdLsBus,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_LPD_LSBUS,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_LPD_LSBUS_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24 |
		PM_CLOCK_TYPE_SYSTEM,
	.useCount = 0U,
};

static PmClockGen pmClockDbgLpd = {
	.base = {
		.derived = &pmClockDbgLpd,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_DBG_LPD,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_DBG_LPD_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24 |
		PM_CLOCK_TYPE_SYSTEM,
	.useCount = 0U,
};

static PmClockGen pmClockNand = {
	.base = {
		.derived = &pmClockNand,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_NAND_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_NAND_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockAdma = {
	.base = {
		.derived = &pmClockAdma,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_ADMA_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_ADMA_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockPl0 = {
	.base = {
		.derived = &pmClockPl0,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_PL0_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_PL0_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockPl1 = {
	.base = {
		.derived = &pmClockPl1,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_PL1_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_PL1_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockPl2 = {
	.base = {
		.derived = &pmClockPl2,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_PL2_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_PL2_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockPl3 = {
	.base = {
		.derived = &pmClockPl3,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_PL3_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_PL3_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockGemTsuRef = {
	.base = {
		.derived = &pmClockGemTsuRef,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM_TSU_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM_TSU_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockDll = {
	.base = {
		.derived = &pmClockDll,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_DLL_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iorMux,
	.ctrlAddr = CRL_APB_DLL_REF_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static PmClockGen pmClockAms = {
	.base = {
		.derived = &pmClockAms,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_AMS_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_AMS_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24 |
		PM_CLOCK_TYPE_SYSTEM,	/* because of the commit 7611b2fc18 */
	.useCount = 0U,
};

static PmClockGen pmClockI2C0 = {
	.base = {
		.derived = &pmClockI2C0,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_I2C0_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_I2C0_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static PmClockGen pmClockI2C1 = {
	.base = {
		.derived = &pmClockI2C1,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_I2C1_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_I2C1_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
	.useCount = 0U,
};

static const PmClockSel2ClkIn iordPsRefSel2ClkIn[] = {
	{
		.clkIn = &pmClockIOpll.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockRpll.base,
		.select = 2U,
	}, {
		.clkIn = &pmClockDpllToLpd.base,
		.select = 3U,
	}, {
		.clkIn = NULL,	/* oscillator */
		.select = 4U,
	}, {
		.clkIn = NULL,	/* oscillator */
		.select = 5U,
	}, {
		.clkIn = NULL,	/* oscillator */
		.select = 6U,
	}, {
		.clkIn = NULL,	/* oscillator */
		.select = 7U,
	},
};

static PmClockMux iordPsRefMux = {
	.inputs = iordPsRefSel2ClkIn,
	.size = ARRAY_SIZE(iordPsRefSel2ClkIn),
	.bits = 3U,
	.shift = 0U,
};

static PmClockGen pmClockTimeStamp = {
	.base = {
		.derived = &pmClockTimeStamp,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_TIMESTAMP_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordPsRefMux,
	.ctrlAddr = CRL_APB_TIMESTAMP_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24 |
		PM_CLOCK_TYPE_SYSTEM,
	.useCount = 0U,
};

static PmClockMux can0MioMux = {
	.inputs = NULL,
	.size = 0U,
	.bits = 7U,
	.shift = 0U,
};
static PmClockGen pmClockCan0Mio = {
	.base = {
		.derived = &pmClockCan0Mio,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_CAN0_MIO,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &can0MioMux,
	.ctrlAddr = IOU_SLCR_CAN_MIO_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static const PmClockSel2ClkIn can0Sel2ClkIn[] = {
	{
		.clkIn = &pmClockCan0Ref.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockCan0Mio.base,
		.select = 1U,
	},
};

static PmClockMux can0Mux = {
	.inputs = can0Sel2ClkIn,
	.size = ARRAY_SIZE(can0Sel2ClkIn),
	.bits = 1U,
	.shift = 7U,
};

static PmClockGen pmClockCan0 = {
	.base = {
		.derived = &pmClockCan0,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_CAN0,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &can0Mux,
	.ctrlAddr = IOU_SLCR_CAN_MIO_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static PmClockMux can1MioMux = {
	.inputs = NULL,
	.size = 0U,
	.bits = 7U,
	.shift = 15U,
};
static PmClockGen pmClockCan1Mio = {
	.base = {
		.derived = &pmClockCan1Mio,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_CAN1_MIO,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &can1MioMux,
	.ctrlAddr = IOU_SLCR_CAN_MIO_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static const PmClockSel2ClkIn can1Sel2ClkIn[] = {
	{
		.clkIn = &pmClockCan1Ref.base,
		.select = 0U,
	}, {
		.clkIn = &pmClockCan1Mio.base,
		.select = 1U,
	},
};

static PmClockMux can1Mux = {
	.inputs = can1Sel2ClkIn,
	.size = ARRAY_SIZE(can1Sel2ClkIn),
	.bits = 1U,
	.shift = 22U,
};

static PmClockGen pmClockCan1 = {
	.base = {
		.derived = &pmClockCan1,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_CAN1,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &can1Mux,
	.ctrlAddr = IOU_SLCR_CAN_MIO_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static const PmClockSel2ClkIn gemTsuSel2ClkIn[] = {
	{
		.clkIn = &pmClockGemTsuRef.base,
		.select = 0U,
	}, {
		.clkIn = NULL,
		.select = 1U,
	}, {
		.clkIn = &pmClockGemTsuRef.base,
		.select = 2U,
	}, {
		.clkIn = NULL,
		.select = 3U,
	},
};

static PmClockMux gemTsuMux = {
	.inputs = gemTsuSel2ClkIn,
	.size = ARRAY_SIZE(gemTsuSel2ClkIn),
	.bits = 2U,
	.shift = 20U,
};

static PmClockGen pmClockGemTsu = {
	.base = {
		.derived = &pmClockGemTsu,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM_TSU,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &gemTsuMux,
	.ctrlAddr = IOU_SLCR_GEM_CLK_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static const PmClockSel2ClkIn gem0RefSel2ClkIn[] = {
	{
		.clkIn = &pmClockGem0RefUngated.base,
		.select = 0U,
	}, {
		.clkIn = NULL,
		.select = 1U,
	},
};
static PmClockMux gem0RefMux = {
	.inputs = gem0RefSel2ClkIn,
	.size = ARRAY_SIZE(gem0RefSel2ClkIn),
	.bits = 1U,
	.shift = 1U,
};
static PmClockGen pmClockGem0Ref = {
	.base = {
		.derived = &pmClockGem0Ref,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM0_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &gem0RefMux,
	.ctrlAddr = IOU_SLCR_GEM_CLK_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static const PmClockSel2ClkIn gem1RefSel2ClkIn[] = {
	{
		.clkIn = &pmClockGem1RefUngated.base,
		.select = 0U,
	}, {
		.clkIn = NULL,
		.select = 1U,
	},
};
static PmClockMux gem1RefMux = {
	.inputs = gem1RefSel2ClkIn,
	.size = ARRAY_SIZE(gem1RefSel2ClkIn),
	.bits = 1U,
	.shift = 6U,
};
static PmClockGen pmClockGem1Ref = {
	.base = {
		.derived = &pmClockGem1Ref,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM1_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &gem1RefMux,
	.ctrlAddr = IOU_SLCR_GEM_CLK_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static const PmClockSel2ClkIn gem2RefSel2ClkIn[] = {
	{
		.clkIn = &pmClockGem2RefUngated.base,
		.select = 0U,
	}, {
		.clkIn = NULL,
		.select = 1U,
	},
};
static PmClockMux gem2RefMux = {
	.inputs = gem2RefSel2ClkIn,
	.size = ARRAY_SIZE(gem2RefSel2ClkIn),
	.bits = 1U,
	.shift = 11U,
};
static PmClockGen pmClockGem2Ref = {
	.base = {
		.derived = &pmClockGem2Ref,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM2_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &gem2RefMux,
	.ctrlAddr = IOU_SLCR_GEM_CLK_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static const PmClockSel2ClkIn gem3RefSel2ClkIn[] = {
	{
		.clkIn = &pmClockGem3RefUngated.base,
		.select = 0U,
	}, {
		.clkIn = NULL,
		.select = 1U,
	},
};
static PmClockMux gem3RefMux = {
	.inputs = gem3RefSel2ClkIn,
	.size = ARRAY_SIZE(gem3RefSel2ClkIn),
	.bits = 1U,
	.shift = 16U,
};
static PmClockGen pmClockGem3Ref = {
	.base = {
		.derived = &pmClockGem3Ref,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM3_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &gem3RefMux,
	.ctrlAddr = IOU_SLCR_GEM_CLK_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static PmClockGen pmClockGem0Tx = {
	.base = {
		.derived = &pmClockGem0Tx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM0_TX,
	},
	.parent = &pmClockGem0Ref.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRL_APB_GEM0_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockGem1Tx = {
	.base = {
		.derived = &pmClockGem1Tx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM1_TX,
	},
	.parent = &pmClockGem1Ref.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRL_APB_GEM1_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockGem2Tx = {
	.base = {
		.derived = &pmClockGem2Tx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM2_TX,
	},
	.parent = &pmClockGem2Ref.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRL_APB_GEM2_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockGem3Tx = {
	.base = {
		.derived = &pmClockGem3Tx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM3_TX,
	},
	.parent = &pmClockGem3Ref.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRL_APB_GEM3_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockGem0Rx = {
	.base = {
		.derived = &pmClockGem0Rx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM0_RX,
	},
	.parent = NULL,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRL_APB_GEM0_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE26,
	.useCount = 0U,
};

static PmClockGen pmClockGem1Rx = {
	.base = {
		.derived = &pmClockGem1Rx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM1_RX,
	},
	.parent = NULL,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRL_APB_GEM1_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE26,
	.useCount = 0U,
};

static PmClockGen pmClockGem2Rx = {
	.base = {
		.derived = &pmClockGem2Rx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM2_RX,
	},
	.parent = NULL,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRL_APB_GEM2_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE26,
	.useCount = 0U,
};

static PmClockGen pmClockGem3Rx = {
	.base = {
		.derived = &pmClockGem3Rx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM3_RX,
	},
	.parent = NULL,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRL_APB_GEM3_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_GATE26,
	.useCount = 0U,
};

static const PmClockSel2ClkIn wdtSel2ClkIn[] = {
	{
		.clkIn = &pmClockTopSwLsBus.base,
		.select = 0U,
	}, {
		.clkIn = NULL,
		.select = 1U,
	},
};
static PmClockMux wdtMux = {
	.inputs = wdtSel2ClkIn,
	.size = ARRAY_SIZE(wdtSel2ClkIn),
	.bits = 1U,
	.shift = 0U,
};
static PmClockGen pmClockFpdWdt = {
	.base = {
		.derived = &pmClockFpdWdt,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_WDT,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &wdtMux,
	.ctrlAddr = FPD_SLCR_WDT_CLK_SEL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static PmClockGen pmClockLpdWdt = {
	.base = {
		.derived = &pmClockLpdWdt,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_LPD_WDT,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &wdtMux,
	.ctrlAddr = IOU_SLCR_WDT_CLK_SEL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

#ifdef ENABLE_POS
static PmClockGen* pmDdrClocks [] = {
	&pmClockDdr,
	&pmClockTopSwMain,
	&pmClockTopSwLsBus,
};
#endif

static PmClock* pmClocks[] = {
	&pmClockIOpll.base,
	&pmClockRpll.base,
	&pmClockApll.base,
	&pmClockDpll.base,
	&pmClockVpll.base,
	&pmClockIOpllToFpd.base,
	&pmClockRpllToFpd.base,
	&pmClockDpllToLpd.base,
	&pmClockVpllToLpd.base,
	&pmClockAcpu.base,
	&pmClockAcpuFull.base,
	&pmClockAcpuHalf.base,
	&pmClockDbgTrace.base,
	&pmClockDbgFpd.base,
	&pmClockDpVideo.base,
	&pmClockDpAudio.base,
	&pmClockDpStc.base,
	&pmClockDdr.base,
	&pmClockGpu.base,
	&pmClockGpuPp0.base,
	&pmClockGpuPp1.base,
	&pmClockSata.base,
	&pmClockPcie.base,
	&pmClockGdma.base,
	&pmClockDpDma.base,
	&pmClockTopSwMain.base,
	&pmClockTopSwLsBus.base,
	&pmClockDbgTstmp.base,
	&pmClockUsb3Dual.base,
	&pmClockGem0RefUngated.base,
	&pmClockGem1RefUngated.base,
	&pmClockGem2RefUngated.base,
	&pmClockGem3RefUngated.base,
	&pmClockUsb0Bus.base,
	&pmClockUsb1Bus.base,
	&pmClockQSpi.base,
	&pmClockSdio0.base,
	&pmClockSdio1.base,
	&pmClockUart0.base,
	&pmClockUart1.base,
	&pmClockSpi0.base,
	&pmClockSpi1.base,
	&pmClockCan0Ref.base,
	&pmClockCan1Ref.base,
	&pmClockCpuR5.base,
	&pmClockCpuR5Core.base,
	&pmClockIouSwitch.base,
	&pmClockCsuPll.base,
	&pmClockPcap.base,
	&pmClockLpdSwitch.base,
	&pmClockLpdLsBus.base,
	&pmClockDbgLpd.base,
	&pmClockNand.base,
	&pmClockAdma.base,
	&pmClockPl0.base,
	&pmClockPl1.base,
	&pmClockPl2.base,
	&pmClockPl3.base,
	&pmClockGemTsuRef.base,
	&pmClockDll.base,
	&pmClockAms.base,
	&pmClockI2C0.base,
	&pmClockI2C1.base,
	&pmClockTimeStamp.base,
	&pmClockCan0.base,
	&pmClockCan1.base,
	&pmClockCan0Mio.base,
	&pmClockCan1Mio.base,
	&pmClockGemTsu.base,
	&pmClockGem0Ref.base,
	&pmClockGem1Ref.base,
	&pmClockGem2Ref.base,
	&pmClockGem3Ref.base,
	&pmClockGem0Tx.base,
	&pmClockGem1Tx.base,
	&pmClockGem2Tx.base,
	&pmClockGem3Tx.base,
	&pmClockGem0Rx.base,
	&pmClockGem1Rx.base,
	&pmClockGem2Rx.base,
	&pmClockGem3Rx.base,
	&pmClockFpdWdt.base,
	&pmClockLpdWdt.base,
};

static PmClockHandle pmClockHandles[] = {
	CONNECT(pmClockAcpu,		pmPowerIslandApu_g.node),
	CONNECT(pmClockAcpuHalf,	pmPowerIslandApu_g.node),
	CONNECT(pmClockAcpuFull,	pmPowerIslandApu_g.node),
	CONNECT(pmClockDpVideo,		pmSlaveDP_g.node),
	CONNECT(pmClockDpAudio,		pmSlaveDP_g.node),
	CONNECT(pmClockDpStc,		pmSlaveDP_g.node),
	CONNECT(pmClockDpDma,		pmSlaveDP_g.node),
	CONNECT(pmClockDdr,		pmSlaveDdr_g.node),
	CONNECT(pmClockTopSwMain,	pmSlaveDdr_g.node),
	CONNECT(pmClockTopSwLsBus,	pmSlaveDdr_g.node),
	CONNECT(pmClockGpu,		pmSlaveGpu_g.node),
	CONNECT(pmClockGpuPp0,		pmSlaveGpu_g.node),
	CONNECT(pmClockGpuPp1,		pmSlaveGpu_g.node),
	CONNECT(pmClockSata,		pmSlaveSata_g.node),
	CONNECT(pmClockPcie,		pmSlavePcie_g.node),
	CONNECT(pmClockGdma,		pmSlaveGdma_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveGdma_g.node),
	CONNECT(pmClockGem0RefUngated,	pmSlaveEth0_g.node),
	CONNECT(pmClockGem0Ref,		pmSlaveEth0_g.node),
	CONNECT(pmClockGem0Tx,		pmSlaveEth0_g.node),
	CONNECT(pmClockGem0Rx,		pmSlaveEth0_g.node),
	CONNECT(pmClockGemTsu,		pmSlaveEth0_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveEth0_g.node),
	CONNECT(pmClockGem1RefUngated,	pmSlaveEth1_g.node),
	CONNECT(pmClockGem1Ref,		pmSlaveEth1_g.node),
	CONNECT(pmClockGem1Tx,		pmSlaveEth1_g.node),
	CONNECT(pmClockGem1Rx,		pmSlaveEth1_g.node),
	CONNECT(pmClockGemTsu,		pmSlaveEth1_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveEth1_g.node),
	CONNECT(pmClockGem2RefUngated,	pmSlaveEth2_g.node),
	CONNECT(pmClockGem2Ref,		pmSlaveEth2_g.node),
	CONNECT(pmClockGem2Tx,		pmSlaveEth2_g.node),
	CONNECT(pmClockGem2Rx,		pmSlaveEth2_g.node),
	CONNECT(pmClockGemTsu,		pmSlaveEth2_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveEth2_g.node),
	CONNECT(pmClockGem3RefUngated,	pmSlaveEth3_g.node),
	CONNECT(pmClockGem3Ref,		pmSlaveEth3_g.node),
	CONNECT(pmClockGem3Tx,		pmSlaveEth3_g.node),
	CONNECT(pmClockGem3Rx,		pmSlaveEth3_g.node),
	CONNECT(pmClockGemTsu,		pmSlaveEth3_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveEth3_g.node),
	CONNECT(pmClockUsb3Dual,	pmSlaveUsb0_g.slv.node),
	CONNECT(pmClockUsb0Bus,		pmSlaveUsb0_g.slv.node),
	CONNECT(pmClockUsb3Dual,	pmSlaveUsb1_g.slv.node),
	CONNECT(pmClockUsb1Bus,		pmSlaveUsb1_g.slv.node),
	CONNECT(pmClockQSpi,		pmSlaveQSpi_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveQSpi_g.node),
	CONNECT(pmClockSdio0,		pmSlaveSD0_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveSD0_g.node),
	CONNECT(pmClockDll,		pmSlaveSD0_g.node),
	CONNECT(pmClockSdio1,		pmSlaveSD1_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveSD1_g.node),
	CONNECT(pmClockDll,		pmSlaveSD1_g.node),
	CONNECT(pmClockUart0,		pmSlaveUart0_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveUart0_g.node),
	CONNECT(pmClockUart1,		pmSlaveUart1_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveUart1_g.node),
	CONNECT(pmClockSpi0,		pmSlaveSpi0_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveSpi0_g.node),
	CONNECT(pmClockSpi1,		pmSlaveSpi1_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveSpi1_g.node),
	CONNECT(pmClockCan0,		pmSlaveCan0_g.node),
	CONNECT(pmClockCan0Ref,		pmSlaveCan0_g.node),
	CONNECT(pmClockCan0Mio,		pmSlaveCan0_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveCan0_g.node),
	CONNECT(pmClockCan1,		pmSlaveCan1_g.node),
	CONNECT(pmClockCan1Ref,		pmSlaveCan1_g.node),
	CONNECT(pmClockCan1Mio,		pmSlaveCan1_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveCan1_g.node),
	CONNECT(pmClockCpuR5,		pmPowerIslandRpu_g.power.node),
	CONNECT(pmClockCpuR5Core,	pmPowerIslandRpu_g.power.node),
	CONNECT(pmClockCsuPll,		pmSlavePcap_g.node),
	CONNECT(pmClockPcap,		pmSlavePcap_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveTtc0_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveTtc1_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveTtc2_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveTtc3_g.node),
	CONNECT(pmClockNand,		pmSlaveNand_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveNand_g.node),
	CONNECT(pmClockAdma,		pmSlaveAdma_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveAdma_g.node),
	CONNECT(pmClockPl0,		pmSlavePl_g.node),
	CONNECT(pmClockPl1,		pmSlavePl_g.node),
	CONNECT(pmClockPl2,		pmSlavePl_g.node),
	CONNECT(pmClockPl3,		pmSlavePl_g.node),
	CONNECT(pmClockI2C0,		pmSlaveI2C0_g.node),
	CONNECT(pmClockI2C1,		pmSlaveI2C1_g.node),
	CONNECT(pmClockFpdWdt,		pmSlaveFpdWdt_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveGpio_g.node),
};

/**
 * PmClockConstructList() - Link clock handles into clock's/node's lists
 */
void PmClockConstructList(void)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmClockHandles); i++) {
		PmClockHandle* ch = &pmClockHandles[i];

		/* Add the clock at the beginning of the node's clocks list */
		ch->nextClock = ch->node->clocks;
		ch->node->clocks = ch;

		/* Add the node at the beginning of the clock's users list */
		ch->nextNode = ch->clock->users;
		ch->clock->users = ch;
	}
}

/**
 * PmClockInit() - Initialize clock parent pointers according to hardware config
 */
void PmClockInit(void)
{
	u32 i;

	/* Initialize parents if possible for a particular clock */
	for (i = 0U; i < ARRAY_SIZE(pmClocks); i++) {
		PmClock* clk = pmClocks[i];

		if ((NULL != clk->class) && (NULL != clk->class->ctrl) &&
		    (NULL != clk->class->ctrl->initParent)) {
			clk->class->ctrl->initParent(clk);
		}
	}
}

/**
 * @PmClockIsActive() Check if any clock for a given node is active
 * @node	Node whose clocks need to be checked
 *
 * @return XST_SUCCESS if any one clock for given node is active
 *         XST_FAILURE if all clocks for given node are inactive
 */
s32 PmClockIsActive(PmNode* const node)
{
	PmClockHandle* ch = node->clocks;
	s32 status = XST_FAILURE;

	while (NULL != ch) {
		PmClock* clk = &ch->clock->base;

		if ((NULL != clk->class) &&
		    (NULL != clk->class->ctrl) &&
		    (NULL != clk->class->ctrl->getGate)) {
			u8 enable = 0U;
			s32 ret = clk->class->ctrl->getGate(clk, &enable);

			if (XST_SUCCESS == ret) {
				if (1U == enable) {
					status = XST_SUCCESS;
					goto done;
				}
			} else {
				PmErr("Clock #%lu model\r\n", clk->id);
			}
		}
		ch = ch->nextClock;
	}

done:
	return status;
}

/**
 * @PmClockSave() - Save control register values for clocks used by the node
 * @node	Node whose clock control regs need to be saved
 */
void PmClockSave(PmNode* const node)
{
	PmClockHandle* ch = node->clocks;

	while (NULL != ch) {
		ch->clock->ctrlVal = XPfw_Read32(ch->clock->ctrlAddr);
		ch = ch->nextClock;
	}
}

/**
 * PmClockRestore() - Restore control register values for clocks of the node
 * @node	Node whose clock control registers need to be restored
 */
void PmClockRestore(PmNode* const node)
{
	PmClockHandle* ch = node->clocks;

	while (NULL != ch) {
		/* Restore the clock configuration if needed */
		if (0U != ch->clock->ctrlVal) {
			XPfw_Write32(ch->clock->ctrlAddr, ch->clock->ctrlVal);
		}
		ch = ch->nextClock;
	}
}

/**
 * PmClockRequest() - Request clocks used by the given node
 * @node	Node whose clocks need to be requested
 * @return	XST_SUCCESS if the request is processed correctly, or error code
 *		if a PLL parent needed to be locked and the locking has failed.
 * @note	The dependency toward a PLL parent is automatically resolved
 */
s32 PmClockRequest(PmNode* const node)
{
	PmClockHandle* ch = node->clocks;
	s32 status = XST_SUCCESS;

	if (0U != (NODE_LOCKED_CLOCK_FLAG & node->flags)) {
		PmWarn("%s double request\r\n", node->name);
		goto done;
	}
	while (NULL != ch) {
		PmClockRequestInt(&ch->clock->base);
		ch = ch->nextClock;
	}
	node->flags |= NODE_LOCKED_CLOCK_FLAG;

done:
	return status;
}

/**
 * PmClockRelease() - Release clocks used by the given node
 * @node	Node whose clocks are released
 *
 * @note	If a PLL parent of a released clock have no other users, the
 *		PM framework will suspend that PLL.
 */
void PmClockRelease(PmNode* const node)
{
	PmClockHandle* ch = node->clocks;

	if (0U == (NODE_LOCKED_CLOCK_FLAG & node->flags)) {
		PmWarn("%s double release\r\n", node->name);
		goto done;
	}
	while (NULL != ch) {
		PmClockReleaseInt(&ch->clock->base);
		ch = ch->nextClock;
	}
	node->flags &= ~(u8)NODE_LOCKED_CLOCK_FLAG;

done:
	return;
}

/**
 * PmClockGetById() - Get clock structure based on clock ID
 * @clockId	ID of the clock to get
 *
 * @return	Pointer to the clock structure if found, otherwise NULL
 */
PmClock* PmClockGetById(const u32 clockId)
{
	u32 i;
	PmClock* clockPtr = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmClocks); i++) {
		if (clockId == pmClocks[i]->id) {
			clockPtr = pmClocks[i];
			break;
		}
	}

	return clockPtr;
}

/**
 * PmClockCheckForCtrl() - Common function for checking validity
 * @clockPtr	Clock to be checked
 *
 * @return	XST_INVALID_PARAM if clock argument in NULL
 *		XST_SUCCESS if clock is valid, has class and control methods
 *		XST_NO_FEATURE otherwise
 */
static s32 PmClockCheckForCtrl(const PmClock* const clockPtr)
{
	s32 status = XST_SUCCESS;

	if (NULL == clockPtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	if ((NULL == clockPtr->class) || (NULL == clockPtr->class->ctrl)) {
		status = XST_NO_FEATURE;
		goto done;
	}

done:
	return status;
}

/**
 * PmClockMuxSetParent() - Configure clock mux
 * @clockPtr	Pointer to the clock structure
 * @select	Mux select value
 *
 * @return	XST_SUCCESS if the mux is configured
 * 		XST_NO_FEATURE if the clock has no mux
 * 		XST_INVALID_PARAM if select value is invalid
 */
s32 PmClockMuxSetParent(PmClock* const clockPtr, const u32 select)
{
	s32 status = PmClockCheckForCtrl(clockPtr);

	if ((XST_SUCCESS != status) || (NULL == clockPtr->class->ctrl->setParent)) {
		status = XST_NO_FEATURE;
		goto done;
	}
	status = clockPtr->class->ctrl->setParent(clockPtr, select);
done:
	return status;
}

/**
 * PmClockMuxGetParent() - Get clock parent (mux select value)
 * @clockPtr	Pointer to the target clock
 * @select	Location to store mux select value of the current parent
 *
 * @return	Status of getting the parent: XST_SUCCESS if the parent pointer
 *		is stored into 'parent' or error code
 */
s32 PmClockMuxGetParent(PmClock* const clockPtr, u32 *const select)
{
	s32 status = PmClockCheckForCtrl(clockPtr);

	if ((XST_SUCCESS != status) || (NULL == clockPtr->class->ctrl->getParent)) {
		status = XST_NO_FEATURE;
		goto done;
	}

	status = clockPtr->class->ctrl->getParent(clockPtr, select);
done:
	return status;
}

/**
 * PmClockGateSetState() - Activate/gate the clock
 * @clockPtr	Pointer to the clock structure
 * @enable	1=enable the clock, 0=disable the clock
 *
 * @return	XST_SUCCESS if the clock is configured
 * 		XST_NO_FEATURE if the clock has no gate
 */
s32 PmClockGateSetState(PmClock* const clockPtr, const u8 enable)
{
	s32 status = PmClockCheckForCtrl(clockPtr);

	if ((XST_SUCCESS != status) || (NULL == clockPtr->class->ctrl->setGate)) {
		status = XST_NO_FEATURE;
		goto done;
	}
#if ((STDOUT_BASEADDRESS == XPMU_UART_0_BASEADDR) && defined(DEBUG_MODE))
	if (&pmClockUart0.base == clockPtr) {
		goto done;
	}
#endif
#if ((STDOUT_BASEADDRESS == XPMU_UART_1_BASEADDR) && defined(DEBUG_MODE))
	if (&pmClockUart1.base == clockPtr) {
		goto done;
	}
#endif
	/*
	 * This is added because equivalent functionality added in commit
	 * 2a1b15d8b2 has been removed from PmMmioWrite in pm_core.c
	 */
	if (&pmClockAms.base == clockPtr) {
		goto done;
	}
	status = clockPtr->class->ctrl->setGate(clockPtr, enable);
done:
	return status;
}

/**
 * PmClockGateGetState() - Get state of the clock gate
 * @clockPtr	Pointer to the target clock
 * @enable	Location where the state will be returned
 *
 * @return	XST_SUCCESS if the clock state is get/enable location is updated
 *		XST_NO_FEATURE if the clock has no gate
 */
s32 PmClockGateGetState(PmClock* const clockPtr, u8* const enable)
{
	s32 status = PmClockCheckForCtrl(clockPtr);

	if ((XST_SUCCESS != status) || (NULL == clockPtr->class->ctrl->getGate)) {
		status = XST_NO_FEATURE;
		goto done;
	}
	status = clockPtr->class->ctrl->getGate(clockPtr, enable);
done:
	return status;
}

/**
 * PmClockDividerSetVal() - Set divider of the clock
 * @clockPtr	Pointer to the target clock
 * @divId	Identifier for the divider to be set
 * @val		Divider value to be set
 *
 * @return	Status of performing the operation:
 *		XST_SUCCESS the divider is set as requested
 *		XST_NO_FEATURE the target clock has no divider
 *		XST_INVALID_PARAM if given clock is NULL
 */
s32 PmClockDividerSetVal(PmClock* const clockPtr, const u32 divId, const u32 val)
{
	s32 status = PmClockCheckForCtrl(clockPtr);

	if ((XST_SUCCESS != status) || (NULL == clockPtr->class->ctrl->setDivider)) {
		status = XST_NO_FEATURE;
		goto done;
	}
	status = clockPtr->class->ctrl->setDivider(clockPtr, divId, val);
done:
	return status;
}

/**
 * PmClockDividerGetVal() - Get divider of the clock
 * @clockPtr	Pointer to the target clock
 * @divId	Identifier of the clock's divider
 * @val		Location where the divider value needs to be stored
 *
 * @return	Status of performing the operation:
 *		XST_SUCCESS the divider location is updated (got divider)
 *		XST_NO_FEATURE the target clock has no divider
 *		XST_INVALID_PARAM if given clock is NULL
 */
s32 PmClockDividerGetVal(PmClock* const clockPtr, const u32 divId, u32* const val)
{
	s32 status = PmClockCheckForCtrl(clockPtr);

	if ((XST_SUCCESS != status) || (NULL == clockPtr->class->ctrl->getDivider)) {
		status = XST_NO_FEATURE;
		goto done;
	}
	status = clockPtr->class->ctrl->getDivider(clockPtr, divId, val);
done:
	return status;
}

/**
 * PmClockCheckPermission() - Check permission for master to control the clock
 * @clockPtr	Pointer to the target clock
 * @ipiMask	Master's IPI mask
 *
 * @return	Status of performing the check:
 *		XST_SUCCESS the permission is granted
 *		XST_PM_NO_ACCESS if control is not allowed
 */
s32 PmClockCheckPermission(const PmClock* const clockPtr, const u32 ipiMask)
{
	s32 status = XST_SUCCESS;
	u32 perms;

	if ((NULL == clockPtr) || (NULL == clockPtr->class) ||
	    (NULL == clockPtr->class->getPerms)) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}
	perms = clockPtr->class->getPerms(clockPtr);
	/*
	 * Access is not allowed if master is not permissible or the resource
	 * is shared (multiple masters are permissible)
	 */
	if ((0U == (perms & ipiMask)) ||( __builtin_popcount(perms) > 1)) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

done:
	return status;
}

#ifdef ENABLE_POS
/**
 * PmClockRestoreDdr() - Restore state of clocks related to DDR node
 */
void PmClockRestoreDdr(void)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmDdrClocks); i++) {
		PmClockRequestInt(&pmDdrClocks[i]->base);

		XPfw_Write32(pmDdrClocks[i]->ctrlAddr, pmDdrClocks[i]->ctrlVal);
	}
}
#endif

#endif
