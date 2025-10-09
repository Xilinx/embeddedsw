/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __XPM_FEATURE_CHECK_H__
#define __XPM_FEATURE_CHECK_H__
#include "xil_types.h"
#include "xstatus.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PM_GET_OP_CHAR_FEATURE_BITMASK ( \
		(1U << (u32)PM_OPCHAR_TYPE_TEMP) | \
		(1U << (u32)PM_OPCHAR_TYPE_LATENCY))

#define PM_QUERY_FEATURE_BITMASK ( \
	(1ULL << (u64)XPM_QID_CLOCK_GET_NAME) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_TOPOLOGY) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_MUXSOURCES) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_ATTRIBUTES) | \
	(1ULL << (u64)XPM_QID_PINCTRL_GET_NUM_PINS) | \
	(1ULL << (u64)XPM_QID_PINCTRL_GET_NUM_FUNCTIONS) | \
	(1ULL << (u64)XPM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS) | \
	(1ULL << (u64)XPM_QID_PINCTRL_GET_FUNCTION_NAME) | \
	(1ULL << (u64)XPM_QID_PINCTRL_GET_FUNCTION_GROUPS) | \
	(1ULL << (u64)XPM_QID_PINCTRL_GET_PIN_GROUPS) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_NUM_CLOCKS) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_MAX_DIVISOR) | \
	(1ULL << (u64)XPM_QID_PLD_GET_PARENT) | \
	(1ULL << (u64)XPM_QID_PINCTRL_GET_ATTRIBUTES))

#define PM_IOCTL_FEATURE_BITMASK ( \
	(1ULL << (u64)IOCTL_GET_APU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_SET_APU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_GET_RPU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_SET_RPU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_RPU_BOOT_ADDR_CONFIG) | \
	(1ULL << (u64)IOCTL_SET_TAPDELAY_BYPASS) | \
	(1ULL << (u64)IOCTL_SD_DLL_RESET) | \
	(1ULL << (u64)IOCTL_SET_SD_TAPDELAY) | \
	(1ULL << (u64)IOCTL_SET_PLL_FRAC_MODE) | \
	(1ULL << (u64)IOCTL_GET_PLL_FRAC_MODE) | \
	(1ULL << (u64)IOCTL_SET_PLL_FRAC_DATA) | \
	(1ULL << (u64)IOCTL_GET_PLL_FRAC_DATA) | \
	(1ULL << (u64)IOCTL_SET_BOOT_HEALTH_STATUS) | \
	(1ULL << (u64)IOCTL_OSPI_MUX_SELECT) | \
	(1ULL << (u64)IOCTL_USB_SET_STATE) | \
	(1ULL << (u64)IOCTL_GET_LAST_RESET_REASON) | \
	(1ULL << (u64)IOCTL_READ_REG) | \
	(1ULL << (u64)IOCTL_MASK_WRITE_REG))
XStatus XPm_FeatureCheck(u32 ApiId, u32 *Version);
#ifdef __cplusplus
}
#endif
#endif /* __XPM_FEATURE_CHECK_H__ */
