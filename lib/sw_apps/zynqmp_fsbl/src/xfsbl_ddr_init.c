/******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

/*****************************************************************************/
/**
 *
 * @file xfsbl_ddr_init.c
 *
 * This is the file which contains re-initialization code for the DDR. This
 * code is used only for ZCU102 and ZCU106 boards. The DDR part (MTA8ATF51264HZ)
 * on these boards have changed to a newer DDR part (MTA4ATF51264HZ) which has
 * different configuration used than the earlier one.
 *
 * This code will identify if the DDR4 DIMM part used is MTA8ATF51264HZ or
 * MTA4ATF51264HZ. If it detects that the DIMM part used is the older one, it
 * will simply return from here, otherwise this will do the complete
 * re-initialization for the New DIMM part. This code is added in 2018.3 for
 * supporting the two DIMMs mentioned above on ZCU102 and ZCU106 and
 * going ahead this will likely be replaced by a more generic solution in
 * future releases.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   mn   07/06/18 Add DDR initialization support for new DDR DIMM part
 *       mn   07/18/18 Move iicps.h inclusion under ZCU102 and ZCU106 macro
 *                     checks
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"

#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)
#include "xiicps.h"
#include "xfsbl_ddr_init.h"

/************************** Constant Definitions *****************************/

/* Clock Period, This is used for calculation of timing parameter values */
#define XFSBL_CLOCK_PERIOD	(1000.0 / XPAR_PSU_DDRC_0_DDR_FREQ_MHZ)

/* All the parameter macros defined below are for
 * the New DIMM (MTA4ATF51264HZ)
 */
/* IC Width of the DRAM for new DIMM
 * 0x1 - x8 Device
 * 0x2 - x16 Device
 * 0x3 - x32 Device
 *
 * New DIMM (MTA4ATF51264HZ) is having IC width of x16 as defined in
 * the data-sheet.
 */
#define XFSBL_DRAM_IC_WIDTH				0x2U
/* Minimum Refresh Recovery Delay(ns) for new DIMM, read from SPD */
#define XFSBL_T_RFC					350.0
/* Minimum Refresh Recovery Delay(ns) (X4 mode) for new DIMM, read from SPD */
#define XFSBL_T_RFC4					160.0
/* Minimum four Activate Window Delay(ns) for new DIMM, read from SPD */
#define XFSBL_T_FAW					21.0
/* Minimum Active to Active/Refresh Delay(ns) for new DIMM, read from SPD */
#define XFSBL_T_RC					46.5
/* Minimum RAS to RAS Delay(ns), same bank group for new DIMM, read from SPD */
#define XFSBL_T_RRDL					6.4
/* Minimum RAS to RAS Delay(ns), different bank group for new DIMM */
#define XFSBL_T_RRDS					5.3
/* Number of Column bits used for new DIMM */
#define XFSBL_COL_WIDTH					10U
/* Number of Bank bits used for new DIMM */
#define XFSBL_BANK_WIDTH				2U
/* Number of Bank Group bits used for new DIMM */
#define XFSBL_BG_WIDTH					1U
/* Number of Row bits used for new DIMM */
#define XFSBL_ROW_WIDTH					16U
/* Number of Rank bits used for new DIMM */
#define XFSBL_RANK_WIDTH				0U

