/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xclk_wiz.c
* @brief Implements CLK_WIZ interface functions.
*
* This file implements the functions to get the CLK_WIZ GUI information and
* Clock Monitor Interrupt status
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0 ram 2/12/16 Initial version for Clock Wizard
* 1.1 siv 8/17/16 Used UINTPTR instead of u32 for Baseaddress
* 	Changed the prototype of XClk_Wiz_CfgInitialize
* 1.2 ms  3/02/17 Fixed compilation warnings. Fix for CR-970507.
* 1.3 sd  4/09/20 Added versal support.
* 1.4 sd  5/22/20 Added zynqmp set rate.
* 		  Use PrimInClkFreq for input clock rate.
*     sd  8/12/20 Added a setrate function that takes the rate in Hz.
* 1.5 sd  5/22/20 Prevent return in void function
* 1.6 sd  7/07/23 Added SDT support.
* 1.8 sd  8/14/24 Added GetRate support.
* 1.11 sd  9/27/25 Added XClk_Wiz_GetInputRate.
*      sd  10/29/25 Conditionally compile XClk_Wiz_GetInputRate.
*      sd  11/21/25 Fix a typo in strcmp.
*      sd  5/7/26  Add the CP and Res update on rate change.
*      ss  7/10/26 Added XClk_Wiz_UpdateDivisors and XClk_Wiz_Reconfig APIs.
* 1.12 ss  8/7/26  Updated clkx5 get/set input and output frequency APIs and
*					clk_in_sel register control APIs.
* 1.12 ss  8/18/26 Added PFD range validation to divisor calculations.
* 1.12 vn  09/3/26  Fix doxygen warnings.
* 1.12  tb  09/16/26 Align Doxygen comments with AMD standards.
* </pre>
******************************************************************************/

/**
* @addtogroup clk_wiz_api CLK_WIZ APIs
* @{
*/

/***************************** Include Files *********************************/

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xclk_wiz.h"
#include "xil_sutil.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*************************** Macros Definitions ******************************/

/************************** Function Prototypes ******************************/

/*
* Stub callbacks for each interrupt type. XClk_Wiz_CfgInitialize
* installs them as the default. If the application registers no
* callback, these functions are called and do nothing.
*/
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask);

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
* @brief Initializes a CLK_WIZ driver instance.
*
* Initialize the XClk_Wiz instance provided by the caller based on the
* given Config structure.
*
* @param[out] InstancePtr XClk_Wiz instance to operate on.
* @param[in] CfgPtr Device configuration structure containing information about
*   a specific CLK_WIZ.
* @param[in] EffectiveAddr Base address of the device. If address translation is
*   being used, then this parameter must reflect the virtual base address.
*   Otherwise, the physical address should be used.
*
* @return
*		- XST_SUCCESS Initialization was successful.
*		- XST_FAILURE Initialization was failure.
*
*****************************************************************************/
u32 XClk_Wiz_CfgInitialize(XClk_Wiz *InstancePtr, XClk_Wiz_Config *CfgPtr,
			   UINTPTR EffectiveAddr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid((UINTPTR *)EffectiveAddr != NULL);

	/* Setup the instance */
	InstancePtr->Config = *CfgPtr;

	InstancePtr->Config.BaseAddr = EffectiveAddr;
#ifdef SDT
	InstancePtr->Config.IntId = CfgPtr->IntId;
	InstancePtr->Config.IntrParent = CfgPtr->IntrParent;
#endif

	/* Set all handlers to stub values, let user configure this data later
	 */
	InstancePtr->ClkOutOfRangeCallBack  = StubErrCallBack;
	InstancePtr->ClkGlitchCallBack      = StubErrCallBack;
	InstancePtr->ClkStopCallBack	    = StubErrCallBack;

	InstancePtr->ErrorCallBack = StubErrCallBack;
	InstancePtr->MinErr = 500000;

	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Calculates M, D, and O values for a rate in MHz.
*
* Calculate the M, D, and O values for the given SetRate frequency.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
* @param[in] SetRate Frequency for which the M, D and O values are to be
*   calculated.
*
* @return
*		- XST_SUCCESS if the divisors were calculated.
*		- XST_FAILURE otherwise.
*
*****************************************************************************/
static u32  XClk_Wiz_CalculateDivisors (XClk_Wiz  *InstancePtr, u64 SetRate)
{
	u32 m;
	u32 d;
	u32 Div;
	u64 Fvco;
	u64 Freq;
	u64 Diff;
	u64 Minerr = 1000;
	u32 Status = XST_FAILURE;
	u64 VcoMin;
	u64 VcoMax;
	u32 InputRate;
	u32 Platform;
	u32 Mmin;
	u32 Mmax;
	u64 Pfd;
	u32 Dmin;
	u32 Dmax;
	u32 Omin;
	u32 Omax;
	u64 PfdMin;
	u64 PfdMax;

	Platform = XGetPlatform_Info();

	if (Platform == (u32)XPLAT_VERSAL) {
		VcoMin = XCLK_VCO_MIN;
		VcoMax = XCLK_VCO_MAX;
		Mmin = XCLK_M_MIN;
		Mmax = XCLK_M_MAX;
		Dmin = XCLK_D_MIN;
		Dmax = XCLK_D_MAX;
		Omin = XCLK_O_MIN;
		Omax = XCLK_O_MAX;
		PfdMin = XCLK_PFD_MIN;
		PfdMax = XCLK_PFD_MAX;
	} else {
		VcoMin = XCLK_US_VCO_MIN;
		VcoMax = XCLK_US_VCO_MAX;
		Mmin = XCLK_US_M_MIN;
		Mmax = XCLK_US_M_MAX;
		Dmin = XCLK_US_D_MIN;
		Dmax = XCLK_US_D_MAX;
		Omin = XCLK_US_O_MIN;
		Omax = XCLK_US_O_MAX;
		PfdMin = XCLK_US_PFD_MIN;
		PfdMax = XCLK_US_PFD_MAX;
	}

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SetRate != 0);

	InputRate = XClk_Wiz_GetInputRate(InstancePtr);

	for (m = Mmin; m <= Mmax; m++) {
		for (d = Dmin; d <= Dmax; d++) {
			Pfd = (u64)InputRate / d;
			if (Pfd < PfdMin * XCLK_MHZ || Pfd > PfdMax * XCLK_MHZ) {
				continue;
			}
			Fvco = (u64)InputRate * m / (d * XCLK_MHZ);
			if ( Fvco >= VcoMin && Fvco <= VcoMax ) {

				for (Div = Omin; Div <= Omax; Div++ ) {
					Freq = Fvco / Div;

					if (Freq > SetRate) {
						Diff = Freq - SetRate;
					} else {
						Diff = SetRate - Freq;
					}
					if (Diff == 0 ) {
						InstancePtr->MVal = m;
						InstancePtr->DVal = d;
						InstancePtr->OVal = Div;
						return XST_SUCCESS;
					} else if (Diff < Minerr) {
						Minerr = Diff;
						InstancePtr->MVal = m;
						InstancePtr->DVal = d;
						InstancePtr->OVal = Div;
						Status =  XST_SUCCESS;
					}

				}
			}
		}
	}
	return Status;
}

