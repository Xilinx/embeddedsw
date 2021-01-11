/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * DDR slave definition
 *
 * Note: In ON state DDR depends on FPD (cannot be accessed if FPD is
 * not on). Therefore, power parent of DDR is FPD.
 *********************************************************************/

#include "crf_apb.h"
#include "pm_ddr.h"
#include "pm_csudma.h"
#include "pm_common.h"
#include "pm_defs.h"
#include "pm_master.h"
#include "xpfw_util.h"
#include "xpfw_aib.h"
#include "pm_system.h"
#include "pm_node.h"
#include "pm_clock.h"
#include "pm_hooks.h"
#include "xpfw_default.h"

#define DDRC_BASE		0xFD070000U
#define DDRC_MSTR		(DDRC_BASE + 0U)
#define DDRC_STAT		(DDRC_BASE + 4U)
#define DDRC_MRCTRL0		(DDRC_BASE + 0X10U)
#define DDRC_DERATEEN		(DDRC_BASE + 0X20U)
#define DDRC_DERATEINT		(DDRC_BASE + 0X24U)
#define DDRC_PWRCTL		(DDRC_BASE + 0x30U)
#define DDRC_PWRTMG		(DDRC_BASE + 0X34U)
#define DDRC_RFSHCTL0		(DDRC_BASE + 0X50U)
#define DDRC_RFSHCTL1		(DDRC_BASE + 0X54U)
#define DDRC_RFSHCTL3		(DDRC_BASE + 0x60U)
#define DDRC_RFSHTMG		(DDRC_BASE + 0x64U)
#define DDRC_ECCCFG0		(DDRC_BASE + 0X70U)
#define DDRC_ECCCFG1		(DDRC_BASE + 0X74U)
#define DDRC_ERRCLR		(DDRC_BASE + 0x7CU)
#define DDRC_ERRCNT		(DDRC_BASE + 0x80U)
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
#define DDRC_DFIUPD0		(DDRC_BASE + 0X1A0U)
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

#define DDRC_MSTR_DDR3		BIT(0U)
#define DDRC_MSTR_LPDDR3	BIT(3U)
#define DDRC_MSTR_DDR4		BIT(4U)
#define DDRC_MSTR_LPDDR4	BIT(5U)
#define DDRC_MSTR_DDR_TYPE	(DDRC_MSTR_DDR3 | \
				 DDRC_MSTR_LPDDR3 | \
				 DDRC_MSTR_DDR4 | \
				 DDRC_MSTR_LPDDR4)

#define DDRC_MSTR_BUS_WIDTH_MASK	3U
#define DDRC_MSTR_BUS_WIDTH_SHIFT	12U
#define DDRC_MSTR_BUS_WIDTH_FULL_DQ	((u32)0 << DDRC_MSTR_BUS_WIDTH_SHIFT)
#define DDRC_MSTR_BUS_WIDTH_HALF_DQ	((u32)1 << DDRC_MSTR_BUS_WIDTH_SHIFT)
#define DDRC_MSTR_BUS_WIDTH_QUART_DQ	((u32)2 << DDRC_MSTR_BUS_WIDTH_SHIFT)

#define DDRC_STAT_OPMODE_MASK	7U
#define DDRC_STAT_OPMODE_SHIFT	0U
#define DDRC_STAT_OPMODE_INIT	0U
#define DDRC_STAT_OPMODE_NORMAL	1U
#define DDRC_STAT_OPMODE_SR	3U

#define DDRC_ADDRMAP0_ADDRMAP_CS_BIT0		((u32)0x0000001FU)
#define DDRC_ADDRMAP2_ADDRMAP_COL_B4		((u32)0x000F0000U)
#define DDRC_ADDRMAP2_ADDRMAP_COL_B4_SHIFT	16U
#define DDRC_ADDRMAP8_ADDRMAP_BG_B0		((u32)0x0000001FU)

#define DDRC_SWSTAT_SWDONE	BIT(0U)

#define DDRC_PSTAT_PORT_BUSY(n)	((BIT(0U) | BIT(16U)) << (n))

#define DDRC_PCTRL_PORT_EN	BIT(0U)

#define DDRC_ZQCTL0_ZQ_DIS	BIT(31U)

#define DDRC_RFSHCTL3_AUTORF_DIS	BIT(0U)

#define DDRC_DFIMISC_DFI_INIT_COMP_EN	BIT(0U)

#define DDRC_SWCTL_SW_DONE	BIT(0U)

#define DDRC_DUAL_RANK_MASK	(BIT(25U) | BIT(24U))

#define DDRPHY_BASE		0xFD080000U
#define DDRPHY_PIR		(DDRPHY_BASE + 4U)
#define DDRPHY_PGCR(n)		(DDRPHY_BASE + 0x10U + (4U * (n)))
#define DDRPHY_PGSR(n)		(DDRPHY_BASE + 0x30U + (4U * (n)))
#define DDRPHY_PTR(n)		(DDRPHY_BASE + 0X40U + (4U * (n)))
#define DDRPHY_PLLCR(n)		(DDRPHY_BASE + 0X68U + (4U * (n)))
#define DDRPHY_DSGCR		(DDRPHY_BASE + 0X90U)
#define DDRPHY_ODTCR		(DDRPHY_BASE + 0X98U)
#define DDRPHY_GPR(n)		(DDRPHY_BASE + 0XC0U + (4U * (n)))
#define DDRPHY_DCR		(DDRPHY_BASE + 0X100U)
#define DDRPHY_DTPR(n)		(DDRPHY_BASE + 0X110U + (4U * (n)))
#define DDRPHY_RDIMMGCR(n)	(DDRPHY_BASE + 0x140U + (4U * (n)))
#define DDRPHY_RDIMMCR(n)	(DDRPHY_BASE + 0x150U + (4U * (n)))
#define DDRPHY_MR(n)		(DDRPHY_BASE + 0X180U + (4U * (n)))
#define DDRPHY_DTCR(n)		(DDRPHY_BASE + 0X200U + (4U * (n)))
#define DDRPHY_CATR(n)		(DDRPHY_BASE + 0X240U + (4U * (n)))
#define DDRPHY_RANKIDR		(DDRPHY_BASE + 0X4DCU)
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
#define DDRPHY_DXGSR0(n)	(DDRPHY_BASE + 0X7e0U + (0x100U * (n)))
#define DDRPHY_DX8SLNOSC(n)	(DDRPHY_BASE + 0x1400U + (0x40U * (n)))
#define DDRPHY_DX8SLPLLCR(n, m)	(DDRPHY_BASE + 0X1404U + (0x40U * (n)) + (4U * (m)))
#define DDRPHY_DX8SLDQSCTL(n)	(DDRPHY_BASE + 0x141cU + (0x40U * (n)))
#define DDRPHY_DX8SLDXCTL2(n)	(DDRPHY_BASE + 0x142cU + (0x40U * (n)))
#define DDRPHY_DX8SLIOCR(n)	(DDRPHY_BASE + 0x1430U + (0x40U * (n)))
#define DDRPHY_DX8SLBOSC	(DDRPHY_BASE + 0x17c0U)

#define DDRPHY_PIR_INIT			BIT(0U)
#define DDRPHY_PIR_ZCAL			BIT(1U)
#define DDRPHY_PIR_CA			BIT(2U)
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
#define DDRPHY_PIR_DQS2DQ		BIT(20U)
#define DDRPHY_PIR_ZCALBYP		BIT(30U)

#define DDRPHY_PGCR0_PHYFRST	BIT(26U)

#define DDRPHY_DXGSR0_DPLOCK	BIT(16U)

#define DDRPHY_PGSR0_APLOCK	BIT(31U)
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

#define DDRPHY_RIOCR2_COEMODE_SHIFT	24U
#define DDRPHY_RIOCR2_COEMODE_MASK	(BIT(25) | BIT(24))
#define DDRPHY_RIOCR2_CSOEMODE_MASK	(BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define DDRPHY_RIOCR5_ODTOEMODE_MASK	(BIT(3) | BIT(2) | BIT(1) | BIT(0))

