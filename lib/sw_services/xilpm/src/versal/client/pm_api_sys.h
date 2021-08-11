/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file pm_api_sys.h
 *
 * @addtogroup xpm_versal_apis XilPM Versal APIs
 * @{
 *****************************************************************************/
#ifndef PM_API_SYS_H_
#define PM_API_SYS_H_

#include "pm_client.h"
#include "xpm_defs.h"
#include "xpm_err.h"
#include "xpm_nodeid.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * pm_init_suspend - Init suspend callback arguments (save for custom handling)
 */
struct pm_init_suspend {
	volatile u8 received;		/**< Has init suspend callback been received/handled */
	enum XPmSuspendReason reason;	/**< Reason of initializing suspend */
	u32 latency;			/**< Maximum allowed latency */
	u32 state;			/**< Targeted sleep/suspend state */
	u32 timeout;			/**< Period of time the client has to response */
};

/*
 * pm_acknowledge - Acknowledge callback arguments (save for custom handling)
 */
struct pm_acknowledge {
	volatile u8 received;		/**< Has acknowledge argument been received? */
	u32 node;			/**< Node argument about which the acknowledge is */
	XStatus status;			/**< Acknowledged status */
	u32 opp;			/**< Operating point of node in question */
};

/**
 * XPm_NodeStatus - struct containing node status information
 */
typedef struct XPm_NdStatus {
	u32 status;		/**< Node power state */
	u32 requirements;	/**< Current requirements asserted on the node (slaves only) */
	u32 usage;		/**< Usage information (which master is currently using the slave) */
} XPm_NodeStatus;

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
	u32 event;	/**< Event argument (the event type to receive notifications about) */
	u32 received_event;	/**< Event received from PLM) */
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

/* Global data declarations */
extern struct pm_init_suspend pm_susp;
extern struct pm_acknowledge pm_ack;

XStatus XPm_InitXilpm(XIpiPsu *IpiInst);
enum XPmBootStatus XPm_GetBootStatus(void);
XStatus XPm_GetChipID(u32* IDCode, u32 *Version);
XStatus XPm_GetApiVersion(u32 *Version);
XStatus XPm_RequestNode(const u32 DeviceId, const u32 Capabilities,
			const u32 QoS, const u32 Ack);
XStatus XPm_ReleaseNode(const u32 DeviceId);
XStatus XPm_SetRequirement(const u32 DeviceId, const u32 Capabilities,
			   const u32 QoS, const u32 Ack);
XStatus XPm_GetNodeStatus(const u32 DeviceId,
			  XPm_NodeStatus *const NodeStatus);
XStatus XPm_ResetAssert(const u32 ResetId, const u32 Action);
XStatus XPm_ResetGetStatus(const u32 ResetId, u32 *const State);
XStatus XPm_PinCtrlRequest(const u32 PinId);
XStatus XPm_PinCtrlRelease(const u32 PinId);
XStatus XPm_PinCtrlSetFunction(const u32 PinId, const u32 FunctionId);
XStatus XPm_PinCtrlGetFunction(const u32 PinId, u32 *const FunctionId);
XStatus XPm_PinCtrlSetParameter(const u32 PinId, const u32 ParamId, const u32 ParamVal);
XStatus XPm_PinCtrlGetParameter(const u32 PinId, const u32 ParamId, u32 *const ParamVal);
XStatus XPm_DevIoctl(const u32 DeviceId, const pm_ioctl_id IoctlId, const u32 Arg1,
		     const u32 Arg2, u32 *const Response);
XStatus XPm_ClockEnable(const u32 ClockId);
XStatus XPm_ClockDisable(const u32 ClockId);
XStatus XPm_ClockGetStatus(const u32 ClockId, u32 *const State);
XStatus XPm_ClockSetDivider(const u32 ClockId, const u32 Divider);
XStatus XPm_ClockGetDivider(const u32 ClockId, u32 *const Divider);
XStatus XPm_ClockSetParent(const u32 ClockId, const u32 ParentIdx);
XStatus XPm_ClockGetParent(const u32 ClockId, u32 *const ParentIdx);
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
XStatus XPm_RequestWakeUp(const u32 TargetDevId, const u8 SetAddress,
			  const u64 Address, const u32 Ack);
void XPm_SuspendFinalize(void);
XStatus XPm_RequestSuspend(const u32 TargetSubsystemId, const u32 Ack,
			   const u32 Latency, const u32 State);
XStatus XPm_AbortSuspend(const enum XPmAbortReason Reason);
XStatus XPm_ForcePowerDown(const u32 TargetDevId, const u32 Ack);
XStatus XPm_SystemShutdown(const u32 Type, const u32 SubType);
XStatus XPm_SetWakeUpSource(const u32 TargetDeviceId,
			    const u32 DeviceId, const u32 Enable);
XStatus XPm_Query(const u32 QueryId, const u32 Arg1, const u32 Arg2,
		  const u32 Arg3, u32 *const Data);
XStatus XPm_SetMaxLatency(const u32 DeviceId, const u32 Latency);
XStatus XPm_GetOpCharacteristic(const u32 DeviceId,
				const enum XPmOpCharType Type,
				u32 *const Result);
XStatus XPm_InitFinalize(void);
XStatus XPm_RegisterNotifier(XPm_Notifier* const Notifier);
XStatus XPm_UnregisterNotifier(XPm_Notifier* const Notifier);
void XPm_NotifyCb(const u32 Node, const u32 Event, const u32 Oppoint);

void XPm_InitSuspendCb(const enum XPmSuspendReason Reason,
		       const u32 Latency, const u32 State, const u32 Timeout);
void XPm_AcknowledgeCb(const u32 Node, const XStatus Status, const u32 Oppoint);
XStatus XPm_ClockSetRate(const u32 ClockId, const u32 Rate);
XStatus XPm_ClockGetRate(const u32 ClockId, u32 *const Rate);
XStatus XPm_FeatureCheck(const u32 FeatureId, u32 *Version);

/** @cond INTERNAL */
XStatus XPm_SetConfiguration(const u32 Address);
XStatus XPm_MmioWrite(const u32 Address, const u32 Mask, const u32 Value);
XStatus XPm_MmioRead(const u32 Address, u32 *const Value);
u32 XPm_GetRegisterNotifierVersionServer(void);
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* PM_API_SYS_H_ */
 /** @} */
