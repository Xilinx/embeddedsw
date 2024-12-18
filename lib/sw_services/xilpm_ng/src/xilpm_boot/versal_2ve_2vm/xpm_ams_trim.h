/******************************************************************************
* Copyright (c) 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_AMS_TRIM_H_
#define XPM_AMS_TRIM_H_
#include "xstatus.h"
#include "xil_types.h"
#ifdef __cplusplus
extern "C" {
#endif

XStatus XPm_ApplyAmsTrim(u32 DestAddress, u32 PowerDomainId, u32 SateliteIdx);

#ifdef __cplusplus
}
#endif
#endif /* XPM_AMS_TRIM_H_ */