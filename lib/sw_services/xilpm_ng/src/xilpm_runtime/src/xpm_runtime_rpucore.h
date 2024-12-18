/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __XPM_RUNTIME_RPUCORE_H__
#define __XPM_RUNTIME_RPUCORE_H__
#include "xstatus.h"
#include "xil_types.h"
#ifdef __cplusplus
extern "C" {
#endif
XStatus XPm_RpuBootAddrConfig(const u32 DeviceId, const u32 BootAddr);

#ifdef __cplusplus
}
#endif
#endif /* __XPM_RUNTIME_RPUCORE_H__ */
