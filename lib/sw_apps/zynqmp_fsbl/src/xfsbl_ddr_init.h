/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xfsbl_ddr_init.h
 *
 * This is the file which contains definition for initialization function
 * for the DDR.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   mn   07/06/18 Add DDR initialization support for new DDR DIMM part
 *       mn   07/18/18 Move iicps.h inclusion under ZCU102 and ZCU106 macro
 *                     checks
 * 2.0   mn   02/28/19 Add Dynamic DDR initialization support for all DDR DIMMs
 *       mn   03/12/19 Select EEPROM Lower Page for reading SPD data
 *       mn   09/03/19 Fix coverity warnings
 * 3.0   bsv  11/12/19 Added support for ZCU216 board
 *       mn   12/24/19 Enable Address Mirroring based on SPD data
 *       bsv  02/05/20 Added support for ZCU208 board
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XFSBL_DDR_INIT_H
#define XFSBL_DDR_INIT_H

#ifdef __cplusplus
extern "C" {
#endif
#ifdef XFSBL_PS_DDR
#ifdef XPAR_DYNAMIC_DDR_ENABLED
/***************************** Include Files *********************************/
#include "psu_init.h"

/************************** Constant Definitions *****************************/
/* Maximum number of Rows in DDR */
#define XFSBL_MAX_ROWS				18U
/* Maximum number of Columns in DDR */
#define XFSBL_MAX_COLUMNS			12U
/* Maximum number of Banks in DDR */
#define XFSBL_MAX_BANKS				3U
/* Maximum number of Bank Groups in DDR */
#define XFSBL_MAX_BANK_GROUPS			2U
/* Total number of DDR controller registers */
#define XFSBL_DDRC_REG_COUNT			115U
/* Total number of DDR PHY registers */
#define XFSBL_DDR_PHY_REG_COUNT			127U

/* In some older designs, this register is not used in psu_init flow */
#ifndef DDR_PHY_GPR0_OFFSET
#define DDR_PHY_GPR0_OFFSET			0xFD0800C0U
#endif
#ifndef DDRC_STAT_OFFSET
#define DDRC_STAT_OFFSET			0xFD070004U
#endif
#ifndef DDRC_MRSTAT_OFFSET
#define DDRC_MRSTAT_OFFSET			0xFD070018U
#endif
#ifndef DDRC_RFSHCTL1_OFFSET
#define DDRC_RFSHCTL1_OFFSET			0xFD070054U
#endif
#ifndef DDRC_DFIUPD0_OFFSET
#define DDRC_DFIUPD0_OFFSET			0xFD0701A0U
#endif
#ifndef DDRC_DQMAP0_OFFSET
#define DDRC_DQMAP0_OFFSET			0xFD070280U
#endif
#ifndef DDRC_DQMAP1_OFFSET
#define DDRC_DQMAP1_OFFSET			0xFD070284U
#endif
#ifndef DDRC_DQMAP2_OFFSET
#define DDRC_DQMAP2_OFFSET			0xFD070288U
#endif
#ifndef DDRC_DQMAP3_OFFSET
#define DDRC_DQMAP3_OFFSET			0xFD07028CU
#endif
#ifndef DDRC_DQMAP4_OFFSET
#define DDRC_DQMAP4_OFFSET			0xFD070290U
#endif

#ifndef DDR_PHY_PIR_OFFSET
#define DDR_PHY_PIR_OFFSET			0xFD080004U
#endif
#ifndef DDR_PHY_PGCR1_OFFSET
#define DDR_PHY_PGCR1_OFFSET		0xFD080014U
#endif
#ifndef DDR_PHY_PGCR6_OFFSET
#define DDR_PHY_PGCR6_OFFSET		0xFD080028U
#endif
#ifndef DDR_PHY_PGSR0_OFFSET
#define DDR_PHY_PGSR0_OFFSET		0xFD080030U
#endif
#ifndef DDR_PHY_PLLCR0_OFFSET
#define DDR_PHY_PLLCR0_OFFSET			0xFD080068U
#endif
#ifndef DDR_PHY_ODTCR_OFFSET
#define DDR_PHY_ODTCR_OFFSET			0xFD080098U
#endif
#ifndef DDR_PHY_GPR1_OFFSET
#define DDR_PHY_GPR1_OFFSET			0xFD0800C4U
#endif
#ifndef DDR_PHY_SCHCR0_OFFSET
#define DDR_PHY_SCHCR0_OFFSET			0xFD080168U
#endif
#ifndef DDR_PHY_SCHCR1_OFFSET
#define DDR_PHY_SCHCR1_OFFSET			0xFD08016CU
#endif
#ifndef DDR_PHY_DQSDR0_OFFSET
#define DDR_PHY_DQSDR0_OFFSET			0xFD080250U
#endif
#ifndef DDR_PHY_DQSDR1_OFFSET
#define DDR_PHY_DQSDR1_OFFSET			0xFD080254U
#endif
#ifndef DDR_PHY_RANKIDR_OFFSET
#define DDR_PHY_RANKIDR_OFFSET			0xFD0804DCU
#endif
#ifndef DDR_PHY_DX0BDLR3_OFFSET
#define DDR_PHY_DX0BDLR3_OFFSET    0xFD080750U
#endif
#ifndef DDR_PHY_DX0BDLR4_OFFSET
#define DDR_PHY_DX0BDLR4_OFFSET    0xFD080754U
#endif
#ifndef DDR_PHY_DX0BDLR5_OFFSET
#define DDR_PHY_DX0BDLR5_OFFSET    0xFD080758U
#endif
#ifndef DDR_PHY_DX0LCDLR1_OFFSET
#define DDR_PHY_DX0LCDLR1_OFFSET	0xFD080784U
#endif
#ifndef DDR_PHY_DX0MDLR0_OFFSET
#define DDR_PHY_DX0MDLR0_OFFSET		0xFD0807A0U
#endif
#ifndef DDR_PHY_DX0GTR0_OFFSET
#define DDR_PHY_DX0GTR0_OFFSET		0xFD0807C0U
#endif
#ifndef DDR_PHY_DX0GSR0_OFFSET
#define DDR_PHY_DX0GSR0_OFFSET		0xFD0807E0U
#endif
#ifndef DDR_PHY_DX2GSR0_OFFSET
#define DDR_PHY_DX2GSR0_OFFSET		0xFD0809E0U
#endif
#ifndef DDR_PHY_DX4GSR0_OFFSET
#define DDR_PHY_DX4GSR0_OFFSET		0xFD080BE0U
#endif
#ifndef DDR_PHY_DX6GSR0_OFFSET
#define DDR_PHY_DX6GSR0_OFFSET		0xFD080DE0U
#endif
#ifndef DDR_PHY_DX8GSR0_OFFSET
#define DDR_PHY_DX8GSR0_OFFSET		0xFD080FE0U
#endif
#ifndef DDR_PHY_DX1BDLR3_OFFSET
#define DDR_PHY_DX1BDLR3_OFFSET    0xFD080850U
#endif
#ifndef DDR_PHY_DX1BDLR4_OFFSET
#define DDR_PHY_DX1BDLR4_OFFSET    0xFD080854U
#endif
#ifndef DDR_PHY_DX1BDLR5_OFFSET
#define DDR_PHY_DX1BDLR5_OFFSET    0xFD080858U
#endif
#ifndef DDR_PHY_DX2BDLR3_OFFSET
#define DDR_PHY_DX2BDLR3_OFFSET    0xFD080950U
#endif
#ifndef DDR_PHY_DX2BDLR4_OFFSET
#define DDR_PHY_DX2BDLR4_OFFSET    0xFD080954U
#endif
#ifndef DDR_PHY_DX2BDLR5_OFFSET
#define DDR_PHY_DX2BDLR5_OFFSET    0xFD080958U
#endif
#ifndef DDR_PHY_DX3BDLR3_OFFSET
#define DDR_PHY_DX3BDLR3_OFFSET    0xFD080A50U
#endif
#ifndef DDR_PHY_DX3BDLR4_OFFSET
#define DDR_PHY_DX3BDLR4_OFFSET    0xFD080A54U
#endif
#ifndef DDR_PHY_DX3BDLR5_OFFSET
#define DDR_PHY_DX3BDLR5_OFFSET    0xFD080A58U
#endif
#ifndef DDR_PHY_DX4BDLR3_OFFSET
#define DDR_PHY_DX4BDLR3_OFFSET    0xFD080B50U
#endif
#ifndef DDR_PHY_DX4BDLR4_OFFSET
#define DDR_PHY_DX4BDLR4_OFFSET    0xFD080B54U
#endif
#ifndef DDR_PHY_DX4BDLR5_OFFSET
#define DDR_PHY_DX4BDLR5_OFFSET    0xFD080B58U
#endif
#ifndef DDR_PHY_DX5BDLR3_OFFSET
#define DDR_PHY_DX5BDLR3_OFFSET    0xFD080C50U
#endif
#ifndef DDR_PHY_DX5BDLR4_OFFSET
#define DDR_PHY_DX5BDLR4_OFFSET    0xFD080C54U
#endif
#ifndef DDR_PHY_DX5BDLR5_OFFSET
#define DDR_PHY_DX5BDLR5_OFFSET    0xFD080C58U
#endif
#ifndef DDR_PHY_DX6BDLR3_OFFSET
#define DDR_PHY_DX6BDLR3_OFFSET    0xFD080D50U
#endif
#ifndef DDR_PHY_DX6BDLR4_OFFSET
#define DDR_PHY_DX6BDLR4_OFFSET    0xFD080D54U
#endif
#ifndef DDR_PHY_DX6BDLR5_OFFSET
#define DDR_PHY_DX6BDLR5_OFFSET    0xFD080D58U
#endif
#ifndef DDR_PHY_DX7BDLR3_OFFSET
#define DDR_PHY_DX7BDLR3_OFFSET    0xFD080E50U
#endif
#ifndef DDR_PHY_DX7BDLR4_OFFSET
#define DDR_PHY_DX7BDLR4_OFFSET    0xFD080E54U
#endif
#ifndef DDR_PHY_DX7BDLR5_OFFSET
#define DDR_PHY_DX7BDLR5_OFFSET    0xFD080E58U
#endif
#ifndef DDR_PHY_DX8BDLR3_OFFSET
#define DDR_PHY_DX8BDLR3_OFFSET    0xFD080F50U
#endif
#ifndef DDR_PHY_DX8BDLR4_OFFSET
#define DDR_PHY_DX8BDLR4_OFFSET    0xFD080F54U
#endif
#ifndef DDR_PHY_DX8BDLR5_OFFSET
#define DDR_PHY_DX8BDLR5_OFFSET    0xFD080F58U
#endif

#ifndef DDR_PHY_DX8SL0PLLCR0_OFFSET
#define DDR_PHY_DX8SL0PLLCR0_OFFSET		0xFD081404U
#endif
#ifndef DDR_PHY_DX8SL1PLLCR0_OFFSET
#define DDR_PHY_DX8SL1PLLCR0_OFFSET		0xFD081444U
#endif
#ifndef DDR_PHY_DX8SL2PLLCR0_OFFSET
#define DDR_PHY_DX8SL2PLLCR0_OFFSET		0xFD081484U
#endif
#ifndef DDR_PHY_DX8SL3PLLCR0_OFFSET
#define DDR_PHY_DX8SL3PLLCR0_OFFSET		0xFD0814C4U
#endif
#ifndef DDR_PHY_DX8SL4PLLCR0_OFFSET
#define DDR_PHY_DX8SL4PLLCR0_OFFSET		0xFD081504U
#endif
#ifndef DDR_PHY_DX8SLBPLLCR0_OFFSET
#define DDR_PHY_DX8SLBPLLCR0_OFFSET		0xFD0817C4U
#endif

#ifndef DDR_QOS_CTRL_DDRPHY_CTRL_OFFSET
#define DDR_QOS_CTRL_DDRPHY_CTRL_OFFSET		0xFD090708U
#endif
#ifndef DDR_QOS_CTRL_DDR_CLK_CTRL_OFFSET
#define DDR_QOS_CTRL_DDR_CLK_CTRL_OFFSET		0xFD090700U
#endif

#define DDR_PHY_PIR_INIT_SHIFT			0U
#define DDR_PHY_PIR_INIT_WIDTH			1U
#define DDR_PHY_PIR_INIT_MASK			0x00000001U
#define DDR_PHY_PIR_INIT_DEFVAL			0x0U

#define DDR_PHY_PIR_DCALPSE_SHIFT   29U
#define DDR_PHY_PIR_DCALPSE_WIDTH   1U
#define DDR_PHY_PIR_DCALPSE_MASK    0x20000000U
#define DDR_PHY_PIR_DCALPSE_DEFVAL  0x0U

#define DDR_PHY_PIR_CTLDINIT_SHIFT   18U
#define DDR_PHY_PIR_CTLDINIT_WIDTH   1U
#define DDR_PHY_PIR_CTLDINIT_MASK    0x00040000U
#define DDR_PHY_PIR_CTLDINIT_DEFVAL  0x0U

#define DDR_PHY_PIR_PHYRST_SHIFT   6U
#define DDR_PHY_PIR_PHYRST_WIDTH   1U
#define DDR_PHY_PIR_PHYRST_MASK    0x00000040U
#define DDR_PHY_PIR_PHYRST_DEFVAL  0x0U

#define DDR_PHY_PIR_DCAL_SHIFT   5U
#define DDR_PHY_PIR_DCAL_WIDTH   1U
#define DDR_PHY_PIR_DCAL_MASK    0x00000020U
#define DDR_PHY_PIR_DCAL_DEFVAL  0x0U

#define DDR_PHY_PIR_PLLINIT_SHIFT   4U
#define DDR_PHY_PIR_PLLINIT_MASK    0x00000010U

#define DDR_PHY_PIR_QSGATE_SHIFT   10U
#define DDR_PHY_PIR_QSGATE_MASK    0x00000400U

#define DDR_PHY_PIR_WL_SHIFT   9U
#define DDR_PHY_PIR_WL_MASK    0x00000200U

#define DDR_PHY_PGSR0_IDONE_SHIFT   0U
#define DDR_PHY_PGSR0_IDONE_MASK    0x00000001U

#define DDR_PHY_PGCR1_PUBMODE_SHIFT		6U
#define DDR_PHY_PGCR1_PUBMODE_WIDTH		1U
#define DDR_PHY_PGCR1_PUBMODE_MASK		0x00000040U
#define DDR_PHY_PGCR1_PUBMODE_DEFVAL	0x0U

#define DDR_PHY_PGCR1_PHYHRST_SHIFT   25U
#define DDR_PHY_PGCR1_PHYHRST_WIDTH   1U
#define DDR_PHY_PGCR1_PHYHRST_MASK    0x02000000U
#define DDR_PHY_PGCR1_PHYHRST_DEFVAL  0x1U

#define DDR_PHY_SCHCR0_SP_CMD_SHIFT   8U
#define DDR_PHY_SCHCR0_SP_CMD_WIDTH   4U
#define DDR_PHY_SCHCR0_SP_CMD_MASK    0x00000F00U
#define DDR_PHY_SCHCR0_SP_CMD_DEFVAL  0x0U

#define DDR_PHY_SCHCR0_CMD_SHIFT   4U
#define DDR_PHY_SCHCR0_CMD_WIDTH   4U
#define DDR_PHY_SCHCR0_CMD_MASK    0x000000F0U
#define DDR_PHY_SCHCR0_CMD_DEFVAL  0x0U

#define DDR_PHY_SCHCR0_SCHTRIG_SHIFT   0U
#define DDR_PHY_SCHCR0_SCHTRIG_WIDTH   4U
#define DDR_PHY_SCHCR0_SCHTRIG_MASK    0x0000000FU
#define DDR_PHY_SCHCR0_SCHTRIG_DEFVAL  0x0U

#define DDR_PHY_SCHCR1_ALLRANK_SHIFT   2U
#define DDR_PHY_SCHCR1_ALLRANK_WIDTH   1U
#define DDR_PHY_SCHCR1_ALLRANK_MASK    0x00000004U
#define DDR_PHY_SCHCR1_ALLRANK_DEFVAL  0x0U

#define DDR_PHY_RANKIDR_RANKWID_SHIFT   0U
#define DDR_PHY_RANKIDR_RANKWID_WIDTH   4U
#define DDR_PHY_RANKIDR_RANKWID_MASK    0x0000000FU
#define DDR_PHY_RANKIDR_RANKWID_DEFVAL  0x0U

#define DDR_PHY_PGCR6_INHVT_SHIFT   0U
#define DDR_PHY_PGCR6_INHVT_MASK    0x00000001U

#define DDR_PHY_ODTCR_WRODT_SHIFT   16U
#define DDR_PHY_ODTCR_WRODT_WIDTH   2U
#define DDR_PHY_ODTCR_WRODT_MASK    0x00030000U
#define DDR_PHY_ODTCR_WRODT_DEFVAL  0x1U

#define DDR_PHY_DTCR0_INCWEYE_SHIFT   4U
#define DDR_PHY_DTCR0_INCWEYE_WIDTH   1U
#define DDR_PHY_DTCR0_INCWEYE_MASK    0x00000010U
#define DDR_PHY_DTCR0_INCWEYE_DEFVAL  0x0U

#define DDR_PHY_DQSDR1_DFTRDIDLC_SHIFT   0U
#define DDR_PHY_DQSDR1_DFTRDIDLC_WIDTH   8U
#define DDR_PHY_DQSDR1_DFTRDIDLC_MASK    0x000000FFU
#define DDR_PHY_DQSDR1_DFTRDIDLC_DEFVAL  0x0U

#define DDR_PHY_DQSDR1_DFTRDIDLF_SHIFT   16U
#define DDR_PHY_DQSDR1_DFTRDIDLF_WIDTH   4U
#define DDR_PHY_DQSDR1_DFTRDIDLF_MASK    0x000F0000U
#define DDR_PHY_DQSDR1_DFTRDIDLF_DEFVAL  0x0U

#define DDR_PHY_DXBDLR_DQ0RBD_SHIFT   0U
#define DDR_PHY_DXBDLR_DQ0RBD_MASK    0x0000003FU

#define DDR_PHY_DXBDLR_DQ1RBD_SHIFT   8U
#define DDR_PHY_DXBDLR_DQ1RBD_MASK    0x00003F00U

#define DDR_PHY_DXBDLR_DQ2RBD_SHIFT   16U
#define DDR_PHY_DXBDLR_DQ2RBD_MASK    0x003F0000U

#define DDR_PHY_DXBDLR_DQ3RBD_SHIFT   24U
#define DDR_PHY_DXBDLR_DQ3RBD_MASK    0x3F000000U

#define DDR_PHY_DX0BDLR5_DMRBD_SHIFT   0U
#define DDR_PHY_DXBDLR5_DMRBD_MASK    0x0000003FU

#define DDR_PHY_DXGTR0_WDQSL_SHIFT   24U
#define DDR_PHY_DXGTR0_WDQSL_MASK    0x07000000U

#define DDR_PHY_DXLCDLR1_WDQD_SHIFT   0U
#define DDR_PHY_DXLCDLR1_WDQD_MASK    0x000001FFU

#define DDR_PHY_DXMDLR0_IPRD_SHIFT   0U
#define DDR_PHY_DXMDLR0_IPRD_MASK    0x000001FFU

#define DDR_PHY_PIR_DQS2DQ_SHIFT   20U
#define DDR_PHY_PIR_DQS2DQ_MASK    0x00100000U

#define DDR_PHY_PGSR0_DQS2DQDONE_SHIFT   15U
#define DDR_PHY_PGSR0_DQS2DQDONE_MASK    0x00008000U

#define DDR_PHY_PIR_DQS2DQ_SHIFT   20U
#define DDR_PHY_PIR_DQS2DQ_MASK    0x00100000U

#define DDR_PHY_PGSR0_DQS2DQDONE_SHIFT   15U
#define DDR_PHY_PGSR0_DQS2DQDONE_MASK    0x00008000U

#define DDR_PHY_PIR_VREF_SHIFT   17U
#define DDR_PHY_PIR_VREF_MASK    0x00020000U

#define DDR_PHY_PIR_WREYE_SHIFT   15U
#define DDR_PHY_PIR_WREYE_MASK    0x00008000U

#define DDR_PHY_PIR_RDEYE_SHIFT   14U
#define DDR_PHY_PIR_RDEYE_MASK    0x00004000U

#define DDR_PHY_PGSR0_REDONE_SHIFT   10U
#define DDR_PHY_PGSR0_REDONE_MASK    0x00000400U

#define DDR_PHY_PIR_WLADJ_SHIFT   11U
#define DDR_PHY_PIR_WLADJ_MASK    0x00000800U

#define DDR_PHY_PIR_WRDSKW_SHIFT   13U
#define DDR_PHY_PIR_WRDSKW_MASK    0x00002000U

#define DDR_PHY_PIR_RDDSKW_SHIFT   12U
#define DDR_PHY_PIR_RDDSKW_MASK    0x00001000U

#define DDR_PHY_RANKIDR_RANKRID_SHIFT   16U
#define DDR_PHY_RANKIDR_RANKRID_MASK    0x000F0000U

#define DDR_QOS_CTRL_DDRPHY_CTRL_BYP_MODE_SHIFT   0U
#define DDR_QOS_CTRL_DDRPHY_CTRL_BYP_MODE_WIDTH   1U
#define DDR_QOS_CTRL_DDRPHY_CTRL_BYP_MODE_MASK    0x00000001U
#define DDR_QOS_CTRL_DDRPHY_CTRL_BYP_MODE_DEFVAL  0x0U

#define DDR_QOS_CTRL_DDR_CLK_CTRL_CLKACT_SHIFT   0U
#define DDR_QOS_CTRL_DDR_CLK_CTRL_CLKACT_WIDTH   1U
#define DDR_QOS_CTRL_DDR_CLK_CTRL_CLKACT_MASK    0x00000001U
#define DDR_QOS_CTRL_DDR_CLK_CTRL_CLKACT_DEFVAL  0x1U

/*
 * Byte 2U Fundamental Memory Types.
 */
#define SPD_MEMTYPE_FPM		(0x01U)
#define SPD_MEMTYPE_EDO		(0x02U)
#define SPD_MEMTYPE_PIPE_NIBBLE	(0x03U)
#define SPD_MEMTYPE_SDRAM	(0x04U)
#define SPD_MEMTYPE_ROM		(0x05U)
#define SPD_MEMTYPE_SGRAM	(0x06U)
#define SPD_MEMTYPE_DDR		(0x07U)
#define SPD_MEMTYPE_DDR2	(0x08U)
#define SPD_MEMTYPE_DDR2_FBDIMM	(0x09U)
#define SPD_MEMTYPE_DDR2_FBDIMM_PROBE	(0x0AU)
#define SPD_MEMTYPE_DDR3	(0x0BU)
#define SPD_MEMTYPE_DDR4	(0x0CU)
#define SPD_MEMTYPE_LPDDR3	(0x0FU)
#define SPD_MEMTYPE_LPDDR4	(0x10U)

/* Byte 3U Key Byte / Module Type for DDR3 SPD */
#define DDR3_SPD_MODULETYPE_MASK	(0x0FU)
#define DDR3_SPD_MODULETYPE_RDIMM	(0x01U)
#define DDR3_SPD_MODULETYPE_UDIMM	(0x02U)
#define DDR3_SPD_MODULETYPE_SO_DIMM	(0x03U)
#define DDR3_SPD_MODULETYPE_MICRO_DIMM	(0x04U)
#define DDR3_SPD_MODULETYPE_MINI_RDIMM	(0x05U)
#define DDR3_SPD_MODULETYPE_MINI_UDIMM	(0x06U)
#define DDR3_SPD_MODULETYPE_MINI_CDIMM	(0x07U)
#define DDR3_SPD_MODULETYPE_72B_SO_UDIMM	(0x08U)
#define DDR3_SPD_MODULETYPE_72B_SO_RDIMM	(0x09U)
#define DDR3_SPD_MODULETYPE_72B_SO_CDIMM	(0x0AU)
#define DDR3_SPD_MODULETYPE_LRDIMM	(0x0BU)
#define DDR3_SPD_MODULETYPE_16B_SO_DIMM	(0x0CU)
#define DDR3_SPD_MODULETYPE_32B_SO_DIMM	(0x0DU)

/* DIMM Type for DDR4 SPD */
#define DDR4_SPD_MODULETYPE_MASK	(0x0FU)
#define DDR4_SPD_MODULETYPE_EXT		(0x00U)
#define DDR4_SPD_MODULETYPE_RDIMM	(0x01U)
#define DDR4_SPD_MODULETYPE_UDIMM	(0x02U)
#define DDR4_SPD_MODULETYPE_SO_DIMM	(0x03U)
#define DDR4_SPD_MODULETYPE_LRDIMM	(0x04U)
#define DDR4_SPD_MODULETYPE_MINI_RDIMM	(0x05U)
#define DDR4_SPD_MODULETYPE_MINI_UDIMM	(0x06U)
#define DDR4_SPD_MODULETYPE_72B_SO_RDIMM	(0x08U)
#define DDR4_SPD_MODULETYPE_72B_SO_UDIMM	(0x09U)
#define DDR4_SPD_MODULETYPE_16B_SO_DIMM	(0x0CU)
#define DDR4_SPD_MODULETYPE_32B_SO_DIMM	(0x0DU)

/* DIMM Type for LPDDR SPD */
#define LPDDR_SPD_MODULETYPE_MASK	(0x0FU)
#define LPDDR_SPD_MODULETYPE_EXT	(0x00U)
#define LPDDR_SPD_MODULETYPE_LPDIMM	(0x07U)
#define LPDDR_SPD_MODULETYPE_NON_DIMM	(0xEU)

/* DDRC Register fields */
enum {
	DDR_DEVICE_CONFIG,
	DDR_FREQUENCY_MODE,
	DDR_ACTIVE_RANKS,
	DDR_BURST_RDWR,
	DDR_DLL_OFF_MODE,
	DDR_DATA_BUS_WIDTH,
	DDR_GEARDOWN_MODE,
	DDR_EN_2T_TIMING_MODE,
	DDR_BURSTCHOP,
	DDR_LPDDR4,
	DDR_DDR4,
	DDR_LPDDR3,
	DDR_LPDDR2,
	DDR_DDR3,
	DDR_MR_WR,
	DDR_MR_ADDR,
	DDR_MR_RANK,
	DDR_SW_INIT_INT,
	DDR_PDA_EN,
	DDR_MPR_EN,
	DDR_MR_TYPE,
	DDR_RC_DERATE_VALUE,
	DDR_DERATE_BYTE,
	DDR_DERATE_VALUE,
	DDR_DERATE_ENABLE,
	DDR_MR4_READ_INTERVAL,
	DDR_STAY_IN_SELFREF,
	DDR_SELFREF_SW,
	DDR_MPSM_EN,
	DDR_EN_DFI_DRAM_CLK_DISABLE,
	DDR_DEEPPOWERDOWN_EN,
	DDR_POWERDOWN_EN,
	DDR_SELFREF_EN,
	DDR_SELFREF_TO_X32,
	DDR_T_DPD_X4096,
	DDR_POWERDOWN_TO_X32,
	DDR_REFRESH_MARGIN,
	DDR_REFRESH_TO_X32,
	DDR_REFRESH_BURST,
	DDR_PER_BANK_REFRESH,
	DDR_REFRESH_TIMER1_START_VALUE_X32,
	DDR_REFRESH_TIMER0_START_VALUE_X32,
	DDR_REFRESH_MODE,
	DDR_REFRESH_UPDATE_LEVEL,
	DDR_DIS_AUTO_REFRESH,
	DDR_T_RFC_NOM_X32,
	DDR_LPDDR3_TREFBW_EN,
	DDR_T_RFC_MIN,
	DDR_DIS_SCRUB,
	DDR_ECC_MODE,
	DDR_DATA_POISON_BIT,
	DDR_DATA_POISON_EN,
	DDR_DFI_T_PHY_RDLAT,
	DDR_ALERT_WAIT_FOR_SW,
	DDR_CRC_PARITY_RETRY_ENABLE,
	DDR_CRC_INC_DM,
	DDR_CRC_ENABLE,
	DDR_PARITY_ENABLE,
	DDR_T_PAR_ALERT_PW_MAX,
	DDR_T_CRC_ALERT_PW_MAX,
	DDR_RETRY_FIFO_MAX_HOLD_TIMER_X4,
	DDR_SKIP_DRAM_INIT,
	DDR_POST_CKE_X1024,
	DDR_PRE_CKE_X1024,
	DDR_DRAM_RSTN_X1024,
	DDR_FINAL_WAIT_X32,
	DDR_PRE_OCD_X32,
	DDR_IDLE_AFTER_RESET_X32,
	DDR_MIN_STABLE_CLOCK_X1,
	DDR_MR,
	DDR_EMR,
	DDR_EMR2,
	DDR_EMR3,
	DDR_DEV_ZQINIT_X32,
	DDR_MAX_AUTO_INIT_X1024,
	DDR_MR4,
	DDR_MR5,
	DDR_MR6,
	DDR_DIMM_DIS_BG_MIRRORING,
	DDR_MRS_BG1_EN,
	DDR_MRS_A17_EN,
	DDR_DIMM_OUTPUT_INV_EN,
	DDR_DIMM_ADDR_MIRR_EN,
	DDR_DIMM_STAGGER_CS_EN,
	DDR_DIFF_RANK_WR_GAP,
	DDR_DIFF_RANK_RD_GAP,
	DDR_MAX_RANK_RD,
	DDR_WR2PRE,
	DDR_T_FAW,
	DDR_T_RAS_MAX,
	DDR_T_RAS_MIN,
	DDR_T_XP,
	DDR_RD2PRE,
	DDR_T_RC,
	DDR_WRITE_LATENCY,
	DDR_READ_LATENCY,
	DDR_RD2WR,
	DDR_WR2RD,
	DDR_T_MRW,
	DDR_T_MRD,
	DDR_T_MOD,
	DDR_T_RCD,
	DDR_T_CCD,
	DDR_T_RRD,
	DDR_T_RP,
	DDR_T_CKSRX,
	DDR_T_CKSRE,
	DDR_T_CKESR,
	DDR_T_CKE,
	DDR_T_CKDPDE,
	DDR_T_CKDPDX,
	DDR_T_CKCSX,
	DDR_T_CKPDE,
	DDR_T_CKPDX,
	DDR_T_XS_FAST_X32,
	DDR_T_XS_ABORT_X32,
	DDR_T_XS_DLL_X32,
	DDR_T_XS_X32,
	DDR_DDR4_WR_PREAMBLE,
	DDR_T_CCD_S,
	DDR_T_RRD_S,
	DDR_WR2RD_S,
	DDR_POST_MPSM_GAP_X32,
	DDR_T_MPX_LH,
	DDR_T_MPX_S,
	DDR_T_CKMPE,
	DDR_T_CMDCKE,
	DDR_T_CKEHCMD,
	DDR_T_MRD_PDA,
	DDR_DIS_AUTO_ZQ,
	DDR_DIS_SRX_ZQCL,
	DDR_ZQ_RESISTOR_SHARED,
	DDR_DIS_MPSMX_ZQCL,
	DDR_T_ZQ_LONG_NOP,
	DDR_T_ZQ_SHORT_NOP,
	DDR_T_ZQ_RESET_NOP,
	DDR_T_ZQ_SHORT_INTERVAL_X1024,
	DDR_DFI_T_CTRL_DELAY,
	DDR_DFI_RDDATA_USE_SDR,
	DDR_DFI_T_RDDATA_EN,
	DDR_DFI_WRDATA_USE_SDR,
	DDR_DFI_TPHY_WRDATA,
	DDR_DFI_TPHY_WRLAT,
	DDR_DFI_T_CMD_LAT,
	DDR_DFI_T_PARIN_LAT,
	DDR_DFI_T_WRDATA_DELAY,
	DDR_DFI_T_DRAM_CLK_DISABLE,
	DDR_DFI_T_DRAM_CLK_ENABLE,
	DDR_DFI_TLP_RESP,
	DDR_DFI_LP_WAKEUP_DPD,
	DDR_DFI_LP_EN_DPD,
	DDR_DFI_LP_WAKEUP_SR,
	DDR_DFI_LP_EN_SR,
	DDR_DFI_LP_WAKEUP_PD,
	DDR_DFI_LP_EN_PD,
	DDR_DFI_LP_WAKEUP_MPSM,
	DDR_DFI_LP_EN_MPSM,
	DDR_DIS_AUTO_CTRLUPD,
	DDR_DIS_AUTO_CTRLUPD_SRX,
	DDR_DFI_T_CTRLUP_MAX,
	DDR_DFI_T_CTRLUP_MIN,
	DDR_DFI_T_CTRLUPD_INTERVAL_MIN_X1024,
	DDR_DFI_T_CTRLUPD_INTERVAL_MAX_X1024,
	DDR_DFI_DATA_CS_POLARITY,
	DDR_PHY_DBI_MODE,
	DDR_DFI_INIT_COMPLETE_EN,
	DDR_DFI_TPHY_RDCSLAT,
	DDR_DFI_TPHY_WRCSLAT,
	DDR_RD_DBI_EN,
	DDR_WR_DBI_EN,
	DDR_DM_EN,
	DDR_ADDRMAP_CS_BIT0,
	DDR_ADDRMAP_BANK_B2,
	DDR_ADDRMAP_BANK_B1,
	DDR_ADDRMAP_BANK_B0,
	DDR_ADDRMAP_COL_B5,
	DDR_ADDRMAP_COL_B4,
	DDR_ADDRMAP_COL_B3,
	DDR_ADDRMAP_COL_B2,
	DDR_ADDRMAP_COL_B9,
	DDR_ADDRMAP_COL_B8,
	DDR_ADDRMAP_COL_B7,
	DDR_ADDRMAP_COL_B6,
	DDR_ADDRMAP_COL_B11,
	DDR_ADDRMAP_COL_B10,
	DDR_ADDRMAP_ROW_B11,
	DDR_ADDRMAP_ROW_B2_10,
	DDR_ADDRMAP_ROW_B1,
	DDR_ADDRMAP_ROW_B0,
	DDR_LPDDR3_6GB_12GB,
	DDR_ADDRMAP_ROW_B15,
	DDR_ADDRMAP_ROW_B14,
	DDR_ADDRMAP_ROW_B13,
	DDR_ADDRMAP_ROW_B12,
	DDR_ADDRMAP_ROW_B17,
	DDR_ADDRMAP_ROW_B16,
	DDR_ADDRMAP_BG_B1,
	DDR_ADDRMAP_BG_B0,
	DDR_ADDRMAP_ROW_B5,
	DDR_ADDRMAP_ROW_B4,
	DDR_ADDRMAP_ROW_B3,
	DDR_ADDRMAP_ROW_B2,
	DDR_ADDRMAP_ROW_B9,
	DDR_ADDRMAP_ROW_B8,
	DDR_ADDRMAP_ROW_B7,
	DDR_ADDRMAP_ROW_B6,
	DDR_ADDRMAP_ROW_B10,
	DDR_WR_ODT_HOLD,
	DDR_WR_ODT_DELAY,
	DDR_RD_ODT_HOLD,
	DDR_RD_ODT_DELAY,
	DDR_RANK1_RD_ODT,
	DDR_RANK1_WR_ODT,
	DDR_RANK0_RD_ODT,
	DDR_RANK0_WR_ODT,
	DDR_RDWR_IDLE_GAP,
	DDR_GO2CRITICAL_HYSTERESIS,
	DDR_LPR_NUM_ENTRIES,
	DDR_PAGECLOSE,
	DDR_PREFER_WRITE,
	DDR_FORCE_LOW_PRI_N,
	DDR_LPR_XACT_RUN_LENGTH,
	DDR_LPR_MAX_STARVE,
	DDR_W_XACT_RUN_LENGTH,
	DDR_W_MAX_STARVE,
	DDR_DQ_NIBBLE_MAP_12_15,
	DDR_DQ_NIBBLE_MAP_8_11,
	DDR_DQ_NIBBLE_MAP_4_7,
	DDR_DQ_NIBBLE_MAP_0_3,
	DDR_DQ_NIBBLE_MAP_28_31,
	DDR_DQ_NIBBLE_MAP_24_27,
	DDR_DQ_NIBBLE_MAP_20_23,
	DDR_DQ_NIBBLE_MAP_16_19,
	DDR_DQ_NIBBLE_MAP_44_47,
	DDR_DQ_NIBBLE_MAP_40_43,
	DDR_DQ_NIBBLE_MAP_36_39,
	DDR_DQ_NIBBLE_MAP_32_35,
	DDR_DQ_NIBBLE_MAP_60_63,
	DDR_DQ_NIBBLE_MAP_56_59,
	DDR_DQ_NIBBLE_MAP_52_55,
	DDR_DQ_NIBBLE_MAP_48_51,
	DDR_DQ_NIBBLE_MAP_CB_4_7,
	DDR_DQ_NIBBLE_MAP_CB_0_3,
	DDR_DIS_DQ_RANK_SWAP,
	DDR_DIS_COLLISION_PAGE_OPT,
	DDR_DIS_WC,
	DDR_HW_REF_ZQ_EN,
	DDR_CTRLUPD,
	DDR_ZQ_CALIB_SHORT,
	DDR_RANK1_REFRESH,
	DDR_RANK0_REFRESH,
	DDR_SW_DONE,
	DDR_BL_EXP_MODE,
	DDR_PAGEMATCH_LIMIT,
	DDR_GO2CRITICAL_EN,
	DDR_RD_PORT_PAGEMATCH_EN,
	DDR_RD_PORT_URGENT_EN,
	DDR_RD_PORT_AGING_EN,
	DDR_RD_PORT_PRIORITY,
	DDR_WR_PORT_PAGEMATCH_EN,
	DDR_WR_PORT_URGENT_EN,
	DDR_WR_PORT_AGING_EN,
	DDR_WR_PORT_PRIORITY,
	DDR_PORT_EN,
	DDR_RQOS_MAP_TIMEOUTR,
	DDR_RQOS_MAP_TIMEOUTB,
	DDR_RQOS_MAP_REGION2,
	DDR_RQOS_MAP_REGION1,
	DDR_RQOS_MAP_REGION0,
	DDR_RQOS_MAP_LEVEL2,
	DDR_RQOS_MAP_LEVEL1,
	DDR_RQOS_MAP_LEVEL11,
	DDR_WQOS_MAP_REGION1,
	DDR_WQOS_MAP_REGION0,
	DDR_WQOS_MAP_LEVEL,
	DDR_WQOS_MAP_TIMEOUT,
	DDR_BASE_ADDR,
	DDR_NBLOCKS
};

/* PHY Register fields */
enum {
	PHY_ADCP,
	PHY_RESERVED_30_27,
	PHY_PHYFRST1,
	PHY_OSCACDL,
	PHY_RESERVED_23_19,
	PHY_DTOSEL,
	PHY_OSCDIV1,
	PHY_OSCEN1,
	PHY_CLRTSTAT,
	PHY_CLRZCAL,
	PHY_CLRPERR,
	PHY_ICPC,
	PHY_DTPMXTMR,
	PHY_INITFSMBYP,
	PHY_PLLFSMBYP,
	PHY_TREFPRD,
	PHY_CKNEN,
	PHY_CKEN,
	PHY_GATEACRDCLK,
	PHY_GATEACDDRCLK,
	PHY_GATEACCTLCLK,
	PHY_DDLBYPMODE,
	PHY_IOLB0,
	PHY_RDMODE0,
	PHY_DISRST0,
	PHY_CLKLEVEL0,
	PHY_FRQBT,
	PHY_FRQAT,
	PHY_DISCNPERIOD,
	PHY_VREF_RBCTRL,
	PHY_DXREFISELRANGE,
	PHY_DDLPGACT,
	PHY_DDLPGRW,
	PHY_TPLLPD,
	PHY_TPLLGS,
	PHY_TPHYRST,
	PHY_TPLLLOCK,
	PHY_TPLLRST,
	PHY_PLLBYP,
	PHY_PLLRST,
	PHY_PLLPD,
	PHY_RSTOPM,
	PHY_FRQSEL,
	PHY_RLOCKM,
	PHY_CPPC,
	PHY_CPIC,
	PHY_GSHIFT,
	PHY_ATOEN,
	PHY_ATC,
	PHY_DTC,
	PHY_RDBICLSEL,
	PHY_RDBICL,
	PHY_PHYZUEN,
	PHY_RSTOE,
	PHY_SDRMODE,
	PHY_ATOAE,
	PHY_DTOOE,
	PHY_DTOIOM,
	PHY_DTOPDR,
	PHY_DTOODT,
	PHY_PUAD,
	PHY_CUAEN,
	PHY_CTLZUEN,
	PHY_RESERVED_1,
	PHY_PUREN,
	PHY_0_GPR0,
	PHY_GPR1,
	PHY_GEARDN,
	PHY_UBG,
	PHY_UDIMM,
	PHY_DDR2T,
	PHY_NOSRA,
	PHY_BYTEMASK,
	PHY_DDRTYPE,
	PHY_MPRDQ,
	PHY_PDQ,
	PHY_DDR8BNK,
	PHY_DDRMD,
	PHY_TRRD,
	PHY_TRAS,
	PHY_TRP,
	PHY_TRTP,
	PHY_TWLMRD,
	PHY_TFAW,
	PHY_TMOD,
	PHY_TMRD,
	PHY_TRTW,
	PHY_TRTODT,
	PHY_TCKE,
	PHY_TXS,
	PHY_TOFDX,
	PHY_TCCD,
	PHY_TDLLK,
	PHY_TDQSCKMAX,
	PHY_TDQSCK,
	PHY_TAOND_TAOFD,
	PHY_TRFC,
	PHY_TWLO,
	PHY_TXP,
	PHY_TRC,
	PHY_TRCD,
	PHY_TWTR,
	PHY_PUBWLEN,
	PHY_PUBRLEN,
	PHY_PUBWL,
	PHY_PUBRL,
	PHY_QCSEN,
	PHY_RDIMMIOM,
	PHY_ERROUTOE,
	PHY_ERROUTIOM,
	PHY_ERROUTPDR,
	PHY_ERROUTODT,
	PHY_LRDIMM,
	PHY_PARINIOM,
	PHY_RNKMRREN,
	PHY_SOPERR,
	PHY_ERRNOREG,
	PHY_RDIMM,
	PHY_A17BID,
	PHY_TBCMRD_L2,
	PHY_TBCMRD_L,
	PHY_TBCMRD,
	PHY_TBCSTAB,
	PHY_RC7,
	PHY_RC6,
	PHY_RC5,
	PHY_RC4,
	PHY_RC3,
	PHY_RC2,
	PHY_RC1,
	PHY_RC0,
	PHY_RC15,
	PHY_RC14,
	PHY_RC13,
	PHY_RC12,
	PHY_RC11,
	PHY_RC10,
	PHY_RC9,
	PHY_RC8,
	PHY_RSVD_6_5,
	PHY_RSVD_2_0,
	PHY_RDPST,
	PHY_NWR,
	PHY_RDPRE,
	PHY_WRPRE,
	PHY_BL,
	PHY_WRLEV,
	PHY_WLS,
	PHY_WL0,
	PHY_RL,
	PHY_DBIWR,
	PHY_DBIRD,
	PHY_PDDS,
	PHY_RSVD,
	PHY_WRPST,
	PHY_PUCAL,
	PHY_RSVD_15_13,
	PHY_WRP,
	PHY_RDP,
	PHY_RPTM,
	PHY_SRA,
	PHY_CS2CMDL,
	PHY_IVM,
	PHY_TCRM,
	PHY_TCRR,
	PHY_MPDM,
	PHY_RSVD_0,
	PHY_RDBI0,
	PHY_WDBI0,
	PHY_DM,
	PHY_CAPPE,
	PHY_RTTPARK,
	PHY_ODTIBPD,
	PHY_CAPES,
	PHY_CRCEC,
	PHY_CAPM,
	PHY_TCCDL,
	PHY_RSVD_9_8,
	PHY_VDDQTEN,
	PHY_VDQTRG,
	PHY_VDQTVAL,
	PHY_RSVD_7,
	PHY_CAODT,
	PHY_RSVD_3,
	PHY_DQODT,
	PHY_VREFCA_RANGE,
	PHY_VREFCA,
	PHY_FSPOP,
	PHY_FSPWR,
	PHY_DMD,
	PHY_RRO,
	PHY_VRCG,
	PHY_VRO,
	PHY_RPT,
	PHY_CBT,
	PHY_VREFDQ_RANGE,
	PHY_VREFDQ,
	PHY_ODTD_CA,
	PHY_ODTE_CS,
	PHY_ODTE_CK,
	PHY_CODT,
	PHY_RFSHDT,
	PHY_DTDRS,
	PHY_DTEXG,
	PHY_DTEXD,
	PHY_DTDSTP,
	PHY_DTDEN,
	PHY_DTDBS,
	PHY_DTRDBITR,
	PHY_DTWBDDM,
	PHY_RFSHENT,
	PHY_DTCMPD,
	PHY_DTMPR,
	PHY_DTRPTN,
	PHY_RANKEN,
	PHY_DTRANK,
	PHY_RDLVLGDIFF,
	PHY_RDLVLGS,
	PHY_RDPRMVL_TRN,
	PHY_RDLVLEN,
	PHY_BSTEN,
	PHY_CACD,
	PHY_CAADR,
	PHY_CA1BYTE1,
	PHY_CA1BYTE0,
	PHY_DFTDLY,
	PHY_DFTZQUP,
	PHY_DFTDDLUP,
	PHY_DFTRDSPC,
	PHY_DFTB2BRD,
	PHY_DFTIDLRD,
	PHY_RESERVED_11_8,
	PHY_DFTGPULSE,
	PHY_DFTUPMODE,
	PHY_DFTDTMODE,
	PHY_DFTDTEN,
	PHY_SEED,
	PHY_RESERVED_31_16,
	PHY_ODTOEMODE,
	PHY_ACSR,
	PHY_RSTIOM,
	PHY_RSTPDR,
	PHY_RSTODT,
	PHY_CKDCC,
	PHY_ACPDRMODE,
	PHY_ACODTMODE,
	PHY_ACRANKCLKSEL,
	PHY_CLKGENCLKGATE,
	PHY_ACOECLKGATE0,
	PHY_ACPDRCLKGATE0,
	PHY_ACTECLKGATE0,
	PHY_CKNCLKGATE0,
	PHY_CKCLKGATE0,
	PHY_ACCLKGATE0,
	PHY_PAROEMODE,
	PHY_BGOEMODE,
	PHY_BAOEMODE,
	PHY_A17OEMODE,
	PHY_A16OEMODE,
	PHY_ACTOEMODE,
	PHY_CKOEMODE,
	PHY_LBCLKGATE,
	PHY_ACOECLKGATE1,
	PHY_ACPDRCLKGATE1,
	PHY_ACTECLKGATE1,
	PHY_CKNCLKGATE1,
	PHY_CKCLKGATE1,
	PHY_ACCLKGATE1,
	PHY_ACREFIOM,
	PHY_ACREFPEN,
	PHY_ACREFEEN,
	PHY_ACREFSEN,
	PHY_ACREFIEN,
	PHY_ACREFESELRANGE,
	PHY_ACREFESEL,
	PHY_ACREFSSELRANGE,
	PHY_ACREFSSEL,
	PHY_ACVREFISELRANGE,
	PHY_ACVREFISEL,
	PHY_TVREF,
	PHY_DVEN,
	PHY_PDAEN,
	PHY_VWCR,
	PHY_DVSS,
	PHY_DVMAX,
	PHY_DVMIN,
	PHY_DVINIT,
	PHY_HVSS,
	PHY_HVMAX,
	PHY_HVMIN,
	PHY_SHRNK,
	PHY_SHREN,
	PHY_TVREFIO,
	PHY_EOFF,
	PHY_ENUM,
	PHY_HVEN,
	PHY_HVIO,
	PHY_PARBD,
	PHY_A16BD,
	PHY_A17BD,
	PHY_ACTBD,
	PHY_BG1BD,
	PHY_BG0BD,
	PHY_BA1BD,
	PHY_BA0BD,
	PHY_A03BD,
	PHY_A02BD,
	PHY_A01BD,
	PHY_A00BD,
	PHY_A07BD,
	PHY_A06BD,
	PHY_A05BD,
	PHY_A04BD,
	PHY_A11BD,
	PHY_A10BD,
	PHY_A09BD,
	PHY_A08BD,
	PHY_A15BD,
	PHY_A14BD,
	PHY_A13BD,
	PHY_A12BD,
	PHY_ZQREFISELRANGE,
	PHY_PGWAIT_FRQB,
	PHY_PGWAIT_FRQA,
	PHY_ZQREFPEN,
	PHY_ZQREFIEN,
	PHY_ODT_MODE,
	PHY_FORCE_ZCAL_VT_UPDATE,
	PHY_IODLMT,
	PHY_AVGEN,
	PHY_AVGMAX,
	PHY_ZCALT,
	PHY_PD_DRV_ZDEN,
	PHY_PU_DRV_ZDEN,
	PHY_PD_ODT_ZDEN,
	PHY_PU_ODT_ZDEN,
	PHY_ZSEGBYP,
	PHY_ZLE_MODE,
	PHY_ODT_ADJUST,
	PHY_PD_DRV_ADJUST,
	PHY_PU_DRV_ADJUST,
	PHY_ZPROG_DRAM_ODT,
	PHY_ZPROG_HOST_ODT,
	PHY_ZPROG_ASYM_DRV_PD,
	PHY_ZPROG_ASYM_DRV_PU,
	PHY_CALBYP,
	PHY_MDLEN,
	PHY_CODTSHFT,
	PHY_DQSDCC,
	PHY_RDDLY,
	PHY_DQSNSEPDR,
	PHY_DQSSEPDR,
	PHY_RTTOAL,
	PHY_RTTOH,
	PHY_CPDRSHFT,
	PHY_DQSRPD,
	PHY_DQSGPDR,
	PHY_DQSGODT,
	PHY_DQSGOE,
	PHY_DXPDRMODE,
	PHY_QSNSEL,
	PHY_QSSEL,
	PHY_OEEN,
	PHY_PDREN,
	PHY_TEEN,
	PHY_DSEN,
	PHY_DMEN,
	PHY_DQEN,
	PHY_RDBVT,
	PHY_WDBVT,
	PHY_RGLVT,
	PHY_RDLVT,
	PHY_WDLVT,
	PHY_WLLVT,
	PHY_DSNOEMODE,
	PHY_DSNTEMODE,
	PHY_DSNPDRMODE,
	PHY_DMOEMODE,
	PHY_DMTEMODE,
	PHY_DMPDRMODE,
	PHY_DSOEMODE,
	PHY_DSTEMODE,
	PHY_DSPDRMODE,
	PHY_DXREFIOM,
	PHY_DXREFPEN,
	PHY_DXREFEEN,
	PHY_DXREFSEN,
	PHY_DXREFESELRANGE,
	PHY_DXREFESEL,
	PHY_DXREFSSELRANGE,
	PHY_DXREFSSEL,
	PHY_DXREFIEN,
	PHY_DXREFIMON,
	PHY_DXREFISELR3,
	PHY_DXREFISELR2,
	PHY_DXREFISELR1,
	PHY_DXREFISELR0,
	PHY_DXDQVREFR3,
	PHY_DXDQVREFR2,
	PHY_DXDQVREFR1,
	PHY_DXDQVREFR0,
	PHY_DXOEMODE,
	PHY_DXTEMODE,
	PHY_GATEDXRDCLK,
	PHY_GATEDXDDRCLK,
	PHY_GATEDXCTLCLK,
	PHY_CLKLEVEL,
	PHY_LBMODE,
	PHY_LBGSDQS,
	PHY_LBDGDQS,
	PHY_LBDQSS,
	PHY_PHYHRST,
	PHY_PHYFRST,
	PHY_DLTST,
	PHY_DLTMODE,
	PHY_RESERVED_12_11,
	PHY_OSCWDDL,
	PHY_RESERVED_8_7,
	PHY_OSCWDL,
	PHY_OSCDIV,
	PHY_OSCEN,
	PHY_RRRMODE,
	PHY_WRRMODE,
	PHY_DQSGX,
	PHY_LPPLLPD,
	PHY_LPIOPD,
	PHY_QSCNTEN,
	PHY_UDQIOM,
	PHY_DXSR,
	PHY_DQSNRES,
	PHY_DQSRES,
	PHY_CRDEN,
	PHY_POSOEX,
	PHY_PREOEX,
	PHY_IOAG,
	PHY_IOLB,
	PHY_LPWAKEUP_THRSH,
	PHY_RDBI,
	PHY_WDBI,
	PHY_PRFBYP,
	PHY_RDMODE,
	PHY_DISRST,
	PHY_DQSGLB,
	PHY_DXDACRANGE,
	PHY_DXVREFIOM,
	PHY_DXIOM,
	PHY_DXTXM,
	PHY_DXRXM,
	PHY_ZCALBYP,
	PHY_DCALPSE,
	PHY_DQS2DQ,
	PHY_RDIMMINIT,
	PHY_CTLDINIT,
	PHY_VREF,
	PHY_SRD,
	PHY_WREYE,
	PHY_RDEYE,
	PHY_WRDSKW,
	PHY_RDDSKW,
	PHY_WLADJ,
	PHY_QSGATE,
	PHY_WL,
	PHY_DRAMINIT,
	PHY_DRAMRST,
	PHY_PHYRST,
	PHY_DCAL,
	PHY_PLLINIT,
	PHY_CA,
	PHY_ZCAL,
	PHY_INIT,
	PHY_R060,
	PHY_R061,
	PHY_R062,
	PHY_R063,
	PHY_R064,
	PHY_R065,
	PHY_R066
};

/************************** Variable Definitions *****************************/

struct Ddr3SpdEeprom {
	/* General Section: Bytes 0U-59U */
	u8 InfoSizeCrc;   /*  0U # bytes written into serial memory,
					     CRC coverage */
	u8 SpdRev;         /*  1U Total # bytes of SPD mem device */
	u8 MemType;        /*  2U Key Byte / Fundamental mem type */
	u8 ModuleType;     /*  3U Key Byte / Module Type */
	u8 DensityBanks;   /*  4U SDRAM Density and Banks */
	u8 Addressing;      /*  5U SDRAM Addressing */
	u8 ModuleVdd;      /*  6U Module nominal voltage, VDD */
	u8 Organization;    /*  7U Module Organization */
	u8 BusWidth;       /*  8U Module Memory Bus Width */
	u8 FtbDiv;         /*  9U Fine Timebase (FTB)
					     Dividend / Divisor */
	u8 MtbDividend;    /* 10U Medium Timebase (MTB) Dividend */
	u8 MtbDivisor;     /* 11U Medium Timebase (MTB) Divisor */
	u8 TckMin;         /* 12U SDRAM Minimum Cycle Time */
	u8 Res13;          /* 13U Reserved */
	u8 CaslatLsb;      /* 14U CAS Latencies Supported,
					     Least Significant Byte */
	u8 CaslatMsb;      /* 15U CAS Latencies Supported,
					     Most Significant Byte */
	u8 TaaMin;         /* 16U Min CAS Latency Time */
	u8 TwrMin;         /* 17U Min Write REcovery Time */
	u8 TrcdMin;        /* 18U Min RAS# to CAS# Delay Time */
	u8 TrrdMin;        /* 19U Min Row Active to
					     Row Active Delay Time */
	u8 TrpMin;         /* 20U Min Row Precharge Delay Time */
	u8 TrasTrcExt;    /* 21U Upper Nibbles for tRAS and tRC */
	u8 TrasMinLsb;    /* 22U Min Active to Precharge
					     Delay Time */
	u8 TrcMinLsb;     /* 23U Min Active to Active/Refresh
					     Delay Time, LSB */
	u8 TrfcMinLsb;    /* 24U Min Refresh Recovery Delay Time */
	u8 TrfcMinMsb;    /* 25U Min Refresh Recovery Delay Time */
	u8 TwtrMin;        /* 26U Min Internal Write to
					     Read Command Delay Time */
	u8 TrtpMin;        /* 27U Min Internal Read to Precharge
					     Command Delay Time */
	u8 TfawMsb;        /* 28U Upper Nibble for tFAW */
	u8 TfawMin;        /* 29U Min Four Activate Window
					     Delay Time*/
	u8 OptFeatures;    /* 30U SDRAM Optional Features */
	u8 ThermRefOpt;   /* 31U SDRAM Thermal and Refresh Opts */
	u8 ThermSensor;    /* 32U Module Thermal Sensor */
	u8 DeviceType;     /* 33U SDRAM device type */
	s8 FineTckMin;	       /* 34U Fine offset for tCKmin */
	s8 FineTaaMin;	       /* 35U Fine offset for tAAmin */
	s8 FineTrcdMin;	       /* 36U Fine offset for tRCDmin */
	s8 FineTrpMin;	       /* 37U Fine offset for tRPmin */
	s8 FineTrcMin;	       /* 38U Fine offset for tRCmin */
	u8 Res3959[21U];   /* 39U-59U Reserved, General Section */

	/* Module-Specific Section: Bytes 60U-116U */
	union {
		struct {
			/* 60U (Unbuffered) Module Nominal Height */
			u8 ModHeight;
			/* 61U (Unbuffered) Module Maximum Thickness */
			u8 ModThickness;
			/* 62U (Unbuffered) Reference Raw Card Used */
			u8 RefRawCard;
			/* 63U (Unbuffered) Address Mapping from
			      Edge Connector to DRAM */
			u8 AddrMapping;
			/* 64U-116U (Unbuffered) Reserved */
			u8 Res64116[53U];
		} unbuffered;
		struct {
			/* 60U (Registered) Module Nominal Height */
			u8 ModHeight;
			/* 61U (Registered) Module Maximum Thickness */
			u8 ModThickness;
			/* 62U (Registered) Reference Raw Card Used */
			u8 RefRawCard;
			/* 63U DIMM Module Attributes */
			u8 ModuAttr;
			/* 64U RDIMM Thermal Heat Spreader Solution */
			u8 Thermal;
			/* 65U Register Manufacturer ID Code, Least Significant Byte */
			u8 RegIdLo;
			/* 66U Register Manufacturer ID Code, Most Significant Byte */
			u8 RegIdHi;
			/* 67U Register Revision Number */
			u8 RegRev;
			/* 68U Register Type */
			u8 RegType;
			/* 69U-76U RC1,3U,5...15 (MS Nibble) / RC0,2U,4...14 (LS Nibble) */
			u8 Rcw[8U];
		} registered;
		u8 Uc[57U]; /* 60U-116U Module-Specific Section */
	} ModSection;

	/* Unique Module ID: Bytes 117U-125U */
	u8 MmidLsb;        /* 117U Module MfgID Code LSB - JEP-106U */
	u8 MmidMsb;        /* 118U Module MfgID Code MSB - JEP-106U */
	u8 Mloc;            /* 119U Mfg Location */
	u8 Mdate[2U];        /* 120U-121U Mfg Date */
	u8 Sernum[4U];       /* 122U-125U Module Serial Number */

	/* CRC: Bytes 126U-127U */
	u8 Crc[2U];          /* 126U-127U SPD CRC */

	/* Other Manufacturer Fields and User Space: Bytes 128U-255U */
	u8 Mpart[18U];       /* 128U-145U Mfg's Module Part Number */
	u8 Mrev[2U];         /* 146U-147U Module Revision Code */

	u8 DmidLsb;        /* 148U DRAM MfgID Code LSB - JEP-106U */
	u8 DmidMsb;        /* 149U DRAM MfgID Code MSB - JEP-106U */

	u8 Msd[26U];         /* 150U-175U Mfg's Specific Data */
	u8 Cust[80U];        /* 176U-255U Open for Customer Use */

};

/* From JEEC Standard No. 21U-C release 23A */
struct Ddr4SpdEeprom {
	/* General Section: Bytes 0U-127U */
	u8 InfoSizeCrc;		/*  0U # bytes */
	u8 SpdRev;		/*  1U Total # bytes of SPD */
	u8 MemType;		/*  2U Key Byte / mem type */
	u8 ModuleType;		/*  3U Key Byte / Module Type */
	u8 DensityBanks;		/*  4U Density and Banks	*/
	u8 Addressing;		/*  5U Addressing */
	u8 PackageType;		/*  6U Package type */
	u8 OptFeature;		/*  7U Optional features */
	u8 ThermalRef;		/*  8U Thermal and refresh */
	u8 OthOptFeatures;	/*  9U Other optional features */
	u8 Res10;			/* 10U Reserved */
	u8 ModuleVdd;		/* 11U Module nominal voltage */
	u8 Organization;		/* 12U Module Organization */
	u8 BusWidth;		/* 13U Module Memory Bus Width */
	u8 ThermSensor;		/* 14U Module Thermal Sensor */
	u8 ExtType;		/* 15U Extended module type */
	u8 Res16;
	u8 Timebases;		/* 17U MTb and FTB */
	u8 TckMin;		/* 18U tCKAVGmin */
	u8 TckMax;		/* 19U TCKAVGmax */
	u8 CaslatB1;		/* 20U CAS latencies, 1st byte */
	u8 CaslatB2;		/* 21U CAS latencies, 2nd byte */
	u8 CaslatB3;		/* 22U CAS latencies, 3rd byte */
	u8 CaslatB4;		/* 23U CAS latencies, 4th byte */
	u8 TaaMin;		/* 24U Min CAS Latency Time */
	u8 TrcdMin;		/* 25U Min RAS# to CAS# Delay Time */
	u8 TrpMin;		/* 26U Min Row Precharge Delay Time */
	u8 TrasTrcExt;		/* 27U Upper Nibbles for tRAS and tRC */
	u8 TrasMinLsb;		/* 28U tRASmin, lsb */
	u8 TrcMinLsb;		/* 29U tRCmin, lsb */
	u8 Trfc1MinLsb;		/* 30U Min Refresh Recovery Delay Time */
	u8 Trfc1MinMsb;		/* 31U Min Refresh Recovery Delay Time */
	u8 Trfc2MinLsb;		/* 32U Min Refresh Recovery Delay Time */
	u8 Trfc2MinMsb;		/* 33U Min Refresh Recovery Delay Time */
	u8 Trfc4MinLsb;		/* 34U Min Refresh Recovery Delay Time */
	u8 Trfc4MinMsb;		/* 35U Min Refresh Recovery Delay Time */
	u8 TfawMsb;		/* 36U Upper Nibble for tFAW */
	u8 TfawMin;		/* 37U tFAW, lsb */
	u8 TrrdsMin;		/* 38U tRRD_Smin, MTB */
	u8 TrrdlMin;		/* 39U tRRD_Lmin, MTB */
	u8 TccdlMin;		/* 40U tCCS_Lmin, MTB */
	u8 Res41[60U-41U];		/* 41U Rserved */
	u8 Mapping[78U-60U];		/* 60U~77U Connector to SDRAM bit map */
	u8 Res78[117U-78U];		/* 78U~116U, Reserved */
	s8 FineTccdlMin;		/* 117U Fine offset for tCCD_Lmin */
	s8 FineTrrdlMin;		/* 118U Fine offset for tRRD_Lmin */
	s8 FineTrrdsMin;		/* 119U Fine offset for tRRD_Smin */
	s8 FineTrcMin;		/* 120U Fine offset for tRCmin */
	s8 FineTrpMin;		/* 121U Fine offset for tRPmin */
	s8 FineTrcdMin;		/* 122U Fine offset for tRCDmin */
	s8 FineTaaMin;		/* 123U Fine offset for tAAmin */
	s8 FineTckMax;		/* 124U Fine offset for tCKAVGmax */
	s8 FineTckMin;		/* 125U Fine offset for tCKAVGmin */
	/* CRC: Bytes 126U-127U */
	u8 Crc[2U];			/* 126U-127U SPD CRC */

	/* Module-Specific Section: Bytes 128U-255U */
	union {
		struct {
			/* 128U (Unbuffered) Module Nominal Height */
			u8 ModHeight;
			/* 129U (Unbuffered) Module Maximum Thickness */
			u8 ModThickness;
			/* 130U (Unbuffered) Reference Raw Card Used */
			u8 RefRawCard;
			/* 131U (Unbuffered) Address Mapping from
			      Edge Connector to DRAM */
			u8 AddrMapping;
			/* 132U~253U (Unbuffered) Reserved */
			u8 Res132[254U-132U];
			/* 254U~255U CRC */
			u8 Crc[2U];
		} unbuffered;
		struct {
			/* 128U (Registered) Module Nominal Height */
			u8 ModHeight;
			/* 129U (Registered) Module Maximum Thickness */
			u8 ModThickness;
			/* 130U (Registered) Reference Raw Card Used */
			u8 RefRawCard;
			/* 131U DIMM Module Attributes */
			u8 ModuAttr;
			/* 132U RDIMM Thermal Heat Spreader Solution */
			u8 Thermal;
			/* 133U Register Manufacturer ID Code, LSB */
			u8 RegIdLo;
			/* 134U Register Manufacturer ID Code, MSB */
			u8 RegIdHi;
			/* 135U Register Revision Number */
			u8 RegRev;
			/* 136U Address mapping from register to DRAM */
			u8 RegMap;
			/* 137U~253U Reserved */
			u8 Res137[254U-137U];
			/* 254U~255U CRC */
			u8 Crc[2U];
		} registered;
		struct {
			/* 128U (Loadreduced) Module Nominal Height */
			u8 ModHeight;
			/* 129U (Loadreduced) Module Maximum Thickness */
			u8 ModThickness;
			/* 130U (Loadreduced) Reference Raw Card Used */
			u8 RefRawCard;
			/* 131U DIMM Module Attributes */
			u8 ModuAttr;
			/* 132U RDIMM Thermal Heat Spreader Solution */
			u8 Thermal;
			/* 133U Register Manufacturer ID Code, LSB */
			u8 RegIdLo;
			/* 134U Register Manufacturer ID Code, MSB */
			u8 RegIdHi;
			/* 135U Register Revision Number */
			u8 RegRev;
			/* 136U Address mapping from register to DRAM */
			u8 RegMap;
			/* 137U Register Output Drive Strength for CMD/Add*/
			u8 RegDrv;
			/* 138U Register Output Drive Strength for CK */
			u8 RegDrvCk;
			/* 139U Data Buffer Revision Number */
			u8 DataBufRev;
			/* 140U DRAM VrefDQ for Package Rank 0U */
			u8 VrefqeR0;
			/* 141U DRAM VrefDQ for Package Rank 1U */
			u8 VrefqeR1;
			/* 142U DRAM VrefDQ for Package Rank 2U */
			u8 VrefqeR2;
			/* 143U DRAM VrefDQ for Package Rank 3U */
			u8 VrefqeR3;
			/* 144U Data Buffer VrefDQ for DRAM Interface */
			u8 DataIntf;
			/*
			 * 145U Data Buffer MDQ Drive Strength and RTT
			 * for data rate <= 1866U
			 */
			u8 DataDrv1866;
			/*
			 * 146U Data Buffer MDQ Drive Strength and RTT
			 * for 1866U < data rate <= 2400U
			 */
			u8 DataDrv2400;
			/*
			 * 147U Data Buffer MDQ Drive Strength and RTT
			 * for 2400U < data rate <= 3200U
			 */
			u8 DataDrv3200;
			/* 148U DRAM Drive Strength */
			u8 DramDrv;
			/*
			 * 149U DRAM ODT (RTT_WR, RTT_NOM)
			 * for data rate <= 1866U
			 */
			u8 DramOdt1866;
			/*
			 * 150U DRAM ODT (RTT_WR, RTT_NOM)
			 * for 1866U < data rate <= 2400U
			 */
			u8 DramOdt2400;
			/*
			 * 151U DRAM ODT (RTT_WR, RTT_NOM)
			 * for 2400U < data rate <= 3200U
			 */
			u8 DramOdt3200;
			/*
			 * 152U DRAM ODT (RTT_PARK)
			 * for data rate <= 1866U
			 */
			u8 DramOdtPark1866;
			/*
			 * 153U DRAM ODT (RTT_PARK)
			 * for 1866U < data rate <= 2400U
			 */
			u8 DramOdtPark2400;
			/*
			 * 154U DRAM ODT (RTT_PARK)
			 * for 2400U < data rate <= 3200U
			 */
			u8 DramOdtPark3200;
			u8 Res155[254U-155U];	/* Reserved */
			/* 254U~255U CRC */
			u8 Crc[2U];
		} LoadReduced;
		u8 Uc[128U]; /* 128U-255U Module-Specific Section */
	} ModSection;

	u8 Res256[320U-256U];	/* 256U~319U Reserved */

	/* Module supplier's data: Byte 320U~383U */
	u8 MmidLsb;		/* 320U Module MfgID Code LSB */
	u8 MmidMsb;		/* 321U Module MfgID Code MSB */
	u8 Mloc;			/* 322U Mfg Location */
	u8 Mdate[2U];		/* 323U~324U Mfg Date */
	u8 Sernum[4U];		/* 325U~328U Module Serial Number */
	u8 Mpart[20U];		/* 329U~348U Mfg's Module Part Number */
	u8 Mrev;			/* 349U Module Revision Code */
	u8 DmidLsb;		/* 350U DRAM MfgID Code LSB */
	u8 DmidMsb;		/* 351U DRAM MfgID Code MSB */
	u8 Stepping;		/* 352U DRAM stepping */
	u8 Msd[29U];		/* 353U~381U Mfg's Specific Data */
	u8 Res382[2U];		/* 382U~383U Reserved */

	u8 User[512U-384U];		/* 384U~511U End User Programmable */
};

/* From JEEC Standard No. 21U-C release 23A */
struct LpDdrSpdEeprom {
	/* General Section: Bytes 0U-127U */
	u8 InfoSizeCrc;		/*  0U # bytes */
	u8 SpdRev;		/*  1U Total # bytes of SPD */
	u8 MemType;		/*  2U Key Byte / mem type */
	u8 ModuleType;		/*  3U Key Byte / Module Type */
	u8 DensityBanks;		/*  4U Density and Banks	*/
	u8 Addressing;		/*  5U Addressing */
	u8 PackageType;		/*  6U Package type */
	u8 OptFeature;		/*  7U Optional features */
	u8 ThermalRef;		/*  8U Thermal and refresh */
	u8 OthOptFeatures;	/*  9U Other optional features */
	u8 Res10;			/* 10U Reserved */
	u8 ModuleVdd;		/* 11U Module nominal voltage */
	u8 Organization;		/* 12U Module Organization */
	u8 BusWidth;		/* 13U Module Memory Bus Width */
	u8 ThermSensor;		/* 14U Module Thermal Sensor */
	u8 ExtType;		/* 15U Extended module type */
	u8 SignalLoading;	/* 16U Signal Loading */
	u8 Timebases;		/* 17U MTb and FTB */
	u8 TckMin;		/* 18U tCKAVGmin */
	u8 TckMax;		/* 19U TCKAVGmax */
	u8 CaslatB1;		/* 20U CAS latencies, 1st byte */
	u8 CaslatB2;		/* 21U CAS latencies, 2nd byte */
	u8 CaslatB3;		/* 22U CAS latencies, 3rd byte */
	u8 CaslatB4;		/* 23U CAS latencies, 4th byte */
	u8 TaaMin;		/* 24U Min CAS Latency Time */
	u8 RdWrLatSet;		/* 25U Read & Write Latency Set Options */
	u8 TrcdMin;		/* 26U Min RAS# to CAS# Delay Time */
	u8 TrpabMin;		/* 27U All banks Min Row Precharge Delay Time */
	u8 TrppbMin;		/* 28U Per bank Min Row Precharge Delay Time */
	u8 TrfcabMinLsb;		/* 29U All banks Min Refresh Recovery Delay Time */
	u8 TrfcabMinMsb;		/* 30U All banks Min Refresh Recovery Delay Time */
	u8 TrfcpbMinLsb;		/* 31U Per bank Min Refresh Recovery Delay Time */
	u8 TrfcpbMinMsb;		/* 32U Per bank Min Refresh Recovery Delay Time */
	u8 Res33[60U-33U];		/* 33U Rserved */
	u8 Mapping[78U-60U];		/* 60U~77U Connector to SDRAM bit map */
	u8 Res78[120U-78U];		/* 78U~119U, Reserved */
	s8 FineTrppbMin;		/* 120U Fine offset for tRPpbmin */
	s8 FineTrpabMin;		/* 121U Fine offset for tRPabmin */
	s8 FineTrcdMin;		/* 122U Fine offset for tRCDmin */
	s8 FineTaaMin;		/* 123U Fine offset for tAAmin */
	s8 FineTckMax;		/* 124U Fine offset for tCKAVGmax */
	s8 FineTckMin;		/* 125U Fine offset for tCKAVGmin */
	/* CRC: Bytes 126U-127U */
	u8 Crc[2U];			/* 126U-127U SPD CRC */

	/* Module-Specific Section: Bytes 128U-255U */
	union {
		struct {
			/* 128U (Unbuffered) Module Nominal Height */
			u8 ModHeight;
			/* 129U (Unbuffered) Module Maximum Thickness */
			u8 ModThickness;
			/* 130U (Unbuffered) Reference Raw Card Used */
			u8 RefRawCard;
			/* 131U~253U (Unbuffered) Reserved */
			u8 Res132[254U-131U];
			/* 254U~255U CRC */
			u8 Crc[2U];
		} lpdimm;
		u8 Uc[128U]; /* 128U-255U Module-Specific Section */
	} ModSection;

	u8 Res256[320U-256U];	/* 256U~319U Reserved */

	/* Module supplier's data: Byte 320U~383U */
	u8 MmidLsb;		/* 320U Module MfgID Code LSB */
	u8 MmidMsb;		/* 321U Module MfgID Code MSB */
	u8 Mloc;			/* 322U Mfg Location */
	u8 Mdate[2U];		/* 323U~324U Mfg Date */
	u8 Sernum[4U];		/* 325U~328U Module Serial Number */
	u8 Mpart[20U];		/* 329U~348U Mfg's Module Part Number */
	u8 Mrev;			/* 349U Module Revision Code */
	u8 DmidLsb;		/* 350U DRAM MfgID Code LSB */
	u8 DmidMsb;		/* 351U DRAM MfgID Code MSB */
	u8 Stepping;		/* 352U DRAM stepping */
	u8 Msd[29U];		/* 353U~381U Mfg's Specific Data */
	u8 Res382[2U];		/* 382U~383U Reserved */

	u8 User[512U-384U];		/* 384U~511U End User Programmable */
};

/* Parameters for a DDR dimm computed from the SPD */
typedef struct {
	/* DIMM organization parameters */
	s8 Mpart[19U];		/* guaranteed null terminated */

	u32 MemType;
	u32 NRanks;
	u64 RankDensity;
	u64 Capacity;
	u32 BusWidth;
	u32 PrimaryBusWidth;
	u32 EccBusWidth;
	u32 UDimm;
	u32 RDimm;
	u32 SpeedBin;
	u32 DramWidth;	/* x4, x8, x16 components */

	/* SDRAM device parameters */
	u32 NumRankAddr;
	u32 NumRowAddr;
	u32 NumColAddr;
	u32 EdcConfig;	/* 0U = none, 1U = parity, 2U = ECC */
	u32 NumBankAddr;
	u32 NumBgAddr;
	u32 NBanksPerSdramDevice;
	u32 BurstLength;	/* BL=4U bit 2U, BL=8U = bit 3U */
	u32 RowDensity;

	/* used in computing base address of DIMMs */
	u64 BaseAddress;
	/* mirrored DIMMs */
	u32 AddrMirror;	/* only for ddr3 */

	/* DIMM timing parameters */

	u32 MtbPs;	/* medium timebase ps */
	u32 Ftb10thPs; /* fine timebase, in 1U/10U ps */
	u32 TaaPs;	/* minimum CAS latency time */
	u32 TFawPs;	/* four active window delay */

	/*
	 * SDRAM clock periods
	 * The range for these are 1000U-10000U so a short should be sufficient
	 */
	u32 TckminXPs;
	u32 TckminXMinus1Ps;
	u32 TckminXMinus2Ps;
	u32 TckmaxPs;
	float ClockPeriod;
	float FreqMhz;

	/* SPD-defined CAS latencies */
	u32 CaslatX;
	u32 CaslatXMinus1;
	u32 CaslatXMinus2;

	u32 CaslatLowestDerated;	/* Derated CAS latency */
	u32 CasLatency;
	u32 CasWriteLatency;
	u32 AdditiveLatency;
	u32 WriteLatency;
	u32 ReadLatency;

	/* basic timing parameters */
	u32 TRcdPs;
	u32 TRpPs;
	u32 TrpabPs;
	u32 TrppbPs;
	u32 TRasPs;

	u32 TRfc1Ps;
	u32 TRfc2Ps;
	u32 TRfc4Ps;
	u32 TRfcAbPs;
	u32 TRfcPbPs;
	u32 TRrdsPs;
	u32 TRrdlPs;
	u32 TCcdlPs;
	u32 TwrPs;	/* maximum = 63750U ps */
	u32 TRfcPs;	/* max = 255U ns + 256U ns + .75 ns
				       = 511750U ps */
	u32 TRrdPs;	/* maximum = 63750U ps */
	u32 TwtrPs;	/* maximum = 63750U ps */
	u32 TrtpPs;	/* byte 38U, spd->trtp */

	u32 TRcPs;	/* maximum = 254U ns + .75 ns = 254750U ps */

	u32 TRefi;
	float TXp;
	float TMod;
	u32 TWtr;

    u32 Vref;
    u32 Ecc;
    u32 TRefMode;
    u32 TRefRange;
    u32 RdDbi;
    u32 WrDbi;
    u32 PhyDbiMode;
    u32 DataMask;
    u32 EccScrub;
    u32 EccPoison;
    u32 Parity;
    u32 ParityLatency;
    u32 En2tTimingMode;
    u32 Geardown;
    u32 MaxPwrSavEn;
    u32 CalModeEn;
    u32 DeepPwrDnEn;
    u32 PwrDnEn;
    u32 Crc;
    u32 WrPreamble;
    u32 RdPreamble;
    u32 RdPostamble;
    u32 WrPostamble;
    u32 Lpddr4Hynix;
    u32 Lpddr4Samsung;
    u32 Video;
    u32 Decoder;
    u32 TileWidth;
    u32 TileHeight;
    u32 PerBankRefresh;
    u32 Fgrm;
    u32 LpAsr;
    u32 DisDfiLpSr;
    u32 DisDfiLpPd;
    u32 DisDfiLpMpsm;
    u32 Ddr4AddrMapping;
    u32 BrcMapping;
    u32 PllByp;
    u32 FreqB;
    u32 StaticRdMode;
    u32 SelfRefAbort;
    u32 Gls;
    u32 Slowboot;
    u32 En2ndClk;
    u32 Dp;
    u32 HasEccComp;
    u32 ClockStopEn;
    u32 Zcu100;
    u32 DdriobCtrl;
    u32 WrDrift;
    u32 RdDrift;
    u32 GateExt;
    u32 NoGateExtNoTrain;
    u32 Preoex;
    u32 DerateIntD;
    u32 Zc1650;
    u32 Zc1656;
    u32 RdbiWrkAround;
    u32 Lp4catrain;
    u32 RdDqsCenter;
    u32 NoDerate;
    u32 HostOdt;
    u32 HostDrv;
    u32 DramOdt;
    u32 DramDrv;
    u32 DramCaOdt;
    u32 DeskewTrn;
    u32 DisOpInv;
    u32 PhyClkGate;
    u32 NoRetry;
    u32 Lp4FmaxWrkAround;
    u32 EnOpInvAfterTrain;
    u32 Dqmap03;
    u32 Dqmap47;
    u32 Dqmap811;
    u32 Dqmap1215;
    u32 Dqmap1619;
    u32 Dqmap2023;
    u32 Dqmap2427;
    u32 Dqmap2831;
    u32 Dqmap3235;
    u32 Dqmap3639;
    u32 Dqmap4043;
    u32 Dqmap4447;
    u32 Dqmap4851;
    u32 Dqmap5255;
    u32 Dqmap5659;
    u32 Dqmap6063;
    u32 Dqmap6467;
    u32 Dqmap6871;
    u32 Ddp;
    u32 LprNumEntries;
    u32 PllRetry;
    u32 UseSetB;
    u32 Lp4NoOdt;
    u32 Mr;
    u32 Emr;
    u32 Emr2;
    u32 Emr3;
    u32 Mr4;
    u32 Mr5;
    u32 Mr6;
    u32 WdqsOn;
    u32 WdqsOff;
    u32 CtlClkFreq;
} XFsbl_DimmParams;

/* DDR Initialization Data Structure */
struct DdrcInitData {
	u32 AddrMapRowBit[XFSBL_MAX_ROWS]; /* Row Bits in Address Map */
	u32 AddrMapColBit[XFSBL_MAX_COLUMNS]; /* Column Bits in Address Map */
	u32 AddrMapBankBit[XFSBL_MAX_BANKS]; /* Bank Bits in Address Map */
	u32 AddrMapBgBit[XFSBL_MAX_BANK_GROUPS]; /* BG Bits in Address Map */
	u32 AddrMapCsBit0; /* Rank Bits in Address Map */
	u32 AddrMapRowBits2To10; /* Row Bits (2U-10U) in Address Map */
	XFsbl_DimmParams PDimm;
};

u32 XFsbl_DdrInit(void);

#endif /* XPAR_DYNAMIC_DDR_ENABLED */
#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_DDR_INIT_H */
#endif /* XFSBL_PS_DDR */
