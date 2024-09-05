/******************************************************************************
* Copyright (C) 2003 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef GPIO_HEADER_H		/* prevent circular inclusions */
#define GPIO_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int GpioOutputExample(u16 DeviceId, u32 GpioWidth);
int GpioInputExample(u16 DeviceId, u32 *DataRead);
#else
int GpioOutputExample(XGpio *GpioOutputPtr, UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif

