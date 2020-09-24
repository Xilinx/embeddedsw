/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_API_H_
#define XPM_API_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_defs.h"


#ifdef __cplusplus
extern "C" {
#endif

/* Persistent global general storage register base address */
#define PGGS_BASEADDR	(0xF1110050U)

#define MAX_BASEADDR_LEN	3

/* Extern Variable and Function */
extern u32 ResetReason;

XStatus XPm_Init(void (*const RequestCb)(const u32 SubsystemId, const XPmApiCbId_t EventId, u32 *Payload),
		 int (*const RestartCb)(u32 ImageId, u32 *FuncId));

int XPm_GetChipID(u32* IDCode, u32 *Version);

XStatus XPm_GetApiVersion(u32 *Version);

XStatus XPm_AddSubsystem(u32 SubsystemId);

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

XStatus XPm_SetWakeUpSource(const u32 SubsystemId,
			    const u32 TargetNodeId,
			    const u32 SourceNodeId,
			    const u32 Enable);

XStatus XPm_RequestDevice(const u32 SubsystemId, const u32 DeviceId,
			  const u32 Capabilities, const u32 QoS, const u32 Ack);

XStatus XPm_ReleaseDevice(const u32 SubsystemId, const u32 DeviceId);

XStatus XPm_SetRequirement(const u32 SubsystemId, const u32 DeviceId,
			   const u32 Capabilities, const u32 QoS,
			   const u32 Ack);

int XPm_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId,
		      const u32 Latency);

XStatus XPm_GetDeviceStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus);

XStatus XPm_Query(const u32 Qid, const u32 Arg1, const u32 Arg2,
		  const u32 Arg3, u32 *const Output);

XStatus XPm_SetClockState(const u32 SubsystemId, const u32 ClockId, const u32 Enable);

XStatus XPm_GetClockState(const u32 ClockId, u32 *const State);

XStatus XPm_SetClockDivider(const u32 SubsystemId, const u32 ClockId, const u32 Divider);

XStatus XPm_GetClockDivider(const u32 ClockId, u32 *const Divider);

XStatus XPm_SetClockParent(const u32 SubsystemId, const u32 ClockId, const u32 ParentIdx);

XStatus XPm_GetClockParent(const u32 ClockId, u32 *const ParentIdx);

XStatus XPm_SetPllParameter(const u32 SubsystemId, const u32 ClockId, const u32 ParamId, const u32 Value);

XStatus XPm_GetPllParameter(const u32 ClockId, const u32 ParamId, u32 *const Value);

XStatus XPm_SetPllMode(const u32 SubsystemId, const u32 ClockId, const u32 Value);

XStatus XPm_GetPllMode(const u32 ClockId, u32 *const Value);

XStatus XPm_SetResetState(const u32 SubsystemId, const u32 IpiMask,
			  const u32 ResetId, const u32 Action);

XStatus XPm_GetResetState(const u32 ResetId, u32 *const State);

XStatus XPm_SetPinFunction(const u32 SubsystemId, const u32 PinId, const u32 FunctionId);

XStatus XPm_GetPinFunction(const u32 PinId, u32 *const FunctionId);

XStatus XPm_SetPinParameter(const u32 SubsystemId, const u32 PinId,
			const u32 ParamId,
			const u32 ParamVal);

XStatus XPm_GetPinParameter(const u32 PinId,
			const u32 ParamId,
			u32 *const ParamVal);

XStatus XPm_PinCtrlRequest(const u32 SubsystemId, const u32 PinId);

XStatus XPm_PinCtrlRelease(const u32 SubsystemId, const u32 PinId);

XStatus XPm_DevIoctl(const u32 SubsystemId, const u32 DeviceId,
                        const pm_ioctl_id IoctlId,
                        const u32 Arg1,
                        const u32 Arg2,u32 *const Response);
int XPm_InitFinalize(const u32 SubsystemId);

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
XStatus XPm_AddRequirement(const u32 SubsystemId, const u32 DeviceId, u32 Flags, u32 *Params, u32 NumParams);
XStatus XPm_SetCurrentSubsystem(u32 SubsystemId);
XStatus XPm_InitNode(u32 NodeId, u32 Function, u32 *Args, u32 NumArgs);
int XPm_FeatureCheck(const u32 ApiId, u32 *const Version);
XStatus XPm_IsoControl(u32 NodeId, u32 Enable);
XStatus XPm_GetOpCharacteristic(const u32 DeviceId, const u32 Type,
				u32 *Result);
int XPm_RegisterNotifier(const u32 SubsystemId, const u32 NodeId,
			 const u32 Event, const u32 Wake, const u32 Enable,
			 const u32 IpiMask);
int XPm_GicProxyWakeUp(const u32 PeriphIdx);
XStatus XPm_HookAfterPlmCdo(void);
int XPm_RestartCbWrapper(const u32 SubsystemId);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_API_H_ */
