/*
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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

#include "pm_clock.h"
#include "pm_power.h"
#include "pm_usb.h"
#include "pm_periph.h"
#include "pm_ddr.h"
#include "crf_apb.h"
#include "crl_apb.h"
#include "afi.h"

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

#define IOU_SLCR_BASE			0xFF180000U
#define IOU_SLCR_CAN_MIO_CTRL		(IOU_SLCR_BASE + 0x304U)
#define IOU_SLCR_GEM_CLK_CTRL		(IOU_SLCR_BASE + 0x308U)

#define FPD_SLCR_WDT_CLK_SEL		(FPD_SLCR_BASEADDR + 0x100U)

#define PM_CLOCK_TYPE_DIV0	(1 << 1)	/* bits 13:8 */
#define PM_CLOCK_TYPE_DIV1	(1 << 2)	/* bits 21:16 */
#define PM_CLOCK_TYPE_GATE24	(1 << 3)	/* bit 24 */
#define PM_CLOCK_TYPE_GATE25	(1 << 4)	/* bit 25 */
#define PM_CLOCK_TYPE_GATE26	(1 << 5)	/* bit 26 */

/*********************************************************************
 * Structure definitions
 ********************************************************************/

/**
 * PmClockClass - Structure that encapsulates essential clock methods
 * @request	Pointer to the function that is used to request clock
 * @release	Pointer to the function that is used to release clock
 *
 * @note	A class of clocks for which the maintenance of use count is
 * important must implement request/release methods. Other clock control
 * methods are optional and depend on a particular clock class. If none of this
 * is relevant for certain clock, e.g. oscillator, the class doesn't have to
 * be defined.
 */
typedef struct PmClockClass {
	PmClock* (*const request)(PmClock* const clock);
	PmClock* (*const release)(PmClock* const clock);
} PmClockClass;

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
 * @type	Type of the clock (specifies available dividers and gate)
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
typedef struct PmClockHandle {
	PmClockGen* clock;
	PmNode* node;
	PmClockHandle* nextClock;
	PmClockHandle* nextNode;
} PmClockHandle;

/**
 * PmClockRequestInt() - Wrapper function for a chained requesting of a clock
 * @clock	Pointer to the clock to be requested
 *
 * @note	This function implements non-recursive chained requesting of
 *		a clock and all its parents. Such an approach is required
 *		because recursion is not allowed due to the MISRA.
 */
static void PmClockRequestInt(PmClock* const clock)
{
	PmClock* clk = clock;

	while (NULL != clk) {
		if (clk->class && clk->class->request) {
			clk = clk->class->request(clk);
		} else {
			clk = NULL;
		}
	}
}

/**
 * PmClockReleaseInt() - Wrapper function for a chained releasing of a clock
 * @clock	Pointer to the clock to be released
 *
 * @note	This function implements non-recursive chained releasing of
 *		a clock and all its parents. Such an approach is required
 *		because recursion is not allowed due to the MISRA.
 */