#define DDRPHY_ACIOCR0_RSTPDR		BIT(28)
#define DDRPHY_ACIOCR0_RSTODT		BIT(26)
#define DDRPHY_ACIOCR0_ACPDRMDOE_SHIFT	4U
#define DDRPHY_ACIOCR0_ACPDRMDOE_MASK	(BIT(5) | BIT(4))
#define DDRPHY_ACIOCR0_ACODTMODE_SHIFT	2U
#define DDRPHY_ACIOCR0_ACODTMODE_MASK	(BIT(3) | BIT(2))
#define DDRPHY_ACIOCR1_AOEMODE_MASK	((u32)0xFFFFFFFFU)
#define DDRPHY_ACIOCR3_PAROEMODE_SHIFT	30U
#define DDRPHY_ACIOCR3_PAROEMODE_MASK	(BIT(31) | BIT(30))
#define DDRPHY_ACIOCR3_BGOEMODE_SHIFT	26U
#define DDRPHY_ACIOCR3_BGOEMODE_MASK	(BIT(29) | BIT(28) | BIT(27) | BIT(26))
#define DDRPHY_ACIOCR3_BAOEMODE_SHITT	22U
#define DDRPHY_ACIOCR3_BAOEMODE_MASK	(BIT(25) | BIT(24) | BIT(23) | BIT(22))
#define DDRPHY_ACIOCR3_A17OEMODE_SHIFT	20U
#define DDRPHY_ACIOCR3_A17OEMODE_MASK	(BIT(21) | BIT(20))
#define DDRPHY_ACIOCR3_A16OEMODE_SHIFT	18U
#define DDRPHY_ACIOCR3_A16OEMODE_MASK	(BIT(19) | BIT(18))
#define DDRPHY_ACIOCR3_ACTOEMODE_SHIFT	16U
#define DDRPHY_ACIOCR3_ACTOEMODE_MASK	(BIT(17) | BIT(16))
#define DDRPHY_ACIOCR3_CKOEMODE		(BIT(3) | BIT(2) | BIT(1) | BIT(0))

#define DDRPHY_IOVCR0_ACREFSEN		BIT(25)
#define DDRPHY_IOVCR0_ACREFIEN		BIT(24)

#define DDRPHY_ZQCR_ZQREFIEN		BIT(11)
#define DDRPHY_ZQCR_ZQPD			BIT(0)

#define DDRPHY_ZQnPR0_ZDEN_SHIFT	28U
#define DDRPHY_ZQnPR0_ZDEN_MASK		((u32)0xF0000000U)
#define DDRPHY_ZQnPR0_ZSEGBYP		BIT(27U)
#define DDRPHY_ZQnOR_OFFSET		8U

#define DDRPHY_DX8SLBOSC_PHYFRST	BIT(15U)

#define DDRPHY_DTCR0_INCWEYE		BIT(4U)
#define DDRPHY_DTCR0_RFSHDT_SHIFT	28U
#define DDRPHY_DTCR0_RFSHDT_MASK	((u32)0xF0000000U)
#define DDRPHY_DTCR0_RFSHEN_SHIFT	8U
#define DDRPHY_DTCR0_RFSHEN_MASK	((u32)0x00000F00U)

#define DDRPHY_DSGCR_CTLZUEN		BIT(2U)
#define DDRPHY_DSGCR_DTOPDR		BIT(14U)
#define DDRPHY_DSGCR_DTOODT		BIT(12U)

#define DDRPHY_DXGCR3_WDLVT		BIT(25U)
#define DDRPHY_DXGCR3_RGLVT		BIT(27U)

#define DDRPHY_RANKWID_MASK		(BIT(3U)| BIT(2U) | BIT(1U) | BIT(0U))
#define DDRPHY_RANKRID_MASK		(BIT(19U) | BIT(18U) | BIT(17U) | BIT(16U))
#define DDRPHY_RANK0_WRITE		BIT(0U)
#define DDRPHY_RANK1_WRITE		BIT(1U)
#define DDRPHY_RANK0_READ		BIT(16U)
#define DDRPHY_RANK1_READ		BIT(17U)

#define DDRQOS_BASE		0xFD090000U
#define DDRQOS_DDR_CLK_CTRL	(DDRQOS_BASE + 0x700U)

#define DDRQOS_DDR_CLK_CTRL_CLKACT	BIT(0U)

#define PM_DDR_POLL_PERIOD		32000U	/* ~1ms @220MHz */

#ifdef ENABLE_DDR_SR_WR
/* Timeout period of around 1 second to poll for DDR flags */
#define DDR_FLAG_POLL_PERIOD		XPAR_MICROBLAZE_FREQ
#endif

#define REPORT_IF_ERROR(status) \
		if (XST_SUCCESS != (status)) { \
			PmErr("@line %d\r\n", __LINE__); \
		}

/* Power states of DDR */
#define PM_DDR_STATE_OFF	0U
#define PM_DDR_STATE_SR		1U
#define PM_DDR_STATE_ON		2U
#define PM_DDR_STATE_MAX	3U

/* Power consumptions for DDR defined by its states */
#define DEFAULT_DDR_POWER_ON		100U
#define DEFAULT_DDR_POWER_SR		50U
#define DEFAULT_DDR_POWER_OFF		0U

/* Number of memory locations used for ddr data training */
#define DDR3_SIZE	0X100U >> 2U
#define DDR4_SIZE	0x200U >> 2U
#define DDR4_SIZE_OLD	0x100U >> 2U
#define LPDDR3_SIZE	0x100U >> 2U
#define LPDDR4_SIZE	0x100U >> 2U

/* DDR4 old mapping ddr data training location offset */
#define OLD_MAP_OFFSET	0x2000U
#define LPDDR4_OLD_MAP_OFFSET	0x4000U

/* DDR reserved address to store training data */
#define RESERVED_ADDRESS	XPAR_MICROBLAZE_DDR_RESERVE_SA

/* DIMM address mirroring */
#define DDRC_DIMMCTL_DIMM_ADDR_MIRR_EN	(0x00000002U)

#define IS_ADDR_MIRR()	(Xil_In32(DDRC_DIMMCTL) & DDRC_DIMMCTL_DIMM_ADDR_MIRR_EN)

/* ADDRMAP_BG_B1 */
#define DDRC_ADDRMAP8_BG_B1_MASK	(0x00001F00U)
#define DDRC_ADDRMAP8_BG_B1_SHIFT	(8U)
#define DDRC_ADDRMAP8_BG_B1_BASE	(3U)

/* Low DDR address size: 2 GB */
#define DDR_LO_ADDR		(0x0000000000000000ULL)
#define DDR_LO_SIZE		(0x0000000080000000ULL)

/* High DDR address size: 32 */
#define DDR_HI_ADDR		(0x0000000800000000ULL)
#define DDR_HI_SIZE		(0x0000000800000000ULL)

#define ADDR_HI(ADDR)	((u32)((u64)(ADDR) >> 32U))
#define ADDR_LO(ADDR)	((u32)((u64)(ADDR) & 0x00000000FFFFFFFFULL))

