/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * PM callbacks interface.
 * Used by the power management to send a message to the PM master and
 * optionally generate interrupt using IPI.
 *********************************************************************/

#include "pm_callbacks.h"
#include "pm_defs.h"

#include "xpfw_ipi_manager.h"
#include "xpfw_mod_pm.h"

/**
 * PmAcknowledgeCb() - sends acknowledge via callback
 * @master      Master who is blocked and waiting for the acknowledge
 * @nodeId      Node id of the node in question
 * @status      Status of the PM operation
 * @oppoint     Operating point of the node in question
 *
 * @note        Master is not blocked waiting for this acknowledge. Master
 *              acknowledge through the IPI interrupt and registered callback.
 */
void PmAcknowledgeCb(const PmMaster* const master, const PmNodeId nodeId,
		     const u32 status, const u32 oppoint)
{
	IPI_REQUEST4(master->ipiMask, PM_ACKNOWLEDGE_CB, nodeId, status,
		     oppoint);
	if (XST_SUCCESS != XPfw_IpiTrigger(master->ipiMask)) {
		PmWarn("Error in IPI trigger\r\n");
	}
}

/**
 * PmNotifyCb() - notifies a master about an event occurrence
 * @master      Master to be notified about the event
 * @nodeId      Node id regarding which the event is triggered
 * @event       Event to informa master about
 * @oppoint     Optionally event is related to some operating point change
 */
void PmNotifyCb(const PmMaster* const master, const PmNodeId nodeId,
		const u32 event, const u32 oppoint)
{
	IPI_REQUEST4(master->ipiMask, PM_NOTIFY_CB, nodeId, event, oppoint);
	if (XST_SUCCESS != XPfw_IpiTrigger(master->ipiMask)) {
		PmWarn("Error in IPI trigger\r\n");
	}
}

/**
 * PmInitSuspendCb() - request a master to suspend itself
 * @master      Master to be asked to suspend
 * @reason      The reason of initiating the suspend
 * @latency     Not supported
 * @state       State to which the master should suspend
 * @timeout     How much time the master has to respond
 */
void PmInitSuspendCb(const PmMaster* const master, const u32 reason,
		     const u32 latency, const u32 state, const u32 timeout)
{
	PmInfo("%s> (%lu, %lu, %lu, %lu)\r\n", master->name, reason, latency,
	       state, timeout);

	IPI_REQUEST5(master->ipiMask, PM_INIT_SUSPEND_CB, reason, latency,
		     state, timeout);
	if (XST_SUCCESS != XPfw_IpiTrigger(master->ipiMask)) {
		PmWarn("Error in IPI trigger\r\n");
	}
}

#endif
