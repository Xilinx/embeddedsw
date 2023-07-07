/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
 *       mn   07/29/20 Modify code to use DRAM VRef for 2D Write Eye Test
 * 1.2   mn   02/11/21 Added support for 16-Bit Bus Width
 *       mn   05/24/21 Fixed Eye Test issue with higher rank
 *       mn   05/27/21 Get the PS Ref Clk from design
 * 1.3   mn   09/08/21 Removed illegal write to DXnGTR0.WDQSL register field
 * 1.4   mn   11/29/21 Usability Enhancements for 2D Read/Write Eye
 * 1.5   sg   05/05/22 Fixed GCC warnings
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
#ifndef SDT
#include "xtime_l.h"
#else
#include "xiltimer.h"
#endif

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

#ifdef XPAR_PSU_PSS_REF_CLK_FREQ_HZ
#define XMT_REF_FREQ				(XPAR_PSU_PSS_REF_CLK_FREQ_HZ / 1000000.0)
#else
#define XMT_REF_FREQ				33.3333
#endif

/* DDR Controller Register Definitions */
#define XMT_DDRC_MSTR				0xFD070000
#define XMT_DDRC_MSTR_DEVICE_CONFIG_SHIFT	30
#define XMT_DDRC_MSTR_DEVICE_CONFIG_MASK	0xc0000000
#define XMT_DDRC_MSTR_ACTIVE_RANKS_SHIFT	24
#define XMT_DDRC_MSTR_ACTIVE_RANKS_MASK		0x03000000
#define XMT_DDRC_MSTR_DATA_BUS_WIDTH_SHIFT	12
#define XMT_DDRC_MSTR_DATA_BUS_WIDTH_MASK	0x00003000
#define XMT_DDRC_MSTR_DDR_TYPE_SHIFT		0
#define XMT_DDRC_MSTR_DDR_TYPE_MASK		0x0000003F

#define XMT_DDR_ECC_CONFIG0			0xFD070070
#define XMT_DDR_ECC_CONFIG0_ECC_MODE_MASK	0x7
#define XMT_DDR_ECC_CONFIG0_ECC_MODE_SHIFT	0

#define XMT_DDRC_MRCTRL0			0xFD070010
#define XMT_DDRC_MRCTRL0_MR_WR_SHIFT		31
#define XMT_DDRC_MRCTRL0_MR_WR_MASK		0x80000000
#define XMT_DDRC_MRCTRL0_MR_ADDR_SHIFT		12
#define XMT_DDRC_MRCTRL0_MR_ADDR_MASK		0x0000F000
#define XMT_DDRC_MRCTRL0_MR_RANK_SHIFT		4
#define XMT_DDRC_MRCTRL0_MR_RANK_MASK		0x00000030
#define XMT_DDRC_MRCTRL0_SW_INIT_INT_SHIFT	3
#define XMT_DDRC_MRCTRL0_SW_INIT_INT_MASK	0x00000008
#define XMT_DDRC_MRCTRL0_PDA_EN_SHIFT		2
#define XMT_DDRC_MRCTRL0_PDA_EN_MASK		0x00000004
#define XMT_DDRC_MRCTRL0_MPR_EN_SHIFT		1
#define XMT_DDRC_MRCTRL0_MPR_EN_MASK		0x00000002
#define XMT_DDRC_MRCTRL0_MR_TYPE_SHIFT		0
#define XMT_DDRC_MRCTRL0_MR_TYPE_MASK		0x00000001

#define XMT_DDRC_MRCTRL1			0xFD070014
#define XMT_DDRC_MRCTRL1_MR_DATA_SHIFT		0
#define XMT_DDRC_MRCTRL1_MR_DATA_MASK		0x0003FFFF

#define XMT_DDRC_MRSTAT				0xFD070018
#define XMT_DDRC_MRSTAT_PDA_DONE_SHIFT		8
#define XMT_DDRC_MRSTAT_PDA_DONE_MASK		0x00000100
#define XMT_DDRC_MRSTAT_MR_WR_BUSY_SHIFT	0
#define XMT_DDRC_MRSTAT_MR_WR_BUSY_MASK		0x00000001

