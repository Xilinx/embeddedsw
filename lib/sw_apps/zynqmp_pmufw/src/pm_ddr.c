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

#define DDRC_BASE		0xFD070000
#define DDRC_MSTR       	0XFD070000
#define DDRC_STAT		(DDRC_BASE + 4)
#define DDRC_MRCTRL0		(DDRC_BASE + 0X10)
#define DDRC_DERATEEN		(DDRC_BASE + 0X20)
#define DDRC_DERATEINT		(DDRC_BASE + 0X24)
#define DDRC_PWRCTL		(DDRC_BASE + 0x30)
#define DDRC_PWRTMG		(DDRC_BASE + 0X34)
#define DDRC_RFSHCTL0		(DDRC_BASE + 0X50)
#define DDRC_RFSHCTL3		(DDRC_BASE + 0x60)
#define DDRC_RFSHTMG		(DDRC_BASE + 0x64)
#define DDRC_ECCCFG0		(DDRC_BASE + 0X70)
#define DDRC_ECCCFG1		(DDRC_BASE + 0X74)
#define DDRC_CRCPARCTL1		(DDRC_BASE + 0XC4)
#define DDRC_CRCPARCTL2		(DDRC_BASE + 0XC8)
#define DDRC_INIT(n)		(DDRC_BASE + 0xD0 + (4 * n))
#define DDRC_DIMMCTL		(DDRC_BASE + 0XF0)
#define DDRC_RANKCTL		(DDRC_BASE + 0XF4)
#define DDRC_DRAMTMG(n)		(DDRC_BASE + 0x100 + (4 * n))
#define DDRC_ZQCTL(n)		(DDRC_BASE + 0x180 + (4 * n))
#define DDRC_DFITMG0		(DDRC_BASE + 0X190)
#define DDRC_DFITMG1		(DDRC_BASE + 0X194)
#define DDRC_DFILPCFG(n)	(DDRC_BASE + 0X198 + (4 * n))
#define DDRC_DFIUPD1		(DDRC_BASE + 0X1A4)
#define DDRC_DFIMISC		(DDRC_BASE + 0x1b0)
#define DDRC_DFITMG2		(DDRC_BASE + 0X1B4)
#define DDRC_DBICTL		(DDRC_BASE + 0X1C0)
#define DDRC_ADDRMAP(n)		(DDRC_BASE + 0X200 + (4 * n))
#define DDRC_ODTCFG		(DDRC_BASE + 0X240)
#define DDRC_ODTMAP		(DDRC_BASE + 0X244)
#define DDRC_SCHED		(DDRC_BASE + 0X250)
#define DDRC_PERFLPR1		(DDRC_BASE + 0X264)
#define DDRC_PERFWR1		(DDRC_BASE + 0X26C)
#define DDRC_DQMAP5		(DDRC_BASE + 0X294)
#define DDRC_DBG0		(DDRC_BASE + 0X300)
#define DDRC_DBGCMD		(DDRC_BASE + 0X30C)
#define DDRC_SWCTL		(DDRC_BASE + 0x320)
#define DDRC_SWSTAT		(DDRC_BASE + 0x324)
#define DDRC_PSTAT		(DDRC_BASE + 0x3fc)
#define DDRC_PCCFG		(DDRC_BASE + 0X400)
#define DDRC_PCFGR(n)		(DDRC_BASE + 0X404 + (0xb0 * n))
#define DDRC_PCFGW(n)		(DDRC_BASE + 0X408 + (0xb0 * n))
#define DDRC_PCTRL(n)		(DDRC_BASE + 0x490 + (0xb0 * n))
#define DDRC_PCFGQOS(n, m)	(DDRC_BASE + 0X494 + (4 * n) + (0xb0 * m))
#define DDRC_PCFGWQOS(n, m)	(DDRC_BASE + 0x49c + (4 * n) + (0xb0 * m))
#define DDRC_SARBASE(n)		(DDRC_BASE + 0XF04 + (8 * n))
#define DDRC_SARSIZE(n)		(DDRC_BASE + 0XF08 + (8 * n))
#define DDRC_DFITMG0_SHADOW	(DDRC_BASE + 0X2190)

#define DDRC_PWRCTL_SR_SW	BIT(5)
#define DDRC_PWRCTL_STAY_IN_SR	BIT(6)

#define DDRC_STAT_OPMODE_MASK	7
#define DDRC_STAT_OPMODE_SHIFT	0
#define DDRC_STAT_OPMODE_INIT	0
#define DDRC_STAT_OPMODE_NORMAL	1
#define DDRC_STAT_OPMODE_SR	3

