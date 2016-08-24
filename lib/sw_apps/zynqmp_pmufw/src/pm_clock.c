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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
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

#include "pm_clock.h"
#include "pm_power.h"
#include "pm_usb.h"
#include "pm_periph.h"
#include "pm_ddr.h"
#include "crf_apb.h"
#include "crl_apb.h"

static const PmClockSel2Pll advSel2Pll[] = {
	{
		.pll = &pmSlaveApll_g,
		.select = 0U,
	}, {
		.pll = &pmSlaveDpll_g,
		.select = 2U,
	}, {
		.pll = &pmSlaveVpll_g,
		.select = 3U,
	},
};

static PmClockMux advMux = {
	.inputs = advSel2Pll,
	.size = ARRAY_SIZE(advSel2Pll),
};

static const PmClockSel2Pll avdSel2Pll[] = {
	{
		.pll = &pmSlaveApll_g,
		.select = 0U,
	}, {
		.pll = &pmSlaveVpll_g,
		.select = 2U,
	}, {
		.pll = &pmSlaveDpll_g,
		.select = 3U,
	},
};

static PmClockMux avdMux = {
	.inputs = avdSel2Pll,
	.size = ARRAY_SIZE(avdSel2Pll),
};

static const PmClockSel2Pll vdrSel2Pll[] = {
	{
		.pll = &pmSlaveVpll_g,
		.select = 0U,
	}, {
		.pll = &pmSlaveDpll_g,
		.select = 2U,
	}, {
		.pll = &pmSlaveRpll_g,
		.select = 3U,
	},
};

static PmClockMux vdrMux = {
	.inputs = vdrSel2Pll,
	.size = ARRAY_SIZE(vdrSel2Pll),
};

static const PmClockSel2Pll dvSel2Pll[] = {
	{
		.pll = &pmSlaveDpll_g,
		.select = 0U,
	}, {
		.pll = &pmSlaveVpll_g,
		.select = 1U,
	},
};

static PmClockMux dvMux = {
	.inputs = dvSel2Pll,
	.size = ARRAY_SIZE(dvSel2Pll),
};

static const PmClockSel2Pll iovdSel2Pll[] = {
	{
		.pll = &pmSlaveIOpll_g,
		.select = 0U,
	}, {
		.pll = &pmSlaveVpll_g,
		.select = 2U,
	}, {
		.pll = &pmSlaveDpll_g,
		.select = 3U,
	},
};

static PmClockMux iovdMux = {
	.inputs = iovdSel2Pll,
	.size = ARRAY_SIZE(iovdSel2Pll),
};

static const PmClockSel2Pll ioadSel2Pll[] = {
	{
		.pll = &pmSlaveIOpll_g,
		.select = 0U,
	}, {
		.pll = &pmSlaveApll_g,
		.select = 2U,
	}, {
		.pll = &pmSlaveDpll_g,
		.select = 3U,
	},
};

static PmClockMux ioadMux = {
	.inputs = ioadSel2Pll,
	.size = ARRAY_SIZE(ioadSel2Pll),
};

static const PmClockSel2Pll iodaSel2Pll[] = {
	{
		.pll = &pmSlaveIOpll_g,
		.select = 0U,
	}, {
		.pll = &pmSlaveDpll_g,
		.select = 2U,
	}, {
		.pll = &pmSlaveApll_g,
		.select = 3U,
	},
};

static PmClockMux iodaMux = {
	.inputs = iodaSel2Pll,
	.size = ARRAY_SIZE(iodaSel2Pll),
};

static const PmClockSel2Pll iorSel2Pll[] = {
	{
		.pll = &pmSlaveIOpll_g,
		.select = 0U,
	}, {
		.pll = &pmSlaveRpll_g,
		.select = 2U,
	},
};

static PmClockMux iorMux = {
	.inputs = iorSel2Pll,
	.size = ARRAY_SIZE(iorSel2Pll),
};

static const PmClockSel2Pll iordSel2Pll[] = {
	{
		.pll = &pmSlaveIOpll_g,
		.select = 0U,
	}, {
		.pll = &pmSlaveRpll_g,
		.select = 2U,
	}, {
		.pll = &pmSlaveDpll_g,
		.select = 3U,
	},
};

static PmClockMux iordMux = {
	.inputs = iordSel2Pll,
	.size = ARRAY_SIZE(iordSel2Pll),
};

