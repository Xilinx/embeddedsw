/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xpm_apucore.h"

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

XStatus XPmCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;
	u32 SubsystemId;
	const XPm_Subsystem *Subsystem;

	if (1U == Core->isCoreUp) {
		Status = XPM_ERR_WAKEUP;
		goto done;
	}

	DisableWake(Core);

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
		Status = XPmCore_PlatClkReq(Core->Device.ClkHandles);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Reset will be released at a part of direct power up sequence */
	Status = XPm_DirectPwrUp(Core->Device.Node.Id);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	SubsystemId = XPmDevice_GetSubsystemIdOfCore(&Core->Device);
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	/**
	 * Mark core as pending power down if subsystem restart or force power
	 * down is pending.
	 */
	if ((NULL != Subsystem) && (((u8)PENDING_RESTART == Subsystem->State) ||
	   ((u8)PENDING_POWER_OFF == Subsystem->State))) {
		Core->Device.Node.State = (u8)XPM_DEVSTATE_PENDING_PWR_DWN;
	} else {
		Core->Device.Node.State = (u8)XPM_DEVSTATE_RUNNING;
	}

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
	u32 SubsystemId;
	const XPm_Subsystem *Subsystem;

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
		Status = XPmCore_PlatClkReq(Core->Device.ClkHandles);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	SubsystemId = XPmDevice_GetSubsystemIdOfCore(&Core->Device);
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	/**
	 * Mark core as pending power down if subsystem restart or force power
	 * down is pending.
	 */
	if ((NULL != Subsystem) && (((u8)PENDING_RESTART == Subsystem->State) ||
	   ((u8)PENDING_POWER_OFF == Subsystem->State))) {
		Core->Device.Node.State = (u8)XPM_DEVSTATE_PENDING_PWR_DWN;
	} else {
		Core->Device.Node.State = (u8)XPM_DEVSTATE_RUNNING;
	}

	Core->isCoreUp = 1;
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
		DisableWake(Core);
	} else {
		Status = XST_FAILURE;
		goto done;
	}
	/*
	 * Do APU GIC pulse reset if All the cores are in Power OFF
	 * state and FPD in Power ON state. Now APU has two core as
	 * ACPU0 and ACPU1.
	 */
	Status = ResetAPUGic(DeviceId);

done:
	return Status;
}

XStatus XPmCore_ProcessPendingForcePwrDwn(u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId;
	const XPm_Requirement *Reqm;
	XPm_Subsystem *Subsystem;
	const XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
	const XPm_ApuCore *ApuCore;
	u32 Ack = 0U;
	u32 IpiMask = 0U;
	u32 NodeState = 0U;

	if (NULL == Core) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Ack = Core->FrcPwrDwnReq.AckType;
	IpiMask = Core->FrcPwrDwnReq.InitiatorIpiMask;
	NodeState = Core->Device.Node.State;

	/* Powerdown core forcefully */
	Status = XPmCore_ForcePwrDwn(DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	NodeState = Core->Device.Node.State;
	/*clear pwr dwn status. this will make boot status as initial boot*/
	if(XPM_NODETYPE_DEV_CORE_APU == NODETYPE(DeviceId)){
		ApuCore = (XPm_ApuCore *)Core;
		XPm_RMW32(ApuCore->PcilPwrDwnReg,ApuCore->Core.PwrDwnMask,
			~ApuCore->Core.PwrDwnMask);
	}

	SubsystemId = XPmDevice_GetSubsystemIdOfCore(&Core->Device);
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check any of core is pending power down in subsystem */
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
		/* Process pending subsystem force power down if all cores are
		 * powered off.
		 */
		if (NULL == Reqm) {
			Subsystem->Flags = 0U;
			Status = XPmSubsystem_ForcePwrDwn(Subsystem->Id);
		}
	} else if ((u8)PENDING_RESTART == Subsystem->State) {
		/* Process pending subsystem restart if all cores are powered
		 * off.
		 */
		if (NULL == Reqm) {
			/*
			 * Control reached here means the idle notification is already sent
			 * to the core.So, clear the subsystem flags which was set to
			 * SUBSYSTEM_IDLE_SUPPORTED during the XPm_RegisterNotifier.
			 * This will avoid idling the cores again during subsystem restart.
			 */
			Subsystem->Flags = 0U;
			Status = XPmSubsystem_ForcePwrDwn(Subsystem->Id);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			Status = XPm_SubsystemPwrUp(Subsystem->Id);
			XPm_Power *Fpd = XPmPower_GetById(PM_POWER_FPD);
			/* Restore FPD use count */
			Fpd->UseCount--;
		}
	} else {
		/* Required by MISRA */
	}

done:
	XPm_ProcessAckReq(Ack, IpiMask, Status, DeviceId, NodeState);

	return Status;
}