#define DDRC_SWSTAT_SWDONE	BIT(0)

#define DDRC_PSTAT_PORT_BUSY(n)	((BIT(0) | BIT(16)) << n)

#define DDRC_PCTRL_PORT_EN	BIT(0)

#define DDRC_ZQCTL0_ZQ_DIS	BIT(31)

#define DDRC_RFSHCTL3_AUTORF_DIS	BIT(0)

#define DDRC_DFIMISC_DFI_INIT_COMP_EN	BIT(0)

#define DDRC_SWCTL_SW_DONE	BIT(0)

#define DDRPHY_BASE		0xFD080000
#define DDRPHY_PIR		(DDRPHY_BASE + 4)
#define DDRPHY_PGCR(n)		(DDRPHY_BASE + 0x10 + (4 * n))
#define DDRPHY_PGSR(n)		(DDRPHY_BASE + 0x30 + (4 * n))
#define DDRPHY_PTR(n)		(DDRPHY_BASE + 0X40 + (4 * n))
#define DDRPHY_DSGCR		(DDRPHY_BASE + 0X90)
#define DDRPHY_DCR		(DDRPHY_BASE + 0X100)
#define DDRPHY_DTPR(n)		(DDRPHY_BASE + 0X110 + (4 * n))
#define DDRPHY_RDIMMGCR(n)	(DDRPHY_BASE + 0x140 + (4 * n))
#define DDRPHY_RDIMMCR(n)	(DDRPHY_BASE + 0x150 + (4 * n))
#define DDRPHY_MR(n)		(DDRPHY_BASE + 0X180 + (4 * n))
#define DDRPHY_DTCR(n)		(DDRPHY_BASE + 0X200 + (4 * n))
#define DDRPHY_CATR(n)		(DDRPHY_BASE + 0X240 + (4 * n))
#define DDRPHY_RIOCR(n)		(DDRPHY_BASE + 0X4E0 + (4 * n))
#define DDRPHY_ACIOCR(n)	(DDRPHY_BASE + 0X500 + (4 * n))
#define DDRPHY_IOVCR(n)		(DDRPHY_BASE + 0X520 + (4 * n))
#define DDRPHY_VTCR(n)		(DDRPHY_BASE + 0X528 + (4 * n))
#define DDRPHY_DQSDR0		(DDRPHY_BASE + 0x250)
#define DDRPHY_ACBDLR(n)	(DDRPHY_BASE + 0x540 + (4 * n))
#define DDRPHY_ZQCR		(DDRPHY_BASE + 0x680)
#define DDRPHY_ZQPR(n, m)	(DDRPHY_BASE + 0x684 + (0x20 * n) + (4 * m))
#define DDRPHY_ZQDR0(n)		(DDRPHY_BASE + 0x68c + (0x20 * n))
#define DDRPHY_ZQDR1(n)		(DDRPHY_BASE + 0x690 + (0x20 * n))
#define DDRPHY_DXGCR(n, m)	(DDRPHY_BASE + 0X700 + (0x100 * n) + (4 * m))
#define DDRPHY_DXGTR0(n)	(DDRPHY_BASE + 0X7c0 + (0x100 * n))
#define DDRPHY_DX8SLNOSC(n)	(DDRPHY_BASE + 0x1400 + (0x40 * n))
#define DDRPHY_DX8SLDQSCTL(n)	(DDRPHY_BASE + 0x141c + (0x40 * n))
#define DDRPHY_DX8SLDXCTL2(n)	(DDRPHY_BASE + 0x142c + (0x40 * n))
#define DDRPHY_DX8SLIOCR(n)	(DDRPHY_BASE + 0x1430 + (0x40 * n))
#define DDRPHY_DX8SLBOSC	(DDRPHY_BASE + 0x17c0)

#define DDRPHY_PIR_INIT			BIT(0)
#define DDRPHY_PIR_ZCAL			BIT(1)
#define DDRPHY_PIR_PLLINIT		BIT(4)
#define DDRPHY_PIR_DCAL			BIT(5)
#define DDRPHY_PIR_PHYRST		BIT(6)
#define DDRPHY_PIR_WL			BIT(9)
#define DDRPHY_PIR_QSGATE		BIT(10)
#define DDRPHY_PIR_WLADJ		BIT(11)
#define DDRPHY_PIR_RDDSKW		BIT(12)
#define DDRPHY_PIR_WRDSKW		BIT(13)
#define DDRPHY_PIR_RDEYE		BIT(14)
#define DDRPHY_PIR_WREYE		BIT(15)
#define DDRPHY_PIR_VREF			BIT(17)
#define DDRPHY_PIR_CTLDINIT		BIT(18)
#define DDRPHY_PIR_RDIMMINIT		BIT(19)
#define DDRPHY_PIR_ZCALBYP		BIT(30)