static const PmClockSel2Pll iorvSel2Pll[] = {
	{
		.pll = &pmSlaveIOpll_g,
		.select = 0U,
	}, {
		.pll = &pmSlaveRpll_g,
		.select = 2U,
	}, {
		.pll = &pmSlaveVpll_g,
		.select = 3U,
	},
};

static PmClockMux iorvMux = {
	.inputs = iorvSel2Pll,
	.size = ARRAY_SIZE(iorvSel2Pll),
};

static const PmClockSel2Pll riodSel2Pll[] = {
	{
		.pll = &pmSlaveRpll_g,
		.select = 0U,
	}, {
		.pll = &pmSlaveIOpll_g,
		.select = 2U,
	}, {
		.pll = &pmSlaveDpll_g,
		.select = 3U,
	},
};

static PmClockMux riodMux = {
	.inputs = riodSel2Pll,
	.size = ARRAY_SIZE(riodSel2Pll),
};

/* CRF_APB clocks */

static PmClock pmClockAcpu = {
	.mux = &advMux,
	.ctrlAddr = CRF_APB_ACPU_CTRL,
	.pll = NULL,
	.users = NULL,
};

/* Floating clock */
static PmClock pmClockDbgTrace = {
	.mux = &iodaMux,
	.ctrlAddr = CRF_APB_DBG_TRACE_CTRL,
	.pll = NULL,
	.users = NULL,
};

