/******************************************************************************
* Copyright (C)  2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xstatus.h"
#include "xplmi.h"
#include "xplmi_scheduler.h"
#include "xplmi_err.h"
#include "xpm_subsystem.h"
#include "xpm_alloc.h"
#include "xpm_requirement.h"
#include "xpm_runtime_device.h"
#include "xpm_device_fsm.h"
#include "xpm_regs.h"
#include "xpm_runtime_api.h"

/*
 * HBMon Scheduler interval in ms
 */
#define HBMON_SCHED_PERIOD	(100U)

/**
 * For HBMon[0:3], map to SW error events bit 0-3
 * For HBMon[4:7], map to SW error events bit 10-13
 */
#define HB_MON_SW_ERR_MASK(Idx) \
	(((Idx) < XPM_NODEIDX_DEV_HB_MON_4) ? \
	 ((u32)1U << (Idx)) : ((u32)1U << ((Idx) + 6U)))

/*
 ***************************************************************
 * Healthy Boot Scheduler related global variable Initialization
 ***************************************************************
 */
static u32  HbMon_IsSchedRunning = 0U;

/*
 * Timeout (in ms) List for each Healthy boot node when its
 * active (0 if not active)
 */
static u32 HbMon_TimeoutList[XPM_NODEIDX_DEV_HB_MON_MAX];

/* Scheduler Interval in ms */
static const u32 HbMon_SchedFreq = HBMON_SCHED_PERIOD;

/*
 * Unique Owner ID. Keep it same as Node id of first HBMon node.
 */
static const u32 HbMon_SchedId = PM_DEV_HB_MON_0;

/* Function prototypes for scheduler and timers*/
static int HbMon_Scheduler(void *data);