/****************************************************************************/
/**
* @brief Calculates M, D, and O values for a rate in Hz.
*
* Calculate the M, D, and O values for the given SetRate frequency.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
* @param[in] SetRate Frequency for which the M, D and O values are to be
*   calculated in Hz.
*
* @return
*		- XST_SUCCESS if the divisors were calculated.
*		- XST_FAILURE otherwise.
*
*****************************************************************************/
static u32  XClk_Wiz_CalculateDivisorsHz (XClk_Wiz  *InstancePtr, u64 SetRate)
{
	u32 m;
	u32 d;
	u32 Div;
	u64 Fvco;
	u64 Freq;
	u64 Diff;
	u64 VcoMin;
	u64 VcoMax;
	u32 InputRate;
	u32 Platform;
	u32 Mmin;
	u32 Mmax;
	u64 Pfd;
	u32 Dmin;
	u32 Dmax;
	u32 Omin;
	u32 Omax;
	u64 PfdMin;
	u64 PfdMax;
	u64 BestDiv = -1ULL;

	Platform = XGetPlatform_Info();

	if (Platform == (u32)XPLAT_VERSAL) {
		VcoMin = XCLK_VCO_MIN;
		VcoMax = XCLK_VCO_MAX;
		Mmin = XCLK_M_MIN;
		Mmax = XCLK_M_MAX;
		Dmin = XCLK_D_MIN;
		Dmax = XCLK_D_MAX;
		Omin = XCLK_O_MIN;
		Omax = XCLK_O_MAX;
		PfdMin = XCLK_PFD_MIN;
		PfdMax = XCLK_PFD_MAX;
	} else {
		VcoMin = XCLK_US_VCO_MIN;
		VcoMax = XCLK_US_VCO_MAX;
		Mmin = XCLK_US_M_MIN;
		Mmax = XCLK_US_M_MAX;
		Dmin = XCLK_US_D_MIN;
		Dmax = XCLK_US_D_MAX;
		Omin = XCLK_US_O_MIN;
		Omax = XCLK_US_O_MAX;
		PfdMin = XCLK_US_PFD_MIN;
		PfdMax = XCLK_US_PFD_MAX;
	}

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SetRate != 0);

	InputRate = XClk_Wiz_GetInputRate(InstancePtr);

	for (m = Mmin; m <= Mmax; m++) {
		for (d = Dmin; d <= Dmax; d++) {
			Pfd = (u64)InputRate / d;
			if (Pfd < PfdMin * XCLK_MHZ || Pfd > PfdMax * XCLK_MHZ) {
				continue;
			}
			Fvco = (u64)InputRate * m / d;
			if ( Fvco >= (VcoMin * XCLK_MHZ) && Fvco <= (VcoMax * XCLK_MHZ) ) {

				for (Div = Omin; Div <= Omax; Div++ ) {
					Freq = Fvco / Div;

					if (Freq > SetRate) {
						Diff = Freq - SetRate;
					} else {
						Diff = SetRate - Freq;
					}
					if (Diff < BestDiv) {
						BestDiv = Diff;
						InstancePtr->MVal = m;
						InstancePtr->DVal = d;
						InstancePtr->OVal = Div;
						if (!Diff)
							return XST_SUCCESS;
					}

				}
			}
		}
	}
	if (BestDiv == -1ULL) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Sets the minimum frequency-error margin.
*
* Set the Minimum error that can be tolerated.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
* @param[in] Minerr Error margin that can be tolerated in Hz.
*
* @note		Should be called only if there is only one output clock.
*****************************************************************************/
void XClk_Wiz_SetMinErr(XClk_Wiz  *InstancePtr, u64 Minerr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	InstancePtr->MinErr  = Minerr;
}
/*****************************************************************************/
/**
* @brief Programs the output-divider registers for one clock.
*
* @param[in,out] InstancePtr Pointer to the CLK_WIZ instance.
* @param[in] ClockId Output clock identifier.
*
*****************************************************************************/
static void XClk_Wiz_UpdateO(XClk_Wiz  *InstancePtr, u32 ClockId)
{
	u32 HighTime;
	u32 DivEdge;
	u32 Reg;
	u32 P5Enable;
	u32 P5fEdge;
	u32 RegisterOffset;

	if (InstancePtr->OVal > XCLK_O_MAX) {
		InstancePtr->OVal = XCLK_O_MAX;
	}

	if (ClockId < 3) {
		RegisterOffset = XCLK_WIZ_REG3_OFFSET + ClockId * 8;
	} else {
		RegisterOffset = XCLK_WIZ_REG19_OFFSET + ClockId * 8;
	}
	HighTime = (InstancePtr->OVal / 4);
	Reg = XCLK_WIZ_REG3_PREDIV2 | XCLK_WIZ_REG3_USED | XCLK_WIZ_REG3_MX;
	if (InstancePtr->OVal % 4 <= 1) {
		DivEdge = 0;
	} else {
		DivEdge = 1;
	}
	Reg |= (DivEdge << 8);
	P5fEdge = InstancePtr->OVal % 2;
	P5Enable = InstancePtr->OVal % 2;
	Reg = Reg | P5Enable << XCLK_WIZ_CLKOUT0_P5EN_SHIFT | P5fEdge << XCLK_WIZ_CLKOUT0_P5FEDGE_SHIFT;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, RegisterOffset, Reg);
	Reg = HighTime | HighTime << 8;
	RegisterOffset = RegisterOffset + 4;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, RegisterOffset, Reg);
}
/*****************************************************************************/
/**
* @brief Programs the input-divider registers.
*
* @param[in] InstancePtr Pointer to the CLK_WIZ instance.
*
*****************************************************************************/
static void XClk_Wiz_UpdateD(XClk_Wiz  *InstancePtr)
{
	u32 HighTime;
	u32 DivEdge;
	u32 Reg;

	HighTime = (InstancePtr->DVal / 2);
	Reg  = 0;
	Reg = Reg & ~(1 << XCLK_WIZ_REG12_EDGE_SHIFT);
	DivEdge = InstancePtr->DVal % 2;
	Reg = Reg | DivEdge << XCLK_WIZ_REG12_EDGE_SHIFT;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG12_OFFSET, Reg);
	Reg = HighTime | HighTime << 8;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG13_OFFSET, Reg);

}