static void PmClockReleaseInt(PmClock* const clock)
{
	PmClock* clk = clock;

	while (NULL != clk) {
		if (clk->class && clk->class->release) {
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
 * @clock	Pointer to a PLL clock
 *
 * @return	This function always returns NULL because the PLL has no clock
 *		parent (its parent is a PLL which is not modeled as a clock)
 */
static PmClock* PmClockRequestPll(PmClock* const clock)
{
	PmClockPll* pclk = (PmClockPll*)clock->derived;

	if (0U == pclk->useCount++) {
		PmPllRequest(pclk->pll);
	}

	return NULL;
}

/**
 * PmClockReleasePll() - PLL specific release clock method
 * @clock	Pointer to a PLL clock
 *
 * @return	This function always returns NULL because the PLL has no clock
 *		parent (its parent is a PLL which is not modeled as a clock)
 */
static PmClock* PmClockReleasePll(PmClock* const clock)
{
	PmClockPll* pclk = (PmClockPll*)clock->derived;

	if (0U == --pclk->useCount) {
		PmPllRelease(pclk->pll);
	}

	return NULL;
}

static PmClockClass pmClockClassPll = {
	.request = PmClockRequestPll,
	.release = PmClockReleasePll,
};

static PmClockPll pmClockApll = {
	.base = {
		.derived = &pmClockApll,
		.class = &pmClockClassPll,
		.id = PM_CLOCK_APLL,
	},
	.pll = &pmApll_g,
	.useCount = 0,
};

static PmClockPll pmClockDpll = {
	.base = {
		.derived = &pmClockDpll,
		.class = &pmClockClassPll,
		.id = PM_CLOCK_DPLL,
	},
	.pll = &pmDpll_g,
	.useCount = 0,
};

static PmClockPll pmClockVpll = {
	.base = {
		.derived = &pmClockVpll,
		.class = &pmClockClassPll,
		.id = PM_CLOCK_VPLL,
	},
	.pll = &pmVpll_g,
	.useCount = 0,
};

static PmClockPll pmClockRpll = {
	.base = {
		.derived = &pmClockRpll,
		.class = &pmClockClassPll,
		.id = PM_CLOCK_RPLL,
	},
	.pll = &pmRpll_g,
	.useCount = 0,
};

static PmClockPll pmClockIOpll = {
	.base = {
		.derived = &pmClockIOpll,
		.class = &pmClockClassPll,
		.id = PM_CLOCK_IOPLL,
	},
	.pll = &pmIOpll_g,
	.useCount = 0,
};

/******************************************************************************/
/* On-chip/generic clocks that can drive PM nodes */

/**
 * PmClockRequestGen() - Request clock method for generic clocks
 * @clock	Pointer to a generic clock
 *
 * @return	Pointer to the parent clock
 */
static PmClock* PmClockRequestGen(PmClock* const clock)
{
	PmClockGen* clk = (PmClockGen*)clock->derived;
	PmClock* parent = NULL;

	if (0U == clk->useCount++) {
		parent = clk->parent;
	}

	return parent;
}

/**
 * PmClockReleaseGen() - Release clock method for generic clocks
 * @clock	Pointer to a generic clock
 *
 * @return	Pointer to the parent clock
 */
static PmClock* PmClockReleaseGen(PmClock* const clock)
{
	PmClockGen* clk = (PmClockGen*)clock->derived;
	PmClock* parent = NULL;

	if (0U == --clk->useCount) {
		parent = clk->parent;
	}

	return parent;
}

static PmClockClass pmClockClassGen = {
	.request = PmClockRequestGen,
	.release = PmClockReleaseGen,
};

static PmClockGen pmClockIOpllToFpd = {
	.base = {
		.derived = &pmClockIOpllToFpd,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_IOPLL_TO_FPD,
	},
	.parent = &pmClockIOpll.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRL_APB_IOPLL_TO_FPD_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0,
	.useCount = 0U,
};

static PmClockGen pmClockRpllToFpd = {
	.base = {
		.derived = &pmClockRpllToFpd,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_RPLL_TO_FPD,
	},
	.parent = &pmClockRpll.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRL_APB_RPLL_TO_FPD_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0,
	.useCount = 0U,
};

static PmClockGen pmClockDpllToLpd = {
	.base = {
		.derived = &pmClockDpllToLpd,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_DPLL_TO_LPD,
	},
	.parent = &pmClockDpll.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRF_APB_DPLL_TO_LPD_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0,
	.useCount = 0U,
};

static PmClockGen pmClockVpllToLpd = {
	.base = {
		.derived = &pmClockVpllToLpd,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_VPLL_TO_LPD,
	},
	.parent = &pmClockVpll.base,
	.users = NULL,
	.mux = NULL,
	.ctrlAddr = CRF_APB_VPLL_TO_LPD_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0,
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
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
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
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
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
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
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
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
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
	.type = PM_CLOCK_TYPE_DIV0,
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

static PmClockGen pmClockGem0Ref = {
	.base = {
		.derived = &pmClockGem0Ref,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM0_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM0_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockGem1Ref = {
	.base = {
		.derived = &pmClockGem1Ref,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM1_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM1_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockGem2Ref = {
	.base = {
		.derived = &pmClockGem2Ref,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM2_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM2_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE25,
	.useCount = 0U,
};

static PmClockGen pmClockGem3Ref = {
	.base = {
		.derived = &pmClockGem3Ref,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM3_REF,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM3_REF_CTRL,
	.ctrlVal = 0U,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE25,
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
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
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
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
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
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
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
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
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
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1 | PM_CLOCK_TYPE_GATE24,
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
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_GATE24,
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

static const PmClockSel2ClkIn gem0TxSel2ClkIn[] = {
	{
		.clkIn = &pmClockGem0Ref.base,
		.select = 0U,
	}, {
		.clkIn = NULL,
		.select = 1U,
	},
};
static PmClockMux gem0TxMux = {
	.inputs = gem0TxSel2ClkIn,
	.size = ARRAY_SIZE(gem0TxSel2ClkIn),
	.bits = 1U,
	.shift = 1U,
};
static PmClockGen pmClockGem0Tx = {
	.base = {
		.derived = &pmClockGem0Tx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM0_TX,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &gem0TxMux,
	.ctrlAddr = IOU_SLCR_GEM_CLK_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static const PmClockSel2ClkIn gem1TxSel2ClkIn[] = {
	{
		.clkIn = &pmClockGem1Ref.base,
		.select = 0U,
	}, {
		.clkIn = NULL,
		.select = 1U,
	},
};
static PmClockMux gem1TxMux = {
	.inputs = gem1TxSel2ClkIn,
	.size = ARRAY_SIZE(gem1TxSel2ClkIn),
	.bits = 1U,
	.shift = 6U,
};
static PmClockGen pmClockGem1Tx = {
	.base = {
		.derived = &pmClockGem1Tx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM1_TX,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &gem1TxMux,
	.ctrlAddr = IOU_SLCR_GEM_CLK_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static const PmClockSel2ClkIn gem2TxSel2ClkIn[] = {
	{
		.clkIn = &pmClockGem2Ref.base,
		.select = 0U,
	}, {
		.clkIn = NULL,
		.select = 1U,
	},
};
static PmClockMux gem2TxMux = {
	.inputs = gem2TxSel2ClkIn,
	.size = ARRAY_SIZE(gem2TxSel2ClkIn),
	.bits = 1U,
	.shift = 11U,
};
static PmClockGen pmClockGem2Tx = {
	.base = {
		.derived = &pmClockGem2Tx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM2_TX,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &gem2TxMux,
	.ctrlAddr = IOU_SLCR_GEM_CLK_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static const PmClockSel2ClkIn gem3TxSel2ClkIn[] = {
	{
		.clkIn = &pmClockGem3Ref.base,
		.select = 0U,
	}, {
		.clkIn = NULL,
		.select = 1U,
	},
};
static PmClockMux gem3TxMux = {
	.inputs = gem3TxSel2ClkIn,
	.size = ARRAY_SIZE(gem3TxSel2ClkIn),
	.bits = 1U,
	.shift = 16U,
};
static PmClockGen pmClockGem3Tx = {
	.base = {
		.derived = &pmClockGem3Tx,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_GEM3_TX,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &gem3TxMux,
	.ctrlAddr = IOU_SLCR_GEM_CLK_CTRL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

static const PmClockSel2ClkIn fpdWdtSel2ClkIn[] = {
	{
		.clkIn = &pmClockTopSwLsBus.base,
		.select = 0U,
	}, {
		.clkIn = NULL,
		.select = 1U,
	},
};
static PmClockMux fpdWdtMux = {
	.inputs = fpdWdtSel2ClkIn,
	.size = ARRAY_SIZE(fpdWdtSel2ClkIn),
	.bits = 1U,
	.shift = 0,
};
static PmClockGen pmClockFpdWdt = {
	.base = {
		.derived = &pmClockFpdWdt,
		.class = &pmClockClassGen,
		.id = PM_CLOCK_WDT,
	},
	.parent = NULL,
	.users = NULL,
	.mux = &fpdWdtMux,
	.ctrlAddr = FPD_SLCR_WDT_CLK_SEL,
	.ctrlVal = 0U,
	.type = 0U,
	.useCount = 0U,
};

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
	&pmClockGem0Ref.base,
	&pmClockGem1Ref.base,
	&pmClockGem2Ref.base,
	&pmClockGem3Ref.base,
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
	&pmClockGem0Tx.base,
	&pmClockGem1Tx.base,
	&pmClockGem2Tx.base,
	&pmClockGem3Tx.base,
	&pmClockFpdWdt.base,
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
	CONNECT(pmClockGem0Ref,		pmSlaveEth0_g.node),
	CONNECT(pmClockGem0Tx,		pmSlaveEth0_g.node),
	CONNECT(pmClockGemTsu,		pmSlaveEth0_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveEth0_g.node),
	CONNECT(pmClockGem1Ref,		pmSlaveEth1_g.node),
	CONNECT(pmClockGem1Tx,		pmSlaveEth1_g.node),
	CONNECT(pmClockGemTsu,		pmSlaveEth1_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveEth1_g.node),
	CONNECT(pmClockGem2Ref,		pmSlaveEth2_g.node),
	CONNECT(pmClockGem2Tx,		pmSlaveEth2_g.node),
	CONNECT(pmClockGemTsu,		pmSlaveEth2_g.node),
	CONNECT(pmClockLpdLsBus,	pmSlaveEth2_g.node),
	CONNECT(pmClockGem3Ref,		pmSlaveEth3_g.node),
	CONNECT(pmClockGem3Tx,		pmSlaveEth3_g.node),
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
	CONNECT(pmClockPl0,		pmPowerDomainPld_g.power.node),
	CONNECT(pmClockPl1,		pmPowerDomainPld_g.power.node),
	CONNECT(pmClockPl2,		pmPowerDomainPld_g.power.node),
	CONNECT(pmClockPl3,		pmPowerDomainPld_g.power.node),
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
 * @PmClockIsActive() Check if any clock for a given node is active
 * @node	Node whose clocks need to be checked
 *
 * @return XST_SUCCESS if any one clock for given node is active
 *         XST_FAILURE if all clocks for given node are inactive
 */
int PmClockIsActive(PmNode* const node)
{
	PmClockHandle* ch = node->clocks;
	int status = XST_FAILURE;

        while (NULL != ch) {
		/* FIXME */
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
int PmClockRequest(PmNode* const node)
{
	PmClockHandle* ch = node->clocks;
	int status = XST_SUCCESS;

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
	node->flags &= ~NODE_LOCKED_CLOCK_FLAG;

done:
	return;
}

#endif
