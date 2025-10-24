/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_NOTIFIER_PLAT_H_
#define XPM_NOTIFIER_PLAT_H_

#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

XStatus XPmNotifier_PlatHandleSsit(u32 SubsystemId, u32 NodeId, u32 Event, u32 Enable);

#ifdef ENABLE_UNREGISTER_ALL_NOTIFIER_SUPPORT
XStatus XPmNotifier_PlatUnregisterAllSsitErr(u32 SubsystemId);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XPM_NOTIFIER_PLAT_H_ */
