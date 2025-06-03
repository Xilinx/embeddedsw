/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int BramExample(u16 DeviceId);
#else
int BramExample(XBram* InstancePtr, u16 DeviceId);
int BramIntrExample(XBram* InstancePtr, UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif
