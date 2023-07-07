/*******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
 *       mn   07/01/19 Add support to specify number of iteration for memtest
 *       mn   09/09/19 Correct the DDR type name for LPDDR4
 *       mn   07/29/20 Modify code to use DRAM VRef for 2D Write Eye Test
 * 1.2   mn   02/11/21 Added support for 16-Bit Bus Width
 *       mn   03/10/21 Fixed doxygen warnings
 *       mn   04/30/21 Fixed rank selection logic for multi rank DDR
 *       mn   05/24/21 Fixed Eye Test issue with higher rank
 * 1.3   mn   09/08/21 Removed illegal write to DXnGTR0.WDQSL register field
 * 1.4   mn   11/29/21 Usability Enhancements for 2D Read/Write Eye
 * 		 sg   11/11/22 Added 2D Read Eye support for LPDDR4
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

	/* Get the DDR Type */
	XMtPtr->DdrType = XMt_GetRegValue(XMT_DDRC_MSTR,
			   XMT_DDRC_MSTR_DDR_TYPE_MASK,
			   XMT_DDRC_MSTR_DDR_TYPE_SHIFT);

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
	} else if (BusWidth == 0x2U) {
		XMtPtr->BusWidth = XMT_DDR_CONFIG_16BIT_WIDTH;
		XMtPtr->DdrConfigLanes = XMT_DDR_CONFIG_2_LANE;
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

	XMtPtr->ReadCenterFetched = 0U;
	XMtPtr->WriteCenterFetched = 0U;


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
	} else if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_2_LANE) {
		xil_printf("16-bit ");
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
		xil_printf("LPDDR4 ");
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
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_EnableRefresh(void)
{
	XMt_SetRegValue(XMT_DDR_PHY_DTCR0, XMT_DDR_PHY_DTCR0_RFSHDT_MASK,
			XMT_DDR_PHY_DTCR0_RFSHDT_SHIFT, 0x8);
}

/*****************************************************************************/
/**
 * This function is used to disable Refresh During Training
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_DisableRefresh(void)
{
	XMt_SetRegValue(XMT_DDR_PHY_DTCR0, XMT_DDR_PHY_DTCR0_RFSHDT_MASK,
			XMT_DDR_PHY_DTCR0_RFSHDT_SHIFT, 0x0);
}

/*****************************************************************************/
/**
 * This function is used to enable DFI
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_DfiEnable(void)
{
	XMt_SetRegValue(XMT_DDRC_SWCTL, XMT_DDRC_SWCTL_SW_DONE_MASK,
			XMT_DDRC_SWCTL_SW_DONE_SHIFT, 0x0);

	XMt_SetRegValue(XMT_DDRC_DFIUPD0, XMT_DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_MASK,
			XMT_DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_SHIFT, 0x0);

	XMt_SetRegValue(XMT_DDRC_DERATEEN, XMT_DDRC_DERATEEN_DERATE_ENABLE_MASK,
			XMT_DDRC_DERATEEN_DERATE_ENABLE_SHIFT, 0x0);

	XMt_SetRegValue(XMT_DDRC_SWCTL, XMT_DDRC_SWCTL_SW_DONE_MASK,
			XMT_DDRC_SWCTL_SW_DONE_SHIFT, 0x1);

	XMt_SetRegValue(XMT_DDR_PHY_DSGCR, XMT_DDR_PHY_DSGCR_PUREN_MASK,
			XMT_DDR_PHY_DSGCR_PUREN_SHIFT, 0x1);

	XMt_SetRegValue(XMT_DDR_PHY_DQSDR0, XMT_DDR_PHY_DQSDR0_DFTDTEN_MASK,
			XMT_DDR_PHY_DQSDR0_DFTDTEN_SHIFT, 0x0);

	XMt_SetRegValue(XMT_DDR_PHY_DTCR0, XMT_DDR_PHY_DTCR0_INCWEYE_MASK,
			XMT_DDR_PHY_DTCR0_INCWEYE_SHIFT, 0x0);
}

/*****************************************************************************/
/**
 * This function is used to disable DFI
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_DfiDisable(void)
{
	XMt_SetRegValue(XMT_DDRC_SWCTL, XMT_DDRC_SWCTL_SW_DONE_MASK,
			XMT_DDRC_SWCTL_SW_DONE_SHIFT, 0x0);

	XMt_SetRegValue(XMT_DDRC_DFIUPD0, XMT_DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_MASK,
			XMT_DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_SHIFT, 0x1);

	XMt_SetRegValue(XMT_DDRC_DERATEEN, XMT_DDRC_DERATEEN_DERATE_ENABLE_MASK,
			XMT_DDRC_DERATEEN_DERATE_ENABLE_SHIFT, 0x0);

	XMt_SetRegValue(XMT_DDRC_SWCTL, XMT_DDRC_SWCTL_SW_DONE_MASK,
			XMT_DDRC_SWCTL_SW_DONE_SHIFT, 0x1);

	XMt_SetRegValue(XMT_DDR_PHY_DSGCR, XMT_DDR_PHY_DSGCR_PUREN_MASK,
			XMT_DDR_PHY_DSGCR_PUREN_SHIFT, 0x0);

	XMt_SetRegValue(XMT_DDR_PHY_DQSDR0, XMT_DDR_PHY_DQSDR0_DFTDTEN_MASK,
			XMT_DDR_PHY_DQSDR0_DFTDTEN_SHIFT, 0x0);

	XMt_SetRegValue(XMT_DDR_PHY_DTCR0, XMT_DDR_PHY_DTCR0_INCWEYE_MASK,
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
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param Addr is the address of GCR5 register
 *
 * @return VRef Value value
 *
 * @note none
 *****************************************************************************/
static INLINE u32 XMt_GetVref(XMt_CfgData *XMtPtr, u32 Addr)
{
	if (XMtPtr->RankSel == 1U) {
		return XMt_GetRegValue(Addr, XMT_DDR_PHY_DX0GCR5_DXREFISELR1_MASK,
				XMT_DDR_PHY_DX0GCR5_DXREFISELR1_SHIFT);
	} else {
		return XMt_GetRegValue(Addr, XMT_DDR_PHY_DX0GCR5_DXREFISELR0_MASK,
				XMT_DDR_PHY_DX0GCR5_DXREFISELR0_SHIFT);
	}
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
		XMtPtr->VRefAuto[Index] = XMt_GetVref(XMtPtr, XMT_LANE0GCR5_OFFSET +
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
		if (XMtPtr->RankSel == 1U) {
			XMt_MaskWrite(XMT_LANE0GCR5_OFFSET + (XMT_LANE_OFFSET * Index),
					XMT_DDR_PHY_DX0GCR5_DXREFISELR1_MASK, VRef << 8U);
		} else {
			XMt_MaskWrite(XMT_LANE0GCR5_OFFSET + (XMT_LANE_OFFSET * Index),
					XMT_DDR_PHY_DX0GCR5_DXREFISELR0_MASK, VRef);
		}
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
		if (XMtPtr->RankSel == 1U) {
			XMt_MaskWrite(XMT_LANE0GCR5_OFFSET + (XMT_LANE_OFFSET * Index),
					XMT_DDR_PHY_DX0GCR5_DXREFISELR1_MASK,
					XMtPtr->VRefAuto[Index] << 8U);
		} else {
			XMt_MaskWrite(XMT_LANE0GCR5_OFFSET + (XMT_LANE_OFFSET * Index),
					XMT_DDR_PHY_DX0GCR5_DXREFISELR0_MASK,
					XMtPtr->VRefAuto[Index]);
		}
	}
}

/*****************************************************************************/
/**
 * This function is used to read the VRef value via MPR mode
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return VRef Value
 *
 * @note none
 *****************************************************************************/
static u32 XMt_ReadMprVRef(XMt_CfgData *XMtPtr)
{
	u32 Data[24] = {0};
	u64 Data64[8] = {0};
	u8 Dataarrange[8] = {0};
	u32 Emr3;
	u32 RetVal = 0U;
	u32 Index;
	u32 Index1;

	/* Get the MR3 value for DDR */
	Emr3 = Xil_In32(XMT_DDRC_INIT4) & XMT_DDRC_INIT4_EMR3_MASK;
	Emr3 &= XMT_DDR_MR3_CONFIG_MASK;

	/*
	 * Set MPR page [1:0] = 0x2U (Page 2),
	 * MPR mode [2] = 0x1U (Enable),
	 * Read Format [12:11] = 0x0U (Serial)
	 */
	Emr3 |= XMT_DDR_MR3_MPR_P2_CONFIG;

	/* Select MR3 on MRCTRL0 register and select rank 0 of DDR */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0,
				  (XMT_DDRC_MRCTRL0_MR_ADDR_MASK
				 | XMT_DDRC_MRCTRL0_MR_RANK_MASK),
				  (XMT_DDR_MR_ADDR_MR3
				 | XMT_DDR_MR_RANK_0));

	/* Write the MR Data to MRCTRL1 register */
	XMt_MaskWrite(XMT_DDRC_MRCTRL1, XMT_DDRC_MRCTRL1_MR_DATA_MASK, Emr3);

	/* Trigger the transaction for MR write */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_WR_MASK, XMT_DDR_MR_WR);

	/* Wait for MR operation to complete */
	while (Xil_In32(XMT_DDRC_MRSTAT) & XMT_DDR_MR_WR_BUSY);

	/* Clear the MR Data in MRCTRL1 register */
	XMt_MaskWrite(XMT_DDRC_MRCTRL1, XMT_DDRC_MRCTRL1_MR_DATA_MASK, 0x0U);

	/*
	 * Select the MPR location '1' in MRCTRL0 register
	 * Select Rank '0' of DDR
	 * Select the MPR mode 1 (enable)
	 * Select the MR Type as 1 (Read)
	 */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0,
				  (XMT_DDRC_MRCTRL0_MR_ADDR_MASK
				 | XMT_DDRC_MRCTRL0_MR_RANK_MASK
				 | XMT_DDRC_MRCTRL0_MPR_EN_MASK
				 | XMT_DDRC_MRCTRL0_MR_TYPE_MASK),
				  (XMT_DDR_MR_ADDR_MR1
				 | XMT_DDR_MR_RANK_0
				 | XMT_DDR_MPR_ENABLE
				 | XMT_DDR_MR_READ));

	/* Trigger the transaction for MR read */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_WR_MASK, XMT_DDR_MR_WR);

	/* Wait for MR operation to complete */
	while (Xil_In32(XMT_DDRC_MRSTAT) & XMT_DDR_MR_WR_BUSY);

	/* Wait till the valid data is available in the FIFO */
	while (!(Xil_In32(XMT_DDRC_MRR_STATUS) & XMT_DDRC_MRR_STATUS_VALID));

	/* Read the FIFO and determine the VRef values */
	for (Index = 0U; Index < 12U; Index++) {
		Data[Index] = Xil_In32(XMT_DDRC_MRR_DATA0 + (Index * 4U));
	}

	for (Index = 0U; Index < 12U; Index++) {
		Data[Index + 12U] = Xil_In32(XMT_DDRC_MRR_DATA0 + (Index * 4U));
	}

	for (Index = 0U; Index < XMT_DDRC_MRR_DATA_WIDTH; Index++) {
		Data64[Index] = ((u64)Data[(Index * 3U) + 1U] << 32U) | Data[(Index * 3U)];
	}

	for (Index = 0U; Index < XMtPtr->DdrConfigLanes; Index++) {
		for (Index1 = 0U; Index1 < XMT_DDRC_MRR_DATA_WIDTH; Index1++) {
			Dataarrange[Index] |= ((!!((u8)(Data64[Index1] >>
								  (Index * XMT_DDRC_MRR_DATA_WIDTH)))) <<
								  ((XMT_DDRC_MRR_DATA_WIDTH - 1U) - Index1));
		}
		Dataarrange[Index] >>= 1U;
		RetVal += Dataarrange[Index];
	}
	RetVal /= XMtPtr->DdrConfigLanes;

	/* Get the MR3 value for DDR */
	Emr3 = Xil_In32(XMT_DDRC_INIT4) & XMT_DDRC_INIT4_EMR3_MASK;

	/* Clear the values related to MPR mode */
	Emr3 &= XMT_DDR_MR3_CONFIG_MASK;

	/*
	 * Select MR3 on MRCTRL0 register
	 * Select rank 0 of DDR
	 * Disable MPR mode
	 * Disable Read mode
	 */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0,
				  (XMT_DDRC_MRCTRL0_MR_ADDR_MASK
				 | XMT_DDRC_MRCTRL0_MR_RANK_MASK
				 | XMT_DDRC_MRCTRL0_MPR_EN_MASK
				 | XMT_DDRC_MRCTRL0_MR_TYPE_MASK), 0x3010U);

	/* Write the MR Data to MRCTRL1 register */
	XMt_MaskWrite(XMT_DDRC_MRCTRL1, XMT_DDRC_MRCTRL1_MR_DATA_MASK, Emr3);

	/* Trigger the transaction for MR write */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_WR_MASK, XMT_DDR_MR_WR);

	/* Wait for MR operation to complete */
	while (Xil_In32(XMT_DDRC_MRSTAT) & XMT_DDR_MR_WR_BUSY);

	return RetVal;
}

