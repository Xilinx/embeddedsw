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
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

/*********************************************************************
 * DDR slave definition
 *
 * Note: In ON state DDR depends on FPD (cannot be accessed if FPD is
 * not on). Therefore, power parent of DDR is FPD.
 *********************************************************************/

#include "crf_apb.h"
#include "pm_ddr.h"
#include "pm_common.h"
#include "pm_defs.h"
#include "pm_master.h"
#include "xpfw_util.h"

#define DDRC_BASE		0xFD070000U
#define DDRC_MSTR		(DDRC_BASE + 0U)
#define DDRC_STAT		(DDRC_BASE + 4U)
#define DDRC_MRCTRL0		(DDRC_BASE + 0X10U)
#define DDRC_DERATEEN		(DDRC_BASE + 0X20U)
#define DDRC_DERATEINT		(DDRC_BASE + 0X24U)
#define DDRC_PWRCTL		(DDRC_BASE + 0x30U)
#define DDRC_PWRTMG		(DDRC_BASE + 0X34U)
#define DDRC_RFSHCTL0		(DDRC_BASE + 0X50U)
#define DDRC_RFSHCTL3		(DDRC_BASE + 0x60U)
#define DDRC_RFSHTMG		(DDRC_BASE + 0x64U)
#define DDRC_ECCCFG0		(DDRC_BASE + 0X70U)
#define DDRC_ECCCFG1		(DDRC_BASE + 0X74U)
#define DDRC_CRCPARCTL1		(DDRC_BASE + 0XC4U)
#define DDRC_CRCPARCTL2		(DDRC_BASE + 0XC8U)
#define DDRC_INIT(n)		(DDRC_BASE + 0xD0U + (4U * (n)))
#define DDRC_DIMMCTL		(DDRC_BASE + 0XF0U)
#define DDRC_RANKCTL		(DDRC_BASE + 0XF4U)
#define DDRC_DRAMTMG(n)		(DDRC_BASE + 0x100U + (4U * (n)))
#define DDRC_ZQCTL(n)		(DDRC_BASE + 0x180U + (4U * (n)))
#define DDRC_DFITMG0		(DDRC_BASE + 0X190U)
#define DDRC_DFITMG1		(DDRC_BASE + 0X194U)
#define DDRC_DFILPCFG(n)	(DDRC_BASE + 0X198U + (4U * (n)))
#define DDRC_DFIUPD1		(DDRC_BASE + 0X1A4U)
#define DDRC_DFIMISC		(DDRC_BASE + 0x1b0U)
#define DDRC_DFITMG2		(DDRC_BASE + 0X1B4U)
#define DDRC_DBICTL		(DDRC_BASE + 0X1C0U)
#define DDRC_ADDRMAP(n)		(DDRC_BASE + 0X200U + (4U * (n)))
#define DDRC_ODTCFG		(DDRC_BASE + 0X240U)
#define DDRC_ODTMAP		(DDRC_BASE + 0X244U)
#define DDRC_SCHED		(DDRC_BASE + 0X250U)
#define DDRC_PERFLPR1		(DDRC_BASE + 0X264U)
#define DDRC_PERFWR1		(DDRC_BASE + 0X26CU)
#define DDRC_DQMAP5		(DDRC_BASE + 0X294U)
#define DDRC_DBG0		(DDRC_BASE + 0X300U)
#define DDRC_DBGCMD		(DDRC_BASE + 0X30CU)
#define DDRC_SWCTL		(DDRC_BASE + 0x320U)
#define DDRC_SWSTAT		(DDRC_BASE + 0x324U)
#define DDRC_PSTAT		(DDRC_BASE + 0x3fcU)
#define DDRC_PCCFG		(DDRC_BASE + 0X400U)
#define DDRC_PCFGR(n)		(DDRC_BASE + 0X404U + (0xb0U * (n)))
#define DDRC_PCFGW(n)		(DDRC_BASE + 0X408U + (0xb0U * (n)))
#define DDRC_PCTRL(n)		(DDRC_BASE + 0x490U + (0xb0U * (n)))
#define DDRC_PCFGQOS(n, m)	(DDRC_BASE + 0X494U + (4U * (n)) + (0xb0U * (m)))
#define DDRC_PCFGWQOS(n, m)	(DDRC_BASE + 0x49cU + (4U * (n)) + (0xb0U * (m)))
#define DDRC_SARBASE(n)		(DDRC_BASE + 0XF04U + (8U * (n)))
#define DDRC_SARSIZE(n)		(DDRC_BASE + 0XF08U + (8U * (n)))
#define DDRC_DFITMG0_SHADOW	(DDRC_BASE + 0X2190U)

#define DDRC_PWRCTL_SR_SW	BIT(5U)

#define DDRC_MSTR_LPDDR3		BIT(3U)
#define DDRC_MSTR_LPDDR4		BIT(5U)

#define DDRC_STAT_OPMODE_MASK	7U
#define DDRC_STAT_OPMODE_SHIFT	0U
#define DDRC_STAT_OPMODE_INIT	0U
#define DDRC_STAT_OPMODE_NORMAL	1U
#define DDRC_STAT_OPMODE_SR	3U

#define DDRC_SWSTAT_SWDONE	BIT(0U)

#define DDRC_PSTAT_PORT_BUSY(n)	((BIT(0U) | BIT(16U)) << (n))

#define DDRC_PCTRL_PORT_EN	BIT(0U)

#define DDRC_ZQCTL0_ZQ_DIS	BIT(31U)

#define DDRC_RFSHCTL3_AUTORF_DIS	BIT(0U)

#define DDRC_DFIMISC_DFI_INIT_COMP_EN	BIT(0U)

#define DDRC_SWCTL_SW_DONE	BIT(0U)

