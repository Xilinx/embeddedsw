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

#ifdef ENABLE_FEATURE_CONFIG
/*********************************************************************
 * Enum definitions
 ********************************************************************/
/* Feature IDs to enable or disable feature at runtime using IOCTL call */
typedef enum {
	XPM_FEATURE_INVALID = 0,
#ifdef ENABLE_RUNTIME_OVERTEMP
	XPM_FEATURE_OVERTEMP_STATUS = 1,
	XPM_FEATURE_OVERTEMP_VALUE = 2,
#endif /* ENABLE_RUNTIME_OVERTEMP */
#ifdef ENABLE_RUNTIME_EXTWDT
	XPM_FEATURE_EXTWDT_STATUS = 3,
	XPM_FEATURE_EXTWDT_VALUE = 4,
#endif /* ENABLE_RUNTIME_EXTWDT */
} XPm_FeatureConfigId;
#endif /* ENABLE_FEATURE_CONFIG */

#ifdef ENABLE_DYNAMIC_MIO_CONFIG
/* Config types for SD configs at run time */
typedef enum {
	SD_CONFIG_INVALID = 0,
	SD_CONFIG_EMMC_SEL = 1, /* To set SD_EMMC_SEL in CTRL_REG_SD and SD_SLOTTYPE */
	SD_CONFIG_BASECLK = 2, /* To set SD_BASECLK in SD_CONFIG_REG1 */
	SD_CONFIG_8BIT = 3, /* To set SD_8BIT in SD_CONFIG_REG2 */
	SD_CONFIG_FIXED = 4, /* To set fixed config registers */
} XPm_SdConfigType;

/* Config types for GEM configs at run time */
typedef enum {
	GEM_CONFIG_INVALID = 0,
	GEM_CONFIG_SGMII_MODE = 1, /* To set GEM_SGMII_MODE in GEM_CLK_CTRL register */
	GEM_CONFIG_FIXED = 2, /* To set fixed config registers */
} XPm_GemConfigType;

/* Config types for USB configs at run time */
typedef enum {
	USB_CONFIG_INVALID = 0,
	USB_CONFIG_FIXED = 1, /* To set fixed config registers */
} XPm_UsbConfigType;
#endif /* ENABLE_DYNAMIC_MIO_CONFIG */

/*********************************************************************
 * Function declarations
 ********************************************************************/
#ifdef ENABLE_FEATURE_CONFIG
s32 PmSetFeatureConfig(XPm_FeatureConfigId configId, u32 value);
s32 PmGetFeatureConfig(XPm_FeatureConfigId configId, u32 *value);
#endif /* ENABLE_FEATURE_CONFIG */
#ifdef ENABLE_DYNAMIC_MIO_CONFIG
s32 PmSetSdConfig(u32 nodeId, XPm_SdConfigType configType, u32 value);
s32 PmSetGemConfig(u32 nodeId, XPm_GemConfigType configType, u32 value);
s32 PmSetUsbConfig(u32 nodeId, XPm_UsbConfigType configType, u32 value);
#endif /* ENABLE_DYNAMIC_MIO_CONFIG */
#endif /* ENABLE_IOCTL */

#ifdef __cplusplus
}
#endif

#endif /* PM_IOCTL_H_ */
