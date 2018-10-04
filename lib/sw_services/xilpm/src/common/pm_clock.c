/******************************************************************************
*
* Copyright (C) 2015-2018 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/
/**
 * @file pm_clock.c
 *
 * PM Definitions implementation
 * @addtogroup xpm_apis XilPM APIs
 * @{
 *****************************************************************************/
#include "pm_clock.h"
#include "pm_common.h"

#define PM_CLOCK_TYPE_DIV0	(1 << PM_CLOCK_DIV0_ID)	/* bits 13:8 */
#define PM_CLOCK_TYPE_DIV1	(1 << PM_CLOCK_DIV1_ID)	/* bits 21:16 */
#define PM_DIV_WIDTH		0x3FU
#define PM_2xDIV_WIDTH		(PM_DIV_WIDTH * PM_DIV_WIDTH)

#define PM_CLOCK_HAS_DIV0(clk)	(0U != ((clk)->type & PM_CLOCK_TYPE_DIV0))
#define PM_CLOCK_HAS_DIV1(clk)	(0U != ((clk)->type & PM_CLOCK_TYPE_DIV1))

typedef struct XPmClockModel XPmClockModel;

/**
 * Pair of multiplexer select value and selected clock input
 */
typedef struct {
	/**
	ID of the clock that is selected with the 'select' value
	*/
	const enum XPmClock clkIn;
	/**
	Select value of the clock multiplexer
	*/
	const u8 select;
} XPmClockSel2ClkIn;

/**
 * MUX select values to clock input mapping
 */
typedef struct {
	/** Mux select to pll mapping at the input of the multiplexer */
	const XPmClockSel2ClkIn* const inputs;
	/** Size of the inputs array*/
	const u8 size;
	/** Number of bits of mux select*/
	const u8 bits;
	/** Number of bits to shift 'bits' in order to get mux select mask*/
	const u8 shift;
} XPmClockMux;

/**
 * Clock model
 */
typedef struct XPmClockModel {
	/** Clock ID*/
	const enum XPmClock id;
	/** Pointer to the mux model*/
	const XPmClockMux* const mux;
	/** Type specifying the available divisors*/
	const u8 type;
	/** Next clock in the list*/
	const XPmClockModel* const next;
} XPmClockModel;

/******************************************************************************/
/* Clock multiplexer models */

static const XPmClockSel2ClkIn advSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_APLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_VPLL,
		.select = 3U,
	},
};

static XPmClockMux advMux = {
	.inputs = advSel2ClkIn,
	.size = PM_ARRAY_SIZE(advSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn avdSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_APLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_VPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 3U,
	},
};

static XPmClockMux avdMux = {
	.inputs = avdSel2ClkIn,
	.size = PM_ARRAY_SIZE(avdSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn aiodSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_APLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_IOPLL_TO_FPD,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 3U,
	},
};

static XPmClockMux aiodMux = {
	.inputs = aiodSel2ClkIn,
	.size = PM_ARRAY_SIZE(aiodSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn vdrSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_VPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_RPLL_TO_FPD,
		.select = 3U,
	},
};

static XPmClockMux vdrMux = {
	.inputs = vdrSel2ClkIn,
	.size = PM_ARRAY_SIZE(vdrSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn dvSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_DPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_VPLL,
		.select = 1U,
	},
};

static XPmClockMux dvMux = {
	.inputs = dvSel2ClkIn,
	.size = PM_ARRAY_SIZE(dvSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn iovdSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL_TO_FPD,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_VPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 3U,
	},
};

static XPmClockMux iovdMux = {
	.inputs = iovdSel2ClkIn,
	.size = PM_ARRAY_SIZE(iovdSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn ioadSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL_TO_FPD,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_APLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 3U,
	},
};

static XPmClockMux ioadMux = {
	.inputs = ioadSel2ClkIn,
	.size = PM_ARRAY_SIZE(ioadSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn iodaSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL_TO_FPD,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_APLL,
		.select = 3U,
	},
};

static XPmClockMux iodaMux = {
	.inputs = iodaSel2ClkIn,
	.size = PM_ARRAY_SIZE(iodaSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn iorSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_RPLL,
		.select = 2U,
	},
};