#define DDRPHY_BASE		0xFD080000U
#define DDRPHY_PIR		(DDRPHY_BASE + 4U)
#define DDRPHY_PGCR(n)		(DDRPHY_BASE + 0x10U + (4U * (n)))
#define DDRPHY_PGSR(n)		(DDRPHY_BASE + 0x30U + (4U * (n)))
#define DDRPHY_PTR(n)		(DDRPHY_BASE + 0X40U + (4U * (n)))
#define DDRPHY_DSGCR		(DDRPHY_BASE + 0X90U)
#define DDRPHY_DCR		(DDRPHY_BASE + 0X100U)
#define DDRPHY_DTPR(n)		(DDRPHY_BASE + 0X110U + (4U * (n)))
#define DDRPHY_RDIMMGCR(n)	(DDRPHY_BASE + 0x140U + (4U * (n)))
#define DDRPHY_RDIMMCR(n)	(DDRPHY_BASE + 0x150U + (4U * (n)))
#define DDRPHY_MR(n)		(DDRPHY_BASE + 0X180U + (4U * (n)))
#define DDRPHY_DTCR(n)		(DDRPHY_BASE + 0X200U + (4U * (n)))
#define DDRPHY_CATR(n)		(DDRPHY_BASE + 0X240U + (4U * (n)))
#define DDRPHY_RIOCR(n)		(DDRPHY_BASE + 0X4E0U + (4U * (n)))
#define DDRPHY_ACIOCR(n)	(DDRPHY_BASE + 0X500U + (4U * (n)))
#define DDRPHY_IOVCR(n)		(DDRPHY_BASE + 0X520U + (4U * (n)))
#define DDRPHY_VTCR(n)		(DDRPHY_BASE + 0X528U + (4U * (n)))
#define DDRPHY_DQSDR(n)		(DDRPHY_BASE + 0x250U + (4U * (n)))
#define DDRPHY_ACBDLR(n)	(DDRPHY_BASE + 0x540U + (4U * (n)))
#define DDRPHY_ZQCR		(DDRPHY_BASE + 0x680U)
#define DDRPHY_ZQPR(n, m)	(DDRPHY_BASE + 0x684U + (0x20U * (n)) + (4U * (m)))
#define DDRPHY_ZQDR0(n)		(DDRPHY_BASE + 0x68cU + (0x20U * (n)))
#define DDRPHY_ZQDR1(n)		(DDRPHY_BASE + 0x690U + (0x20U * (n)))
#define DDRPHY_DXGCR(n, m)	(DDRPHY_BASE + 0X700U + (0x100U * (n)) + (4U * (m)))
#define DDRPHY_DXGTR0(n)	(DDRPHY_BASE + 0X7c0U + (0x100U * (n)))
#define DDRPHY_DX8SLNOSC(n)	(DDRPHY_BASE + 0x1400U + (0x40U * (n)))
#define DDRPHY_DX8SLDQSCTL(n)	(DDRPHY_BASE + 0x141cU + (0x40U * (n)))
#define DDRPHY_DX8SLDXCTL2(n)	(DDRPHY_BASE + 0x142cU + (0x40U * (n)))
#define DDRPHY_DX8SLIOCR(n)	(DDRPHY_BASE + 0x1430U + (0x40U * (n)))
#define DDRPHY_DX8SLBOSC	(DDRPHY_BASE + 0x17c0U)

#define DDRPHY_PIR_INIT			BIT(0U)
#define DDRPHY_PIR_ZCAL			BIT(1U)
#define DDRPHY_PIR_PLLINIT		BIT(4U)
#define DDRPHY_PIR_DCAL			BIT(5U)
#define DDRPHY_PIR_PHYRST		BIT(6U)
#define DDRPHY_PIR_WL			BIT(9U)
#define DDRPHY_PIR_QSGATE		BIT(10U)
#define DDRPHY_PIR_WLADJ		BIT(11U)
#define DDRPHY_PIR_RDDSKW		BIT(12U)
#define DDRPHY_PIR_WRDSKW		BIT(13U)
#define DDRPHY_PIR_RDEYE		BIT(14U)
#define DDRPHY_PIR_WREYE		BIT(15U)
#define DDRPHY_PIR_VREF			BIT(17U)
#define DDRPHY_PIR_CTLDINIT		BIT(18U)
#define DDRPHY_PIR_RDIMMINIT		BIT(19U)
#define DDRPHY_PIR_ZCALBYP		BIT(30U)

#define DDRPHY_PGCR0_PHYFRST	BIT(26U)

#define DDRPHY_PGSR0_SRDERR	BIT(30U)
#define DDRPHY_PGSR0_CAWRN	BIT(29U)
#define DDRPHY_PGSR0_CAERR	BIT(28U)
#define DDRPHY_PGSR0_WEERR	BIT(27U)
#define DDRPHY_PGSR0_REERR	BIT(26U)
#define DDRPHY_PGSR0_WDERR	BIT(25U)
#define DDRPHY_PGSR0_RDERR	BIT(24U)
#define DDRPHY_PGSR0_WLAERR	BIT(23U)
#define DDRPHY_PGSR0_QSGERR	BIT(22U)
#define DDRPHY_PGSR0_WLERR	BIT(21U)
#define DDRPHY_PGSR0_ZCERR	BIT(20U)
#define DDRPHY_PGSR0_VERR	BIT(19U)
#define DDRPHY_PGSR0_DQS2DQERR	BIT(18U)
#define DDRPHY_PGSR0_IDONE	BIT(0U)
#define DDRPHY_PGSR0_TRAIN_ERRS	(DDRPHY_PGSR0_DQS2DQERR | \
				 DDRPHY_PGSR0_VERR | \
				 DDRPHY_PGSR0_ZCERR | \
				 DDRPHY_PGSR0_WLERR | \
				 DDRPHY_PGSR0_QSGERR | \
				 DDRPHY_PGSR0_WLAERR | \
				 DDRPHY_PGSR0_RDERR | \
				 DDRPHY_PGSR0_WDERR | \
				 DDRPHY_PGSR0_REERR | \
				 DDRPHY_PGSR0_WEERR | \
				 DDRPHY_PGSR0_CAERR | \
				 DDRPHY_PGSR0_CAWRN | \
				 DDRPHY_PGSR0_SRDERR)

#define DDRPHY_RDIMMGCR0_RDIMM	BIT(0U)

#define DDRPHY_DQSDR0_DFTDTEN		BIT(0U)
#define DDRPHY_DQSDR0_DFTDTMODE		BIT(1U)
#define DDRPHY_DQSDR0_DFTUPMODE		(BIT(2U) | BIT(3U))
#define DDRPHY_DQSDR0_DFTUPMODE_SHIFT	2U
#define DDRPHY_DQSDR0_DFTGPULSE		(BIT(4U) | BIT(5U) | BIT(6U) | BIT(7U))
#define DDRPHY_DQSDR0_DFTRDSPC		(BIT(20U) | BIT(21U))
#define DDRPHY_DQSDR0_DFTRDSPC_SHIFT	20U
#define DDRPHY_DQSDR0_DFTDLY		(BIT(28U) | BIT(29U) | BIT(30U) | BIT(31U))
#define DDRPHY_DQSDR0_DFTDLY_SHIFT	28U