/* DDR Controller Registers Offsets */
#define DDRC_REG_OFFSET {\
	DDRC_MSTR_OFFSET,                       /* Offset 0 */\
	DDRC_MRCTRL0_OFFSET,                    /* Offset 1 */\
	DDRC_DERATEEN_OFFSET,                   /* Offset 2 */\
	DDRC_DERATEINT_OFFSET,                  /* Offset 3 */\
	DDRC_PWRCTL_OFFSET,                     /* Offset 4 */\
	DDRC_PWRTMG_OFFSET,                     /* Offset 5 */\
	DDRC_RFSHCTL0_OFFSET,                   /* Offset 6 */\
	DDRC_RFSHCTL1_OFFSET,                   /* Offset 7 */\
	DDRC_RFSHCTL3_OFFSET,                   /* Offset 8 */\
	DDRC_RFSHTMG_OFFSET,                    /* Offset 9 */\
	DDRC_ECCCFG0_OFFSET,                    /* Offset 10 */\
	DDRC_ECCCFG1_OFFSET,                    /* Offset 11 */\
	DDRC_CRCPARCTL1_OFFSET,                 /* Offset 12 */\
	DDRC_CRCPARCTL2_OFFSET,                 /* Offset 13 */\
	DDRC_INIT0_OFFSET,                      /* Offset 14 */\
	DDRC_INIT1_OFFSET,                      /* Offset 15 */\
	DDRC_INIT2_OFFSET,                      /* Offset 16 */\
	DDRC_INIT3_OFFSET,                      /* Offset 17 */\
	DDRC_INIT4_OFFSET,                      /* Offset 18 */\
	DDRC_INIT5_OFFSET,                      /* Offset 19 */\
	DDRC_INIT6_OFFSET,                      /* Offset 20 */\
	DDRC_INIT7_OFFSET,                      /* Offset 21 */\
	DDRC_DIMMCTL_OFFSET,                    /* Offset 22 */\
	DDRC_RANKCTL_OFFSET,                    /* Offset 23 */\
	DDRC_DRAMTMG0_OFFSET,                   /* Offset 24 */\
	DDRC_DRAMTMG1_OFFSET,                   /* Offset 25 */\
	DDRC_DRAMTMG2_OFFSET,                   /* Offset 26 */\
	DDRC_DRAMTMG3_OFFSET,                   /* Offset 27 */\
	DDRC_DRAMTMG4_OFFSET,                   /* Offset 28 */\
	DDRC_DRAMTMG5_OFFSET,                   /* Offset 29 */\
	DDRC_DRAMTMG6_OFFSET,                   /* Offset 30 */\
	DDRC_DRAMTMG7_OFFSET,                   /* Offset 31 */\
	DDRC_DRAMTMG8_OFFSET,                   /* Offset 32 */\
	DDRC_DRAMTMG9_OFFSET,                   /* Offset 33 */\
	DDRC_DRAMTMG11_OFFSET,                  /* Offset 34 */\
	DDRC_DRAMTMG12_OFFSET,                  /* Offset 35 */\
	DDRC_ZQCTL0_OFFSET,                     /* Offset 36 */\
	DDRC_ZQCTL1_OFFSET,                     /* Offset 37 */\
	DDRC_DFITMG0_OFFSET,                    /* Offset 38 */\
	DDRC_DFITMG1_OFFSET,                    /* Offset 39 */\
	DDRC_DFILPCFG0_OFFSET,                  /* Offset 40 */\
	DDRC_DFILPCFG1_OFFSET,                  /* Offset 41 */\
	DDRC_DFIUPD0_OFFSET,                    /* Offset 42 */\
	DDRC_DFIUPD1_OFFSET,                    /* Offset 43 */\
	DDRC_DFIMISC_OFFSET,                    /* Offset 44 */\
	DDRC_DFITMG2_OFFSET,                    /* Offset 45 */\
	DDRC_DBICTL_OFFSET,                     /* Offset 46 */\
	DDRC_ADDRMAP0_OFFSET,                   /* Offset 47 */\
	DDRC_ADDRMAP1_OFFSET,                   /* Offset 48 */\
	DDRC_ADDRMAP2_OFFSET,                   /* Offset 49 */\
	DDRC_ADDRMAP3_OFFSET,                   /* Offset 50 */\
	DDRC_ADDRMAP4_OFFSET,                   /* Offset 51 */\
	DDRC_ADDRMAP5_OFFSET,                   /* Offset 52 */\
	DDRC_ADDRMAP6_OFFSET,                   /* Offset 53 */\
	DDRC_ADDRMAP7_OFFSET,                   /* Offset 54 */\
	DDRC_ADDRMAP8_OFFSET,                   /* Offset 55 */\
	DDRC_ADDRMAP9_OFFSET,                   /* Offset 56 */\
	DDRC_ADDRMAP10_OFFSET,                  /* Offset 57 */\
	DDRC_ADDRMAP11_OFFSET,                  /* Offset 58 */\
	DDRC_ODTCFG_OFFSET,                     /* Offset 59 */\
	DDRC_ODTMAP_OFFSET,                     /* Offset 60 */\
	DDRC_SCHED_OFFSET,                      /* Offset 61 */\
	DDRC_PERFLPR1_OFFSET,                   /* Offset 62 */\
	DDRC_PERFWR1_OFFSET,                    /* Offset 63 */\
	DDRC_DQMAP0_OFFSET,                     /* Offset 64 */\
	DDRC_DQMAP1_OFFSET,                     /* Offset 65 */\
	DDRC_DQMAP2_OFFSET,                     /* Offset 66 */\
	DDRC_DQMAP3_OFFSET,                     /* Offset 67 */\
	DDRC_DQMAP4_OFFSET,                     /* Offset 68 */\
	DDRC_DQMAP5_OFFSET,                     /* Offset 69 */\
	DDRC_DBG0_OFFSET,                       /* Offset 70 */\
	DDRC_DBGCMD_OFFSET,                     /* Offset 71 */\
	DDRC_SWCTL_OFFSET,                      /* Offset 72 */\
	DDRC_PCCFG_OFFSET,                      /* Offset 73 */\
	DDRC_PCFGR_0_OFFSET,                    /* Offset 74 */\
	DDRC_PCFGW_0_OFFSET,                    /* Offset 75 */\
	DDRC_PCTRL_0_OFFSET,                    /* Offset 76 */\
	DDRC_PCFGQOS0_0_OFFSET,                 /* Offset 77 */\
	DDRC_PCFGQOS1_0_OFFSET,                 /* Offset 78 */\
	DDRC_PCFGR_1_OFFSET,                    /* Offset 79 */\
	DDRC_PCFGW_1_OFFSET,                    /* Offset 80 */\
	DDRC_PCTRL_1_OFFSET,                    /* Offset 81 */\
	DDRC_PCFGQOS0_1_OFFSET,                 /* Offset 82 */\
	DDRC_PCFGQOS1_1_OFFSET,                 /* Offset 83 */\
	DDRC_PCFGR_2_OFFSET,                    /* Offset 84 */\
	DDRC_PCFGW_2_OFFSET,                    /* Offset 85 */\
	DDRC_PCTRL_2_OFFSET,                    /* Offset 86 */\
	DDRC_PCFGQOS0_2_OFFSET,                 /* Offset 87 */\
	DDRC_PCFGQOS1_2_OFFSET,                 /* Offset 88 */\
	DDRC_PCFGR_3_OFFSET,                    /* Offset 89 */\
	DDRC_PCFGW_3_OFFSET,                    /* Offset 90 */\
	DDRC_PCTRL_3_OFFSET,                    /* Offset 91 */\
	DDRC_PCFGQOS0_3_OFFSET,                 /* Offset 92 */\
	DDRC_PCFGQOS1_3_OFFSET,                 /* Offset 93 */\
	DDRC_PCFGWQOS0_3_OFFSET,                /* Offset 94 */\
	DDRC_PCFGWQOS1_3_OFFSET,                /* Offset 95 */\
	DDRC_PCFGR_4_OFFSET,                    /* Offset 96 */\
	DDRC_PCFGW_4_OFFSET,                    /* Offset 97 */\
	DDRC_PCTRL_4_OFFSET,                    /* Offset 98 */\
	DDRC_PCFGQOS0_4_OFFSET,                 /* Offset 99 */\
	DDRC_PCFGQOS1_4_OFFSET,                 /* Offset 100 */\
	DDRC_PCFGWQOS0_4_OFFSET,                /* Offset 101 */\
	DDRC_PCFGWQOS1_4_OFFSET,                /* Offset 102 */\
	DDRC_PCFGR_5_OFFSET,                    /* Offset 103 */\
	DDRC_PCFGW_5_OFFSET,                    /* Offset 104 */\
	DDRC_PCTRL_5_OFFSET,                    /* Offset 105 */\
	DDRC_PCFGQOS0_5_OFFSET,                 /* Offset 106 */\
	DDRC_PCFGQOS1_5_OFFSET,                 /* Offset 107 */\
	DDRC_PCFGWQOS0_5_OFFSET,                /* Offset 108 */\
	DDRC_PCFGWQOS1_5_OFFSET,                /* Offset 109 */\
	DDRC_SARBASE0_OFFSET,                   /* Offset 110 */\
	DDRC_SARSIZE0_OFFSET,                   /* Offset 111 */\
	DDRC_SARBASE1_OFFSET,                   /* Offset 112 */\
	DDRC_SARSIZE1_OFFSET,                   /* Offset 113 */\
	DDRC_DFITMG0_SHADOW_OFFSET,             /* Offset 114 */\
}

