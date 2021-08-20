/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_api.h"
#include "xpm_core.h"
#include "xpm_ipi.h"
#include "xpm_notifier.h"
#include "xpm_power.h"
#include "xplmi_err.h"
#include "xpm_common.h"
#include "xplmi_task.h"
#include "xplmi_scheduler.h"

#ifdef XILPM_NOTIFIER_LIST_SIZE
#define XPM_NOTIFIERS_COUNT XILPM_NOTIFIER_LIST_SIZE /* Provide by user */
#else
#define XPM_NOTIFIERS_COUNT    10U /* Default size */
#endif

#define XILPM_NOTIFIER_INTERVAL	(10U)
#define PRESENT		(1)
#define NOT_PRESENT	(0)

/* Use Time out count 0 to just check for IPI ack without holding the PLM */
#define XPM_NOTIFY_TIMEOUTCOUNT	(0U)

extern XPm_Subsystem *PmSubsystems;

typedef struct {
	const XPm_Subsystem* Subsystem;
	u32 NodeId;
	u32 EventMask;
	u32 WakeMask;
	u32 IpiMask;  /* TODO: Remove this when IPI mask support in CDO is available*/
	u32 PendEvent;
} XPmNotifier;

static XPmNotifier PmNotifiers[XPM_NOTIFIERS_COUNT];

static volatile u32 SchedulerTask = (u32)NOT_PRESENT;

static int XPmNotifier_SchedulerTask(void *Arg);

/****************************************************************************/
/**
 * @brief  Register the notifier for given subsystem, NodeId and event
 *
 * @param  Subsystem  Subsystem to be notified
 * @param  NodeId     NodeId related to event
 * @param  Event      Event to be notified about
 * @param  Wake       Flag specifying whether the subsystem should be woken
 *                    upon event notification
 *
 * @return None
 *
 ****************************************************************************/