#define DDRPHY_DQSDR1_DFTRDIDLC		0x000000FFU
#define DDRPHY_DQSDR1_DFTRDIDLC_SHIFT	0U
#define DDRPHY_DQSDR1_DFTRDIDLF		0x000F0000U
#define DDRPHY_DQSDR1_DFTRDIDLF_SHIFT	16U

#define DDRPHY_DX8SLBOSC_PHYFRST	BIT(15U)

#define DDRPHY_DTCR0_INCWEYE		BIT(4U)

#define DDRPHY_DSGCR_CTLZUEN		BIT(2U)

#define DDRPHY_DXGCR3_WDLVT		BIT(25U)
#define DDRPHY_DXGCR3_RGLVT		BIT(27U)

#define DDRQOS_BASE		0xFD090000U
#define DDRQOS_DDR_CLK_CTRL	(DDRQOS_BASE + 0x700U)

#define DDRQOS_DDR_CLK_CTRL_CLKACT	BIT(0U)

/* Power states of DDR */
#define PM_DDR_STATE_OFF	0U
#define PM_DDR_STATE_SR		1U
#define PM_DDR_STATE_ON		2U
#define PM_DDR_STATE_MAX	3U

/* Power consumptions for DDR defined by its states */
#define DEFAULT_DDR_POWER_ON		100U
#define DEFAULT_DDR_POWER_SR		50U
#define DEFAULT_DDR_POWER_OFF		0U

/* DDR states */
static const u32 pmDdrStates[PM_DDR_STATE_MAX] = {
	[PM_DDR_STATE_OFF] = 0U,
	[PM_DDR_STATE_SR] = PM_CAP_CONTEXT,
	[PM_DDR_STATE_ON] = PM_CAP_ACCESS | PM_CAP_CONTEXT | PM_CAP_POWER |
			    PM_CAP_CLOCK,
};

/* DDR transition table (from which to which state DDR can transit) */
static const PmStateTran pmDdrTransitions[] = {
	{
		.fromState = PM_DDR_STATE_ON,
		.toState = PM_DDR_STATE_SR,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_DDR_STATE_SR,
		.toState = PM_DDR_STATE_ON,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_DDR_STATE_ON,
		.toState = PM_DDR_STATE_OFF,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_DDR_STATE_OFF,
		.toState = PM_DDR_STATE_ON,
		.latency = PM_DEFAULT_LATENCY,
	},
};

static PmRegisterContext ctx_ddrc[] = {
	{ .addr = DDRC_MSTR, },
	{ .addr = DDRC_MRCTRL0, },
	{ .addr = DDRC_DERATEEN, },
	{ .addr = DDRC_DERATEINT, },
	{ .addr = DDRC_PWRCTL, },
	{ .addr = DDRC_PWRTMG, },
	{ .addr = DDRC_RFSHCTL0, },
	{ .addr = DDRC_RFSHCTL3, },
	{ .addr = DDRC_RFSHTMG, },
	{ .addr = DDRC_ECCCFG0, },
	{ .addr = DDRC_ECCCFG1, },
	{ .addr = DDRC_CRCPARCTL1, },
	{ .addr = DDRC_CRCPARCTL2, },
	{ .addr = DDRC_INIT(0U), },
	{ .addr = DDRC_INIT(1U), },
	{ .addr = DDRC_INIT(2U), },
	{ .addr = DDRC_INIT(3U), },
	{ .addr = DDRC_INIT(4U), },
	{ .addr = DDRC_INIT(5U), },
	{ .addr = DDRC_INIT(6U), },
	{ .addr = DDRC_INIT(7U), },
	{ .addr = DDRC_DIMMCTL, },
	{ .addr = DDRC_RANKCTL, },
	{ .addr = DDRC_DRAMTMG(0U), },
	{ .addr = DDRC_DRAMTMG(1U), },
	{ .addr = DDRC_DRAMTMG(2U), },
	{ .addr = DDRC_DRAMTMG(3U), },
	{ .addr = DDRC_DRAMTMG(4U), },
	{ .addr = DDRC_DRAMTMG(5U), },
	{ .addr = DDRC_DRAMTMG(6U), },
	{ .addr = DDRC_DRAMTMG(7U), },
	{ .addr = DDRC_DRAMTMG(8U), },
	{ .addr = DDRC_DRAMTMG(9U), },
	{ .addr = DDRC_DRAMTMG(11U), },
	{ .addr = DDRC_DRAMTMG(12U), },
	{ .addr = DDRC_ZQCTL(0U), },
	{ .addr = DDRC_ZQCTL(1U), },
	{ .addr = DDRC_DFITMG0, },
	{ .addr = DDRC_DFITMG1, },
	{ .addr = DDRC_DFILPCFG(0U), },
	{ .addr = DDRC_DFILPCFG(1U), },
	{ .addr = DDRC_DFIUPD1, },
	{ .addr = DDRC_DFIMISC, },
	{ .addr = DDRC_DFITMG2, },
	{ .addr = DDRC_DBICTL, },
	{ .addr = DDRC_ADDRMAP(0U), },
	{ .addr = DDRC_ADDRMAP(1U), },
	{ .addr = DDRC_ADDRMAP(2U), },
	{ .addr = DDRC_ADDRMAP(3U), },
	{ .addr = DDRC_ADDRMAP(4U), },
	{ .addr = DDRC_ADDRMAP(5U), },
	{ .addr = DDRC_ADDRMAP(6U), },
	{ .addr = DDRC_ADDRMAP(7U), },
	{ .addr = DDRC_ADDRMAP(8U), },
	{ .addr = DDRC_ADDRMAP(9U), },
	{ .addr = DDRC_ADDRMAP(10U), },
	{ .addr = DDRC_ADDRMAP(11U), },
	{ .addr = DDRC_ODTCFG, },
	{ .addr = DDRC_ODTMAP, },
	{ .addr = DDRC_SCHED, },
	{ .addr = DDRC_PERFLPR1, },
	{ .addr = DDRC_PERFWR1, },
	{ .addr = DDRC_DQMAP5, },
	{ .addr = DDRC_DBG0, },
	{ .addr = DDRC_DBGCMD, },
	{ .addr = DDRC_PCCFG, },
	{ .addr = DDRC_PCFGR(0U), },
	{ .addr = DDRC_PCFGW(0U), },
	{ .addr = DDRC_PCTRL(0U), },
	{ .addr = DDRC_PCFGQOS(0U, 0U), },
	{ .addr = DDRC_PCFGQOS(1U, 0U), },
	{ .addr = DDRC_PCFGR(1U), },
	{ .addr = DDRC_PCFGW(1U), },
	{ .addr = DDRC_PCTRL(1U), },
	{ .addr = DDRC_PCFGQOS(0U, 1U), },
	{ .addr = DDRC_PCFGQOS(1U, 1U), },
	{ .addr = DDRC_PCFGR(2U), },
	{ .addr = DDRC_PCFGW(2U), },
	{ .addr = DDRC_PCTRL(2U), },
	{ .addr = DDRC_PCFGQOS(0U, 2U), },
	{ .addr = DDRC_PCFGQOS(1U, 2U), },
	{ .addr = DDRC_PCFGR(3U), },
	{ .addr = DDRC_PCFGW(3U), },
	{ .addr = DDRC_PCTRL(3U), },
	{ .addr = DDRC_PCFGQOS(0U, 3U), },
	{ .addr = DDRC_PCFGQOS(1U, 3U), },
	{ .addr = DDRC_PCFGWQOS(0U, 3U), },
	{ .addr = DDRC_PCFGWQOS(1U, 3U), },
	{ .addr = DDRC_PCFGR(4U), },
	{ .addr = DDRC_PCFGW(4U), },
	{ .addr = DDRC_PCTRL(4U), },
	{ .addr = DDRC_PCFGQOS(0U, 4U), },
	{ .addr = DDRC_PCFGQOS(1U, 4U), },
	{ .addr = DDRC_PCFGWQOS(0U, 4U), },
	{ .addr = DDRC_PCFGWQOS(1U, 4U), },
	{ .addr = DDRC_PCFGR(5U), },
	{ .addr = DDRC_PCFGW(5U), },
	{ .addr = DDRC_PCTRL(5U), },
	{ .addr = DDRC_PCFGQOS(0U, 5U), },
	{ .addr = DDRC_PCFGQOS(1U, 5U), },
	{ .addr = DDRC_PCFGWQOS(0U, 5U), },
	{ .addr = DDRC_PCFGWQOS(1U, 5U), },
	{ .addr = DDRC_SARBASE(0U), },
	{ .addr = DDRC_SARSIZE(0U), },
	{ .addr = DDRC_SARBASE(1U), },
	{ .addr = DDRC_SARSIZE(1U), },
	{ },
};

