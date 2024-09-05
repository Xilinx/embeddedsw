/******************************************************************************
* Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef QSPIPS_HEADER_H		/* prevent circular inclusions */
#define QSPIPS_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int QspiPsSelfTestExample(u16 DeviceId);
#else
int QspiPsSelfTestExample(UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif
#endif
