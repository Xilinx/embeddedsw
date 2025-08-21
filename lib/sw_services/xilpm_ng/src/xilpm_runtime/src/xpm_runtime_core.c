/******************************************************************************
* Copyright (C) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
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
#include "xpm_runtime_clock.h"
#include "xpm_core.h"

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
		if (RuntimeCoreNode->Data->Core == Core) {
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
		RuntimeCore->Core = Core;
		RuntimeCore->IsCoreIdleSupported = (u8)Value;
		LIST_PREPEND(RuntimeCoreList, RuntimeCore);
	} else {
		RuntimeCore->IsCoreIdleSupported = (u8)Value;
	}
	PmInfo("Core Idle supported for 0x%x\r\n", Core->Device.Node.Id);

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
		if (RuntimeCoreNode->Data->Core == Core) {
			PmInfo("Core Idle support found for 0x%x\r\n", RuntimeCoreNode->Data->Core->Device.Node.Id);
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
		if (RuntimeCoreNode->Data->Core == Core) {
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
		if (RuntimeCoreNode->Data->Core == Core) {
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
		RuntimeCore->Core = Core;
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

	Status = XPmCore_PowerDown(Core);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	/**
	 * Disable the direct wake in case of force
	 * power down.
	 */
	DisableWake(Core);
	/*
	 * Do APU GIC pulse reset if All the cores are in Power OFF
	 * state and FPD in Power ON state. Now APU has two core as
	 * ACPU0 and ACPU1.
	 */
	Status = ResetAPUGic(DeviceId);

done:
	return Status;
}

XStatus XPmCore_ProcessPendingForcePwrDwn(XPm_Subsystem *Subsystem, XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Subsystem->Id;
	u32 DeviceId = Core->Device.Node.Id;
	const XPm_Requirement *Reqm = NULL;

	PmInfo("Processing pending force power down for 0x%x, State: 0x%x\r\n", DeviceId, Core->Device.Node.State);

	/* Clear power down status, this will make boot status as initial boot */
	if ((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(DeviceId)) {
		const XPm_ApuCore *ApuCore = (XPm_ApuCore *)Core;
		XPm_RMW32(ApuCore->PcilPwrDwnReg, ApuCore->Core.PwrDwnMask, ~ApuCore->Core.PwrDwnMask);
	}

	/* Check any of core is pending power down in subsystem */
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		Reqm = ReqmNode->Data;
		if ((1U == Reqm->Allocated) &&
		    ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(Reqm->Device->Node.Id)) &&
		    ((u8)XPM_DEVSTATE_PENDING_PWR_DWN == Reqm->Device->Node.State)) {
			PmWarn("Core 0x%x is still pending power down, cannot proceed with subsystem shutdown\r\n",
				Reqm->Device->Node.Id);
			break;
		} else {
			Reqm = NULL;
		}
	}

	if (NULL != Reqm) {
		PmInfo("Core 0x%x is pending power down!\r\n", Reqm->Device->Node.Id);
	} else {
		PmInfo("All cores are powered off\r\n");
	}

	if ((u8)PENDING_POWER_OFF == Subsystem->State) {
		/* Process pending subsystem force power down if all cores are powered off */
		PmInfo("Subsystem 0x%x is pending power off, begin ShutDown\r\n", Subsystem->Id);
		if (NULL == Reqm) {
			Subsystem->Flags = 0U;
			Status = XPmSubsystem_ForcePwrDwn(Subsystem->Id);
		}
	} else if ((u8)PENDING_RESTART == Subsystem->State) {
		/* Process pending subsystem restart if all cores are powered off */
		PmInfo("Subsystem 0x%x is pending restart\r\n", Subsystem->Id);
		if (NULL == Reqm) {
			PmInfo("All cores are powered off, triggering subsys restart 0x%x\r\n", SubsystemId);
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
		/* Required by MISRA */
	}

	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}


XStatus XPmCore_ReleaseFromSubsys(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = XPmDevice_GetSubsystemIdOfCore(&Core->Device);
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	PmInfo("Releasing core 0x%x (state: 0x%x) from subsystem 0x%x (state: 0x%x)\r\n",
		Core->Device.Node.Id, Core->Device.Node.Id, SubsystemId, Subsystem->State);

	Status = XPmDevice_Release(SubsystemId, Core->Device.Node.Id,
					XPLMI_CMD_NON_SECURE);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to release core 0x%x from subsystem 0x%x\r\n", Core->Device.Node.Id, SubsystemId);
		goto done;
	}
	PmInfo("Core 0x%x released from subsystem 0x%x, State: 0x%x\r\n", Core->Device.Node.Id, SubsystemId, Core->Device.Node.State);

	PmInfo("Core->isCoreUp = %d, Core->Device.Node.State = %d\r\n",
		Core->isCoreUp, Core->Device.Node.State);

	Status = XPmCore_ProcessPendingForcePwrDwn(Subsystem, Core);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to trigger subsys restart, from pwr done core 0x%x: 0x%x\n", Core->Device.Node.Id, Status);
		goto done;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("Failed during release, Subsys: 0x%x, Core: 0x%x, Status 0x%x\n\r",
			SubsystemId, Core->Device.Node.Id, Status);
	}
	return Status;
}


XStatus ResetAPUGic(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Power *AcpuPwrNode;
	const XPm_Power *FpdPwrNode = XPmPower_GetById(PM_POWER_FPD);
	u32 NodeId;

	if (((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(DeviceId)) &&
	    (NULL != FpdPwrNode) && ((u8)XPM_POWER_STATE_OFF != FpdPwrNode->Node.State)) {
		for (NodeId = PM_POWER_ACPU_0_0; NodeId <= PM_POWER_ACPU_3_3; NodeId++) {
			AcpuPwrNode = XPmPower_GetById(NodeId);
			if ((NULL != AcpuPwrNode) && ((u8)XPM_POWER_STATE_OFF !=
			    AcpuPwrNode->Node.State)) {
				break;
			}
		}
		if (PM_POWER_ACPU_3_3 < NodeId) {
			PmInfo("Resetting APU Gic (No APU cores are off) 0x%x!\r\n", DeviceId);
			Status = XPmReset_AssertbyId(PM_RST_ACPU_GIC, (u32)PM_RESET_ACTION_PULSE);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPmCore_SetClock(u32 CoreId, u32 Enable) {
	XStatus Status = XST_FAILURE;
	XPm_Device *DevCore= XPmDevice_GetById(CoreId);
	if (NULL == DevCore) {
		PmErr("CoreId 0x%x not found\n", CoreId);
		goto done;
	}
	if (Enable) {
		Status = XPmClock_Request(DevCore->ClkHandles);
	} else {
		Status = XPmClock_Release(DevCore->ClkHandles);
	}
done:
	if (XST_SUCCESS != Status) {
		PmErr("CoreId 0x%x Enable = %d Status 0x%x\n", CoreId, Enable, Status);
	}
	return Status;
}
