/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_POWER_PLAT_H_
#define XPM_POWER_PLAT_H_

#include "xpm_defs.h"
#include "xpm_node.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_Power XPm_Power;

/**
 * The power node class.  This is the base class for all the power island and
 * power domain classes.
 */
struct XPm_Power {
	XPm_Node Node; /**< Node: Node base class */
	XPm_Power *Parent; /**< Parent: Parent node in the power topology */
	u8 UseCount; /**< No. of devices currently using this power node */
	u8 WfParentUseCnt; /**< Pending use count of the parent */
	u16 PwrDnLatency; /**< Latency (in us) for transition to OFF state */
	u16 PwrUpLatency; /**< Latency (in us) for transition to ON state */
	u32 PwrUpEnOffset; /**< PSM request power up interrupt enable register offset */
	u32 PwrDwnEnOffset; /**< PSM request power down interrupt enable register offset */
	u32 PwrUpMask; /**< PSM request power up interrupt mask */
	u32 PwrDwnMask; /**< PSM request power down interrupt mask */
	u32 PwrStatOffset; /**< PSM power state register offset */
	u32 PwrStatMask; /**< PSM power state mask */
	XStatus (* HandleEvent)(XPm_Node *Node, u32 Event);
		/**< HandleEvent: Pointer to event handler */
};

/************************** Function Prototypes ******************************/
void XPmPower_SetPsmRegInfo(XPm_Power *Power, const u32 *Args);
XStatus XPmPower_SendIslandPowerDwnReq(const XPm_Node *Node);
XStatus XPmPower_SendIslandPowerUpReq(const XPm_Node *Node);
XStatus XPmPower_PlatSendPowerUpReq(XPm_Node *Node);
XStatus XPmPower_PlatSendPowerDownReq(const XPm_Node *Node);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWER_PLAT_H_ */