#define XMT_DDRC_DERATEEN			0xFD070020
#define XMT_DDRC_DERATEEN_DERATE_ENABLE_SHIFT	0
#define XMT_DDRC_DERATEEN_DERATE_ENABLE_MASK	0x00000001

#define XMT_DDRC_INIT4				0xFD0700E0
#define XMT_DDRC_INIT4_EMR3_SHIFT	0U
#define XMT_DDRC_INIT4_EMR3_MASK	0x0000FFFFU

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

#define XMT_DDR_PHY_DX0GCR5_DXREFISELR1_SHIFT	8
#define XMT_DDR_PHY_DX0GCR5_DXREFISELR1_MASK	0x7F00

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

/* DDR QOS Registers */

#define XMT_DDRC_MRR_STATUS			0xFD090518U
#define XMT_DDRC_MRR_DATA0			0xFD09051CU
#define XMT_DDRC_MRR_DATA1			0xFD090520U
#define XMT_DDRC_MRR_DATA2			0xFD090524U
#define XMT_DDRC_MRR_DATA3			0xFD090528U
#define XMT_DDRC_MRR_DATA4			0xFD09052CU
#define XMT_DDRC_MRR_DATA5			0xFD090530U
#define XMT_DDRC_MRR_DATA6			0xFD090534U
#define XMT_DDRC_MRR_DATA7			0xFD090538U
#define XMT_DDRC_MRR_DATA8			0xFD09053CU
#define XMT_DDRC_MRR_DATA9			0xFD090540U
#define XMT_DDRC_MRR_DATA10			0xFD090544U
#define XMT_DDRC_MRR_DATA11			0xFD090548U


/* Results register */
#define XMT_RESULTS_BASE			0xFF410020

#define XMT_LEFT_EYE_TEST			0x0U
#define XMT_RIGHT_EYE_TEST			0x1U
#define XMT_2D_EYE_TEST			0x2U
#define XMT_DDR_CONFIG_8_LANE			8U
#define XMT_DDR_CONFIG_4_LANE			4U
#define XMT_DDR_CONFIG_2_LANE			2U
#define XMT_DDR_CONFIG_64BIT_WIDTH			64U
#define XMT_DDR_CONFIG_32BIT_WIDTH			32U
#define XMT_DDR_CONFIG_16BIT_WIDTH			16U

#define XMT_DDR_TYPE_DDR3			0x01U
#define XMT_DDR_TYPE_LPDDR2			0x04U
#define XMT_DDR_TYPE_LPDDR3			0x08U
#define XMT_DDR_TYPE_DDR4			0x10U
#define XMT_DDR_TYPE_LPDDR4			0x20U

#define XMT_DDR_MR3_CONFIG_MASK			0x0000E7F8U
#define XMT_DDR_MR3_MPR_P2_CONFIG		0x00000006U

#define XMT_DDR_MR_ADDR_MR0			0x0000U
#define XMT_DDR_MR_ADDR_MR1			0x1000U
#define XMT_DDR_MR_ADDR_MR2			0x2000U
#define XMT_DDR_MR_ADDR_MR3			0x3000U
#define XMT_DDR_MR_ADDR_MR4			0x4000U
#define XMT_DDR_MR_ADDR_MR5			0x5000U
#define XMT_DDR_MR_ADDR_MR6			0x6000U
#define XMT_DDR_MR_ADDR_MR7			0x7000U

#define XMT_DDR_MR_ADDR_MR14		0xE00U
#define XMT_DDR_VREF_CALIB_MODE_EN	0x80U
#define XMT_DDR_VREF_CALIB_MODE_DIS	0x00U


#define XMT_DDR_MR_RANK_0			0x10U
#define XMT_DDR_MR_RANK_1			0x20U
#define XMT_DDR_MR_RANK_1_2			0x30U

#define XMT_DDR_MR_WR				0x80000000U
#define XMT_DDR_MPR_ENABLE			0x2U
#define XMT_DDR_MRS_ENABLE			0x0U
#define XMT_DDR_MR_READ				0x1U
#define XMT_DDR_MR_WRITE			0x0U

