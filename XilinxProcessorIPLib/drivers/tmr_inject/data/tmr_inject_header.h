/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef TMRINJECT_HEADER_H		/* prevent circular inclusions */
#define TMRINJECT_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int TMR_InjectSelfTestExample(u16 DeviceId);

#ifdef __cplusplus
}
#endif
#endif
