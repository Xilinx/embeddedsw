/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_AIEDEVICE_H_
#define XPM_AIEDEVICE_H_

#include "xpm_pldevice.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IS_DEV_AIE(id)			(((u32)XPM_NODECLASS_DEVICE == NODECLASS(id)) && \
					 ((u32)XPM_NODESUBCL_DEV_AIE == NODESUBCLASS(id)))

/**
 * AIE node class.
 */
typedef struct XPm_AieDeviceNode XPm_AieDevice;

struct XPm_AieInitNodeOps {
	XStatus (*InitStart)(const XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs);
	XStatus (*InitFinish)(const XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs);
};

struct XPm_AieDeviceNode {
	XPm_Device Device;              /**< Device: Base class */
	struct XPm_AieInitNodeOps *Ops; /**< Node Initialization Operations */
	XPm_PlDevice *Parent;           /**< Parent of Aie device */
};

/************************** Function Prototypes ******************************/
XStatus XPmAieDevice_Init(XPm_AieDevice *AieDevice, u32 NodeId,
			  u32 BaseAddress, XPm_Power *Power,
			  XPm_ClockNode *Clock, XPm_ResetNode *Reset);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_AIEDEVICE_H_ */
