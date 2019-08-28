/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xpm_api.h"
#include "xpm_ipi.h"
#include "xpm_notifier.h"

#define XPM_NOTIFIERS_COUNT	10U

typedef struct {
	const XPm_Subsystem* Subsystem;
	const XPm_Device* Device;
	u32 EventMask;
	u32 WakeMask;
	u32 IpiMask;  /* TODO: Remove this when IPI mask support in CDO is available*/
} XPmNotifier;

static XPmNotifier PmNotifiers[XPM_NOTIFIERS_COUNT];
extern void (* PmRequestCb)(u32 SubsystemId, const u32 EventId, u32 *Payload);

/****************************************************************************/
/**
 * @brief  Register the notifier for given subsystem, device and event
 *
 * @param  Subsystem  Subsystem to be notified
 * @param  Device     Device related to event
 * @param  Event      Event to be notified about
 * @param  Wake       Flag specifying whether the subsystem should be woken
 *                    upon event notification
 *
 * @return None
 *
 ****************************************************************************/
int XPmNotifier_Register(const XPm_Subsystem* const Subsystem,
			 const XPm_Device* const Device,
			 const u32 Event, const u32 Wake, const u32 IpiMask)
{
	int Status = XST_SUCCESS;
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
		    (Device == PmNotifiers[Idx].Device)) {
			/* Drop empty index - existing entry found */
			EmptyIdx = ARRAY_SIZE(PmNotifiers);
			break;
		}
	}

	if (EmptyIdx != ARRAY_SIZE(PmNotifiers)) {
		/* Add new entry in empty place if no notifier found for given pair */
		PmNotifiers[EmptyIdx].Subsystem = Subsystem;
		PmNotifiers[EmptyIdx].Device = Device;
		PmNotifiers[EmptyIdx].IpiMask = IpiMask;
		Idx = EmptyIdx;
	} else if (Idx >= ARRAY_SIZE(PmNotifiers)) {
		/* There is no free entry in PmNotifiers array, report error */
		Status = XST_FAILURE;
		goto done;
	}

	/* Update event and wake mask for given entry */
	PmNotifiers[Idx].EventMask |= Event;
	if (0U != Wake) {
		/* Wake subsystem for this event */
		PmNotifiers[Idx].WakeMask |= Event;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Unregister the notifier for given subsystem, device and event
 *
 * @param  Subsystem  Subsystem which was registered for notification
 * @param  Device     Device related to event
 * @param  Event      Notification event
 *
 * @return None
 *
 ****************************************************************************/
void XPmNotifier_Unregister(const XPm_Subsystem* const Subsystem,
			    const XPm_Device* const Device,
			    const u32 Event)
{
	u32 Idx;

	for (Idx = 0U; Idx < ARRAY_SIZE(PmNotifiers); Idx++) {
		if ((Subsystem == PmNotifiers[Idx].Subsystem) &&
		    (Device == PmNotifiers[Idx].Device)) {
			/* Entry for subsystem/device pair found */
			PmNotifiers[Idx].EventMask &= ~Event;
			PmNotifiers[Idx].WakeMask &= ~Event;
			if (0U == PmNotifiers[Idx].EventMask) {
				(void)memset(&PmNotifiers[Idx], 0U,
					     sizeof(XPmNotifier));
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
			(void)memset(&PmNotifiers[Idx], 0U, sizeof(XPmNotifier));
		}
	}
}

/****************************************************************************/
/**
 * @brief  This function triggers the notification if enabled for current
 *         device and current event.
 *
 * @param  Device  Device for which event is occured
 * @param  Event   Event type
 *
 * @return None
 *
 ****************************************************************************/
void XPmNotifier_Event(const XPm_Device* const Device, const u32 Event)
{
	u32 Idx;
	XPmNotifier* Notifier = NULL;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};

	for (Idx = 0U; Idx < ARRAY_SIZE(PmNotifiers); Idx++) {
		/* Search for the given device */
		if (Device != PmNotifiers[Idx].Device) {
			continue;
		}
		/* Device is matching, check for event */
		if (0U == (Event & PmNotifiers[Idx].EventMask)) {
			continue;
		}

		Notifier = &PmNotifiers[Idx];
		break;
	}

	if ((NULL == Notifier) || (NULL == PmRequestCb)) {
		return;
	}

	/*
	 * If subsystem is OFFLINE then it should be notified about
	 * the event only if it requested to be woken up.
	 */
	if ((OFFLINE != Notifier->Subsystem->State) ||
	    (0U != (Event & Notifier->WakeMask))) {
		Payload[0] = XPM_NOTIFY_CB;
		Payload[1] = Notifier->Device->Node.Id;
		Payload[2] = Event;
		Payload[3] = Notifier->Device->Node.State;
		(*PmRequestCb)(Notifier->IpiMask, XPM_NOTIFY_CB, Payload);
	}
}
