/******************************************************************************
* Copyright (c) 2021 - 2022, Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
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

struct XPm_AieInitNodeOps {
	XStatus (*InitStart)(XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs);
	XStatus (*InitFinish)(const XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs);
};

struct XPm_AieDeviceNode {
	XPm_Device Device;		/**< Device: Base class */
	struct XPm_AieInitNodeOps *Ops; /**< Node Initialization Operations */
	XPm_PlDevice *Parent;		/**< Parent of Aie device */
};

/************************** Function Prototypes ******************************/
XStatus XPmAieDevice_Init(XPm_AieDevice *AieDevice, u32 NodeId,
			  u32 BaseAddress, XPm_Power *Power,
			  XPm_ClockNode *Clock, XPm_ResetNode *Reset);
XStatus XPmAieDevice_UpdateClockDiv(const u32 Divider);
/**
 * @brief Query the current AIE clock divider value.
 *
 * @param Response Pointer to store the divider query result.
 *
 * @return XST_SUCCESS on success, error code otherwise.
 */
XStatus XPmAieDevice_QueryDivider(u32 *Response);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_AIEDEVICE_H_ */