/* DDR PHY Registers Offsets */
#define PHY_REG_OFFSET {\
	DDR_PHY_PGCR0_OFFSET,			/* Offset 0 */\
	DDR_PHY_PGCR2_OFFSET,                   /* Offset 1 */\
	DDR_PHY_PGCR3_OFFSET,                   /* Offset 2 */\
	DDR_PHY_PGCR5_OFFSET,                   /* Offset 3 */\
	DDR_PHY_PTR0_OFFSET,                    /* Offset 4 */\
	DDR_PHY_PTR1_OFFSET,                    /* Offset 5 */\
	DDR_PHY_PLLCR0_OFFSET,                  /* Offset 6 */\
	DDR_PHY_DSGCR_OFFSET,                   /* Offset 7 */\
	DDR_PHY_GPR0_OFFSET,                    /* Offset 8 */\
	DDR_PHY_DCR_OFFSET,                     /* Offset 9 */\
	DDR_PHY_DTPR0_OFFSET,                   /* Offset 10 */\
	DDR_PHY_DTPR1_OFFSET,                   /* Offset 11 */\
	DDR_PHY_DTPR2_OFFSET,                   /* Offset 12 */\
	DDR_PHY_DTPR3_OFFSET,                   /* Offset 13 */\
	DDR_PHY_DTPR4_OFFSET,                   /* Offset 14 */\
	DDR_PHY_DTPR5_OFFSET,                   /* Offset 15 */\
	DDR_PHY_DTPR6_OFFSET,                   /* Offset 16 */\
	DDR_PHY_RDIMMGCR0_OFFSET,               /* Offset 17 */\
	DDR_PHY_RDIMMGCR1_OFFSET,               /* Offset 18 */\
	DDR_PHY_RDIMMCR0_OFFSET,                /* Offset 19 */\
	DDR_PHY_RDIMMCR1_OFFSET,                /* Offset 20 */\
	DDR_PHY_MR0_OFFSET,                     /* Offset 21 */\
	DDR_PHY_MR1_OFFSET,                     /* Offset 22 */\
	DDR_PHY_MR2_OFFSET,                     /* Offset 23 */\
	DDR_PHY_MR3_OFFSET,                     /* Offset 24 */\
	DDR_PHY_MR4_OFFSET,                     /* Offset 25 */\
	DDR_PHY_MR5_OFFSET,                     /* Offset 26 */\
	DDR_PHY_MR6_OFFSET,                     /* Offset 27 */\
	DDR_PHY_MR11_OFFSET,                    /* Offset 28 */\
	DDR_PHY_MR12_OFFSET,                    /* Offset 29 */\
	DDR_PHY_MR13_OFFSET,                    /* Offset 30 */\
	DDR_PHY_MR14_OFFSET,                    /* Offset 31 */\
	DDR_PHY_MR22_OFFSET,                    /* Offset 32 */\
	DDR_PHY_DTCR0_OFFSET,                   /* Offset 33 */\
	DDR_PHY_DTCR1_OFFSET,                   /* Offset 34 */\
	DDR_PHY_CATR0_OFFSET,                   /* Offset 35 */\
	DDR_PHY_DQSDR0_OFFSET,                  /* Offset 36 */\
	DDR_PHY_BISTLSR_OFFSET,                 /* Offset 37 */\
	DDR_PHY_RIOCR5_OFFSET,                  /* Offset 38 */\
	DDR_PHY_ACIOCR0_OFFSET,                 /* Offset 39 */\
	DDR_PHY_ACIOCR2_OFFSET,                 /* Offset 40 */\
	DDR_PHY_ACIOCR3_OFFSET,                 /* Offset 41 */\
	DDR_PHY_ACIOCR4_OFFSET,                 /* Offset 42 */\
	DDR_PHY_IOVCR0_OFFSET,                  /* Offset 43 */\
	DDR_PHY_VTCR0_OFFSET,                   /* Offset 44 */\
	DDR_PHY_VTCR1_OFFSET,                   /* Offset 45 */\
	DDR_PHY_ACBDLR1_OFFSET,                 /* Offset 46 */\
	DDR_PHY_ACBDLR2_OFFSET,                 /* Offset 47 */\
	DDR_PHY_ACBDLR6_OFFSET,                 /* Offset 48 */\
	DDR_PHY_ACBDLR7_OFFSET,                 /* Offset 49 */\
	DDR_PHY_ACBDLR8_OFFSET,                 /* Offset 50 */\
	DDR_PHY_ACBDLR9_OFFSET,                 /* Offset 51 */\
	DDR_PHY_ZQCR_OFFSET,                    /* Offset 52 */\
	DDR_PHY_ZQ0PR0_OFFSET,                  /* Offset 53 */\
	DDR_PHY_ZQ0OR0_OFFSET,                  /* Offset 54 */\
	DDR_PHY_ZQ0OR1_OFFSET,                  /* Offset 55 */\
	DDR_PHY_ZQ1PR0_OFFSET,                  /* Offset 56 */\
	DDR_PHY_DX0GCR0_OFFSET,                 /* Offset 57 */\
	DDR_PHY_DX0GCR4_OFFSET,                 /* Offset 58 */\
	DDR_PHY_DX0GCR5_OFFSET,                 /* Offset 59 */\
	DDR_PHY_DX0GCR6_OFFSET,                 /* Offset 60 */\
	DDR_PHY_DX1GCR0_OFFSET,                 /* Offset 61 */\
	DDR_PHY_DX1GCR4_OFFSET,                 /* Offset 62 */\
	DDR_PHY_DX1GCR5_OFFSET,                 /* Offset 63 */\
	DDR_PHY_DX1GCR6_OFFSET,                 /* Offset 64 */\
	DDR_PHY_DX2GCR0_OFFSET,                 /* Offset 65 */\
	DDR_PHY_DX2GCR1_OFFSET,                 /* Offset 66 */\
	DDR_PHY_DX2GCR4_OFFSET,                 /* Offset 67 */\
	DDR_PHY_DX2GCR5_OFFSET,                 /* Offset 68 */\
	DDR_PHY_DX2GCR6_OFFSET,                 /* Offset 69 */\
	DDR_PHY_DX3GCR0_OFFSET,                 /* Offset 70 */\
	DDR_PHY_DX3GCR1_OFFSET,                 /* Offset 71 */\
	DDR_PHY_DX3GCR4_OFFSET,                 /* Offset 72 */\
	DDR_PHY_DX3GCR5_OFFSET,                 /* Offset 73 */\
	DDR_PHY_DX3GCR6_OFFSET,                 /* Offset 74 */\
	DDR_PHY_DX4GCR0_OFFSET,                 /* Offset 75 */\
	DDR_PHY_DX4GCR1_OFFSET,                 /* Offset 76 */\
	DDR_PHY_DX4GCR4_OFFSET,                 /* Offset 77 */\
	DDR_PHY_DX4GCR5_OFFSET,                 /* Offset 78 */\
	DDR_PHY_DX4GCR6_OFFSET,                 /* Offset 79 */\
	DDR_PHY_DX5GCR0_OFFSET,                 /* Offset 80 */\
	DDR_PHY_DX5GCR1_OFFSET,                 /* Offset 81 */\
	DDR_PHY_DX5GCR4_OFFSET,                 /* Offset 82 */\
	DDR_PHY_DX5GCR5_OFFSET,                 /* Offset 83 */\
	DDR_PHY_DX5GCR6_OFFSET,                 /* Offset 84 */\
	DDR_PHY_DX6GCR0_OFFSET,                 /* Offset 85 */\
	DDR_PHY_DX6GCR1_OFFSET,                 /* Offset 86 */\
	DDR_PHY_DX6GCR4_OFFSET,                 /* Offset 87 */\
	DDR_PHY_DX6GCR5_OFFSET,                 /* Offset 88 */\
	DDR_PHY_DX6GCR6_OFFSET,                 /* Offset 89 */\
	DDR_PHY_DX7GCR0_OFFSET,                 /* Offset 90 */\
	DDR_PHY_DX7GCR1_OFFSET,                 /* Offset 91 */\
	DDR_PHY_DX7GCR4_OFFSET,                 /* Offset 92 */\
	DDR_PHY_DX7GCR5_OFFSET,                 /* Offset 93 */\
	DDR_PHY_DX7GCR6_OFFSET,                 /* Offset 94 */\
	DDR_PHY_DX8GCR0_OFFSET,                 /* Offset 95 */\
	DDR_PHY_DX8GCR1_OFFSET,                 /* Offset 96 */\
	DDR_PHY_DX8GCR4_OFFSET,                 /* Offset 97 */\
	DDR_PHY_DX8GCR5_OFFSET,                 /* Offset 98 */\
	DDR_PHY_DX8GCR6_OFFSET,                 /* Offset 99 */\
	DDR_PHY_DX8SL0OSC_OFFSET,               /* Offset 100 */\
	DDR_PHY_DX8SL0PLLCR0_OFFSET,            /* Offset 101 */\
	DDR_PHY_DX8SL0DQSCTL_OFFSET,            /* Offset 102 */\
	DDR_PHY_DX8SL0DXCTL2_OFFSET,            /* Offset 103 */\
	DDR_PHY_DX8SL0IOCR_OFFSET,              /* Offset 104 */\
	DDR_PHY_DX8SL1OSC_OFFSET,               /* Offset 105 */\
	DDR_PHY_DX8SL1PLLCR0_OFFSET,            /* Offset 106 */\
	DDR_PHY_DX8SL1DQSCTL_OFFSET,            /* Offset 107 */\
	DDR_PHY_DX8SL1DXCTL2_OFFSET,            /* Offset 108 */\
	DDR_PHY_DX8SL1IOCR_OFFSET,              /* Offset 109 */\
	DDR_PHY_DX8SL2OSC_OFFSET,               /* Offset 110 */\
	DDR_PHY_DX8SL2PLLCR0_OFFSET,            /* Offset 111 */\
	DDR_PHY_DX8SL2DQSCTL_OFFSET,            /* Offset 112 */\
	DDR_PHY_DX8SL2DXCTL2_OFFSET,            /* Offset 113 */\
	DDR_PHY_DX8SL2IOCR_OFFSET,              /* Offset 114 */\
	DDR_PHY_DX8SL3OSC_OFFSET,               /* Offset 115 */\
	DDR_PHY_DX8SL3PLLCR0_OFFSET,            /* Offset 116 */\
	DDR_PHY_DX8SL3DQSCTL_OFFSET,            /* Offset 117 */\
	DDR_PHY_DX8SL3DXCTL2_OFFSET,            /* Offset 118 */\
	DDR_PHY_DX8SL3IOCR_OFFSET,              /* Offset 119 */\
	DDR_PHY_DX8SL4OSC_OFFSET,               /* Offset 120 */\
	DDR_PHY_DX8SL4PLLCR0_OFFSET,            /* Offset 121 */\
	DDR_PHY_DX8SL4DQSCTL_OFFSET,            /* Offset 122 */\
	DDR_PHY_DX8SL4DXCTL2_OFFSET,            /* Offset 123 */\
	DDR_PHY_DX8SL4IOCR_OFFSET,              /* Offset 124 */\
	DDR_PHY_DX8SLBPLLCR0_OFFSET,            /* Offset 125 */\
	DDR_PHY_DX8SLBDQSCTL_OFFSET,            /* Offset 126 */\
}

