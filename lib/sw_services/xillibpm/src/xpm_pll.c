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

#include "xpm_pll.h"

static struct XPm_PllTopology PllTopologies[] =
{
	{TOPOLOGY_GENERIC_PLL, PLLPARAMS, RESET_SHIFT, BYPASS_SHIFT, GEN_LOCK_SHIFT, GEN_STABLE_SHIFT },
	{TOPOLOGY_NOC_PLL, PLLPARAMS, RESET_SHIFT, BYPASS_SHIFT, NPLL_LOCK_SHIFT, NPLL_STABLE_SHIFT },
};

XStatus XPmClockPll_AddNode(u32 Id, u32 ControlReg, u8 TopologyType, u32 PowerDomainId, u16 *Offsets)
{
	int Status = XST_SUCCESS;
	u32 ClockIndex = NODEINDEX(Id);
	XPm_PllClockNode *PllClkPtr;

	if (ClkNodeList[ClockIndex] != NULL || ClockIndex > MaxClkNodes) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (TopologyType!=TOPOLOGY_GENERIC_PLL && TopologyType!=TOPOLOGY_NOC_PLL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PllClkPtr = XPm_AllocBytes(sizeof(XPm_PllClockNode));
	if (PllClkPtr == NULL) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}
	XPmNode_Init(&PllClkPtr->ClkNode.Node, Id, (u32)XPM_CLK_STATE_OFF, 0);
	PllClkPtr->ClkNode.Node.BaseAddress = ControlReg;
	PllClkPtr->ClkNode.ClkHandles = NULL;
	PllClkPtr->ClkNode.UseCount = 0;
	PllClkPtr->ClkNode.NumParents = 1;
	PllClkPtr->Topology = &PllTopologies[TopologyType-TOPOLOGY_GENERIC_PLL];
	PllClkPtr->StatusReg = ControlReg + Offsets[0];
	PllClkPtr->ConfigReg = ControlReg + Offsets[1];
	PllClkPtr->FracConfigReg = ControlReg + Offsets[2];

	ClkNodeList[ClockIndex] = (XPm_ClockNode *)PllClkPtr;

done:
	return Status;
}