static XPmClockMux iorMux = {
	.inputs = iorSel2ClkIn,
	.size = PM_ARRAY_SIZE(iorSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn iordFpdSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL_TO_FPD,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_RPLL_TO_FPD,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 3U,
	},
};

static XPmClockMux iordFpdMux = {
	.inputs = iordFpdSel2ClkIn,
	.size = PM_ARRAY_SIZE(iordFpdSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn iordSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_RPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL_TO_LPD,
		.select = 3U,
	},
};

static XPmClockMux iordMux = {
	.inputs = iordSel2ClkIn,
	.size = PM_ARRAY_SIZE(iordSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn iorvSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_RPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_VPLL_TO_LPD,
		.select = 3U,
	},
};

static XPmClockMux iorvMux = {
	.inputs = iorvSel2ClkIn,
	.size = PM_ARRAY_SIZE(iorvSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn riodSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_RPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_IOPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL_TO_LPD,
		.select = 3U,
	},
};

static XPmClockMux riodMux = {
	.inputs = riodSel2ClkIn,
	.size = PM_ARRAY_SIZE(riodSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const XPmClockSel2ClkIn iordPsRefSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_RPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL_TO_LPD,
		.select = 3U,
	}, {
		.clkIn = PM_CLOCK_EXT_PSS_REF,
		.select = 4U,
	}, {
		.clkIn = PM_CLOCK_EXT_PSS_REF,
		.select = 5U,
	}, {
		.clkIn = PM_CLOCK_EXT_PSS_REF,
		.select = 6U,
	}, {
		.clkIn = PM_CLOCK_EXT_PSS_REF,
		.select = 7U,
	},
};

static XPmClockMux iordPsRefMux = {
	.inputs = iordPsRefSel2ClkIn,
	.size = PM_ARRAY_SIZE(iordPsRefSel2ClkIn),
	.bits = 3U,
	.shift = 0U,
};

static XPmClockMux can0MioMux = {
	.inputs = NULL,		/* NULL is reserved for MIO inputs */
	.size = 0U,
	.bits = 7U,
	.shift = 0U,
};

static XPmClockMux can1MioMux = {
	.inputs = NULL,		/* NULL is reserved for MIO inputs */
	.size = 0U,
	.bits = 7U,
	.shift = 15U,
};

static const XPmClockSel2ClkIn can0Sel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_CAN0_REF,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_CAN0_MIO,
		.select = 1U,
	},
};

static XPmClockMux can0Mux = {
	.inputs = can0Sel2ClkIn,
	.size = PM_ARRAY_SIZE(can0Sel2ClkIn),
	.bits = 1U,
	.shift = 7U,
};

static const XPmClockSel2ClkIn can1Sel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_CAN1_REF,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_CAN1_MIO,
		.select = 1U,
	},
};

static XPmClockMux can1Mux = {
	.inputs = can1Sel2ClkIn,
	.size = PM_ARRAY_SIZE(can1Sel2ClkIn),
	.bits = 1U,
	.shift = 22U,
};

static const XPmClockSel2ClkIn gemTsuSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_GEM_TSU_REF,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_MIO26,
		.select = 1U,
	}, {
		.clkIn = PM_CLOCK_GEM_TSU_REF,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_EXT_MIO50_OR_MIO51,
		.select = 3U,
	},
};

static XPmClockMux gemTsuMux = {
	.inputs = gemTsuSel2ClkIn,
	.size = PM_ARRAY_SIZE(gemTsuSel2ClkIn),
	.bits = 2U,
	.shift = 20U,
};

static const XPmClockSel2ClkIn gem0RefSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_GEM0_REF_UNGATED,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_GEM0_TX_EMIO,
		.select = 1U,
	},
};

static XPmClockMux gem0RefMux = {
	.inputs = gem0RefSel2ClkIn,
	.size = PM_ARRAY_SIZE(gem0RefSel2ClkIn),
	.bits = 1U,
	.shift = 1U,
};

static const XPmClockSel2ClkIn gem1RefSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_GEM1_REF_UNGATED,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_GEM1_TX_EMIO,
		.select = 1U,
	},
};