/* Column offset value used for HIF calculation */
#define XFSBL_HIF_COLUMN(x)				(100U + (x))
/* Row offset value used for HIF calculation */
#define XFSBL_HIF_ROW(x)				(200U + (x))
/* Bank offset value used for HIF calculation */
#define XFSBL_HIF_BANK(x)				(300U + (x))
/* Bank Group offset value used for HIF calculation */
#define XFSBL_HIF_BG(x)					(400U + (x))
/* Rank offset value used for HIF calculation */
#define XFSBL_HIF_RANK(x)				(500U + (x))

/* IIC Serial Clock rate */
#define XFSBL_IIC_SCLK_RATE		100000U
/* IIC Mux Address */
#define XFSBL_MUX_ADDR			0x75U
/* SODIMM Slave Address */
#define XFSBL_SODIMM_SLAVE_ADDR		0x51U
/* SODIMM Control Address */
#define XFSBL_SODIMM_CONTROL_ADDR	0x37U
/* IIC Bus Idle Timeout */
#define XFSBL_IIC_BUS_TIMEOUT		1000000U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function returns the next integer value of the current float value
 *
 * @param	Num is the float number
 *
 * @return	returns the next Integer value
 *
 *****************************************************************************/
static s32 XFsbl_Ceil(float Num)
{
	s32 Inum = (s32)Num;

	if (Num != (float)Inum) {
		Inum += 1U;
	}

	return Inum;
}

/*****************************************************************************/
/**
 * This function calculates the HIF Addresses for default mode
 *
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcHifAddr(u32 *HifAddr)
{
	u32 Position = 0U;
	u32 Index;

	/* Define Column positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_COL_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_COLUMN(Index);
		Position++;
	}

	/* Define Bank Group positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_BG_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_BG(Index);
		Position++;
	}

	/* Define Bank positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_BANK_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_BANK(Index);
		Position++;
	}

	/* Define Row positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_ROW_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_ROW(Index);
		Position++;
	}

	/* Define Rank positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_RANK_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_RANK(Index);
		Position++;
	}
}

/*****************************************************************************/
/**
 * This function calculates the HIF Addresses for Video mapping mode
 *
 * @param	HifAddr is the pointer to the HIF Addresses Array
 * @param	VideoBuf is size of Video Buffer used in the design
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcHifAddrVideo(u32 *HifAddr, u32 VideoBuf)
{
	u32 Position;
	u32 Index;
	u32 BufWidth;
	u32 RemainingRow;

	/* Calculate Buffer Width based on Video Buffer Size */
	for (Index = 0U; Index <= 6U; Index++) {
		if (((u32)1 << Index) == VideoBuf) {
			BufWidth = Index + 20U;
			break;
		} else {
			BufWidth = 20U;
		}
	}

	/* First four HIF addresses are fixed for Video Mapping */
	HifAddr[0] = XFSBL_HIF_COLUMN(0U);
	HifAddr[1] = XFSBL_HIF_COLUMN(1U);
	HifAddr[2] = XFSBL_HIF_COLUMN(2U);
	HifAddr[3] = XFSBL_HIF_BG(0U);

	Position = 4U;
	/* Define Column positions in HIF Addresses */
	for (Index = 3U; Index < XFSBL_COL_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_COLUMN(Index);
		Position++;
	}

	/* Define Row positions in HIF Addresses */
	for (Index = 0U; Index < (BufWidth - XFSBL_COL_WIDTH - 4U); Index++) {
		HifAddr[Position] = XFSBL_HIF_ROW(Index);
		Position++;
	}

	/* Define Bank positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_BANK_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_BANK(Index);
		Position++;
	}

	/* Define Remaining Row positions in HIF Addresses */
	RemainingRow = XFSBL_ROW_WIDTH - (BufWidth - XFSBL_COL_WIDTH - 4U);
	for (Index = 0U; Index < RemainingRow; Index++) {
		HifAddr[Position] = XFSBL_HIF_ROW(Index) + (BufWidth -
				XFSBL_COL_WIDTH - 4U);
		Position++;
	}

	/* Define Rank positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_RANK_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_RANK(Index);
		Position++;
	}
}

/*****************************************************************************/
/**
 * This function calculates the HIF Addresses for Bank-Row-Column mapping mode
 *
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcHifAddrBrcMap(u32 *HifAddr)
{
	u32 Position;
	u32 Index;

	/* First four HIF addresses are fixed for BRC Mapping */
	HifAddr[0] = XFSBL_HIF_COLUMN(0U);
	HifAddr[1] = XFSBL_HIF_COLUMN(1U);
	HifAddr[2] = XFSBL_HIF_COLUMN(2U);
	HifAddr[3] = XFSBL_HIF_BG(0U);

	Position = 4U;
	/* Define Column positions in HIF Addresses */
	for (Index = 3U; Index < XFSBL_COL_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_COLUMN(Index);
		Position++;
	}

	/* Define Row positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_ROW_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_ROW(Index);
		Position++;
	}

	/* Define Bank positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_BANK_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_BANK(Index);
		Position++;
	}

	/* Define Rank positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_RANK_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_RANK(Index);
		Position++;
	}
}

/*****************************************************************************/
/**
 * This function calculates the HIF Addresses for Address Mapping Enabled mode
 *
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcHifAddrMemMap(u32 *HifAddr)
{
	u32 Position;
	u32 Index;

	/* First four HIF addresses are fixed for DDR4 Address Mapped Mode */
	HifAddr[0] = XFSBL_HIF_COLUMN(0U);
	HifAddr[1] = XFSBL_HIF_COLUMN(1U);
	HifAddr[2] = XFSBL_HIF_COLUMN(2U);
	HifAddr[3] = XFSBL_HIF_BG(0U);

	Position = 4U;
	/* Define Column positions in HIF Addresses */
	for (Index = 3U; Index < XFSBL_COL_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_COLUMN(Index);
		Position++;
	}

	/* Define Bank positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_BANK_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_BANK(Index);
		Position++;
	}

	/* Define Row positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_ROW_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_ROW(Index);
		Position++;
	}

	/* Define Rank positions in HIF Addresses */
	for (Index = 0U; Index < XFSBL_RANK_WIDTH; Index++) {
		HifAddr[Position] = XFSBL_HIF_RANK(Index);
		Position++;
	}
}

