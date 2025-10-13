/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_util.h"
#include "xpm_pll.h"
#include "xpm_regs.h"
#include "xpm_alloc.h"

static const struct XPm_PllTopology PllTopologies[] =
{
	{ TOPOLOGY_GENERIC_PLL, PLLPARAMS, RESET_SHIFT, BYPASS_SHIFT,
	  GEN_LOCK_SHIFT, GEN_STABLE_SHIFT, GEN_REG3_OFFSET },
	{ TOPOLOGY_NOC_PLL, PLLPARAMS, RESET_SHIFT, BYPASS_SHIFT,
	  NPLL_LOCK_SHIFT, NPLL_STABLE_SHIFT, NPLL_REG3_OFFSET },
};

XStatus XPmClockPll_AddNode(u32 Id, u32 ControlReg, u8 TopologyType,
			    const u16 *Offsets, u32 PowerDomainId, u8 ClkFlags)
{
	XStatus Status = XST_FAILURE;
	XPm_PllClockNode *PllClkPtr;

	if (NULL != XPmClock_GetById(Id)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}
	if ((TopologyType != TOPOLOGY_GENERIC_PLL) &&
	    (TopologyType != TOPOLOGY_NOC_PLL)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PllClkPtr = (XPm_PllClockNode *)XPm_AllocBytes(sizeof(XPm_PllClockNode));
	if (PllClkPtr == NULL) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}
	XPmNode_Init(&PllClkPtr->ClkNode.Node, Id, (u8)PM_PLL_STATE_SUSPENDED, 0);
	PllClkPtr->ClkNode.Node.BaseAddress = ControlReg;
	PllClkPtr->ClkNode.ClkHandles = NULL;
	PllClkPtr->ClkNode.UseCount = 0;
	PllClkPtr->ClkNode.NumParents = 1;
	PllClkPtr->ClkNode.Flags = ClkFlags;
	PllClkPtr->Topology = &PllTopologies[TopologyType-TOPOLOGY_GENERIC_PLL];
	PllClkPtr->StatusReg = ControlReg + Offsets[0];
	PllClkPtr->ConfigReg = ControlReg + Offsets[1];
	PllClkPtr->FracConfigReg = ControlReg + Offsets[2];

	Status = XPmClock_SetById(Id, (XPm_ClockNode *)PllClkPtr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (((u32)XPM_NODECLASS_POWER != NODECLASS(PowerDomainId)) ||
	    ((u32)XPM_NODESUBCL_POWER_DOMAIN != NODESUBCLASS(PowerDomainId))) {
		PllClkPtr->ClkNode.PwrDomain = NULL;
		goto done;
	}

	PllClkPtr->ClkNode.PwrDomain = XPmPower_GetById(PowerDomainId);
	if (NULL == PllClkPtr->ClkNode.PwrDomain) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

done:
	return Status;
}

XStatus XPmClockPll_AddParent(u32 Id, const u32 *Parents, u8 NumParents)
{
	XStatus Status = XST_FAILURE;
	XPm_PllClockNode *PllPtr = (XPm_PllClockNode *)XPmClock_GetById(Id);

	if (PllPtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if ((PllPtr->ClkNode.NumParents == 1U) && (NumParents != 1U)) {
		Status = XST_INVALID_PARAM;
		goto done;
	} else {
		PllPtr->ClkNode.ParentIdx = (u16)(NODEINDEX(Parents[0]));
		Status = XST_SUCCESS;
	}

done:
	return Status;
}
static void XPm_PllSaveContext(XPm_PllClockNode* Pll)
{
	/* Save register setting */
	Pll->Context.Ctrl = XPm_Read32(Pll->ClkNode.Node.BaseAddress);
	Pll->Context.Cfg = XPm_Read32(Pll->ConfigReg);
	Pll->Context.Frac = XPm_Read32(Pll->FracConfigReg);
	Pll->Context.Flag |= PM_PLL_CONTEXT_SAVED;
}

static void XPm_PllRestoreContext(XPm_PllClockNode* Pll)
{
	XPm_Write32(Pll->ClkNode.Node.BaseAddress, Pll->Context.Ctrl);
	XPm_Write32(Pll->ConfigReg, Pll->Context.Cfg);
	XPm_Write32(Pll->FracConfigReg, Pll->Context.Frac);
	Pll->Context.Flag &= (u8)(~PM_PLL_CONTEXT_SAVED);
}

XStatus XPmClockPll_Suspend(XPm_PllClockNode *Pll)
{
	XStatus Status = XST_FAILURE;

	XPm_PllSaveContext(Pll);

	/* If PLL is not already in reset, bypass it and put in reset/pwrdn */
	if (PM_PLL_STATE_RESET != Pll->ClkNode.Node.State) {
		Status = XPmClockPll_Reset(Pll, PLL_RESET_ASSERT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Pll->ClkNode.Node.State = PM_PLL_STATE_SUSPENDED;
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClockPll_Resume(XPm_PllClockNode *Pll)
{
	XStatus Status = XST_FAILURE;

	if (0U != (Pll->Context.Flag & PM_PLL_CONTEXT_SAVED)) {
		XPm_PllRestoreContext(Pll);
	}

	/* By saved configuration PLL is in reset, leave it as is */
	if (0U != (Pll->Context.Ctrl & BIT32(Pll->Topology->ResetShift))) {
		Pll->ClkNode.Node.State = PM_PLL_STATE_RESET;
		Status = XST_SUCCESS;
	} else {
		Status = XPmClockPll_Reset(Pll, PLL_RESET_RELEASE);
	}

	return Status;
}

XStatus XPmClockPll_Request(u32 PllId)
{
	XStatus Status = XST_FAILURE;

	XPm_PllClockNode *Pll = (XPm_PllClockNode *)XPmClock_GetById(PllId);
	if (Pll == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XPm_Power *PowerDomain = Pll->ClkNode.PwrDomain;
	if ((0U == Pll->ClkNode.UseCount) && (NULL != PowerDomain)) {
		Status = PowerDomain->HandleEvent(&PowerDomain->Node,
						  XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Pll->ClkNode.UseCount++;

	/* If the PLL is suspended it needs to be resumed first */
	if (Pll->ClkNode.Node.State == PM_PLL_STATE_SUSPENDED) {
		Status = XPmClockPll_Resume(Pll);
	}
	else if (Pll->ClkNode.Node.State == PM_PLL_STATE_RESET) {
		Status = XPmClockPll_Reset(Pll, PLL_RESET_RELEASE);
	} else {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

XStatus XPmClockPll_Release(u32 PllId)
{
	XStatus Status = XST_FAILURE;
	XPm_PllClockNode *Pll = (XPm_PllClockNode *)XPmClock_GetById(PllId);
	if (Pll == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	PmDecrement(Pll->ClkNode.UseCount);
	/**
	 * Do not suspend the PLL if its use count goes to 0 because it may
	 * possible that IOU_SWITCH of other domain is using this PLL.
	 * Just decrement its parent use count and PLL will be suspended
	 * when its power domain goes off.
	 */
	XPm_Power *PowerDomain = Pll->ClkNode.PwrDomain;
	if ((0U == Pll->ClkNode.UseCount) && (NULL != PowerDomain)) {
		Status = PowerDomain->HandleEvent(&PowerDomain->Node,
						  XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClockPll_Reset(XPm_PllClockNode *Pll, uint8_t Flags)
{
	XStatus Status = XST_FAILURE;
	u32 ControlReg = Pll->ClkNode.Node.BaseAddress;

	if (0U != (Flags & PLL_RESET_ASSERT)) {
		/* Bypass PLL before putting it into the reset */
		XPm_RMW32(ControlReg, BIT32(Pll->Topology->BypassShift),
			   BIT32(Pll->Topology->BypassShift));

		/* Power down PLL (= reset PLL) */
		XPm_RMW32(ControlReg, BIT32(Pll->Topology->ResetShift),
			   BIT32(Pll->Topology->ResetShift));
		Pll->ClkNode.Node.State = PM_PLL_STATE_RESET;

	}
	if (0U != (Flags & PLL_RESET_RELEASE)) {
		/* Deassert the reset */
		XPm_RMW32(ControlReg, BIT32(Pll->Topology->ResetShift),
			   ~BIT32(Pll->Topology->ResetShift));
		/* Poll status register for the lock */
		Status = XPm_PollForMask(Pll->StatusReg, BIT32(Pll->Topology->LockShift),
						  PLL_LOCK_TIMEOUT);
		/* Deassert bypass if the PLL locked */
		if (XST_SUCCESS == Status) {
			XPm_RMW32(ControlReg, BIT32(Pll->Topology->BypassShift),
					~BIT32(Pll->Topology->BypassShift));
			Pll->ClkNode.Node.State = PM_PLL_STATE_LOCKED;

		} else {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