static XStatus HbMon_StartTimer(u32 HbMonIdx, u32 TimeoutVal)
{
	XStatus Status = XST_FAILURE;

	PmWarn("HbMon_StartTimer: HbMonIdx=%lu, TimeoutVal=%lu ms\r\n",
					HbMonIdx, TimeoutVal);

	if (TimeoutVal == 0U) {
		Status = XST_FAILURE;
		PmErr("Invalid Timeout value of %lu ms for HbMon[%lu]\r\n",
					TimeoutVal, HbMonIdx);
		goto done;
	}

	if (0U == HbMon_IsSchedRunning) {
		/*
		 * Start the scheduler if not running
		 */
		PmWarn("Starting the scheduler for Healthy Boot Monitors\r\n");
		Status = XPlmi_SchedulerAddTask(HbMon_SchedId, &HbMon_Scheduler,
						NULL, HbMon_SchedFreq, XPLM_TASK_PRIORITY_0,
						NULL, XPLMI_PERIODIC_TASK);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		HbMon_IsSchedRunning = 1U;
	}

	/* Set Timeout in milliseconds. */
	HbMon_TimeoutList[HbMonIdx] = TimeoutVal;

	/*
	 * To ensure minimum time before triggering recovery,
	 * absorb the possible 100ms margin of error (task can be triggered
	 * 100ms faster than expected, if this is not done) by adding 100 to
	 * the timeout value. Also check the overflow condition.
	 */
	if (TimeoutVal <= (UINT32_MAX - HbMon_SchedFreq)) {
		HbMon_TimeoutList[HbMonIdx] += HbMon_SchedFreq;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

static void HbMon_StopTimer(u32 HbMonIdx)
{
	HbMon_TimeoutList[HbMonIdx] = 0U;
}

/*
 * Handler for the scheduled task.
 * This will be called periodically every 'HbMon_SchedFreq' ms
 */
static int HbMon_Scheduler(void *data)
{
	(void)data;
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device, *Ipi;
	XPm_Subsystem *Subsystem;
	XPmRuntime_DeviceOps *DevOps = NULL;
	u32 ActiveMonitors = 0U;
	u32 IsAllocated = 0U;
	u32 Idx, Timeout, NodeId, IpiIsrVal;

	for (Idx = (u32)XPM_NODEIDX_DEV_HB_MON_0;
			Idx < (u32)XPM_NODEIDX_DEV_HB_MON_MAX; Idx++)
	{
		if (HbMon_TimeoutList[Idx] == 0U) {
			continue;
		}
		ActiveMonitors++;

		if (HbMon_TimeoutList[Idx] <= HbMon_SchedFreq) {
			Device = XPmDevice_GetHbMonDeviceByIndex(Idx);
			DevOps = XPm_GetDevOps_ById(Device->Node.Id);
			if (NULL == DevOps || NULL == DevOps->PendingReqm) {
				PmErr("DeviceOps/PendingReqm is NULL - active HB Mon %d\r\n", Idx);
				Status = XST_FAILURE;
				goto done;
			}
			Subsystem = DevOps->PendingReqm->Subsystem;
			IsAllocated = DevOps->PendingReqm->Allocated;
			if (1U != IsAllocated) {
				PmWarn("PendingReqm is not allocated - active HB Mon %d\r\n", Idx);
			}
			if ((((u8)PENDING_RESTART == Subsystem->State) ||
			    ((u8)PENDING_POWER_OFF == Subsystem->State)) &&
			    (1U == IsAllocated) &&
			    (0U == (Subsystem->Flags & (u8)SUBSYSTEM_IDLE_CB_IS_SENT))) {
				Subsystem->Flags |= (u8)SUBSYSTEM_IDLE_CB_IS_SENT;

				/**
				 * Extract the timeout value from the requirement
				 * structure. It is assumed that this node
				 * will not be shared with any subsystem, so
				 * there should be only one requirement
				 */
				Timeout = DevOps->PendingReqm->PreallocQoS;

				PmWarn("Extracted timeout %lu ms for HbMon[%lu] before idle callback\r\n",
									Timeout, Idx);
				PmWarn("Sending idle callback to Subsystem 0x%x (again)\r\n",
									Subsystem->Id);

				Status = HbMon_StartTimer(Idx, Timeout);
				if (XST_SUCCESS != Status) {
					goto done;
				}

				/**
				 * Clear pending idle callback status. In case of Linux hang, Linux
				 * will not clear IPI ISR register. So clear from here to re-send idle
				 * callback to TF-A.
				 */
				for (NodeId = PM_DEV_IPI_0; NodeId <= PM_DEV_IPI_6; NodeId++) {
					Ipi = XPmDevice_GetById(NodeId);
					if (NULL != Ipi) {
						IpiIsrVal = XPm_Read32(Ipi->Node.BaseAddress +
								       IPI_ISR_OFFSET);
						if (0U != (IpiIsrVal & PMC_IPI_MASK)) {
							XPm_Write32(Ipi->Node.BaseAddress + IPI_ISR_OFFSET,
								    (IpiIsrVal & PMC_IPI_MASK));
						}
					}
				}

				Status = XPm_SubsystemIdleCores(Subsystem);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			} else {
				PmErr("Healthy Boot Timer %lu Expired. Triggering recovery\r\n",
									Idx);
				HbMon_TimeoutList[Idx] = 0U;
				XPlmi_HandleSwError(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
							HB_MON_SW_ERR_MASK(Idx));
			}
		} else {
			HbMon_TimeoutList[Idx] = HbMon_TimeoutList[Idx] - HbMon_SchedFreq;
		}
	}

	/*
	 * Disable the scheduler if no monitors were active during this pass.
	 * Note: We are waiting for next pass, even when the action is triggered
	 * during this pass.
	 */
	if (ActiveMonitors == 0U) {
		PmWarn("No active Healthy Boot Monitors. Stopping the scheduler\r\n");
		Status = XPlmi_SchedulerRemoveTask(HbMon_SchedId, &HbMon_Scheduler,
							HbMon_SchedFreq, NULL);
		if (XST_SUCCESS == Status) {
			HbMon_IsSchedRunning = 0U;
		} else {
			PmErr("Removal of scheduler failed. Assuming it is still running \r\n");
			goto done;
		}
	}

	Status = XST_SUCCESS;
done:
	return (int)Status;
}


static XStatus ActionStart(XPm_Device* const Device)
{
	XStatus Status = XST_FAILURE;
	u32 HbMon_Id = NODEINDEX(Device->Node.Id);
	XPmRuntime_DeviceOps *DevOps = XPm_GetDevOps_ById(Device->Node.Id);

	PmWarn("%s: HB Mon[%lu]\r\n", __func__, HbMon_Id);

	if ((NULL == DevOps) || (NULL == DevOps->PendingReqm)) {
		PmErr("DeviceOps/PendingReqm is NULL - HB Mon\r\n");
		goto done;
	}

	if (DevOps->PendingReqm->IsPending != 1) {
		PmErr("PendingReqm is not pending - HB Mon\r\n");
		goto done;
	}

	/* Extract the timeout value from the requirement
	 * structure. It is assumed that this node
	 * will not be shared with any subsystem, so
	 * there should be only one requirement
	 */
	u32 Timeout = DevOps->PendingReqm->Curr.QoS;

	Status = HbMon_StartTimer(HbMon_Id, Timeout);

	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Update the subsystemId for the error node.
	 * This is required for subsystem restart action
	 * configuration in the error node.
	 */
	XPlmi_UpdateErrorSubsystemId(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
			(u32)1U << HbMon_Id,
			DevOps->PendingReqm->Subsystem->Id);
	/* Update use count. */
	Device->WfPwrUseCnt = Device->Power->UseCount + 1U;
	if (NULL != Device->Power) {
		Status = Device->Power->HandleEvent(&Device->Power->Node,
				(u32)XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus ActionStop(XPm_Device* const Device)
{
	XStatus Status = XST_FAILURE;
	u32 HbMon_Id = NODEINDEX(Device->Node.Id);

	PmWarn("%s: HB Mon[%lu]\r\n", __func__, HbMon_Id);

	HbMon_StopTimer(HbMon_Id);
	/* Update use count. */
	if (NULL != Device->Power) {
		Status = Device->Power->HandleEvent(&Device->Power->Node,
				(u32)XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

static const XPmFsm_StateCap XPmHbMonDeviceStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = (u32)PM_CAP_ACCESS,
	},
};

static const XPmFsm_Tran XPmHbMonDevEventTransitions[] = {
	{
	.Event = (u32)XPM_DEVEVENT_BRINGUP_ALL,
	.FromState = (u32)XPM_DEVSTATE_UNUSED,
	.ToState = (u32)XPM_DEVSTATE_RUNNING,
	.Latency = XPM_DEF_LATENCY,
	.Action = ActionStart,
	}, {
	.Event = (u32)XPM_DEVEVENT_SHUTDOWN,
	.FromState = (u32)XPM_DEVSTATE_RUNNING,
	.ToState = (u32)XPM_DEVSTATE_UNUSED,
	.Latency = XPM_DEF_LATENCY,
	.Action = ActionStop,
	},
};

#define DEFINE_DEV_STATES(S)	.States = (S), \
				.StatesCnt = ARRAY_SIZE(S)

#define DEFINE_DEV_TRANS(T)	.Trans = (T), \
				.TransCnt = ARRAY_SIZE(T)

const XPm_Fsm XPmHbMonDeviceFsm = {
	DEFINE_DEV_STATES(XPmHbMonDeviceStates),
	DEFINE_DEV_TRANS(XPmHbMonDevEventTransitions),
};

XPm_Fsm *XPmSubsystem_GetHbMonFsm(void)
{
	PmWarn("Getting Healthy Boot Monitor FSM\r\n");
	return (XPm_Fsm *)&XPmHbMonDeviceFsm;
}
