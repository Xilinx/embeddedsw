/*******************************************************************************
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
 * OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xmt_common.c
 *
 * This file contains code for common functions which are used by Read/Write
 * Eye tests and the Memory Tests.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   mn   08/17/18 Initial release
 *       mn   09/21/18 Modify code manually enter the DDR memory test size
 *       mn   09/27/18 Modify code to add 2D Read/Write Eye Tests support
 *       mn   12/17/18 Limit VRefMin to minimum of 0 for 2D eye scan
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xmt_common.h"

/************************** Constant Definitions *****************************/

#define XMT_LANE0GCR5_OFFSET	XMT_DDR_PHY_DX0GCR5
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
 * This function is used to get DDR configuration parameters
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return returns the error codes described in xerror.h on any error
 *			returns XST_SUCCESS on success, XST_FAILURE on failure
 *
 * @note none
 *****************************************************************************/
u32 XMt_GetDdrConfigParams(XMt_CfgData *XMtPtr)
{
	u32 Status;
	u32 DdrConfigRanks;
	u32 BusWidth;
	u32 EccEnabled;
	u32 DdrDiv0;
	u32 DdrSrcsel;

	/* Get the DDR Bus Width and the Number of Lanes */
	BusWidth = XMt_GetRegValue(XMT_DDRC_MSTR,
				   XMT_DDRC_MSTR_DATA_BUS_WIDTH_MASK,
				   XMT_DDRC_MSTR_DATA_BUS_WIDTH_SHIFT);
	if (BusWidth == 0x0U) {
		XMtPtr->BusWidth = XMT_DDR_CONFIG_64BIT_WIDTH;
		XMtPtr->DdrConfigLanes = XMT_DDR_CONFIG_8_LANE;
	} else if (BusWidth == 0x1U) {
		XMtPtr->BusWidth = XMT_DDR_CONFIG_32BIT_WIDTH;
		XMtPtr->DdrConfigLanes = XMT_DDR_CONFIG_4_LANE;
	} else {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Get the DDR Ranks */
	DdrConfigRanks = XMt_GetRegValue(XMT_DDRC_MSTR,
					 XMT_DDRC_MSTR_ACTIVE_RANKS_MASK,
					 XMT_DDRC_MSTR_ACTIVE_RANKS_SHIFT);

	if (DdrConfigRanks == 0x1U) {
		XMtPtr->DdrConfigRanks = 1U;
	} else if (DdrConfigRanks == 0x3U) {
		XMtPtr->DdrConfigRanks = 2U;
	} else {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Get the DDR ECC configuration */
	EccEnabled = XMt_GetRegValue(XMT_DDR_ECC_CONFIG0,
				     XMT_DDR_ECC_CONFIG0_ECC_MODE_MASK,
				     XMT_DDR_ECC_CONFIG0_ECC_MODE_SHIFT);
	if (EccEnabled == 0x0U) {
		XMtPtr->EccEnabled = 0U;
	} else if (EccEnabled == 0x4U) {
		XMtPtr->EccEnabled = 1U;
	} else {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Get the DDR clock divisor value */
	DdrDiv0 = XMt_GetRegValue(XMT_DDR_CTRL,
				  XMT_SOURCE_DIV0_MASK,
				  XMT_SOURCE_DIV0_SHIFT);
	/* Get the DDR clock input source */
	DdrSrcsel = XMt_GetRegValue(XMT_DDR_CTRL,
				    XMT_SOURCE_SRCSEL_MASK,
				    XMT_SOURCE_SRCSEL_SHIFT);

	if (DdrSrcsel == 0x0U) {
		XMtPtr->DdrFreq = XMt_PllFreq(XMT_DPLL_CTRL) / (DdrDiv0 * 1.0);
	} else if (DdrSrcsel == 0x1U) {
		XMtPtr->DdrFreq = XMt_PllFreq(XMT_VPLL_CTRL) / (DdrDiv0 * 1.0);
	} else {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to print DDR configuration parameters
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_PrintDdrConfigParams(XMt_CfgData *XMtPtr)
{
	u32 DataRate;
	u32 DdrMstrRegval;

	DataRate = XMtPtr->DdrFreq*4;
	DdrMstrRegval = Xil_In32(XMT_DDRC_MSTR);

	xil_printf("Device information\r\n");
	xil_printf("%d-Rank ", XMtPtr->DdrConfigRanks);

	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_8_LANE) {
		xil_printf("64-bit ");
	} else if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_4_LANE) {
		xil_printf("32-bit ");
	} else {
		xil_printf("UNKNOWN! ");
	}

	if (XMT_CHECK_BIT(DdrMstrRegval, 0U)) {
		xil_printf("DDR3 ");
	} else if (XMT_CHECK_BIT(DdrMstrRegval, 2U)) {
		xil_printf("LPDDR2 ");
	} else if (XMT_CHECK_BIT(DdrMstrRegval, 3U)) {
		xil_printf("LPDDR3 ");
	} else if (XMT_CHECK_BIT(DdrMstrRegval, 4U)) {
		xil_printf("DDR4 ");
	} else if (XMT_CHECK_BIT(DdrMstrRegval, 5U)) {
		xil_printf("LPLPDDR2 ");
	} else {
		xil_printf("UNKNOWN! ");
	}

	xil_printf("%u\r\n", DataRate);
}

/*****************************************************************************/
/**
 * This function is used to select DDR Rank
 *
 * @param Rank is the rank to be selected
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_SelectRank(s32 Rank)
{
	u32 RegVal;

	xil_printf("\r\nSelecting Rank: %d\r\n", Rank);

	RegVal = Xil_In32(XMT_DDR_PHY_RANKIDR);
	RegVal = RegVal & !XMT_DDR_PHY_RANKIDR_RANKRID_MASK &
			!XMT_DDR_PHY_RANKIDR_RANKWID_MASK;
	RegVal = RegVal | ((Rank) << XMT_DDR_PHY_RANKIDR_RANKRID_SHIFT) |
			((Rank) << XMT_DDR_PHY_RANKIDR_RANKWID_SHIFT);
	Xil_Out32(XMT_DDR_PHY_RANKIDR, RegVal);
}

/*****************************************************************************/
/**
 * This function is used to disable VT compensation
 *
 * @param none
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_DisableVtcomp(void)
{
	u32 RdVal, WrVal;
	s32 Index, done = 0;

	RdVal = Xil_In32(XMT_DDR_PHY_PGCR6);
	WrVal = RdVal | (1 << XMT_DDR_PHY_PGCR6_INHVT_SHIFT);
	Xil_Out32(XMT_DDR_PHY_PGCR6, WrVal);

	for (Index = 0; Index < 10; Index++) {
		RdVal = Xil_In32(XMT_DDR_PHY_PGSR1);
		if (RdVal & XMT_DDR_PHY_PGSR1_VTSTOP_MASK) {
			done = 1;
		}
	}

	if (!done) {
		xil_printf("Disabling VT compensation... FAILED\r\n");
	}
}

/*****************************************************************************/
/**
 * This function is used to enable VT compensation
 *
 * @param none
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_EnableVtcomp(void)
{
	u32 RdVal;
	u32 WrVal;

	RdVal = Xil_In32(XMT_DDR_PHY_PGCR6);
	WrVal = RdVal & (!XMT_DDR_PHY_PGCR6_INHVT_MASK);
	Xil_Out32(XMT_DDR_PHY_PGCR6, WrVal);
}

/*****************************************************************************/
/**
 * This function is used to enable Refresh During Training
 *
 * @param none
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_EnableRefresh(void)
{
	XMT_UPDATE_REG(XMT_DDR_PHY_DTCR0, XMT_DDR_PHY_DTCR0_RFSHDT_MASK,
			XMT_DDR_PHY_DTCR0_RFSHDT_SHIFT, 0x8);
}

/*****************************************************************************/
/**
 * This function is used to disable Refresh During Training
 *
 * @param none
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_DisableRefresh(void)
{
	XMT_UPDATE_REG(XMT_DDR_PHY_DTCR0, XMT_DDR_PHY_DTCR0_RFSHDT_MASK,
			XMT_DDR_PHY_DTCR0_RFSHDT_SHIFT, 0x0);
}

/*****************************************************************************/
/**
 * This function is used to enable DFI
 *
 * @param none
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_DfiEnable(void)
{
	XMT_UPDATE_REG(XMT_DDRC_SWCTL, XMT_DDRC_SWCTL_SW_DONE_MASK,
			XMT_DDRC_SWCTL_SW_DONE_SHIFT, 0x0);

	XMT_UPDATE_REG(XMT_DDRC_DFIUPD0, XMT_DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_MASK,
			XMT_DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_SHIFT, 0x0);

	XMT_UPDATE_REG(XMT_DDRC_DERATEEN, XMT_DDRC_DERATEEN_DERATE_ENABLE_MASK,
			XMT_DDRC_DERATEEN_DERATE_ENABLE_SHIFT, 0x0);

	XMT_UPDATE_REG(XMT_DDRC_SWCTL, XMT_DDRC_SWCTL_SW_DONE_MASK,
			XMT_DDRC_SWCTL_SW_DONE_SHIFT, 0x1);

	XMT_UPDATE_REG(XMT_DDR_PHY_DSGCR, XMT_DDR_PHY_DSGCR_PUREN_MASK,
			XMT_DDR_PHY_DSGCR_PUREN_SHIFT, 0x1);

	XMT_UPDATE_REG(XMT_DDR_PHY_DQSDR0, XMT_DDR_PHY_DQSDR0_DFTDTEN_MASK,
			XMT_DDR_PHY_DQSDR0_DFTDTEN_SHIFT, 0x0);

	XMT_UPDATE_REG(XMT_DDR_PHY_DTCR0, XMT_DDR_PHY_DTCR0_INCWEYE_MASK,
			XMT_DDR_PHY_DTCR0_INCWEYE_SHIFT, 0x0);
}

/*****************************************************************************/
/**
 * This function is used to disable DFI
 *
 * @param none
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_DfiDisable(void)
{
	XMT_UPDATE_REG(XMT_DDRC_SWCTL, XMT_DDRC_SWCTL_SW_DONE_MASK,
			XMT_DDRC_SWCTL_SW_DONE_SHIFT, 0x0);

	XMT_UPDATE_REG(XMT_DDRC_DFIUPD0, XMT_DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_MASK,
			XMT_DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_SHIFT, 0x1);

	XMT_UPDATE_REG(XMT_DDRC_DERATEEN, XMT_DDRC_DERATEEN_DERATE_ENABLE_MASK,
			XMT_DDRC_DERATEEN_DERATE_ENABLE_SHIFT, 0x0);

	XMT_UPDATE_REG(XMT_DDRC_SWCTL, XMT_DDRC_SWCTL_SW_DONE_MASK,
			XMT_DDRC_SWCTL_SW_DONE_SHIFT, 0x1);

	XMT_UPDATE_REG(XMT_DDR_PHY_DSGCR, XMT_DDR_PHY_DSGCR_PUREN_MASK,
			XMT_DDR_PHY_DSGCR_PUREN_SHIFT, 0x0);

	XMT_UPDATE_REG(XMT_DDR_PHY_DQSDR0, XMT_DDR_PHY_DQSDR0_DFTDTEN_MASK,
			XMT_DDR_PHY_DQSDR0_DFTDTEN_SHIFT, 0x0);

	XMT_UPDATE_REG(XMT_DDR_PHY_DTCR0, XMT_DDR_PHY_DTCR0_INCWEYE_MASK,
			XMT_DDR_PHY_DTCR0_INCWEYE_SHIFT, 0x0);
}

/*****************************************************************************/
/**
 * This function is used to get PLL Frequency
 *
 * @param Ddr is the DDR register address
 *
 * @return PLL Frequency value
 *
 * @note none
 *****************************************************************************/
double XMt_PllFreq(u32 Ddr)
{
	u32 Multiplier;
	u32 Div2;
	double Freq;

	Multiplier = XMt_GetRegValue(Ddr, XMT_PLL_FBDIV_MASK,
			XMT_PLL_FBDIV_SHIFT);
	Div2 = XMt_GetRegValue(Ddr, XMT_PLL_DIV2_MASK,
			XMT_PLL_DIV2_SHIFT);

	if (Div2) {
		Freq = (XMT_REF_FREQ * Multiplier) / 2.0;
	} else {
		Freq = (XMT_REF_FREQ * Multiplier);
	}

	return Freq;
}

/*****************************************************************************/
/**
 * This function is used to calculate Delay per Tap
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param Lane is the lane number for which delay is required
 *
 * @return Tap Delay Value
 *
 * @note none
 *****************************************************************************/
double XMt_CalcPerTapDelay(XMt_CfgData *XMtPtr, u32 Lane)
{
	u32 Iprd;
	double TapPs;

	Iprd = XMt_GetRegValue(XMT_LANE0MDLR0 + (XMT_LANE_OFFSET * Lane),
			XMT_DDR_PHY_DX0MDLR0_IPRD_MASK,
			XMT_DDR_PHY_DX0MDLR0_IPRD_SHIFT);

	TapPs = (1000000.0 / (XMtPtr->DdrFreq * 4.0)) / (double)Iprd;

	return TapPs;
}

/*****************************************************************************/
/**
 * This function is used to clear the Results register
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param Addr is the results register address
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_ClearResults(XMt_CfgData *XMtPtr, u32 Addr)
{
	u32 Index;

	for (Index = 0U; Index < XMtPtr->DdrConfigLanes; Index++) {
		Xil_Out32(Addr + (Index * 4), 0U);
	}
}

/*****************************************************************************/
/**
 * This function is used to clear the Eye
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param Addr is the results register address
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_ClearEye(XMt_CfgData *XMtPtr, u32 *Addr)
{
	memset(Addr, 0U, XMtPtr->DdrConfigLanes * sizeof(u32));
}

/*****************************************************************************/
/**
 * This function is used to get the VRef value
 *
 * @param Addr is the address of GCR5 register
 *
 * @return VRef Value value
 *
 * @note none
 *****************************************************************************/
static INLINE u32 XMt_GetVref(u32 Addr)
{
	return XMt_GetRegValue(Addr, XMT_DDR_PHY_DX0GCR5_DXREFISELR0_MASK,
			XMT_DDR_PHY_DX0GCR5_DXREFISELR0_SHIFT);
}

/*****************************************************************************/
/**
 * This function is used to get the Auto Trained VRef value
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return XST_SUCCESS on success, XST_FAILURE on failure
 *
 * @note none
 *****************************************************************************/
u32 XMt_GetVRefAuto(XMt_CfgData *XMtPtr)
{
	s32 Index;

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		XMtPtr->VRefAuto[Index] = XMt_GetVref(XMT_LANE0GCR5_OFFSET +
				(XMT_LANE_OFFSET * Index));
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to get the Auto Trained VRef minimum value
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return VRef minimum value
 *
 * @note none
 *****************************************************************************/
u32 XMt_GetVRefAutoMin(XMt_CfgData *XMtPtr)
{
	s32 Index;
	u32 VRefMin;

	VRefMin = XMtPtr->VRefAuto[0];

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		VRefMin = VRefMin < XMtPtr->VRefAuto[Index] ? VRefMin :
				XMtPtr->VRefAuto[Index];
	}

	VRefMin = (VRefMin > 25) ? (VRefMin - 25) : 0;

	return VRefMin;
}

/*****************************************************************************/
/**
 * This function is used to get the Auto Trained VRef maximum value
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return VRef maximum value
 *
 * @note none
 *****************************************************************************/
u32 XMt_GetVRefAutoMax(XMt_CfgData *XMtPtr)
{
	s32 Index;
	u32 VRefMax;

	VRefMax = XMtPtr->VRefAuto[0];

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		VRefMax = VRefMax > XMtPtr->VRefAuto[Index] ? VRefMax :
				XMtPtr->VRefAuto[Index];
	}

	return VRefMax + 25;
}

/*****************************************************************************/
/**
 * This function is used to set the VRef value
 *
 * @param Addr is the address of GCR5 register
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_SetVrefVal(XMt_CfgData *XMtPtr, u32 VRef)
{
	s32 Index;

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		XMT_MASK_WRITE(XMT_LANE0GCR5_OFFSET + (XMT_LANE_OFFSET * Index),
				XMT_DDR_PHY_DX0GCR5_DXREFISELR0_MASK, VRef);
	}
}

/*****************************************************************************/
/**
 * This function is used to set the VRef value
 *
 * @param Addr is the address of GCR5 register
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_ResetVrefAuto(XMt_CfgData *XMtPtr)
{
	s32 Index;

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		XMT_MASK_WRITE(XMT_LANE0GCR5_OFFSET + (XMT_LANE_OFFSET * Index),
				XMT_DDR_PHY_DX0GCR5_DXREFISELR0_MASK,
				XMtPtr->VRefAuto[Index]);
	}
}

/*****************************************************************************/
/**
 * This function is used to print the 2D Eye Test Results
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param VRef is the Value selected to be tested
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_Print2DEyeResults(XMt_CfgData *XMtPtr, u32 VRef)
{
	s32 Index;

	xil_printf(" %3d |", VRef);
	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		xil_printf(" %3d |", 0 - XMtPtr->EyeStart[Index]);
	}
	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		xil_printf(" %3d |", XMtPtr->EyeEnd[Index]);
	}
	xil_printf("\r\n");
}

/*****************************************************************************/
/**
 * This function is used to print the help menu
 *
 * @param none
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_PrintHelp(void)
{
	xil_printf("\r\n****************************************************************************\r\n");
	xil_printf("   Zynq MPSoC\r\n");
	xil_printf("   DRAM Diagnostics Test (A53)\r\n");
	xil_printf("****************************************************************************\r\n");
	xil_printf("   Select one of the options below:\r\n");
	xil_printf("   +--------------------------------------------------------------------+\r\n");
	xil_printf("   |  Memory Tests                                                      |\r\n");
	xil_printf("   +-----+--------------------------------------------------------------+\r\n");
	xil_printf("   | '0' | Test first 16MB region of DDR                                |\r\n");
	xil_printf("   | '1' | Test first 32MB region of DDR                                |\r\n");
	xil_printf("   | '2' | Test first 64MB region of DDR                                |\r\n");
	xil_printf("   | '3' | Test first 128MB region of DDR                               |\r\n");
	xil_printf("   | '4' | Test first 256MB region of DDR                               |\r\n");
	xil_printf("   | '5' | Test first 512MB region of DDR                               |\r\n");
	xil_printf("   | '6' | Test first 1GB region of DDR                                 |\r\n");
	xil_printf("   | '7' | Test first 2GB region of DDR                                 |\r\n");
	xil_printf("   | '8' | Test first 4GB region of DDR                                 |\r\n");
	xil_printf("   | '9' | Test first 8GB region of DDR                                 |\r\n");
	xil_printf("   | 'm' | Test user specified size in MB of DDR                        |\r\n");
	xil_printf("   | 'g' | Test user specified size in GB of DDR                        |\r\n");
	xil_printf("   +-----+--------------------------------------------------------------+\r\n");
	xil_printf("   |  Eye Tests                                                         |\r\n");
	xil_printf("   +-----+--------------------------------------------------------------+\r\n");
	xil_printf("   | 'r' | Perform a read eye analysis test                             |\r\n");
	xil_printf("   | 'w' | Perform a write eye analysis test                            |\r\n");
	xil_printf("   | 'c' | Perform a 2-D read eye analysis test                         |\r\n");
	xil_printf("   | 'e' | Perform a 2-D write eye analysis test                        |\r\n");
	xil_printf("   | 'a' | Print test start address                                     |\r\n");
	xil_printf("   | 'l' | Select Number of Iterations for Read/Write Eye Test          |\r\n");
	xil_printf("   | 't' | Specify test start address (default=0x0)                     |\r\n");
	xil_printf("   | 's' | Select the DRAM Rank (default=1)                             |\r\n");
	xil_printf("   +-----+--------------------------------------------------------------+\r\n");
	xil_printf("   |  Miscellaneous options                                             |\r\n");
	xil_printf("   +-----+--------------------------------------------------------------+\r\n");
	xil_printf("   | 'i' | Print DDR information                                        |\r\n");
	xil_printf("   | 'v' | Verbose Mode ON/OFF                                          |\r\n");
	xil_printf("   | 'o' | Toggle cache enable/disable                                  |\r\n");
	xil_printf("   | 'b' | Toggle between 32/64-bit bus widths                          |\r\n");
	xil_printf("   | 'q' | Exit the DRAM Test                                           |\r\n");
	xil_printf("   | 'h' | Print this help menu                                         |\r\n");
	xil_printf("   +-----+--------------------------------------------------------------+\r\n");
}

/*****************************************************************************/
/**
 * This function is used to print the Memory Test Header
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_PrintMemTestHeader(XMt_CfgData *XMtPtr)
{
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_4_LANE) {
		xil_printf("---------+--------+-------------------------+-----------\r\n");
		xil_printf("  TEST   | ERROR  |PER-BYTE-LANE ERROR COUNT|  TIME\r\n");
		xil_printf("         | COUNT  |  #0 ,  #1 ,  #2 ,  #3   |  (sec)\r\n");
		xil_printf("---------+--------+-------------------------+-----------\r\n");
	}

	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_8_LANE) {
		xil_printf("---------+--------+------------------------------------------------+-----------\r\n");
		xil_printf("  TEST   | ERROR  |          PER-BYTE-LANE ERROR COUNT             |  TIME\r\n");
		xil_printf("         | COUNT  |  #0 ,  #1 ,  #2 ,  #3 ,  #4 ,  #5 ,  #6 ,  #7  |  (sec)\r\n");
		xil_printf("---------+--------+------------------------------------------------+-----------\r\n");
	}
}

/*****************************************************************************/
/**
 * This function is used to print the Eye Header
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_PrintEyeHeader(XMt_CfgData *XMtPtr)
{
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_4_LANE) {
		xil_printf("-------+--------+--------+--------+--------+\r\n");
		xil_printf("Offset | LANE-0 | LANE-1 | LANE-2 | LANE-3 |\r\n");
		xil_printf("-------+--------+--------+--------+--------+\r\n");
	}

	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_8_LANE) {
		xil_printf("-------+--------+--------+--------+--------+--------+--------+--------+--------+\r\n");
		xil_printf("Offset | LANE-0 | LANE-1 | LANE-2 | LANE-3 | LANE-4 | LANE-5 | LANE-6 | LANE-7 |\r\n");
		xil_printf("-------+--------+--------+--------+--------+--------+--------+--------+--------+\r\n");
	}
}

/*****************************************************************************/
/**
 * This function is used to print the Eye Results Header
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_PrintEyeResultsHeader(XMt_CfgData *XMtPtr)
{
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_4_LANE) {
		xil_printf("---------+---------+---------+---------+\r\n");
		xil_printf("  LANE-0 |  LANE-1 |  LANE-2 |  LANE-3 |\r\n");
		xil_printf("---------+---------+---------+---------+\r\n");
	}

	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_8_LANE) {
		xil_printf("---------+---------+---------+---------+---------+---------+---------+---------+\r\n");
		xil_printf("  LANE-0 |  LANE-1 |  LANE-2 |  LANE-3 |  LANE-4 |  LANE-5 |  LANE-6 |  LANE-7 |\r\n");
		xil_printf("---------+---------+---------+---------+---------+---------+---------+---------+\r\n");
	}
}

/*****************************************************************************/
/**
 * This function is used to print the 2D Eye Results Header
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_Print2DEyeResultsHeader(XMt_CfgData *XMtPtr)
{
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_4_LANE) {
		xil_printf("-----+-----+-----+-----+-----+-----+-----+-----+-----+\r\n");
		xil_printf("VREF | LL0 | LL1 | LL2 | LL3 | RL0 | RL1 | RL2 | RL3 |\r\n");
		xil_printf("-----+-----+-----+-----+-----+-----+-----+-----+-----+\r\n");
	}

	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_8_LANE) {
		xil_printf("-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\r\n");
		xil_printf("VREF | LL0 | LL1 | LL2 | LL3 | LL4 | LL5 | LL6 | LL7 | RL0 | RL1 | RL2 | RL3 | RL4 | RL5 | RL6 | RL7 |\r\n");
		xil_printf("-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\r\n");
	}
}
/*****************************************************************************/
/**
 * This function is used to print the ASCII line
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param LineCode is the number indicating the line pattern to be printed
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_PrintLine(XMt_CfgData *XMtPtr, u8 LineCode)
{
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_4_LANE) {
		if (LineCode == 1U) {
			xil_printf("----------------------------------------");
		} else if (LineCode == 2U) {
			xil_printf("-------+--------+--------+--------+--------+\r\n");
		} else if (LineCode == 3U) {
			xil_printf("---------+---------+---------+---------+\r\n");
		} else if (LineCode == 4U) {
			xil_printf("---------+--------+-------------------------+-----------\r\n");
		} else if (LineCode == 5U) {
			xil_printf("-----+-----+-----+-----+-----+-----+-----+-----+-----+\r\n");
		} else {
			xil_printf("\r\n");
		}
	}
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_8_LANE) {
		if (LineCode == 1U) {
			xil_printf("--------------------------------------------------------------------------------");
		} else if (LineCode == 2U) {
			xil_printf("-------+--------+--------+--------+--------+--------+--------+--------+--------+\r\n");
		} else if (LineCode == 3U) {
			xil_printf("---------+---------+---------+---------+---------+---------+---------+----------\r\n");
		} else if (LineCode == 4U) {
			xil_printf("---------+--------+------------------------------------------------+-----------\r\n");
		} else if (LineCode == 5U) {
			xil_printf("-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+\r\n");
		} else {
			xil_printf("\r\n");
		}
	}
}

/*****************************************************************************/
/**
 * This function is used to print the Results of the Eye Test
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_PrintResults(XMt_CfgData *XMtPtr)
{
	s32 Index;

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		xil_printf(" %6d |", Xil_In32(XMT_RESULTS_BASE + (Index * 4)));
	}
	xil_printf("\r\n");
}
