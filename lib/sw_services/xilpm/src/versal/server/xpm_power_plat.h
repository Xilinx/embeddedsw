/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_POWER_PLAT_H_
#define XPM_POWER_PLAT_H_

#include "xpm_defs.h"
#include "xpm_node.h"
#include "xpm_common.h"
#ifdef SDT
#include "xpm_config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Versal DVS feature requires controlling external power rails, therefore,
 * include the power rail control code.
 */
#if defined (VERSAL_DVS) && ! defined (RAIL_CONTROL)
#define RAIL_CONTROL
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
	XStatus (* HandleEvent)(XPm_Node *Node, u32 Event);
		/**< HandleEvent: Pointer to event handler */
};

#if defined (RAIL_CONTROL)
/* Support for up to 4 words of data for I2C commands */
#define MAX_I2C_COMMAND_LEN	16
#endif /* RAIL_CONTROL */

/************************** Function Prototypes ******************************/
maybe_unused static inline void XPmPower_SetPsmRegInfo(XPm_Power *Power, const u32 *Args)
{
	(void)Power;
	(void)Args;
}
XStatus XPmPower_SendIslandPowerUpReq(const XPm_Node *Node);
XStatus XPmPower_SendIslandPowerDwnReq(const XPm_Node *Node);
XStatus XPmPower_PlatSendPowerUpReq(const XPm_Node *Node);
XStatus XPmPower_PlatSendPowerDownReq(const XPm_Node *Node);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWER_H_ */
