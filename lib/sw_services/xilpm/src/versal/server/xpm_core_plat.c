/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_core.h"
#include "xpm_notifier.h"
#include "xpm_psm.h"

static void EnableWake(const struct XPm_Core *Core)
{
	ENABLE_WAKE(Core->SleepMask);
}

static XStatus SkipRpuReset(const XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	const XPm_Device *DevTcm0A = XPmDevice_GetById(PM_DEV_TCM_0_A);
	const XPm_Device *DevTcm0B = XPmDevice_GetById(PM_DEV_TCM_0_B);
	const XPm_Device *DevTcm1A = XPmDevice_GetById(PM_DEV_TCM_1_A);
	const XPm_Device *DevTcm1B = XPmDevice_GetById(PM_DEV_TCM_1_B);
	if (!(((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Core->Device.Node.Id)) &&
	      (((u8)XPM_DEVSTATE_RUNNING == DevTcm0A->Node.State) ||
	       ((u8)XPM_DEVSTATE_RUNNING == DevTcm0B->Node.State) ||
	       ((u8)XPM_DEVSTATE_RUNNING == DevTcm1A->Node.State) ||
	       ((u8)XPM_DEVSTATE_RUNNING == DevTcm1B->Node.State)))) {
		Status = XPmDevice_Reset(&Core->Device, PM_RESET_ACTION_ASSERT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmCore_PwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	if ((u32)XPM_DEVSTATE_UNUSED == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State) {
		DISABLE_WFI(Core->SleepMask);
	}

	/* If parent is on, then only send sleep request */
	if ((Core->Device.Power->Parent->Node.State == (u8)XPM_POWER_STATE_ON) &&
	    ((u32)XPM_NODETYPE_DEV_CORE_RPU != NODETYPE(Core->Device.Node.Id))) {
		/* Power down the core */
		Status = XPm_DirectPwrDwn(Core->Device.Node.Id);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XPmCore_AfterDirectPwrDwn(Core);

done:
	return Status;
}

XStatus XPmCore_AfterDirectPwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;

	if (NULL != Core->Device.ClkHandles) {
		Status = XPmClock_Release(Core->Device.ClkHandles);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Skip reset for RPU cores if any of the TCM is ON */
	Status = SkipRpuReset(Core);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (NULL != Core->Device.Power) {
		PwrNode = Core->Device.Power;
		Status = PwrNode->HandleEvent(&PwrNode->Node, XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/**
	 * Enabling of wake interrupt is removed from PSM direct power down
	 * sequence so enable wakeup interrupt here.
	 */
	if ((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State) {
		EnableWake(Core);
	}

	Core->Device.Node.State = (u8)XPM_DEVSTATE_UNUSED;
	Core->isCoreUp = 0;
	Status = XST_SUCCESS;
	/* Send notification about core state change */
	XPmNotifier_Event(Core->Device.Node.Id, (u32)EVENT_STATE_CHANGE);

done:
	return Status;
}

XStatus ResetAPUGic(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Power *Acpu0PwrNode = XPmPower_GetById(PM_POWER_ACPU_0);
	const XPm_Power *Acpu1PwrNode = XPmPower_GetById(PM_POWER_ACPU_1);
	const XPm_Power *FpdPwrNode = XPmPower_GetById(PM_POWER_FPD);

	if (((PM_DEV_ACPU_0 == DeviceId) || (PM_DEV_ACPU_1 == DeviceId)) &&
	    (NULL != Acpu0PwrNode) && (NULL != Acpu1PwrNode) &&
	    (NULL != FpdPwrNode) &&
	    ((u8)XPM_POWER_STATE_OFF != FpdPwrNode->Node.State) &&
	    ((u8)XPM_POWER_STATE_OFF == Acpu0PwrNode->Node.State) &&
	    ((u8)XPM_POWER_STATE_OFF == Acpu1PwrNode->Node.State)) {
		Status = XPmReset_AssertbyId(PM_RST_ACPU_GIC,
					     (u32)PM_RESET_ACTION_PULSE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
void DisableWake(const struct XPm_Core *Core)
{
	DISABLE_WAKE(Core->SleepMask);
}