#define DDRPHY_PGCR0_PHYFRST	BIT(26)

#define DDRPHY_PGSR0_SRDERR	BIT(30)
#define DDRPHY_PGSR0_CAWRN	BIT(29)
#define DDRPHY_PGSR0_CAERR	BIT(28)
#define DDRPHY_PGSR0_WEERR	BIT(27)
#define DDRPHY_PGSR0_REERR	BIT(26)
#define DDRPHY_PGSR0_WDERR	BIT(25)
#define DDRPHY_PGSR0_RDERR	BIT(24)
#define DDRPHY_PGSR0_WLAERR	BIT(23)
#define DDRPHY_PGSR0_QSGERR	BIT(22)
#define DDRPHY_PGSR0_WLERR	BIT(21)
#define DDRPHY_PGSR0_ZCERR	BIT(20)
#define DDRPHY_PGSR0_VERR	BIT(19)
#define DDRPHY_PGSR0_DQS2DQERR	BIT(18)
#define DDRPHY_PGSR0_IDONE	BIT(0)
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

#define DDRPHY_DQSDR0_DFTDTEN	BIT(0)

#define DDRPHY_DX8SLBOSC_PHYFRST	BIT(15)

#define DDRQOS_BASE		0xFD090000
#define DDRQOS_DDR_CLK_CTRL	(DDRQOS_BASE + 0x700)

#define DDRQOS_DDR_CLK_CTRL_CLKACT	BIT(0)

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
	[PM_DDR_STATE_ON] = PM_CAP_ACCESS | PM_CAP_CONTEXT | PM_CAP_POWER,
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
	{ .addr = DDRC_INIT(0), },
	{ .addr = DDRC_INIT(1), },
	{ .addr = DDRC_INIT(2), },
	{ .addr = DDRC_INIT(3), },
	{ .addr = DDRC_INIT(4), },
	{ .addr = DDRC_INIT(5), },
	{ .addr = DDRC_INIT(6), },
	{ .addr = DDRC_INIT(7), },
	{ .addr = DDRC_DIMMCTL, },
	{ .addr = DDRC_RANKCTL, },
	{ .addr = DDRC_DRAMTMG(0), },
	{ .addr = DDRC_DRAMTMG(1), },
	{ .addr = DDRC_DRAMTMG(2), },
	{ .addr = DDRC_DRAMTMG(3), },
	{ .addr = DDRC_DRAMTMG(4), },
	{ .addr = DDRC_DRAMTMG(5), },
	{ .addr = DDRC_DRAMTMG(6), },
	{ .addr = DDRC_DRAMTMG(7), },
	{ .addr = DDRC_DRAMTMG(8), },
	{ .addr = DDRC_DRAMTMG(9), },
	{ .addr = DDRC_DRAMTMG(11), },
	{ .addr = DDRC_DRAMTMG(12), },
	{ .addr = DDRC_ZQCTL(0), },
	{ .addr = DDRC_ZQCTL(1), },
	{ .addr = DDRC_DFITMG0, },
	{ .addr = DDRC_DFITMG1, },
	{ .addr = DDRC_DFILPCFG(0), },
	{ .addr = DDRC_DFILPCFG(1), },
	{ .addr = DDRC_DFIUPD1, },
	{ .addr = DDRC_DFIMISC, },
	{ .addr = DDRC_DFITMG2, },
	{ .addr = DDRC_DBICTL, },
	{ .addr = DDRC_ADDRMAP(0), },
	{ .addr = DDRC_ADDRMAP(1), },
	{ .addr = DDRC_ADDRMAP(2), },
	{ .addr = DDRC_ADDRMAP(3), },
	{ .addr = DDRC_ADDRMAP(4), },
	{ .addr = DDRC_ADDRMAP(5), },
	{ .addr = DDRC_ADDRMAP(6), },
	{ .addr = DDRC_ADDRMAP(7), },
	{ .addr = DDRC_ADDRMAP(8), },
	{ .addr = DDRC_ADDRMAP(9), },
	{ .addr = DDRC_ADDRMAP(10), },
	{ .addr = DDRC_ADDRMAP(11), },
	{ .addr = DDRC_ODTCFG, },
	{ .addr = DDRC_ODTMAP, },
	{ .addr = DDRC_SCHED, },
	{ .addr = DDRC_PERFLPR1, },
	{ .addr = DDRC_PERFWR1, },
	{ .addr = DDRC_DQMAP5, },
	{ .addr = DDRC_DBG0, },
	{ .addr = DDRC_DBGCMD, },
	{ .addr = DDRC_PCCFG, },
	{ .addr = DDRC_PCFGR(0), },
	{ .addr = DDRC_PCFGW(0), },
	{ .addr = DDRC_PCTRL(0), },
	{ .addr = DDRC_PCFGQOS(0, 0), },
	{ .addr = DDRC_PCFGQOS(1, 0), },
	{ .addr = DDRC_PCFGR(1), },
	{ .addr = DDRC_PCFGW(1), },
	{ .addr = DDRC_PCTRL(1), },
	{ .addr = DDRC_PCFGQOS(0, 1), },
	{ .addr = DDRC_PCFGQOS(1, 1), },
	{ .addr = DDRC_PCFGR(2), },
	{ .addr = DDRC_PCFGW(2), },
	{ .addr = DDRC_PCTRL(2), },
	{ .addr = DDRC_PCFGQOS(0, 2), },
	{ .addr = DDRC_PCFGQOS(1, 2), },
	{ .addr = DDRC_PCFGR(3), },
	{ .addr = DDRC_PCFGW(3), },
	{ .addr = DDRC_PCTRL(3), },
	{ .addr = DDRC_PCFGQOS(0, 3), },
	{ .addr = DDRC_PCFGQOS(1, 3), },
	{ .addr = DDRC_PCFGWQOS(0, 3), },
	{ .addr = DDRC_PCFGWQOS(1, 3), },
	{ .addr = DDRC_PCFGR(4), },
	{ .addr = DDRC_PCFGW(4), },
	{ .addr = DDRC_PCTRL(4), },
	{ .addr = DDRC_PCFGQOS(0, 4), },
	{ .addr = DDRC_PCFGQOS(1, 4), },
	{ .addr = DDRC_PCFGWQOS(0, 4), },
	{ .addr = DDRC_PCFGWQOS(1, 4), },
	{ .addr = DDRC_PCFGR(5), },
	{ .addr = DDRC_PCFGW(5), },
	{ .addr = DDRC_PCTRL(5), },
	{ .addr = DDRC_PCFGQOS(0, 5), },
	{ .addr = DDRC_PCFGQOS(1, 5), },
	{ .addr = DDRC_PCFGWQOS(0, 5), },
	{ .addr = DDRC_PCFGWQOS(1, 5), },
	{ .addr = DDRC_SARBASE(0), },
	{ .addr = DDRC_SARSIZE(0), },
	{ .addr = DDRC_SARBASE(1), },
	{ .addr = DDRC_SARSIZE(1), },
	{ },
};