/* DDR states */
static const u8 pmDdrStates[PM_DDR_STATE_MAX] = {
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

#ifdef XPAR_DDRCPSU_0_DEVICE_ID
/* If it is required to enable drift */
static u8 drift_enable_req __attribute__((__section__(".srdata")));

static PmRegisterContext ctx_ddrc[] __attribute__((__section__(".srdata"))) = {
	{ .addr = DDRC_MSTR, },
	{ .addr = DDRC_MRCTRL0, },
	{ .addr = DDRC_DERATEEN, },
	{ .addr = DDRC_DERATEINT, },
	{ .addr = DDRC_PWRCTL, },
	{ .addr = DDRC_PWRTMG, },
	{ .addr = DDRC_RFSHCTL0, },
	{ .addr = DDRC_RFSHCTL1, },
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
	{ .addr = DDRC_DRAMTMG(13U), },
	{ .addr = DDRC_DRAMTMG(14U), },
	{ .addr = DDRC_ZQCTL(0U), },
	{ .addr = DDRC_ZQCTL(1U), },
	{ .addr = DDRC_DFITMG0, },
	{ .addr = DDRC_DFITMG1, },
	{ .addr = DDRC_DFILPCFG(0U), },
	{ .addr = DDRC_DFILPCFG(1U), },
	{ .addr = DDRC_DFIUPD0, },
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

static PmRegisterContext ctx_ddrphy[] __attribute__((__section__(".srdata"))) = {
	{ .addr = DDRPHY_PGCR(0U), },
	{ .addr = DDRPHY_PGCR(2U), },
	{ .addr = DDRPHY_PGCR(3U), },
	{ .addr = DDRPHY_PGCR(5U), },
	{ .addr = DDRPHY_PTR(0U), },
	{ .addr = DDRPHY_PTR(1U), },
	{ .addr = DDRPHY_PLLCR(0U), },
	{ .addr = DDRPHY_DSGCR, },
	{ .addr = DDRPHY_GPR(0U), },
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
	{ .addr = DDRPHY_ACIOCR(5U), },
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
	{ .addr = DDRPHY_DXGCR(0U, 1U), },
	{ .addr = DDRPHY_DXGCR(0U, 2U), },
	{ .addr = DDRPHY_DXGCR(0U, 3U), },
	{ .addr = DDRPHY_DXGCR(0U, 4U), },
	{ .addr = DDRPHY_DXGCR(0U, 5U), },
	{ .addr = DDRPHY_DXGCR(0U, 6U), },
	{ .addr = DDRPHY_DXGCR(1U, 0U), },
	{ .addr = DDRPHY_DXGCR(1U, 1U), },
	{ .addr = DDRPHY_DXGCR(1U, 2U), },
	{ .addr = DDRPHY_DXGCR(1U, 3U), },
	{ .addr = DDRPHY_DXGCR(1U, 4U), },
	{ .addr = DDRPHY_DXGCR(1U, 5U), },
	{ .addr = DDRPHY_DXGCR(1U, 6U), },
	{ .addr = DDRPHY_DXGCR(2U, 0U), },
	{ .addr = DDRPHY_DXGCR(2U, 1U), },
	{ .addr = DDRPHY_DXGCR(2U, 2U), },
	{ .addr = DDRPHY_DXGCR(2U, 3U), },
	{ .addr = DDRPHY_DXGCR(2U, 4U), },
	{ .addr = DDRPHY_DXGCR(2U, 5U), },
	{ .addr = DDRPHY_DXGCR(2U, 6U), },
	{ .addr = DDRPHY_DXGCR(3U, 0U), },
	{ .addr = DDRPHY_DXGCR(3U, 1U), },
	{ .addr = DDRPHY_DXGCR(3U, 2U), },
	{ .addr = DDRPHY_DXGCR(3U, 3U), },
	{ .addr = DDRPHY_DXGCR(3U, 4U), },
	{ .addr = DDRPHY_DXGCR(3U, 5U), },
	{ .addr = DDRPHY_DXGCR(3U, 6U), },
	{ .addr = DDRPHY_DXGCR(4U, 0U), },
	{ .addr = DDRPHY_DXGCR(4U, 1U), },
	{ .addr = DDRPHY_DXGCR(4U, 2U), },
	{ .addr = DDRPHY_DXGCR(4U, 3U), },
	{ .addr = DDRPHY_DXGCR(4U, 4U), },
	{ .addr = DDRPHY_DXGCR(4U, 5U), },
	{ .addr = DDRPHY_DXGCR(4U, 6U), },
	{ .addr = DDRPHY_DXGCR(5U, 0U), },
	{ .addr = DDRPHY_DXGCR(5U, 1U), },
	{ .addr = DDRPHY_DXGCR(5U, 2U), },
	{ .addr = DDRPHY_DXGCR(5U, 3U), },
	{ .addr = DDRPHY_DXGCR(5U, 4U), },
	{ .addr = DDRPHY_DXGCR(5U, 5U), },
	{ .addr = DDRPHY_DXGCR(5U, 6U), },
	{ .addr = DDRPHY_DXGCR(6U, 0U), },
	{ .addr = DDRPHY_DXGCR(6U, 1U), },
	{ .addr = DDRPHY_DXGCR(6U, 2U), },
	{ .addr = DDRPHY_DXGCR(6U, 3U), },
	{ .addr = DDRPHY_DXGCR(6U, 4U), },
	{ .addr = DDRPHY_DXGCR(6U, 5U), },
	{ .addr = DDRPHY_DXGCR(6U, 6U), },
	{ .addr = DDRPHY_DXGCR(7U, 0U), },
	{ .addr = DDRPHY_DXGCR(7U, 1U), },
	{ .addr = DDRPHY_DXGCR(7U, 2U), },
	{ .addr = DDRPHY_DXGCR(7U, 3U), },
	{ .addr = DDRPHY_DXGCR(7U, 4U), },
	{ .addr = DDRPHY_DXGCR(7U, 5U), },
	{ .addr = DDRPHY_DXGCR(7U, 6U), },
	{ .addr = DDRPHY_DXGCR(8U, 0U), },
	{ .addr = DDRPHY_DXGCR(8U, 1U), },
	{ .addr = DDRPHY_DXGCR(8U, 2U), },
	{ .addr = DDRPHY_DXGCR(8U, 3U), },
	{ .addr = DDRPHY_DXGCR(8U, 4U), },
	{ .addr = DDRPHY_DXGCR(8U, 5U), },
	{ .addr = DDRPHY_DXGCR(8U, 6U), },
	{ .addr = DDRPHY_DX8SLNOSC(0U), },
	{ .addr = DDRPHY_DX8SLPLLCR(0U, 0U), },
	{ .addr = DDRPHY_DX8SLDQSCTL(0U), },
	{ .addr = DDRPHY_DX8SLDXCTL2(0U), },
	{ .addr = DDRPHY_DX8SLIOCR(0U), },
	{ .addr = DDRPHY_DX8SLNOSC(1U), },
	{ .addr = DDRPHY_DX8SLPLLCR(1U, 0U), },
	{ .addr = DDRPHY_DX8SLDQSCTL(1U), },
	{ .addr = DDRPHY_DX8SLDXCTL2(1U), },
	{ .addr = DDRPHY_DX8SLIOCR(1U), },
	{ .addr = DDRPHY_DX8SLNOSC(2U), },
	{ .addr = DDRPHY_DX8SLPLLCR(2U, 0U), },
	{ .addr = DDRPHY_DX8SLDQSCTL(2U), },
	{ .addr = DDRPHY_DX8SLDXCTL2(2U), },
	{ .addr = DDRPHY_DX8SLIOCR(2U), },
	{ .addr = DDRPHY_DX8SLNOSC(3U), },
	{ .addr = DDRPHY_DX8SLPLLCR(3U, 0U), },
	{ .addr = DDRPHY_DX8SLDQSCTL(3U), },
	{ .addr = DDRPHY_DX8SLDXCTL2(3U), },
	{ .addr = DDRPHY_DX8SLIOCR(3U), },
	{ .addr = DDRPHY_DX8SLNOSC(4U), },
	{ .addr = DDRPHY_DX8SLPLLCR(4U, 0U), },
	{ .addr = DDRPHY_DX8SLDQSCTL(4U), },
	{ .addr = DDRPHY_DX8SLDXCTL2(4U), },
	{ .addr = DDRPHY_DX8SLIOCR(4U), },
	{ },
};

static PmRegisterContext ctx_ddrphy_odtcr[] __attribute__((__section__(".srdata"))) = {
	{ .addr = DDRPHY_ODTCR, },
	{ .addr = DDRPHY_ODTCR, },
};

static PmRegisterContext ctx_ddrphy_zqdata[] __attribute__((__section__(".srdata"))) = {
	{ .addr = DDRPHY_ZQDR0(0U), },
	{ .addr = DDRPHY_ZQDR1(0U), },
	{ .addr = DDRPHY_ZQDR0(1U), },
	{ .addr = DDRPHY_ZQDR1(1U), },
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
	u32 i;

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

	for (i = 0U; i < 9U; i++) {
		r = Xil_In32(DDRPHY_DXGCR(i, 3U));
		r |= DDRPHY_DXGCR3_RGLVT;
		Xil_Out32(DDRPHY_DXGCR(i, 3U), r);
	}

	r = Xil_In32(DDRPHY_DQSDR(1U));
	r &= ~DDRPHY_DQSDR1_DFTRDIDLC;
	r |= (1U << DDRPHY_DQSDR1_DFTRDIDLC_SHIFT);
	Xil_Out32(DDRPHY_DQSDR(1U), r);

	r = Xil_In32(DDRPHY_DQSDR(1U));
	r &= ~DDRPHY_DQSDR1_DFTRDIDLF;
	r |= ((u32)10U << DDRPHY_DQSDR1_DFTRDIDLF_SHIFT);
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
	u32 i;

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTDTMODE;
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTUPMODE;
	r |= (1U << DDRPHY_DQSDR0_DFTUPMODE_SHIFT);
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTGPULSE;
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTRDSPC;
	r |= ((u32)1 << DDRPHY_DQSDR0_DFTRDSPC_SHIFT);
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	r = Xil_In32(DDRPHY_DQSDR(0U));
	r &= ~DDRPHY_DQSDR0_DFTDLY;
	r |= ((u32)2 << DDRPHY_DQSDR0_DFTDLY_SHIFT);
	Xil_Out32(DDRPHY_DQSDR(0U), r);

	for (i = 0U; i < 9U; i++) {
		r = Xil_In32(DDRPHY_DXGCR(i, 3U));
		r &= ~DDRPHY_DXGCR3_RGLVT;
		Xil_Out32(DDRPHY_DXGCR(i, 3U), r);
	}

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

static void ddr_enable_drift(void)
{
	u32 readVal;
	/* Enable drift only if it is previously enabled */
	if (0U == drift_enable_req) {
		return;
	}

	readVal = Xil_In32(DDRC_MSTR);
	if (0U != (readVal & DDRC_MSTR_LPDDR3)) {
		/* enable read drift only for LPDDR3 */
		ddr_enable_rd_drift();
	} else if (0U != (readVal & DDRC_MSTR_LPDDR4)) {
		/* enable read and write drift for LPDDR4 */
		ddr_enable_rd_drift();
		ddr_enable_wr_drift();
	} else {
		/* For MISRA compliance */
	}
	drift_enable_req = 0U;
	/* do not enable drift for DDR3/4, and LPDDR2 is not supported */
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

static s32 ddrc_enable_sr(void)
{
	u32 r;
	size_t i;

	/* disable AXI ports */
	for (i = 0U; i < 6U; i++) {
		while ((Xil_In32(DDRC_PSTAT) & DDRC_PSTAT_PORT_BUSY(i)) != 0U) {
			;
		}
		r = Xil_In32(DDRC_PCTRL(i));
		r &= ~DDRC_PCTRL_PORT_EN;
		Xil_Out32(DDRC_PCTRL(i), r);
	}

	/* enable self refresh */
	r = Xil_In32(DDRC_PWRCTL);
	r |= DDRC_PWRCTL_SR_SW;
	Xil_Out32(DDRC_PWRCTL, r);

	while (true != ddrc_opmode_is_sr()) {
		;
	}
	while ((Xil_In32(DDRC_STAT) & (3U << 4U)) != (2U << 4U)) {
		;
	}
	return XST_SUCCESS;
}

static void ddr_clock_enable(void)
{
	u32 r = Xil_In32(DDRQOS_DDR_CLK_CTRL);
	r |= DDRQOS_DDR_CLK_CTRL_CLKACT;
	Xil_Out32(DDRQOS_DDR_CLK_CTRL, r);
}

static void store_state(PmRegisterContext *context)
{
	while (context->addr != 0U) {
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
		} else if ((context->addr == DDRPHY_DX8SLNOSC(0U)) ||
			   (context->addr == DDRPHY_DX8SLNOSC(1U)) ||
			   (context->addr == DDRPHY_DX8SLNOSC(2U)) ||
			   (context->addr == DDRPHY_DX8SLNOSC(3U)) ||
			   (context->addr == DDRPHY_DX8SLNOSC(4U))) {
			/* assert FIFO reset */
			context->value &= ~DDRPHY_DX8SLBOSC_PHYFRST;
		} else {
			/* For MISRA compliance */
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
	while (context->addr != 0U) {
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
	while (context->addr != 0U) {
#ifdef DDRSR_DEBUG_STATE
		ddr_print_dbg("%s: addr:%lx, value:%lx\r\n",
			      __func__, context->addr + 8U, context->value);
#endif
		/* write result data back to override register */
		Xil_Out32(context->addr + DDRPHY_ZQnOR_OFFSET, context->value);
		context++;
	}
}

static void store_ddrphy_odtcr(PmRegisterContext *context)
{
	u32 rank = Xil_In32(DDRC_MSTR) & DDRC_DUAL_RANK_MASK;
	u32 readVal = Xil_In32(DDRC_MSTR) & DDRC_MSTR_DDR_TYPE;

	if (DDRC_MSTR_LPDDR3 == readVal) {
		if (DDRC_DUAL_RANK_MASK == rank) {
			XPfw_RMW32(DDRPHY_RANKIDR, DDRPHY_RANKRID_MASK,
				   DDRPHY_RANK1_READ);
			context->value = Xil_In32(context->addr);
		}
		context++;
		XPfw_RMW32(DDRPHY_RANKIDR, DDRPHY_RANKRID_MASK,
			   DDRPHY_RANK0_READ);
		context->value = Xil_In32(context->addr);
	}
}

static void restore_ddrphy_odtcr(PmRegisterContext *context)
{
	u32 rank = Xil_In32(DDRC_MSTR) & DDRC_DUAL_RANK_MASK;
	u32 readVal = Xil_In32(DDRC_MSTR) & DDRC_MSTR_DDR_TYPE;

	if (DDRC_MSTR_LPDDR3 == readVal) {
		if (DDRC_DUAL_RANK_MASK == rank) {
			XPfw_RMW32(DDRPHY_RANKIDR, DDRPHY_RANKWID_MASK,
				   DDRPHY_RANK1_WRITE);
			Xil_Out32(context->addr, context->value);
		}
		context++;
		XPfw_RMW32(DDRPHY_RANKIDR, DDRPHY_RANKWID_MASK,
			   DDRPHY_RANK0_WRITE);
		Xil_Out32(context->addr, context->value);
	}
}

static void ddr_io_retention_set(bool en)
{
	u32 r = Xil_In32(PMU_GLOBAL_DDR_CNTRL);
	if (false != en) {
		r |= PMU_GLOBAL_DDR_CNTRL_RET_MASK;
	} else {
		r &= ~PMU_GLOBAL_DDR_CNTRL_RET_MASK;
	}
	Xil_Out32(PMU_GLOBAL_DDR_CNTRL, r);
}

static void ddr_power_down_io(void)
{
	u32 i;

	/* prepare for SR, minimizing power consumption */
	/* DSGCR.DTOPDR[14] = 1 */
	XPfw_UtilRMW(DDRPHY_DSGCR, DDRPHY_DSGCR_DTOPDR, DDRPHY_DSGCR_DTOPDR);

	/* DSGCR.DTOODT[12] = 0 */
	XPfw_UtilRMW(DDRPHY_DSGCR, DDRPHY_DSGCR_DTOODT, ~DDRPHY_DSGCR_DTOODT);

	/* RIOCR2.COEMODE[25:24] = 10 */
	XPfw_UtilRMW(DDRPHY_RIOCR(2U), DDRPHY_RIOCR2_COEMODE_MASK,
		     (u32)0x2 << DDRPHY_RIOCR2_COEMODE_SHIFT);

	/* RIOCR2.CSOEMODE[3:0] = 1010 */
	XPfw_UtilRMW(DDRPHY_RIOCR(2U), DDRPHY_RIOCR2_CSOEMODE_MASK , 0xAU);

	/* RIOCR5.ODTOEMODE[3:0] = 1010 */
	XPfw_UtilRMW(DDRPHY_RIOCR(5U), DDRPHY_RIOCR5_ODTOEMODE_MASK, 0xAU);

	/* ACIOCR0.RSTPDR[28] = 1 */
	XPfw_UtilRMW(DDRPHY_ACIOCR(0U), DDRPHY_ACIOCR0_RSTPDR,
		     DDRPHY_ACIOCR0_RSTPDR);

	/* ACIOCR0.RSTODT[26] = 0 */
	XPfw_UtilRMW(DDRPHY_ACIOCR(0U), DDRPHY_ACIOCR0_RSTODT,
		     ~DDRPHY_ACIOCR0_RSTODT);

	/* ACIOCR0.ACPDRMODE[5:4] = 01 */
	XPfw_UtilRMW(DDRPHY_ACIOCR(0U), DDRPHY_ACIOCR0_ACPDRMDOE_MASK,
		     (u32)0x1 << DDRPHY_ACIOCR0_ACPDRMDOE_SHIFT);

	/* ACIOCR0.ACODTMODE[3:2] = 10 */
	XPfw_UtilRMW(DDRPHY_ACIOCR(0U), DDRPHY_ACIOCR0_ACODTMODE_MASK,
		     (u32)0x2 << DDRPHY_ACIOCR0_ACODTMODE_SHIFT);

	/* ACIOCR1.AOEMODE[31:0] = 0xAAAAAAAA */
	XPfw_UtilRMW(DDRPHY_ACIOCR(1U), DDRPHY_ACIOCR1_AOEMODE_MASK,
		     0xAAAAAAAAU);

	/* ACIOCR3.PAROEMODE[31:30] = 10 */
	XPfw_UtilRMW(DDRPHY_ACIOCR(3U), DDRPHY_ACIOCR3_PAROEMODE_MASK,
		     (u32)0x2 << DDRPHY_ACIOCR3_PAROEMODE_SHIFT);

	/* ACIOCR3.BGOEMODE[29:26] = 1010 */
	XPfw_UtilRMW(DDRPHY_ACIOCR(3U), DDRPHY_ACIOCR3_BGOEMODE_MASK,
		     (u32)0xA << DDRPHY_ACIOCR3_BGOEMODE_SHIFT);

	/* ACIOCR3.BAOEMODE[25:22] = 1010 */
	XPfw_UtilRMW(DDRPHY_ACIOCR(3U), DDRPHY_ACIOCR3_BAOEMODE_MASK,
		     (u32)0xA << DDRPHY_ACIOCR3_BAOEMODE_SHITT);

	/* ACIOCR3.A17OEMODE[21:20] = 10 */
	XPfw_UtilRMW(DDRPHY_ACIOCR(3U), DDRPHY_ACIOCR3_A17OEMODE_MASK,
		     (u32)0x2 << DDRPHY_ACIOCR3_A17OEMODE_SHIFT);

	/* ACIOCR3.A16OEMODE[19:18] = 10 */
	XPfw_UtilRMW(DDRPHY_ACIOCR(3U), DDRPHY_ACIOCR3_A16OEMODE_MASK,
		     (u32)0x2 << DDRPHY_ACIOCR3_A16OEMODE_SHIFT);

	/* ACIOCR3.ACTOEMODE[17:16] = 10 */
	XPfw_UtilRMW(DDRPHY_ACIOCR(3U), DDRPHY_ACIOCR3_ACTOEMODE_MASK,
		     (u32)0x2 << DDRPHY_ACIOCR3_ACTOEMODE_SHIFT);

	/* ACIOCR3.CKOEMODE[3:0] = 1010 */
	XPfw_UtilRMW(DDRPHY_ACIOCR(3U), DDRPHY_ACIOCR3_CKOEMODE, 0xAU);

	/* IOVCR0.ACREFSEN[25] = 0 */
	/* IOVCR0.ACREFIEN[24] = 0 */
	XPfw_UtilRMW(DDRPHY_IOVCR(0U), (DDRPHY_IOVCR0_ACREFSEN |
		     DDRPHY_IOVCR0_ACREFIEN),
		     ~(DDRPHY_IOVCR0_ACREFSEN |
		     DDRPHY_IOVCR0_ACREFIEN));

	/* ZQCR.ZQREFIEN[11] = 0 */
	XPfw_UtilRMW(DDRPHY_ZQCR, DDRPHY_ZQCR_ZQREFIEN, ~DDRPHY_ZQCR_ZQREFIEN);

	/* ZQCR.ZQPD[0] = 1 */
	XPfw_UtilRMW(DDRPHY_ZQCR, DDRPHY_ZQCR_ZQPD, DDRPHY_ZQCR_ZQPD);

	/* DX[8:0]GCR0.DQSNSEPDR[13] = 1 */
	/* DX[8:0]GCR0.DQSSEPDR[12] = 1 */
	/* DX[8:0]GCR0.DQSRPD[6] = 1 */
	/* DX[8:0]GCR0.DQSGPDR[5] = 1 */
	/* DX[8:0]GCR0.DQSGODT[3] = 0 */
	/* DX[8:0]GCR0.DQSGOE[2] = 0 */
	for (i = 0U; i < 9U; i++) {
		XPfw_UtilRMW(DDRPHY_DXGCR(i, 0U), 0x306CU, 0x3060U);
	}

	/* DX[8:0]GCR1.DXPDRMODE[31:16] = 0x5555 */
	for (i = 0U; i < 9U; i++) {
		XPfw_UtilRMW(DDRPHY_DXGCR(i, 1U), 0xFFFF0000U, (u32)0x5555 << 16U);
	}

	/* DX[8:0]GCR2.DXOEMODE[31:16] = 0xAAAA */
	/* DX[8:0]GCR2.DXTEMODE[15:0] = 0xAAAA */
	for (i = 0U; i < 9U; i++) {
		XPfw_UtilRMW(DDRPHY_DXGCR(i, 2U), 0xFFFFFFFFU, 0xAAAAAAAAU);
	}

	/* DX[8:0]GCR3.DSNOEMODE[21:20] = 10 */
	/* DX[8:0]GCR3.DSNTEMODE[19:18] = 10 */
	/* DX[8:0]GCR3.DSNPDRMODE[17:16] = 01 */
	/* DX[8:0]GCR3.DMOEMODE[15:14] = 10 */
	/* DX[8:0]GCR3.DMTEMODE[13:12] = 10 */
	/* DX[8:0]GCR3.DMPDRMODE[11:10] = 01 */
	/* DX[8:0]GCR3.DSOEMODE[7:6] = 10 */
	/* DX[8:0]GCR3.DSTEMODE[5:4] = 10 */
	/* DX[8:0]GCR3.DSPDRMODE[3:2] = 01 */
	for (i = 0U; i < 9U; i++) {
		XPfw_UtilRMW(DDRPHY_DXGCR(i, 3U), 0x3FFCFCU, 0x29A4A4U);
	}

	/* DX[8:0]GCR4.DXREFSEN[25] = 0 */
	/* DX[8:0]GCR4.DXREFIEN[5:2] = 0000 */
	for (i = 0U; i < 9U; i++) {
		XPfw_UtilRMW(DDRPHY_DXGCR(i, 4U), 0x200003CU, 0x0U);
	}
}

static void DDR_reinit(bool ddrss_is_reset)
{
	size_t i;
	u32 readVal, busWidth;
	XStatus status = XST_FAILURE;

	if (true == ddrss_is_reset) {
		/* Data Bus Width */
		readVal = Xil_In32(DDRC_MSTR);
		busWidth = (readVal & ((u32)DDRC_MSTR_BUS_WIDTH_MASK <<
				       DDRC_MSTR_BUS_WIDTH_SHIFT));

		/* PHY init */
		do {
			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_ZCALBYP |
					      DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_PLLINIT);
			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_ZCALBYP |
					      DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_PLLINIT |
					      DDRPHY_PIR_INIT);
			status = XPfw_UtilPollForMask(DDRPHY_PGSR(0U),
						      DDRPHY_PGSR0_IDONE |
						      DDRPHY_PGSR0_APLOCK,
						      PM_DDR_POLL_PERIOD);
			if (XST_SUCCESS != status) {
				continue;
			}
			status = XPfw_UtilPollForMask(DDRPHY_DXGSR0(0U),
						      DDRPHY_DXGSR0_DPLOCK,
						      PM_DDR_POLL_PERIOD);
			if (XST_SUCCESS != status) {
				continue;
			}
			if ((DDRC_MSTR_BUS_WIDTH_FULL_DQ == busWidth) ||
			    (DDRC_MSTR_BUS_WIDTH_HALF_DQ == busWidth)) {
				status = XPfw_UtilPollForMask(DDRPHY_DXGSR0(2U),
							   DDRPHY_DXGSR0_DPLOCK,
							   PM_DDR_POLL_PERIOD);
				if (XST_SUCCESS != status) {
					continue;
				}
			}
			if (DDRC_MSTR_BUS_WIDTH_FULL_DQ == busWidth)  {
				status = XPfw_UtilPollForMask(DDRPHY_DXGSR0(4U),
							   DDRPHY_DXGSR0_DPLOCK,
							   PM_DDR_POLL_PERIOD);
				if (XST_SUCCESS != status) {
					continue;
				}
				status = XPfw_UtilPollForMask(DDRPHY_DXGSR0(6U),
							   DDRPHY_DXGSR0_DPLOCK,
							   PM_DDR_POLL_PERIOD);
				if (XST_SUCCESS != status) {
					continue;
				}
			}
#if XPAR_PSU_DDRC_0_HAS_ECC
			status = XPfw_UtilPollForMask(DDRPHY_DXGSR0(8U),
						      DDRPHY_DXGSR0_DPLOCK,
						      PM_DDR_POLL_PERIOD);
			if (XST_SUCCESS != status) {
				continue;
			}
#endif
		} while (XST_SUCCESS != status);

		status = XPfw_UtilPollForZero(DDRPHY_PGSR(0U),
					      DDRPHY_PGSR0_TRAIN_ERRS,
					      PM_DDR_POLL_PERIOD);
		REPORT_IF_ERROR(status);

		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_ZCALBYP |
				      DDRPHY_PIR_CTLDINIT |
				      DDRPHY_PIR_PHYRST |
				      DDRPHY_PIR_DCAL);
		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_ZCALBYP |
				      DDRPHY_PIR_CTLDINIT |
				      DDRPHY_PIR_PHYRST |
				      DDRPHY_PIR_DCAL |
				      DDRPHY_PIR_INIT);
		status = XPfw_UtilPollForMask(DDRPHY_PGSR(0U),
					      DDRPHY_PGSR0_IDONE,
					      PM_DDR_POLL_PERIOD);
		REPORT_IF_ERROR(status);

		status = XPfw_UtilPollForZero(DDRPHY_PGSR(0U),
					      DDRPHY_PGSR0_TRAIN_ERRS,
					      PM_DDR_POLL_PERIOD);
		REPORT_IF_ERROR(status);

		for (i = 0U; i < 2U; i++) {
			readVal = Xil_In32(DDRPHY_ZQPR(i, 0U));
			readVal |= (DDRPHY_ZQnPR0_ZSEGBYP |
				    DDRPHY_ZQnPR0_ZDEN_MASK);
			Xil_Out32(DDRPHY_ZQPR(i, 0U), readVal);
		}
		restore_ddrphy_odtcr(ctx_ddrphy_odtcr);
		restore_ddrphy_zqdata(ctx_ddrphy_zqdata);

		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT);
		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
				      DDRPHY_PIR_INIT);
		status = XPfw_UtilPollForMask(DDRPHY_PGSR(0U),
					      DDRPHY_PGSR0_IDONE,
					      PM_DDR_POLL_PERIOD);
		REPORT_IF_ERROR(status);

		status = XPfw_UtilPollForZero(DDRPHY_PGSR(0U),
					      DDRPHY_PGSR0_TRAIN_ERRS,
					      PM_DDR_POLL_PERIOD);
		REPORT_IF_ERROR(status);

		ddr_io_retention_set(false);
#ifdef ENABLE_POS
		PmHookPowerOffSuspendDdrReady();
#endif

		/* remove ZQ override */
		for (i = 0U; i < 2U; i++) {
			readVal = Xil_In32(DDRPHY_ZQPR(i, 0U));
			readVal &= ~(DDRPHY_ZQnPR0_ZSEGBYP |
				     DDRPHY_ZQnPR0_ZDEN_MASK);
			Xil_Out32(DDRPHY_ZQPR(i, 0U), readVal);
		}

		/* zcal */
		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
				      DDRPHY_PIR_ZCAL);
		Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
				      DDRPHY_PIR_ZCAL |
				      DDRPHY_PIR_INIT);
		status = XPfw_UtilPollForMask(DDRPHY_PGSR(0U),
					      DDRPHY_PGSR0_IDONE,
					      PM_DDR_POLL_PERIOD);
		REPORT_IF_ERROR(status);

		status = XPfw_UtilPollForZero(DDRPHY_PGSR(0U),
					      DDRPHY_PGSR0_TRAIN_ERRS,
					      PM_DDR_POLL_PERIOD);
		REPORT_IF_ERROR(status);

		ddr_enable_drift();

		/* FIFO reset */
		readVal = Xil_In32(DDRPHY_PGCR(0U));
		readVal |= DDRPHY_PGCR0_PHYFRST;
		Xil_Out32(DDRPHY_PGCR(0U), readVal);

		for (i = 0U; i < 5U; i++) {
			readVal = Xil_In32(DDRPHY_DX8SLNOSC(i));
			readVal |= DDRPHY_DX8SLBOSC_PHYFRST;
			Xil_Out32(DDRPHY_DX8SLNOSC(i), readVal);
		}

		Xil_Out32(DDRC_DFIMISC, DDRC_DFIMISC_DFI_INIT_COMP_EN);
		Xil_Out32(DDRC_SWCTL, DDRC_SWCTL_SW_DONE);
	} else {
		ddr_enable_drift();
	}

	Xil_Out32(DDRC_PWRCTL, 0U);
	do {
		readVal = Xil_In32(DDRC_STAT);
		readVal &= 3U << 4U;
	} while (readVal != 0U);

	do {
		readVal = Xil_In32(DDRC_STAT);
		readVal &= DDRC_STAT_OPMODE_MASK;
		readVal >>= DDRC_STAT_OPMODE_SHIFT;
	} while (readVal != DDRC_STAT_OPMODE_NORMAL);

	if (true == ddrss_is_reset) {
		readVal = Xil_In32(DDRC_MSTR) & DDRC_MSTR_DDR_TYPE;
		if (readVal == DDRC_MSTR_LPDDR3 ) {
			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_WREYE |
					      DDRPHY_PIR_RDEYE |
					      DDRPHY_PIR_WRDSKW |
					      DDRPHY_PIR_RDDSKW |
					      DDRPHY_PIR_WLADJ |
					      DDRPHY_PIR_QSGATE |
					      DDRPHY_PIR_WL |
					      DDRPHY_PIR_CA);
			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_WREYE |
					      DDRPHY_PIR_RDEYE |
					      DDRPHY_PIR_WRDSKW |
					      DDRPHY_PIR_RDDSKW |
					      DDRPHY_PIR_WLADJ |
					      DDRPHY_PIR_QSGATE |
					      DDRPHY_PIR_WL |
					      DDRPHY_PIR_CA |
					      DDRPHY_PIR_INIT);
			status = XPfw_UtilPollForMask(DDRPHY_PGSR(0U),
						      DDRPHY_PGSR0_IDONE,
						      PM_DDR_POLL_PERIOD);
			REPORT_IF_ERROR(status);

			status = XPfw_UtilPollForZero(DDRPHY_PGSR(0U),
						      DDRPHY_PGSR0_TRAIN_ERRS,
						      PM_DDR_POLL_PERIOD);
			REPORT_IF_ERROR(status);
		} else if (readVal == DDRC_MSTR_LPDDR4) {
			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_QSGATE |
					      DDRPHY_PIR_WL);
			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_QSGATE |
					      DDRPHY_PIR_WL |
					      DDRPHY_PIR_INIT);
			status = XPfw_UtilPollForMask(DDRPHY_PGSR(0U),
						      DDRPHY_PGSR0_IDONE,
						      PM_DDR_POLL_PERIOD);
			REPORT_IF_ERROR(status);

			status = XPfw_UtilPollForZero(DDRPHY_PGSR(0U),
						      DDRPHY_PGSR0_TRAIN_ERRS,
						      PM_DDR_POLL_PERIOD);
			REPORT_IF_ERROR(status);

			readVal = Xil_In32(DDRPHY_DTCR(0U));
			readVal &= ~(DDRPHY_DTCR0_RFSHEN_MASK |
				     DDRPHY_DTCR0_RFSHDT_MASK);
			Xil_Out32(DDRPHY_DTCR(0U), readVal);

			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_DQS2DQ);
			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_DQS2DQ |
					      DDRPHY_PIR_INIT);
			status = XPfw_UtilPollForMask(DDRPHY_PGSR(0U),
						      DDRPHY_PGSR0_IDONE,
						      PM_DDR_POLL_PERIOD);
			REPORT_IF_ERROR(status);

			status = XPfw_UtilPollForZero(DDRPHY_PGSR(0U),
						      DDRPHY_PGSR0_TRAIN_ERRS,
						      PM_DDR_POLL_PERIOD);
			REPORT_IF_ERROR(status);

			readVal = Xil_In32(DDRPHY_DTCR(0U));
			readVal &= ~(DDRPHY_DTCR0_RFSHEN_MASK |
				     DDRPHY_DTCR0_RFSHDT_MASK);
			readVal |= (((u32)0x8 << DDRPHY_DTCR0_RFSHDT_SHIFT) |
				    ((u32)0x1 << DDRPHY_DTCR0_RFSHEN_SHIFT));
			Xil_Out32(DDRPHY_DTCR(0U), readVal);

			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_WREYE |
					      DDRPHY_PIR_RDEYE |
					      DDRPHY_PIR_WRDSKW |
					      DDRPHY_PIR_RDDSKW |
					      DDRPHY_PIR_WLADJ);
			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_WREYE |
					      DDRPHY_PIR_RDEYE |
					      DDRPHY_PIR_WRDSKW |
					      DDRPHY_PIR_RDDSKW |
					      DDRPHY_PIR_WLADJ |
					      DDRPHY_PIR_INIT);
			status = XPfw_UtilPollForMask(DDRPHY_PGSR(0U),
						      DDRPHY_PGSR0_IDONE,
						      PM_DDR_POLL_PERIOD);
			REPORT_IF_ERROR(status);

			status = XPfw_UtilPollForZero(DDRPHY_PGSR(0U),
						      DDRPHY_PGSR0_TRAIN_ERRS,
						      PM_DDR_POLL_PERIOD);
			REPORT_IF_ERROR(status);
		} else {
			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_WREYE |
					      DDRPHY_PIR_RDEYE |
					      DDRPHY_PIR_WRDSKW |
					      DDRPHY_PIR_RDDSKW |
					      DDRPHY_PIR_WLADJ |
					      DDRPHY_PIR_QSGATE |
					      DDRPHY_PIR_WL);
			Xil_Out32(DDRPHY_PIR, DDRPHY_PIR_CTLDINIT |
					      DDRPHY_PIR_WREYE |
					      DDRPHY_PIR_RDEYE |
					      DDRPHY_PIR_WRDSKW |
					      DDRPHY_PIR_RDDSKW |
					      DDRPHY_PIR_WLADJ |
					      DDRPHY_PIR_QSGATE |
					      DDRPHY_PIR_WL |
					      DDRPHY_PIR_INIT);
			status = XPfw_UtilPollForMask(DDRPHY_PGSR(0U),
						      DDRPHY_PGSR0_IDONE,
						      PM_DDR_POLL_PERIOD);
			REPORT_IF_ERROR(status);

			status = XPfw_UtilPollForZero(DDRPHY_PGSR(0U),
						      DDRPHY_PGSR0_TRAIN_ERRS,
						      PM_DDR_POLL_PERIOD);
			REPORT_IF_ERROR(status);
		}

		readVal = DDRPHY_PIR_CTLDINIT;
		if (0U != (Xil_In32(DDRPHY_RDIMMGCR(0U)) & DDRPHY_RDIMMGCR0_RDIMM)) {
			readVal |= DDRPHY_PIR_RDIMMINIT;
		}

		Xil_Out32(DDRPHY_PIR, readVal);
		Xil_Out32(DDRPHY_PIR, readVal | DDRPHY_PIR_INIT);
		status = XPfw_UtilPollForMask(DDRPHY_PGSR(0U),
					      DDRPHY_PGSR0_IDONE,
					      PM_DDR_POLL_PERIOD);
		REPORT_IF_ERROR(status);

		status = XPfw_UtilPollForZero(DDRPHY_PGSR(0U),
					      DDRPHY_PGSR0_TRAIN_ERRS,
					      PM_DDR_POLL_PERIOD);
		REPORT_IF_ERROR(status);

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
		status = XPfw_UtilPollForMask(DDRPHY_PGSR(0U),
					      DDRPHY_PGSR0_IDONE,
					      PM_DDR_POLL_PERIOD);
		REPORT_IF_ERROR(status);

		status = XPfw_UtilPollForZero(DDRPHY_PGSR(0U),
					      DDRPHY_PGSR0_TRAIN_ERRS,
					      PM_DDR_POLL_PERIOD);
		REPORT_IF_ERROR(status);

		/* enable AXI ports */
		for (i = 0U; i < 6U; i++) {
			while ((Xil_In32(DDRC_PSTAT) & DDRC_PSTAT_PORT_BUSY(i)) != 0U) {
				;
			}
			readVal = Xil_In32(DDRC_PCTRL(i));
			readVal |= DDRC_PCTRL_PORT_EN;
			Xil_Out32(DDRC_PCTRL(i), readVal);
		}
	}
}

