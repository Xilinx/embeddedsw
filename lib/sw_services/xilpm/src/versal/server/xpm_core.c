/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xplmi.h"
#include "xplmi_scheduler.h"
#include "xpm_psm_api.h"
#include "xpm_core.h"
#include "xpm_psm.h"
#include "xpm_debug.h"
#include "xpm_notifier.h"
#include "xpm_requirement.h"
#include "xpm_subsystem.h"

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
	Core->IsCoreIdleSupported = 0U;
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

XStatus XPmCore_StoreResumeAddr(const XPm_Core *Core, u64 Address)
{
	XStatus Status = XST_FAILURE;
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

XStatus XPmCore_HasResumeAddr(const XPm_Core *Core)
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

XStatus XPmCore_SetCPUIdleFlag(const XPm_Core *Core, u32 CpuIdleFlag)
{
	XStatus Status = XST_FAILURE;

	if ((NULL == Core) || ((u8)PROC_DEV_MAX == Core->PsmToPlmEvent_ProcIdx)) {
		goto done;
	}

	/* Store the CPU idle flag to PSM reserved RAM location */
	PsmToPlmEvent->CpuIdleFlag[Core->PsmToPlmEvent_ProcIdx] = CpuIdleFlag;
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmCore_GetCPUIdleFlag(const XPm_Core *Core, u32 *CpuIdleFlag)
{
	XStatus Status = XST_FAILURE;

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
	/* Send notification about core state change */
	XPmNotifier_Event(Core->Device.Node.Id, (u32)EVENT_STATE_CHANGE);

done:
	return Status;
}

XStatus XPmCore_AfterDirectWakeUp(XPm_Core *Core)
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
	/* Send notification about core state change */
	XPmNotifier_Event(Core->Device.Node.Id, (u32)EVENT_STATE_CHANGE);

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
	const XPm_Device *DevTcm0A = XPmDevice_GetById(PM_DEV_TCM_0_A);
	const XPm_Device *DevTcm0B = XPmDevice_GetById(PM_DEV_TCM_0_B);
	const XPm_Device *DevTcm1A = XPmDevice_GetById(PM_DEV_TCM_1_A);
	const XPm_Device *DevTcm1B = XPmDevice_GetById(PM_DEV_TCM_1_B);

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
	/* Send notification about core state change */
	XPmNotifier_Event(Core->Device.Node.Id, (u32)EVENT_STATE_CHANGE);

done:
	return Status;
}

XStatus XPmCore_GetWakeupLatency(const u32 DeviceId, u32 *Latency)
{
	XStatus Status = XST_SUCCESS;
	const XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
	const XPm_Power *Power;
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

XStatus XPmCore_ForcePwrDwn(u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Power *Acpu0PwrNode = XPmPower_GetById(PM_POWER_ACPU_0);
	const XPm_Power *Acpu1PwrNode = XPmPower_GetById(PM_POWER_ACPU_1);
	const XPm_Power *FpdPwrNode = XPmPower_GetById(PM_POWER_FPD);
	XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);

	if (NULL == Core) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((NULL != Core->CoreOps) && (NULL != Core->CoreOps->PowerDown)) {
		Status = Core->CoreOps->PowerDown(Core);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		/**
		 * Disable the direct wake in case of force
		 * power down.
		 */
		DISABLE_WAKE(Core->SleepMask);
	} else {
		Status = XST_FAILURE;
		goto done;
	}
	/*
	 * Do APU GIC pulse reset if All the cores are in Power OFF
	 * state and FPD in Power ON state. Now APU has two core as
	 * ACPU0 and ACPU1.
	 */
	if (((PM_DEV_ACPU_0 == DeviceId) || (PM_DEV_ACPU_1 == DeviceId)) &&
	    (NULL != Acpu0PwrNode) && (NULL != Acpu1PwrNode) &&
	    (NULL != FpdPwrNode) &&
	    ((u8)XPM_POWER_STATE_OFF != FpdPwrNode->Node.State) &&
	    ((u8)XPM_POWER_STATE_OFF == Acpu0PwrNode->Node.State) &&
	    ((u8)XPM_POWER_STATE_OFF == Acpu1PwrNode->Node.State)) {
		Status = XPmReset_AssertbyId(PM_RST_ACPU_GIC,
					     (u32)PM_RESET_ACTION_PULSE);
	}

done:
	return Status;
}

XStatus XPmCore_ProcessPendingForcePwrDwn(u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId;
	const XPm_Requirement *Reqm;
	const XPm_Subsystem *Subsystem;
	const XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
	u32 Ack = 0U;
	u32 IpiMask = 0U;
	u32 NodeState = 0U;

	if (NULL == Core) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmCore_ForcePwrDwn(DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	SubsystemId = XPmDevice_GetSubsystemIdOfCore(&Core->Device);
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XST_SUCCESS;
		goto done;
	}

	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
		if ((1U == Reqm->Allocated) &&
		    ((u32)XPM_NODESUBCL_DEV_CORE ==
		     NODESUBCLASS(Reqm->Device->Node.Id)) &&
		    ((u8)XPM_DEVSTATE_PENDING_PWR_DWN == Reqm->Device->Node.State)) {
			break;
		}
		Reqm = Reqm->NextDevice;
	}

	if ((u8)PENDING_POWER_OFF == Subsystem->State) {
		if (NULL == Reqm) {
			Status = XPmSubsystem_ForcePwrDwn(Subsystem->Id);
		}
	} else if ((u8)PENDING_RESTART == Subsystem->State) {
		if (NULL == Reqm) {
			Status = XPm_SystemShutdown(SubsystemId,
					PM_SHUTDOWN_TYPE_RESET,
					PM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM,
					XPLMI_CMD_SECURE);
		}
	} else {
		Ack = Core->FrcPwrDwnReq.AckType;
		IpiMask = Core->FrcPwrDwnReq.InitiatorIpiMask;
		NodeState = Core->Device.Node.State;
		Status = XPlmi_SchedulerRemoveTask(XPLMI_MODULE_XILPM_ID,
						   XPm_ForcePwrDwnCb, 0U,
						   (void *)DeviceId);
		if (XST_SUCCESS != Status) {
			PmDbg("Task not present\r\n");
			Status = XST_SUCCESS;
		}
	}

done:
	XPm_ProcessAckReq(Ack, IpiMask, Status, DeviceId, NodeState);

	return Status;
}