static PmRegisterContext ctx_ddrphy[] = {
	{ .addr = DDRPHY_PGCR(0), },
	{ .addr = DDRPHY_PGCR(2), },
	{ .addr = DDRPHY_PGCR(3), },
	{ .addr = DDRPHY_PGCR(5), },
	{ .addr = DDRPHY_PTR(0), },
	{ .addr = DDRPHY_PTR(1), },
	{ .addr = DDRPHY_DSGCR, },
	{ .addr = DDRPHY_DCR, },
	{ .addr = DDRPHY_DTPR(0), },
	{ .addr = DDRPHY_DTPR(1), },
	{ .addr = DDRPHY_DTPR(2), },
	{ .addr = DDRPHY_DTPR(3), },
	{ .addr = DDRPHY_DTPR(4), },
	{ .addr = DDRPHY_DTPR(5), },
	{ .addr = DDRPHY_DTPR(6), },
	{ .addr = DDRPHY_RDIMMGCR(0), },
	{ .addr = DDRPHY_RDIMMGCR(1), },
	{ .addr = DDRPHY_RDIMMCR(0), },
	{ .addr = DDRPHY_RDIMMCR(1), },
	{ .addr = DDRPHY_MR(0), },
	{ .addr = DDRPHY_MR(1), },
	{ .addr = DDRPHY_MR(2), },
	{ .addr = DDRPHY_MR(3), },
	{ .addr = DDRPHY_MR(4), },
	{ .addr = DDRPHY_MR(5), },
	{ .addr = DDRPHY_MR(6), },
	{ .addr = DDRPHY_MR(11), },
	{ .addr = DDRPHY_MR(12), },
	{ .addr = DDRPHY_MR(13), },
	{ .addr = DDRPHY_MR(14), },
	{ .addr = DDRPHY_MR(22), },
	{ .addr = DDRPHY_DTCR(0), },
	{ .addr = DDRPHY_DTCR(1), },
	{ .addr = DDRPHY_CATR(0), },
	{ .addr = DDRPHY_RIOCR(5), },
	{ .addr = DDRPHY_ACIOCR(0), },
	{ .addr = DDRPHY_ACIOCR(2), },
	{ .addr = DDRPHY_ACIOCR(3), },
	{ .addr = DDRPHY_ACIOCR(4), },
	{ .addr = DDRPHY_IOVCR(0), },
	{ .addr = DDRPHY_VTCR(0), },
	{ .addr = DDRPHY_VTCR(1), },
	{ .addr = DDRPHY_ACBDLR(1), },
	{ .addr = DDRPHY_ACBDLR(2), },
	{ .addr = DDRPHY_ACBDLR(6), },
	{ .addr = DDRPHY_ACBDLR(7), },
	{ .addr = DDRPHY_ACBDLR(8), },
	{ .addr = DDRPHY_ACBDLR(9), },
	{ .addr = DDRPHY_ZQCR, },
	{ .addr = DDRPHY_ZQPR(0, 0), },
	{ .addr = DDRPHY_ZQPR(1, 0), },
	{ .addr = DDRPHY_DXGCR(0, 0), },
	{ .addr = DDRPHY_DXGCR(0, 4), },
	{ .addr = DDRPHY_DXGCR(0, 5), },
	{ .addr = DDRPHY_DXGCR(0, 6), },
	{ .addr = DDRPHY_DXGTR0(0), },
	{ .addr = DDRPHY_DXGCR(1, 0), },
	{ .addr = DDRPHY_DXGCR(1, 4), },
	{ .addr = DDRPHY_DXGCR(1, 5), },
	{ .addr = DDRPHY_DXGCR(1, 6), },
	{ .addr = DDRPHY_DXGTR0(1), },
	{ .addr = DDRPHY_DXGCR(2, 0), },
	{ .addr = DDRPHY_DXGCR(2, 1), },
	{ .addr = DDRPHY_DXGCR(2, 4), },
	{ .addr = DDRPHY_DXGCR(2, 5), },
	{ .addr = DDRPHY_DXGCR(2, 6), },
	{ .addr = DDRPHY_DXGTR0(2), },
	{ .addr = DDRPHY_DXGCR(3, 0), },
	{ .addr = DDRPHY_DXGCR(3, 1), },
	{ .addr = DDRPHY_DXGCR(3, 4), },
	{ .addr = DDRPHY_DXGCR(3, 5), },
	{ .addr = DDRPHY_DXGCR(3, 6), },
	{ .addr = DDRPHY_DXGTR0(3), },
	{ .addr = DDRPHY_DXGCR(4, 0), },
	{ .addr = DDRPHY_DXGCR(4, 1), },
	{ .addr = DDRPHY_DXGCR(4, 4), },
	{ .addr = DDRPHY_DXGCR(4, 5), },
	{ .addr = DDRPHY_DXGCR(4, 6), },
	{ .addr = DDRPHY_DXGTR0(4), },
	{ .addr = DDRPHY_DXGCR(5, 0), },
	{ .addr = DDRPHY_DXGCR(5, 1), },
	{ .addr = DDRPHY_DXGCR(5, 4), },
	{ .addr = DDRPHY_DXGCR(5, 5), },
	{ .addr = DDRPHY_DXGCR(5, 6), },
	{ .addr = DDRPHY_DXGTR0(5), },
	{ .addr = DDRPHY_DXGCR(6, 0), },
	{ .addr = DDRPHY_DXGCR(6, 1), },
	{ .addr = DDRPHY_DXGCR(6, 4), },
	{ .addr = DDRPHY_DXGCR(6, 5), },
	{ .addr = DDRPHY_DXGCR(6, 6), },
	{ .addr = DDRPHY_DXGTR0(6), },
	{ .addr = DDRPHY_DXGCR(7, 0), },
	{ .addr = DDRPHY_DXGCR(7, 1), },
	{ .addr = DDRPHY_DXGCR(7, 4), },
	{ .addr = DDRPHY_DXGCR(7, 5), },
	{ .addr = DDRPHY_DXGCR(7, 6), },
	{ .addr = DDRPHY_DXGTR0(7), },
	{ .addr = DDRPHY_DXGCR(8, 0), },
	{ .addr = DDRPHY_DXGCR(8, 1), },
	{ .addr = DDRPHY_DXGCR(8, 4), },
	{ .addr = DDRPHY_DXGCR(8, 5), },
	{ .addr = DDRPHY_DXGCR(8, 6), },
	{ .addr = DDRPHY_DXGTR0(8), },
	{ .addr = DDRPHY_DX8SLNOSC(0), },
	{ .addr = DDRPHY_DX8SLDQSCTL(0), },
	{ .addr = DDRPHY_DX8SLDXCTL2(0), },
	{ .addr = DDRPHY_DX8SLIOCR(0), },
	{ .addr = DDRPHY_DX8SLNOSC(1), },
	{ .addr = DDRPHY_DX8SLDQSCTL(1), },
	{ .addr = DDRPHY_DX8SLDXCTL2(1), },
	{ .addr = DDRPHY_DX8SLIOCR(1), },
	{ .addr = DDRPHY_DX8SLNOSC(2), },
	{ .addr = DDRPHY_DX8SLDQSCTL(2), },
	{ .addr = DDRPHY_DX8SLDXCTL2(2), },
	{ .addr = DDRPHY_DX8SLIOCR(2), },
	{ .addr = DDRPHY_DX8SLNOSC(3), },
	{ .addr = DDRPHY_DX8SLDQSCTL(3), },
	{ .addr = DDRPHY_DX8SLDXCTL2(3), },
	{ .addr = DDRPHY_DX8SLIOCR(3), },
	{ .addr = DDRPHY_DX8SLNOSC(4), },
	{ .addr = DDRPHY_DX8SLDQSCTL(4), },
	{ .addr = DDRPHY_DX8SLDXCTL2(4), },
	{ .addr = DDRPHY_DX8SLIOCR(4), },
	{ },
};

