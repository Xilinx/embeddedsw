/******************************************************************************
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#include "xpm_fsm.h"
#include "xpm_device.h"
#include "xpm_device_fsm.h"
#include "xpm_runtime_device.h"
#include "xpm_power.h"
#include "xpm_debug.h"
#include "xpm_core.h"
#include "xpm_runtime_clock.h"
#include "xpm_requirement.h"
#include "xpm_mem_tcm.h"

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
	/** FIXME: TCM doesn't have any clock so we need to set clock from the corresponding RPU */
	if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(NodeId)) &&
	    ((u32)XPM_NODESUBCL_DEV_MEM == NODESUBCLASS(NodeId)) &&
	    ((u32)XPM_NODETYPE_DEV_TCM == NODETYPE(NodeId))) {
		u32 RpuId = 0;
		Status = XPm_GetRpuByTcmId(NodeId, &RpuId);
		if (XST_SUCCESS != Status) {
			PmErr("Failed to get RPU ID for TCM ID 0x%x\n", Device->Node.Id);
			goto done;
		}
		Device = (XPm_Device*)XPmDevice_GetById(RpuId);
		if (NULL == Device) {
			PmErr("Not found RPU ID = 0x%x\n", RpuId);
			Status = XST_FAILURE;
			goto done;
		}
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

static XStatus ActionBringUpAll(XPm_Device* Device)
{
	XStatus Status = XST_FAILURE;

	/** Perform actions to bring up the device */
	PmDbg("Bringing up the device %x \n\r", Device->Node.Id);
	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/** Power on Device */
	Status = XPmDevice_BringUp(Device);
	if (XST_SUCCESS != Status) {
		PmErr("Bring up device failed!\n");
		goto done;
	}
	/** Start clocks */
	Status = SetClocks(Device, 1U);
	if (XST_SUCCESS != Status) {
		PmErr("clock set failed!\n");
		goto done;
	}
	/** Skip releasing reset if device is core type; since it is done as the part of request wakeup */
	if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(Device->Node.Id)) &&
	    ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(Device->Node.Id))) {
		Status = XST_SUCCESS;
		goto done;
	}

	/** Release reset only for peripheral devices */
	if ((u32)XPM_NODESUBCL_DEV_PERIPH == NODESUBCLASS(Device->Node.Id)) {
		Status = XPmDevice_Reset(Device, PM_RESET_ACTION_RELEASE);
		if (XST_SUCCESS != Status) {
			PmErr("Failed to de-assert reset for 0x%x: 0x%x\n",
					Device->Node.Id, Status);
			goto done;
		}
	}

done:
	return Status;
}

static XStatus ActionShutdown(XPm_Device* const Device) {

	XStatus Status = XST_FAILURE;
	u32 DbgErr = XPM_INT_ERR_UNDEFINED;

	PmInfo("Shutting down the device %x\n", Device->Node.Id);
	if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(Device->Node.Id)) &&
	((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(Device->Node.Id))) {
		/** Shutdown Core */
		XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(Device->Node.Id);
		if (NULL != Core) {
			Status = XPmCore_PowerDown(Core);
			goto done;
		}
	}

	if (XPM_NODESUBCL_DEV_MEM == NODESUBCLASS(Device->Node.Id)) {
		PmInfo("Skipping shutdown for memory device 0x%x\n", Device->Node.Id);
		Status = XST_SUCCESS;
		goto done;
	}

	switch (Device->Node.Id) {
	/* Do not shutdown any IPI devices */
	case PM_DEV_IPI_0 ... PM_DEV_IPI_6:
	case PM_DEV_IPI_NOBUF_1 ... PM_DEV_IPI_NOBUF_6:
		PmInfo("Skipping shutdown for IPI device 0x%x\n", Device->Node.Id);
		Status = XST_SUCCESS;
		goto done;
	default:
		break;
	}

	PmDbg("Periph reset/clock dev 0x%x\r\n", Device->Node.Id);
	/* FIXME: Following reset and clock handling to be tweaked to handle device sharing between subsystems */
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
	if (NULL != Device->Power) {
		/* Power down the device */
		Status = Device->Power->HandleEvent(&Device->Power->Node, (u32)XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PWRDN;
			goto done;
		}
		Device->Node.Flags &= (u8)(~NODE_IDLE_DONE);
		if (Device->WfPwrUseCnt == Device->Power->UseCount) {
			// TODO: Needs to be implemented correctly
			// if (1U == Device->WfDealloc) {
			// 	Device->PendingReqm->Allocated = 0;
			// 	Device->WfDealloc = 0;
			// }
		}
	}

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("Failed during shutdown, Device=0x%x, Status=0x%x\n\r",
			Device->Node.Id, Status);
	}
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus ActionRuntimeSuspend(XPm_Device* const Device) {
	// Perform actions to suspend the device
	PmInfo("Suspending the device %x\n", Device->Node.Id);
	/* TODO: IMPLEMENT ME */
	(void)Device;
	return XST_SUCCESS;
}

static XStatus ActionResume(XPm_Device* const Device) {
	// Perform actions to resume the device
	PmInfo("Resuming the device %x\n", Device->Node.Id);
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
		.State = (u8)XPM_DEVSTATE_PENDING_PWR_DWN,
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
	.Event = (u32)XPM_DEVEVENT_SHUTDOWN,
	.FromState = (u32)XPM_DEVSTATE_PENDING_PWR_DWN,
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
	{
	.Event = (u32)XPM_DEVEVENT_SHUTDOWN,
	.FromState = (u32)XPM_DEVSTATE_RUNTIME_SUSPEND,
	.ToState = (u32)XPM_DEVSTATE_UNUSED,
	.Latency = XPM_DEF_LATENCY,
	.Action = ActionShutdown,
	},

};

#define DEFINE_DEV_STATES(S)	.States = (S), \
				.StatesCnt = ARRAY_SIZE(S)

#define DEFINE_DEV_TRANS(T)	.Trans = (T), \
				.TransCnt = ARRAY_SIZE(T)

const XPm_Fsm XPmGenericDeviceFsm = {
	DEFINE_DEV_STATES(XPmGenericDeviceStates),
	DEFINE_DEV_TRANS(XPmGenericDevEventTransitions),
};

XStatus XPmDeviceFsm_Init(XPmRuntime_DeviceOps* const DevOps) {
	XStatus Status = XST_FAILURE;
	if (NULL == DevOps) {
		PmErr("Runtime Device Ops is not initalized.");
		Status = XST_FAILURE;
		goto done;
	}
	DevOps->FsmType = XPM_FSM_TYPE_GENERIC_DEVICE;

	/* Override FSM type for specific devices */
	if (XPM_NODETYPE_DEV_HB_MON == NODETYPE(DevOps->Device->Node.Id)) {
		DevOps->FsmType = XPM_FSM_TYPE_HB_MON;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}
