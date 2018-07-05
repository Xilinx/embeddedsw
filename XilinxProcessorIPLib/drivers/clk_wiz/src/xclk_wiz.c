/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xclk_wiz.c
* @addtogroup clk_wiz_v1_4
* @{
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
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xclk_wiz.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/*************************** Macros Definitions ******************************/


/************************** Function Prototypes ******************************/

/*
* Each of callback functions to be called on different types of interrupts.
* These stub functions are set during XClk_Wiz_CfgInitialize as default
* callback functions. If application is not registered any of the callback
* function, these functions will be called for doing nothing.
*/
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask);

/************************** Variable Definitions *****************************/


/****************************************************************************/
/**
* Initialize the XClk_Wiz instance provided by the caller based on the
* given Config structure.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
* @param	CfgPtr is the device configuration structure containing
*			information about a specific CLK_WIZ.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*		- XST_SUCCESS Initialization was successful.
*		- XST_FAILURE Initialization was failure.
*
* @note		None
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

	/* Set all handlers to stub values, let user configure this data later
	 */
	InstancePtr->ClkOutOfRangeCallBack  = StubErrCallBack;
	InstancePtr->ClkGlitchCallBack      = StubErrCallBack;
	InstancePtr->ClkStopCallBack        = StubErrCallBack;

	InstancePtr->ErrorCallBack = StubErrCallBack;
	InstancePtr->MinErr = 500000;

	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* Calculate the M, D, and O values for the given SetRate frequency.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