/*****************************************************************************/
/**
* @brief Programs the feedback-multiplier and lock registers.
*
* @param[in] InstancePtr Pointer to the CLK_WIZ instance.
*
*****************************************************************************/
static void XClk_Wiz_UpdateM(XClk_Wiz  *InstancePtr)
{
	u32 HighTime;
	u32 DivEdge;
	u32 Reg;
	u32 Cp = 15, Res = 15;
	u32 LockRefDly = 16;
	u32 LockFbDly  = 16;
	u32 LockCnt	 = 250;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG25_OFFSET, 0);

	DivEdge = InstancePtr->MVal % 2;
	HighTime = InstancePtr->MVal / 2;
	Reg = HighTime | HighTime << 8;
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG2_OFFSET, Reg);
	Reg = XCLK_WIZ_REG1_PREDIV2 | XCLK_WIZ_REG1_EN | XCLK_WIZ_REG1_MX;

	if (DivEdge) {
		Reg = Reg | (1 << XCLK_WIZ_REG1_EDGE_SHIFT);
	} else {
		Reg = Reg & ~(1 << XCLK_WIZ_REG1_EDGE_SHIFT);
	}
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG1_OFFSET, Reg);

	if (InstancePtr->MVal == 4) {
		Cp = 5;
		Res = 15;
	} else if (InstancePtr->MVal == 5) {
		Cp = 6;
		Res = 15;
	} else if (InstancePtr->MVal == 6) {
		Cp = 7;
		Res = 15;
	} else if (InstancePtr->MVal == 7) {
		Cp = 13;
		Res = 15;
	} else if (InstancePtr->MVal == 8) {
		Cp = 14;
		Res = 15;
	} else if (InstancePtr->MVal == 9) {
		Cp = 15;
		Res = 15;
	} else if (InstancePtr->MVal == 10) {
		Cp = 14;
		Res = 7;
	} else if (InstancePtr->MVal == 11) {
		Cp = 15;
		Res = 7;
	} else if (InstancePtr->MVal >= 12 && InstancePtr->MVal <= 13) {
		Cp = 15;
		Res = 11;
	} else if (InstancePtr->MVal == 14) {
		Cp = 15;
		Res = 13;
	} else if (InstancePtr->MVal == 15) {
		Cp = 15;
		Res = 3;
	} else if (InstancePtr->MVal >= 16 && InstancePtr->MVal <= 17) {
		Cp = 14;
		Res = 5;
	} else if (InstancePtr->MVal >= 18 && InstancePtr->MVal <= 19) {
		Cp = 15;
		Res = 5;
	} else if (InstancePtr->MVal >= 20 && InstancePtr->MVal <= 21) {
		Cp = 15;
		Res = 9;
	} else if (InstancePtr->MVal >= 22 && InstancePtr->MVal <= 23) {
		Cp = 14;
		Res = 14;
	} else if (InstancePtr->MVal >= 24 && InstancePtr->MVal <= 26) {
		Cp = 15;
		Res = 14;
	} else if (InstancePtr->MVal >= 27 && InstancePtr->MVal <= 28) {
		Cp = 14;
		Res = 1;
	} else if (InstancePtr->MVal >= 29 && InstancePtr->MVal <= 33) {
		Cp = 15;
		Res = 1;
	} else if (InstancePtr->MVal >= 34 && InstancePtr->MVal <= 37) {
		Cp = 14;
		Res = 6;
	} else if (InstancePtr->MVal >= 38 && InstancePtr->MVal <= 44) {
		Cp = 15;
		Res = 6;
	} else if (InstancePtr->MVal >= 45 && InstancePtr->MVal <= 57) {
		Cp = 15;
		Res = 10;
	} else if (InstancePtr->MVal >= 58 && InstancePtr->MVal <= 63) {
		Cp = 13;
		Res = 12;
	} else if (InstancePtr->MVal >= 64 && InstancePtr->MVal <= 70) {
		Cp = 14;
		Res = 12;
	} else if (InstancePtr->MVal >= 71 && InstancePtr->MVal <= 86) {
		Cp = 15;
		Res = 12;
	} else if (InstancePtr->MVal >= 87 && InstancePtr->MVal <= 94) {
		Cp = 14;
		Res = 2;
	} else if (InstancePtr->MVal >= 95 && InstancePtr->MVal <= 145) {
		Cp = 15;
		Res = 2;
	} else if (InstancePtr->MVal >= 146 && InstancePtr->MVal <= 163) {
		Cp = 12;
		Res = 4;
	} else if (InstancePtr->MVal >= 164 && InstancePtr->MVal <= 181) {
		Cp = 13;
		Res = 4;
	} else if (InstancePtr->MVal >= 182 && InstancePtr->MVal <= 200) {
		Cp = 14;
		Res = 4;
	} else if (InstancePtr->MVal >= 201 && InstancePtr->MVal <= 273) {
		Cp = 15;
		Res = 4;
	} else if (InstancePtr->MVal >= 274 && InstancePtr->MVal <= 300) {
		Cp = 13;
		Res = 8;
	} else if (InstancePtr->MVal >= 301 && InstancePtr->MVal <= 325) {
		Cp = 14;
		Res = 8;
	} else if (InstancePtr->MVal >= 326 && InstancePtr->MVal <= 432) {
		Cp = 15;
		Res = 8;
	}

	Reg = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG11_OFFSET);
	Reg = Reg | (Cp & XCLK_WIZ_REG11_CP_MASK);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG11_OFFSET, Reg);

	Reg = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG17_OFFSET);
	Reg = Reg | ((Res & XCLK_WIZ_REG17_RES_MASK) << XCLK_WIZ_REG17_RES_SHIFT);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG17_OFFSET, Reg);

	if (InstancePtr->MVal == 4) {
		LockRefDly = 4;
		LockFbDly  = 4;
		LockCnt	 = 1000;
	} else if (InstancePtr->MVal == 5) {
		LockRefDly = 6;
		LockFbDly  = 6;
		LockCnt	 = 1000;
	} else if (InstancePtr->MVal >= 6 && InstancePtr->MVal <= 7) {
		LockRefDly = 7;
		LockFbDly  = 7;
		LockCnt	 = 1000;
	} else if (InstancePtr->MVal == 8) {
		LockRefDly = 7;
		LockFbDly  = 7;
		LockCnt	 = 1000;
	} else if (InstancePtr->MVal >= 9 && InstancePtr->MVal <= 12) {
		LockRefDly = 8;
		LockFbDly  = 8;
		LockCnt	 = 1000;
	} else if (InstancePtr->MVal == 13) {
		LockRefDly = 10;
		LockFbDly  = 10;
		LockCnt	 = 1000;
	} else if (InstancePtr->MVal >= 14 && InstancePtr->MVal <= 16) {
		LockRefDly = 13;
		LockFbDly  = 13;
		LockCnt	 = 1000;
	} else if (InstancePtr->MVal == 17) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 825;
	} else if (InstancePtr->MVal == 18) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 750;
	} else if (InstancePtr->MVal >= 19 && InstancePtr->MVal <= 20) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 700;
	} else if (InstancePtr->MVal == 21) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 650;
	} else if (InstancePtr->MVal >= 22 && InstancePtr->MVal <= 23) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 625;
	} else if (InstancePtr->MVal == 24) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 575;
	} else if (InstancePtr->MVal == 25) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 550;
	} else if (InstancePtr->MVal >= 26 && InstancePtr->MVal <= 28) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 525;
	} else if (InstancePtr->MVal >= 29 && InstancePtr->MVal <= 30) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 475;
	} else if (InstancePtr->MVal == 31) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 450;
	} else if (InstancePtr->MVal >= 32 && InstancePtr->MVal <= 33) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 425;
	} else if (InstancePtr->MVal >= 34 && InstancePtr->MVal <= 36) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 400;
	} else if (InstancePtr->MVal == 37) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 375;
	} else if (InstancePtr->MVal >= 38 && InstancePtr->MVal <= 40) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 350;
	} else if (InstancePtr->MVal >= 41 && InstancePtr->MVal <= 43) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 325;
	} else if (InstancePtr->MVal >= 44 && InstancePtr->MVal <= 47) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 300;
	} else if (InstancePtr->MVal >= 48 && InstancePtr->MVal <= 51) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 275;
	} else if (InstancePtr->MVal >= 52 && InstancePtr->MVal <= 205) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 250;
	} else if (InstancePtr->MVal >= 206 && InstancePtr->MVal <= 432) {
		LockRefDly = 16;
		LockFbDly  = 16;
		LockCnt	 = 225;
	}

	Reg = LockCnt | LockFbDly << XCLK_WIZ_REG15_LOCK_FB_DLY_SHIFT;
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG15_OFFSET, Reg);
	Reg = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG16_OFFSET);
	Reg = Reg | (LockRefDly << XCLK_WIZ_REG16_LOCK_REF_DLY_SHIFT);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG16_OFFSET, Reg);
}

