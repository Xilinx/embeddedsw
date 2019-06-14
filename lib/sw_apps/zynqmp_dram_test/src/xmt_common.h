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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xmt_common.h
 *
 * This is the header file containing all the Global defines and function
 * declarations for ZynqMP DRAM Test.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   mn   08/17/18 Initial release
 *       mn   09/27/18 Modify code to add 2D Read/Write Eye Tests support
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef __XMT_COMMON_H
#define __XMT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"

#include "xil_printf.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xpseudo_asm.h"
#include "xdebug.h"
#include "xtime_l.h"

/************************** Constant Definitions *****************************/

/* Byte to KByte (multiplier macro) */
#define XMT_KB2BYTE				1024
/* Byte to MByte (multiplier macro) */
#define XMT_MB2BYTE				(1024*1024)
/* 64-Bit Data Mask */
#define U64_MASK				0xFFFFFFFFFFFFFFFFU

/* Clock and Control Registers */
#define XMT_DPLL_CTRL				0xFD1A002C
#define XMT_VPLL_CTRL				0xFD1A0038
#define XMT_DDR_CTRL				0xFD1A0080

#define XMT_PLL_FBDIV_SHIFT			8
#define XMT_PLL_FBDIV_MASK			0x00007F00
#define XMT_PLL_DIV2_SHIFT			16
#define XMT_PLL_DIV2_MASK			0x00010000

#define XMT_SOURCE_DIV0_SHIFT			8
#define XMT_SOURCE_DIV0_MASK			0x00003F00
#define XMT_SOURCE_SRCSEL_SHIFT			0
#define XMT_SOURCE_SRCSEL_MASK			0x00000007

#define XMT_REF_FREQ				33.3333

/* DDR Controller Register Definitions */
#define XMT_DDRC_MSTR				0xFD070000
#define XMT_DDRC_MSTR_DEVICE_CONFIG_SHIFT	30
#define XMT_DDRC_MSTR_DEVICE_CONFIG_MASK	0xc0000000
#define XMT_DDRC_MSTR_ACTIVE_RANKS_SHIFT	24
#define XMT_DDRC_MSTR_ACTIVE_RANKS_MASK		0x03000000
#define XMT_DDRC_MSTR_DATA_BUS_WIDTH_SHIFT	12
#define XMT_DDRC_MSTR_DATA_BUS_WIDTH_MASK	0x00003000

#define XMT_DDR_ECC_CONFIG0			0xFD070070
#define XMT_DDR_ECC_CONFIG0_ECC_MODE_MASK	0x7
#define XMT_DDR_ECC_CONFIG0_ECC_MODE_SHIFT	0

#define XMT_DDRC_DERATEEN			0xFD070020
#define XMT_DDRC_DERATEEN_DERATE_ENABLE_SHIFT	0
#define XMT_DDRC_DERATEEN_DERATE_ENABLE_MASK	0x00000001

#define XMT_DDRC_DFIUPD0			0xFD0701A0
#define XMT_DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_SHIFT	31
#define XMT_DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_MASK	0x80000000

#define XMT_DDRC_SWCTL				0xFD070320
#define XMT_DDRC_SWCTL_SW_DONE_SHIFT		0
#define XMT_DDRC_SWCTL_SW_DONE_MASK		0x00000001

/* DDR PHY Register Definitions */
#define XMT_DDR_PHY_PIR				0xFD080004
#define XMT_DDR_PHY_PIR_INIT_SHIFT		0

#define XMT_DDR_PHY_PGCR6			0xFD080028
#define XMT_DDR_PHY_PGCR6_INHVT_SHIFT		0
#define XMT_DDR_PHY_PGCR6_INHVT_MASK		0x00000001

#define XMT_DDR_PHY_PGSR0			0xFD080030
#define XMT_DDR_PHY_PGSR0_REDONE_SHIFT		10
#define XMT_DDR_PHY_PGSR0_IDONE_SHIFT		0

#define XMT_DDR_PHY_PGSR1			0xFD080034
#define XMT_DDR_PHY_PGSR1_VTSTOP_MASK		0x40000000

#define XMT_DDR_PHY_DSGCR			0xFD080090
#define XMT_DDR_PHY_DSGCR_PUREN_SHIFT		0
#define XMT_DDR_PHY_DSGCR_PUREN_MASK		0x00000001