#define XMT_DDR_MR_WR_BUSY			0x1U

#define XMT_DDRC_MRR_STATUS_VALID	0x1U
#define XMT_DDRC_MRR_DATA_U8_MASK	0xFFU
#define XMT_DDRC_MRR_DATA_WIDTH		8U

#define XMT_MAX_WR_VREF				0x32U
#define XMT_MAX_WR_VREF_DDR4_R1		0x16U
#define XMT_MAX_WR_VREF_LPDDR4_R1	0x1CU

#define XMT_RD_VREF_INTERVAL		0.362913
#define XMT_RD_MIN_VREF				7.73

#define XMT_WR_VREF_INTERVAL_DDR4	0.65
#define XMT_WR_MIN_VREF_DDR4_R1		60.0
#define XMT_WR_MIN_VREF_DDR4_R2		45.0

#define XMT_WR_VREF_INTERVAL_LPDDR4	0.4
#define XMT_WR_MIN_VREF_LPDDR4_R1	10.0
#define XMT_WR_MIN_VREF_LPDDR4_R2	22.0

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XMT_CHECK_BIT(var, pos) ((var) & (1<<(pos)))
#define XMT_YLFSR(a) ((a << 1) + (((a >> 60) & 1) ^ ((a >> 54) & 1) ^ 1))
#define XMT_RANDOM_VALUE(x) (0x12345678+19*(XMT_YLFSR(x))+0x017c1e2313567c9b)

/*****************************************************************************/
/**
 * This function is used to set Register with mask value
 *
 * @param Offset	DDR configuration register offset
 * @param Mask 	Data mask
 * @param Value	Register value
 *
 * @return none
 *
 *****************************************************************************/
static INLINE void XMt_MaskWrite(u32 Offset, u32 Mask, u32 Value)
{
	u32 RegVal = 0x0;
	RegVal = Xil_In32(Offset);
	RegVal &= ~(Mask);
	RegVal |= (Value & Mask);
	Xil_Out32(Offset, RegVal);
}

/*****************************************************************************/
/**
 * This function is used to set Register value
 *
 * @param Addr	DDR configuration register address
 * @param Mask	Data mask
 * @param Shift	Bit shift value
 * @param Value	Register value
 *
 * @return none
 *
 *****************************************************************************/
static INLINE void XMt_SetRegValue(u32 Addr, u32 Mask, u32 Shift, u32 Value)
{
	u32 Rd = 0;
	Rd  = Xil_In32(Addr);
	Rd  = Rd & (~Mask);
	Rd  = Rd | (Value << Shift);
	Xil_Out32(Addr, Rd);
}

/*****************************************************************************/
/**
 * This function is used to get Register value
 *
 * @param Ddr	DDR configuration register address
 * @param Mask	Mask to the data
 * @param Shift Bit shift value
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
	u32 VRefAutoWr;
	XMt_ReadCenter RdCenter[8];
	u32 ReadCenterFetched;
	XMt_WriteCenter WrCenter[8];
	u32 WriteCenterFetched;
	XMt_WriteDs WrDs[8];
	double TapPs;
	double DdrFreq;
	u32 DdrType;
	u32 RankSel;
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
u32 XMt_GetWrVRef(XMt_CfgData *XMtPtr);
void XMt_SetWrVref(XMt_CfgData *XMtPtr, u32 VRef);
void XMt_ResetWrVref(XMt_CfgData *XMtPtr);
void XMt_Print2DReadEyeResults(XMt_CfgData *XMtPtr, u32 VRef);
void XMt_Print2DWriteEyeResultsR1(XMt_CfgData *XMtPtr, u32 VRef);
void XMt_Print2DWriteEyeResultsR2(XMt_CfgData *XMtPtr, u32 VRef);
u32 XMt_GetVRefAutoMin(XMt_CfgData *XMtPtr);
u32 XMt_GetVRefAutoMax(XMt_CfgData *XMtPtr);

#ifdef __cplusplus
}
#endif

#endif /* __XMT_COMMON_H */
