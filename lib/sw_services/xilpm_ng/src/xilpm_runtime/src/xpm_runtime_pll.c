/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_util.h"
#include "xpm_pll.h"
#include "xpm_regs.h"
#include "xpm_runtime_pll.h"

#define CLK_PARENTS_PAYLOAD_LEN		12U

/* Period of time needed to lock the PLL (TODO: measure actual latency) */
#define PM_PLL_LOCKING_TIME		1U


XStatus XPmClockPll_SetMode(XPm_PllClockNode *Pll, u32 Mode)
{
	XStatus Status = XST_FAILURE;

	Status = XPmClockPll_Reset(Pll, PLL_RESET_ASSERT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if ((u32)PM_PLL_MODE_RESET == Mode) {
		Status = XST_SUCCESS;
		goto done;
	} else if ((u32)PM_PLL_MODE_FRACTIONAL == Mode) {
		/* Enable fractional mode */
		XPm_RMW32(Pll->FracConfigReg, PLL_FRAC_CFG_ENABLED_MASK,
			  PLL_FRAC_CFG_ENABLED_MASK);
	} else if ((u32)PM_PLL_MODE_INTEGER == Mode) {
		/* Disable fractional mode */
		XPm_RMW32(Pll->FracConfigReg, PLL_FRAC_CFG_ENABLED_MASK, 0);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClockPll_Reset(Pll, PLL_RESET_RELEASE);

done:
	if (XST_SUCCESS == Status) {
		Pll->PllMode = (u8)Mode;
	}
	return Status;
}

XStatus XPmClockPll_GetMode(XPm_PllClockNode *Pll, u32 *Mode)
{
	u32 Val;
	XStatus Status = XST_FAILURE;
	const XPm_Power *PowerDomain = Pll->ClkNode.PwrDomain;

	if ((u8)XPM_POWER_STATE_ON != PowerDomain->Node.State) {
		Status = XST_NO_ACCESS;
		goto done;
	}

	Val = XPm_Read32(Pll->ClkNode.Node.BaseAddress);
	if (0U != (Val & BIT32(Pll->Topology->ResetShift))) {
		*Mode = (u32)PM_PLL_MODE_RESET;
	} else {
		Val = XPm_Read32(Pll->FracConfigReg);
		if (0U != (Val & PLL_FRAC_CFG_ENABLED_MASK)) {
			*Mode = (u32)PM_PLL_MODE_FRACTIONAL;
		} else {
			*Mode = (u32)PM_PLL_MODE_INTEGER;
		}
	}

	Pll->PllMode = (u8)(*Mode);

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClockPll_SetParam(XPm_PllClockNode *Pll, u32 Param, u32 Value)
{
	XStatus Status = XST_FAILURE;
	const XPm_PllParam *PtrParam;
	u32 Mask, ParamValue, Reg = 0;

	if (Param >= (u32)PM_PLL_PARAM_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PtrParam = &Pll->Topology->ConfigParams[Param];
	if (Value > BITMASK(PtrParam->Width)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClockPll_Reset(Pll, PLL_RESET_ASSERT);

	/* Allow config change only if PLL is in reset mode */
	if (Pll->ClkNode.Node.State != PM_PLL_STATE_RESET) {
		/*
		 * TODO - revisit it to allow subsystem CDO
		 * re-parsing which re-sets PLL parameters.
		 */
		PmInfo("Warning: Setting PLL parameter while not in reset state.\r\n");
		Status = XST_SUCCESS;
		goto done;
	}

	Mask = BITNMASK(PtrParam->Shift,PtrParam->Width);
	ParamValue = Value << PtrParam->Shift;

	switch (Param) {
	case (u32)PM_PLL_PARAM_ID_DIV2:
	case (u32)PM_PLL_PARAM_ID_FBDIV:
		Reg = Pll->ClkNode.Node.BaseAddress;
		Status = XST_SUCCESS;
		break;
	case (u32)PM_PLL_PARAM_ID_DATA:
		Reg = Pll->FracConfigReg;
		Status = XST_SUCCESS;
		break;
	case (u32)PM_PLL_PARAM_ID_PRE_SRC:
	case (u32)PM_PLL_PARAM_ID_POST_SRC:
	case (u32)PM_PLL_PARAM_ID_LOCK_DLY:
	case (u32)PM_PLL_PARAM_ID_LOCK_CNT:
	case (u32)PM_PLL_PARAM_ID_LFHF:
	case (u32)PM_PLL_PARAM_ID_CP:
	case (u32)PM_PLL_PARAM_ID_RES:
		Reg = Pll->ConfigReg;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	if (XST_SUCCESS == Status) {
		XPm_RMW32(Reg, Mask, ParamValue);
	}

	Status = XPmClockPll_Reset(Pll, PLL_RESET_RELEASE);

done:
	return Status;
}

XStatus XPmClockPll_GetParam(const XPm_PllClockNode *Pll, u32 Param, u32 *Val)
{
	XStatus Status = XST_FAILURE;
	const XPm_PllParam *PtrParam;
	u32 Shift, Mask, Reg = 0;
	const XPm_Power *PowerDomain = Pll->ClkNode.PwrDomain;

	if ((u32)XPM_POWER_STATE_ON != PowerDomain->Node.State) {
		Status = XST_NO_ACCESS;
		goto done;
	}

	if (Param >= (u32)PM_PLL_PARAM_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PtrParam = &Pll->Topology->ConfigParams[Param];
	Mask = BITNMASK(PtrParam->Shift, PtrParam->Width);
	Shift = PtrParam->Shift;

	switch (Param) {
	case (u32)PM_PLL_PARAM_ID_DIV2:
	case (u32)PM_PLL_PARAM_ID_FBDIV:
		Reg = Pll->ClkNode.Node.BaseAddress;
		Status = XST_SUCCESS;
		break;
	case (u32)PM_PLL_PARAM_ID_DATA:
		Reg = Pll->FracConfigReg;
		Status = XST_SUCCESS;
		break;
	case (u32)PM_PLL_PARAM_ID_PRE_SRC:
	case (u32)PM_PLL_PARAM_ID_POST_SRC:
	case (u32)PM_PLL_PARAM_ID_LOCK_DLY:
	case (u32)PM_PLL_PARAM_ID_LOCK_CNT:
	case (u32)PM_PLL_PARAM_ID_LFHF:
	case (u32)PM_PLL_PARAM_ID_CP:
	case (u32)PM_PLL_PARAM_ID_RES:
		Reg = Pll->ConfigReg;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	if (XST_SUCCESS == Status) {
		*Val = (XPm_Read32(Reg) & Mask) >> Shift;
	}

done:
	return Status;
}

XStatus XPmClockPll_QueryMuxSources(u32 Id, u32 Index, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	const XPm_PllClockNode *PllPtr = (XPm_PllClockNode *)XPmClock_GetById(Id);

	if (PllPtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = Xil_SMemSet(Resp, CLK_PARENTS_PAYLOAD_LEN, 0, CLK_PARENTS_PAYLOAD_LEN);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (Index != 0U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Resp[0] = PllPtr->ClkNode.ParentIdx;
	Resp[1] = 0xFFFFFFFFU;

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClockPll_GetWakeupLatency(const u32 Id, u32 *Latency)
{
	XStatus Status = XST_SUCCESS;
	const XPm_PllClockNode *Pll = (XPm_PllClockNode *)XPmClock_GetById(Id);
	u32 Lat = 0;

	*Latency = 0;
	if (NULL == Pll) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (PM_PLL_STATE_LOCKED == Pll->ClkNode.Node.State) {
		goto done;
	}

	*Latency += PM_PLL_LOCKING_TIME;

	Status = XPmPower_GetWakeupLatency(Pll->ClkNode.PwrDomain->Node.Id,
					   &Lat);
	if (XST_SUCCESS == Status) {
		*Latency += Lat;
	}

done:
	return Status;
}
