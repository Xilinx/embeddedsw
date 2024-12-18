/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_DEVICE_H_
#define XPM_RUNTIME_DEVICE_H_
#include "xpm_device.h"
#include "xpm_subsystem.h"
#include "xpm_fsm.h"
#include "xpm_power.h"
#include "xpm_common.h"
#include "xpm_requirement.h"
#include "xpm_api.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct XPmRuntime_DeviceOps XPmRuntime_DeviceOps;
/* Device Operations */
struct XPmRuntime_DeviceOps
{
	XPm_Device *Device;
	XPm_RequirementList* Requirements;
	XPm_Fsm *Fsm; /**< Pointer to the FSM of the device */
	void* DeviceOps; /**< Pointer to the device specific operations */
};

CREATE_LIST(XPmRuntime_DeviceOps);
XStatus XPmDevice_SetRequirement(const u32 SubsystemId, const u32 DeviceId,
				 const u32 Capabilities, const u32 QoS);
u32 XPmDevice_GetUsageStatus(const XPm_Subsystem *Subsystem, const XPm_Device *Device);
XStatus XPmDevice_GetStatus(XPm_Subsystem *Subsystem,
			u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus);
XPmRuntime_DeviceOps* XPmDevice_SetDevOps_ById(u32 DeviceId);
XPmRuntime_DeviceOps* XPm_GetDevOps_ById(u32 DeviceId);

struct XPm_Reqm* XPmDevice_FindRequirement(const u32 DeviceId, const u32 SubsystemId);
u32 XPmDevice_GetSubsystemIdOfCore(const XPm_Device *Device);
XStatus XPmDevice_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId,
			    const u32 Latency);
u8 XPmDevice_IsRequestable(u32 NodeId);
XStatus XPmDevice_CheckPermissions(const XPm_Subsystem *Subsystem, u32 DeviceId);

XStatus XPmDevice_Request(const u32 SubsystemId, const u32 DeviceId,
			  const u32 Capabilities, const u32 QoS, const u32 CmdType);
XStatus XPm_CheckCapabilities(const XPm_Device *Device, u32 Caps);
XStatus XPmDevice_GetWakeupLatency(const u32 DeviceId, u32 *Latency);
XStatus XPmDevice_GetPermissions(const XPm_Device *Device, u32 *PermissionMask);
XStatus XPmCore_GetWakeupLatency(const u32 DeviceId, u32 *Latency);
XStatus XPmDevice_ChangeState(XPm_Device *Device, const u32 NextState);
#ifdef __cplusplus
}
#endif
#endif /* XPM_RUNTIME_DEVICE_H_ */