* @param	SetRate is the frequency for which the M, D and O values are to
*		be calculated.
*
* @return
*		- XST_SUCCESS Initialization was successful.
*		- XST_FAILURE Initialization was failure.
*
* @note		None
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
	u32 Platform;
	u32 Mmin;
	u32 Mmax;
	u32 Dmin;
	u32 Dmax;
	u32 Omin;
	u32 Omax;

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
	} else {
		VcoMin = XCLK_US_VCO_MIN;
		VcoMax = XCLK_US_VCO_MAX;
		Mmin = XCLK_US_M_MIN;
		Mmax = XCLK_US_M_MAX;
		Dmin = XCLK_US_D_MIN;
		Dmax = XCLK_US_D_MAX;
		Omin = XCLK_US_O_MIN;
		Omax = XCLK_US_O_MAX;
	}

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SetRate != 0);

	for (m = Mmin; m <= Mmax; m++) {
		for (d = Dmin; d <= Dmax; d++) {
			Fvco = InstancePtr->Config.PrimInClkFreq  * m / d;
			if ( Fvco >= VcoMin && Fvco <= VcoMax ) {

				for (Div = Omin; Div <= Omax; Div++ ) {
					Freq = Fvco/Div;

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
* Calculate the M, D, and O values for the given SetRate frequency.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
* @param	SetRate is the frequency for which the M, D and O values are to
*		be calculated in Hz.
*
* @return
*		- XST_SUCCESS Initialization was successful.
*		- XST_FAILURE Initialization was failure.
*
* @note		None
*****************************************************************************/
static u32  XClk_Wiz_CalculateDivisorsHz (XClk_Wiz  *InstancePtr, u64 SetRate)
{
	u32 m;
	u32 d;
	u32 Div;
	u64 Fvco;
	u64 Freq;
	u64 Diff;
	u64 Minerr = InstancePtr->MinErr;
	u32 Status = XST_FAILURE;
	u64 VcoMin;
	u64 VcoMax;
	u32 Platform;
	u32 Mmin;
	u32 Mmax;
	u32 Dmin;
	u32 Dmax;
	u32 Omin;
	u32 Omax;

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
	} else {
		VcoMin = XCLK_US_VCO_MIN;
		VcoMax = XCLK_US_VCO_MAX;
		Mmin = XCLK_US_M_MIN;
		Mmax = XCLK_US_M_MAX;
		Dmin = XCLK_US_D_MIN;
		Dmax = XCLK_US_D_MAX;
		Omin = XCLK_US_O_MIN;
		Omax = XCLK_US_O_MAX;
	}

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SetRate != 0);

	for (m = Mmin; m <= Mmax; m++) {
		for (d = Dmin; d <= Dmax; d++) {
			Fvco = InstancePtr->Config.PrimInClkFreq  * XCLK_MHZ * m / d;
			if ( Fvco >= VcoMin * XCLK_MHZ && Fvco <= VcoMax * XCLK_MHZ ) {

				for (Div = Omin; Div <= Omax; Div++ ) {
					Freq = Fvco/Div;

					if (Freq > SetRate) {
						Diff = Freq - SetRate;
					} else {
						Diff = SetRate - Freq;
					}
					if (Diff < Minerr) {
						InstancePtr->MVal = m;
						InstancePtr->DVal = d;
						InstancePtr->OVal = Div;
						return XST_SUCCESS;
					}

				}
			}
		}
	}
	return Status;
}

/****************************************************************************/
/**
* Set the Minimum error that can be tolerated.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
* @param	Minerr is the error margin that can be tolerated in Hz.
*
* @return	None
*
* @note		Should be called only if there is only one output clock.
*****************************************************************************/
void XClk_Wiz_SetMinErr(XClk_Wiz  *InstancePtr, u64 Minerr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	InstancePtr->MinErr  = Minerr;
}
/****************************************************************************/
/**
* Change the frequency to the given rate in Hz.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
* @param	SetRate is the frequency in Hz to be set.
*
* @return
*		- XST_SUCCESS Frequency setting was successful.
*		- XST_FAILURE frequency setting failed.
*
* @note		Should be called only if there is only one output clock.
*****************************************************************************/
u32 XClk_Wiz_SetRateHz(XClk_Wiz  *InstancePtr, u64 SetRate)
{
	u32 Platform;
	u32 HighTime;
	u32 DivEdge;
	u32 Reg;
	u32 P5Enable;
	u32 P5fEdge;
	u32 Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SetRate != 0);


	if (InstancePtr->Config.NumClocks  != 1 )
		return Status;

	Status = XClk_Wiz_CalculateDivisorsHz(InstancePtr, SetRate);
	if ( Status != XST_SUCCESS)
		return Status;

	Platform = XGetPlatform_Info();

	if(Platform != (u32)XPLAT_VERSAL) {
		Reg = InstancePtr->MVal << 8 | InstancePtr->DVal;
		XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_ZYNQMP_REG0_OFFSET, Reg);
		Reg =  InstancePtr->OVal;
		XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_ZYNQMP_REG2_OFFSET, Reg);
		return XST_SUCCESS;
	}

	/* Implement O */
	HighTime = (InstancePtr->OVal / 4);
	Reg =  XCLK_WIZ_REG3_PREDIV2 | XCLK_WIZ_REG3_USED | XCLK_WIZ_REG3_MX;
	if (InstancePtr->OVal % 4 <= 1) {
		DivEdge = 0;
	} else {
		DivEdge = 1;
	}
	Reg |= (DivEdge << 8);
	P5fEdge = InstancePtr->OVal % 2;
	P5Enable = InstancePtr->OVal % 2;
	Reg = Reg | P5Enable << XCLK_WIZ_CLKOUT0_P5EN_SHIFT | P5fEdge << XCLK_WIZ_CLKOUT0_P5FEDGE_SHIFT;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG3_OFFSET, Reg);
	Reg = HighTime | HighTime << 8;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG4_OFFSET, Reg);

	/* Implement D */
	HighTime = (InstancePtr->DVal / 2);
	Reg  = 0;
	Reg = Reg & ~(1 << XCLK_WIZ_REG12_EDGE_SHIFT);
	DivEdge = InstancePtr->DVal % 2;
	Reg = Reg | DivEdge << XCLK_WIZ_REG12_EDGE_SHIFT;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG12_OFFSET, Reg);
	Reg = HighTime | HighTime << 8;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG13_OFFSET, Reg);

	/* Implement M*/
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
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG11_OFFSET, 0x2e);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG14_OFFSET, 0xe80);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG15_OFFSET, 0x4271);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG16_OFFSET, 0x43e9);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG17_OFFSET, 0x001C);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG26_OFFSET, 0x0001);

	return XST_SUCCESS;
}
/****************************************************************************/
/**
* Change the frequency to the given rate.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
* @param	SetRate is the frequency for which is desired.
*
* @return
*		- XST_SUCCESS frequency setting was successful.
*		- XST_FAILURE frequency setting failed.
*
* @note		Should be called only if there is only one output clock.
*****************************************************************************/
u32 XClk_Wiz_SetRate(XClk_Wiz  *InstancePtr, u64 SetRate)
{
	u32 Platform;
	u32 HighTime;
	u32 DivEdge;
	u32 Reg;
	u32 P5Enable;
	u32 P5fEdge;
	u32 Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SetRate != 0);

	Platform = XGetPlatform_Info();

	if (InstancePtr->Config.NumClocks  != 1 )
		return Status;

	Status = XClk_Wiz_CalculateDivisors(InstancePtr, SetRate);
	if ( Status != XST_SUCCESS)
		return Status;

	if(Platform != (u32)XPLAT_VERSAL) {
		Reg = InstancePtr->MVal << 8 | InstancePtr->DVal;
		XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_ZYNQMP_REG0_OFFSET, Reg);
		Reg =  InstancePtr->OVal;
		XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_ZYNQMP_REG2_OFFSET, Reg);
		return XST_SUCCESS;
	}

	/* Implement O */
	HighTime = (InstancePtr->OVal / 4);
	Reg =  XCLK_WIZ_REG3_PREDIV2 | XCLK_WIZ_REG3_USED | XCLK_WIZ_REG3_MX;
	if (InstancePtr->OVal % 4 <= 1) {
		DivEdge = 0;
	} else {
		DivEdge = 1;
	}
	Reg |= (DivEdge << 8);
	P5fEdge = InstancePtr->OVal % 2;
	P5Enable = InstancePtr->OVal % 2;
	Reg = Reg | P5Enable << XCLK_WIZ_CLKOUT0_P5EN_SHIFT | P5fEdge << XCLK_WIZ_CLKOUT0_P5FEDGE_SHIFT;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG3_OFFSET, Reg);
	Reg = HighTime | HighTime << 8;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG4_OFFSET, Reg);

	/* Implement D */
	HighTime = (InstancePtr->DVal / 2);
	Reg  = 0;
	Reg = Reg & ~(1 << XCLK_WIZ_REG12_EDGE_SHIFT);
	DivEdge = InstancePtr->DVal % 2;
	Reg = Reg | DivEdge << XCLK_WIZ_REG12_EDGE_SHIFT;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG12_OFFSET, Reg);
	Reg = HighTime | HighTime << 8;
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, XCLK_WIZ_REG13_OFFSET, Reg);

	/* Implement M*/
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
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG11_OFFSET, 0x2e);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG14_OFFSET, 0xe80);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG15_OFFSET, 0x4271);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG16_OFFSET, 0x43e9);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG17_OFFSET, 0x001C);
	XClk_Wiz_WriteReg(InstancePtr->Config.BaseAddr, XCLK_WIZ_REG26_OFFSET, 0x0001);

	return XST_SUCCESS;
}
/****************************************************************************/
/**
* Enable Clock for the given ClockId.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
* @param	ClockId is the output clock.
*
* @return
*		- XST_SUCCESS Initialization was successful.
*		- XST_FAILURE Initialization was failure.
*
*****************************************************************************/
u32 XClk_Wiz_EnableClock(XClk_Wiz  *InstancePtr, u32 ClockId)
{
	u32 Platform;
	u32 RegisterOffset;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (ClockId > XCLK_WIZ_MAX_OUTPUT)
		return XST_FAILURE;

	Platform = XGetPlatform_Info();

	if(Platform != (u32)XPLAT_VERSAL)
		return XST_SUCCESS;

	if (ClockId < 3)
		RegisterOffset = XCLK_WIZ_REG3_OFFSET + ClockId * 8 ;
	else
		RegisterOffset = XCLK_WIZ_REG19_OFFSET + ClockId * 8 ;

	XCLK_WIZ_BIT_SET(InstancePtr->Config.BaseAddr, RegisterOffset, XCLK_WIZ_REG3_USED);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* Disable Clock for the given ClockId.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
* @param	ClockId is the output clock.
*
* @return
*		- XST_SUCCESS Initialization was successful.
*		- XST_FAILURE Initialization was failure.
*
*****************************************************************************/
u32 XClk_Wiz_DisableClock(XClk_Wiz  *InstancePtr, u32 ClockId)
{
	u32 Platform;
	u32 RegisterOffset;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Platform = XGetPlatform_Info();

	if(Platform != (u32)XPLAT_VERSAL)
		return XST_SUCCESS;

	if (ClockId > XCLK_WIZ_MAX_OUTPUT)
		return XST_FAILURE;

	if (ClockId < 3)
		RegisterOffset = XCLK_WIZ_REG3_OFFSET + ClockId * 8 ;
	else
		RegisterOffset = XCLK_WIZ_REG19_OFFSET + ClockId * 8 ;

	XCLK_WIZ_BIT_RESET(InstancePtr->Config.BaseAddr, RegisterOffset, XCLK_WIZ_REG3_USED);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
* Wait till the clocking wizard is locked to the frequency.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
*
* @return
*		- XST_SUCCESS if lock was successful.
*		- XST_FAILURE on timeout.
*
*****************************************************************************/
u32 XClk_Wiz_WaitForLock(XClk_Wiz  *InstancePtr)
{
	u32 Platform;
	u32 Count = 0;
	u32 RegisterOffset;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Platform = XGetPlatform_Info();

	if (Platform != (u32)XPLAT_VERSAL) {
		RegisterOffset = XCLK_WIZ_STATUS_OFFSET;
	} else {
		RegisterOffset = XCLK_WIZ_REG4_OFFSET;
	}

	while (!XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, RegisterOffset) & XCLK_WIZ_LOCK) {
		if (Count == 10000) {
			return XST_FAILURE;
		}
		usleep(100);
		Count++;
	}
	return XST_SUCCESS;

}
/****************************************************************************/
/**
* Change the Input frequency to the given rate.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
* @param	Rate is the frequency for which is to be set.
*
* @return 	None
*
* @note		Should be called only if the input provider clock is changed eg
* 		input clock is si570.
*****************************************************************************/
void XClk_Wiz_SetInputRate(XClk_Wiz  *InstancePtr, double Rate)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Rate != 0);
	InstancePtr->Config.PrimInClkFreq  = Rate;

}
/*****************************************************************************/
/**
* XClk_Wiz_GetInterruptSettings will get the information from  clock wizard
* IER  and ISR Registers.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
*
* @return 	None
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
* This routine is a stub for the asynchronous error interrupt callback. The
* stub is here in case the upper layer forgot to set the handler. On
* initialization, Error interrupt handler is set to this callback. It is
* considered an error for this handler to be invoked.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param 	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XCLK_WIZ_ISR_*_MASK values
*		defined in xclk_wiz_hw.h.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask)
{
	(void) ((void *)CallBackRef);
	(void) ErrorMask;
	Xil_AssertVoidAlways();
}
/** @} */