XStatus XPmNotifier_Register(XPm_Subsystem* const Subsystem,
			 const u32 NodeId,
			 const u32 Event, const u32 Wake, const u32 IpiMask)
{
	XStatus Status = XST_FAILURE;
	u32 Idx;
	u32 EmptyIdx = ARRAY_SIZE(PmNotifiers);

	for (Idx = 0U; Idx < ARRAY_SIZE(PmNotifiers); Idx++) {
		if (NULL == PmNotifiers[Idx].Subsystem) {
			/* Empty entry found in PmNotifiers array */
			if (EmptyIdx > Idx) {
				/* Remember only first found empty entry */
				EmptyIdx = Idx;
			}
			continue;
		}

		if ((Subsystem == PmNotifiers[Idx].Subsystem) &&
		    (NodeId == PmNotifiers[Idx].NodeId)) {
			/* Drop empty index - existing entry found */
			EmptyIdx = ARRAY_SIZE(PmNotifiers);
			break;
		}
	}

	if ((EmptyIdx == ARRAY_SIZE(PmNotifiers)) &&
	    (Idx >= ARRAY_SIZE(PmNotifiers))) {
		/* There is no free entry in PmNotifiers array, report error */
		Status = XST_FAILURE;
		goto done;
	}

	/*
	 * Check if Node Class is EVENT and enable error action.
	 */
	if ((u32)XPM_NODECLASS_EVENT == NODECLASS(NodeId)) {
		Status = XPlmi_EmSetAction(NodeId, Event, XPLMI_EM_ACTION_CUSTOM,
					   XPmNotifier_Event);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (EmptyIdx != ARRAY_SIZE(PmNotifiers)) {
		/* Add new entry in empty place if no notifier found for given pair */
		PmNotifiers[EmptyIdx].Subsystem = Subsystem;
		PmNotifiers[EmptyIdx].NodeId = NodeId;
		PmNotifiers[EmptyIdx].IpiMask = IpiMask;
		PmNotifiers[EmptyIdx].PendEvent = 0U;
		Idx = EmptyIdx;
	}

	/* Update event and wake mask for given entry */
	PmNotifiers[Idx].EventMask |= Event;
	if (0U != Wake) {
		/* Wake subsystem for this event */
		PmNotifiers[Idx].WakeMask |= Event;
	}

	if ((u8)EVENT_CPU_IDLE_FORCE_PWRDWN == Event) {
		XPm_Core* const Core = (XPm_Core *)XPmDevice_GetById(NodeId);
		if (NULL == Core) {
			goto done;
		}

		Core->IsCoreIdleSupported = 1U;
		Subsystem->Flags |= (u8)SUBSYSTEM_IDLE_SUPPORTED;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus XPmNotifier_GetNotifyCbData(const u32 Idx, u32 *Payload)
{
	XStatus Status = XST_FAILURE;
	const XPmNotifier *Notifier = NULL;
	const XPm_Device *Device = NULL;
	const XPm_Power *Power = NULL;

	Notifier = &PmNotifiers[Idx];
	Payload[0] = (u32)PM_NOTIFY_CB;
	Payload[1] = Notifier->NodeId;

	switch (NODECLASS(Notifier->NodeId)) {
	case (u32)XPM_NODECLASS_EVENT:
		Status = XST_SUCCESS;
		Payload[2] = Notifier->PendEvent;
		Payload[3] = 0U;
		break;
	case (u32)XPM_NODECLASS_DEVICE:
		Device = XPmDevice_GetById(Notifier->NodeId);
		if (NULL == Device) {
			goto done;
		}
		if ((u32)EVENT_STATE_CHANGE ==
		    ((u32)EVENT_STATE_CHANGE & Notifier->PendEvent)) {
			Payload[2] = (u32)EVENT_STATE_CHANGE;
		} else if ((u32)EVENT_CPU_IDLE_FORCE_PWRDWN ==
			   ((u32)EVENT_CPU_IDLE_FORCE_PWRDWN &
			   Notifier->PendEvent)) {
			Payload[2] = (u32)EVENT_CPU_IDLE_FORCE_PWRDWN;
		} else {
			Payload[2] = (u32)EVENT_ZERO_USERS;
		}
		Payload[3] = Device->Node.State;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODECLASS_POWER:
		Power = XPmPower_GetById(Notifier->NodeId);
		if (NULL == Power) {
			goto done;
		}
		if ((u32)EVENT_STATE_CHANGE ==
		    ((u32)EVENT_STATE_CHANGE & Notifier->PendEvent)) {
			Payload[2] = (u32)EVENT_STATE_CHANGE;
		} else {
			Payload[2] = (u32)EVENT_ZERO_USERS;
		}
		Payload[3] = Power->Node.State;
		Status = XST_SUCCESS;
		break;
	default:
		PmErr("Unsupported Node Class: %d\r\n", NODECLASS(Notifier->NodeId));
		break;
	}
done:
	return Status;
}

static int XPmNotifier_SchedulerTask(void *Arg)
{
	(void)Arg;
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};
	XStatus IpiAck;
	u32 Index = 0U;
	u32 Event;
	u32 PendEvent = (u32)NOT_PRESENT;
	XPmNotifier* Notifier = NULL;
	XPm_Subsystem *SubSystem = PmSubsystems; /* Head of SubSystem list */

	/* Search for the pending suspend callback */
	while (NULL != SubSystem) {
		if (0U != SubSystem->PendCb.Reason) {
			PendEvent = (u32)PRESENT;
			IpiAck = XPm_IpiPollForAck(SubSystem->IpiMask,
						   XPM_NOTIFY_TIMEOUTCOUNT);
			if ((XST_SUCCESS == IpiAck) &&
			    ((u8)ONLINE == SubSystem->State)) {
				Payload[0] = (u32)PM_INIT_SUSPEND_CB;
				Payload[1] = SubSystem->PendCb.Reason;
				Payload[2] = SubSystem->PendCb.Latency;
				Payload[3] = SubSystem->PendCb.State;

				(*PmRequestCb)(SubSystem->IpiMask,
					       (u32)PM_INIT_SUSPEND_CB,
					       Payload);

				SubSystem->PendCb.Reason = 0U;
			}
			if ((u8)ONLINE != SubSystem->State) {
				SubSystem->PendCb.Reason = 0U;
			}
		}
		SubSystem = SubSystem->NextSubsystem;
	}

	/* scan for pending events */
	for (Index = 0; Index < ARRAY_SIZE(PmNotifiers); Index++) {
		/* Search for the pending Event */
		if (0U != PmNotifiers[Index].PendEvent) {
			PendEvent = (u32)PRESENT;
			Notifier = &PmNotifiers[Index];
			Status = XPmNotifier_GetNotifyCbData(Index, Payload);
			if (XST_SUCCESS != Status) {
				Notifier->PendEvent = 0U;
				continue;
			}
			IpiAck = XPm_IpiPollForAck(Notifier->IpiMask,
						   XPM_NOTIFY_TIMEOUTCOUNT);
			if (XST_SUCCESS == IpiAck) {
				Event = Payload[2];
				if (((u8)ONLINE == Notifier->Subsystem->State) ||
				    ((u8)PENDING_POWER_OFF ==  Notifier->Subsystem->State) ||
				    ((u8)PENDING_RESTART == Notifier->Subsystem->State) ||
				    (0U != (Event & Notifier->WakeMask))) {
					(*PmRequestCb)(Notifier->IpiMask,
						       (u32)PM_NOTIFY_CB,
						       Payload);
					Notifier->PendEvent &= ~Event;
				} else {
					Notifier->PendEvent &= ~Event;
				}
			}
		}
	}

	/*
	 * Remove the scheduler task when the loops is in the beginning of the
	 * function start at 0 and don't find any event.
	 */
	if ((u32)NOT_PRESENT == PendEvent) {
		Status = XPlmi_SchedulerRemoveTask(XPLMI_MODULE_XILPM_ID,
						   XPmNotifier_SchedulerTask,
						   XILPM_NOTIFIER_INTERVAL,
						   NULL);
		if (Status != XST_SUCCESS) {
			PmErr("[%s] Failed to remove task\r\n", __func__);
			goto done;
		}
		SchedulerTask = (u32)NOT_PRESENT;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus XPmNotifier_AddSuspEvent(const u32 IpiMask, const u32 *Payload)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = NULL;
	u32 SubsystemId;

	SubsystemId = XPmSubsystem_GetSubSysIdByIpiMask(IpiMask);
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Store pending suspend callback entry in empty subsystem */
	Subsystem->PendCb.Reason = Payload[1];
	Subsystem->PendCb.Latency = Payload[2];
	Subsystem->PendCb.State = Payload[3];

	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus XPmNotifier_AddPendingEvent(const u32 IpiMask, const u32 *Payload)
{
	XStatus Status = XST_FAILURE;
	XPmNotifier* Notifier = NULL;
	u32 CbType;
	u32 NodeId;
	u32 Event;
	u32 Idx;

	if (NULL == Payload) {
		goto done;
	}

	CbType = Payload[0];

	switch (CbType) {
	case (u32)PM_INIT_SUSPEND_CB:
		Status = XPmNotifier_AddSuspEvent(IpiMask, Payload);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case (u32)PM_NOTIFY_CB:
		NodeId = Payload[1];
		Event = Payload[2];
		for (Idx = 0U; Idx < ARRAY_SIZE(PmNotifiers); Idx++) {
			/* Search for the given NodeId */
			if (NodeId == PmNotifiers[Idx].NodeId) {
				break;
			}
		}
		if (Idx >= ARRAY_SIZE(PmNotifiers)) {
			goto done;
		}
		Notifier = &PmNotifiers[Idx];
		Notifier->PendEvent |= Event;
		Status = XST_SUCCESS;
		break;
	default:
		PmErr("Invalid callback type %d\r\n", CbType);
		break;
	}

	if (XST_SUCCESS != Status) {
		goto done;
	}
	if ((u32)NOT_PRESENT == SchedulerTask) {
		Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_XILPM_ID,
						XPmNotifier_SchedulerTask, NULL,
						XILPM_NOTIFIER_INTERVAL,
						XPLM_TASK_PRIORITY_0,
						NULL, XPLMI_PERIODIC_TASK);
		if (Status != XST_SUCCESS) {
			PmErr("[%s] Failed to create task\r\n",__func__);
			goto done;
		}
		SchedulerTask = (u32)PRESENT;
	}
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Notify target only if IPI acked for previous request else add as
 * 	   pending event.
 *
 * @param  IpiMask    IpiMask of the target
 * @param  Payload    Payload with callback data
 *
 ****************************************************************************/
void XPmNotifier_NotifyTarget(u32 IpiMask, u32 *Payload)
{
	XStatus IpiAck;
	u32 CbType = Payload[0];
	u32 TaskPresent = SchedulerTask;

	IpiAck = XPm_IpiPollForAck(IpiMask, XPM_NOTIFY_TIMEOUTCOUNT);
	if ((XST_SUCCESS == IpiAck) &&
	    (NULL != PmRequestCb) &&
	    ((u32)NOT_PRESENT == TaskPresent)) {
		(*PmRequestCb)(IpiMask, CbType, Payload);
	} else if (NULL != PmRequestCb){
		(void)XPmNotifier_AddPendingEvent(IpiMask, Payload);
	} else {
		PmInfo("Invalid Call back Handler \r\n");
	}
}

/****************************************************************************/
/**
 * @brief  Unregister the notifier for given subsystem, NodeId and event
 *
 * @param  Subsystem  Subsystem which was registered for notification
 * @param  NodeId     NodeId related to event
 * @param  Event      Notification event
 *
 * @return None
 *
 ****************************************************************************/
void XPmNotifier_Unregister(const XPm_Subsystem* const Subsystem,
			    const u32 NodeId,
			    const u32 Event)
{
	u32 Idx;

	for (Idx = 0U; Idx < ARRAY_SIZE(PmNotifiers); Idx++) {
		if ((Subsystem == PmNotifiers[Idx].Subsystem) &&
		    (NodeId == PmNotifiers[Idx].NodeId)) {
			/* Entry for subsystem/NodeId pair found */
			PmNotifiers[Idx].EventMask &= ~Event;
			PmNotifiers[Idx].WakeMask &= ~Event;
			if (0U != (PmNotifiers[Idx].PendEvent & Event)) {
				PmNotifiers[Idx].PendEvent &= ~Event;
			}
			if (0U == PmNotifiers[Idx].EventMask) {
				(void)memset(&PmNotifiers[Idx], 0,
					     sizeof(XPmNotifier));
			}
			/*
			 * Check if Node Class is EVENT and disable error action.
			 */
			if ((u32)XPM_NODECLASS_EVENT == NODECLASS(NodeId)) {
				(void)XPlmi_EmDisable(NodeId, Event);
			}
			break;
		}
	}
}

/****************************************************************************/
/**
 * @brief  This function unregisters all notifiers of the given subsystem
 *
 * @param  Subsystem  Subsystem for which notifiers to be unregistered
 *
 * @return None
 *
 ****************************************************************************/
void XPmNotifier_UnregisterAll(const XPm_Subsystem* const Subsystem)
{
	u32 Idx;

	for (Idx = 0U; Idx < ARRAY_SIZE(PmNotifiers); Idx++) {
		if (Subsystem == PmNotifiers[Idx].Subsystem) {
			(void)memset(&PmNotifiers[Idx], 0, sizeof(XPmNotifier));
		}
	}
}

/****************************************************************************/
/**
 * @brief  This function triggers the notification if enabled for current
 *         NodeId and current event.
 *
 * @param  NodeId  NodeId for which event is occurred
 * @param  Event   Event type
 *
 * @return None
 *
 ****************************************************************************/
void XPmNotifier_Event(const u32 NodeId, const u32 Event)
{
	u32 Idx;
	const XPmNotifier* Notifier = NULL;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};
	const XPm_Device* Device;
	const XPm_Power *Power;
	XStatus Status = XST_FAILURE;

	for (Idx = 0U; Idx < ARRAY_SIZE(PmNotifiers); Idx++) {
		/* Search for the given NodeId */
		if ((NodeId != PmNotifiers[Idx].NodeId) ||
		    (0U == (Event & PmNotifiers[Idx].EventMask))) {
			continue; /* Match not found */
		}

		Notifier = &PmNotifiers[Idx];

		if (NULL == PmRequestCb) {
			goto done;
		}

		/* Populate the PayLoad */
		Payload[0] = (u32)PM_NOTIFY_CB;
		Payload[1] = Notifier->NodeId;
		Payload[2] = Event;

		switch (NODECLASS(NodeId)) {
		case (u32)XPM_NODECLASS_EVENT:
			/* Disable the error event. Agent will re-register for
			 * notification if needed */
			(void)XPlmi_EmDisable(NodeId, Event);
			Payload[3] = 0U;
			Status = XST_SUCCESS;
			break;
		case (u32)XPM_NODECLASS_DEVICE:
			Device = XPmDevice_GetById(NodeId);
			if (NULL != Device) {
				Payload[3] = Device->Node.State;
				Status = XST_SUCCESS;
			}
			break;
		case (u32)XPM_NODECLASS_POWER:
			Power = XPmPower_GetById(NodeId);
			if (NULL != Power) {
				Payload[3] = Power->Node.State;
				Status = XST_SUCCESS;
			}
			break;
		default:
			PmErr("Unsupported Node Class: %d\r\n", NODECLASS(NodeId));
			break;
		}
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/*
		 * If subsystem is OFFLINE then it should be notified about
		 * the event only if it requested to be woken up.
		 */
		if (((u8)OFFLINE != Notifier->Subsystem->State) ||
		    (0U != (Event & Notifier->WakeMask))) {
			XPmNotifier_NotifyTarget(Notifier->IpiMask, Payload);
		}
	}

 done:
	return;
}
