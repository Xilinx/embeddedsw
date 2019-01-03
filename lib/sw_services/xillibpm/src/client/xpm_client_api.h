/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

XStatus XPmClient_InitXillibpm(XIpiPsu *IpiInst);
XStatus XPmClient_GetApiVersion(u32 *version);
XStatus XPmClient_RequestDevice(const u32 TargetSubsystemId, const u32 DeviceId,
				const u32 Capabilities, const u32 Latency,
				const u32 QoS);
XStatus XPmClient_ReleaseDevice(const u32 DeviceId);
XStatus XPmClient_SetRequirement(const u32 DeviceId, const u32 Capabilities,
				 const u32 Latency, const u32 QoS);
XStatus XPmClient_GetDeviceStatus(const u32 DeviceId,
				  XPm_DeviceStatus *const DeviceStatus);
XStatus XPmClient_ResetAssert(const u32 ResetId, const u32 Action);
XStatus XPmClient_ResetGetStatus(const u32 ResetId, u32 *const State);
XStatus XPmClient_PinCtrlRequest(const u32 PinId);
XStatus XPmClient_PinCtrlRelease(const u32 PinId);
XStatus XPmClient_SetPinFunction(const u32 PinId, const u32 FunctionId);
XStatus XPmClient_GetPinFunction(const u32 PinId, u32 *const FunctionId);
XStatus XPmClient_SetPinParameter(const u32 PinId, const u32 ParamId, const u32 ParamVal);
XStatus XPmClient_GetPinParameter(const u32 PinId, const u32 ParamId, u32 *const ParamVal);
XStatus XPmClient_ClockEnable(const u32 ClockId);
XStatus XPmClient_ClockDisable(const u32 ClockId);
XStatus XPmClient_GetClockState(const u32 ClockId, u32 *const State);
XStatus XPmClient_SetClockDivider(const u32 ClockId, const u32 Divider);
XStatus XPmClient_GetClockDivider(const u32 ClockId, u32 *const Divider);
XStatus XPmClient_SetClockParent(const u32 ClockId, const u32 ParentId);
XStatus XPmClient_GetClockParent(const u32 ClockId, u32 *const ParentId);
XStatus XPmClient_SetPllParameter(const u32 ClockId,
				  const enum XPm_PllConfigParams ParamId,
				  const u32 Value);
XStatus XPmClient_GetPllParameter(const u32 ClockId,
				  const enum XPm_PllConfigParams ParamId,
				  u32 *const Value);
XStatus XPmClient_SetPllMode(const u32 ClockId, const u32 Value);
XStatus XPmClient_GetPllMode(const u32 ClockId, u32 *const Value);
XStatus XPmClient_SelfSuspend(const u32 DeviceId, const u32 Latency,
			      const u8 State, const u64 Address);
XStatus XPmClient_RequestWakeUp(const u32 TargetDevId, const bool SetAddress,
				const u64 Address);
void XPmClient_SuspendFinalize(void);
XStatus XPmClient_RequestSuspend(const u32 TargetSubsystemId, const u32 Latency,
				 const u32 State);
XStatus XPmClient_AbortSuspend(const enum XPmAbortReason Reason);
XStatus XPmClient_ForcePowerDown(const u32 TargetDevId, const u32 Ack);

#endif /* _XPM_CLIENT_API_H_ */