static PmRegisterContext ctx_ddrphy[] = {
	{ .addr = DDRPHY_PGCR(0U), },
	{ .addr = DDRPHY_PGCR(2U), },
	{ .addr = DDRPHY_PGCR(3U), },
	{ .addr = DDRPHY_PGCR(5U), },
	{ .addr = DDRPHY_PTR(0U), },
	{ .addr = DDRPHY_PTR(1U), },
	{ .addr = DDRPHY_DSGCR, },
	{ .addr = DDRPHY_DCR, },
	{ .addr = DDRPHY_DTPR(0U), },
	{ .addr = DDRPHY_DTPR(1U), },
	{ .addr = DDRPHY_DTPR(2U), },
	{ .addr = DDRPHY_DTPR(3U), },
	{ .addr = DDRPHY_DTPR(4U), },
	{ .addr = DDRPHY_DTPR(5U), },
	{ .addr = DDRPHY_DTPR(6U), },
	{ .addr = DDRPHY_RDIMMGCR(0U), },
	{ .addr = DDRPHY_RDIMMGCR(1U), },
	{ .addr = DDRPHY_RDIMMCR(0U), },
	{ .addr = DDRPHY_RDIMMCR(1U), },
	{ .addr = DDRPHY_MR(0U), },
	{ .addr = DDRPHY_MR(1U), },
	{ .addr = DDRPHY_MR(2U), },
	{ .addr = DDRPHY_MR(3U), },
	{ .addr = DDRPHY_MR(4U), },
	{ .addr = DDRPHY_MR(5U), },
	{ .addr = DDRPHY_MR(6U), },
	{ .addr = DDRPHY_MR(11U), },
	{ .addr = DDRPHY_MR(12U), },
	{ .addr = DDRPHY_MR(13U), },
	{ .addr = DDRPHY_MR(14U), },
	{ .addr = DDRPHY_MR(22U), },
	{ .addr = DDRPHY_DTCR(0U), },
	{ .addr = DDRPHY_DTCR(1U), },
	{ .addr = DDRPHY_CATR(0U), },
	{ .addr = DDRPHY_RIOCR(5U), },
	{ .addr = DDRPHY_ACIOCR(0U), },
	{ .addr = DDRPHY_ACIOCR(2U), },
	{ .addr = DDRPHY_ACIOCR(3U), },
	{ .addr = DDRPHY_ACIOCR(4U), },
	{ .addr = DDRPHY_IOVCR(0U), },
	{ .addr = DDRPHY_VTCR(0U), },
	{ .addr = DDRPHY_VTCR(1U), },
	{ .addr = DDRPHY_DQSDR(0U), },
	{ .addr = DDRPHY_DQSDR(1U), },
	{ .addr = DDRPHY_ACBDLR(1U), },
	{ .addr = DDRPHY_ACBDLR(2U), },
	{ .addr = DDRPHY_ACBDLR(6U), },
	{ .addr = DDRPHY_ACBDLR(7U), },
	{ .addr = DDRPHY_ACBDLR(8U), },
	{ .addr = DDRPHY_ACBDLR(9U), },
	{ .addr = DDRPHY_ZQCR, },
	{ .addr = DDRPHY_ZQPR(0U, 0U), },
	{ .addr = DDRPHY_ZQPR(1U, 0U), },
	{ .addr = DDRPHY_DXGCR(0U, 0U), },
	{ .addr = DDRPHY_DXGCR(0U, 3U), },
	{ .addr = DDRPHY_DXGCR(0U, 4U), },
	{ .addr = DDRPHY_DXGCR(0U, 5U), },
	{ .addr = DDRPHY_DXGCR(0U, 6U), },
	{ .addr = DDRPHY_DXGTR0(0U), },
	{ .addr = DDRPHY_DXGCR(1U, 0U), },
	{ .addr = DDRPHY_DXGCR(1U, 3U), },
	{ .addr = DDRPHY_DXGCR(1U, 4U), },
	{ .addr = DDRPHY_DXGCR(1U, 5U), },
	{ .addr = DDRPHY_DXGCR(1U, 6U), },
	{ .addr = DDRPHY_DXGTR0(1U), },
	{ .addr = DDRPHY_DXGCR(2U, 0U), },
	{ .addr = DDRPHY_DXGCR(2U, 1U), },
	{ .addr = DDRPHY_DXGCR(2U, 3U), },
	{ .addr = DDRPHY_DXGCR(2U, 4U), },
	{ .addr = DDRPHY_DXGCR(2U, 5U), },
	{ .addr = DDRPHY_DXGCR(2U, 6U), },
	{ .addr = DDRPHY_DXGTR0(2U), },
	{ .addr = DDRPHY_DXGCR(3U, 0U), },
	{ .addr = DDRPHY_DXGCR(3U, 1U), },
	{ .addr = DDRPHY_DXGCR(3U, 3U), },
	{ .addr = DDRPHY_DXGCR(3U, 4U), },
	{ .addr = DDRPHY_DXGCR(3U, 5U), },
	{ .addr = DDRPHY_DXGCR(3U, 6U), },
	{ .addr = DDRPHY_DXGTR0(3U), },
	{ .addr = DDRPHY_DXGCR(4U, 0U), },
	{ .addr = DDRPHY_DXGCR(4U, 1U), },
	{ .addr = DDRPHY_DXGCR(4U, 3U), },
	{ .addr = DDRPHY_DXGCR(4U, 4U), },
	{ .addr = DDRPHY_DXGCR(4U, 5U), },
	{ .addr = DDRPHY_DXGCR(4U, 6U), },
	{ .addr = DDRPHY_DXGTR0(4U), },
	{ .addr = DDRPHY_DXGCR(5U, 0U), },
	{ .addr = DDRPHY_DXGCR(5U, 1U), },
	{ .addr = DDRPHY_DXGCR(5U, 3U), },
	{ .addr = DDRPHY_DXGCR(5U, 4U), },
	{ .addr = DDRPHY_DXGCR(5U, 5U), },
	{ .addr = DDRPHY_DXGCR(5U, 6U), },
	{ .addr = DDRPHY_DXGTR0(5U), },
	{ .addr = DDRPHY_DXGCR(6U, 0U), },
	{ .addr = DDRPHY_DXGCR(6U, 1U), },
	{ .addr = DDRPHY_DXGCR(6U, 3U), },
	{ .addr = DDRPHY_DXGCR(6U, 4U), },
	{ .addr = DDRPHY_DXGCR(6U, 5U), },
	{ .addr = DDRPHY_DXGCR(6U, 6U), },
	{ .addr = DDRPHY_DXGTR0(6U), },
	{ .addr = DDRPHY_DXGCR(7U, 0U), },
	{ .addr = DDRPHY_DXGCR(7U, 1U), },
	{ .addr = DDRPHY_DXGCR(7U, 3U), },
	{ .addr = DDRPHY_DXGCR(7U, 4U), },
	{ .addr = DDRPHY_DXGCR(7U, 5U), },
	{ .addr = DDRPHY_DXGCR(7U, 6U), },
	{ .addr = DDRPHY_DXGTR0(7U), },
	{ .addr = DDRPHY_DXGCR(8U, 0U), },
	{ .addr = DDRPHY_DXGCR(8U, 1U), },
	{ .addr = DDRPHY_DXGCR(8U, 3U), },
	{ .addr = DDRPHY_DXGCR(8U, 4U), },
	{ .addr = DDRPHY_DXGCR(8U, 5U), },
	{ .addr = DDRPHY_DXGCR(8U, 6U), },
	{ .addr = DDRPHY_DXGTR0(8U), },
	{ .addr = DDRPHY_DX8SLNOSC(0U), },
	{ .addr = DDRPHY_DX8SLDQSCTL(0U), },
	{ .addr = DDRPHY_DX8SLDXCTL2(0U), },
	{ .addr = DDRPHY_DX8SLIOCR(0U), },
	{ .addr = DDRPHY_DX8SLNOSC(1U), },
	{ .addr = DDRPHY_DX8SLDQSCTL(1U), },
	{ .addr = DDRPHY_DX8SLDXCTL2(1U), },
	{ .addr = DDRPHY_DX8SLIOCR(1U), },
	{ .addr = DDRPHY_DX8SLNOSC(2U), },
	{ .addr = DDRPHY_DX8SLDQSCTL(2U), },
	{ .addr = DDRPHY_DX8SLDXCTL2(2U), },
	{ .addr = DDRPHY_DX8SLIOCR(2U), },
	{ .addr = DDRPHY_DX8SLNOSC(3U), },
	{ .addr = DDRPHY_DX8SLDQSCTL(3U), },
	{ .addr = DDRPHY_DX8SLDXCTL2(3U), },
	{ .addr = DDRPHY_DX8SLIOCR(3U), },
	{ .addr = DDRPHY_DX8SLNOSC(4U), },
	{ .addr = DDRPHY_DX8SLDQSCTL(4U), },
	{ .addr = DDRPHY_DX8SLDXCTL2(4U), },
	{ .addr = DDRPHY_DX8SLIOCR(4U), },
	{ },
};

