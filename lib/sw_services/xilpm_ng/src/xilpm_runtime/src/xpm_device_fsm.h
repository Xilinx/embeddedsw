/******************************************************************************
 * Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#ifndef XPM_DEVICE_FSM_H
#define XPM_DEVICE_FSM_H
#include "xstatus.h"
#include "xpm_fsm.h"
#ifdef __cplusplus
extern "C" {
#endif
extern const XPm_Fsm XPmGenericDeviceFsm;
XStatus XPmDeviceFsm_Init(XPmRuntime_DeviceOps* const DevOps);
#ifdef __cplusplus
}
#endif
#endif /* XPM_DEVICE_FSM_H */
