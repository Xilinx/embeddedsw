/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_CORE_PLAT_H_
#define XPM_CORE_PLAT_H_

#include "xpm_clock.h"
#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_Core XPm_Core;
/************************** Function Prototypes ******************************/
XStatus ResetAPUGic(const u32 DeviceId);
void EnableWake(const struct XPm_Core *Core);
void DisableWake(const struct XPm_Core *Core);
XStatus XPm_PlatSendDirectPowerDown(XPm_Core *Core);
maybe_unused static inline XStatus XPmCore_PlatClkReq(const XPm_ClockHandle *ClkHandles)
{
	(void)ClkHandles;
	return XST_SUCCESS;
}

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_CORE_PLAT_H_ */