static XPmClockMux gem1RefMux = {
	.inputs = gem1RefSel2ClkIn,
	.size = PM_ARRAY_SIZE(gem1RefSel2ClkIn),
	.bits = 1U,
	.shift = 6U,
};

static const XPmClockSel2ClkIn gem2RefSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_GEM2_REF_UNGATED,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_GEM2_TX_EMIO,
		.select = 1U,
	},
};

static XPmClockMux gem2RefMux = {
	.inputs = gem2RefSel2ClkIn,
	.size = PM_ARRAY_SIZE(gem2RefSel2ClkIn),
	.bits = 1U,
	.shift = 11U,
};

static const XPmClockSel2ClkIn gem3RefSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_GEM3_REF_UNGATED,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_GEM3_TX_EMIO,
		.select = 1U,
	},
};

static XPmClockMux gem3RefMux = {
	.inputs = gem3RefSel2ClkIn,
	.size = PM_ARRAY_SIZE(gem3RefSel2ClkIn),
	.bits = 1U,
	.shift = 16U,
};

static const XPmClockSel2ClkIn fpdWdtSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_TOPSW_LSBUS,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_SWDT1,
		.select = 1U,
	},
};

static XPmClockMux fpdWdtMux = {
	.inputs = fpdWdtSel2ClkIn,
	.size = PM_ARRAY_SIZE(fpdWdtSel2ClkIn),
	.bits = 1U,
	.shift = 0,
};

/******************************************************************************/
/* Clock models (only clocks with mux and divisor need to be modeled) */

static XPmClockModel pmClockAcpu = {
	.id = PM_CLOCK_ACPU,
	.mux = &advMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = NULL,
};

static XPmClockModel pmClockDbgTrace = {
	.id = PM_CLOCK_DBG_TRACE,
	.mux = &iodaMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockAcpu,
};

static XPmClockModel pmClockDbgFpd = {
	.id = PM_CLOCK_DBG_FPD,
	.mux = &iodaMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockDbgTrace,
};

static XPmClockModel pmClockDpVideo = {
	.id = PM_CLOCK_DP_VIDEO_REF,
	.mux = &vdrMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDbgFpd,
};

static XPmClockModel pmClockDpAudio = {
	.id = PM_CLOCK_DP_AUDIO_REF,
	.mux = &vdrMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDpVideo,
};

static XPmClockModel pmClockDpStc = {
	.id = PM_CLOCK_DP_STC_REF,
	.mux = &vdrMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDpAudio,
};

static XPmClockModel pmClockDdr = {
	.id = PM_CLOCK_DDR_REF,
	.mux = &dvMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockDpStc,
};

static XPmClockModel pmClockGpu = {
	.id = PM_CLOCK_GPU_REF,
	.mux = &iovdMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockDdr,
};

static XPmClockModel pmClockSata = {
	.id = PM_CLOCK_SATA_REF,
	.mux = &ioadMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockGpu,
};

static XPmClockModel pmClockPcie = {
	.id = PM_CLOCK_PCIE_REF,
	.mux = &iordFpdMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockSata,
};

static XPmClockModel pmClockGdma = {
	.id = PM_CLOCK_GDMA_REF,
	.mux = &avdMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockPcie,
};

static XPmClockModel pmClockDpDma = {
	.id = PM_CLOCK_DPDMA_REF,
	.mux = &avdMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockGdma,
};

static XPmClockModel pmClockTopSwMain = {
	.id = PM_CLOCK_TOPSW_MAIN,
	.mux = &avdMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockDpDma,
};

static XPmClockModel pmClockTopSwLsBus = {
	.id = PM_CLOCK_TOPSW_LSBUS,
	.mux = &aiodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockTopSwMain,
};

static XPmClockModel pmClockDbgTstmp = {
	.id = PM_CLOCK_DBG_TSTMP,
	.mux = &iodaMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockTopSwLsBus,
};

static XPmClockModel pmClockUsb3Dual = {
	.id = PM_CLOCK_USB3_DUAL_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDbgTstmp,
};