static inline u32 get_old_map_offset(void)
{
	if (DDRC_MSTR_LPDDR4 ==
		(Xil_In32(DDRC_MSTR) & DDRC_MSTR_DDR_TYPE)) {
		return LPDDR4_OLD_MAP_OFFSET;
	} else {
		return OLD_MAP_OFFSET;
	}
}

static bool ddr4_is_old_mapping(void)
{
	u32 bg_b0, col_b4;
	bool old_mapping = false;
	u32 ddrcMstr;

	bg_b0 = Xil_In32(DDRC_ADDRMAP(8U)) & DDRC_ADDRMAP8_ADDRMAP_BG_B0;
	col_b4 = (Xil_In32(DDRC_ADDRMAP(2U)) & DDRC_ADDRMAP2_ADDRMAP_COL_B4) >>
		DDRC_ADDRMAP2_ADDRMAP_COL_B4_SHIFT;
	ddrcMstr = Xil_In32(DDRC_MSTR);
	if (((bg_b0 + 2U) > (col_b4 + 4U)) ||
			(DDRC_MSTR_DDR4 != (ddrcMstr &
					    DDRC_MSTR_DDR_TYPE))) {
		old_mapping = true;
	}

	return old_mapping;
}

static void ddr_rank1_addr(u32 *haddr, u32 *laddr)
{
	u32 reg;

	reg = Xil_In32(DDRC_ADDRMAP(0U)) & DDRC_ADDRMAP0_ADDRMAP_CS_BIT0;
	if (31U != reg) {
		if (reg <= 21U) {
			*haddr = 0U;
			*laddr = ((u32)1 << (reg + 9U));
		} else if (22U == reg) {
			/* Upper 32 bits for address at 32 GB. */
			*haddr = 0x00000008U;
			/* Lower 32 bits for address at 32 GB. */
			*laddr = 0x00000000U;
		} else if (23U == reg) {
			/* Upper 32 bits for address at 34 GB. */
			*haddr = 0x00000008U;
			/* Lower 32 bits for address at 34 GB. */
			*laddr = 0x80000000U;
		} else if (24U == reg) {
			/* Upper 32 bits for address at 38 GB. */
			*haddr = 0x00000009U;
			/* Lower 32 bits for address at 38 GB. */
			*laddr = 0x80000000U;
		} else if (25U == reg) {
			/* Upper 32 bits for address at 46 GB. */
			*haddr = 0x0000000BU;
			/* Lower 32 bits for address at 46 GB. */
			*laddr = 0x80000000U;
		} else {
			/*
			 * We don't support these sizes, configuration is
			 * incorrect.
			 */
			*haddr = 0U;
			*laddr = 0U;
		}
	} else {
		*haddr = 0U;
		*laddr = 0U;
	}
}

