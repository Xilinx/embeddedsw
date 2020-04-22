/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_api.h"
#include "xpm_ipi.h"
#include "xpm_notifier.h"
#include "xplmi_err.h"

#define XPM_NOTIFIERS_COUNT	10U

typedef struct {
	const XPm_Subsystem* Subsystem;
	u32 NodeId;
	u32 EventMask;
	u32 WakeMask;
	u32 IpiMask;  /* TODO: Remove this when IPI mask support in CDO is available*/
} XPmNotifier;

static XPmNotifier PmNotifiers[XPM_NOTIFIERS_COUNT];

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
int XPmNotifier_Register(const XPm_Subsystem* const Subsystem,
			 const u32 NodeId,
			 const u32 Event, const u32 Wake, const u32 IpiMask)
{
	int Status = XST_FAILURE;
	u32 Idx, EmptyIdx = ARRAY_SIZE(PmNotifiers);

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

	if (EmptyIdx != ARRAY_SIZE(PmNotifiers)) {
		/* Add new entry in empty place if no notifier found for given pair */
		PmNotifiers[EmptyIdx].Subsystem = Subsystem;
		PmNotifiers[EmptyIdx].NodeId = NodeId;
		PmNotifiers[EmptyIdx].IpiMask = IpiMask;
		Idx = EmptyIdx;
	} else if (Idx >= ARRAY_SIZE(PmNotifiers)) {
		/* There is no free entry in PmNotifiers array, report error */
		Status = XST_FAILURE;
		goto done;
	} else {
		/* Required due to MISRA */
		PmDbg("[%d] Unknown else case\r\n", __LINE__);
	}

	/* Update event and wake mask for given entry */
	PmNotifiers[Idx].EventMask |= Event;
	if (0U != Wake) {
		/* Wake subsystem for this event */
		PmNotifiers[Idx].WakeMask |= Event;
	}
	/*
	 * Check if Node Class is EVENT and enable error action.
	 */
	if (XPM_NODECLASS_EVENT == NODECLASS(NodeId)) {
		Status = XPlmi_EmSetAction(NodeId, Event, XPLMI_EM_ACTION_CUSTOM,
				XPmNotifier_Event);
	}

	Status = XST_SUCCESS;

done:
	return Status;
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
			if (0U == PmNotifiers[Idx].EventMask) {
				(void)memset(&PmNotifiers[Idx], 0,
					     sizeof(XPmNotifier));
			}
			/*
			 * Check if Node Class is EVENT and disable error action.
			 */
			if (XPM_NODECLASS_EVENT == NODECLASS(NodeId)) {
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
	XPmNotifier* Notifier = NULL;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};
	XPm_Device* Device;
	int Status = XST_FAILURE;

	for (Idx = 0U; Idx < ARRAY_SIZE(PmNotifiers); Idx++) {
		/* Search for the given NodeId */
		if (NodeId != PmNotifiers[Idx].NodeId) {
			continue;
		}
		/*
		 * NodeId is matching, check for event
		 * Event 0 is valid for Node Class EVENT.
		 */
		if ((XPM_NODECLASS_EVENT != NODECLASS(NodeId)) &&
			(0U == (Event & PmNotifiers[Idx].EventMask))) {
			continue;
		}

		Notifier = &PmNotifiers[Idx];
		break;
	}

	if ((NULL == Notifier) || (NULL == PmRequestCb)) {
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
		if (NULL == Device) {
			goto done;
		}
		Payload[3] = Device->Node.State;
		Status = XST_SUCCESS;
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
		(*PmRequestCb)(Notifier->IpiMask, PM_NOTIFY_CB, Payload);
	}

 done:
	return;
}
