/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * Implementation of notifications and event handling within
 * power management.
 *********************************************************************/

#include "pm_notifier.h"
#include <string.h>
#include "pm_callbacks.h"

/*
 * Maximum number of notifier master/node pairs supported by PM -
 * determines size of PmNotifiers array.
 */
#define PM_NOTIFIERS_COUNT	10U

/**
 * PmNotifier - Encapsulates all info related to master/node notification
 * @master      Master to be notified about the event
 * @node        Pointer to the node which is subject of notification
 * @eventMask   Event mask of the event that triggers notification to be sent
 * @wakeMask    Mask specifyng whether the master should be woken up if it is
 *              sleeping at the moment when event happens. Bitfield matches
 *              eventMask (if a bitfield for event is 0 - no wake; otherwise,
 *              if event's bitfield in wakeMask is set the master is woken-up
 *              when the event occurs)
 */
typedef struct {
	const PmMaster* master;
	const PmNode* node;
	u32 eventMask;
	u32 wakeMask;
} PmNotifier;

static PmNotifier pmNotifiers[PM_NOTIFIERS_COUNT];

/**
 * PmRegisterEvent() - Register one event in pmNotifiers array's entry
 * @entry       Valid index in pmNotifiers array
 * @event       Event to be registered for
 * @wake        Flag specifying whether the master should be woken up to
 *              handle event
 *
 * @note        Master and node pointers for the entry are set by the function
 *              calling this.
 */
static void PmRegisterEvent(const u32 entry, const u32 event, const u32 wake)
{
	pmNotifiers[entry].eventMask |= event;
	if (0U != wake) {
		/* Wake master for this event (event mask matches wake mask) */
		pmNotifiers[entry].wakeMask |= event;
	}
}

/**
 * PmNotifierRegister() - Register notifier for given master, node and event
 * @mst         Master to be notified
 * @nd          Node related to event
 * @event       Event to be notified about
 * @wake        Flag specifying whether the master should be woken upin order
 *              to be notified (if it is sleeping when the event happens)
 *
 * @return      Status of registration
 *              - XST_FAILURE - notifier is not registered as all entries in
 *                pmNotifiers array are occupied
 *              - XST_SUCCESS - notifier is successfully registered
 */
s32 PmNotifierRegister(const PmMaster* const mst, const PmNode* const nd,
		       const u32 event, const u32 wake)
{
	s32 status = XST_FAILURE;
	u32 i, empty = ARRAY_SIZE(pmNotifiers); /* init just to exceed max */

	for (i = 0U; i < ARRAY_SIZE(pmNotifiers); i++) {
		if (NULL == pmNotifiers[i].master) {
			/* Empty entry found in pmNotifiers array */
			if (ARRAY_SIZE(pmNotifiers) == empty) {
				/* Remember only first found empty entry */
				empty = i;
				status = XST_SUCCESS;
			}
			continue;
		}
		if ((mst == pmNotifiers[i].master) &&
		    (nd == pmNotifiers[i].node)) {
			/* Reuse existing entry for master/node pair */
			PmRegisterEvent(i, event, wake);
			/* Drop empty value - done with registration */
			empty = ARRAY_SIZE(pmNotifiers);
			status = XST_SUCCESS;
			break;
		}
	}
	if (XST_SUCCESS != status) {
		/* There is no free entry in pmNotifiers array, report error */
		goto done;
	}
	if (ARRAY_SIZE(pmNotifiers) != empty) {
		/* Empty entry should be filled with registration info */
		pmNotifiers[empty].master = mst;
		pmNotifiers[empty].node = nd;
		PmRegisterEvent(empty, event, wake);
	}

done:
	return status;
}

/**
 * PmUnregisterEvent() - Unregister event in pmNotifiers' array entry
 * @entry       Valid index in pmNotifiers array
 * @event       Event to be unregistered
 *
 * @note        This function also clears master/node pointer values if no other
 *              event is left registered for the entry, so the entry can be used
 *              for other pairs.
 */
static void PmUnregisterEvent(const u32 entry, const u32 event)
{
	pmNotifiers[entry].eventMask &= ~event;
	pmNotifiers[entry].wakeMask &= ~event;
	if (0U == pmNotifiers[entry].eventMask) {
		(void)memset(&pmNotifiers[entry], (s32)0U, sizeof(PmNotifier));
	}
}

/**
 * PmNotifierUnregister() - Unregister notifier for given master, node and event
 * @mst         Master which was registered for notification
 * @nd          Node related to event
 * @event       Notification event
 */
void PmNotifierUnregister(const PmMaster* const mst, const PmNode* const nd,
			  const u32 event)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmNotifiers); i++) {
		if ((mst == pmNotifiers[i].master) &&
		    (nd == pmNotifiers[i].node)) {
			/* Entry for master/node pair found */
			PmUnregisterEvent(i, event);
			break;
		}
	}
}

/**
 * PmNotifierUnregisterAll() - Unregister all notifiers of the given master
 * @mst Master whose notifiers should be unregistered
 */
void PmNotifierUnregisterAll(const PmMaster* const mst)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmNotifiers); i++) {
		if (mst == pmNotifiers[i].master) {
			(void)memset(&pmNotifiers[i], (s32)0U, sizeof(PmNotifier));
		}
	}
}

/**
 * PmNotifierProcessEvent() - Process event for the registration
 * @nt          Pointer to registered notifier
 * @event       Event that occurred in the system
 */
static void PmNotifierProcessEvent(const PmNotifier* const nt,
				   const u32 event)
{
	if ((false == PmMasterIsActive(nt->master)) &&
	    (0U == (event & nt->wakeMask))) {
		/*
		 * If master has no active processor it should be notified about
		 * the event only if it requested to be woken up.
		 */
		goto done;
	}

	PmNotifyCb(nt->master, nt->node->nodeId, event, nt->node->currState);

done:
	return;
}

/**
 * PmNotifierEvent() - Called to trigger the notification framework to check
 *              whether the notification callback should be sent for given
 *              arguments
 * @nd          Node regarding which the event is generated
 * @event       Event that occurred in the system (check for its PU recipients)
 */
void PmNotifierEvent(const PmNode* const nd, const u32 event)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmNotifiers); i++) {
		/* Search for the given node */
		if (nd != pmNotifiers[i].node) {
			continue;
		}
		/* Node is matching, check for event */
		if (0U == (event & pmNotifiers[i].eventMask)) {
			continue;
		}
		PmNotifierProcessEvent(&pmNotifiers[i], event);
	}
}

#endif
