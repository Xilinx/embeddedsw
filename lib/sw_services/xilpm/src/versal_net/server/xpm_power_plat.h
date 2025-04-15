/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_POWER_PLAT_H_
#define XPM_POWER_PLAT_H_

#include "xpm_defs.h"
#include "xpm_node.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Support for VID is a requirement for this platform, and since that support
 * requires controlling external power rails, include the power rail control code.
 */
#if ! defined (RAIL_CONTROL)
#define RAIL_CONTROL
#endif

typedef struct XPm_Power XPm_Power;

/**
 * The power node class.  This is the base class for all the power island and
 * power domain classes.
 */
struct XPm_Power {
	XPm_Node Node; /**< Node: Node base class */
	u16 PwrDnLatency; /**< Latency (in us) for transition to OFF state */
	u16 PwrUpLatency; /**< Latency (in us) for transition to ON state */
	u32 PwrUpEnOffset; /**< PSM request power up interrupt enable register offset */
	u32 PwrDwnEnOffset; /**< PSM request power down interrupt enable register offset */
	u32 PwrUpMask; /**< PSM request power up interrupt mask */
	u32 PwrDwnMask; /**< PSM request power down interrupt mask */
	u32 PwrStatOffset; /**< PSM power state register offset */
	u32 PwrStatMask; /**< PSM power state mask */
	SAVE_REGION(
	u8 UseCount; /**< No. of devices currently using this power node */
	u8 WfParentUseCnt; /**< Pending use count of the parent */
	)
	XPm_Power *Parent; /**< Parent: Parent node in the power topology */
	XStatus (* HandleEvent)(XPm_Node *Node, u32 Event);
		/**< HandleEvent: Pointer to event handler */
};

/* Support for up to 7 words of data for I2C commands */
#define MAX_I2C_COMMAND_LEN	28

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
