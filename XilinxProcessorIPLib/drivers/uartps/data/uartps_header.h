/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTPS_HEADER_H		/* prevent circular inclusions */
#define UARTPS_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int UartPsPolledExample(u16 DeviceId);
#else
int UartPsPolledExample(XUartPs *UartInstPtr, UINTPTR BaseAddress);
int UartPsIntrExample(XUartPs *UartInstPtr, UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif
#endif
