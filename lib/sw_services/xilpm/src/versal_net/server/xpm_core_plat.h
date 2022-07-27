/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_CORE_PLAT_H_
#define XPM_CORE_PLAT_H_

#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_Core XPm_Core;
/************************** Function Prototypes ******************************/
XStatus ResetAPUGic(const u32 DeviceId);
XStatus SkipRpuReset(const struct XPm_Core *Core);
void EnableWake(const struct XPm_Core *Core);
void DisableWake(const struct XPm_Core *Core);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_CORE_PLAT_H_ */