static PmRegisterContext ctx_ddrphy_zqdata[] = {
	{ .addr = DDRPHY_ZQDR0(0U), },
	{ .addr = DDRPHY_ZQDR1(0U), },
	{ .addr = DDRPHY_ZQDR0(1U), },
	{ .addr = DDRPHY_ZQDR1(1U), },
	{ .addr = DDRPHY_ZQDR0(2U), },
	{ .addr = DDRPHY_ZQDR1(2U), },
	{ .addr = DDRPHY_ZQDR0(3U), },
	{ .addr = DDRPHY_ZQDR1(3U), },
	{ },
};

static void ddr_disable_wr_drift(void)
{
	u32 r;

	r = Xil_In32(DDRPHY_DTCR(0U));
	r &= ~DDRPHY_DTCR0_INCWEYE;
	Xil_Out32(DDRPHY_DTCR(0U), r);

	r = Xil_In32(DDRPHY_DSGCR);
	r &= ~DDRPHY_DSGCR_CTLZUEN;
	Xil_Out32(DDRPHY_DSGCR, r);

	r = Xil_In32(DDRPHY_DXGCR(0U, 3U));
	r |= DDRPHY_DXGCR3_WDLVT;
	Xil_Out32(DDRPHY_DXGCR(0U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(1U, 3U));
	r |= DDRPHY_DXGCR3_WDLVT;
	Xil_Out32(DDRPHY_DXGCR(1U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(2U, 3U));
	r |= DDRPHY_DXGCR3_WDLVT;
	Xil_Out32(DDRPHY_DXGCR(2U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(3U, 3U));
	r |= DDRPHY_DXGCR3_WDLVT;
	Xil_Out32(DDRPHY_DXGCR(3U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(8U, 3U));
	r |= DDRPHY_DXGCR3_WDLVT;
	Xil_Out32(DDRPHY_DXGCR(8U, 3U), r);
}

static void ddr_disable_rd_drift(void)
{
	u32 r;

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTDTEN;
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTDTMODE;
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTUPMODE;
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTGPULSE;
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTRDSPC;
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTDLY;
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DXGCR(0U, 3U));
	r |= DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(0U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(1U, 3U));
	r |= DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(1U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(2U, 3U));
	r |= DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(2U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(3U, 3U));
	r |= DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(3U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(4U, 3U));
	r |= DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(4U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(5U, 3U));
	r |= DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(5U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(6U, 3U));
	r |= DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(6U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(7U, 3U));
	r |= DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(7U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(8U, 3U));
	r |= DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(8U, 3U), r);

	r = Xil_In32(DDRPHY_DQSDR(1U));
	r &= ~DDRPHY_DQSDR1_DFTRDIDLC;
	r |= (1U << DDRPHY_DQSDR1_DFTRDIDLC_SHIFT);
	Xil_Out32(DDRPHY_DQSDR(1U), r);

	r = Xil_In32(DDRPHY_DQSDR(1U));
	r &= ~DDRPHY_DQSDR1_DFTRDIDLF;
	r |= (10U << DDRPHY_DQSDR1_DFTRDIDLF_SHIFT);
	Xil_Out32(DDRPHY_DQSDR(1U), r);
}

static void ddr_enable_wr_drift(void)
{
	u32 r;

	r = Xil_In32(DDRPHY_DSGCR);
	r |= DDRPHY_DSGCR_CTLZUEN;
	Xil_Out32(DDRPHY_DSGCR, r);

	r = Xil_In32(DDRPHY_DXGCR(0U, 3U));
	r &= ~DDRPHY_DXGCR3_WDLVT;
	Xil_Out32(DDRPHY_DXGCR(0U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(1U, 3U));
	r &= ~DDRPHY_DXGCR3_WDLVT;
	Xil_Out32(DDRPHY_DXGCR(1U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(2U, 3U));
	r &= ~DDRPHY_DXGCR3_WDLVT;
	Xil_Out32(DDRPHY_DXGCR(2U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(3U, 3U));
	r &= ~DDRPHY_DXGCR3_WDLVT;
	Xil_Out32(DDRPHY_DXGCR(3U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(8U, 3U));
	r &= ~DDRPHY_DXGCR3_WDLVT;
	Xil_Out32(DDRPHY_DXGCR(8U, 3U), r);

	r = Xil_In32(DDRPHY_DTCR(0U));
	r |= DDRPHY_DTCR0_INCWEYE;
	Xil_Out32(DDRPHY_DTCR(0U), r);
}

static void ddr_enable_rd_drift(void)
{
	u32 r;

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTDTMODE;
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTUPMODE;
	r |= (1 << DDRPHY_DQSDR0_DFTUPMODE_SHIFT);
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTGPULSE;
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTRDSPC;
	r |= (1 << DDRPHY_DQSDR0_DFTRDSPC_SHIFT);
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTDLY;
	r |= (2 << DDRPHY_DQSDR0_DFTDLY_SHIFT);
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DXGCR(0U, 3U));
	r &= ~DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(0U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(1U, 3U));
	r &= ~DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(1U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(2U, 3U));
	r &= ~DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(2U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(3U, 3U));
	r &= ~DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(3U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(4U, 3U));
	r &= ~DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(4U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(5U, 3U));
	r &= ~DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(5U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(6U, 3U));
	r &= ~DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(6U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(7U, 3U));
	r &= ~DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(7U, 3U), r);

	r = Xil_In32(DDRPHY_DXGCR(8U, 3U));
	r &= ~DDRPHY_DXGCR3_RGLVT;
	Xil_Out32(DDRPHY_DXGCR(8U, 3U), r);

	r = Xil_In32(DDRPHY_DQSDR(1U));
	r &= ~DDRPHY_DQSDR1_DFTRDIDLC;
	Xil_Out32(DDRPHY_DQSDR(1U), r);

	r = Xil_In32(DDRPHY_DQSDR(1U));
	r &= ~DDRPHY_DQSDR1_DFTRDIDLF;
	Xil_Out32(DDRPHY_DQSDR(1U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r |= DDRPHY_DQSDR0_DFTDTEN;
	Xil_Out32(DDRPHY_DQSDR(0U), r);
}

static bool ddrc_opmode_is(u32 m)
{
	u32 r = Xil_In32(DDRC_STAT);
	r &= DDRC_STAT_OPMODE_MASK;
	r >>= DDRC_STAT_OPMODE_SHIFT;

	return r == m;
}

static bool ddrc_opmode_is_sr(void)
{
	return ddrc_opmode_is(DDRC_STAT_OPMODE_SR);
}

static int ddrc_enable_sr(void)
{
	u32 r;
	size_t i;

	/* disable AXI ports */
	for (i = 0U; i < 6U; i++) {
		while (Xil_In32(DDRC_PSTAT) & DDRC_PSTAT_PORT_BUSY(i))
			;
		r = Xil_In32(DDRC_PCTRL(i));
		r &= ~DDRC_PCTRL_PORT_EN;
		Xil_Out32(DDRC_PCTRL(i), r);
	}

	/* enable self refresh */
	r = Xil_In32(DDRC_PWRCTL);
	r |= DDRC_PWRCTL_SR_SW;
	Xil_Out32(DDRC_PWRCTL, r);

	while (true != ddrc_opmode_is_sr())
		;

	while ((Xil_In32(DDRC_STAT) & (3U << 4U)) != (2U << 4U))
		;

	return XST_SUCCESS;
}

static void ddr_clock_set(bool en)
{
	u32 r = Xil_In32(DDRQOS_DDR_CLK_CTRL);
	if (true == en)
		r |= DDRQOS_DDR_CLK_CTRL_CLKACT;
	else
		r &= ~DDRQOS_DDR_CLK_CTRL_CLKACT;
	Xil_Out32(DDRQOS_DDR_CLK_CTRL, r);
}

static void ddr_clock_enable(void)
{
	ddr_clock_set(true);
}

static void ddr_clock_disable(void)
{
	ddr_clock_set(false);
}

static void store_state(PmRegisterContext *context)
{
	while (context->addr) {
		context->value = Xil_In32(context->addr);

		if (context->addr == DDRC_RFSHCTL3) {
			/* disable auto-refresh */
			context->value |= DDRC_RFSHCTL3_AUTORF_DIS;
		} else if (context->addr == DDRC_ZQCTL(0U)) {
			/* disable auto-sq */
			context->value |= DDRC_ZQCTL0_ZQ_DIS;
		} else if (context->addr == DDRC_PWRCTL) {
			/* self-refresh mode */
			context->value = 0x00000020U;
		} else if (context->addr == DDRC_INIT(0U)) {
			/* skip DRAM init and start in self-refresh */
			context->value |= 0xc0000000U;
		} else if (context->addr == DDRC_DFIMISC) {
			context->value &= ~1U;
		} else if (context->addr == DDRPHY_PGCR(0U)) {
			/* assert FIFO reset */
			context->value &= ~DDRPHY_PGCR0_PHYFRST;
		} else if (context->addr == DDRPHY_DX8SLNOSC(0U) ||
			   context->addr == DDRPHY_DX8SLNOSC(1U) ||
			   context->addr == DDRPHY_DX8SLNOSC(2U) ||
			   context->addr == DDRPHY_DX8SLNOSC(3U) ||
			   context->addr == DDRPHY_DX8SLNOSC(4U) ||
			   context->addr == DDRPHY_DX8SLNOSC(5U) ||
			   context->addr == DDRPHY_DX8SLNOSC(6U) ||
			   context->addr == DDRPHY_DX8SLNOSC(7U) ||
			   context->addr == DDRPHY_DX8SLNOSC(8U)) {
			/* assert FIFO reset */
			context->value &= ~DDRPHY_DX8SLBOSC_PHYFRST;
		}
#ifdef DDRSR_DEBUG_STATE
		ddr_print_dbg("%s: addr:%lx, value:%lx\r\n",
			      __func__, context->addr, context->value);
#endif
		context++;
	}
}

static void restore_state(PmRegisterContext *context)
{
	while (context->addr) {
#ifdef DDRSR_DEBUG_STATE
		ddr_print_dbg("%s: addr:0x%lx, value:0x%lx\r\n",
			      __func__, context->addr, context->value);
#endif
		Xil_Out32(context->addr, context->value);
		context++;
	}
}

static void restore_ddrphy_zqdata(PmRegisterContext *context)
{
	while (context->addr) {
#ifdef DDRSR_DEBUG_STATE
		ddr_print_dbg("%s: addr:%lx, value:%lx\r\n",
			      __func__, context->addr + 8U, context->value);
#endif
		/* write result data back to override register */
		Xil_Out32(context->addr + 8U, context->value);
		context++;
	}
}

static void DDR_reinit(bool ddrss_is_reset)
{
	size_t i;
	u32 readVal;

	if (true == ddrss_is_reset) {
		/* PHY init */
		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_ZCALBYP |
				      DDRPHY_PIR_CTLDINIT |
				      DDRPHY_PIR_PHYRST |
				      DDRPHY_PIR_DCAL |
				      DDRPHY_PIR_PLLINIT |
				      DDRPHY_PIR_INIT);
		do {
			readVal = Xil_In32(DDRPHY_PGSR(0U));
		} while (!(readVal & DDRPHY_PGSR0_IDONE));
		while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
			;

		for (i = 0U; i < 4U; i++) {
			readVal = Xil_In32(DDRPHY_ZQPR(i, 0U));
			readVal |= 0xfU << 28U;
			Xil_Out32(DDRPHY_ZQPR(i, 0U), readVal);
		}
		restore_ddrphy_zqdata(ctx_ddrphy_zqdata);

		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
				      DDRPHY_PIR_INIT);
		do {
			readVal = Xil_In32(DDRPHY_PGSR(0U));
		} while (!(readVal & DDRPHY_PGSR0_IDONE));
		while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
			;

		ddr_io_retention_set(false);

		/* remove ZQ override */
		for (i = 0U; i < 4U; i++) {
			readVal = Xil_In32(DDRPHY_ZQPR(i, 0U));
			readVal &= ~(0xfU << 28U);
			Xil_Out32(DDRPHY_ZQPR(i, 0U), readVal);
		}

		/* zcal */
		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
				      DDRPHY_PIR_ZCAL |
				      DDRPHY_PIR_INIT);
		do {
			readVal = Xil_In32(DDRPHY_PGSR(0U));
		} while (!(readVal & DDRPHY_PGSR0_IDONE));
		while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
			;

		/* enable drift */
		readVal = Xil_In32(DDRC_MSTR);
		if (0U != (readVal & DDRC_MSTR_LPDDR3)) {
			/* enable read drift only for LPDDR3 */
			ddr_enable_rd_drift();
		} else if (0U != (readVal & DDRC_MSTR_LPDDR4)) {
			/* enable read and write drift for LPDDR4 */
			ddr_enable_rd_drift();
			ddr_enable_wr_drift();
		}
		/* do not enable drift for DDR3/4, and LPDDR2 is not supported */

		/* FIFO reset */
		readVal = Xil_In32(DDRPHY_PGCR(0U));
		readVal |= DDRPHY_PGCR0_PHYFRST;
		Xil_Out32(DDRPHY_PGCR(0U), readVal);

		for (i = 0U; i < 9U; i++) {
			readVal = Xil_In32(DDRPHY_DX8SLNOSC(i));
			readVal |= DDRPHY_DX8SLBOSC_PHYFRST;
			Xil_Out32(DDRPHY_DX8SLNOSC(i), readVal);
		}

		Xil_Out32(DDRC_DFIMISC, DDRC_DFIMISC_DFI_INIT_COMP_EN);
		Xil_Out32(DDRC_SWCTL, DDRC_SWCTL_SW_DONE);
	}

	Xil_Out32(DDRC_PWRCTL, 0U);
	do {
		readVal = Xil_In32(DDRC_STAT);
		readVal &= 3U << 4U;
	} while (readVal);

	do {
		readVal = Xil_In32(DDRC_STAT);
		readVal &= DDRC_STAT_OPMODE_MASK;
		readVal >>= DDRC_STAT_OPMODE_SHIFT;
	} while (readVal != DDRC_STAT_OPMODE_NORMAL);

	if (true == ddrss_is_reset) {
		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
				      DDRPHY_PIR_WREYE |
				      DDRPHY_PIR_RDEYE |
				      DDRPHY_PIR_WRDSKW |
				      DDRPHY_PIR_RDDSKW |
				      DDRPHY_PIR_WLADJ |
				      DDRPHY_PIR_QSGATE |
				      DDRPHY_PIR_WL |
				      DDRPHY_PIR_INIT);
		do {
			readVal = Xil_In32(DDRPHY_PGSR(0U));
		} while (!(readVal & DDRPHY_PGSR0_IDONE));
		while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
			;

		/* enable static read mode for VREF training */
		for (i = 0U; i < 5U; i++) {
			readVal = Xil_In32(DDRPHY_DX8SLDXCTL2(i));
			readVal |= 3U << 4U;
			Xil_Out32(DDRPHY_DX8SLDXCTL2(i), readVal);
		}

		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
				      DDRPHY_PIR_VREF |
				      DDRPHY_PIR_INIT);
		do {
			readVal = Xil_In32(DDRPHY_PGSR(0U));
		} while (!(readVal & DDRPHY_PGSR0_IDONE));
		while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
			;

		/* disable static read mode */
		for (i = 0U; i < 5U; i++) {
			readVal = Xil_In32(DDRPHY_DX8SLDXCTL2(i));
			readVal &= ~(3U << 4U);
			Xil_Out32(DDRPHY_DX8SLDXCTL2(i), readVal);
		}

		readVal = DDRPHY_PIR_CTLDINIT | DDRPHY_PIR_INIT;
		if (0U != (Xil_In32(DDRPHY_RDIMMGCR(0U)) & DDRPHY_RDIMMGCR0_RDIMM)) {
			readVal |= DDRPHY_PIR_RDIMMINIT;
		}

		Xil_Out32(DDRPHY_PIR, readVal);
		do {
			readVal = Xil_In32(DDRPHY_PGSR(0U));
		} while (!(readVal & DDRPHY_PGSR0_IDONE));
		while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
			;

		readVal = Xil_In32(DDRC_RFSHCTL3);
		readVal &= ~DDRC_RFSHCTL3_AUTORF_DIS;
		Xil_Out32(DDRC_RFSHCTL3, readVal);

		readVal = Xil_In32(DDRC_ZQCTL(0U));
		readVal &= ~DDRC_ZQCTL0_ZQ_DIS;
		Xil_Out32(DDRC_ZQCTL(0U), readVal);
	} else {
		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_WREYE |
				      DDRPHY_PIR_RDEYE |
				      DDRPHY_PIR_WRDSKW |
				      DDRPHY_PIR_RDDSKW);
		do {
			readVal = Xil_In32(DDRPHY_PGSR(0U));
		} while (!(readVal & DDRPHY_PGSR0_IDONE));
		while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
			;

		/* enable AXI ports */
		for (i = 0U; i < 6U; i++) {
			while (Xil_In32(DDRC_PSTAT) & DDRC_PSTAT_PORT_BUSY(i))
				;
			readVal = Xil_In32(DDRC_PCTRL(i));
			readVal |= DDRC_PCTRL_PORT_EN;
			Xil_Out32(DDRC_PCTRL(i), readVal);
		}
	}
}