/*****************************************************************************/
/**
 * This function is used to read the VRef value via MR mode
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return VRef Value
 *
 * @note none
 *****************************************************************************/
static u32 XMt_ReadMrsVRef(XMt_CfgData *XMtPtr)
{
	u32 RetVal;

	/* Wait for MR operations to complete */
	while(Xil_In32(XMT_DDRC_MRSTAT) & XMT_DDR_MR_WR_BUSY);

	/* Select MR14 for the Read */
	Xil_Out32(XMT_DDRC_MRCTRL1, XMT_DDR_MR_ADDR_MR14);

	/* Select Rank 0 */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_RANK_MASK, XMT_DDR_MR_RANK_0);

	/* 0 for Write, 1 for Read */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_TYPE_MASK, XMT_DDR_MR_READ);

	/* Trigger a mode register read or write operation */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_WR_MASK, XMT_DDR_MR_WR);

	/* Wait for MR operations to complete */
	while(Xil_In32(XMT_DDRC_MRSTAT) & XMT_DDR_MR_WR_BUSY);

	RetVal =  Xil_In32(XMT_DDRC_MRR_DATA0) & XMT_DDRC_MRR_DATA_U8_MASK;

	/* Dummy read to clear FIFO */
	(void) Xil_In32(XMT_DDRC_MRR_DATA11);

	return RetVal;
}