static XPmClockModel pmClockGem0RefUngated = {
	.id = PM_CLOCK_GEM0_REF_UNGATED,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockUsb3Dual,
};

static XPmClockModel pmClockGem1RefUngated = {
	.id = PM_CLOCK_GEM1_REF_UNGATED,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockGem0RefUngated,
};

static XPmClockModel pmClockGem2RefUngated = {
	.id = PM_CLOCK_GEM2_REF_UNGATED,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockGem1RefUngated,
};

static XPmClockModel pmClockGem3RefUngated = {
	.id = PM_CLOCK_GEM3_REF_UNGATED,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockGem2RefUngated,
};

static XPmClockModel pmClockUsb0Bus = {
	.id = PM_CLOCK_USB0_BUS_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockGem3RefUngated,
};

static XPmClockModel pmClockUsb1Bus = {
	.id = PM_CLOCK_USB1_BUS_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockUsb0Bus,
};

static XPmClockModel pmClockQSpi = {
	.id = PM_CLOCK_QSPI_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockUsb1Bus,
};

static XPmClockModel pmClockSdio0 = {
	.id = PM_CLOCK_SDIO0_REF,
	.mux = &iorvMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockQSpi,
};

static XPmClockModel pmClockSdio1 = {
	.id = PM_CLOCK_SDIO1_REF,
	.mux = &iorvMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockSdio0,
};

static XPmClockModel pmClockUart0 = {
	.id = PM_CLOCK_UART0_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockSdio1,
};

static XPmClockModel pmClockUart1 = {
	.id = PM_CLOCK_UART1_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockUart0,
};

static XPmClockModel pmClockSpi0 = {
	.id = PM_CLOCK_SPI0_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockUart1,
};

static XPmClockModel pmClockSpi1 = {
	.id = PM_CLOCK_SPI1_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockSpi0,
};

static XPmClockModel pmClockCan0Ref = {
	.id = PM_CLOCK_CAN0_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockSpi1,
};

static XPmClockModel pmClockCan1Ref = {
	.id = PM_CLOCK_CAN1_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockCan0Ref,
};

static XPmClockModel pmClockCpuR5 = {
	.id = PM_CLOCK_CPU_R5,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockCan1Ref,
};

static XPmClockModel pmClockIouSwitch = {
	.id = PM_CLOCK_IOU_SWITCH,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockCpuR5,
};

static XPmClockModel pmClockCsuPll = {
	.id = PM_CLOCK_CSU_PLL,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockIouSwitch,
};

static XPmClockModel pmClockPcap = {
	.id = PM_CLOCK_PCAP,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockCsuPll,
};

static XPmClockModel pmClockLpdSwitch = {
	.id = PM_CLOCK_LPD_SWITCH,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockPcap,
};

static XPmClockModel pmClockLpdLsBus = {
	.id = PM_CLOCK_LPD_LSBUS,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockLpdSwitch,
};

static XPmClockModel pmClockDbgLpd = {
	.id = PM_CLOCK_DBG_LPD,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockLpdLsBus,
};

static XPmClockModel pmClockNand = {
	.id = PM_CLOCK_NAND_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDbgLpd,
};

static XPmClockModel pmClockAdma = {
	.id = PM_CLOCK_ADMA_REF,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockNand,
};

static XPmClockModel pmClockPl0 = {
	.id = PM_CLOCK_PL0_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockAdma,
};

static XPmClockModel pmClockPl1 = {
	.id = PM_CLOCK_PL1_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockPl0,
};

static XPmClockModel pmClockPl2 = {
	.id = PM_CLOCK_PL2_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockPl1,
};

static XPmClockModel pmClockPl3 = {
	.id = PM_CLOCK_PL3_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockPl2,
};

static XPmClockModel pmClockGemTsuRef = {
	.id = PM_CLOCK_GEM_TSU_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockPl3,
};

static XPmClockModel pmClockDll = {
	.id = PM_CLOCK_DLL_REF,
	.mux = &iorMux,
	.type = 0U,
	.next = &pmClockGemTsuRef,
};

static XPmClockModel pmClockAms = {
	.id = PM_CLOCK_AMS_REF,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDll,
};

