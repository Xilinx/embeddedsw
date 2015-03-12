/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*********************************************************************
 * PM callbacks interface.
 * Used by the power management to send a message to the PM master and
 * optionally generate interrupt using IPI.
 *********************************************************************/

#include "pm_callbacks.h"
#include "pm_defs.h"
#include "pm_api.h"
#include "ipi_buffer.h"

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
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, PM_ACKNOWLEDGE_CB);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + PAYLOAD_ELEM_SIZE,
		     nodeId);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + 2 * PAYLOAD_ELEM_SIZE,
		     status);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + 3 * PAYLOAD_ELEM_SIZE,
		     oppoint);
	XPfw_Write32(IPI_PMU_0_TRIG, master->ipiTrigMask);
}

/**
 * PmNotifyCb() - notifies a master about an event occurance
 * @nodeId      Node id regarding which the event is triggered
 * @event       Event to informa master about
 * @oppoint     Optionally event is related to some operating point change
 */
void PmNotifyCb(const PmMaster* const master, const PmNodeId nodeId,
		const u32 event, const u32 oppoint)
{
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, PM_NOTIFY_CB);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + PAYLOAD_ELEM_SIZE,
		     nodeId);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + 2 * PAYLOAD_ELEM_SIZE,
		     event);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + 3 * PAYLOAD_ELEM_SIZE,
		     oppoint);
	XPfw_Write32(IPI_PMU_0_TRIG, master->ipiTrigMask);
}

/**
 * PmInitSuspendCb() - request a master to suspend itself
 * @nodeId      Node within the master to be suspended
 * @reason      The reason of initiating the suspend
 * @latency     Not supported
 * @state       State to which the master should suspend
 * @timeout     How much time the master has to respond
 */
void PmInitSuspendCb(const PmMaster* const master, const PmNodeId nodeId,
		     const u32 reason, const u32 latency, const u32 state,
		     const u32 timeout)
{
	PmDbg("%s(%s, %d, %d, %d, %d)\n", __func__, PmStrNode(nodeId), reason,
	      latency, state, timeout);

	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, PM_INIT_SUSPEND_CB);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + PAYLOAD_ELEM_SIZE,
		     nodeId);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + 2 * PAYLOAD_ELEM_SIZE,
		     reason);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + 3 * PAYLOAD_ELEM_SIZE,
		     latency);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + 4 * PAYLOAD_ELEM_SIZE,
		     state);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + 5 * PAYLOAD_ELEM_SIZE,
		     timeout);
	XPfw_Write32(IPI_PMU_0_TRIG, master->ipiTrigMask);
}
