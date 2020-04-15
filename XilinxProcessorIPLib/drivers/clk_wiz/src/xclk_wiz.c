/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xclk_wiz.c
* @addtogroup clk_wiz_v1_3
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

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(SetRate != 0);

	for (m = XCLK_M_MIN; m <= XCLK_M_MAX; m++) {
		for (d = XCLK_D_MIN; d <= XCLK_D_MAX; d++) {
			Fvco = InstancePtr->Config.RefClkFreq  * m / d;
			if ( Fvco >= XCLK_VCO_MIN && Fvco <= XCLK_VCO_MAX ) {

				for (Div = XCLK_O_MIN; Div <= XCLK_O_MAX; Div++ ) {
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
						Status =  XST_SUCCESS;
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

/**
* Change the frequency to the given rate.
*
* @param	InstancePtr is the XClk_Wiz instance to operate on.
* @param	SetRate is the frequency for which is desired.
*
* @return
*		- XST_SUCCESS Initialization was successful.
*		- XST_FAILURE Initialization was failure.
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
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(SetRate != 0);

	Platform = XGetPlatform_Info();

	if(Platform != (u32)XPLAT_VERSAL)
		return XST_SUCCESS;

	Status = XClk_Wiz_CalculateDivisors(InstancePtr, SetRate);
	if ( Status != XST_SUCCESS)
		return Status;
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