XStatus XPmClockPll_SetMode(XPm_PllClockNode *Pll, u32 Mode)
{
	u32 Status = XST_SUCCESS;
	u32 Val;

	if (PM_PLL_MODE_FRACTIONAL == Mode) {
		/* Check if fractional value has been set */
		XPmClockPll_GetParam(Pll, PLL_PARAM_ID_FRAC_DATA, &Val);
		if (0U == Val) {
			Status = XST_FAILURE;
			goto done;
		}
	}

	Status = XPmClockPll_Reset(Pll, PLL_RESET_ASSERT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (PM_PLL_MODE_RESET == Mode) {
		Status = XST_SUCCESS;
		goto done;
	} else if (PM_PLL_MODE_FRACTIONAL == Mode) {
		/* Enable fractional mode */
		XPm_RMW32(Pll->ConfigReg, PLL_FRAC_CFG_ENABLED_MASK,
			  PLL_FRAC_CFG_ENABLED_MASK);
	} else if (PM_PLL_MODE_INTEGER == Mode) {
		/* Disable fractional mode */
		XPm_RMW32(Pll->ConfigReg, PLL_FRAC_CFG_ENABLED_MASK, 0);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClockPll_Reset(Pll, PLL_RESET_RELEASE);

done:
	if (XST_SUCCESS == Status) {
		Pll->PllMode = Mode;
	}
	return Status;
}

XStatus XPmClockPll_GetMode(XPm_PllClockNode *Pll, u32 *Mode)
{
	u32 Val;

	Val = XPm_Read32(Pll->ClkNode.Node.BaseAddress);
	if (0 != (Val & BIT(Pll->Topology->ResetShift))) {
		*Mode = PM_PLL_MODE_RESET;
	} else {
		Val = XPm_Read32(Pll->ConfigReg);
		if (0 != (Val & PLL_FRAC_CFG_ENABLED_MASK)) {
			*Mode = PM_PLL_MODE_FRACTIONAL;
		} else {
			*Mode = PM_PLL_MODE_INTEGER;
		}
	}

	Pll->PllMode = *Mode;
	return XST_SUCCESS;
}

XStatus XPmClockPll_Suspend(XPm_PllClockNode *Pll)
{
	u32 Status = XST_SUCCESS;
	/* TBD */
	Pll->ClkNode.Node.State = PM_PLL_STATE_SUSPENDED;
	return Status;
}

XStatus XPmClockPll_Resume(XPm_PllClockNode *Pll)
{
	u32 Status = XST_SUCCESS;
	/* TBD */
	Pll->ClkNode.Node.State = PM_PLL_STATE_LOCKED;
	return Status;
}

XStatus XPmClockPll_Request(u32 PllId)
{
	u32 Status = XST_SUCCESS;

	XPm_PllClockNode *Pll = (XPm_PllClockNode *)XPmClock_GetById(PllId);
	if (Pll == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Pll->ClkNode.UseCount++;

	/* If the PLL is suspended it needs to be resumed first */
	if (Pll->ClkNode.Node.State == PM_PLL_STATE_SUSPENDED) {
		Status = XPmClockPll_Resume(Pll);
	}
	else if (Pll->ClkNode.Node.State == PM_PLL_STATE_RESET) {
		Status = XPmClockPll_Reset(Pll, PLL_RESET_RELEASE);
	}
done:
	return Status;
}

XStatus XPmClockPll_Release(u32 PllId)
{
	u32 Status = XST_SUCCESS;
	XPm_PllClockNode *Pll = (XPm_PllClockNode *)XPmClock_GetById(PllId);
	if (Pll == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Pll->ClkNode.UseCount--;

	if (Pll->ClkNode.UseCount == 0) {
		Status = XPmClockPll_Suspend(Pll);
	}
done:
	return Status;
}


XStatus XPmClockPll_Reset(XPm_PllClockNode *Pll, uint8_t Flags)
{
	u32 Status = XST_SUCCESS;
	u32 ControlReg = Pll->ClkNode.Node.BaseAddress;

	if (Flags & PLL_RESET_ASSERT) {
		/* Bypass PLL before putting it into the reset */
		XPm_RMW32(ControlReg, BIT(Pll->Topology->BypassShift),
			   BIT(Pll->Topology->BypassShift));

		/* Power down PLL (= reset PLL) */
		XPm_RMW32(ControlReg, BIT(Pll->Topology->ResetShift),
			   BIT(Pll->Topology->ResetShift));
		Pll->ClkNode.Node.State = PM_PLL_STATE_RESET;
	}
	if (Flags & PLL_RESET_RELEASE) {
		/* Deassert the reset */
		XPm_RMW32(ControlReg, BIT(Pll->Topology->ResetShift),
			   ~BIT(Pll->Topology->ResetShift));
		/* Poll status register for the lock */
		Status = XPfw_PollForMask(Pll->StatusReg, BIT(Pll->Topology->LockShift),
						  PLL_LOCK_TIMEOUT);
		/* Deassert bypass if the PLL locked */
		if (XST_SUCCESS == Status) {
			XPm_RMW32(ControlReg, BIT(Pll->Topology->BypassShift),
					~BIT(Pll->Topology->BypassShift));
			Pll->ClkNode.Node.State = PM_PLL_STATE_LOCKED;
		}
	}

	return Status;
}

XStatus XPmClockPll_SetParam(XPm_PllClockNode *Pll, u32 Param, u32 Value)
{
	u32 Status = XST_SUCCESS;
	XPm_PllParam *PtrParam;
	u32 Mask, ParamValue, Reg;

	if (Param >= PLL_PARAM_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PtrParam = &Pll->Topology->ConfigParams[Param];
	if (Value > BITMASK(PtrParam->Width)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Allow config change only if PLL is in reset mode */
	if (Pll->ClkNode.Node.State != PM_PLL_STATE_RESET) {
		Status = XST_FAILURE;
		goto done;
	}

	Mask = BITNMASK(PtrParam->Shift,PtrParam->Width);
	ParamValue = Value << PtrParam->Shift;

	switch (Param) {
	case PLL_PARAM_ID_CLKOUTDIV:
	case PLL_PARAM_ID_FBDIV:
		Reg = Pll->ClkNode.Node.BaseAddress;
                break;
	case PLL_PARAM_ID_FRAC_DATA:
		Reg = Pll->FracConfigReg;
                break;
	case PLL_PARAM_ID_PRE_SRC:
	case PLL_PARAM_ID_POST_SRC:
	case PLL_PARAM_ID_LOCK_DLY:
	case PLL_PARAM_ID_LOCK_CNT:
	case PLL_PARAM_ID_LFHF:
	case PLL_PARAM_ID_CP:
	case PLL_PARAM_ID_RES:
		Reg = Pll->ConfigReg;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	if (XST_SUCCESS == Status) {
		XPm_RMW32(Reg, Mask, ParamValue);
	}

done:
	return Status;
}

XStatus XPmClockPll_GetParam(XPm_PllClockNode *Pll, u32 Param, u32 *Val)
{
	u32 Status = XST_SUCCESS;
	XPm_PllParam *PtrParam;
	u32 Shift, Mask, Reg;

	if (Param >= PLL_PARAM_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PtrParam = &Pll->Topology->ConfigParams[Param];
	Mask = BITNMASK(PtrParam->Shift, PtrParam->Width);
	Shift = PtrParam->Shift;

	switch (Param) {
	case PLL_PARAM_ID_CLKOUTDIV:
	case PLL_PARAM_ID_FBDIV:
		Reg = Pll->ClkNode.Node.BaseAddress;
                break;
	case PLL_PARAM_ID_FRAC_DATA:
		Reg = Pll->FracConfigReg;
                break;
	case PLL_PARAM_ID_PRE_SRC:
	case PLL_PARAM_ID_POST_SRC:
	case PLL_PARAM_ID_LOCK_DLY:
	case PLL_PARAM_ID_LOCK_CNT:
	case PLL_PARAM_ID_LFHF:
	case PLL_PARAM_ID_CP:
	case PLL_PARAM_ID_RES:
		Reg = Pll->ConfigReg;
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