/****************************************************************************/
/**
* @brief Stores M, D, and O divisor values on the instance.
*
* Set the M, D, and O divisor values directly on the instance.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
* @param[in] Mvalue Feedback multiplier value.
* @param[in] Dvalue Input divider value.
* @param[in] Ovalue Output divider value.
*
* @return
*		- XST_SUCCESS when divisor values are valid.
*		- XST_FAILURE when divisor values are invalid or unsupported.
*
* @note		Call XClk_Wiz_Reconfig() after this to apply the new values.
*		Should be called only if there is only one output clock.
*****************************************************************************/
u32 XClk_Wiz_UpdateDivisors(XClk_Wiz  *InstancePtr, u32 Mvalue, u32 Dvalue, u32 Ovalue)
{
	u32 Platform;
	u32 Mmin;
	u32 Mmax;
	u32 Dmin;
	u32 Dmax;
	u32 Omin;
	u32 Omax;
	u32 Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->Config.NumClocks != 1) {
		return Status;
	}

	Platform = XGetPlatform_Info();

	if (Platform == (u32)XPLAT_VERSAL) {
		Mmin = XCLK_M_MIN;
		Mmax = XCLK_M_MAX;
		Dmin = XCLK_D_MIN;
		Dmax = XCLK_D_MAX;
		Omin = XCLK_O_MIN;
		Omax = XCLK_O_MAX;
	} else {
		Mmin = XCLK_US_M_MIN;
		Mmax = XCLK_US_M_MAX;
		Dmin = XCLK_US_D_MIN;
		Dmax = XCLK_US_D_MAX;
		Omin = XCLK_US_O_MIN;
		Omax = XCLK_US_O_MAX;
	}

	if ((Mvalue < Mmin) || (Mvalue > Mmax) ||
	    (Dvalue < Dmin) || (Dvalue > Dmax) ||
	    (Ovalue < Omin) || (Ovalue > Omax)) {
		return XST_FAILURE;
	}

	InstancePtr->MVal = Mvalue;
	InstancePtr->DVal = Dvalue;
	InstancePtr->OVal = Ovalue;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Reconfigures CLK_WIZ hardware from stored divisors.