static PmRegisterContext ctx_ddrphy_zqdata[] = {
	{ .addr = DDRPHY_ZQDR0(0), },
	{ .addr = DDRPHY_ZQDR1(0), },
	{ .addr = DDRPHY_ZQDR0(1), },
	{ .addr = DDRPHY_ZQDR1(1), },
	{ .addr = DDRPHY_ZQDR0(2), },
	{ .addr = DDRPHY_ZQDR1(2), },
	{ .addr = DDRPHY_ZQDR0(3), },
	{ .addr = DDRPHY_ZQDR1(3), },
	{ },
};

static int ddrc_opmode_is(u32 m)
{
	u32 r = Xil_In32(DDRC_STAT);
	r &= DDRC_STAT_OPMODE_MASK;
	r >>= DDRC_STAT_OPMODE_SHIFT;

	return r == m;
}

static int ddrc_opmode_is_sr(void)
{
	return ddrc_opmode_is(DDRC_STAT_OPMODE_SR);
}

static int ddrc_enable_sr(void)
{
	u32 r;
	size_t i;

	/* disable AXI ports */
	for (i = 0; i < 6; i++) {
		while (Xil_In32(DDRC_PSTAT) & DDRC_PSTAT_PORT_BUSY(i))
			;
		r = Xil_In32(DDRC_PCTRL(i));
		r &= ~DDRC_PCTRL_PORT_EN;
		Xil_Out32(DDRC_PCTRL(i), r);
	}

	/* disable drift detection */
	r = Xil_In32(DDRPHY_DQSDR0);
	r &= ~DDRPHY_DQSDR0_DFTDTEN;
	Xil_Out32(DDRPHY_DQSDR0, r);

	/* enable self refresh */
	r = Xil_In32(DDRC_PWRCTL);
	r |= DDRC_PWRCTL_STAY_IN_SR | DDRC_PWRCTL_SR_SW;
	Xil_Out32(DDRC_PWRCTL, r);

	while (!ddrc_opmode_is_sr())
		;

	while ((Xil_In32(DDRC_STAT) & (3 << 4)) != (2 << 4))
		;

	return 0;
}

