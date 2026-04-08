/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_MEM_TCM_H
#define XPM_MEM_TCM_H
#include "xil_types.h"
#include "xstatus.h"
#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif
XStatus XPm_GetRpuByTcmId(u32 TcmId, u32 *RpuId);
XStatus XPmMem_TcmResetRelease(const XPm_Device *Device);
#ifdef __cplusplus
}
#endif

#endif /* XPM_MEM_TCM_H */