*
* Reconfigure the clock wizard hardware with the current M, D, and O values
* stored in the instance. For Versal, updates output, input, and feedback
* divider registers then triggers a load. For UltraScale/ZynqMP, writes the
* combined M/D and O registers directly.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
*
* @return
*		- XST_SUCCESS always.
*
*****************************************************************************/
u32 XClk_Wiz_Reconfig(XClk_Wiz  *InstancePtr)
{
	u32 Platform;
	u32 Reg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Platform = XGetPlatform_Info();

	if (Platform != (u32)XPLAT_VERSAL) {
		Reg = InstancePtr->MVal << 8 | InstancePtr->DVal;
		XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_ZYNQMP_REG0_OFFSET, Reg);
		Reg = InstancePtr->OVal;
		XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_ZYNQMP_REG2_OFFSET, Reg);
		return XST_SUCCESS;
	}

	/* Implement O */
	XClk_Wiz_UpdateO(InstancePtr, 0);
	/* Implement D */
	XClk_Wiz_UpdateD(InstancePtr);
	/* Implement M */
	XClk_Wiz_UpdateM(InstancePtr);
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_RECONFIG_OFFSET,
			  (XCLK_WIZ_RECONFIG_LOAD | XCLK_WIZ_RECONFIG_SADDR));

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Sets the output frequency in Hz.
*
* Change the frequency to the given rate in Hz.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
* @param[in] SetRate Frequency in Hz to be set.
*
* @return
*		- XST_SUCCESS Frequency setting was successful.
*		- XST_FAILURE frequency setting failed.
*
* @note		Should be called only if there is only one output clock.
*****************************************************************************/
u32 XClk_Wiz_SetRateHz(XClk_Wiz  *InstancePtr, u64 SetRate)
{
	u32 Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SetRate != 0);

	if (InstancePtr->Config.NumClocks  != 1 ) {
		return Status;
	}

	Status = XClk_Wiz_CalculateDivisorsHz(InstancePtr, SetRate);
	if ( Status != XST_SUCCESS) {
		return Status;
	}
	XClk_Wiz_Reconfig(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Returns the VCO frequency in Hz.
*
* Computes the frequency from the input rate and the divider
* registers.
*
* @param[in] InstancePtr Pointer to the CLK_WIZ instance.
*
* @return VCO frequency in Hz.
*
*****************************************************************************/
static u64 XClk_Wiz_GetVco(XClk_Wiz  *InstancePtr)
{
	u32 Reg;
	u32 Div;
	u64 Fvco;
	u32 Edge;
	u32 Low;
	u32 High;
	u32 Mult;
	u32 Platform;

	Platform = XGetPlatform_Info();

	if (Platform != (u32)XPLAT_VERSAL) {
		Reg = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_ZYNQMP_REG0_OFFSET);
		Div = Reg & XCLK_WIZ_REG0_DIV_MASK;
		Mult = (Reg & XCLK_WIZ_REG0_FBMULT_MASK) >> XCLK_WIZ_REG0_FBMULT_SHIFT;
	} else {
		Reg = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG1_OFFSET);
		Edge = !!(Reg & XCLK_WIZ_REG1_EDGE_MASK);
		Reg = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG2_OFFSET);
		Low = Reg & XCLK_WIZ_CLKFBOUT_L_MASK;
		High = (Reg & XCLK_WIZ_CLKFBOUT_H_MASK) >> XCLK_WIZ_CLKFBOUT_H_SHIFT;
		Mult = Low + High + Edge;

		Reg = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG13_OFFSET);
		Low = Reg & XCLK_WIZ_CLKFBOUT_L_MASK;
		High = (Reg & XCLK_WIZ_CLKFBOUT_H_MASK) >> XCLK_WIZ_CLKFBOUT_H_SHIFT;
		Reg = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG12_OFFSET);
		Edge  = !!(Reg & XCLK_WIZ_REG12_EDGE_MASK);
		Div = Low + High + Edge;
	}

	if (!Mult) {
		Mult = 1;
	}
	if (!Div) {
		Div = 1;
	}

	Fvco = (u64)XClk_Wiz_GetInputRate(InstancePtr) * Mult / Div;
	return Fvco;
}
/****************************************************************************/
/**
* @brief Gets the output frequency for a clock ID.
*
* Get the clock frequency for the given ClockId.
*
* @param[in] InstancePtr XClk_Wiz instance to operate on.
* @param[in] ClockId Output clock.
* @param[out] Rate Clock rate in Hz.
*
* @return
*		- XST_SUCCESS getting the frequency was successful.
*		- XST_FAILURE getting the frequency failed.
*
*****************************************************************************/
s32 XClk_Wiz_GetRate(XClk_Wiz  *InstancePtr, u32 ClockId, u64 *Rate)
{
	u32 Status = XST_FAILURE;
	u32 Platform;
	u32 Reg;
	u32 Leaf;
	u64 Fvco;
	u64 Freq;
	u32 RegisterOffset;
	u32 Edge;
	u32 Low;
	u32 High;
	u32 DivO;
	u32 P5en;
	u32 Prediv;

	Platform = XGetPlatform_Info();

	if (InstancePtr->Config.NumClocks  < ClockId) {
		return Status;
	}

	Fvco = XClk_Wiz_GetVco(InstancePtr);

	if (Platform != (u32)XPLAT_VERSAL) {
		RegisterOffset = XCLK_WIZ_ZYNQMP_REG2_OFFSET + ClockId * 12;
		Reg = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, RegisterOffset);
		DivO = Reg & XCLK_WIZ_REG2_DIV_MASK;
		Freq = Fvco / DivO;
		*Rate = Freq;

		return (s32)XST_SUCCESS;
	} else {
		if (ClockId < 3) {
			RegisterOffset = XCLK_WIZ_REG3_OFFSET + ClockId * 8;
		} else {
			RegisterOffset = XCLK_WIZ_REG19_OFFSET + ClockId * 8;
		}

		Reg = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, RegisterOffset);
		Edge  = !!(Reg & XCLK_WIZ_REG3_EDGE_MASK);
		P5en  = !!(Reg & XCLK_WIZ_REG3_P5EN_MASK);
		Prediv  = !!(Reg & XCLK_WIZ_REG3_PREDIV2);

		RegisterOffset = RegisterOffset + 4;
		Reg = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, RegisterOffset);
		Low = Reg & XCLK_WIZ_CLKFBOUT_L_MASK;
		High = (Reg & XCLK_WIZ_CLKFBOUT_H_MASK) >> XCLK_WIZ_CLKFBOUT_H_SHIFT;
		Leaf = High + Low + Edge;
		DivO = (Prediv + 1) * Leaf + (Prediv * P5en);
	}
	if (!DivO) {
		DivO = 1;
	}
	Freq = Fvco / DivO;
	*Rate = Freq;

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Sets one output-clock frequency in Hz.
*
* Set the clock rate frequency for the given ClockId.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
* @param[in] ClockId Output clock.
* @param[in] SetRate Clock rate in Hz.
*
* @return
*		- XST_SUCCESS Setting the rate was successful.
*		- XST_FAILURE Setting rate was failed.
*
*****************************************************************************/
s32 XClk_Wiz_SetLeafRateHz(XClk_Wiz  *InstancePtr, u32 ClockId, u64 SetRate)
{
	u32 Platform;
	u32 DivO;
	u32 RegisterOffset;
	u64 Fvco;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SetRate != 0);

	Platform = XGetPlatform_Info();

	Fvco = XClk_Wiz_GetVco(InstancePtr);

	if (Platform != (u32)XPLAT_VERSAL) {
		if (SetRate > Fvco) {
			return XST_FAILURE;
		}

		RegisterOffset = XCLK_WIZ_ZYNQMP_REG2_OFFSET + ClockId * 12;
		DivO = Fvco / SetRate;
		if (DivO > XCLK_US_O_MAX) {
			DivO = XCLK_US_O_MAX;
		}
		if (DivO < XCLK_US_O_MIN ) {
			DivO = XCLK_US_O_MIN;
		}
		XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, RegisterOffset, DivO);
		return XST_SUCCESS;
	}

	InstancePtr->OVal = Fvco / SetRate;
	XClk_Wiz_UpdateO(InstancePtr, ClockId);

	return XST_SUCCESS;
}
/****************************************************************************/
/**
* @brief Sets the output frequency in MHz.
*
* Change the frequency to the given rate.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
* @param[in] SetRate Frequency for which is requested in MHz.
*
* @return
*		- XST_SUCCESS frequency setting was successful.
*		- XST_FAILURE frequency setting failed.
*
* @note		Should be called only if there is only one output clock.
*****************************************************************************/
u32 XClk_Wiz_SetRate(XClk_Wiz  *InstancePtr, u64 SetRate)
{
	u32 Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SetRate != 0);

	if (InstancePtr->Config.NumClocks  != 1 ) {
		return Status;
	}

	Status = XClk_Wiz_CalculateDivisors(InstancePtr, SetRate);
	if ( Status != XST_SUCCESS) {
		return Status;
	}
	XClk_Wiz_Reconfig(InstancePtr);

	return XST_SUCCESS;
}
/****************************************************************************/
/**
* @brief Enables the specified output clock.
*
* Enable Clock for the given ClockId.
*
* @param[in] InstancePtr XClk_Wiz instance to operate on.
* @param[in] ClockId Output clock.
*
* @return
*		- XST_SUCCESS if the clock was enabled.
*		- XST_FAILURE otherwise.
*
*****************************************************************************/
u32 XClk_Wiz_EnableClock(XClk_Wiz  *InstancePtr, u32 ClockId)
{
	u32 Platform;
	u32 RegisterOffset;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (ClockId > XCLK_WIZ_MAX_OUTPUT) {
		return XST_FAILURE;
	}

	Platform = XGetPlatform_Info();

	if (Platform != (u32)XPLAT_VERSAL) {
		return XST_SUCCESS;
	}

	if (ClockId < 3) {
		RegisterOffset = XCLK_WIZ_REG3_OFFSET + ClockId * 8 ;
	} else {
		RegisterOffset = XCLK_WIZ_REG19_OFFSET + ClockId * 8 ;
	}

	XCLK_WIZ_BIT_SET(InstancePtr->Config.BaseAddr, RegisterOffset, XCLK_WIZ_REG3_USED);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Disables the specified output clock.
*
* Disable Clock for the given ClockId.
*
* @param[in] InstancePtr XClk_Wiz instance to operate on.
* @param[in] ClockId Output clock.
*
* @return
*		- XST_SUCCESS if the clock was disabled.
*		- XST_FAILURE otherwise.
*
*****************************************************************************/
u32 XClk_Wiz_DisableClock(XClk_Wiz  *InstancePtr, u32 ClockId)
{
	u32 Platform;
	u32 RegisterOffset;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Platform = XGetPlatform_Info();

	if (Platform != (u32)XPLAT_VERSAL) {
		return XST_SUCCESS;
	}

	if (ClockId > XCLK_WIZ_MAX_OUTPUT) {
		return XST_FAILURE;
	}

	if (ClockId < 3) {
		RegisterOffset = XCLK_WIZ_REG3_OFFSET + ClockId * 8 ;
	} else {
		RegisterOffset = XCLK_WIZ_REG19_OFFSET + ClockId * 8 ;
	}

	XCLK_WIZ_BIT_RESET(InstancePtr->Config.BaseAddr, RegisterOffset, XCLK_WIZ_REG3_USED);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Waits until the clocking wizard locks.
*
* Wait till the clocking wizard is locked to the frequency.
*
* @param[in] InstancePtr XClk_Wiz instance to operate on.
*
* @return
*		- XST_SUCCESS if lock was successful.
*		- XST_FAILURE on timeout.
*
*****************************************************************************/
u32 XClk_Wiz_WaitForLock(XClk_Wiz  *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return Xil_WaitForEvent(InstancePtr->Config.BaseAddr + XCLK_WIZ_STATUS_OFFSET, XCLK_WIZ_LOCK,
					  XCLK_WIZ_LOCK, 100U);

}
/****************************************************************************/
/**
* @brief Sets the input clock frequency.
*
* Change the Input frequency to the given rate.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
* @param[in] Rate Frequency for which is to be set.
*
* @note		Should be called only if the input provider clock is
*		changed, for example input clock is si570.
*****************************************************************************/
void XClk_Wiz_SetInputRate(XClk_Wiz  *InstancePtr, double Rate)
{
#ifdef SDT
	u64 RateKhz;
	u32 RegLow;
	u32 RegHigh = 0;
	u32 RegValue;
#endif

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Rate != 0);

#ifdef SDT
	RateKhz = (u64)(Rate / (double)XCLK_KHZ);
	RegLow = RateKhz & 0xFFFF;
	RegHigh = (RateKhz >> 16 ) & 0xFFFF;

	if ((strncmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-", 15U) == 0) &&
	    (strcmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-1.0") > 0)) {
		RegValue = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
					   XCLK_WIZ_CLKINSEL_OFFSET);
		if ((RegValue & XCLK_WIZ_CLKINSEL_FINAL_MASK) != 0U) {
			InstancePtr->Config.PrimInClkFreq  = (u64)Rate;
			XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr,
					  XCLK_WIZ_PRIM_L_OFFSET, RegLow);
			XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr,
					  XCLK_WIZ_PRIM_H_OFFSET, RegHigh);
		} else {
			XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr,
					  XCLK_WIZ_SEC_L_OFFSET, RegLow);
			XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr,
					  XCLK_WIZ_SEC_H_OFFSET, RegHigh);
		}
	} else if (strcmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-1.0") == 0) {
		XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr,
				  XCLK_WIZ_PRIM_L_OFFSET, RegLow);
		XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr,
				  XCLK_WIZ_PRIM_H_OFFSET, RegHigh);
	} else {
		InstancePtr->Config.PrimInClkFreq  = (u64)Rate;
	}