static u32 ddr_training_size(void)
{
	u32 reg, size;

	reg = Xil_In32(DDRC_MSTR) & DDRC_MSTR_DDR_TYPE;
	switch (reg) {
	case DDRC_MSTR_LPDDR4:
		size = LPDDR4_SIZE;
		break;
	case DDRC_MSTR_DDR4:
		if (false != ddr4_is_old_mapping()) {
			size = DDR4_SIZE_OLD;
		} else {
			size = DDR4_SIZE;
		}
		break;
	case DDRC_MSTR_LPDDR3:
		size = LPDDR3_SIZE;
		break;
	case DDRC_MSTR_DDR3:
		size = DDR3_SIZE;
		break;
	default:
		size = 0U;
		break;
	}

	return size;
}

static u64 hif_to_axi_addr(u64 hif_addr)
{
	u64 axi_addr = 0xFFFFFFFFFFFFFFFFULL;

	if (hif_addr < DDR_LO_SIZE) {
		/* HIF address in low DDR address range */
		axi_addr = hif_addr;
	} else if (hif_addr < (DDR_LO_SIZE + DDR_HI_SIZE)) {
		/* HIF address in high DDR address range */
		axi_addr = hif_addr + (DDR_HI_ADDR - DDR_LO_SIZE);
	} else {
		/* For MISRA compliance */
	}

	return axi_addr;
}

