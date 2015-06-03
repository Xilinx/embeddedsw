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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef _PM_API_SYS_H_
#define _PM_API_SYS_H_

#include "pm_defs.h"
#include "pm_common.h"

enum XPmBootStatus XPm_GetBootStatus();

/* System-level API function declarations */
enum XPmStatus XPm_RequestSuspend(const enum XPmNodeId node,
				  const enum XPmRequestAck ack,
				  const u32 latency,
				  const u8 state);

enum XPmStatus XPm_SelfSuspend(const enum XPmNodeId node,
				   const u32 latency,
				   const u8 state);

enum XPmStatus XPm_ForcePowerDown(const enum XPmNodeId node,
				      const enum XPmRequestAck ack);

enum XPmStatus XPm_AbortSuspend(const enum XPmAbortReason reason);

enum XPmStatus XPm_RequestWakeUp(const enum XPmNodeId node,
				 const enum XPmRequestAck ack);

enum XPmStatus XPm_SetWakeUpSource(const enum XPmNodeId target,
					const enum XPmNodeId wkup_node,
					const u8 enable);

enum XPmStatus XPm_SystemShutdown(const u8 restart);

/* Callback API function */
/**
 * pm_init_suspend - Init suspend callback arguments (save for custom handling)
 * @received	Has init suspend callback been received/handled
 * @reason	Reason of initializing suspend
 * @latency	Maximum allowed latency
 * @timeout	Period of time the client has to response
 */
struct pm_init_suspend {
	volatile bool received;
	enum XPmSuspendReason reason;
	u32 latency;
	u32 state;
	u32 timeout;
};

/**
 * pm_acknowledge - Acknowledge callback arguments (save for custom handling)
 * @received	Has acknowledge argument been received
 * @node	Node argument about which the acknowledge is
 * @status	Acknowledged status
 * @opp		Operating point of node in question
 */
struct pm_acknowledge {
	volatile bool received;
	enum XPmNodeId node;
	enum XPmStatus status;
	u32 opp;
};

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern struct pm_init_suspend pm_susp;
extern struct pm_acknowledge pm_ack;

void XPm_InitSuspendCb(const enum XPmSuspendReason reason,
		       const u32 latency,
		       const u32 state,
		       const u32 timeout);

void XPm_AcknowledgeCb(const enum XPmNodeId node,
		       const enum XPmStatus status,
		       const u32 oppoint);

void XPm_NotifyCb(const enum XPmNodeId node,
		  const u32 event,
		  const u32 oppoint);

/* API functions for managing PM Slaves */
enum XPmStatus XPm_RequestNode(const enum XPmNodeId node,
			       const u32 capabilities,
			       const u32 qos,
			       const enum XPmRequestAck ack);
enum XPmStatus XPm_ReleaseNode(const enum XPmNodeId node,
				   const u32 latency);
enum XPmStatus XPm_SetRequirement(const enum XPmNodeId node,
				      const u32 capabilities,
				      const u32 qos,
				      const enum XPmRequestAck ack);
enum XPmStatus XPm_SetMaxLatency(const enum XPmNodeId node,
				      const u32 latency);

/* Miscellaneous API functions */
enum XPmStatus XPm_GetApiVersion(u32 *version);

enum XPmStatus XPm_GetNodeStatus(const enum XPmNodeId node);

/* Direct-Control API functions */
enum XPmStatus XPm_MmioWrite(const u32 address, const u32 mask,
			     const u32 value);

enum XPmStatus XPm_MmioRead(const u32 address, u32 *const value);

#endif /* _PM_API_SYS_H_ */
