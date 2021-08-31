/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_API_H_
#define XPM_API_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_defs.h"
#include "xplmi_ipi.h"


#ifdef __cplusplus
extern "C" {
#endif

/* Persistent global general storage register base address */
#define PGGS_BASEADDR	(0xF1110050U)

/* Force power down timeout in us */
#define XPM_PWR_DWN_TIMEOUT	(60000U)

#define MAX_BASEADDR_LEN	3

#ifdef XPAR_XIPIPSU_0_DEVICE_ID
/* Macros for IPI responses (return values and callbacks) */
#define IPI_RESPONSE1(Mask, Arg0)						\
{										\
	u32 Response[XPLMI_CMD_RESP_SIZE] = {Arg0};				\
	if (XST_SUCCESS != XPlmi_IpiWrite(Mask, Response, XPLMI_CMD_RESP_SIZE,	\
					  XIPIPSU_BUF_TYPE_RESP)) {		\
		PmWarn("Error in IPI write response\r\n");			\
	}									\
}

#define IPI_MESSAGE4(Mask, Arg0, Arg1, Arg2, Arg3)				\
{										\
	u32 Response[XPLMI_CMD_RESP_SIZE] = {Arg0, Arg1, Arg2, Arg3};		\
	if (XST_SUCCESS != XPlmi_IpiWrite(Mask, Response, XPLMI_CMD_RESP_SIZE,	\
					  XIPIPSU_BUF_TYPE_MSG)) {		\
		PmWarn("Error in IPI write response\r\n");			\
	}									\
}
#endif

struct XPm_FrcPwrDwnReq {
	u32 AckType;
	u32 InitiatorIpiMask;
};

/* Extern Variable and Function */
extern u32 ResetReason;

XStatus XPm_Init(void (*const RequestCb)(const u32 SubsystemId, const XPmApiCbId_t EventId, u32 *Payload),
		 int (*const RestartCb)(u32 ImageId, u32 *FuncId));

XStatus XPm_GetChipID(u32* IDCode, u32 *Version);

XStatus XPm_GetApiVersion(u32 *Version);

XStatus XPm_AddSubsystem(u32 SubsystemId);

XStatus XPm_DestroySubsystem(u32 SubsystemId);

XStatus XPm_RequestWakeUp(u32 SubsystemId, const u32 DeviceId,
			const u32 SetAddress,
			const u64 Address,
			const u32 Ack,
			const u32 CmdType);

XStatus XPm_ForcePowerdown(u32 SubsystemId,
                             const u32 NodeId,
                             const u32 Ack,
			     const u32 CmdType, const u32 IpiMask);
XStatus XPm_SystemShutdown(u32 SubsystemId, const u32 Type, const u32 SubType,
			   const u32 CmdType);

XStatus XPm_SetWakeUpSource(const u32 SubsystemId,
			    const u32 TargetNodeId,
			    const u32 SourceNodeId,
			    const u32 Enable);

XStatus XPm_RequestDevice(const u32 SubsystemId, const u32 DeviceId,
			  const u32 Capabilities, const u32 QoS, const u32 Ack,
			  const u32 CmdType);

XStatus XPm_ReleaseDevice(const u32 SubsystemId, const u32 DeviceId,
			  const u32 CmdType);

XStatus XPm_SetRequirement(const u32 SubsystemId, const u32 DeviceId,
			   const u32 Capabilities, const u32 QoS,
			   const u32 Ack);

XStatus XPm_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId,
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

XStatus XPm_SetResetState(const u32 SubsystemId, const u32 ResetId,
			  const u32 Action, const u32 CmdType);

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
                        const u32 Arg2,u32 *const Response, const u32 CmdType);
XStatus XPm_InitFinalize(const u32 SubsystemId);

XStatus XPm_DescribeNodes(u32 NumArgs);
XStatus XPm_AddNodeParent(const u32 *Args, u32 NumArgs);
XStatus XPm_AddNodeName(const u32 *Args, u32 NumArgs);
XStatus XPm_AddNode(const u32 *Args, u32 NumArgs);
XStatus XPm_SelfSuspend(const u32 SubsystemId, const u32 DeviceId,
			const u32 Latency, const u8 State,
			u32 AddrLow, u32 AddrHigh);
XStatus XPm_AbortSuspend(const u32 SubsystemId, const u32 Reason,
			 const u32 DeviceId);
XStatus XPm_RequestSuspend(const u32 SubsystemId, const u32 TargetSubsystemId,
			   const u32 Ack, const u32 Latency, const u32 State,
			   const u32 CmdType);
XStatus XPm_InitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs);
XStatus XPm_FeatureCheck(const u32 ApiId, u32 *const Version);
XStatus XPm_IsoControl(u32 NodeId, u32 Enable);
XStatus XPm_GetOpCharacteristic(const u32 DeviceId, const u32 Type,
				u32 *Result);
XStatus XPm_RegisterNotifier(const u32 SubsystemId, const u32 NodeId,
			 const u32 Event, const u32 Wake, const u32 Enable,
			 const u32 IpiMask);
XStatus XPm_GicProxyWakeUp(const u32 PeriphIdx);
XStatus XPm_HookAfterPlmCdo(void);
int XPm_RestartCbWrapper(const u32 SubsystemId);
u32 XPm_GetSubsystemId(u32 ImageId);
XStatus XPm_GetDeviceBaseAddr(u32 DeviceId, u32 *BaseAddr);
int XPm_ForcePwrDwnCb(void *Data);
void XPm_ProcessAckReq(const u32 Ack, const u32 IpiMask, const int Status,
		       const u32 NodeId, const u32 NodeState);
XStatus XPm_IdleRestartHandler(const u32 SubsystemId);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_API_H_ */
