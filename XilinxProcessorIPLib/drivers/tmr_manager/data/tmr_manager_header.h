/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef TMRMANAGER_HEADER_H		/* prevent circular inclusions */
#define TMRMANAGER_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int TMR_ManagerSelfTestExample(u16 DeviceId);
#else
int TMR_ManagerSelfTestExample(UINTPTR BaseAddress);
int TMR_ManagerIntrExample(XTMR_Manager *TMR_ManagerInstancePtr,
			   UINTPTR BaseAddress);
#endif


#ifdef __cplusplus
}
#endif
#endif

