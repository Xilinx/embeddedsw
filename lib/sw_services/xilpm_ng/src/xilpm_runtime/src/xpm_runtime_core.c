/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_apucore.h"
#include "xpm_rpucore.h"
#include "xplmi.h"
#include "xplmi_scheduler.h"
#include "xpm_runtime_core.h"
#include "xpm_runtime_api.h"
#include "xpm_runtime_reset.h"
#include "xpm_runtime_device.h"
static XPmRuntime_CoreList *RuntimeCoreList = NULL;

XStatus XPmCore_SetCoreIdleSupport(XPm_Core* Core, const u32 Value) {
	XStatus Status = XST_SUCCESS;
	XPmRuntime_Core *RuntimeCore = NULL;

	if (NULL == Core) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (NULL == RuntimeCoreList) {
		RuntimeCoreList = (XPmRuntime_CoreList*)XPm_AllocBytesDevOps(sizeof(XPmRuntime_CoreList));
		if (NULL == RuntimeCoreList) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	}
	LIST_FOREACH(RuntimeCoreList, RuntimeCoreNode) {
		if (RuntimeCoreNode->Data->Device == Core) {
			RuntimeCore = RuntimeCoreNode->Data;
			break;
		}
	}
	if (NULL == RuntimeCore) {
		RuntimeCore = (XPmRuntime_Core*)XPm_AllocBytesDevOps(sizeof(XPmRuntime_Core));
		if (NULL == RuntimeCore) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		RuntimeCore->Device = Core;
		RuntimeCore->IsCoreIdleSupported = (u8)Value;
		LIST_PREPEND(RuntimeCoreList, RuntimeCore);
	} else {
		RuntimeCore->IsCoreIdleSupported = (u8)Value;
	}

done:
	return Status;
}

XStatus XPmCore_GetCoreIdleSupport(const XPm_Core* Core, u8 *IsCoreIdleSupported) {
	XStatus Status = XST_SUCCESS;
	const XPmRuntime_Core *RuntimeCore = NULL;

	if (NULL == Core || NULL == IsCoreIdleSupported) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (NULL == RuntimeCoreList) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	LIST_FOREACH(RuntimeCoreList, RuntimeCoreNode) {
		if (RuntimeCoreNode->Data->Device == Core) {
			RuntimeCore = RuntimeCoreNode->Data;
			break;
		}
	}

	if (NULL == RuntimeCore) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	*IsCoreIdleSupported = (u8)(RuntimeCore->IsCoreIdleSupported);

done:
	return Status;
}

XStatus XPmCore_GetFrcPwrDwnReq(const XPm_Core* Core, struct XPm_FrcPwrDwnReq *Req)
{
	XStatus Status = XST_SUCCESS;
	const XPmRuntime_Core *RuntimeCore = NULL;

	if (NULL == Core || NULL == Req) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (NULL == RuntimeCoreList) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	LIST_FOREACH(RuntimeCoreList, RuntimeCoreNode) {
		if (RuntimeCoreNode->Data->Device == Core) {
			RuntimeCore = RuntimeCoreNode->Data;
			break;
		}
	}

	if (NULL == RuntimeCore) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	*Req = RuntimeCore->FrcPwrDwnReq;

done:
	return Status;
}

XStatus XPmCore_SetFrcPwrDwnReq(XPm_Core* Core, struct XPm_FrcPwrDwnReq Req)
{
	XStatus Status = XST_SUCCESS;
	XPmRuntime_Core *RuntimeCore = NULL;

	if (NULL == Core) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (NULL == RuntimeCoreList) {
		RuntimeCoreList = (XPmRuntime_CoreList*)XPm_AllocBytesDevOps(sizeof(XPmRuntime_CoreList));
		if (NULL == RuntimeCoreList) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	}
	LIST_FOREACH(RuntimeCoreList, RuntimeCoreNode) {
		if (RuntimeCoreNode->Data->Device == Core) {
			RuntimeCore = RuntimeCoreNode->Data;
			break;
		}
	}
	if (NULL == RuntimeCore) {
		RuntimeCore = (XPmRuntime_Core*)XPm_AllocBytesDevOps(sizeof(XPmRuntime_Core));
		if (NULL == RuntimeCore) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		RuntimeCore->Device = Core;
		RuntimeCore->FrcPwrDwnReq = Req;
		LIST_PREPEND(RuntimeCoreList, RuntimeCore);
	} else {
		RuntimeCore->FrcPwrDwnReq = Req;
	}

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

	if ((u32)XPM_DEVSTATE_RUNNING == Core->Device.Node.State) {
		goto done;
	}
	/** TODO: Add core power UP lattency  */
	//*Latency += Core->PwrUpLatency;

	/** TODO: Add core powerdown latency here  */
	// if ((u32)XPM_DEVSTATE_SUSPENDING == Core->Device.Node.State) {
	// 	*Latency += Core->PwrDwnLatency;
	// 	goto done;
	// }

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
		Status = XST_NO_FEATURE;
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
	const XPm_Requirement *Reqm = NULL;
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
	struct XPm_FrcPwrDwnReq FrcPwrDwnReq;
	Status = XPmCore_GetFrcPwrDwnReq(Core, &FrcPwrDwnReq);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Ack = FrcPwrDwnReq.AckType;
	IpiMask = FrcPwrDwnReq.InitiatorIpiMask;
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
	LIST_FOREACH(Subsystem->Requirements, ReqmNode){
		Reqm = ReqmNode->Data;
		if ((1U == Reqm->Allocated) &&
		    ((u32)XPM_NODESUBCL_DEV_CORE ==
		     NODESUBCLASS(Reqm->Device->Node.Id)) &&
		    ((u8)XPM_DEVSTATE_PENDING_PWR_DWN == Reqm->Device->Node.State)) {
			break;
		}
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
			Status = XPm_SystemShutdown(SubsystemId,
					PM_SHUTDOWN_TYPE_RESET,
					PM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM,
					XPLMI_CMD_SECURE);
		}
	} else {
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
