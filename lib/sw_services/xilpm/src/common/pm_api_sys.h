/******************************************************************************
*
* Copyright (C) 2015-2018 Xilinx, Inc.  All rights reserved.
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

/*****************************************************************************/
/**
 * @file pm_api_sys.h
 * PM API System implementation
 * @addtogroup xpm_apis XilPM APIs
 *
 * Xilinx Power Management(XilPM) provides Embedded Energy Management
 * Interface (EEMI) APIs for power management on Zynq&reg; UltraScale+&trade;
 * MPSoC. For more details about power management on Zynq Ultrascale+ MPSoC,
 * see the Zynq UltraScale+ MPSoC Power Management User Guide (UG1199).
 * For more details about EEMI, see the Embedded Energy Management Interface
 * (EEMI) API User Guide(UG1200).
 * @{
 *****************************************************************************/

#ifndef _PM_API_SYS_H_
#define _PM_API_SYS_H_

#include <xil_types.h>
#include <xstatus.h>
#include <xipipsu.h>
#include "pm_defs.h"
#include "pm_common.h"

XStatus XPm_InitXilpm(XIpiPsu *IpiInst);

void XPm_SuspendFinalize();

enum XPmBootStatus XPm_GetBootStatus();

/* System-level API function declarations */
XStatus XPm_RequestSuspend(const enum XPmNodeId node,
			   const enum XPmRequestAck ack,
			   const u32 latency,
			   const u8 state);

XStatus XPm_SelfSuspend(const enum XPmNodeId node,
			const u32 latency,
			const u8 state,
			const u64 address);

XStatus XPm_ForcePowerDown(const enum XPmNodeId node,
			   const enum XPmRequestAck ack);

XStatus XPm_AbortSuspend(const enum XPmAbortReason reason);

XStatus XPm_RequestWakeUp(const enum XPmNodeId node,
			  const bool setAddress,
			  const u64 address,
			  const enum XPmRequestAck ack);

XStatus XPm_SetWakeUpSource(const enum XPmNodeId target,
			    const enum XPmNodeId wkup_node,
			    const u8 enable);

XStatus XPm_SystemShutdown(u32 type, u32 subtype);

XStatus XPm_SetConfiguration(const u32 address);

XStatus XPm_InitFinalize();

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

/* Forward declaration to enable self reference in struct definition */
typedef struct XPm_Notifier XPm_Notifier;

/**
 * XPm_Notifier - Notifier structure registered with a callback by app
 */
typedef struct XPm_Notifier {
	/**
	 *  Custom callback handler to be called when the notification is
	 *  received. The custom handler would execute from interrupt
	 *  context, it shall return quickly and must not block! (enables
	 *  event-driven notifications)
	 */
	void (*const callback)(XPm_Notifier* const notifier);
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
	XPm_Notifier* next;
} XPm_Notifier;

/* Notifier Flags */
#define XILPM_NOTIFIER_FLAG_WAKE	BIT(0) /* wake up PU for notification */

/**
 * XPm_NodeStatus - struct containing node status information
 */
typedef struct XPm_NodeStatus {
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
		  const u32 event,
		  const u32 oppoint);

/* API functions for managing PM Slaves */
XStatus XPm_RequestNode(const enum XPmNodeId node,
			const u32 capabilities,
			const u32 qos,
			const enum XPmRequestAck ack);
XStatus XPm_ReleaseNode(const enum XPmNodeId node);
XStatus XPm_SetRequirement(const enum XPmNodeId node,
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
			const enum XPmResetAction assert);

XStatus XPm_ResetGetStatus(const enum XPmReset reset, u32 *status);

XStatus XPm_MmioWrite(const u32 address, const u32 mask, const u32 value);

XStatus XPm_MmioRead(const u32 address, u32 *const value);

/* Clock API */
XStatus XPm_ClockEnable(const enum XPmClock clock);
XStatus XPm_ClockDisable(const enum XPmClock clock);
XStatus XPm_ClockGetStatus(const enum XPmClock clock, u32 *const status);

XStatus XPm_ClockSetDivider(const enum XPmClock clock, const u32 divider);
XStatus XPm_ClockGetDivider(const enum XPmClock clock, u32 *const divider);

XStatus XPm_ClockSetParent(const enum XPmClock clock,
			   const enum XPmClock parent);
XStatus XPm_ClockGetParent(const enum XPmClock clock,
			   enum XPmClock *const parent);

XStatus XPm_ClockSetRate(const enum XPmClock clock, const u32 rate);
XStatus XPm_ClockGetRate(const enum XPmClock clock, u32 *const rate);

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

/** @} */
#endif /* _PM_API_SYS_H_ */