static int pm_ddr_sr_enter(void)
{
	int ret;

	/* disable read and write drift */
	ddr_disable_rd_drift();
	ddr_disable_wr_drift();

	store_state(ctx_ddrc);
	store_state(ctx_ddrphy);
	store_state(ctx_ddrphy_zqdata);

	ret = ddrc_enable_sr();
	if (XST_SUCCESS != ret) {
		goto err;
	}

	ddr_clock_disable();

err:
	return ret;
}

static int pm_ddr_sr_exit(bool ddrss_is_reset)
{
	ddr_clock_enable();

	if (true == ddrss_is_reset) {
		u32 readVal;

		Xil_Out32(DDRC_SWCTL, 0U);
		restore_state(ctx_ddrc);

		readVal = Xil_In32(CRF_APB_RST_DDR_SS);
		readVal &= ~CRF_APB_RST_DDR_SS_DDR_RESET_MASK;
		Xil_Out32(CRF_APB_RST_DDR_SS, readVal);

		restore_state(ctx_ddrphy);
	}

	DDR_reinit(ddrss_is_reset);

	return XST_SUCCESS;
}

/**
 * PmDdrFsmHandler() - DDR FSM handler, performs transition actions
 * @slave       Slave whose state should be changed (pointer to DDR object)
 * @nextState   State the slave should enter
 *
 * @return      Status of performing transition action
 */