/*****************************************************************************/
/**
 * This function calculates the Bank Address Map based on HIF Addresses
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcBankAddr(struct DdrcInitData *DdrDataPtr, u32 *HifAddr)
{
	u32 Index;
	u32 BankBit;

	/* Calculate Bank Addresses */
	for (BankBit = 0U; BankBit < XFSBL_MAX_BANKS; BankBit++) {
		if (BankBit < XFSBL_BANK_WIDTH) {
			Index = 0U;

			while (HifAddr[Index] != XFSBL_HIF_BANK(BankBit)) {
				Index++;
			}

			DdrDataPtr->AddrMapBankBit[BankBit] = Index -
				(BankBit + 2U);
		} else {
			DdrDataPtr->AddrMapBankBit[BankBit] = 0x1FU;
		}
	}
}

/*****************************************************************************/
/**
 * This function calculates the Bank Group Address Map based on HIF Addresses
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcBgAddr(struct DdrcInitData *DdrDataPtr, u32 *HifAddr)
{
	u32 Index;
	u32 BgBit;

	/* Calculate Bank Group Addresses */
	for (BgBit = 0U; BgBit < XFSBL_MAX_BANK_GROUPS; BgBit++) {
		if (BgBit < XFSBL_BG_WIDTH) {
			Index = 0U;

			while (HifAddr[Index] != XFSBL_HIF_BG(BgBit)) {
				Index++;
			}

			if (Index >= (BgBit + 2U)) {
				DdrDataPtr->AddrMapBgBit[BgBit] = Index -
					(BgBit + 2U);
			} else {
				DdrDataPtr->AddrMapBgBit[BgBit] = 0U;
			}
		} else {
			DdrDataPtr->AddrMapBgBit[BgBit] = 0x1FU;
		}
	}
}

/*****************************************************************************/
/**
 * This function calculates the Column Address Map based on HIF Addresses
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcColAddr(struct DdrcInitData *DdrDataPtr, u32 *HifAddr)
{
	u32 Index;
	u32 ColBit;

	/* Calculate Column Addresses */
	for (ColBit = 2U; ColBit < XFSBL_MAX_COLUMNS; ColBit++) {
		if (ColBit < XFSBL_COL_WIDTH) {
			Index = 0U;

			while (HifAddr[Index] != XFSBL_HIF_COLUMN(ColBit)) {
				Index++;
			}

			DdrDataPtr->AddrMapColBit[ColBit] = Index - ColBit;
		} else {
			DdrDataPtr->AddrMapColBit[ColBit] = 0xFU;
		}
	}
}

/*****************************************************************************/
/**
 * This function calculates the Row Address Map based on HIF Addresses
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcRowAddr(struct DdrcInitData *DdrDataPtr, u32 *HifAddr)
{
	u32 Index;
	u32 RowBit;

	/* HIF address bits used as row address bits 2 to 10 */
	DdrDataPtr->AddrMapRowBits2To10 = 15U;

	/* Calculate Row Addresses */
	for (RowBit = 0U; RowBit < XFSBL_MAX_ROWS; RowBit++) {
		if (RowBit < XFSBL_ROW_WIDTH) {
			Index = 0U;

			while (HifAddr[Index] != XFSBL_HIF_ROW(RowBit)) {
				Index++;
			}

			DdrDataPtr->AddrMapRowBit[RowBit] = Index -
				(RowBit + 6U);
		} else {
			DdrDataPtr->AddrMapRowBit[RowBit] = 0xFU;
		}
	}
}

/*****************************************************************************/
/**
 * This function calculates the Address Mapping of the DDR
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcAddrMap(struct DdrcInitData *DdrDataPtr)
{
	u32 HifAddr[40] = {0U};
	u32 VideoBuf = XPAR_PSU_DDRC_0_VIDEO_BUFFER_SIZE;
	u32 BrcMapping = XPAR_PSU_DDRC_0_BRC_MAPPING;
	u32 Ddr4AddrMapping = XPAR_PSU_DDRC_0_DDR4_ADDR_MAPPING;

	if (VideoBuf != 0U) {
		/* Calculate the HIF Addresses when Video Buffers are Enabled */
		XFsbl_DdrCalcHifAddrVideo(HifAddr, VideoBuf);
	} else if (BrcMapping == 1U) {
		/*
		 * Calculate the HIF Addresses when Bank-Row-Column Mapping is
		 * Enabled
		 */
		XFsbl_DdrCalcHifAddrBrcMap(HifAddr);
	} else {
		if (Ddr4AddrMapping == 1U) {
			/*
			 * Calculate the HIF Addresses when DDR4 Address Mapping
			 * is Enabled
			 */
			XFsbl_DdrCalcHifAddrMemMap(HifAddr);
		} else {
			/*
			 * Calculate the HIF Addresses for default DDR4 Mapping
			 */
			XFsbl_DdrCalcHifAddr(HifAddr);
		}
	}

	/* Calculate Bank Address Map based on HIF Addresses */
	XFsbl_DdrCalcBankAddr(DdrDataPtr, HifAddr);

	/* Calculate Column Address Map based on HIF Addresses */
	XFsbl_DdrCalcColAddr(DdrDataPtr, HifAddr);

	/* Calculate Row Address Map based on HIF Addresses */
	XFsbl_DdrCalcRowAddr(DdrDataPtr, HifAddr);

	/* Calculate Bank Group Address Map based on HIF Addresses */
	XFsbl_DdrCalcBgAddr(DdrDataPtr, HifAddr);

	/* Rank Width is 0 for the New DIMM */
	DdrDataPtr->AddrMapCsBit0 = 0x1FU;
}

