/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_NOTIFIER_PLAT_H_
#define XPM_NOTIFIER_PLAT_H_

#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

XStatus XPmNotifier_PlatHandleSsit(u32 SubsystemId, u32 NodeId, u32 Event, u32 Enable);

#ifdef __cplusplus
}
#endif

#endif /* XPM_NOTIFIER_PLAT_H_ */
