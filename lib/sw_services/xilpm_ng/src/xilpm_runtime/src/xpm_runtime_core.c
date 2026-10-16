/******************************************************************************
* Copyright (C) 2024-2026 Advanced Micro Devices, Inc. All rights reserved.
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
#include "xpm_update.h"
#include "xpm_runtime_power.h"

static XPmRuntime_CoreList *RuntimeCoreList XPM_INIT_DATA(RuntimeCoreList) = NULL;

/****************************************************************************/
/**
 * @brief  Prepare the runtime core list and find an existing core entry.
 *
 * @param  Core         Core whose runtime entry is requested
 * @param  RuntimeCore  Pointer to receive the matching entry, or NULL on a miss
 *
 * @return XST_SUCCESS when the list is available, or XST_BUFFER_TOO_SMALL when
 *         the list cannot be allocated
 *
 ****************************************************************************/
static XStatus XPmCore_FindRuntimeEntry(const XPm_Core *Core,
					XPmRuntime_Core **RuntimeCore)
{
	XStatus Status = XST_FAILURE;

	/* Create the runtime list on its first use. */
	if (NULL == RuntimeCoreList) {
		RuntimeCoreList = (XPmRuntime_CoreList*)XPm_AllocBytesDevOps(sizeof(XPmRuntime_CoreList));
		/* Report allocation failure before attempting list traversal. */
		if (NULL == RuntimeCoreList) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	}

	/* Reuse the entry when this core was registered previously. */
	LIST_FOREACH(RuntimeCoreList, RuntimeCoreNode) {
		if ((NULL != RuntimeCoreNode->Data) &&
		    (RuntimeCoreNode->Data->Core == Core)) {
			*RuntimeCore = RuntimeCoreNode->Data;
			break;
		}
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief Store idle-notification support in a runtime core entry.
 *
 * @param Core Core whose runtime entry is updated.
 * @param Value Idle-support value whose low 8 bits are stored.
 *
 * @return XST_SUCCESS on success, or XST_BUFFER_TOO_SMALL on allocation failure.
 *
 ****************************************************************************/
static XStatus XPmCore_StoreIdleSupport(XPm_Core *Core, const u32 Value)
{
	XStatus Status = XST_FAILURE;
	XPmRuntime_Core *RuntimeCore = NULL;

	/* Locate an existing entry before allocating persistent storage. */
	Status = XPmCore_FindRuntimeEntry(Core, &RuntimeCore);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Allocate and link storage only for a newly observed core. */
	if (NULL == RuntimeCore) {
		RuntimeCore = (XPmRuntime_Core*)XPm_AllocBytesDevOps(sizeof(XPmRuntime_Core));
		/* Leave the list unchanged when entry allocation fails. */
		if (NULL == RuntimeCore) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		RuntimeCore->Core = Core;
		RuntimeCore->IsCoreIdleSupported = (u8)Value;
		LIST_PREPEND(RuntimeCoreList, RuntimeCore);
	} else {
		/* Update the value retained by the existing entry. */
		RuntimeCore->IsCoreIdleSupported = (u8)Value;
	}
	PmInfo("Core Idle supported for 0x%x\r\n", Core->Device.Node.Id);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Store whether the given core supports idle notifications.
 *
 * A runtime entry is allocated and added when the core has not been seen
 * before; otherwise the existing entry is updated.
 *
 * @param  Core   Pointer to the core whose idle support is updated
 * @param  Value  Idle-support value whose low 8 bits are stored
 *
 * @return XST_SUCCESS on success, XST_INVALID_PARAM for a NULL core, or
 *         XST_BUFFER_TOO_SMALL if runtime storage cannot be allocated
 *
 ****************************************************************************/
XStatus XPmCore_SetCoreIdleSupport(XPm_Core* Core, const u32 Value) {
	XStatus Status = XST_SUCCESS;

	/* Reject requests that cannot be associated with a core. */
	if (NULL == Core) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* Store the value after validating the runtime entry key. */
	Status = XPmCore_StoreIdleSupport(Core, Value);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Retrieve the stored idle-notification support for a core.
 *
 * @param  Core                 Pointer to the core to look up
 * @param  IsCoreIdleSupported  Pointer to receive the stored support value
 *
 * @return XST_SUCCESS on success, XST_INVALID_PARAM for a NULL argument, or
 *         XST_DEVICE_NOT_FOUND when no runtime entry exists for the core
 *
 ****************************************************************************/
XStatus XPmCore_GetCoreIdleSupport(const XPm_Core* Core, u8 *IsCoreIdleSupported) {
	XStatus Status = XST_SUCCESS;
	const XPmRuntime_Core *RuntimeCore = NULL;

	/* Validate both the lookup key and result destination. */
	if ((NULL == Core) || (NULL == IsCoreIdleSupported)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* An uninitialized list cannot contain a runtime record. */
	if (NULL == RuntimeCoreList) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	/* Locate the runtime record belonging to the requested core. */
	LIST_FOREACH(RuntimeCoreList, RuntimeCoreNode) {
		if ((NULL != RuntimeCoreNode->Data) &&
		    (RuntimeCoreNode->Data->Core == Core)) {
			PmInfo("Core Idle support found for 0x%x\r\n", RuntimeCoreNode->Data->Core->Device.Node.Id);
			RuntimeCore = RuntimeCoreNode->Data;
			break;
		}
	}

	/* Report a miss without modifying the caller's output. */
	if (NULL == RuntimeCore) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	/* Return the value retained in the matching runtime record. */
	*IsCoreIdleSupported = (u8)(RuntimeCore->IsCoreIdleSupported);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Retrieves the stored forced power down request for the given core.
 *
 * @param Core		Pointer to the core to look up
 * @param Req		Pointer that is updated with the forced power down request
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
maybe_unused static XStatus XPmCore_GetFrcPwrDwnReq(const XPm_Core* Core, struct XPm_FrcPwrDwnReq *Req)
{
	XStatus Status = XST_SUCCESS;
	const XPmRuntime_Core *RuntimeCore = NULL;

	if ((NULL == Core) || (NULL == Req)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* An uninitialized list cannot contain a runtime record. */
	if (NULL == RuntimeCoreList) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	/* Locate the runtime record belonging to the requested core. */
	LIST_FOREACH(RuntimeCoreList, RuntimeCoreNode) {
		if ((NULL != RuntimeCoreNode->Data) &&
		    (RuntimeCoreNode->Data->Core == Core)) {
			RuntimeCore = RuntimeCoreNode->Data;
			break;
		}
	}

	/* Report a miss without modifying the caller's output. */
	if (NULL == RuntimeCore) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	/* Return the value retained in the matching runtime record. */
	*Req = RuntimeCore->FrcPwrDwnReq;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief Store a forced power-down request in a runtime core entry.
 *
 * @param Core Core whose runtime entry is updated.
 * @param Req Forced power-down request to store.
 *
 * @return XST_SUCCESS on success, or XST_BUFFER_TOO_SMALL on allocation failure.
 *
 ****************************************************************************/
static XStatus XPmCore_StoreFrcPwrDwnReq(XPm_Core *Core,
					 struct XPm_FrcPwrDwnReq Req)
{
	XStatus Status = XST_FAILURE;
	XPmRuntime_Core *RuntimeCore = NULL;

	/* Locate an existing entry before allocating persistent storage. */
	Status = XPmCore_FindRuntimeEntry(Core, &RuntimeCore);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Allocate and link storage only for a newly observed core. */
	if (NULL == RuntimeCore) {
		RuntimeCore = (XPmRuntime_Core*)XPm_AllocBytesDevOps(sizeof(XPmRuntime_Core));
		/* Leave the list unchanged when entry allocation fails. */
		if (NULL == RuntimeCore) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		RuntimeCore->Core = Core;
		RuntimeCore->FrcPwrDwnReq = Req;
		LIST_PREPEND(RuntimeCoreList, RuntimeCore);
	} else {
		/* Update the request retained by the existing entry. */
		RuntimeCore->FrcPwrDwnReq = Req;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Stores the forced power down request for the given core, allocating
 *         a runtime core entry if one does not already exist.
 *
 * @param Core		Pointer to the core to update
 * @param Req		Forced power down request to store
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
maybe_unused static XStatus XPmCore_SetFrcPwrDwnReq(XPm_Core* Core, struct XPm_FrcPwrDwnReq Req)
{
	XStatus Status = XST_SUCCESS;

	/* Reject a request that has no core to associate with it. */
	if (NULL == Core) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* Store the request after validating the runtime entry key. */
	Status = XPmCore_StoreFrcPwrDwnReq(Core, Req);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Calculate the wake-up latency for a core device.
 *
 * A running core has zero latency. For a non-running core, the function adds
 * the wake-up latency of its power node when one is present.
 *
 * @param  DeviceId  Core device node ID
 * @param  Latency   Non-NULL pointer to receive the latency; initialized to zero
 *
 * @return XST_SUCCESS when the latency is available, XST_INVALID_PARAM when
 *         no device exists for DeviceId, or an error from the power node
 *         latency query
 *
 ****************************************************************************/
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
	/** TODO: Add core power UP lattency  */

	/** TODO: Add core powerdown latency here  */

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

maybe_unused static XStatus XPmCore_ForcePwrDwn(u32 DeviceId)
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

/****************************************************************************/
/**
 * @brief  Processes a pending forced power down for the subsystem, and triggers
 *         a subsystem power off or restart once all of its cores are off.
 *
 * @param Subsystem	Pointer to the subsystem being processed
 * @param Core		Pointer to the core that triggered the processing
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
static XStatus XPmCore_ProcessPendingForcePwrDwn(XPm_Subsystem *Subsystem, const XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Subsystem->Id;
	u32 DeviceId = Core->Device.Node.Id;
	const XPm_Requirement *Reqm = NULL;

	PmInfo("Processing pending force power down for 0x%x, State: 0x%x\r\n", DeviceId, Core->Device.Node.State);

	/* Clear power down status, this will make boot status as initial boot */
	if ((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(DeviceId)) {
		const XPm_ApuCore *ApuCore = (const XPm_ApuCore *)Core;
		XPm_RMW32(ApuCore->PcilPwrDwnReg, ApuCore->Core.PwrDwnMask, ~ApuCore->Core.PwrDwnMask);
	}

	/* Check any of core is pending power down in subsystem */
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		Reqm = ReqmNode->Data;
		if ((NULL != Reqm) && (NULL != Reqm->Device) &&
		    (1U == Reqm->Allocated) &&
		    ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(Reqm->Device->Node.Id)) &&
		    ((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(Reqm->Device->Node.Id)) &&
		     ((u8)XPM_DEVSTATE_PENDING_PWR_DWN == Reqm->Device->Node.State)) {
			PmInfo("Core 0x%x is still running/pending power down, continue\r\n",
				Reqm->Device->Node.Id);
			break;
		} else {
			Reqm = NULL;
		}
	}
	/* Set default status as success here as we may not have any action ahaed */
	Status = XST_SUCCESS;

	if (NULL != Reqm) {
		PmInfo("Core 0x%x is pending power down\r\n", Reqm->Device->Node.Id);
	} else {
		PmInfo("No APU cores are in pending power off\r\n");
	}

	if ((u8)PENDING_POWER_OFF == Subsystem->State) {
		/* Process pending subsystem force power down if all cores are powered off */
		if (NULL == Reqm) {
			PmInfo("All cores are powered off, triggering subsys shutdown 0x%x\r\n", SubsystemId);
			PmInfo("Subsystem 0x%x is pending power off\r\n", Subsystem->Id);
			Subsystem->Flags = 0U;
			Status = XPmSubsystem_ForcePwrDwn(Subsystem->Id);
		}
	} else if ((u8)PENDING_RESTART == Subsystem->State) {
		/* Process pending subsystem restart if all cores are powered off */
		if (NULL == Reqm) {
			PmInfo("All cores are powered off, triggering subsys restart 0x%x\r\n", SubsystemId);
			PmInfo("Subsystem 0x%x is pending restart\r\n", Subsystem->Id);
			/*
			 * Control reached here means the idle notification is already sent
			 * to the core. So, clear the subsystem flags which was set to
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

/****************************************************************************/
/**
 * @brief Release a core and process its subsystem's pending transition.
 *
 * @param Core Core being released.
 * @param SubsystemId Pointer to receive the owning subsystem identifier.
 *
 * @return XST_SUCCESS on success, or the first operation error encountered.
 *
 ****************************************************************************/
static XStatus XPmCore_ReleaseAndProcessPending(const XPm_Core *Core,
						u32 *SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	XPm_Power *Fpd = NULL;

	/* Resolve the core's owning subsystem before attempting its release. */
	*SubsystemId = XPmDevice_GetSubsystemIdOfCore(&Core->Device);
	Subsystem = XPmSubsystem_GetById(*SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/*
	 * While a restart, forced power down or subsystem shutdown is pending,
	 * the cores power down one by one and the last one completes the
	 * request. As in XPmSubsystem_ForcePwrDwn(), hold a reference on FPD
	 * across the core release and that completion, so that neither can
	 * switch FPD off: the APU_PCIL write and the subsystem power up need it
	 * on.
	 */
	if (((u8)PENDING_POWER_OFF == Subsystem->State) ||
	    ((u8)PENDING_RESTART == Subsystem->State)) {
		Fpd = XPmPower_HoldFpd();
	}

	/* Record both endpoint states before the release changes either one. */
	PmInfo("Releasing core 0x%x (state: 0x%x) from subsystem 0x%x (state: 0x%x)\r\n",
	       Core->Device.Node.Id, Core->Device.Node.State, *SubsystemId, Subsystem->State);

	/* Release the subsystem's allocation of the core first. */
	Status = XPmDevice_Release(*SubsystemId, Core->Device.Node.Id,
					XPLMI_CMD_NON_SECURE);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to release core 0x%x from subsystem 0x%x\r\n", Core->Device.Node.Id, *SubsystemId);
		goto done;
	}
	PmInfo("Core 0x%x released from 0x%x, Core->isCoreUp = %d, Core->Device.Node.State = %d\r\n",
		Core->Device.Node.Id, *SubsystemId,
		Core->isCoreUp, Core->Device.Node.State);

	/* Complete any subsystem transition waiting for its cores to stop. */
	Status = XPmCore_ProcessPendingForcePwrDwn(Subsystem, Core);
	if (XST_SUCCESS != Status) {
		PmErr("Failed during process pending pwr done core 0x%x: 0x%x\n", Core->Device.Node.Id, Status);
		goto done;
	}
	PmInfo("Success, Core 0x%x released from 0x%x\r\n", Core->Device.Node.Id, *SubsystemId);

done:
	/* Drop the FPD reference taken for a pending request, if any */
	XPmPower_ReleaseFpd(Fpd);
	/*
	 * Single exit carrying the FIRST failure: later stages are skipped once
	 * one fails, so the returned status identifies the stage that stopped
	 * the release.  SubsystemId is written before the first failure point,
	 * so the caller can still name the subsystem in its own diagnostic.
	 */
	return Status;
}

/****************************************************************************/
/**
 * @brief  Releases the given core from its subsystem and processes any pending
 *         forced power down for that subsystem.
 *
 * @param Core		Pointer to the core to release
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
XStatus XPmCore_ReleaseFromSubsys(const XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId;

	/* Release the core only after resolving its owning subsystem. */
	Status = XPmCore_ReleaseAndProcessPending(Core, &SubsystemId);

	/* Preserve common failure reporting for every release stage. */
	if (XST_SUCCESS != Status) {
		PmErr("Failed during release, Subsys: 0x%x, Core: 0x%x, Status 0x%x\n\r",
			SubsystemId, Core->Device.Node.Id, Status);
	}
	return Status;
}


XStatus ResetAPUGic(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Power *AcpuPwrNode = NULL;
	const XPm_Power *FpdPwrNode = XPmPower_GetById(PM_POWER_FPD);
	u32 NodeId;

	if (((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(DeviceId)) &&
	    (NULL != FpdPwrNode) && ((u8)XPM_POWER_STATE_OFF != FpdPwrNode->Node.State)) {
		for (NodeId = PM_POWER_ACPU_0_0; NodeId <= PM_POWER_ACPU_3_1; NodeId++) {
			AcpuPwrNode = XPmPower_GetById(NodeId);
			if (AcpuPwrNode == NULL) {
				continue;
			}
			if ((NULL != AcpuPwrNode) &&
			    ((u8)XPM_POWER_STATE_OFF != AcpuPwrNode->Node.State)) {
				break;
			}
		}
		if (PM_POWER_ACPU_3_1 < NodeId) {
			PmInfo("Resetting APU GIC, all APU cores are off\r\n");
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
	const XPm_Device *DevCore= XPmDevice_GetById(CoreId);

	/* Resolve the core before accessing its clock handles. */
	if (NULL == DevCore) {
		PmErr("CoreId 0x%x not found\n", CoreId);
		goto done;
	}
	/*
	 * ClkHandles is the core's whole clock list, so one call covers every
	 * clock the core depends on.  Request and Release are reference
	 * counted rather than absolute: a clock shared with another active
	 * device stays on after the release below.
	 */
	if (0U != Enable) {
		Status = XPmClock_Request(DevCore->ClkHandles);
	} else {
		Status = XPmClock_Release(DevCore->ClkHandles);
	}
done:
	/*
	 * Report the core and the direction here.  The clock layer names the
	 * individual clock that failed, and this function's callers pass on
	 * neither piece of information.
	 */
	if (XST_SUCCESS != Status) {
		PmErr("CoreId 0x%x Enable = %d Status 0x%x\n", CoreId, Enable, Status);
	}
	return Status;
}
