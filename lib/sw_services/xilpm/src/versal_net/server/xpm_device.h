/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_DEVICE_H_
#define XPM_DEVICE_H_

#include "xpm_node.h"
#include "xpm_defs.h"
#include "xpm_common.h"
#ifdef __cplusplus
extern "C" {
#endif

XStatus XPmDevice_GetStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_DEVICE_H_ */
