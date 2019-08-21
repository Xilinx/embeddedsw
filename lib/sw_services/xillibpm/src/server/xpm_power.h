/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#ifndef XPM_POWER_H_
#define XPM_POWER_H_

#include "xillibpm_defs.h"
#include "xpm_node.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_POWERID_FPD		NODEID(XPM_NODECLASS_POWER, \
					XPM_NODESUBCL_POWER_DOMAIN, \
					XPM_NODETYPE_POWER_DOMAIN_PS_FULL, \
					XPM_NODEIDX_POWER_FPD)

#define XPM_POWERID_LPD		NODEID(XPM_NODECLASS_POWER, \
					XPM_NODESUBCL_POWER_DOMAIN, \
					XPM_NODETYPE_POWER_DOMAIN_PS_LOW, \
					XPM_NODEIDX_POWER_LPD)

#define XPM_POWERID_CPM		NODEID(XPM_NODECLASS_POWER, \
					XPM_NODESUBCL_POWER_DOMAIN, \
					XPM_NODETYPE_POWER_DOMAIN_CPM, \
					XPM_NODEIDX_POWER_CPM)

#define XPM_POWERID_PLD		NODEID(XPM_NODECLASS_POWER, \
					XPM_NODESUBCL_POWER_DOMAIN, \
					XPM_NODETYPE_POWER_DOMAIN_PL, \
					XPM_NODEIDX_POWER_PLD)


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
	u32 PwrDnLatency; /**< Latency (in us) for transition to OFF state */
	u32 PwrUpLatency; /**< Latency (in us) for transition to ON state */
};

/************************** Function Prototypes ******************************/
XPm_Power *XPmPower_GetById(u32 Id);
XStatus XPmPower_Init(XPm_Power *Power,
	u32 Id, u32 BaseAddress, XPm_Power *Parent);
XStatus XPmPower_AddParent(u32 Id, u32 *Parents, u32 NumParents);
XStatus XPmPower_GetStatus(const u32 SubsystemId, const u32 DeviceId, XPm_DeviceStatus *const DeviceStatus);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWER_H_ */
