/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_psm_api.h"
#include "xpm_core.h"
#include "xpm_psm.h"
#include "xpm_debug.h"

XStatus XPmCore_Init(XPm_Core *Core, u32 Id, XPm_Power *Power,
		     XPm_ClockNode *Clock, XPm_ResetNode *Reset, u8 IpiCh,
		     struct XPm_CoreOps *Ops)
{
	XStatus Status = XST_FAILURE;
	u32 Idx;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPmDevice_Init(&Core->Device, Id, 0, Power, Clock, Reset);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_DEVICE_INIT;
		goto done;
	}

	Core->DebugMode = 0;
	Core->ImageId = 0;
	Core->Ipi = IpiCh;
	Core->CoreOps = Ops;
	Core->PwrUpLatency = 0;
	Core->PwrDwnLatency = 0;
	Core->isCoreUp = 0;
	Core->PsmToPlmEvent_ProcIdx = (u8)PROC_DEV_MAX;

	if (((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(Id)) ||
	    ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Id))) {
		/* Find and store PsmToPlmEvent_ProcIdx in Core structure */
		for (Idx = 0U; Idx < ARRAY_SIZE(ProcDevList); Idx++) {
			if (ProcDevList[Idx] == Id) {
				Core->PsmToPlmEvent_ProcIdx = (u8)Idx;
				break;
			}
		}
		if (Idx >= ARRAY_SIZE(ProcDevList)) {
			DbgErr = XPM_INT_ERR_INVALID_PROC;
			Status = XST_FAILURE;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

int XPmCore_StoreResumeAddr(XPm_Core *Core, u64 Address)
{
	int Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Check for valid resume address */
	if (0U == (Address & 1ULL)) {
		DbgErr = XPM_INT_ERR_INVALID_RESUME_ADDR;
		goto done;
	}

	if ((NULL == Core) || ((u8)PROC_DEV_MAX == Core->PsmToPlmEvent_ProcIdx)) {
		DbgErr = XPM_INT_ERR_INVALID_PROC;
		goto done;
	}

	/* Store the resume address to PSM reserved RAM location */
	PsmToPlmEvent->ResumeAddress[Core->PsmToPlmEvent_ProcIdx] = Address;
	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

int XPmCore_HasResumeAddr(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	u64 ResumeAddr;

	if ((NULL == Core) || ((u8)PROC_DEV_MAX == Core->PsmToPlmEvent_ProcIdx)) {
		goto done;
	}

	ResumeAddr = PsmToPlmEvent->ResumeAddress[Core->PsmToPlmEvent_ProcIdx];
	if (0U != (ResumeAddr & 1ULL)) {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

int XPmCore_SetCPUIdleFlag(XPm_Core *Core, u32 CpuIdleFlag)
{
	int Status = XST_FAILURE;

	if ((NULL == Core) || ((u8)PROC_DEV_MAX == Core->PsmToPlmEvent_ProcIdx)) {
		goto done;
	}

	/* Store the CPU idle flag to PSM reserved RAM location */
	PsmToPlmEvent->CpuIdleFlag[Core->PsmToPlmEvent_ProcIdx] = CpuIdleFlag;
	Status = XST_SUCCESS;

done:
	return Status;
}

int XPmCore_GetCPUIdleFlag(XPm_Core *Core, u32 *CpuIdleFlag)
{
	int Status = XST_FAILURE;

	if ((NULL == Core) || ((u8)PROC_DEV_MAX == Core->PsmToPlmEvent_ProcIdx)) {
		goto done;
	}

	/* Get the CPU idle flag from PSM reserved RAM location */
	*CpuIdleFlag = PsmToPlmEvent->CpuIdleFlag[Core->PsmToPlmEvent_ProcIdx];
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;

	if (1U == Core->isCoreUp) {
		Status = XPM_ERR_WAKEUP;
		goto done;
	}

	DISABLE_WAKE(Core->SleepMask);
	if (((u32)XPM_DEVSTATE_RUNNING != Core->Device.Node.State) &&
	    (NULL != Core->Device.Power)) {
		PwrNode = Core->Device.Power;
		Status = PwrNode->HandleEvent(&PwrNode->Node, XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Set reset address */
	if (1U == SetAddress) {
		Status = XPmCore_StoreResumeAddr(Core, (Address | 1U));
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (((u32)XPM_DEVSTATE_RUNNING != Core->Device.Node.State)
	    && (NULL != Core->Device.ClkHandles)) {
		Status = XPmClock_Request(Core->Device.ClkHandles);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Reset will be released at a part of direct power up sequence */
	Status = XPm_DirectPwrUp(Core->Device.Node.Id);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Core->Device.Node.State = (u8)XPM_DEVSTATE_RUNNING;
	Core->isCoreUp = 1;

done:
	return Status;
}

int XPmCore_AfterDirectWakeUp(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;

	if ((u32)XPM_DEVSTATE_RUNNING == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (NULL != Core->Device.Power) {
		PwrNode = Core->Device.Power;
		Status = PwrNode->HandleEvent(&PwrNode->Node, XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (NULL != Core->Device.ClkHandles) {
		Status = XPmClock_Request(Core->Device.ClkHandles);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Core->Device.Node.State = (u8)XPM_DEVSTATE_RUNNING;
	Core->isCoreUp = 1;
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

int XPmCore_AfterDirectPwrDwn(XPm_Core *Core)
{
	int Status = XST_FAILURE;
	XPm_Power *PwrNode;
	XPm_Device *DevTcm0A = XPmDevice_GetById(PM_DEV_TCM_0_A);
	XPm_Device *DevTcm0B = XPmDevice_GetById(PM_DEV_TCM_0_B);
	XPm_Device *DevTcm1A = XPmDevice_GetById(PM_DEV_TCM_1_A);
	XPm_Device *DevTcm1B = XPmDevice_GetById(PM_DEV_TCM_1_B);

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
		Status = XPmDevice_Reset(&Core->Device, PM_RESET_ACTION_ASSERT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
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
		ENABLE_WAKE(Core->SleepMask);
	}

	Core->Device.Node.State = (u8)XPM_DEVSTATE_UNUSED;
	Core->isCoreUp = 0;
	Status = XST_SUCCESS;

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
