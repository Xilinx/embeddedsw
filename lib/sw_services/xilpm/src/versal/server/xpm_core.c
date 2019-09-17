/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_psm_api.h"
#include "xpm_core.h"
#include "xpm_psm.h"

XStatus XPmCore_Init(XPm_Core *Core, u32 Id, XPm_Power *Power,
		     XPm_ClockNode *Clock, XPm_ResetNode *Reset, u8 IpiCh,
		     struct XPm_CoreOps *Ops)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(&Core->Device, Id, 0, Power, Clock, Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Core->DebugMode = 0;
	Core->ImageId = 0;
	Core->Ipi = IpiCh;
	Core->CoreOps = Ops;
	Core->PwrUpLatency = 0;
	Core->PwrDwnLatency = 0;

done:
	return Status;
}

static XStatus XPmCore_Sleep(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	XPm_Device *Device;
	XPm_Device *DevTcm0A = XPmDevice_GetById(PM_DEV_TCM_0_A);
	XPm_Device *DevTcm0B = XPmDevice_GetById(PM_DEV_TCM_0_B);
	XPm_Device *DevTcm1A = XPmDevice_GetById(PM_DEV_TCM_1_A);
	XPm_Device *DevTcm1B = XPmDevice_GetById(PM_DEV_TCM_1_B);

	/*
	 * If parent is on, then only send sleep request
	 */
	if ((Core->Device.Power->Parent->Node.State == (u8)XPM_POWER_STATE_ON) &&
	    ((u32)XPM_NODETYPE_DEV_CORE_RPU != NODETYPE(Core->Device.Node.Id))) {
		/*
		 * Power down the core
		 */
		Status = XPm_DirectPwrDwn(Core->Device.Node.Id);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (NULL != Core->Device.ClkHandles) {
		Status = XPmClock_Release(Core->Device.ClkHandles);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Skip reset for RPU cores if any of the TCM is ON */
	if (!(((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Core->Device.Node.Id)) &&
	      (((u8)XPM_DEVSTATE_RUNNING == DevTcm0A->Node.State) ||
	       ((u8)XPM_DEVSTATE_RUNNING == DevTcm0B->Node.State) ||
	       ((u8)XPM_DEVSTATE_RUNNING == DevTcm1A->Node.State) ||
	       ((u8)XPM_DEVSTATE_RUNNING == DevTcm1B->Node.State)))) {
		Device = &Core->Device;
		Status = XPmDevice_Reset(Device, PM_RESET_ACTION_ASSERT);
	} else {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

XStatus XPmCore_WakeUp(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;

	DISABLE_WAKE(Core->SleepMask);

	if (((u32)XPM_DEVSTATE_RUNNING != Core->Device.Node.State) &&
	    (NULL != Core->Device.Power)) {
		PwrNode = Core->Device.Power;
		Status = PwrNode->HandleEvent(&PwrNode->Node, XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (NULL != Core->CoreOps && NULL != Core->CoreOps->RestoreResumeAddr) {
		Status = Core->CoreOps->RestoreResumeAddr(Core);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if ((u32)XPM_DEVSTATE_RUNNING != Core->Device.Node.State) {
		if (NULL != Core->Device.ClkHandles) {
			Status = XPmClock_Request(Core->Device.ClkHandles);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		Status = XPm_DirectPwrUp(Core->Device.Node.Id);
	}

done:
	return Status;
}

XStatus XPmCore_PwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;

	if ((u32)XPM_DEVSTATE_UNUSED == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State) {
		DISABLE_WFI(Core->SleepMask);
	}

	Status = XPmCore_Sleep(Core);
	if(Status != XST_SUCCESS) {
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
	 * Since RPU direct power down is skipped in case of power-down,
	 * wakeup interrupt needs to be enabled here.
	 */
	if (((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Core->Device.Node.Id)) &&
	    ((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State)) {
		ENABLE_WAKE(Core->SleepMask);
	}

	Core->Device.Node.State = (u8)XPM_DEVSTATE_UNUSED;

done:
	return Status;
}

int XPmCore_GetWakeupLatency(const u32 DeviceId, u32 *Latency)
{
	int Status = XST_SUCCESS;
	XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
	XPm_Power *Power;
	u32 Lat = 0;

	*Latency = 0;

	if (NULL == Core) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u32)XPM_DEVSTATE_RUNNING == Core->Device.Node.State) {
		goto done;
	}

	*Latency += Core->PwrUpLatency;
	if ((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State) {
		*Latency += Core->PwrDwnLatency;
		goto done;
	}

	Power = Core->Device.Power;
	if (NULL != Power) {
		Status = XPmPower_GetWakeupLatency(Power->Node.Id, &Lat);
		if (XST_SUCCESS == Status) {
			*Latency += Lat;
		}
	}

done:
	return Status;
}