#define XMT_DDR_PHY_DTCR0			0xFD080200
#define XMT_DDR_PHY_DTCR0_RFSHDT_SHIFT		28
#define XMT_DDR_PHY_DTCR0_RFSHDT_MASK		0xf0000000
#define XMT_DDR_PHY_DTCR0_INCWEYE_SHIFT		4
#define XMT_DDR_PHY_DTCR0_INCWEYE_MASK		0x00000010
#define XMT_DDR_PHY_DQSDR0			0xFD080250
#define XMT_DDR_PHY_DQSDR0_DFTDTEN_SHIFT	0
#define XMT_DDR_PHY_DQSDR0_DFTDTEN_MASK		0x00000001

#define XMT_DDR_PHY_RANKIDR			0xFD0804DC
#define XMT_DDR_PHY_RANKIDR_RANKRID_SHIFT	16
#define XMT_DDR_PHY_RANKIDR_RANKRID_MASK	0x000f0000
#define XMT_DDR_PHY_RANKIDR_RANKWID_SHIFT	0
#define XMT_DDR_PHY_RANKIDR_RANKWID_MASK	0x0000000f

#define XMT_DDR_PHY_DX0GCR5			0xFD080714
#define XMT_DDR_PHY_DX0GCR5_DXREFISELR0_SHIFT	0
#define XMT_DDR_PHY_DX0GCR5_DXREFISELR0_MASK	0x7F

#define XMT_DDR_PHY_DX0LCDLR0			0xFD080780
#define XMT_DDR_PHY_DX0LCDLR0_WLD_SHIFT		0
#define XMT_DDR_PHY_DX0LCDLR0_WLD_MASK		0x000001ff

#define XMT_DDR_PHY_DX0LCDLR1			0xFD080784
#define XMT_DDR_PHY_DX0LCDLR1_WDQD_SHIFT	0
#define XMT_DDR_PHY_DX0LCDLR1_WDQD_MASK		0x000001ff

#define XMT_DDR_PHY_DX0LCDLR3			0xFD08078C
#define XMT_DDR_PHY_DX0LCDLR3_RDQSD_SHIFT	0
#define XMT_DDR_PHY_DX0LCDLR3_RDQSD_MASK	0x000001ff

#define XMT_DDR_PHY_DX0LCDLR4			0xFD080790
#define XMT_DDR_PHY_DX0LCDLR4_DEFVAL		0x0
#define XMT_DDR_PHY_DX0LCDLR4_RDQSND_SHIFT	0
#define XMT_DDR_PHY_DX0LCDLR4_RDQSND_MASK	0x000001ff

#define XMT_DDR_PHY_DX0MDLR0			0xFD0807A0
#define XMT_DDR_PHY_DX0MDLR0_TPRD_SHIFT		16
#define XMT_DDR_PHY_DX0MDLR0_TPRD_MASK		0x01ff0000
#define XMT_DDR_PHY_DX0MDLR0_IPRD_SHIFT		0
#define XMT_DDR_PHY_DX0MDLR0_IPRD_MASK		0x000001ff

#define XMT_DDR_PHY_DX0GTR0			0xFD0807C0
#define XMT_DDR_PHY_DX0GTR0_DEFVAL		0x20000
#define XMT_DDR_PHY_DX0GTR0_WDQSL_SHIFT		24
#define XMT_DDR_PHY_DX0GTR0_WDQSL_MASK		0x07000000
#define XMT_DDR_PHY_DX0GTR0_WLSL_SHIFT		16
#define XMT_DDR_PHY_DX0GTR0_WLSL_MASK		0x000f0000
#define XMT_DDR_PHY_DX0GTR0_DGSL_SHIFT		0
#define XMT_DDR_PHY_DX0GTR0_DGSL_MASK		0x0000001f

#define XMT_LANE0MDLR0				XMT_DDR_PHY_DX0MDLR0

#define XMT_LANE_OFFSET				0x100

/* Results register */
#define XMT_RESULTS_BASE			0xFF410020

#define XMT_LEFT_EYE_TEST			0x0U
#define XMT_RIGHT_EYE_TEST			0x1U
#define XMT_2D_EYE_TEST			0x2U
#define XMT_DDR_CONFIG_8_LANE			8U
#define XMT_DDR_CONFIG_4_LANE			4U
#define XMT_DDR_CONFIG_64BIT_WIDTH			64U
#define XMT_DDR_CONFIG_32BIT_WIDTH			32U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XMT_CHECK_BIT(var, pos) ((var) & (1<<(pos)))
#define XMT_YLFSR(a) ((a << 1) + (((a >> 60) & 1) ^ ((a >> 54) & 1) ^ 1))
#define XMT_RANDOM_VALUE(x) (0x12345678+19*(x)+0x017c1e2313567c9b)

#define XMT_UPDATE_REG(Addr, Mask, Shift, Value) {\
    u32 Rd = 0;\
    Rd  = Xil_In32(Addr);\
    Rd  = Rd & (~Mask);\
    Rd  = Rd | (Value << Shift);\
    Xil_Out32(Addr, Rd);\
}