static u64 mirrored_r1_addr(void)
{
	u32 bg_b1_pos;
	u64 r1_hif_addr;
	u64 r1_axi_addr;

	/* Read register field */
	bg_b1_pos = Xil_In32(DDRC_ADDRMAP(8U));
	bg_b1_pos &= DDRC_ADDRMAP8_BG_B1_MASK;
	bg_b1_pos >>= DDRC_ADDRMAP8_BG_B1_SHIFT;

	/* Add register base */
	bg_b1_pos += DDRC_ADDRMAP8_BG_B1_BASE;

	/* Add burst line size: 8 bytes, or 3 bit shifts */
	bg_b1_pos += 3U;

	r1_hif_addr = (1ULL << bg_b1_pos);
	r1_axi_addr = hif_to_axi_addr(r1_hif_addr);

	return r1_axi_addr;
}

static s32 store_training_data(void)
{
	u32 size, old_map_offset;
	bool old_mapping;
	u32 haddr, laddr;
	u64 mirr_offset;
	s32 status;

	ddr_rank1_addr(&haddr, &laddr);
	size = ddr_training_size();
	old_mapping = ddr4_is_old_mapping();
	old_map_offset = get_old_map_offset();

	status = PmDmaInit();
	if (XST_SUCCESS != status) {
		goto done;
	}
	PmSetCsuDmaLoopbackMode();

	PmDma64BitTransfer(RESERVED_ADDRESS, 0U, 0U, 0U, size);

	if (((0U != haddr) || (0U != laddr)) && old_mapping) {
		PmDma64BitTransfer(RESERVED_ADDRESS + size, 0U,
				    old_map_offset, 0U, size);

		PmDma64BitTransfer(RESERVED_ADDRESS + (2U * size), 0U, laddr,
				    haddr, size);

		if (0U != IS_ADDR_MIRR()) {
			mirr_offset = mirrored_r1_addr();
			mirr_offset += (((u64)haddr) << 32U);
			mirr_offset += (u64)laddr;
			PmDma64BitTransfer(RESERVED_ADDRESS + (3U * size), 0U,
					   ADDR_LO(mirr_offset),
					   ADDR_HI(mirr_offset),
					   size);
		} else {
			PmDma64BitTransfer(RESERVED_ADDRESS + (3U * size), 0U,
					   laddr + old_map_offset, haddr,
					   size);
		}
	} else if (old_mapping) {
		PmDma64BitTransfer(RESERVED_ADDRESS + size, 0U, old_map_offset,
				   0U, size);
	} else {
		PmDma64BitTransfer(RESERVED_ADDRESS + size, 0U, laddr,
				   haddr, size);
	}

done:
	return status;
}