static XPmClockModel pmClockI2C0 = {
	.id = PM_CLOCK_I2C0_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockAms,
};

static XPmClockModel pmClockI2C1 = {
	.id = PM_CLOCK_I2C1_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockI2C0,
};

static XPmClockModel pmClockTimeStamp = {
	.id = PM_CLOCK_TIMESTAMP_REF,
	.mux = &iordPsRefMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockI2C1,
};

static XPmClockModel pmClockCan0Mio = {
	.id = PM_CLOCK_CAN0_MIO,
	.mux = &can0MioMux,
	.type = 0U,
	.next = &pmClockTimeStamp,
};

static XPmClockModel pmClockCan0 = {
	.id = PM_CLOCK_CAN0,
	.mux = &can0Mux,
	.type = 0U,
	.next = &pmClockCan0Mio,
};

static XPmClockModel pmClockCan1Mio = {
	.id = PM_CLOCK_CAN1_MIO,
	.mux = &can1MioMux,
	.type = 0U,
	.next = &pmClockCan0,
};

static XPmClockModel pmClockCan1 = {
	.id = PM_CLOCK_CAN1,
	.mux = &can1Mux,
	.type = 0U,
	.next = &pmClockCan1Mio,
};

static XPmClockModel pmClockGemTsu = {
	.id = PM_CLOCK_GEM_TSU,
	.mux = &gemTsuMux,
	.type = 0U,
	.next = &pmClockCan1,
};

static XPmClockModel pmClockGem0Ref = {
	.id = PM_CLOCK_GEM0_REF,
	.mux = &gem0RefMux,
	.type = 0U,
	.next = &pmClockGemTsu,
};

static XPmClockModel pmClockGem1Ref = {
	.id = PM_CLOCK_GEM1_REF,
	.mux = &gem1RefMux,
	.type = 0U,
	.next = &pmClockGem0Ref,
};

static XPmClockModel pmClockGem2Ref = {
	.id = PM_CLOCK_GEM2_REF,
	.mux = &gem2RefMux,
	.type = 0U,
	.next = &pmClockGem1Ref,
};

static XPmClockModel pmClockGem3Ref = {
	.id = PM_CLOCK_GEM3_REF,
	.mux = &gem3RefMux,
	.type = 0U,
	.next = &pmClockGem2Ref,
};

static XPmClockModel pmClockFpdWdt = {
	.id = PM_CLOCK_WDT,
	.mux = &fpdWdtMux,
	.type = 0U,
	.next = &pmClockGem3Ref,
};

static const XPmClockModel* const head = &pmClockFpdWdt;

/****************************************************************************/
/**
 * @brief  Get clock structure by clock ID
 *
 * @param  id ID of the target clock
 *
 * @return Returns pointer to the found clock or NULL
 *
 * @note   None
 *
 ****************************************************************************/
static const XPmClockModel* XPm_GetClockById(const enum XPmClock id)
{
	const XPmClockModel* clk = head;

	while (clk) {
		if (clk->id == id) {
			break;
		}
		clk = clk->next;
	}

	return clk;
}

