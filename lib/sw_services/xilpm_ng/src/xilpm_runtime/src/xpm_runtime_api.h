/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_API_H
#define XPM_RUNTIME_API_H

#include "xpm_device.h"
#include "xpm_mem.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEMREGN_DEVID(IDX)		NODEID((u32)XPM_NODECLASS_DEVICE, \
					(u32)XPM_NODESUBCL_DEV_MEM_REGN, \
					(u32)XPM_NODETYPE_DEV_MEM_REGN, (IDX))

int XPm_RegisterWakeUpHandlers(void);
XStatus XPmDevice_UpdateStatus(XPm_Device *Device);
XStatus XPm_RequestDevice(const u32 SubsystemId, const u32 DeviceId,
			  const u32 Capabilities, const u32 QoS, const u32 Ack,
			  const u32 CmdType);
int XPm_RestartCbWrapper(const u32 SubsystemId);
void XPm_ProcessAckReq(const u32 Ack, const u32 IpiMask, const int Status,
		       const u32 NodeId, const u32 NodeState);


XStatus XPm_SelfSuspend(const u32 SubsystemId, const u32 DeviceId,
			const u32 Latency, const u8 State,
			u32 AddrLow, u32 AddrHigh);

XStatus XPm_ForcePowerdown(u32 SubsystemId, const u32 NodeId, const u32 Ack,
			   const u32 CmdType, const u32 IpiMask);

XStatus XPm_SubsystemIdleCores(const XPm_Subsystem *Subsystem);

XStatus XPm_AbortSuspend(const u32 SubsystemId, const u32 Reason,
			 const u32 DeviceId);
XStatus XPm_RequestWakeUp(u32 SubsystemId, const u32 DeviceId,
			  const u32 SetAddress, const u64 Address,
			  const u32 Ack,
			  const u32 CmdType);

XStatus XPm_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId,
		      const u32 Latency);
XStatus XPm_SetResetState(const u32 SubsystemId, const u32 ResetId,
			  const u32 Action, const u32 CmdType);
XStatus XPm_GetResetState(const u32 ResetId, u32 *const State);
XStatus XPm_InitFinalize(const u32 SubsystemId);
XStatus XPm_GetChipID(u32* IDCode, u32 *Version);
XStatus XPmCore_ForcePwrDwn(u32 DeviceId);
XStatus XPm_PinCtrlRequest(const u32 SubsystemId, const u32 PinId);
XStatus XPm_PinCtrlRelease(const u32 SubsystemId, const u32 PinId);
XStatus XPm_GetPinFunction(const u32 PinId, u32 *const FunctionId);
XStatus XPm_SetPinFunction(const u32 SubsystemId,
	const u32 PinId, const u32 FunctionId);
XStatus XPm_GetPinParameter(const u32 PinId,
			const u32 ParamId,
			u32 * const ParamVal);
XStatus XPm_SetPinParameter(const u32 SubsystemId, const u32 PinId,
			const u32 ParamId,
			const u32 ParamVal);
XStatus XPm_DevIoctl(const u32 SubsystemId, const u32 DeviceId,
			const pm_ioctl_id  IoctlId,
			const u32 Arg1,
			const u32 Arg2,
			const u32 Arg3,
			u32 *const Response,
			const u32 CmdType);
XStatus XPm_GetPllMode(const u32 ClockId, u32 *const Value);
XStatus XPm_SetPllMode(const u32 SubsystemId, const u32 ClockId, const u32 Value);
XStatus XPm_SetPllParameter(const u32 SubsystemId, const u32 ClockId, const u32 ParamId, const u32 Value);
XStatus XPm_GetPllParameter(const u32 ClockId, const u32 ParamId, u32 *const Value);
XStatus XPm_Query(const u32 Qid, const u32 Arg1, const u32 Arg2,
		  const u32 Arg3, u32 *const Output);
XStatus XPm_SetClockState(const u32 SubsystemId, const u32 ClockId, const u32 Enable);
XStatus XPm_GetClockState(const u32 ClockId, u32 *const Enable);
XStatus XPm_SetClockDivider(const u32 SubsystemId, const u32 ClockId, const u32 Divider);
XStatus XPm_GetClockDivider(const u32 ClockId, u32 *const Divider);
XStatus XPm_SetClockParent(const u32 SubsystemId, const u32 ClockId, const u32 ParentIdx);
XStatus XPm_GetClockParent(const u32 ClockId, u32 *const ParentIdx);
XStatus XPm_AddPSMemRegnForDefaultSubsystem(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_RUNTIME_API_H_ */