/*****************************************************************************/
/**
 * This function calculates and writes DDR controller registers
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrcRegsInit(struct DdrcInitData *DdrDataPtr)
{
	u32 RegVal;
	u32 Index;

	/* Calculate the values to be written to DDRC registers */

	/* Master Register */
	DdrDataPtr->DdrcReg[0] &= 0x3FFFFFFFU;
	DdrDataPtr->DdrcReg[0] |= (XFSBL_DRAM_IC_WIDTH << 30U);

	/* Refresh Control Register 3 */
	DdrDataPtr->DdrcReg[8] |= 0x1U;

	/* Refresh Timing Register */
	DdrDataPtr->DdrcReg[9] &= 0xFFFFFC00U;
	DdrDataPtr->DdrcReg[9] |= (u32)XFsbl_Ceil((XFSBL_T_RFC / 2.0) /
			XFSBL_CLOCK_PERIOD);

	/* SDRAM Timing Register 0 */
	DdrDataPtr->DdrcReg[24] &= 0xFF00FFFFU;
	DdrDataPtr->DdrcReg[24] |= ((u32)XFsbl_Ceil((XFSBL_T_FAW / 2.0) /
				XFSBL_CLOCK_PERIOD) << 16U);

	/* SDRAM Timing Register 1 */
	DdrDataPtr->DdrcReg[25] &= 0xFFFFFF80U;
	DdrDataPtr->DdrcReg[25] |= (u32)XFsbl_Ceil((XFSBL_T_RC / 2.0) /
			XFSBL_CLOCK_PERIOD);

	/* SDRAM Timing Register 4 */
	DdrDataPtr->DdrcReg[28] &= 0xFFFFF0FFU;
	DdrDataPtr->DdrcReg[28] |= ((u32)XFsbl_Ceil((XFSBL_T_RRDL / 2.0) /
				XFSBL_CLOCK_PERIOD) << 8U);

	/* SDRAM Timing Register 8 */
	DdrDataPtr->DdrcReg[32] &= 0x0000FF00U;
	DdrDataPtr->DdrcReg[32] |= (((u32)XFsbl_Ceil((XFSBL_T_RFC4 + 10.0) /
				XFSBL_CLOCK_PERIOD / 32.0 / 2.0) + 1U) << 24U);
	DdrDataPtr->DdrcReg[32] |= (((u32)XFsbl_Ceil((XFSBL_T_RFC4 + 10.0) /
				XFSBL_CLOCK_PERIOD / 32.0 / 2.0) + 1U) << 16U);
	DdrDataPtr->DdrcReg[32] |= ((u32)XFsbl_Ceil((XFSBL_T_RFC + 10.0) /
				XFSBL_CLOCK_PERIOD / 32.0 / 2.0) + 1U);

	/* SDRAM Timing Register 9 */
	DdrDataPtr->DdrcReg[33] &= 0xFFFFF0FFU;
	DdrDataPtr->DdrcReg[33] |= ((u32)XFsbl_Ceil((XFSBL_T_RRDS / 2.0) /
				XFSBL_CLOCK_PERIOD) << 8U);

	/* SDRAM Timing Register 11 */
	DdrDataPtr->DdrcReg[34] &= 0x00FFFFFFU;
	DdrDataPtr->DdrcReg[34] |= ((u32)XFsbl_Ceil(((XFSBL_T_RFC + 10.0 +
					(768.0 * XFSBL_CLOCK_PERIOD)) /
					(32.0 * 2.0)) + 1.0) << 24U);

	/* ZQ Control Register 0 */
	DdrDataPtr->DdrcReg[36] |= 0x80000000U;

	/* DFI Miscellaneous Control Register */
	DdrDataPtr->DdrcReg[44] &= ~0x1U;

	/* Address Map Register 0 */
	DdrDataPtr->DdrcReg[47] = ((DdrDataPtr->AddrMapCsBit0 & 0x1fU) << 0U);

	/* Address Map Register 1 */
	DdrDataPtr->DdrcReg[48]  = ((DdrDataPtr->AddrMapBankBit[2] &
								0x1fU) << 16U);
	DdrDataPtr->DdrcReg[48] |= ((DdrDataPtr->AddrMapBankBit[1] &
								0x1fU) << 8U);
	DdrDataPtr->DdrcReg[48] |= ((DdrDataPtr->AddrMapBankBit[0] &
								0x1fU) << 0U);

	/* Address Map Register 2 */
	DdrDataPtr->DdrcReg[49]  = ((DdrDataPtr->AddrMapColBit[5] &
								0xfU) << 24U);
	DdrDataPtr->DdrcReg[49] |= ((DdrDataPtr->AddrMapColBit[4] &
								0xfU) << 16U);
	DdrDataPtr->DdrcReg[49] |= ((DdrDataPtr->AddrMapColBit[3] &
								0xfU) << 8U);
	DdrDataPtr->DdrcReg[49] |= ((DdrDataPtr->AddrMapColBit[2] &
								0xfU) << 0U);

	/* Address Map Register 3 */
	DdrDataPtr->DdrcReg[50]  = ((DdrDataPtr->AddrMapColBit[9] &
								0xfU) << 24U);
	DdrDataPtr->DdrcReg[50] |= ((DdrDataPtr->AddrMapColBit[8] &
								0xfU) << 16U);
	DdrDataPtr->DdrcReg[50] |= ((DdrDataPtr->AddrMapColBit[7] &
								0xfU) << 8U);
	DdrDataPtr->DdrcReg[50] |= ((DdrDataPtr->AddrMapColBit[6] &
								0xfU) << 0U);

	/* Address Map Register 4 */
	DdrDataPtr->DdrcReg[51]  = ((DdrDataPtr->AddrMapColBit[11] &
								0xfU) << 8U);
	DdrDataPtr->DdrcReg[51] |= ((DdrDataPtr->AddrMapColBit[10] &
								0xfU) << 0U);

	/* Address Map Register 5 */
	DdrDataPtr->DdrcReg[52]  = ((DdrDataPtr->AddrMapRowBit[11] &
								0xfU) << 24U);
	DdrDataPtr->DdrcReg[52] |= ((DdrDataPtr->AddrMapRowBits2To10 &
								0xfU) << 16U);
	DdrDataPtr->DdrcReg[52] |= ((DdrDataPtr->AddrMapRowBit[1] &
								0xfU) << 8U);
	DdrDataPtr->DdrcReg[52] |= ((DdrDataPtr->AddrMapRowBit[0] &
								0xfU) << 0U);

	/* Address Map Register 6 */
	DdrDataPtr->DdrcReg[53]  = ((DdrDataPtr->AddrMapRowBit[15] &
								0xfU) << 24U);
	DdrDataPtr->DdrcReg[53] |= ((DdrDataPtr->AddrMapRowBit[14] &
								0xfU) << 16U);
	DdrDataPtr->DdrcReg[53] |= ((DdrDataPtr->AddrMapRowBit[13] &
								0xfU) << 8U);
	DdrDataPtr->DdrcReg[53] |= ((DdrDataPtr->AddrMapRowBit[12] &
								0xfU) << 0U);

	/* Address Map Register 7 */
	DdrDataPtr->DdrcReg[54]  = ((DdrDataPtr->AddrMapRowBit[17] &
								0xfU) << 8U);
	DdrDataPtr->DdrcReg[54] |= ((DdrDataPtr->AddrMapRowBit[16] &
								0xfU) << 0U);

	/* Address Map Register 8 */
	DdrDataPtr->DdrcReg[55]  = ((DdrDataPtr->AddrMapBgBit[1] &
								0x1fU) << 8U);
	DdrDataPtr->DdrcReg[55] |= ((DdrDataPtr->AddrMapBgBit[0] &
								0x1fU) << 0U);

	/* Address Map Register 9 */
	DdrDataPtr->DdrcReg[56]  = ((DdrDataPtr->AddrMapRowBit[5] &
								0xfU) << 24U);
	DdrDataPtr->DdrcReg[56] |= ((DdrDataPtr->AddrMapRowBit[4] &
								0xfU) << 16U);
	DdrDataPtr->DdrcReg[56] |= ((DdrDataPtr->AddrMapRowBit[3] &
								0xfU) << 8U);
	DdrDataPtr->DdrcReg[56] |= ((DdrDataPtr->AddrMapRowBit[2] &
								0xfU) << 0U);

	/* Address Map Register 10 */
	DdrDataPtr->DdrcReg[57]  = ((DdrDataPtr->AddrMapRowBit[9] &
								0xfU) << 24U);
	DdrDataPtr->DdrcReg[57] |= ((DdrDataPtr->AddrMapRowBit[8] &
								0xfU) << 16U);
	DdrDataPtr->DdrcReg[57] |= ((DdrDataPtr->AddrMapRowBit[7] &
								0xfU) << 8U);
	DdrDataPtr->DdrcReg[57] |= ((DdrDataPtr->AddrMapRowBit[6] &
								0xfU) << 0U);

	/* Address Map Register 11 */
	DdrDataPtr->DdrcReg[58] = ((DdrDataPtr->AddrMapRowBit[10] &
								0xfU) << 0U);

	/* Software register programming control enable */
	DdrDataPtr->DdrcReg[72] &= ~0x1U;

	/* Assert Reset for DDR controller */
	RegVal = Xil_In32(CRF_APB_RST_DDR_SS_OFFSET);
	RegVal |= 0x00000008U;
	Xil_Out32(CRF_APB_RST_DDR_SS_OFFSET, RegVal);

	/* Write all the DDRC registers */
	for (Index = 0U; Index < XFSBL_DDRC_REG_COUNT; Index++) {
		Xil_Out32(DdrDataPtr->DdrcRegOfst[Index],
				DdrDataPtr->DdrcReg[Index]);
	}

	/* De-assert Reset for DDR controller */
	RegVal = Xil_In32(CRF_APB_RST_DDR_SS_OFFSET);
	RegVal &= ~0x0000000CU;
	Xil_Out32(CRF_APB_RST_DDR_SS_OFFSET, RegVal);
}

