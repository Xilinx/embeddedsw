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
 *       mn   07/30/18 Define some DDR registers addresses if not defined
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
/***************************** Include Files *********************************/
#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)
#include "psu_init.h"

/************************** Constant Definitions *****************************/
/* Maximum number of Rows in DDR */
#define XFSBL_MAX_ROWS				18
/* Maximum number of Columns in DDR */
#define XFSBL_MAX_COLUMNS			12
/* Maximum number of Banks in DDR */
#define XFSBL_MAX_BANKS				3
/* Maximum number of Bank Groups in DDR */
#define XFSBL_MAX_BANK_GROUPS			2
/* Total number of DDR controller registers */
#define XFSBL_DDRC_REG_COUNT			115
/* Total number of DDR PHY registers */
#define XFSBL_DDR_PHY_REG_COUNT			127

/* In some older designs, this register is not used in psu_init flow */
#ifndef DDR_PHY_GPR0_OFFSET
#define DDR_PHY_GPR0_OFFSET			0XFD0800C0
#endif
#ifndef DDRC_RFSHCTL1_OFFSET
#define DDRC_RFSHCTL1_OFFSET			0XFD070054
#endif
#ifndef DDRC_DFIUPD0_OFFSET
#define DDRC_DFIUPD0_OFFSET			0XFD0701A0
#endif
#ifndef DDRC_DQMAP0_OFFSET
#define DDRC_DQMAP0_OFFSET			0XFD070280
#endif
#ifndef DDRC_DQMAP1_OFFSET
#define DDRC_DQMAP1_OFFSET			0XFD070284
#endif
#ifndef DDRC_DQMAP2_OFFSET
#define DDRC_DQMAP2_OFFSET			0XFD070288
#endif
#ifndef DDRC_DQMAP3_OFFSET
#define DDRC_DQMAP3_OFFSET			0XFD07028C
#endif
#ifndef DDRC_DQMAP4_OFFSET
#define DDRC_DQMAP4_OFFSET			0XFD070290
#endif
#ifndef DDR_PHY_PLLCR0_OFFSET
#define DDR_PHY_PLLCR0_OFFSET			0XFD080068
#endif
#ifndef DDR_PHY_DQSDR0_OFFSET
#define DDR_PHY_DQSDR0_OFFSET			0XFD080250
#endif
#ifndef DDR_PHY_DX8SL0PLLCR0_OFFSET
#define DDR_PHY_DX8SL0PLLCR0_OFFSET		0XFD081404
#endif
#ifndef DDR_PHY_DX8SL1PLLCR0_OFFSET
#define DDR_PHY_DX8SL1PLLCR0_OFFSET		0XFD081444
#endif
#ifndef DDR_PHY_DX8SL2PLLCR0_OFFSET
#define DDR_PHY_DX8SL2PLLCR0_OFFSET		0XFD081484
#endif
#ifndef DDR_PHY_DX8SL3PLLCR0_OFFSET
#define DDR_PHY_DX8SL3PLLCR0_OFFSET		0XFD0814C4
#endif
#ifndef DDR_PHY_DX8SL4PLLCR0_OFFSET
#define DDR_PHY_DX8SL4PLLCR0_OFFSET		0XFD081504
#endif
#ifndef DDR_PHY_DX8SLBPLLCR0_OFFSET
#define DDR_PHY_DX8SLBPLLCR0_OFFSET		0XFD0817C4
#endif


/************************** Variable Definitions *****************************/
u32 XFsbl_DdrInit(void);
unsigned long psu_ddr_phybringup_data(void);

/* DDR Initialization Data Structure */
struct DdrcInitData {
	u32 DdrcRegOfst[XFSBL_DDRC_REG_COUNT]; /* DDR Register Offsets */
	u32 PhyRegOfst[XFSBL_DDR_PHY_REG_COUNT]; /* DDR PHY Register Offsets */
	u32 DdrcReg[XFSBL_DDRC_REG_COUNT]; /* DDR Register Values */
	u32 PhyReg[XFSBL_DDR_PHY_REG_COUNT]; /* DDR PHY Register Values */
	u32 AddrMapRowBit[XFSBL_MAX_ROWS]; /* Row Bits in Address Map */
	u32 AddrMapColBit[XFSBL_MAX_COLUMNS]; /* Column Bits in Address Map */
	u32 AddrMapBankBit[XFSBL_MAX_BANKS]; /* Bank Bits in Address Map */
	u32 AddrMapBgBit[XFSBL_MAX_BANK_GROUPS]; /* BG Bits in Address Map */
	u32 AddrMapCsBit0; /* Rank Bits in Address Map */
	u32 AddrMapRowBits2To10; /* Row Bits (2-10) in Address Map */
	u8 SpdData[20]; /* SPD Data */
};

#endif /* defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106) */
#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_DDR_INIT_H */
