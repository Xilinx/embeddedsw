/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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

#ifndef XILLIBPM_API_H_
#define XILLIBPM_API_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xillibpm_defs.h"

#define XIpiPsu	u32
#define MAX_BASEADDR_LEN	3

/* Global general storage register base address */
#define GGS_BASEADDR	(0xFFC90030U)
#define GGS_NUM_REGS	(4)

/* Persistent global general storage register base address */
#define PGGS_BASEADDR	(0xFFD90050U)
#define PGGS_NUM_REGS	(4)

/* Tap delay bypass */
#define TAPDLY_BYPASS_OFFSET			(0x0000003CU)
#define XPM_TAP_DELAY_MASK			(0x4U)

/* Suspend reasons */
#define SUSPEND_REASON_SUBSYSTEM_REQ		(201U)
#define SUSPEND_REASON_ALERT			(202U)
#define SUSPEND_REASON_SYS_SHUTDOWN 		(203U)

/* PM API callback ids */
#define XPM_INIT_SUSPEND_CB			(30U)
#define XPM_ACKNOWLEDGE_CB			(31U)
#define XPM_NOTIFY_CB				(32U)

XStatus XPm_Init(void (* const RequestCb)(u32 SubsystemId, const u32 EventId, u32 *Payload));

XStatus XPm_GetApiVersion(u32 *Version);

enum XPmBootStatus XPm_GetBootStatus();

XStatus XPm_CreateSubsystem(void (*const NotifyCb)(u32 SubsystemId,
						   const u32 EventId),
			    u32 *SubsystemId);

XStatus XPm_DestroySubsystem(u32 SubsystemId);

XStatus XPm_RequestWakeUp(u32 SubsystemId, const u32 DeviceId,
			const u32 SetAddress,
			const u64 Address,
			const u32 Ack);

XStatus XPm_ForcePowerdown(u32 SubsystemId,
                             const u32 NodeId,
                             const u32 Ack);
XStatus XPm_SystemShutdown(u32 SubsystemId, const u32 Type,
                             const u32 SubType);

XStatus XPm_SetWakeupSource(const u32 SubsystemId,
			    const u32 TargetNodeId,
			    const u32 SourceNodeId,
			    const u32 Enable);

XStatus XPm_RequestDevice(const u32 TargetSubsystemId, const u32 DeviceId,
			  const u32 Capabilities, const u32 QoS, const u32 Ack);

XStatus XPm_ReleaseDevice(const u32 SubsystemId, const u32 DeviceId);

XStatus XPm_SetRequirement(const u32 SubsystemId, const u32 DeviceId,
			   const u32 Capabilities, const u32 QoS,
			   const u32 Ack);

XStatus XPm_GetDeviceStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus);

XStatus XPm_Query(const u32 Qid, const u32 Arg1, const u32 Arg2,
		  const u32 Arg3, u32 *const Output);

XStatus XPm_SetClockState(const u32 SubsystemId, const u32 ClockId, const u32 Enable);

XStatus XPm_GetClockState(const u32 ClockId, u32 *const State);

XStatus XPm_SetClockDivider(const u32 SubsystemId, const u32 ClockId, const u32 Divider);

XStatus XPm_GetClockDivider(const u32 ClockId, u32 *const Divider);

XStatus XPm_SetClockParent(const u32 SubsystemId, const u32 ClockId, const u32 ParentId);

XStatus XPm_GetClockParent(const u32 ClockId, u32 *const ParentId);

XStatus XPm_SetPllParameter(const u32 SubsystemId, const u32 ClockId, const u32 ParamId, const u32 Value);

XStatus XPm_GetPllParameter(const u32 ClockId, const u32 ParamId, u32 *const Value);

XStatus XPm_SetPllMode(const u32 SubsystemId, const u32 ClockId, const u32 Value);

XStatus XPm_GetPllMode(const u32 ClockId, u32 *const Value);

XStatus XPm_SetResetState(const u32 SubsystemId, const u32 DeviceId, const u32 Reset);

XStatus XPm_GetResetState(const u32 DeviceId, u32 *const Reset);

XStatus XPm_SetPinFunction(const u32 SubsystemId, const u32 PinId, const u32 FunctionId);

XStatus XPm_GetPinFunction(const u32 PinId, u32 *const DeviceId);

XStatus XPm_SetPinParameter(const u32 SubsystemId, const u32 PinId,
			const u32 ParamId,
			const u32 ParamVal);

XStatus XPm_GetPinParameter(const u32 PinId,
			const u32 ParamId,
			u32 *const ParamVal);

XStatus XPm_PinCtrlRequest(const u32 SubsystemId, const u32 PinId);

XStatus XPm_PinCtrlRelease(const u32 SubsystemId, const u32 PinId);

XStatus XPm_DevIoctl(const u32 SubsystemId, const u32 DeviceId,
                        const u32 IoctlId,
                        const u32 Arg1,
                        const u32 Arg2,u32 *const Response);

XStatus XPm_DescribeNodes(u32 NumArgs);
XStatus XPm_AddNodeParent(u32 *Args, u32 NumArgs);
XStatus XPm_AddNodeName(u32 *Args, u32 NumArgs);
XStatus XPm_AddNode(u32 *Args, u32 NumArgs);
XStatus XPm_SelfSuspend(const u32 SubsystemId, const u32 DeviceId,
			const u32 Latency, const u8 State,
			u32 AddrLow, u32 AddrHigh);
XStatus XPm_AbortSuspend(const u32 SubsystemId, const u32 Reason,
			 const u32 DeviceId);
XStatus XPm_RequestSuspend(const u32 SubsystemId, const u32 TargetSubsystemId,
			   const u32 Ack, const u32 Latency, const u32 State);
XStatus XPm_AddRequirement(const u32 SubsystemId, const u32 DeviceId);
XStatus XPm_SetCurrentSubsystem(u32 SubsystemId);

/** @} */
#endif /* XILLIBPM_API_H_ */
