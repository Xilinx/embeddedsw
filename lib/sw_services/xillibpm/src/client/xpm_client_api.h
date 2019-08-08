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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#ifndef _XPM_CLIENT_API_H_
#define _XPM_CLIENT_API_H_

#include "xpm_client_common.h"
#include "xillibpm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * XPm_Notifier - Notifier structure registered with a callback by app
 */
typedef struct XPm_Ntfier {
	/**
	 *  Custom callback handler to be called when the notification is
	 *  received. The custom handler would execute from interrupt
	 *  context, it shall return quickly and must not block! (enables
	 *  event-driven notifications)
	 */
	void (*const callback)(struct XPm_Ntfier* const notifier);
	const u32 node; /**< Node argument (the node to receive notifications about) */
	enum XPmNotifyEvent event;	/**< Event argument (the event type to receive notifications about) */
	u32 flags;	/**< Flags */
	/**
	 *  Operating point of node in question. Contains the value updated
	 *  when the last event notification is received. User shall not
	 *  modify this value while the notifier is registered.
	 */
	volatile u32 oppoint;
	/**
	 *  How many times the notification has been received - to be used
	 *  by application (enables polling). User shall not modify this
	 *  value while the notifier is registered.
	 */
	volatile u32 received;
	/**
	 *  Pointer to next notifier in linked list. Must not be modified
	 *  while the notifier is registered. User shall not ever modify
	 *  this value.
	 */
	struct XPm_Ntfier* next;
} XPm_Notifier;

XStatus XPm_InitXilpm(XIpiPsu *IpiInst);
enum XPmBootStatus XPm_GetBootStatus(void);
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
XStatus XPm_GetOpCharacteristic(const u32 DeviceId,
				const enum XPmOpCharType Type,
				u32 *const Result);
int XPm_InitFinalize(void);

#ifdef __cplusplus
}
#endif

#endif /* _XPM_CLIENT_API_H_ */