#else
	InstancePtr->Config.PrimInClkFreq  = Rate / XCLK_MHZ;
#endif

}

/****************************************************************************/
/**
* @brief Gets the input clock frequency in Hz.
*
* Get the Input frequency.
*
* @param[in] InstancePtr XClk_Wiz instance to operate on.
*
* @return 	Clock frequency in Hz
*
* @note		Should be called only if the input provider clock is
*		changed, for example input clock is si570.
*****************************************************************************/
u32 XClk_Wiz_GetInputRate(XClk_Wiz  *InstancePtr)
{
#ifdef SDT
	u32 RegLow;
	u32 RateKhz = 0;
	u32 RegValue;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if ((strncmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-", 15U) == 0) &&
	    (strcmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-1.0") > 0)) {
		RegValue = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
					   XCLK_WIZ_CLKINSEL_OFFSET);

		if ((RegValue & XCLK_WIZ_CLKINSEL_FINAL_MASK) != 0U) {
			RegLow = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
						  XCLK_WIZ_PRIM_L_OFFSET);
			RateKhz = (XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
						   XCLK_WIZ_PRIM_H_OFFSET) << 16) | RegLow;
		} else {
			RegLow = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
						  XCLK_WIZ_SEC_L_OFFSET);
			RateKhz = (XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
						   XCLK_WIZ_SEC_H_OFFSET) << 16) | RegLow;
		}
	} else if (strcmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-1.0") == 0) {
		RegLow = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_PRIM_L_OFFSET);
		RateKhz = (XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_PRIM_H_OFFSET) << 16) | RegLow;
	} else {
		/* Non-clkx5 SDT uses the configured primary input rate. */
		return (u32)InstancePtr->Config.PrimInClkFreq;
	}

	return RateKhz * XCLK_KHZ;
