/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SCUGIC_HEADER_H		/* prevent circular inclusions */
#define SCUGIC_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_assert.h"
#include "xil_types.h"
#include "xstatus.h"

#ifndef SDT
int ScuGicSelfTestExample(u16 DeviceId);
int ScuGicInterruptSetup(XScuGic *IntcInstancePtr, u16 DeviceId);
#else
int ScuGicSelfTestExample(u32 BaseAddr);
int ScuGicInterruptSetup(XScuGic *IntcInstancePtr, u32 BaseAddr);
#endif

#ifdef __cplusplus
}
#endif

#endif
