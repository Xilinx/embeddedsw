/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#include "xpm_fsm.h"
#include "xpm_device.h"
#include "xpm_runtime_device.h"
#include "xpm_power.h"
#include "xpm_debug.h"
#include "xpm_core.h"
#include "xpm_runtime_clock.h"
#include "xpm_requirement.h"

static XStatus ActionBringUpAll(XPm_Device* const Device) {
	// Perform actions to bring up the device
	PmDbg("Bringing up the device\n");
	return XPmDevice_BringUp(Device);
}
static XStatus SetClocks(const XPm_Device *Device, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	u32 NodeId;

	/*
	 * Clocks for PLD and AIE is handled by CDO, hence do not handle in
	 * firmware. Skip for PLD & AIE Devices
	*/
	NodeId = Device->Node.Id;
	if (((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(NodeId)) ||
	    ((u32)XPM_NODESUBCL_DEV_AIE == NODESUBCLASS(NodeId))) {
		Status = XST_SUCCESS;
		goto done;
	}

	const XPm_ClockHandle *ClkHandle = Device->ClkHandles;

	/* Enable all the clock gates, skip over others */
	if (1U == Enable) {
		Status = XPmClock_Request(ClkHandle);
	} else {
		Status = XPmClock_Release(ClkHandle);
	}

done:
	return Status;
}
static u32 IsRunning(const XPm_Device *Device)
{
	u32 Running = 0;
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	LIST_FOREACH(DevOps->Requirements, ReqmNode){
		if (1 == ReqmNode->Data->Allocated) {
			Running = 1;
			break;
		}
	}
	return Running;
}
static XStatus ActionShutdown(XPm_Device* const Device) {
	// Perform actions to shut down the device
	XStatus Status = XST_FAILURE;
	u32 DbgErr = XPM_INT_ERR_UNDEFINED;
	PmInfo("Shutting down the device\n");
	if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(Device->Node.Id)) &&
	((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(Device->Node.Id))) {
		/** Shutdown Core */
		XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(Device->Node.Id);
		if ((NULL != Core) && (NULL != Core->CoreOps) && (NULL != Core->CoreOps->PowerDown)) {
			Status = Core->CoreOps->PowerDown(Core);
			goto done;
		}
	}
	/* Assert reset for peripheral devices */
	if ((u32)XPM_NODESUBCL_DEV_PERIPH == NODESUBCLASS(Device->Node.Id)) {
		Status = XPmDevice_Reset(Device, PM_RESET_ACTION_ASSERT);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RST_ASSERT;
			goto done;
		}
	}
	/* Disable all clocks */
	Status = SetClocks(Device, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_CLK_DISABLE;
		goto done;
	}
	/* Power down the device */
	Status = Device->Power->HandleEvent(&Device->Power->Node, (u32)XPM_POWER_EVENT_PWR_DOWN);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PWRDN;
	}
	Device->Node.Flags &= (u8)(~NODE_IDLE_DONE);
	if (Device->WfPwrUseCnt == Device->Power->UseCount) {
		/** TODO: waitfor dealloc need to be implement in runtime */
		// if (1U == Device->WfDealloc) {
		// 	Device->PendingReqm->Allocated = 0;
		// 	Device->WfDealloc = 0;
		// }

		XPmRequirement_ReleaseFromAllSubsystem(Device);

		if (0U == IsRunning(Device)) {
			Status = XST_FAILURE;
			PmErr("Error during power down device ID=0x%x. Device still running ... ", Device->Node.Id);
		}
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus ActionRuntimeSuspend(XPm_Device* const Device) {
	// Perform actions to suspend the device
	PmWarn("Suspending the device\n");
	/* TODO: IMPLEMENT ME */
	(void)Device;
	return XST_SUCCESS;
}

static XStatus ActionResume(XPm_Device* const Device) {
	// Perform actions to resume the device
	PmWarn("Resuming the device\n");
	/* TODO: IMPLEMENT ME */
	(void)Device;
	return XST_SUCCESS;
}
static const XPmFsm_StateCap XPmGenericDeviceStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.Cap = (u32)PM_CAP_UNUSABLE,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = XPM_MAX_CAPABILITY | (u32)PM_CAP_UNUSABLE,
	},
};

static const XPmFsm_Tran XPmGenericDevEventTransitions[] = {
	{
	.Event = (u32)XPM_DEVEVENT_BRINGUP_ALL,
	.FromState = (u32)XPM_DEVSTATE_UNUSED,
	.ToState = (u32)XPM_DEVSTATE_RUNNING,
	.Latency = XPM_DEF_LATENCY,
	.Action = ActionBringUpAll,
	}, {
	.Event = (u32)XPM_DEVEVENT_SHUTDOWN,
	.FromState = (u32)XPM_DEVSTATE_RUNNING,
	.ToState = (u32)XPM_DEVSTATE_UNUSED,
	.Latency = XPM_DEF_LATENCY,
	.Action = ActionShutdown,
	}, {
	.Event = (u32)XPM_DEVEVENT_RUNTIME_SUSPEND,
	.FromState = (u32)XPM_DEVSTATE_RUNNING,
	.ToState = (u32)XPM_DEVSTATE_RUNTIME_SUSPEND,
	.Latency = XPM_DEF_LATENCY,
	.Action = ActionRuntimeSuspend,
	}, {
	.Event = (u32)XPM_DEVEVENT_BRINGUP_ALL,
	.FromState = (u32)XPM_DEVSTATE_RUNTIME_SUSPEND,
	.ToState = (u32)XPM_DEVSTATE_RUNNING,
	.Latency = XPM_DEF_LATENCY,
	.Action = ActionResume,
	},
};


#define DEFINE_DEV_STATES(S)	.States = (S), \
				.StatesCnt = ARRAY_SIZE(S)

#define DEFINE_DEV_TRANS(T)	.Trans = (T), \
				.TransCnt = ARRAY_SIZE(T)

static XPm_Fsm XPmGenericDeviceFsm = {
	DEFINE_DEV_STATES(XPmGenericDeviceStates),
	DEFINE_DEV_TRANS(XPmGenericDevEventTransitions),
};
XStatus XPm_Fsm_Init(XPmRuntime_DeviceOps* const DevOps) {
	XStatus Status = XST_FAILURE;
	if (NULL == DevOps) {
		PmErr("Runtime Device Ops is not initalized.");
		Status = XST_FAILURE;
		goto done;
	}
	DevOps->Fsm =&XPmGenericDeviceFsm;
	Status = XST_SUCCESS;
done:
	return Status;
}