#else
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	return (u32)(InstancePtr->Config.PrimInClkFreq * (double)XCLK_MHZ);
#endif
}
/*****************************************************************************/
/**
* @brief Reads CLK_WIZ interrupt enable and status.
*
* XClk_Wiz_GetInterruptSettings gets the information from  clock wizard
* IER  and ISR Registers.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
*
****************************************************************************/
void XClk_Wiz_GetInterruptSettings(XClk_Wiz  *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->ClkWizIntrStatus = XCLK_WIZ_GET_BITFIELD_VALUE
					((InstancePtr)->Config.BaseAddr, XCLK_WIZ_ISR_OFFSET,
					 XCLK_WIZ_ISR_ALLINTR_MASK, XCLK_WIZ_ISR_ALLINTR_SHIFT);
	InstancePtr->ClkIntrEnable = XCLK_WIZ_GET_BITFIELD_VALUE
				     ((InstancePtr)->Config.BaseAddr, XCLK_WIZ_IER_OFFSET,
				      XCLK_WIZ_IER_ALLINTR_MASK, XCLK_WIZ_IER_ALLINTR_SHIFT);
}

/*****************************************************************************/
/**
* @brief Selects the software-controlled clock input.
*
* Set the clock input selection via the software-programmed value.
*
* @param[in] InstancePtr XClk_Wiz instance to operate on.
* @param[in] Clkinputnumber Input clock selection value
*   (XCLK_WIZ_CLKINSEL_CLKIN2 for clk_in2, XCLK_WIZ_CLKINSEL_CLKIN1 for
*   clk_in1).
*
* @return
*		- XST_SUCCESS setting the input selection was successful.
* - XST_FAILURE setting the input selection failed (device not ready,
*		  unsupported build/device, invalid selection, or other errors).
*
* @note		This function sets Bit[16] (clk_in_sel_src) to 1 to enable
* software control and sets Bit[17] (clk_in_sel_reg) to the specified
*		value. The effective selection (Bit[18]) reflects this change.
* Supported only for SDT + xlnx,clkx5-wiz versions greater than 1.0.
*****************************************************************************/
u32 XClk_Wiz_SetClkInSel(XClk_Wiz  *InstancePtr, u32 Clkinputnumber)
{
	u32 Status = XST_FAILURE;
	u32 RegValue;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (Clkinputnumber > XCLK_WIZ_CLKINSEL_CLKIN1) {
		return Status;
	}

#ifdef SDT
	if ((strncmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-", 15U) != 0) ||
	    (strcmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-1.0") == 0)) {
		return XST_FAILURE;
	}
#else
	return XST_FAILURE;
#endif

	RegValue = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
				    XCLK_WIZ_CLKINSEL_OFFSET);

	RegValue &= ~(XCLK_WIZ_CLKINSEL_SRC_MASK | XCLK_WIZ_CLKINSEL_REG_MASK);
	RegValue |= XCLK_WIZ_CLKINSEL_SRC_MASK;

	if (Clkinputnumber == XCLK_WIZ_CLKINSEL_CLKIN1) {
		RegValue |= XCLK_WIZ_CLKINSEL_REG_MASK;
	}

	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_CLKINSEL_OFFSET,
			  RegValue);

	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
* @brief Gets the active clock-input selection.
*
* Get the current clock input selection.
*
* @param[in] InstancePtr XClk_Wiz instance to operate on.
* @param[out] Clkinputnumber Pointer to store input clock selection.
*
* @return
*		- XST_SUCCESS if selection read was successful.
* - XST_FAILURE if arguments are invalid or operation is unsupported
*		  on this build/device.
*
* @note		This function reads Bit[18] (clk_in_sel_final), which is the
*		effective clock input selection currently applied to the MMCM.
* Supported only for SDT + xlnx,clkx5-wiz versions greater than 1.0.
*****************************************************************************/
u32 XClk_Wiz_GetClkInSel(XClk_Wiz  *InstancePtr, u32 *Clkinputnumber)
{
	u32 RegValue;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Clkinputnumber != NULL);

#ifdef SDT
	if ((strncmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-", 15U) != 0) ||
	    (strcmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-1.0") == 0)) {
		return XST_FAILURE;
	}
#else
	return XST_FAILURE;
#endif

	RegValue = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
				    XCLK_WIZ_CLKINSEL_OFFSET);

	if ((RegValue & XCLK_WIZ_CLKINSEL_FINAL_MASK) != 0U) {
		*Clkinputnumber = XCLK_WIZ_CLKINSEL_CLKIN1;
	} else {
		*Clkinputnumber = XCLK_WIZ_CLKINSEL_CLKIN2;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Gets an input-clock frequency in kHz.
*
* Get the input clock frequency in kHz.
*
* @param[in] InstancePtr XClk_Wiz instance to operate on.
* @param[in] Clkinputnumber Input clock number to query (0 for PRIMARY, 1 for
*   SECONDARY).
* @param[out] RateKhz Pointer to store the input clock frequency in kHz.
*
* @return
*		- XST_SUCCESS getting the input frequency was successful.
* - XST_FAILURE getting the input frequency failed (device not ready,
*		  unsupported clock number, invalid pointer, or other errors).
*
* @note		Supports both PRIMARY (0) and SECONDARY (1) input clocks.
* 		PRIMARY uses XClk_Wiz_GetInputRate internally.
* SECONDARY is supported only for SDT + xlnx,clkx5-wiz versions greater than
*   1.0;
* 		otherwise this API returns XST_FAILURE.
*****************************************************************************/
u32 XClk_Wiz_GetInFreqkHz(XClk_Wiz  *InstancePtr, u32 Clkinputnumber, u32 *RateKhz)
{
	u32 Status = XST_FAILURE;
	u32 Freq;
#ifdef SDT
	u32 RegLow;
	u32 RegHigh = 0;
	u32 ActiveInput;
	u32 SelStatus;
#endif

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(RateKhz != NULL);

	if (Clkinputnumber > 1U) {
		return Status;
	}

#ifdef SDT
	if ((strncmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-", 15U) == 0) &&
	    (strcmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-1.0") > 0)) {
		SelStatus = XClk_Wiz_GetClkInSel(InstancePtr, &ActiveInput);
		if (SelStatus != XST_SUCCESS) {
			return Status;
		}

		if (((Clkinputnumber == 0U) && (ActiveInput == XCLK_WIZ_CLKINSEL_CLKIN1)) ||
		    ((Clkinputnumber == 1U) && (ActiveInput == XCLK_WIZ_CLKINSEL_CLKIN2))) {
			Freq = XClk_Wiz_GetInputRate(InstancePtr);
			if (Freq == 0U) {
				return Status;
			}
			*RateKhz = Freq / XCLK_KHZ;
		} else if (Clkinputnumber == 0U) {
			/* Requested PRIMARY while SECONDARY is active. */
			RegLow = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
						  XCLK_WIZ_PRIM_L_OFFSET);
			RegHigh = (XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
						    XCLK_WIZ_PRIM_H_OFFSET) << 16) | RegLow;
			*RateKhz = RegHigh;
		} else {
			/* Requested SECONDARY while PRIMARY is active. */
			RegLow = XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
						  XCLK_WIZ_SEC_L_OFFSET);
			RegHigh = (XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,
						    XCLK_WIZ_SEC_H_OFFSET) << 16) | RegLow;
			*RateKhz = RegHigh;
		}
	} else {
		/* Non-clkx5 SDT has only the PRIMARY config frequency. */
		if (Clkinputnumber != 0U) {
			return Status;
		}

		Freq = XClk_Wiz_GetInputRate(InstancePtr);
		if (Freq == 0U) {
			return Status;
		}
		*RateKhz = Freq / XCLK_KHZ;
	}
#else
	if (Clkinputnumber != 0U) {
		return Status;
	}

	Freq = XClk_Wiz_GetInputRate(InstancePtr);
	if (Freq == 0U) {
		return Status;
	}
	*RateKhz = Freq / XCLK_KHZ;
#endif

	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
* @brief Sets an input-clock frequency in kHz.
*
* Set the input clock frequency in kHz.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
* @param[in] Clkinputnumber Input clock number to configure (0 for PRIMARY, 1
*   for SECONDARY).
* @param[in] RateKhz Input clock frequency in kHz to be set.
*
* @return
*		- XST_SUCCESS setting the input frequency was successful.
* - XST_FAILURE setting the input frequency failed (device not ready,
*		  unsupported clock number, or other errors).
*
* @note		Supports both PRIMARY (0) and SECONDARY (1) input clocks.
* On SDT + xlnx,clkx5-wiz > 1.0, this API switches clk_in_sel to the
* requested input before programming the rate via XClk_Wiz_SetInputRate.
* Otherwise, SECONDARY is unsupported and this API returns XST_FAILURE.
*****************************************************************************/
u32 XClk_Wiz_SetInFreqkHz(XClk_Wiz  *InstancePtr, u32 Clkinputnumber, u32 RateKhz)
{
	double RateHz;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(RateKhz != 0);

	if (Clkinputnumber > 1U) {
		return XST_FAILURE;
	}

#ifndef SDT
	if (Clkinputnumber != 0U) {
		return XST_FAILURE;
	}
#else
	if ((Clkinputnumber != 0U) &&
	    ((strncmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-", 15U) != 0) ||
	     (strcmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-1.0") == 0))) {
		return XST_FAILURE;
	}
#endif

#ifdef SDT
	if ((strncmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-", 15U) == 0) &&
	    (strcmp(InstancePtr->Config.Name, "xlnx,clkx5-wiz-1.0") > 0)) {
		u32 SelVal = (Clkinputnumber == 0U) ? XCLK_WIZ_CLKINSEL_CLKIN1 : XCLK_WIZ_CLKINSEL_CLKIN2;
		u32 SelStatus = XClk_Wiz_SetClkInSel(InstancePtr, SelVal);
		if (SelStatus != XST_SUCCESS) {
			return SelStatus;
		}
	}
#endif
	RateHz = (double)RateKhz * (double)XCLK_KHZ;
	XClk_Wiz_SetInputRate(InstancePtr, RateHz);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Gets an output-clock frequency in kHz.
*
* Get the output clock frequency in kHz for a given output clock.
*
* @param[in] InstancePtr XClk_Wiz instance to operate on.
* @param[in] Clkoutputnumber Output clock number to query (0 to NumClocks-1).
* @param[out] RateKhz Pointer to store the output clock frequency in kHz.
*
* @return
*		- XST_SUCCESS getting the output frequency was successful.
* - XST_FAILURE getting the output frequency failed (device not ready,
*		  invalid clock number, invalid pointer, or other errors).
*
* @note The output clock number must be within the valid range defined by
* the device configuration (0 to NumClocks-1). Returned frequency is
*		in kHz.
*****************************************************************************/
u32 XClk_Wiz_GetOutFreqkHz(XClk_Wiz  *InstancePtr, u32 Clkoutputnumber, u32 *RateKhz)
{
	u32 Status = XST_FAILURE;
	u64 RateHz;
	s32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(RateKhz != NULL);

	if (Clkoutputnumber >= InstancePtr->Config.NumClocks) {
		return Status;
	}

	Result = XClk_Wiz_GetRate(InstancePtr, Clkoutputnumber, &RateHz);
	if (Result != XST_SUCCESS) {
		return Status;
	}

	*RateKhz = (u32)(RateHz / XCLK_KHZ);
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
* @brief Sets an output-clock frequency in kHz.
*
* Set the output clock frequency in kHz for a given output clock.
*
* @param[in,out] InstancePtr XClk_Wiz instance to operate on.
* @param[in] Clkoutputnumber Output clock number to configure (0 to
*   NumClocks-1).
* @param[in] RateKhz Output clock frequency in kHz to be set.
*
* @return
*		- XST_SUCCESS setting the output frequency was successful.
* - XST_FAILURE setting the output frequency failed (device not ready,
*		  invalid clock number, or other errors).
*
* @note The output clock number must be within the valid range defined by
*		the device configuration (0 to NumClocks-1).
* Clock 0 uses full reconfiguration; non-zero outputs use leaf-rate
*		programming via XClk_Wiz_SetLeafRateHz().
*****************************************************************************/
u32 XClk_Wiz_SetOutFreqkHz(XClk_Wiz  *InstancePtr, u32 Clkoutputnumber, u32 RateKhz)
{
	u32 Status = XST_FAILURE;
	u64 RateHz;
	s32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(RateKhz != 0);

	if (Clkoutputnumber >= InstancePtr->Config.NumClocks) {
		return Status;
	}

	RateHz = (u64)RateKhz * XCLK_KHZ;
	if (Clkoutputnumber == 0) {
		Status = XClk_Wiz_CalculateDivisorsHz(InstancePtr, RateHz);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		XClk_Wiz_Reconfig(InstancePtr);
	} else {
		Result = XClk_Wiz_SetLeafRateHz(InstancePtr, Clkoutputnumber, RateHz);
		if (Result != XST_SUCCESS) {
			return Status;
		}
	}

	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
* @brief Handles an unregistered CLK_WIZ error callback.
*
* This routine is a stub for the asynchronous error interrupt callback. The
* stub is here in case the upper layer forgot to set the handler. On
* initialization, Error interrupt handler is set to this callback. It is
* considered an error for this handler to be invoked.
*
* @param[in] CallBackRef Callback reference passed in by the upper layer when
*   setting the callback functions, and passed back to the upper layer when the
*   callback is invoked.
* @param[in] ErrorMask Bit mask indicating the cause of the error. Its value
*   equals 'OR'ing one or more XCLK_WIZ_ISR_*_MASK values defined in
*   xclk_wiz_hw.h.
*
******************************************************************************/
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask)
{
	(void) ((void *)CallBackRef);
	(void) ErrorMask;
	Xil_AssertVoidAlways();
}
/** @} */
