/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

/*********************************************************************
 * Implementation of notifications and event handling within
 * power management.
 *********************************************************************/

#include "pm_notifier.h"
#include <string.h>

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
int PmNotifierRegister(const PmMaster* const mst, const PmNode* const nd,
		       const u32 event, const u32 wake)
{
	int status = XST_FAILURE;
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
		memset(&pmNotifiers[entry], 0, sizeof(PmNotifier));
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