/*****************************************************************************/
/**
 * This function is used to read the VRef value
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return VRef Value
 *
 * @note none
 *****************************************************************************/
u32 XMt_GetWrVRef(XMt_CfgData *XMtPtr)
{
	u32 RetVal;

	if (XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) {
		RetVal = XMt_ReadMprVRef(XMtPtr);
	} else {
		RetVal = XMt_ReadMrsVRef(XMtPtr);
	}

	return RetVal;
}

/*****************************************************************************/
/**
 * This function is used to set the VRef via MR mode
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @VRef  VRef value to be set
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_SetWrVref(XMt_CfgData *XMtPtr, u32 VRef)
{
	/* Select Rank 0 */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_RANK_MASK, XMT_DDR_MR_RANK_0);

	/* 0 for Write, 1 for Read */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_TYPE_MASK, XMT_DDR_MR_WRITE);

	/* 0 for mode register set (MRS), 1 for WR/RD for MPR */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MPR_EN_MASK, XMT_DDR_MRS_ENABLE);

	if (XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) {
		/* DDR4 MR6 Address */
		XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_ADDR_MASK, XMT_DDR_MR_ADDR_MR6);
	}

	/* Write the VRef Value */
	XMt_MaskWrite(XMT_DDRC_MRCTRL1, XMT_DDRC_MRCTRL1_MR_DATA_MASK, VRef);

	/* Trigger a mode register read or write operation */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_WR_MASK, XMT_DDR_MR_WR);

	/* Wait for MR operations to complete */
	while(Xil_In32(XMT_DDRC_MRSTAT) & XMT_DDR_MR_WR_BUSY);
}