static void restore_training_data(void)
{
	u32 size, old_map_offset;
	bool old_mapping;
	u32 haddr, laddr;
	u64 mirr_offset;
	s32 status;

	ddr_rank1_addr(&haddr, &laddr);
	size = ddr_training_size();
	old_mapping = ddr4_is_old_mapping();
	old_map_offset = get_old_map_offset();

	status = PmDmaInit();
	if (XST_SUCCESS != status) {
#ifdef DDRSR_DEBUG_STATE
		ddr_print_dbg("DMA initialization failed, error = %x\r\n",
			      status);
#endif
	}
	PmSetCsuDmaLoopbackMode();

	PmDma64BitTransfer(0U, 0U, RESERVED_ADDRESS, 0U, size);

	if (((0U != haddr) || (0U != laddr)) && old_mapping) {
		PmDma64BitTransfer(old_map_offset, 0U,
				   RESERVED_ADDRESS + size, 0U, size);
		PmDma64BitTransfer(laddr, haddr,
				   RESERVED_ADDRESS + (2U * size), 0U, size);
		if (0U != IS_ADDR_MIRR()) {
			mirr_offset = mirrored_r1_addr();
			mirr_offset += (((u64)haddr) << 32U);
			mirr_offset += (u64)laddr;
			PmDma64BitTransfer(ADDR_LO(mirr_offset),
					   ADDR_HI(mirr_offset),
					   RESERVED_ADDRESS + (3U * size), 0U,
					   size);
		} else {
			PmDma64BitTransfer(laddr + old_map_offset, haddr,
					   RESERVED_ADDRESS + (3U * size), 0U,
					   size);
		}
	} else if (old_mapping) {
		PmDma64BitTransfer(old_map_offset, 0U, RESERVED_ADDRESS + size,
				   0U, size);
	} else {
		PmDma64BitTransfer(laddr, haddr, RESERVED_ADDRESS + size, 0U,
				   size);
	}

#ifdef ENABLE_DDR_SR_WR
	/*
	 * Clear ECC error counts.
	 * If ECC is enabled, the act of restoring the corrupted memory locations
	 * will cause ECC errors. That is because the Xil_Out32() function first
	 * reads 8 bytes, then modifies the 4 bytes to be updated, before writing
	 * out all 8 bytes. Since the memory has already been corrupted by
	 * calibration, the initial read will cause ECC errors.
	 */
	if (0U != Xil_In32(DDRC_ERRCNT)) {
		Xil_Out32(DDRC_ERRCLR, 0xfU);
	}
#endif
}