#define XMT_MASK_WRITE(Offset, Mask, Value) {\
	u32 RegVal = 0x0;\
	RegVal = Xil_In32(Offset);\
	RegVal &= ~(Mask);\
	RegVal |= (Value & Mask);\
	Xil_Out32(Offset, RegVal);\
}

/*****************************************************************************/
/**
 * This function is used to get Register value
 *
 * @param Ddr is the DDR configuration register address
 * @param Mask is the Mask to the data
 * @param Shift is the bit shift value
 *
 * @return Configuration Value
 *
 * @note none
 *****************************************************************************/
static INLINE u32 XMt_GetRegValue(u32 Ddr, u32 Mask, u32 Shift)
{
	return (Xil_In32(Ddr) & Mask) >> Shift;
}

/************************** Variable Definitions *****************************/

/* Read Center Data */
typedef struct {
	u32 Qsd;
	u32 Qsnd;
} XMt_ReadCenter;

/* Write Center Data */
typedef struct {
	u32 Wdqd;
	u32 Wdqsl;
	u32 Iprd;
	u32 Tprd;
	u32 Dgsl;
} XMt_WriteCenter;

/* Write Eye Data */
typedef struct {
	u32 Wdqd;
	u32 Wdqsl;
	u32 Wlsl;
	u32 Wld;
} XMt_WriteDs;

/* Memtest Configuration Data */
typedef struct {
	u32 DdrConfigLanes;
	u32 DdrConfigRanks;
	u32 BusWidth;
	u32 EccEnabled;
	u32 DCacheEnable;
	u32 TapCount[8];
	s32 EyeStart[8];
	s32 EyeEnd[8];
	u32 VRefAuto[8];
	XMt_ReadCenter RdCenter[8];
	XMt_WriteCenter WrCenter[8];
	XMt_WriteDs WrDs[8];
	double TapPs;
	double DdrFreq;
} XMt_CfgData;

/************************** Function Prototypes ******************************/

void XMt_SelectRank(s32 Rank);
double XMt_CalcPerTapDelay(XMt_CfgData *XMtPtr, u32 Lane);
void XMt_ClearEye(XMt_CfgData *XMtPtr, u32 *Addr);
void XMt_ClearResults(XMt_CfgData *XMtPtr, u32 Addr);
void XMt_DisableVtcomp(void);
void XMt_EnableVtcomp(void);
void XMt_EnableRefresh(void);
void XMt_DisableRefresh(void);
void XMt_DfiEnable(void);
void XMt_DfiDisable(void);
double XMt_GetDdrcFreq(void);
void XMt_PrintHelp(void);
void XMt_PrintMemTestHeader(XMt_CfgData *XMtPtr);
void XMt_PrintLine(XMt_CfgData *XMtPtr, u8 LineCode);
void XMt_PrintResults(XMt_CfgData *XMtPtr);
void XMt_PrintEyeHeader(XMt_CfgData *XMtPtr);
void XMt_PrintEyeResultsHeader(XMt_CfgData *XMtPtr);
void XMt_Print2DEyeResultsHeader(XMt_CfgData *XMtPtr);
void XMt_RunEyeMemtest(XMt_CfgData *XMtPtr, u64 StartAddr, u32 Len);
void XMt_PrintDdrConfigParams(XMt_CfgData *XMtPtr);
u32 XMt_MeasureWrEye(XMt_CfgData *XMtPtr, u64 TestAddr, u32 Len);
u32 XMt_MeasureRdEye(XMt_CfgData *XMtPtr, u64 TestAddr, u32 Len);
u32 XMt_MeasureWrEye2D(XMt_CfgData *XMtPtr, u64 TestAddr, u32 Len);
u32 XMt_MeasureRdEye2D(XMt_CfgData *XMtPtr, u64 TestAddr, u32 Len);
u32 XMt_GetDdrConfigParams(XMt_CfgData *XMtPtr);
double XMt_PllFreq(u32 Ddr);
u32 XMt_GetVRefAuto(XMt_CfgData *XMtPtr);
void XMt_SetVrefVal(XMt_CfgData *XMtPtr, u32 VRef);
void XMt_ResetVrefAuto(XMt_CfgData *XMtPtr);
void XMt_Print2DEyeResults(XMt_CfgData *XMtPtr, u32 VRef);
u32 XMt_GetVRefAutoMin(XMt_CfgData *XMtPtr);
u32 XMt_GetVRefAutoMax(XMt_CfgData *XMtPtr);

#ifdef __cplusplus
}
#endif

#endif /* __XMT_COMMON_H */