static void io_retention_set(int en)
{
	u32 r = Xil_In32(PMU_GLOBAL_DDR_CNTRL);
	if (en)
		r |= PMU_GLOBAL_DDR_CNTRL_RET_MASK;
	else
		r &= ~PMU_GLOBAL_DDR_CNTRL_RET_MASK;
	Xil_Out32(PMU_GLOBAL_DDR_CNTRL, r);
}

static void io_retention_enable(void)
{
	io_retention_set(1);
}

static void io_retention_disable(void)
{
	io_retention_set(0);
}

static void ddr_clock_set(int en)
{
	u32 r = Xil_In32(DDRQOS_DDR_CLK_CTRL);
	if (en)
		r |= DDRQOS_DDR_CLK_CTRL_CLKACT;
	else
		r &= ~DDRQOS_DDR_CLK_CTRL_CLKACT;
	Xil_Out32(DDRQOS_DDR_CLK_CTRL, r);
}

static void ddr_clock_enable(void)
{
	ddr_clock_set(1);
}

static void ddr_clock_disable(void)
{
	ddr_clock_set(0);
}

static void store_state(PmRegisterContext *context)
{
	while (context->addr) {
		context->value = Xil_In32(context->addr);

		if (context->addr == DDRC_RFSHCTL3) {
			/* disable auto-refresh */
			context->value |= DDRC_RFSHCTL3_AUTORF_DIS;
		} else if (context->addr == DDRC_ZQCTL(0)) {
			/* disable auto-sq */
			context->value |= DDRC_ZQCTL0_ZQ_DIS;
		} else if (context->addr == DDRC_PWRCTL) {
			/* self-refresh mode */
			context->value = 0x00000020U;
		} else if (context->addr == DDRC_INIT(0)) {
			/* skip DRAM init and start in self-refresh */
			context->value |= 0xc0000000;
		} else if (context->addr == DDRC_DFIMISC) {
			context->value &= ~1;
		} else if (context->addr == DDRPHY_PGCR(0)) {
			/* assert FIFO reset */
			context->value &= ~DDRPHY_PGCR0_PHYFRST;
		} else if (context->addr == DDRPHY_DX8SLNOSC(0) ||
			   context->addr == DDRPHY_DX8SLNOSC(1) ||
			   context->addr == DDRPHY_DX8SLNOSC(2) ||
			   context->addr == DDRPHY_DX8SLNOSC(3) ||
			   context->addr == DDRPHY_DX8SLNOSC(4) ||
			   context->addr == DDRPHY_DX8SLNOSC(5) ||
			   context->addr == DDRPHY_DX8SLNOSC(6) ||
			   context->addr == DDRPHY_DX8SLNOSC(7) ||
			   context->addr == DDRPHY_DX8SLNOSC(8)) {
			/* assert FIFO reset */
			context->value &= ~DDRPHY_DX8SLBOSC_PHYFRST;
		}
#ifdef DDRSR_DEBUG_STATE
		ddr_print_dbg("%s: addr:%lx, value:%lx\n",
			      __func__, context->addr, context->value);
#endif
		context++;
	}
}

