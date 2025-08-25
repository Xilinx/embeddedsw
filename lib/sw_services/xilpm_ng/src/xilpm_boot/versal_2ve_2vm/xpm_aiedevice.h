/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_AIEDEVICE_H_
#define XPM_AIEDEVICE_H_

#include "xpm_pldevice.h"
#include "xpm_aie.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IS_DEV_AIE(ID)			(((u32)XPM_NODECLASS_DEVICE == NODECLASS(ID)) && \
					 ((u32)XPM_NODESUBCL_DEV_AIE == NODESUBCLASS(ID)) && \
					 ((u32)XPM_NODEIDX_DEV_AIE != NODEINDEX(ID)))

/**
 * AIE node class.
 */
typedef struct XPm_AieDeviceNode XPm_AieDevice;

struct XPm_AieDeviceNode {
	XPm_Device Device;		/**< Device: Base class */
	XPm_PlDevice *Parent;		/**< Parent of Aie device */
	u32 DefaultClockDiv;		/**< Default AIE clock divider */
};

/************************** Function Prototypes ******************************/
XStatus XPmAieDevice_Init(XPm_AieDevice *AieDevice, u32 NodeId,
			  u32 BaseAddress, XPm_Power *Power,
			  XPm_ClockNode *Clock, XPm_ResetNode *Reset);

XStatus XPmAieDevice_InitStart(XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs);

XStatus XPmAieDevice_InitFinish(const XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_AIEDEVICE_H_ */
