/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTPSV_HEADER_H		/* prevent circular inclusions */
#define UARTPSV_HEADER_H		/* by using protection macros */


#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int UartPsvPolledExample(u16 DeviceId);
#else
int UartPsvPolledExample(XUartPsv *UartInstPtr, UINTPTR BaseAddress);
#endif

#endif
