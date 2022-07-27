/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_core.h"
#include "xpm_psm.h"

XStatus SkipRpuReset(const XPm_Core *Core)
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
void EnableWake(const struct XPm_Core *Core)
{
	ENABLE_WAKE(Core->SleepMask);
}
