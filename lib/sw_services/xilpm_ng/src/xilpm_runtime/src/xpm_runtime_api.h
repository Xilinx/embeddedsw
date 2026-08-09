/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
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

XStatus XPm_RequestDevice(const u32 SubsystemId, const u32 DeviceId,
			  const u32 Capabilities, const u32 QoS, const u32 Ack,
			  const u32 CmdType);


XStatus XPm_SelfSuspend(const u32 SubsystemId, const u32 DeviceId,
			const u32 Latency, const u8 State,
			u32 AddrLow, u32 AddrHigh);


XStatus XPm_SubsystemIdleCores(const XPm_Subsystem *Subsystem);

XStatus XPm_RequestWakeUp(u32 SubsystemId, const u32 DeviceId,
			  const u32 SetAddress, const u64 Address,
			  const u32 Ack,
			  const u32 CmdType);

XStatus XPm_GetResetState(const u32 ResetId, u32 *const State);
XStatus XPm_InitFinalize(const u32 SubsystemId);
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
XStatus XPm_GetClockState(const u32 ClockId, u32 *const Enable);

#ifdef __cplusplus
}
#endif

#endif /* XPM_RUNTIME_API_H_ */
