/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_PLDEVICE_H_
#define XPM_PLDEVICE_H_

#include "xpm_device.h"
#include "xpm_powerdomain.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NOC_CLOCK_ARRAY_SIZE    8U

/**
 * PLD node class.
 */
typedef struct XPm_PlDeviceNode XPm_PlDevice;

struct XPm_PldInitNodeOps {
	XStatus (*InitStart)(XPm_PlDevice *PlDevice, const u32 *Args, u32 NumArgs);
	XStatus (*InitFinish)(XPm_PlDevice *PlDevice, const u32 *Args, u32 NumArgs);
	XStatus (*MemCtrlrMap)(XPm_PlDevice *PlDevice, const u32 *Args, u32 NumArgs);
};

struct XPm_PlDeviceNode {
	XPm_Device Device;              /**< Device: Base class */
	u8 PowerBitMask;                /**< Current Power Domain Dependency */
	u8 WfPowerBitMask;              /**< Desired Power Domain Dependency */
	struct XPm_PldInitNodeOps *Ops; /**< Node Initialization Operations */
	XPm_PlDevice *Parent;           /**< Parent of PLD */
	XPm_PlDevice *NextPeer;         /**< Sibling/Peer of PLD */
	XPm_PlDevice *Child;            /**< Child head PLD’s children */
	struct XPm_AieDeviceNode *AieDevice;    /**< Link to AIE Device */
	u32 NocClockEnablement[MAX_NOC_CLOCK_ARRAY_SIZE];	/**< Bit array representing NoC clock enablement */
};

/************************** Function Prototypes ******************************/
XStatus XPmPlDevice_Init(XPm_PlDevice *PlDevice,
		u32 PldId,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset);

XStatus XPmPlDevice_GetParent(u32 NodeId, u32 *Resp);
XStatus XPmPlDevice_IsValidPld(const XPm_PlDevice *PlDevice);
XStatus XPmPlDevice_NocClkEnable(XPm_PlDevice *PlDevice, const u32 *Args, u32 NumArgs);
XStatus XPmPlDevice_IfNocClkEnable(XPlmi_Cmd *Cmd, u32 BitArrayIdx, u16 State,
		        u16 Mask, u32 Level);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PLDEVICE_H_ */