static int PmDdrFsmHandler(PmSlave* const slave, const PmStateId nextState)
{
	int status;

	/* Handle transition to OFF state here */
	if ((PM_DDR_STATE_OFF != slave->node.currState) &&
	    (PM_DDR_STATE_OFF == nextState)) {
		/* TODO : power down DDR here */
		status = XST_SUCCESS;
		goto done;
	}

	switch (slave->node.currState) {
	case PM_DDR_STATE_ON:
		if (PM_DDR_STATE_SR == nextState) {
			status = pm_ddr_sr_enter();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_DDR_STATE_SR:
		if (PM_DDR_STATE_ON == nextState) {
			bool ddrss_is_reset = !Xil_In32(DDRC_STAT);

			status = pm_ddr_sr_exit(ddrss_is_reset);
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_DDR_STATE_OFF:
		if (PM_DDR_STATE_ON == nextState) {
			/* TODO : power up DDR here */
			status = XST_SUCCESS;
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	default:
		status = XST_PM_INTERNAL;
		PmDbg("ERROR: Unknown DDR state #%d\r\n", slave->node.currState);
		break;
	}

done:
	return status;
}

/* DDR FSM */
static const PmSlaveFsm pmSlaveDdrFsm = {
	.states = pmDdrStates,
	.statesCnt = PM_DDR_STATE_MAX,
	.trans = pmDdrTransitions,
	.transCnt = ARRAY_SIZE(pmDdrTransitions),
	.enterState = PmDdrFsmHandler,
};

static u32 PmDdrPowerConsumptions[] = {
	DEFAULT_DDR_POWER_OFF,
	DEFAULT_DDR_POWER_SR,
	DEFAULT_DDR_POWER_ON,
};

PmSlave pmSlaveDdr_g = {
	.node = {
		.derived = &pmSlaveDdr_g,
		.nodeId = NODE_DDR,
		.typeId = PM_TYPE_DDR,
		.parent = &pmPowerDomainFpd_g,
		.clocks = NULL,
		.currState = PM_DDR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmDdrPowerConsumptions,
		.powerInfoCnt = ARRAY_SIZE(PmDdrPowerConsumptions),
	},
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmSlaveDdrFsm,
	.flags = PM_SLAVE_FLAG_IS_SHAREABLE,
};