/*****************************************************************************/
/**
 * This function calculates and writes the DDR-PHY registers
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_PhyRegsInit(struct DdrcInitData *DdrDataPtr)
{
	u32 Index;

	/* Calculate the values to be written to DDR-PHY registers */

	/* PHY General Configuration Register 2 */
	DdrDataPtr->PhyReg[1] |= (((8U*(u32)((7800.0 / XFSBL_CLOCK_PERIOD))) -
						(u32)1000) & 0x3ffffU);

	/* DRAM Timing Parameters Register 0 */
	DdrDataPtr->PhyReg[10] &= 0x00FFFFFFU;
	DdrDataPtr->PhyReg[10] |= (((u32)XFSBL_T_RRDL & 0x1fU) << 24U);

	/* DRAM Timing Parameters Register 1 */
	DdrDataPtr->PhyReg[11] &= 0xFF00FFFFU;
	DdrDataPtr->PhyReg[11] |= (((u32)XFsbl_Ceil(XFSBL_T_FAW /
					XFSBL_CLOCK_PERIOD) & 0x7fU) << 16U);

	/* DRAM Timing Parameters Register 4 */
	DdrDataPtr->PhyReg[14] &= 0xF000FFFFU;
	DdrDataPtr->PhyReg[14] |= (((u32)XFsbl_Ceil(XFSBL_T_RFC /
					XFSBL_CLOCK_PERIOD) & 0x3ffU) << 16U);

	/* DRAM Timing Parameters Register 5 */
	DdrDataPtr->PhyReg[15] &= 0xFF00FFFFU;
	DdrDataPtr->PhyReg[15] |= (((u32)XFsbl_Ceil(XFSBL_T_RC /
					XFSBL_CLOCK_PERIOD) & 0xffU) << 16U);

	/* DATX8 LANE0 General Configuration Register 5 */
	DdrDataPtr->PhyReg[59] = 0x09095555U;

	/* DATX8 LANE0 General Configuration Register 6 */
	DdrDataPtr->PhyReg[60] = 0x09092B2BU;

	/* DATX8 LANE1 General Configuration Register 5 */
	DdrDataPtr->PhyReg[63] = 0x09095555U;

	/* DATX8 LANE1 General Configuration Register 6 */
	DdrDataPtr->PhyReg[64] = 0x09092B2BU;

	/* DATX8 LANE2 General Configuration Register 5 */
	DdrDataPtr->PhyReg[68] = 0x09095555U;

	/* DATX8 LANE2 General Configuration Register 6 */
	DdrDataPtr->PhyReg[69] = 0x09092B2BU;

	/* DATX8 LANE3 General Configuration Register 5 */
	DdrDataPtr->PhyReg[73] = 0x09095555U;

	/* DATX8 LANE3 General Configuration Register 6 */
	DdrDataPtr->PhyReg[74] = 0x09092B2BU;

	/* DATX8 LANE4 General Configuration Register 5 */
	DdrDataPtr->PhyReg[78] = 0x09095555U;

	/* DATX8 LANE4 General Configuration Register 6 */
	DdrDataPtr->PhyReg[79] = 0x09092B2BU;

	/* DATX8 LANE5 General Configuration Register 5 */
	DdrDataPtr->PhyReg[83] = 0x09095555U;

	/* DATX8 LANE5 General Configuration Register 6 */
	DdrDataPtr->PhyReg[84] = 0x09092B2BU;

	/* DATX8 LANE6 General Configuration Register 5 */
	DdrDataPtr->PhyReg[88] = 0x09095555U;

	/* DATX8 LANE6 General Configuration Register 6 */
	DdrDataPtr->PhyReg[89] = 0x09092B2BU;

	/* DATX8 LANE7 General Configuration Register 5 */
	DdrDataPtr->PhyReg[93] = 0x09095555U;

	/* DATX8 LANE7 General Configuration Register 6 */
	DdrDataPtr->PhyReg[94] = 0x09092B2BU;

	/* DATX8 n General Configuration Register 5 */
	DdrDataPtr->PhyReg[98] = 0x09095555U;

	/* DATX8 0-1 DQS Control Register */
	DdrDataPtr->PhyReg[102] = 0x01264300U;

	/* DATX8 0-1 DX Control Register 2 */
	DdrDataPtr->PhyReg[103] = 0x00041800U;

	/* DATX8 0-1 DQS Control Register */
	DdrDataPtr->PhyReg[107] = 0x01264300U;

	/* DATX8 0-1 DX Control Register 2 */
	DdrDataPtr->PhyReg[108] = 0x00041800U;

	/* DATX8 0-1 DQS Control Register */
	DdrDataPtr->PhyReg[112] = 0x01264300U;

	/* DATX8 0-1 DX Control Register 2 */
	DdrDataPtr->PhyReg[113] = 0x00041800U;

	/* DATX8 0-1 DQS Control Register */
	DdrDataPtr->PhyReg[117] = 0x01264300U;

	/* DATX8 0-1 DX Control Register 2 */
	DdrDataPtr->PhyReg[118] = 0x00041800U;

	/* DATX8 0-1 DQS Control Register */
	DdrDataPtr->PhyReg[122] = 0x01264300U;

	/* DATX8 0-1 DX Control Register 2 */
	DdrDataPtr->PhyReg[123] = 0x00041800U;

	/* DAXT8 0-8 PLL Control Register 0 */
	DdrDataPtr->PhyReg[125] = 0x01100000U;

	/* DATX8 0-8 DQS Control Register */
	DdrDataPtr->PhyReg[126] = 0x012643C4U;

	/* Write all the DDR-PHY registers */
	for (Index = 0U; Index < XFSBL_DDR_PHY_REG_COUNT; Index++) {
		Xil_Out32(DdrDataPtr->PhyRegOfst[Index],
				DdrDataPtr->PhyReg[Index]);
	}
}

