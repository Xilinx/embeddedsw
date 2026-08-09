/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_RPUCORE_H
#define XPM_RUNTIME_RPUCORE_H
#include "xstatus.h"
#include "xil_types.h"
#ifdef __cplusplus
extern "C" {
#endif
XStatus XPm_RpuBootAddrConfig(const u32 DeviceId, const u32 BootAddr);

#ifdef __cplusplus
}
#endif
#endif /* XPM_RUNTIME_RPUCORE_H */