/*****************************************************************************/
/**
 * This function is used to reset the original VRef value
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_ResetWrVref(XMt_CfgData *XMtPtr)
{
	/* Enter the Range 1 calibration mode */
	XMt_SetWrVref(XMtPtr, XMT_DDR_VREF_CALIB_MODE_EN);

	/* Select Rank 0 */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_RANK_MASK, XMT_DDR_MR_RANK_0);

	/* 0 for Write, 1 for Read */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_TYPE_MASK, XMT_DDR_MR_WRITE);

	/* 0 for mode register set (MRS), 1 for WR/RD for MPR */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MPR_EN_MASK, XMT_DDR_MRS_ENABLE);

	if (XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) {
		/* DDR4 MR6 Address */
		XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_ADDR_MASK, XMT_DDR_MR_ADDR_MR6);
		/* Write the VRef Value */
		XMt_MaskWrite(XMT_DDRC_MRCTRL1, 0x3FFFF, (XMtPtr->VRefAutoWr | XMT_DDR_VREF_CALIB_MODE_EN));
	} else {
		/* Write the VRef Value */
		XMt_MaskWrite(XMT_DDRC_MRCTRL1, 0x3FFFF, (XMtPtr->VRefAutoWr | XMT_DDR_MR_ADDR_MR14));
	}

	/* Trigger a mode register read or write operation */
	XMt_MaskWrite(XMT_DDRC_MRCTRL0, XMT_DDRC_MRCTRL0_MR_WR_MASK, XMT_DDR_MR_WR);

	/* Wait for MR operations to complete */
	while(Xil_In32(XMT_DDRC_MRSTAT) & XMT_DDR_MR_WR_BUSY);

	/* Exit the Calibration Mode */
	XMt_SetWrVref(XMtPtr, XMT_DDR_VREF_CALIB_MODE_DIS);
}

