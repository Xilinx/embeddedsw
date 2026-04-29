
/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3cpsx_options.c
* @{
*
* Contains functions for the configuration of the XIccPs driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------
* 1.00  sd  06/10/22 First release
* 1.01  sd  12/01/22 REmove the hardcoding of timing values
*	    12/14/22 Fix the warnings
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xi3cpsx.h"
#include "xi3cpsx_pr.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* Sets I3C Scl clock frequency.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if any computed timing
*		count falls outside [SCL_I3C_TIMING_CNT_MIN, SCL_I3C_TIMING_CNT_MAX],
*		indicating the input clock and SCL rate combination is not
*		representable in the hardware timing registers.
*
* @note		The SCL frequency is read from InstancePtr->SclkHz.
*		To configure a custom SCL frequency, use XI3cPsx_SetSClkRate.
*
****************************************************************************/
s32 XI3cPsx_SetSClk(XI3cPsx *InstancePtr)
{
	u32 CoreRate;
	u32 CorePeriod;
	u32 SclTiming;
	UINTPTR BaseAddress;
	u32 Hcnt;
	u32 PpLcnt;
	u32 OdLcnt;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Config.InputClockHz > 0U);
	Xil_AssertNonvoid(InstancePtr->SclkHz > 0U);

	CoreRate = InstancePtr->Config.InputClockHz;
	BaseAddress = InstancePtr->Config.BaseAddress;

	CorePeriod = XI3CPSX_CEIL_DIV(XI3CPSX_NS_1SEC, CoreRate);

	/*
	 * Validate all timing counts before touching any hardware register.
	 * This ensures hardware is never left partially reprogrammed on failure.
	 *
	 * Hcnt:   SCL_I3C_PP_TIMING.HCNT register is N+1 semantics, so subtract 1.
	 *         Derived from THIGH_MAX to keep PP high time within I3C spec.
	 * PpLcnt: remainder of the full SCL period at the requested SclkHz.
	 * OdLcnt: low count for OD mode derived from TLOW_OD_MIN.
	 */
	Hcnt = XI3CPSX_CEIL_DIV(I3C_BUS_THIGH_MAX_NS, CorePeriod) - 1U;
	if (Hcnt < SCL_I3C_TIMING_CNT_MIN || Hcnt > SCL_I3C_TIMING_CNT_MAX) {
		return XST_FAILURE;
	}

	PpLcnt = XI3CPSX_CEIL_DIV(CoreRate, InstancePtr->SclkHz);
	PpLcnt = (PpLcnt > Hcnt) ? (PpLcnt - Hcnt) : 0U;
	if (PpLcnt < SCL_I3C_TIMING_CNT_MIN || PpLcnt > SCL_I3C_TIMING_CNT_MAX) {
		return XST_FAILURE;
	}

	OdLcnt = XI3CPSX_CEIL_DIV(I3C_BUS_TLOW_OD_MIN_NS, CorePeriod);
	if (OdLcnt < SCL_I3C_TIMING_CNT_MIN || OdLcnt > SCL_I3C_TIMING_CNT_MAX) {
		return XST_FAILURE;
	}

	/*
	 * All counts validated — safe to program hardware registers.
	 */
	SclTiming = SCL_I3C_TIMING_HCNT(Hcnt) |
		    (PpLcnt & XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_LCNT_MASK);
	XI3cPsx_WriteReg(BaseAddress, XI3CPSX_SCL_I3C_PP_TIMING, SclTiming);

	SclTiming = SCL_I3C_TIMING_HCNT(Hcnt) | SCL_I3C_TIMING_LCNT(OdLcnt);
	XI3cPsx_WriteReg(BaseAddress, XI3CPSX_SCL_I3C_OD_TIMING, SclTiming);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
 * Sets a custom I3C SCL clock frequency by updating the instance SCL rate
 * and configuring the hardware registers accordingly.
 *
 * @param	InstancePtr is a pointer to the XI3cPsx instance.
 * @param	SclkHz is the desired SCL clock frequency in Hz.
 *
 * @return	XST_SUCCESS if successful, XST_FAILURE if the requested SCL
 *		rate cannot be represented in the hardware timing registers
 *		for the given input clock.
 *
 * @note	This updates InstancePtr->SclkHz and programs the hardware.
 *		For default SCL rate set during initialization, use
 *		XI3cPsx_SetSClk instead.
 *
 ****************************************************************************/
s32 XI3cPsx_SetSClkRate(XI3cPsx *InstancePtr, u32 SclkHz)
{
	s32 Status;
	u32 PrevSclkHz;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(SclkHz > 0U);

	PrevSclkHz = InstancePtr->SclkHz;
	InstancePtr->SclkHz = SclkHz;

	Status = XI3cPsx_SetSClk(InstancePtr);
	if (Status != XST_SUCCESS) {
		InstancePtr->SclkHz = PrevSclkHz;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* Returns the SCL clock frequency currently configured in the instance.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return	The SCL clock frequency in Hz as last successfully programmed
*		by XI3cPsx_SetSClk() or XI3cPsx_SetSClkRate().
*
* @note		This returns the software-stored rate, which is always kept
*		in sync with the hardware. If XI3cPsx_SetSClkRate() fails,
*		the previous valid rate is preserved and returned here.
*
****************************************************************************/
u32 XI3cPsx_GetSClk(const XI3cPsx *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	return InstancePtr->SclkHz;
}
/** @} */
