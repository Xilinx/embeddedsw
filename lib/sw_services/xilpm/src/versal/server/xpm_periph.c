/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_periph.h"
#include "xpm_gic_proxy.h"
#include "xpm_defs.h"
#include "xpm_requirement.h"
#include "xplmi_err.h"
#include "xplmi_scheduler.h"
#include "xpm_prot.h"

/* OSPI/QSPI Memory mapped region */
#define XPM_OSPI_QSPI_MEM_REG_START		(0xC0000000U)

static struct XPm_PeriphOps GenericOps = {
	.SetWakeupSource = XPmGicProxy_WakeEventSet,
};

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
static XStatus HbMon_StartTimer(u32 HbMonIdx, u32 TimeoutVal);
static void HbMon_StopTimer(u32 HbMonIdx);

/************************************************************/

static XStatus XPmPeriph_ProtControl(const XPm_Requirement *Reqm,
				     u32 Enable)
{
	return XPmProt_PpuControl(Reqm, Reqm->Device->Node.BaseAddress,
				  Enable);
}

static XStatus XPmPeriph_ProtControlQspiOspi(const XPm_Requirement *Reqm,
					     u32 Enable)
{
	XStatus Status = XST_FAILURE;

	/* Protect control registers for flash device */
	Status = XPmPeriph_ProtControl(Reqm, Enable);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Protect memory mapped region for flash device */
	Status = XPmProt_PpuControl(Reqm, XPM_OSPI_QSPI_MEM_REG_START,
				    Enable);

done:
	return Status;
}

XStatus XPmPeriph_Init(XPm_Periph *Periph, u32 Id, u32 BaseAddress,
		       XPm_Power *Power, XPm_ClockNode *Clock,
		       XPm_ResetNode *Reset, u32 GicProxyMask,
		       u32 GicProxyGroup)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(&Periph->Device, Id, BaseAddress, Power, Clock,
				Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Protection handler for peripherals */
	switch (Periph->Device.Node.Id) {
	case (u32)PM_DEV_IPI_0:
	case (u32)PM_DEV_IPI_1:
	case (u32)PM_DEV_IPI_2:
	case (u32)PM_DEV_IPI_3:
	case (u32)PM_DEV_IPI_4:
	case (u32)PM_DEV_IPI_5:
	case (u32)PM_DEV_IPI_6:
	case (u32)PM_DEV_IPI_PMC:
		/* Dynamic runtime protection is not supported for IPI devices */
		Periph->Device.HandleProtection = NULL;
		break;
	case (u32)PM_DEV_QSPI:
	case (u32)PM_DEV_OSPI:
		/* Dynamic runtime protection for QSPI/OSPI devices (ctrl + mem) */
		Periph->Device.HandleProtection = &XPmPeriph_ProtControlQspiOspi;
		break;
	default:
		/* Default runtime protection handler for other peripherals */
		Periph->Device.HandleProtection = &XPmPeriph_ProtControl;
		break;
	}

	Periph->PeriphOps = &GenericOps;
	Periph->GicProxyMask = GicProxyMask;
	Periph->GicProxyGroup = GicProxyGroup;

done:
	return Status;
}

/*
 * Implementation for Healthy Boot Monitor
 */
static const XPm_StateCap XPmHbMonDeviceStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = (u32)XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = (u32)PM_CAP_ACCESS,
	},
};

static const XPm_StateTran XPmHbMonDevTransitions[] = {
	{
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_UNUSED,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	},
};

static XStatus HandleHbMonDeviceState(XPm_Device* const Device, const u32 NextState)
{
	XStatus Status = XST_FAILURE;
	u32 HbMon_Id = NODEINDEX(Device->Node.Id);

	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			/* Extract the timeout value from the requirement
			 * structure. It is assumed that this node
			 * will not be shared with any subsystem, so
			 * there should be only one requirement
			 */
			u32 Timeout = Device->PendingReqm->Curr.QoS;

			Status = HbMon_StartTimer(HbMon_Id, Timeout);

			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* Update the subsystemId for the error node.
			 * This is required for subsystem restart action
			 * configuration in the error node.
			 */
			XPlmi_UpdateErrorSubsystemId(XPLMI_NODETYPE_EVENT_ERROR_SW_ERR,
							(u32)1U << HbMon_Id,
							Device->Requirements->Subsystem->Id);
			/* Update use count. */
			Status = XPmDevice_BringUp(Device);

		}
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			HbMon_StopTimer(HbMon_Id);
			/* Update use count. */
			Status = Device->HandleEvent(&Device->Node,
						     XPM_DEVEVENT_SHUTDOWN);

		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

done:
	return Status;
}

static const XPm_DeviceFsm XPmHbMonDeviceFsm = {
	DEFINE_DEV_STATES(XPmHbMonDeviceStates),
	DEFINE_DEV_TRANS(XPmHbMonDevTransitions),
	.EnterState = HandleHbMonDeviceState,
};

XStatus XPmHbMonDev_Init(XPm_Device *Device, u32 Id, XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(Device, Id, 0U, Power, NULL, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Device->DeviceFsm = &XPmHbMonDeviceFsm;

done:
	return Status;
}

static XStatus HbMon_StartTimer(u32 HbMonIdx, u32 TimeoutVal)
{
	XStatus Status = XST_FAILURE;

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
		Status = XPlmi_SchedulerAddTask(HbMon_SchedId, HbMon_Scheduler,
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
	u32 ActiveMonitors = 0U;
	u32 Idx;

	for (Idx = (u32)XPM_NODEIDX_DEV_HB_MON_0;
			Idx < (u32)XPM_NODEIDX_DEV_HB_MON_MAX; Idx++)
	{
		if (HbMon_TimeoutList[Idx] == 0U) {
			continue;
		}

		ActiveMonitors++;

		if (HbMon_TimeoutList[Idx] <= HbMon_SchedFreq) {
			PmErr("Healthy Boot Timer %lu Expired. Triggering recovery\r\n",
									Idx);
			HbMon_TimeoutList[Idx] = 0U;
			XPlmi_HandleSwError(XPLMI_NODETYPE_EVENT_ERROR_SW_ERR,
						(u32)1U << Idx);
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
		Status = XPlmi_SchedulerRemoveTask(HbMon_SchedId, HbMon_Scheduler,
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