/* Floating clock */
static PmClock pmClockDbgFpd = {
	.mux = &iodaMux,
	.ctrlAddr = CRF_APB_DBG_FPD_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockDpVideo = {
	.mux = &vdrMux,
	.ctrlAddr = CRF_APB_DP_VIDEO_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockDpAudio = {
	.mux = &vdrMux,
	.ctrlAddr = CRF_APB_DP_AUDIO_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockDpStc = {
	.mux = &vdrMux,
	.ctrlAddr = CRF_APB_DP_STC_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockDdr = {
	.mux = &dvMux,
	.ctrlAddr = CRF_APB_DDR_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockGpu = {
	.mux = &iovdMux,
	.ctrlAddr = CRF_APB_GPU_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockSata = {
	.mux = &ioadMux,
	.ctrlAddr = CRF_APB_SATA_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockPcie = {
	.mux = &iordMux,
	.pll = NULL,
	.users = NULL,
	.ctrlAddr = CRF_APB_PCIE_REF_CTRL,
};

static PmClock pmClockGdma = {
	.mux = &avdMux,
	.ctrlAddr = CRF_APB_GDMA_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockDpDma = {
	.mux = &avdMux,
	.ctrlAddr = CRF_APB_DPDMA_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

/* Floating clock */
static PmClock pmClockGtgRef0 = {
	.mux = &ioadMux,
	.ctrlAddr = CRF_APB_GTGREF0_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

/* Floating clock */
static PmClock pmClockDbgTstmp = {
	.mux = &advMux,
	.ctrlAddr = CRF_APB_DBG_TSTMP_CTRL,
	.pll = NULL,
	.users = NULL,
};

/* CRL_APB clocks */

/* Floating clock */
static PmClock pmClockUsb3Dual = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_USB3_DUAL_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockGem0 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM0_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockGem1 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM1_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockGem2 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM2_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockGem3 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM3_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockUsb0Bus = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_USB0_BUS_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockUsb1Bus = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_USB1_BUS_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockQSpi = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_QSPI_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockSdio0 = {
	.mux = &iorvMux,
	.ctrlAddr = CRL_APB_SDIO0_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockSdio1 = {
	.mux = &iorvMux,
	.ctrlAddr = CRL_APB_SDIO1_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockUart0 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_UART0_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockUart1 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_UART1_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockSpi0 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_SPI0_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockSpi1 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_SPI1_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockCan0 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_CAN0_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockCan1 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_CAN1_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockCpuR5 = {
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_CPU_R5_CTRL,
	.pll = NULL,
	.users = NULL,
};

/* Floating clock */
static PmClock pmClockIouSwitch = {
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_IOU_SWITCH_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockCsuPll = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_CSU_PLL_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockPcap = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_PCAP_CTRL,
	.pll = NULL,
	.users = NULL,
};

/* Floating clock */
static PmClock pmClockLpdSwitch = {
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_LPD_SWITCH_CTRL,
	.pll = NULL,
	.users = NULL,
};

/* Floating clock */
static PmClock pmClockLpdLsBus = {
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_LPD_LSBUS_CTRL,
	.pll = NULL,
	.users = NULL,
};

/* Floating clock */
static PmClock pmClockDbgLpd = {
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_DBG_LPD_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockNand = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_NAND_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockAdma = {
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_ADMA_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockPl0 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_PL0_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockPl1 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_PL1_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockPl2 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_PL2_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockPl3 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_PL3_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockGemTsu = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_GEM_TSU_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockDll = {
	.mux = &iorMux,
	.ctrlAddr = CRL_APB_DLL_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

/* Floating clock */
static PmClock pmClockAms = {
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_AMS_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockI2C0 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_I2C0_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock pmClockI2C1 = {
	.mux = &iordMux,
	.ctrlAddr = CRL_APB_I2C1_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

/* Floating clock */
static PmClock pmClockTimeStamp = {
	.mux = &riodMux,
	.ctrlAddr = CRL_APB_TIMESTAMP_REF_CTRL,
	.pll = NULL,
	.users = NULL,
};

static PmClock* pmClocks[] = {
	&pmClockAcpu,
	&pmClockDbgTrace,
	&pmClockDbgFpd,
	&pmClockDpVideo,
	&pmClockDpAudio,
	&pmClockDpStc,
	&pmClockDdr,
	&pmClockGpu,
	&pmClockSata,
	&pmClockPcie,
	&pmClockGdma,
	&pmClockDpDma,
	&pmClockGtgRef0,
	&pmClockDbgTstmp,
	&pmClockUsb3Dual,
	&pmClockGem0,
	&pmClockGem1,
	&pmClockGem2,
	&pmClockGem3,
	&pmClockUsb0Bus,
	&pmClockUsb1Bus,
	&pmClockQSpi,
	&pmClockSdio0,
	&pmClockSdio1,
	&pmClockUart0,
	&pmClockUart1,
	&pmClockSpi0,
	&pmClockSpi1,
	&pmClockCan0,
	&pmClockCan1,
	&pmClockCpuR5,
	&pmClockIouSwitch,
	&pmClockCsuPll,
	&pmClockPcap,
	&pmClockLpdSwitch,
	&pmClockLpdLsBus,
	&pmClockDbgLpd,
	&pmClockNand,
	&pmClockAdma,
	&pmClockPl0,
	&pmClockPl1,
	&pmClockPl2,
	&pmClockPl3,
	&pmClockGemTsu,
	&pmClockDll,
	&pmClockAms,
	&pmClockI2C0,
	&pmClockI2C1,
	&pmClockTimeStamp,
};

static PmClockHandle pmClockHandles[] = {
	{
		.clock = &pmClockAcpu,
		.node = &pmPowerIslandApu_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockDpVideo,
		.node = &pmSlaveDP_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockDpAudio,
		.node = &pmSlaveDP_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockDpStc,
		.node = &pmSlaveDP_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockDdr,
		.node = &pmSlaveDdr_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockGpu,
		.node = &pmSlaveGpu_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockSata,
		.node = &pmSlaveSata_g.slv.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockPcie,
		.node = &pmSlavePcie_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockGdma,
		.node = &pmSlaveGdma_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockDpDma,
		.node = &pmSlaveDP_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockGem0,
		.node = &pmSlaveEth0_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockGem1,
		.node = &pmSlaveEth1_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockGem2,
		.node = &pmSlaveEth2_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockGem3,
		.node = &pmSlaveEth3_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockUsb0Bus,
		.node = &pmSlaveUsb0_g.slv.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockUsb1Bus,
		.node = &pmSlaveUsb1_g.slv.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockQSpi,
		.node = &pmSlaveQSpi_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockSdio0,
		.node = &pmSlaveSD0_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockSdio1,
		.node = &pmSlaveSD1_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockUart0,
		.node = &pmSlaveUart0_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockUart1,
		.node = &pmSlaveUart1_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockSpi0,
		.node = &pmSlaveSpi0_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockSpi1,
		.node = &pmSlaveSpi1_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockCan0,
		.node = &pmSlaveCan0_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockCan1,
		.node = &pmSlaveCan1_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockCpuR5,
		.node = &pmPowerIslandRpu_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockCsuPll,
		.node = &pmSlavePcap_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockPcap,
		.node = &pmSlavePcap_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockNand,
		.node = &pmSlaveNand_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockAdma,
		.node = &pmSlaveAdma_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockPl0,
		.node = &pmPowerDomainPld_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockPl1,
		.node = &pmPowerDomainPld_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockPl2,
		.node = &pmPowerDomainPld_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockPl3,
		.node = &pmPowerDomainPld_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockGemTsu,
		.node = &pmSlaveEth0_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockGemTsu,
		.node = &pmSlaveEth1_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockGemTsu,
		.node = &pmSlaveEth2_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockGemTsu,
		.node = &pmSlaveEth3_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockDll,
		.node = &pmSlaveSD0_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockDll,
		.node = &pmSlaveSD1_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockI2C0,
		.node = &pmSlaveI2C0_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	}, {
		.clock = &pmClockI2C1,
		.node = &pmSlaveI2C1_g.node,
		.nextClock = NULL,
		.nextNode = NULL,
	},
};

/**
 * PmClockGetUseCount() - Get the use count for the clock
 * @clk		Clock whose use count shall be counted
 *
 * @return	How many nodes use the clock
 */
static u32 PmClockGetUseCount(const PmClock* const clk)
{
	u32 useCnt = 0U;
	PmClockHandle* ch = clk->users;

	while (NULL != ch) {
		bool depends = PmNodeDependsOnClock(ch->node);

		if (true == depends) {
			useCnt++;
		}

		ch = ch->nextNode;
	}

	return useCnt;
}

/**
 * PmClockGetParent() - Get PLL parent of the clock based on MUX select value
 * @clock	Pointer to the clock whose PLL parent shall be find
 * @muxSel	Multiplexer select value
 *
 * @return	Pointer to the PLL parent of the clock
 */
static PmSlavePll* PmClockGetParent(PmClock* const clock, const u32 sel)
{
	u32 i;
	PmSlavePll* parent = NULL;

	for (i = 0U; i < clock->mux->size; i++) {
		if (sel == clock->mux->inputs[i].select) {
			parent = clock->mux->inputs[i].pll;
			break;
		}
	}

	return parent;
}

/**
 * PmClockInitList() - Init and link clock handles into clock's/node's lists
 */
void PmClockInitList(void)
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
 * PmClockInitData() - Initialize clock data structures based on HW setting
 * @note	Must execute only once upon the system init
 */
void PmClockInitData(void)
{
	u32 i;

	PmPllClearUseCount();

	/* Read the current hardware configuration for all clocks */
	for (i = 0U; i < ARRAY_SIZE(pmClocks); i++) {
		PmClock* const clk = pmClocks[i];
		const u32 val = XPfw_Read32(clk->ctrlAddr);
		u32 clkUseCnt;

		clk->pll = PmClockGetParent(clk, val & PM_CLOCK_MUX_SELECT_MASK);

		/* If parent is not a known pll it's the oscillator clock */
		if (NULL == clk->pll) {
			continue;
		}

		/*
		 * Increase the use count of the PLL parent by the number of
		 * nodes that are in a state that requires clock to be running.
		 */
		clkUseCnt = PmClockGetUseCount(clk);
		clk->pll->useCount += clkUseCnt;
	}
}

/**
 * PmClockRequest() - Request clocks used by the given node
 * @node	Node whose clocks need to be requested
 * @return	XST_SUCCESS if the request is processed correctly, or error code
 *		if a PLL parent needed to be locked and the locking has failed.
 * @note	The dependency toward a PLL parent is automatically resolved
 */
int PmClockRequest(const PmNode* const node)
{
	PmClockHandle* ch = node->clocks;
	int totStatus = XST_SUCCESS;

	while (NULL != ch) {
		int status = XST_SUCCESS;
		u32 clkUseCnt = PmClockGetUseCount(ch->clock);

		if ((0U == clkUseCnt) && (NULL != ch->clock->pll)) {
			/* This node is the first one to depend on the PLL */
			status = PmPllRequest(ch->clock->pll);
		}

		/* If requesting the PLL failed, remember to return the error */
		if (XST_SUCCESS != status) {
			totStatus = status;
		}

		ch = ch->nextClock;
	}

	return totStatus;
}

/**
 * PmClockRelease() - Release clocks used by the given node
 * @node	Node whose clocks are released
 *
 * @note	If a PLL parent of a released clock have no other users, the
 *		PM framework will suspend that PLL.
 */
void PmClockRelease(const PmNode* const node)
{
	PmClockHandle* ch = node->clocks;

	while (NULL != ch) {
		u32 clkUseCnt = PmClockGetUseCount(ch->clock);

		if ((0U == clkUseCnt) && (NULL != ch->clock->pll)) {
			PmPllRelease(ch->clock->pll);
		}
		ch = ch->nextClock;
	}
}
