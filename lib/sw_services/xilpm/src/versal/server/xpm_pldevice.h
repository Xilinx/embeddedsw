/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_PLDEVICE_H_
#define XPM_PLDEVICE_H_

#include "xpm_device.h"
#include "xpm_powerdomain.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * PLD node class.
 */
typedef struct XPm_PlDeviceNode XPm_PlDevice;

struct XPm_PlDeviceNode {
	XPm_Device Device;
	u8 PowerBitMask;
	u8 WfPowerBitMask;
	struct XPm_PowerDomainOps *Ops;
	XPm_PlDevice *Parent;
	XPm_PlDevice *NextPeer;
	XPm_PlDevice *Child;
};

/************************** Function Prototypes ******************************/
XStatus XPmPlDevice_Init(XPm_PlDevice *PlDevice,
		u32 PldId,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PLDEVICE_H_ */
