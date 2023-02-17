/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_CORE_PLAT_H_
#define XPM_CORE_PLAT_H_

#include "xpm_common.h"
#include "xpm_clock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_Core XPm_Core;
/************************** Function Prototypes ******************************/
XStatus ResetAPUGic(const u32 DeviceId);
void DisableWake(const struct XPm_Core *Core);
maybe_unused static inline  XStatus XPm_PlatSendDirectPowerDown(const XPm_Core *Core)
{
	(void)Core;
	return XST_SUCCESS;
}
static inline XStatus XPmCore_PlatClkReq(const XPm_ClockHandle *ClkHandles)
{
	return XPmClock_Request(ClkHandles);
}

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_CORE_PLAT_H_ */