static s32 pm_ddr_sr_enter(void)
{
	s32 ret;

	ret = store_training_data();
	if (XST_SUCCESS != ret) {
		goto err;
	}

	/* Identify if drift is enabled */
	if ((Xil_In32(DDRPHY_DQSDR(0U)) & DDRPHY_DQSDR0_DFTDTEN) != 0U) {
		drift_enable_req = 1U;
	}

	/* disable read and write drift */
	ddr_disable_rd_drift();
	ddr_disable_wr_drift();

	store_state(ctx_ddrc);
	store_state(ctx_ddrphy);
	store_state(ctx_ddrphy_zqdata);
	store_ddrphy_odtcr(ctx_ddrphy_odtcr);

	ret = ddrc_enable_sr();
	if (XST_SUCCESS != ret) {
		goto err;
	}

#ifdef ENABLE_DDR_SR_WR
	/* Set self refresh mode indication flag */
	XPfw_RMW32(XPFW_DDR_STATUS_REGISTER_OFFSET, DDR_STATUS_FLAG_MASK,
		   DDR_STATUS_FLAG_MASK);
	/* Clear DDR controller initialization flag */
	XPfw_RMW32(XPFW_DDR_STATUS_REGISTER_OFFSET, DDRC_INIT_FLAG_MASK,
		   ~DDRC_INIT_FLAG_MASK);
#endif
err:
	return ret;
}

static void pm_ddr_sr_exit(bool ddrss_is_reset)
{
	if (true == ddrss_is_reset) {
		u32 readVal;

		/* re-enable clock only if FPD was off */
		ddr_clock_enable();

		Xil_Out32(DDRC_SWCTL, 0U);
		restore_state(ctx_ddrc);

		readVal = Xil_In32(CRF_APB_RST_DDR_SS);
		readVal &= ~CRF_APB_RST_DDR_SS_DDR_RESET_MASK;
		Xil_Out32(CRF_APB_RST_DDR_SS, readVal);

		restore_state(ctx_ddrphy);
	}

	DDR_reinit(ddrss_is_reset);

	restore_training_data();
}

#ifdef ENABLE_DDR_SR_WR
s32 PmDdrEnterSr(void)
{
	s32 status = XST_FAILURE;

	if (pmSlaveDdr_g.node.currState == PM_DDR_STATE_OFF) {
		/* DDR is OFF, do not enter into self refresh mode */
		goto err;
	} else if (pmSlaveDdr_g.node.currState == PM_DDR_STATE_SR) {
		/* DDR is already in self refresh mode */
		status = XST_SUCCESS;
		goto err;
	}

	XPfw_AibEnable(XPFW_AIB_LPD_TO_DDR);
	status = pm_ddr_sr_enter();
	if (XST_SUCCESS != status) {
		goto err;
	}

	ddr_io_retention_set(true);

err:
	return status;
}

s32 PmDdrExitSr(void)
{
	s32 Status;

	/* Wait until FSBL initialize DDR controller */
	Status = XPfw_UtilPollForMask(XPFW_DDR_STATUS_REGISTER_OFFSET,
				      DDRC_INIT_FLAG_MASK,
				      DDR_FLAG_POLL_PERIOD);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Read DDRC & DDR PHY register values and modify some bitfields */
	store_state(ctx_ddrc);
	store_state(ctx_ddrphy);

	XPfw_RMW32(CRF_APB_RST_DDR_SS, CRF_APB_RST_DDR_SS_DDR_RESET_MASK,
		   CRF_APB_RST_DDR_SS_DDR_RESET_MASK);

	/* Write modified values to DDRC registers */
	restore_state(ctx_ddrc);

	XPfw_RMW32(CRF_APB_RST_DDR_SS, CRF_APB_RST_DDR_SS_DDR_RESET_MASK,
		   ~CRF_APB_RST_DDR_SS_DDR_RESET_MASK);

	/* Write modified values to DDR PHY registers */
	restore_state(ctx_ddrphy);

	DDR_reinit(true);

	restore_training_data();

	/* Indication to FSBL that DDR is out of self refresh mode */
	XPfw_RMW32(XPFW_DDR_STATUS_REGISTER_OFFSET, DDR_STATUS_FLAG_MASK,
		   ~DDR_STATUS_FLAG_MASK);

done:
	return Status;
}
#endif

/**
 * PmDdrFsmHandler() - DDR FSM handler, performs transition actions
 * @slave       Slave whose state should be changed (pointer to DDR object)
 * @nextState   State the slave should enter
 *
 * @return      Status of performing transition action
 */
static s32 PmDdrFsmHandler(PmSlave* const slave, const PmStateId nextState)
{
	s32 status = XST_SUCCESS;

	/* Handle transition to OFF state here */
	if ((PM_DDR_STATE_OFF != slave->node.currState) &&
	    (PM_DDR_STATE_OFF == nextState)) {
		/* Here, user can put the DDR controller in reset */
		XPfw_AibEnable(XPFW_AIB_LPD_TO_DDR);
		goto done;
	}

	switch (slave->node.currState) {
	case PM_DDR_STATE_ON:
		if (PM_DDR_STATE_SR == nextState) {
			XPfw_AibEnable(XPFW_AIB_LPD_TO_DDR);
			status = pm_ddr_sr_enter();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_DDR_STATE_SR:
		if (PM_DDR_STATE_ON == nextState) {
			bool ddrss_is_reset = ((0U == Xil_In32(DDRC_STAT)) ? true : false);
			pm_ddr_sr_exit(ddrss_is_reset);
			XPfw_AibDisable(XPFW_AIB_LPD_TO_DDR);
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_DDR_STATE_OFF:
		if (PM_DDR_STATE_ON == nextState) {
			/*
			 * Bring DDR controller out of reset if it was in reset
			 * during DDR OFF state
			 * */
			XPfw_AibDisable(XPFW_AIB_LPD_TO_DDR);
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	default:
		status = XST_PM_INTERNAL;
		PmNodeLogUnknownState(&slave->node, slave->node.currState);
		break;
	}

done:
	return status;
}

#ifdef ENABLE_POS
/**
 * PmDdrPowerOffSuspendResume() - Take DDR out of self refresh after resume from
 * 				  Power Off Suspend
 *
 * @return      XST_SUCCESS if DDR is resumed, failure code otherwise
 */
s32 PmDdrPowerOffSuspendResume(void)
{
	s32 status;

	PmClockRestoreDdr();

	status = PmDdrFsmHandler(&pmSlaveDdr_g, PM_DDR_STATE_ON);
	if (XST_SUCCESS != status) {
		goto done;
	}
	pmSlaveDdr_g.node.flags = NODE_LOCKED_CLOCK_FLAG |
					NODE_LOCKED_POWER_FLAG;

done:
	return status;
}
#endif

#endif

void ddr_io_prepare(void)
{
#ifdef XPAR_DDRCPSU_0_DEVICE_ID
	ddr_power_down_io();
	ddr_io_retention_set(true);
#endif
}

/* DDR FSM */
static const PmSlaveFsm pmSlaveDdrFsm = {
	.states = pmDdrStates,
	.statesCnt = PM_DDR_STATE_MAX,
	.trans = pmDdrTransitions,
	.transCnt = ARRAY_SIZE(pmDdrTransitions),
#ifdef XPAR_DDRCPSU_0_DEVICE_ID
	.enterState = PmDdrFsmHandler,
#else
	.enterState = NULL,
#endif
};

static u8 PmDdrPowerConsumptions[] = {
	DEFAULT_DDR_POWER_OFF,
	DEFAULT_DDR_POWER_SR,
	DEFAULT_DDR_POWER_ON,
};

PmSlave pmSlaveDdr_g __attribute__((__section__(".srdata"))) = {
	.node = {
		.derived = &pmSlaveDdr_g,
		.nodeId = NODE_DDR,
		.class = &pmNodeClassSlave_g,
		.parent = &pmPowerDomainFpd_g.power,
		.clocks = NULL,
		.currState = PM_DDR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmDdrPowerConsumptions),
		DEFINE_NODE_NAME("ddr"),
	},
	.class = NULL,
	.reqs = NULL,
	.wake = NULL,
	.slvFsm = &pmSlaveDdrFsm,
	.flags = 0U,
};
#endif
