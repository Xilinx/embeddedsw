/******************************************************************************
* Copyright (C) 2012 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef INTC_HEADER_H		/* prevent circular inclusions */
#define INTC_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_assert.h"
#include "xil_types.h"
#include "xstatus.h"

#ifndef SDT
int IntcSelfTestExample(u16 DeviceId);
int IntcInterruptSetup(XIntc *IntcInstancePtr, u16 DeviceId);
#else
int IntcSelfTestExample(UINTPTR BaseAddr);
int IntcInterruptSetup(XIntc *IntcInstancePtr, UINTPTR BaseAddr);
#endif

#ifdef __cplusplus
}
#endif

#endif
