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

#define MAX_BASEADDR_LEN	3

/* Global general storage register base address */
#define GGS_BASEADDR	(0xF1110030U)
#define GGS_NUM_REGS	(4U)

#define GGS_4_OFFSET	(0x10U)

/* Persistent global general storage register base address */
#define PGGS_BASEADDR	(0xF1110050U)
#define PGGS_NUM_REGS	(4U)

/* Tap delay bypass */
#define TAPDLY_BYPASS_OFFSET			(0x0000003CU)
#define XPM_TAP_DELAY_MASK			(0x4U)

/* SD DLL control */
#define SD0_CTRL_OFFSET				(0x00000404U)
#define SD1_CTRL_OFFSET				(0x00000484U)
#define XPM_SD_DLL_RST_MASK			(0x4U)

/* SD ITAPDLY */
#define ITAPDLY_OFFSET				(0x0000F0F8U)
#define XPM_SD_ITAPCHGWIN_MASK			(0x200U)
#define XPM_SD_ITAPDLYENA_MASK			(0x100U)
#define XPM_SD_ITAPDLYSEL_MASK			(0xFFU)

/* SD OTAPDLY */
#define OTAPDLY_OFFSET				(0x0000F0FCU)
#define XPM_SD_OTAPDLYENA_MASK			(0x40U)
#define XPM_SD_OTAPDLYSEL_MASK			(0x3FU)

/* Probe Counter Register related macros */
#define PROBE_COUNTER_REQ_TYPE_SHIFT		(16U)
#define PROBE_COUNTER_REQ_TYPE_MASK		(0xFFU)
#define PROBE_COUNTER_TYPE_SHIFT		(8U)
#define PROBE_COUNTER_TYPE_MASK			(0xFFU)
#define PROBE_COUNTER_IDX_SHIFT			(0U)
#define PROBE_COUNTER_IDX_MASK			(0xFFU)

#define PROBE_COUNTER_CPU_R5_MAX_IDX		(9U)
#define PROBE_COUNTER_LPD_MAX_IDX		(5U)
#define PROBE_COUNTER_FPD_MAX_IDX		(15U)

#define PROBE_COUNTER_CPU_R5_MAX_REQ_TYPE	(3U)
#define PROBE_COUNTER_LPD_MAX_REQ_TYPE		(7U)
#define PROBE_COUNTER_FPD_MAX_REQ_TYPE		(3U)

/* Extern Variable and Function */
extern int XLoader_RestartImage(u32 SubsystemId);

XStatus XPm_Init(void (* const RequestCb)(u32 SubsystemId, const u32 EventId, u32 *Payload));

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
                        const u32 IoctlId,
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
int XPm_DispatchWakeHandler(void *DeviceIdx);
XStatus XPm_HookAfterPlmCdo(void);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_API_H_ */
