/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __XPM_RUNTIME_POWER_H__
#define __XPM_RUNTIME_POWER_H__
#include "xil_types.h"
#include "xstatus.h"
#include "xpm_power.h"
#ifdef __cplusplus
extern "C" {
#endif
XStatus XPmPower_ForcePwrDwn(u32 NodeId);

#ifdef __cplusplus
}
#endif
#endif /* __XPM_RUNTIME_POWER_H__ */
