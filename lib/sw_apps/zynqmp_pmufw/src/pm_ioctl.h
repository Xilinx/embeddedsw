/*
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#ifndef PM_IOCTL_H_
#define PM_IOCTL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpfw_config.h"

#ifdef ENABLE_IOCTL
#include "xil_types.h"

/*********************************************************************
 * Enum definitions
 ********************************************************************/
/* Feature IDs to enable or disable feature at runtime using IOCTL call */
typedef enum {
	XPM_FEATURE_INVALID = 0,
#ifdef ENABLE_RUNTIME_OVERTEMP
	XPM_FEATURE_OVERTEMP_STATUS = 1,
	XPM_FEATURE_OVERTEMP_VALUE = 2,
#endif
#ifdef ENABLE_RUNTIME_EXTWDT
	XPM_FEATURE_EXTWDT_STATUS = 3,
	XPM_FEATURE_EXTWDT_VALUE = 4,
#endif
} XPm_FeatureConfigId;

/*********************************************************************
 * Function declarations
 ********************************************************************/
s32 PmSetFeatureConfig(XPm_FeatureConfigId configId, u32 value);
s32 PmGetFeatureConfig(XPm_FeatureConfigId configId, u32 *value);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PM_IOCTL_H_ */