static void restore_state(PmRegisterContext *context)
{
	while (context->addr) {
#ifdef DDRSR_DEBUG_STATE
		ddr_print_dbg("%s: addr:0x%lx, value:0x%lx\n",
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
		ddr_print_dbg("%s: addr:%lx, value:%lx\n",
			      __func__, context->addr + 8, context->value);
#endif
		/* write result data back to override register */
		Xil_Out32(context->addr + 8, context->value);
		context++;
	}
}

void DDR_reinit(void)
{
	size_t i;
	unsigned int readVal;

	/* PHY init */
	Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_ZCALBYP |
			      DDRPHY_PIR_CTLDINIT |
			      DDRPHY_PIR_PHYRST |
			      DDRPHY_PIR_DCAL |
			      DDRPHY_PIR_PLLINIT |
			      DDRPHY_PIR_INIT);
	do {
		readVal = Xil_In32(DDRPHY_PGSR(0));
	} while (!(readVal & DDRPHY_PGSR0_IDONE));
	while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
		;

	for (i = 0; i < 4; i++) {
		readVal = Xil_In32(DDRPHY_ZQPR(i, 0));
		readVal |= 0xf << 28;
		Xil_Out32(DDRPHY_ZQPR(i, 0), readVal);
	}
	restore_ddrphy_zqdata(ctx_ddrphy_zqdata);

	Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
			      DDRPHY_PIR_INIT);
	do {
		readVal = Xil_In32(DDRPHY_PGSR(0));
	} while (!(readVal & DDRPHY_PGSR0_IDONE));
	while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
		;

	io_retention_disable();

	/* remove ZQ override */
	for (i = 0; i < 4; i++) {
		readVal = Xil_In32(DDRPHY_ZQPR(i, 0));
		readVal &= ~(0xf << 28);
		Xil_Out32(DDRPHY_ZQPR(i, 0), readVal);
	}

	/* zcal */
	Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
			      DDRPHY_PIR_ZCAL |
			      DDRPHY_PIR_INIT);
	do {
		readVal = Xil_In32(DDRPHY_PGSR(0));
	} while (!(readVal & DDRPHY_PGSR0_IDONE));
	while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
		;

	/* FIFO reset */
	readVal = Xil_In32(DDRPHY_PGCR(0));
	readVal |= DDRPHY_PGCR0_PHYFRST;
	Xil_Out32(DDRPHY_PGCR(0), readVal);

	for (i = 0; i < 9; i++) {
		readVal = Xil_In32(DDRPHY_DX8SLNOSC(i));
		readVal |= DDRPHY_DX8SLBOSC_PHYFRST;
		Xil_Out32(DDRPHY_DX8SLNOSC(i), readVal);
	}

	Xil_Out32(DDRC_DFIMISC, DDRC_DFIMISC_DFI_INIT_COMP_EN);
	Xil_Out32(DDRC_SWCTL, DDRC_SWCTL_SW_DONE);

	Xil_Out32(DDRC_PWRCTL, 0);
	do {
		readVal = Xil_In32(DDRC_STAT);
		readVal &= 3 << 4;
	} while (readVal);

	do {
		readVal = Xil_In32(DDRC_STAT);
		readVal &= DDRC_STAT_OPMODE_MASK;
		readVal >>= DDRC_STAT_OPMODE_SHIFT;
	} while (readVal != DDRC_STAT_OPMODE_NORMAL);

	/* training */
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
		readVal = Xil_In32(DDRPHY_PGSR(0));
	} while (!(readVal & DDRPHY_PGSR0_IDONE));
	while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
		;

	/* enable static read mode for VREF training */
	for (i = 0; i < 5; i++) {
		readVal = Xil_In32(DDRPHY_DX8SLDXCTL2(i));
		readVal |= 3 << 4;
		Xil_Out32(DDRPHY_DX8SLDXCTL2(i), readVal);
	}

	Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
			      DDRPHY_PIR_VREF |
			      DDRPHY_PIR_INIT);
	do {
		readVal = Xil_In32(DDRPHY_PGSR(0));
	} while (!(readVal & DDRPHY_PGSR0_IDONE));
	while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
		;

	/* disable static read mode */
	for (i = 0; i < 5; i++) {
		readVal = Xil_In32(DDRPHY_DX8SLDXCTL2(i));
		readVal &= ~(3 << 4);
		Xil_Out32(DDRPHY_DX8SLDXCTL2(i), readVal);
	}

	Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
			      DDRPHY_PIR_RDIMMINIT |
			      DDRPHY_PIR_INIT);
	do {
		readVal = Xil_In32(DDRPHY_PGSR(0));
	} while (!(readVal & DDRPHY_PGSR0_IDONE));
	while (readVal & DDRPHY_PGSR0_TRAIN_ERRS)
		;

	readVal = Xil_In32(DDRC_RFSHCTL3);
	readVal &= ~DDRC_RFSHCTL3_AUTORF_DIS;
	Xil_Out32(DDRC_RFSHCTL3, readVal);

	readVal = Xil_In32(DDRC_ZQCTL(0));
	readVal &= ~DDRC_ZQCTL0_ZQ_DIS;
	Xil_Out32(DDRC_ZQCTL(0), readVal);
}

