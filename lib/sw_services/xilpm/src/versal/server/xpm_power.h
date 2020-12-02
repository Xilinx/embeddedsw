/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_POWER_H_
#define XPM_POWER_H_

#include "xpm_defs.h"
#include "xpm_node.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	/* Default FSM states */
	XPM_POWER_STATE_OFF = 0,
	XPM_POWER_STATE_INITIALIZING,
	XPM_POWER_STATE_ON,
	XPM_POWER_STATE_STANDBY,
	XPM_POWER_STATE_PWR_UP_PARENT,
	XPM_POWER_STATE_PWR_DOWN_PARENT,
	XPM_POWER_STATE_PWR_UP_SELF,
	XPM_POWER_STATE_PWR_DOWN_SELF,
} XPm_PowerState;

typedef enum {
	XPM_POWER_EVENT_PWR_UP,
	XPM_POWER_EVENT_PARENT_UP_DONE,
	XPM_POWER_EVENT_SELF_UP_DONE,
	XPM_POWER_EVENT_PWR_DOWN,
	XPM_POWER_EVENT_SELF_DOWN_DONE,
	XPM_POWER_EVENT_PARENT_DOWN_DONE,
	XPM_POWER_EVENT_TIMER,
} XPm_PowerEvent;

typedef struct XPm_Power XPm_Power;

/**
 * The power node class.  This is the base class for all the power island and
 * power domain classes.
 */
struct XPm_Power {
	XPm_Node Node; /**< Node: Node base class */
	XPm_Power *Parent; /**< Parent: Parent node in the power topology */
	XPm_Power *NextPeer; /**< NextPeer: Next power node of the same parent  */
	u8 UseCount; /**< No. of devices currently using this power node */
	u8 WfParentUseCnt; /**< Pending use count of the parent */
	u16 PwrDnLatency; /**< Latency (in us) for transition to OFF state */
	u16 PwrUpLatency; /**< Latency (in us) for transition to ON state */
	XStatus (* HandleEvent)(XPm_Node *Node, u32 Event);
		/**< HandleEvent: Pointer to event handler */
};

/************************** Function Prototypes ******************************/
XPm_Power *XPmPower_GetById(u32 Id);
XStatus XPmPower_Init(XPm_Power *Power,
	u32 Id, u32 BaseAddress, XPm_Power *Parent);
XStatus XPmPower_AddParent(u32 Id, u32 *Parents, u32 NumParents);
XStatus XPmPower_GetStatus(const u32 SubsystemId, const u32 DeviceId, XPm_DeviceStatus *const DeviceStatus);
int XPmPower_GetWakeupLatency(const u32 DeviceId, u32 *Latency);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWER_H_ */
