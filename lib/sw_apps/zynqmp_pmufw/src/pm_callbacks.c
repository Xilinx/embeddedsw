/*
 * Copyright (C) 2014 - 2019 Xilinx, Inc.  All rights reserved.
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
	(void)XPfw_IpiTrigger( master->ipiMask);
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
	(void)XPfw_IpiTrigger( master->ipiMask);
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
	(void)XPfw_IpiTrigger( master->ipiMask);
}

#endif