static int pm_ddr_sr_enter(void)
{
	int ret;

	store_state(ctx_ddrc);
	store_state(ctx_ddrphy);
	store_state(ctx_ddrphy_zqdata);

	ret = ddrc_enable_sr();
	if (ret) {
		goto err;
	}

	io_retention_enable();

	ddr_clock_disable();

err:
	return ret;
}

static int pm_ddr_sr_exit(void)
{
	unsigned int readVal;

	ddr_clock_enable();

	Xil_Out32(DDRC_SWCTL, 0);
	restore_state(ctx_ddrc);

	readVal = Xil_In32(CRF_APB_RST_DDR_SS);
	readVal &= ~CRF_APB_RST_DDR_SS_DDR_RESET_MASK;
	Xil_Out32(CRF_APB_RST_DDR_SS, readVal);

	restore_state(ctx_ddrphy);

	DDR_reinit();

	return 0;
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
	int status = XST_PM_INTERNAL;

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
			status = pm_ddr_sr_exit();
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
		PmDbg("ERROR: Unknown DDR state #%d\n", slave->node.currState);
		break;
	}

done:
	if (XST_SUCCESS == status) {
		PmNodeUpdateCurrState(&slave->node, nextState);
	}
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

static PmRequirement* const pmDdrReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_DDR],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_DDR],
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
		.currState = PM_DDR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmDdrPowerConsumptions,
		.powerInfoCnt = ARRAY_SIZE(PmDdrPowerConsumptions),
	},
	.reqs = pmDdrReqs,
	.reqsCnt = ARRAY_SIZE(pmDdrReqs),
	.wake = NULL,
	.slvFsm = &pmSlaveDdrFsm,
};
