/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_POWER_H
#define XPM_RUNTIME_POWER_H
#include "xil_types.h"
#include "xstatus.h"
#include "xpm_power.h"
#ifdef __cplusplus
extern "C" {
#endif

XPm_Power *XPmPower_HoldFpd(void);
void XPmPower_ReleaseFpd(XPm_Power *Fpd);

#ifdef __cplusplus
}
#endif
#endif /* XPM_RUNTIME_POWER_H */
