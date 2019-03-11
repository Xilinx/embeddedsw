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

#ifndef _XPM_CLIENT_API_H_
#define _XPM_CLIENT_API_H_

#include "xpm_client_common.h"
#include "xillibpm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

XStatus XPm_InitXilpm(XIpiPsu *IpiInst);
XStatus XPm_GetApiVersion(u32 *version);
XStatus XPm_RequestNode(const u32 DeviceId, const u32 Capabilities,
			const u32 QoS, const u32 Ack);
XStatus XPm_ReleaseNode(const u32 DeviceId);
XStatus XPm_SetRequirement(const u32 DeviceId, const u32 Capabilities,
			   const u32 QoS, const u32 Ack);
XStatus XPm_GetNodeStatus(const u32 DeviceId,
			  XPm_DeviceStatus *const DeviceStatus);
XStatus XPm_ResetAssert(const u32 ResetId, const u32 Action);
XStatus XPm_ResetGetStatus(const u32 ResetId, u32 *const State);
XStatus XPm_PinCtrlRequest(const u32 PinId);
XStatus XPm_PinCtrlRelease(const u32 PinId);
XStatus XPm_PinCtrlSetFunction(const u32 PinId, const u32 FunctionId);
XStatus XPm_PinCtrlGetFunction(const u32 PinId, u32 *const FunctionId);
XStatus XPm_PinCtrlSetParameter(const u32 PinId, const u32 ParamId, const u32 ParamVal);
XStatus XPm_PinCtrlGetParameter(const u32 PinId, const u32 ParamId, u32 *const ParamVal);
XStatus XPm_DevIoctl(const u32 DeviceId, const u32 IoctlId, const u32 Arg1,
		     const u32 Arg2, u32 *const Response);
XStatus XPm_ClockEnable(const u32 ClockId);
XStatus XPm_ClockDisable(const u32 ClockId);
XStatus XPm_ClockGetStatus(const u32 ClockId, u32 *const State);
XStatus XPm_ClockSetDivider(const u32 ClockId, const u32 Divider);
XStatus XPm_ClockGetDivider(const u32 ClockId, u32 *const Divider);
XStatus XPm_ClockSetParent(const u32 ClockId, const u32 ParentId);
XStatus XPm_ClockGetParent(const u32 ClockId, u32 *const ParentId);
XStatus XPm_PllSetParameter(const u32 ClockId,
			    const enum XPm_PllConfigParams ParamId,
			    const u32 Value);
XStatus XPm_PllGetParameter(const u32 ClockId,
			    const enum XPm_PllConfigParams ParamId,
			    u32 *const Value);
XStatus XPm_PllSetMode(const u32 ClockId, const u32 Value);
XStatus XPm_PllGetMode(const u32 ClockId, u32 *const Value);
XStatus XPm_SelfSuspend(const u32 DeviceId, const u32 Latency, const u8 State,
			const u64 Address);
XStatus XPm_RequestWakeUp(const u32 TargetDevId, const bool SetAddress,
			  const u64 Address, const u32 Ack);
void XPm_SuspendFinalize(void);
XStatus XPm_RequestSuspend(const u32 TargetSubsystemId, const u32 Ack,
			   const u32 Latency, const u32 State);
XStatus XPm_AbortSuspend(const enum XPmAbortReason Reason);
XStatus XPm_ForcePowerDown(const u32 TargetDevId, const u32 Ack);
XStatus XPm_SystemShutdown(const u32 Type, const u32 SubType);
XStatus XPm_SetWakeupSource(const u32 TargetDeviceId,
			    const u32 DeviceId, const u32 Enable);
XStatus XPm_Query(const u32 QueryId, const u32 Arg1, const u32 Arg2,
		  const u32 Arg3, u32 *const Data);
int XPm_SetMaxLatency(const u32 DeviceId, const u32 Latency);
int XPm_InitFinalize(void);

#ifdef __cplusplus
}
#endif

#endif /* _XPM_CLIENT_API_H_ */