/*****************************************************************************/
/**
 * This function is used to print the 2D Read Eye Test Results
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param VRef is the Value selected to be tested
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_Print2DReadEyeResults(XMt_CfgData *XMtPtr, u32 VRef)
{
	s32 Index;
	s32 Index1;
	float VrefPercent;
	u32 VrefPercentInteger;
	float VrefPercentDecimalF;
	u32 VrefPercentDecimal;

	if (XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) {
		VrefPercent = 2.0
				* ((((float) VRef) * XMT_RD_VREF_INTERVAL) + XMT_RD_MIN_VREF);
	} else {
		VrefPercent = (((float) VRef) * XMT_RD_VREF_INTERVAL) + XMT_RD_MIN_VREF;
	}

	VrefPercentInteger = (int) VrefPercent;
	VrefPercentDecimalF = (VrefPercent - VrefPercentInteger) * 10;
	VrefPercentDecimal = (int) VrefPercentDecimalF;
	VrefPercentDecimalF = (VrefPercentDecimalF - VrefPercentDecimal) * 10;
	VrefPercentDecimal = (VrefPercentDecimalF >= 5.0) ? VrefPercentDecimal + 1 : VrefPercentDecimal;
	VrefPercentInteger = (VrefPercentDecimal > 9) ? VrefPercentInteger + 1 : VrefPercentInteger;
	VrefPercentDecimal = (VrefPercentDecimal > 9) ? 0 : VrefPercentDecimal;

	xil_printf("%2d", VrefPercentInteger );
	xil_printf(".%1d |", VrefPercentDecimal );

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		for (Index1 = -10; Index1 < 0; Index1++) {
			if (Index1 < XMtPtr->EyeStart[Index]/((XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) ? 4 : 9)) {
				xil_printf(" ");
			} else {
				xil_printf("*");
			}
		}
		xil_printf(":");
		for (Index1 = 0; Index1 < 10; Index1++) {
			if (Index1 < XMtPtr->EyeEnd[Index]/((XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) ? 4 : 9)) {
				xil_printf("*");
			} else {
				xil_printf(" ");
			}
		}
		xil_printf("|");
	}
	xil_printf("\r\n");
}

/*****************************************************************************/
/**
 * This function is used to print the 2D Write Eye Test Results
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param VRef is the Value selected to be tested
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_Print2DWriteEyeResultsR1(XMt_CfgData *XMtPtr, u32 VRef)
{
	s32 Index;
	s32 Index1;
	float VrefPercent;
	u32 VrefPercentInteger;
	float VrefPercentDecimalF;
	u32 VrefPercentDecimal;
	float WrVrefInterval;
	float WrMinVref;

	if (XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) {
		WrVrefInterval = XMT_WR_VREF_INTERVAL_DDR4;
		WrMinVref = XMT_WR_MIN_VREF_DDR4_R1;
	} else {
		WrVrefInterval = XMT_WR_VREF_INTERVAL_LPDDR4;
		WrMinVref = XMT_WR_MIN_VREF_LPDDR4_R1;
	}

	VrefPercent = ( ((float) VRef) * WrVrefInterval ) + WrMinVref;
	VrefPercentInteger = (int) VrefPercent;
	VrefPercentDecimalF = (VrefPercent - VrefPercentInteger) * 10;
	VrefPercentDecimal = (int) VrefPercentDecimalF;
	VrefPercentDecimalF = (VrefPercentDecimalF - VrefPercentDecimal) * 10;
	VrefPercentDecimal = (VrefPercentDecimalF >= 5.0) ? VrefPercentDecimal + 1 : VrefPercentDecimal;
	VrefPercentInteger = (VrefPercentDecimal > 9) ? VrefPercentInteger + 1 : VrefPercentInteger;
	VrefPercentDecimal = (VrefPercentDecimal > 9) ? 0 : VrefPercentDecimal;

	xil_printf("%2d", VrefPercentInteger );
	xil_printf(".%1d |", VrefPercentDecimal );

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		for (Index1 = -10; Index1 < 0; Index1++) {
			if (Index1 < XMtPtr->EyeStart[Index]/((XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) ? 4 : 9)) {
				xil_printf(" ");
			} else {
				xil_printf("*");
			}
		}
		xil_printf(":");
		for (Index1 = 0; Index1 < 10; Index1++) {
			if (Index1 < XMtPtr->EyeEnd[Index]/((XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) ? 4 : 9)) {
				xil_printf("*");
			} else {
				xil_printf(" ");
			}
		}
		xil_printf("|");
	}
	xil_printf("\r\n");
}

/*****************************************************************************/
/**
 * This function is used to print the 2D Write Eye Test Results
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param VRef is the Value selected to be tested
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_Print2DWriteEyeResultsR2(XMt_CfgData *XMtPtr, u32 VRef)
{
	s32 Index;
	s32 Index1;
	float VrefPercent;
	u32 VrefPercentInteger;
	float VrefPercentDecimalF;
	u32 VrefPercentDecimal;
	float WrVrefInterval;
	float WrMinVref;

	if (XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) {
		WrVrefInterval = XMT_WR_VREF_INTERVAL_DDR4;
		WrMinVref = XMT_WR_MIN_VREF_DDR4_R2;
	} else {
		WrVrefInterval = XMT_WR_VREF_INTERVAL_LPDDR4;
		WrMinVref = XMT_WR_MIN_VREF_LPDDR4_R2;
	}

	VrefPercent = ( ((float) VRef) * WrVrefInterval ) + WrMinVref;
	VrefPercentInteger = (int) VrefPercent;
	VrefPercentDecimalF = (VrefPercent - VrefPercentInteger) * 10;
	VrefPercentDecimal = (int) VrefPercentDecimalF;
	VrefPercentDecimalF = (VrefPercentDecimalF - VrefPercentDecimal) * 10;
	VrefPercentDecimal = (VrefPercentDecimalF >= 5.0) ? VrefPercentDecimal + 1 : VrefPercentDecimal;
	VrefPercentInteger = (VrefPercentDecimal > 9) ? VrefPercentInteger + 1 : VrefPercentInteger;
	VrefPercentDecimal = (VrefPercentDecimal > 9) ? 0 : VrefPercentDecimal;

	xil_printf("%2d", VrefPercentInteger );
	xil_printf(".%1d |", VrefPercentDecimal );

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		for (Index1 = -10; Index1 < 0; Index1++) {
			if (Index1 < XMtPtr->EyeStart[Index]/((XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) ? 4 : 9)) {
				xil_printf(" ");
			} else {
				xil_printf("*");
			}
		}
		xil_printf(":");
		for (Index1 = 0; Index1 < 10; Index1++) {
			if (Index1 < XMtPtr->EyeEnd[Index]/((XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) ? 4 : 9)) {
				xil_printf("*");
			} else {
				xil_printf(" ");
			}
		}
		xil_printf("|");
	}
	xil_printf("\r\n");
}

/*****************************************************************************/
/**
 * This function is used to print the help menu
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
	xil_printf("   | 'l' | Select Number of Iterations for Memory/Read/Write-Eye/2D test|\r\n");
	xil_printf("   | 't' | Specify test start address (default=0x0)                     |\r\n");
	xil_printf("   | 's' | Select the DRAM Rank (default=0)                             |\r\n");
	xil_printf("   +-----+--------------------------------------------------------------+\r\n");
	xil_printf("   |  Miscellaneous options                                             |\r\n");
	xil_printf("   +-----+--------------------------------------------------------------+\r\n");
	xil_printf("   | 'i' | Print DDR information                                        |\r\n");
	xil_printf("   | 'v' | Verbose Mode ON/OFF                                          |\r\n");
	xil_printf("   | 'o' | Toggle cache enable/disable                                  |\r\n");
	xil_printf("   | 'b' | Toggle between 16/32/64-bit bus widths                       |\r\n");
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
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_2_LANE) {
		xil_printf("---------+--------+-------------+-----------\r\n");
		xil_printf("  TEST   | ERROR  | LANE ERRS   |  TIME\r\n");
		xil_printf("         | COUNT  |  #0 ,  #1   |  (sec)\r\n");
		xil_printf("---------+--------+-------------+-----------\r\n");
	}

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
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_2_LANE) {
		xil_printf("-------+--------+--------+\r\n");
		xil_printf("Offset | LANE-0 | LANE-1 |\r\n");
		xil_printf("-------+--------+--------+\r\n");
	}

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
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_2_LANE) {
		xil_printf("---------+---------+\r\n");
		xil_printf("  LANE-0 |  LANE-1 |\r\n");
		xil_printf("---------+---------+\r\n");
	}

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
	xil_printf("Note: Each asterisk(*) represents %d Taps\n",
			(XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) ? 4 : 9);
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_2_LANE) {
		xil_printf("-----+---------------------+---------------------+\r\n");
		xil_printf("VREF%%|        LANE0        |        LANE1        |\r\n");
		xil_printf("-----+---------------------+---------------------+\r\n");
	}

	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_4_LANE) {
		xil_printf("-----+---------------------+---------------------+---------------------+---------------------+\r\n");
		xil_printf("VREF%%|        LANE0        |        LANE1        |        LANE2        |        LANE3        |\r\n");
		xil_printf("-----+---------------------+---------------------+---------------------+---------------------+\r\n");
	}

	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_8_LANE) {
		xil_printf("-----+---------------------+---------------------+---------------------+---------------------+---------------------+---------------------+---------------------+---------------------+\r\n");
		xil_printf("VREF%%|        LANE0        |        LANE1        |        LANE2        |        LANE3        |        LANE4        |        LANE5        |        LANE6        |        LANE7        |\r\n");
		xil_printf("-----+---------------------+---------------------+---------------------+---------------------+---------------------+---------------------+---------------------+---------------------+\r\n");
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
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_2_LANE) {
		if (LineCode == 1U) {
			xil_printf("--------------------");
		} else if (LineCode == 2U) {
			xil_printf("-------+--------+--------+\r\n");
		} else if (LineCode == 3U) {
			xil_printf("---------+---------+\r\n");
		} else if (LineCode == 4U) {
			xil_printf("---------+--------+-------------+-----------\r\n");
		} else if (LineCode == 5U) {
			xil_printf("-----+---------------------+---------------------+\r\n");
		} else {
			xil_printf("\r\n");
		}
	}
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
			xil_printf("-----+---------------------+---------------------+---------------------+---------------------+\r\n");
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
			xil_printf("-----+---------------------+---------------------+---------------------+---------------------+---------------------+---------------------+---------------------+---------------------+\r\n");
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
