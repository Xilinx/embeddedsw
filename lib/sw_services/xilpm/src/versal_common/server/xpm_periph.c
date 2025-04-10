/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_api.h"
#include "xpm_periph.h"
#include "xpm_gic_proxy.h"
#include "xpm_defs.h"
#include "xpm_requirement.h"
#include "xplmi_err_common.h"
#include "xplmi_scheduler.h"
#include "xpm_regs.h"
#include "xpm_common.h"
#include "xpm_ioctl.h"

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM

#include "xplmi_ssit.h"

/* Scheduler owner ID for SSIT propagation task */
#define XPM_SSIT_TEMP_PROP_ID	PM_DEV_AMS_ROOT
/* Default task period */
#define DEFAULT_SSIT_TEMP_PERIOD 100U

#endif /* PLM_ENABLE_PLM_TO_PLM_COMM */

static struct XPm_PeriphOps GenericOps = {
	.SetWakeupSource = &XPmGicProxy_WakeEventSet,
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
			XPlmi_UpdateErrorSubsystemId(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
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
	.EnterState = &HandleHbMonDeviceState,
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
	u32 ActiveMonitors = 0U;
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
			Subsystem = Device->Requirements->Subsystem;
			if ((((u8)PENDING_RESTART == Subsystem->State) ||
			    ((u8)PENDING_POWER_OFF == Subsystem->State)) &&
			    (1U == Device->Requirements->Allocated) &&
			    (0U == (Subsystem->Flags & (u8)SUBSYSTEM_IDLE_CB_IS_SENT))) {
				Subsystem->Flags |= (u8)SUBSYSTEM_IDLE_CB_IS_SENT;
				Subsystem->Flags |= (u8)SUBSYSTEM_DO_PERIPH_IDLE;

				/**
				 * Extract the timeout value from the requirement
				 * structure. It is assumed that this node
				 * will not be shared with any subsystem, so
				 * there should be only one requirement
				 */
				Timeout = Device->Requirements->PreallocQoS;

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

				Status = XPm_SubsystemIdleCores(Device->Requirements->Subsystem);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			} else {
				PmErr("Healthy Boot Timer %lu Expired. Triggering recovery\r\n",
									Idx);
				HbMon_TimeoutList[Idx] = 0U;
				XPlmi_HandleSwError(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
							(u32)1U << Idx);
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

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM

/*
 * Handler for SSIT temperature propagation periodic task
 */
#ifdef CPPUTEST
int XPmPeriph_TempPropTask(void *data)
#else
static int XPmPeriph_TempPropTask(void *data)
#endif
{
	XStatus Status = XST_FAILURE;
	u32 Response[XPLMI_CMD_RESP_SIZE-1U];
	s16 Min, Max;
	u32 NodeId = PM_DEV_AMS_ROOT;
	u32 SlrCount = 0U;
	s16 Temp;

	(void)data;

	/* Get primary SLR device temp */
	Min = (s16)XPm_In32(PMC_SYSMON_BASEADDR + PMC_SYSMON_DEVICE_TEMP_MIN_OFFSET);
	Max = (s16)XPm_In32(PMC_SYSMON_BASEADDR + PMC_SYSMON_DEVICE_TEMP_MAX_OFFSET);

	for (u32 SlrMask = XPlmi_GetSlavesSlrMask(); (SlrMask & 0x1U) != 0U; SlrMask >>= 1U) {
		++SlrCount;
		NodeId = PM_DEV_AMS_ROOT | (SlrCount << NODE_SLR_IDX_SHIFT);

		/* Get Max temp from secondary SLR */
		Status = XPm_DevIoctl(PM_SUBSYS_DEFAULT, NodeId, IOCTL_GET_SSIT_TEMP,
				      PMC_SYSMON_DEVICE_TEMP_MAX_OFFSET, 0U, 0U, Response, XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Temp = (s16)Response[0];
		/* Check if SLR temperature reading exceeds current max */
		if (Temp > Max) {
			Max = Temp;
		}

		/* Get Min temp from secondary SLR */
		Status = XPm_DevIoctl(PM_SUBSYS_DEFAULT, NodeId, IOCTL_GET_SSIT_TEMP,
				      PMC_SYSMON_DEVICE_TEMP_MIN_OFFSET, 0U, 0U, Response, XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Temp = (s16)Response[0];
		/* Check if SLR temperature reading exceeds current min */
		if (Temp < Min) {
			Min = Temp;
		}
	}

	XPm_UnlockPcsr(PMC_SYSMON_BASEADDR);

	/* Store Min and Max temperatures in local registers */
	XPm_Out32(PMC_SYSMON_TEST_ANA_CTRL0, (u32)Max);
	XPm_Out32(PMC_SYSMON_TEST_ANA_CTRL1, (u32)Min);

	XPm_LockPcsr(PMC_SYSMON_BASEADDR);

	/* Check if max temperatures have exceeded device upper threshold */
	if (Max > (s16)(XPm_In32(PMC_SYSMON_DEVICE_TEMP_TH_UPPER))) {
		/*
		* Device temperature has exceeded upper threshold.
		* Trigger device temperature EAM event locally in master SLR.
		*/
		XPm_Out32(PMC_GLOBAL_PMC_ERR2_TRIG, XIL_EVENT_ERROR_MASK_PMCSMON9);
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief This function starts the periodic task for SSIT temperature
 *	  propagation. The task is only enabled on SSIT devices and only runs
 *	  on primary SLR. If it is not enabled through RTCA then the task is
 *	  never started. The task period is configured through RTCA and must
 *	  be no lower than 10ms. Anything lower is rounded up to 10ms and any
 *	  period which is not a multiple of 10 is rounded up. Once the
 *	  task is enabled, it is expected to run for the lifetime of the
 *	  system, it can not be disabled during runtime.
 *
 * @param  None
 *
 * @return XST_SUCCESS if successful, return error code otherwise
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmPeriph_SsitTempPropInitTask(void)
{
	XStatus Status = XST_FAILURE;
	u32 Period = (XPm_In32(XPLMI_RTCFG_SSIT_TEMP_PROPAGATION) &
			 XPM_RTCA_SSIT_TEMP_PROP_FREQ_MASK) >>
			 XPM_RTCA_SSIT_TEMP_PROP_FREQ_SHIFT;

	/*
	 * Check if device is primary SLR. If current device is secondary SLR,
	 * do not start the task and exit without failure.
	 */
	if (XPLMI_SSIT_MASTER_SLR_INDEX != XPlmi_GetSlrIndex()) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check if task should be enabled */
	if (0U == (XPm_In32(XPLMI_RTCFG_SSIT_TEMP_PROPAGATION) &
		   XPM_RTCA_SSIT_TEMP_PROP_ENABLE_MASK)) {
		/* Task is not enabled. Exit without failure */
		Status = XST_SUCCESS;
		goto done;
	}

	/* If periodicity is 0 set to default value of 100ms */
	if (0U == Period) {
		Period = DEFAULT_SSIT_TEMP_PERIOD;
	}

	/*
	 * If periodicity is not a multiple of 10 then round up to the nearest
	 * multiple of 10. This ensures that the period is not faster than
	 * expected.
	 */
	if (0U != (Period % XPLMI_SCHED_TICK)) {
		Period = ((Period / XPLMI_SCHED_TICK) + 1U) * XPLMI_SCHED_TICK;
	}

	Status = XPlmi_SchedulerAddTask(XPM_SSIT_TEMP_PROP_ID, XPmPeriph_TempPropTask,
					NULL, Period, XPLM_TASK_PRIORITY_0,
					NULL, XPLMI_PERIODIC_TASK);

done:
	return Status;
}

#else

XStatus XPmPeriph_SsitTempPropInitTask(void)
{
	return XST_SUCCESS;
}

#endif /* PLM_ENABLE_PLM_TO_PLM_COMM */