/****************************************************************************/
/**
 * @brief  Get parent clock ID for a given clock ID and mux select value
 *
 * @param  clockId ID of the target clock
 * @param  select Mux select value
 * @param  parentId Location to store parent clock ID
 *
 * @return Returns XST_SUCCESS if parent clock ID is found, XST_INVALID_PARAM
 * otherwise.
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetClockParentBySelect(const enum XPmClock clockId,
				   const u32 select,
				   enum XPmClock* const parentId)
{
	const XPmClockModel* const clk = XPm_GetClockById(clockId);
	XStatus status = XST_INVALID_PARAM;
	u32 i;

	if (!clk || !clk->mux) {
		goto done;
	}

	if (!clk->mux->inputs) {
		/* MIO mux */
		if (select <= 0x4DU) {
			*parentId = PM_CLOCK_EXT_MIO0 + select;
			status = XST_SUCCESS;
		}
		/* else select parameter is invalid (out of scope) */
		goto done;
	}

	for (i = 0U; i < clk->mux->size; i++) {
		if (clk->mux->inputs[i].select == select) {
			*parentId = clk->mux->inputs[i].clkIn;
			status = XST_SUCCESS;
			break;
		}
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Get mux select value for given clock and clock parent IDs
 *
 * @param  clockId ID of the target clock
 * @param  parentId ID of the parent clock
 * @param  select Location to store mux select value
 *
 * @return Returns XST_SUCCESS if select value is found, XST_INVALID_PARAM
 * otherwise.
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetSelectByClockParent(const enum XPmClock clockId,
				   const enum XPmClock parentId,
				   u32* const select)
{
	const XPmClockModel* const clk = XPm_GetClockById(clockId);
	XStatus status = XST_INVALID_PARAM;
	u32 i;

	if (!clk || !clk->mux) {
		goto done;
	}

	if (!clk->mux->inputs) {
		/* MIO mux */
		u32 mioSel = parentId - PM_CLOCK_EXT_MIO0;
		if (mioSel <= 0x4DU) {
			*select = mioSel;
			status = XST_SUCCESS;
		}
		/* else parentId parameter is invalid (out of scope) */
		goto done;
	}

	for (i = 0U; i < clk->mux->size; i++) {
		if (clk->mux->inputs[i].clkIn == parentId) {
			*select = clk->mux->inputs[i].select;
			status = XST_SUCCESS;
			break;
		}
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Get number of divider that a given clock has
 *
 * @param  clock ID of the target clock
 *
 * @return Encoded clock divider types. If the clock ID is invalid zero is
 * returned.
 *
 * @note   None
 *
 ****************************************************************************/
u8 XPm_GetClockDivType(const enum XPmClock clock)
{
	const XPmClockModel* const clk = XPm_GetClockById(clock);
	u8 divs = 0U;

	if (!clk) {
		goto done;
	}
	divs = clk->type;

done:
	return divs;
}

/****************************************************************************/
/**
 * @brief  Map effective divider value for given clock on DIV0 and DIV1 dividers
 *
 * @param  clock ID of the target clock
 * @param  div Effective divider value
 * @param  div0 Location to store mapped DIV0 value
 * @param  div1 Location to store mapped DIV1 value
 *
 * @return Encoded mask of mapped dividers
 *
 * @note   The effective divider value may not be mappable on 2x 6-bit wide
 * dividers. This is the case if a given divider value is higher than 6-bit
 * divider (requires 2xdividers), but its a prime number (cannot be divided
 * to get 2x divider values).
 *
 ****************************************************************************/
u8 XPm_MapDivider(const enum XPmClock clock,
		       const u32 div,
		       u32* const div0,
		       u32* const div1)
{
	const XPmClockModel* const clk = XPm_GetClockById(clock);
	u32 d0, d1 = 0U;
	u8 mapped = 0U;

	if (!clk || !div0 || !div1) {
		goto done;
	}

	/* Check if clock has no divider */
	if (!PM_CLOCK_HAS_DIV0(clk) && !PM_CLOCK_HAS_DIV1(clk)) {
		goto done;
	}

	/* Check if given div value is out of range */
	if ((!PM_CLOCK_HAS_DIV1(clk) && div > PM_DIV_WIDTH) ||
	    (div > PM_2xDIV_WIDTH)) {
		goto done;
	}

	/* Check if divider fits in Div0 only */
	if (div <= PM_DIV_WIDTH) {
		*div0 = div;
		mapped = PM_CLOCK_TYPE_DIV0;
		if (PM_CLOCK_HAS_DIV1(clk)) {
			*div1 = 1;
			mapped |= PM_CLOCK_TYPE_DIV1;
		}
		goto done;
	}
	/* Divider has to be configured using both DIV0 and DIV1 */
	for (d0 = 2; d0 <= PM_DIV_WIDTH/2 + 1; d0++) {
		if (0U == (div % d0)) {
			d1 = div / d0;
			break;
		}
	}
	/* Check if div is prime number > width (d1 would not be assigned) */
	if (!d1) {
		goto done;
	}

	*div0 = d0;
	*div1 = d1;
	mapped = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1;

done:
	return mapped;
}
 /** @} */