/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 * @file pm_api_sys.h
 * PM API System implementation
 * @addtogroup xpm_apis XilPM APIs
 *
 * @{
 *****************************************************************************/

#ifndef PM_API_SYS_H
#define PM_API_SYS_H

#include <xil_types.h>
#include <xstatus.h>
#include <xipipsu.h>
#include "pm_defs.h"
#include "pm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

XStatus XPm_InitXilpm(XIpiPsu *IpiInst);

void XPm_SuspendFinalize(void);

enum XPmBootStatus XPm_GetBootStatus(void);

/* System-level API function declarations */
XStatus XPm_RequestSuspend(const enum XPmNodeId target,
			   const enum XPmRequestAck ack,
			   const u32 latency,
			   const u8 state);

XStatus XPm_SelfSuspend(const enum XPmNodeId nid,
			const u32 latency,
			const u8 state,
			const u64 address);

XStatus XPm_ForcePowerDown(const enum XPmNodeId target,
			   const enum XPmRequestAck ack);

XStatus XPm_AbortSuspend(const enum XPmAbortReason reason);

XStatus XPm_RequestWakeUp(const enum XPmNodeId target,
			  const bool setAddress,
			  const u64 address,
			  const enum XPmRequestAck ack);

XStatus XPm_SetWakeUpSource(const enum XPmNodeId target,
			    const enum XPmNodeId wkup_node,
			    const u8 enable);

XStatus XPm_SystemShutdown(u32 type, u32 subtype);

XStatus XPm_SetConfiguration(const u32 address);

XStatus XPm_InitFinalize(void);

/* Callback API function */
/*
 * pm_init_suspend - Init suspend callback arguments (save for custom handling)
 */
struct pm_init_suspend {
	volatile bool received;			/**< Has init suspend callback been received/handled */
	enum XPmSuspendReason reason;	/**< Reason of initializing suspend */
	u32 latency;					/**< Maximum allowed latency */
	u32 state;						/**< Targeted sleep/suspend state */
	u32 timeout;					/**< Period of time the client has to response */
};

/*
 * pm_acknowledge - Acknowledge callback arguments (save for custom handling)
 */
struct pm_acknowledge {
	volatile bool received;		/**< Has acknowledge argument been received? */
	enum XPmNodeId node;		/**< Node argument about which the acknowledge is */
	XStatus status;				/**< Acknowledged status */
	u32 opp;					/**< Operating point of node in question */
};


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
	enum XPmNodeId node; /**< Node argument (the node to receive notifications about) */
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


/**
 * XPm_NodeStatus - struct containing node status information
 */
typedef struct XPm_NdStatus {
	u32 status;			/**< Node power state */
	u32 requirements;	/**< Current requirements asserted on the node (slaves only) */
	u32 usage;			/**< Usage information (which master is currently using the slave) */
} XPm_NodeStatus;

/********************************************************************/
/*
 * Global data declarations
 ********************************************************************/
extern struct pm_init_suspend pm_susp;
extern struct pm_acknowledge pm_ack;

void XPm_InitSuspendCb(const enum XPmSuspendReason reason,
		       const u32 latency,
		       const u32 state,
		       const u32 timeout);

void XPm_AcknowledgeCb(const enum XPmNodeId node,
		       const XStatus status,
		       const u32 oppoint);

void XPm_NotifyCb(const enum XPmNodeId node,
		const enum XPmNotifyEvent event,
		  const u32 oppoint);

/* API functions for managing PM Slaves */
XStatus XPm_RequestNode(const enum XPmNodeId node,
			const u32 capabilities,
			const u32 qos,
			const enum XPmRequestAck ack);
XStatus XPm_ReleaseNode(const enum XPmNodeId node);
XStatus XPm_SetRequirement(const enum XPmNodeId nid,
			   const u32 capabilities,
			   const u32 qos,
			   const enum XPmRequestAck ack);
XStatus XPm_SetMaxLatency(const enum XPmNodeId node,
			  const u32 latency);

/* Miscellaneous API functions */
XStatus XPm_GetApiVersion(u32 *version);

XStatus XPm_GetNodeStatus(const enum XPmNodeId node,
			  XPm_NodeStatus *const nodestatus);

XStatus XPm_RegisterNotifier(XPm_Notifier* const notifier);
XStatus XPm_UnregisterNotifier(XPm_Notifier* const notifier);

XStatus XPm_GetOpCharacteristic(const enum XPmNodeId node,
				const enum XPmOpCharType type,
				u32* const result);

/* Direct-Control API functions */
XStatus XPm_ResetAssert(const enum XPmReset reset,
			const enum XPmResetAction resetaction);

XStatus XPm_ResetGetStatus(const enum XPmReset reset, u32 *status);

XStatus XPm_MmioWrite(const u32 address, const u32 mask, const u32 value);

XStatus XPm_MmioRead(const u32 address, u32 *const value);

/* Clock API */
XStatus XPm_ClockEnable(const enum XPmClock clk);
XStatus XPm_ClockDisable(const enum XPmClock clk);
XStatus XPm_ClockGetStatus(const enum XPmClock clk, u32 *const status);

XStatus XPm_ClockSetDivider(const enum XPmClock clk, const u32 divider);
XStatus XPm_ClockGetDivider(const enum XPmClock clk, u32 *const divider);

XStatus XPm_ClockSetParent(const enum XPmClock clk,
			   const enum XPmClock parent);
XStatus XPm_ClockGetParent(const enum XPmClock clk,
			   enum XPmClock *const parent);

XStatus XPm_ClockSetRate(const enum XPmClock clk, const u32 rate);
XStatus XPm_ClockGetRate(const enum XPmClock clk, u32 *const rate);

/* PLL API */
XStatus XPm_PllSetParameter(const enum XPmNodeId node,
			    const enum XPmPllParam parameter,
			    const u32 value);
XStatus XPm_PllGetParameter(const enum XPmNodeId node,
			    const enum XPmPllParam parameter,
			    u32 *const value);

XStatus XPm_PllSetMode(const enum XPmNodeId node, const enum XPmPllMode mode);
XStatus XPm_PllGetMode(const enum XPmNodeId node, enum XPmPllMode* const mode);

/* PIN Mux control API */
XStatus XPm_PinCtrlRequest(const u32 pin);
XStatus XPm_PinCtrlRelease(const u32 pin);

XStatus XPm_PinCtrlSetFunction(const u32 pin, const enum XPmPinFn fn);
XStatus XPm_PinCtrlGetFunction(const u32 pin, enum XPmPinFn* const fn);

XStatus XPm_PinCtrlSetParameter(const u32 pin,
				const enum XPmPinParam param,
				const u32 value);
XStatus XPm_PinCtrlGetParameter(const u32 pin,
				const enum XPmPinParam param,
				u32* const value);
XStatus XPm_DevIoctl(const u32 deviceId, const pm_ioctl_id ioctlId,
		     const u32 arg1, const u32 arg2, u32 *const response);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* PM_API_SYS_H */