/*****************************************************************************/
/**
 * This function Reads the DDR4 SPD from EEPROM via I2C
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 *			returns XFSBL_SUCCESS on success
 *
 *****************************************************************************/
static u32 IicReadSpdEeprom(struct DdrcInitData *DdrDataPtr)
{
	XIicPs IicInstance;		/* The instance of the IIC device. */
	XIicPs_Config *ConfigIic;
	u8 TxArray;
	s32 Status;
	u32 UStatus;
	u32 Regval = 0U;

	/* Lookup for I2C-1 device */
	ConfigIic = XIicPs_LookupConfig(XPAR_PSU_I2C_1_DEVICE_ID);
	if (ConfigIic == NULL) {
		return XFSBL_FAILURE;
	}

	/* Initialize the I2C device */
	Status = XIicPs_CfgInitialize(&IicInstance, ConfigIic,
						  ConfigIic->BaseAddress);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/* Set the Serial Clock for I2C */
	Status = XIicPs_SetSClk(&IicInstance, XFSBL_IIC_SCLK_RATE);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Configure I2C Mux to select DDR4 SODIMM Slave
	 * 0x08 - Enable DDR4 SODIMM module
	 */
	TxArray = 0x08U;
	XIicPs_MasterSendPolled(&IicInstance, &TxArray, 1U, XFSBL_MUX_ADDR);

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	Status = Xil_poll_timeout(Xil_In32, IicInstance.Config.BaseAddress +
		XIICPS_SR_OFFSET, Regval, (Regval & XIICPS_SR_BA_MASK) == 0x0U,
		XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Get Configuration to confirm the selection of the slave
	 * device.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, DdrDataPtr->SpdData, 1U,
							 XFSBL_MUX_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	Status = Xil_poll_timeout(Xil_In32, IicInstance.Config.BaseAddress +
				  XIICPS_SR_OFFSET, Regval, (Regval &
				XIICPS_SR_BA_MASK) == 0x0U,
				XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Set SODIMM control address to enable access to upper
	 * EEPROM page (256 to 511 Bytes).
	 * 0x01 - Enable Read of Upper Page from EEPROM
	 */
	TxArray = 0x01U;
	XIicPs_MasterSendPolled(&IicInstance, &TxArray, 1U,
						XFSBL_SODIMM_CONTROL_ADDR);
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	Status = Xil_poll_timeout(Xil_In32, IicInstance.Config.BaseAddress +
				  XIICPS_SR_OFFSET, Regval, (Regval &
				XIICPS_SR_BA_MASK) == 0x0U,
				XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Configure SODIMM Slave address to select starting address of the
	 * read bytes.
	 * 0x49 - Set starting byte address of read Upper Page from EEPROM
	 * This will result in to starting address of 0x149 (0x100 + 0x49) in
	 * the EEPROM.
	 */
	TxArray = 0x49U;
	XIicPs_MasterSendPolled(&IicInstance, &TxArray, 1U,
						XFSBL_SODIMM_SLAVE_ADDR);
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	Status = Xil_poll_timeout(Xil_In32, IicInstance.Config.BaseAddress +
				  XIICPS_SR_OFFSET, Regval, (Regval &
				XIICPS_SR_BA_MASK) == 0x0U,
				XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Receive the Data of 20 Bytes from SPD EEPROM via I2C.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, DdrDataPtr->SpdData, 20U,
						XFSBL_SODIMM_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Wait until bus is idle.
	 */
	Status = Xil_poll_timeout(Xil_In32, IicInstance.Config.BaseAddress +
				  XIICPS_SR_OFFSET, Regval, (Regval &
				XIICPS_SR_BA_MASK) == 0x0U,
				XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	return XFSBL_SUCCESS;

END:
	return UStatus;
}

/*****************************************************************************/
/**
 * This function checks for the DDR part ID and Initializes the same based on
 * configuration parameters for the new DIMM
 *
 * @param	None
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 *			returns XFSBL_SUCCESS on success
 *
 *****************************************************************************/
u32 XFsbl_DdrInit(void)
{
	u32 Index;
	u32 Status;
	u8 NewDimmId[] = "4ATF51264HZ-2G6E1   ";

	/* Define and Initialize the DDR Initialization data */
	struct DdrcInitData DdrData = {
			.DdrcRegOfst = DDRC_REG_OFFSET,
			.PhyRegOfst = PHY_REG_OFFSET,
			.AddrMapCsBit0 = 0x0U,
			.AddrMapRowBits2To10 = 0x0U,
	};

	/* Get the Model Part Number from the SPD stored in EEPROM */
	Status = IicReadSpdEeprom(&DdrData);
	if (Status != XFSBL_SUCCESS) {
		return XFSBL_FAILURE;
	}

	/*
	 * Check whether it is the New Part (MTA4ATF51264HZ) or the
	 * Old Part(MTA8ATF51264HZ) or Customer Owned Part.
	 * If it is not the new part (MTA4ATF51264HZ), simply return from here.
	 */
	Status = memcmp(DdrData.SpdData, NewDimmId, sizeof(DdrData.SpdData));
	if (Status != 0U) {
		return XFSBL_SUCCESS;
	}

	/* Read back all the registers from DDR controller */
	for (Index = 0U; Index < XFSBL_DDRC_REG_COUNT; Index++) {
		DdrData.DdrcReg[Index] = Xil_In32(DdrData.DdrcRegOfst[Index]);
	}

	/* Read back all the registers from DDR-PHY controller */
	for (Index = 0U; Index < XFSBL_DDR_PHY_REG_COUNT; Index++) {
		DdrData.PhyReg[Index] = Xil_In32(DdrData.PhyRegOfst[Index]);
	}

	/* Calculate Address Map for the DDR */
	XFsbl_DdrCalcAddrMap(&DdrData);

	/* Calculate and Write all the registers of DDR Controller */
	XFsbl_DdrcRegsInit(&DdrData);

	/* Calculate and Write all the registers of DDR-PHY Controller */
	XFsbl_PhyRegsInit(&DdrData);

	/* Execute the Training Sequence */
	(void)psu_ddr_phybringup_data();

	return XFSBL_SUCCESS;
}
#endif /* defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106) */
