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

enum XPmBootStatus XPm_GetBootStatus();

/* System-level API function declarations */
enum XPmStatus XPm_ReqSuspend(const enum XPmNodeId node,
				  const enum XPmRequestAck ack,
				  const u32 latency,
				  const u8 state);

enum XPmStatus XPm_SelfSuspend(const enum XPmNodeId node,
				   const u32 latency,
				   const u8 state);

enum XPmStatus XPm_ForcePowerDown(const enum XPmNodeId node,
				      const enum XPmRequestAck ack);

enum XPmStatus XPm_AbortSuspend(const enum XPmAbortReason reason);

enum XPmStatus XPm_ReqWakeUp(const enum XPmNodeId node,
				 const enum XPmRequestAck ack);

enum XPmStatus XPm_SetWakeUpSource(const enum XPmNodeId target,
					const enum XPmNodeId wkup_node,
					const u8 enable);

enum XPmStatus XPm_SystemShutdown(const u8 restart);

/* API functions for managing PM Slaves */
enum XPmStatus XPm_ReqNode(const enum XPmNodeId node,
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

#endif /* _PM_API_SYS_H_ */
