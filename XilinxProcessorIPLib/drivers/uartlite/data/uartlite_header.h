/******************************************************************************
* Copyright (C) 2006 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTLITE_HEADER_H		/* prevent circular inclusions */
#define UARTLITE_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int UartLiteSelfTestExample(u16 DeviceId);
#else
int UartLiteSelfTestExample(UINTPTR BaseAddress);
int UartLiteIntrExample(XUartLite *UartLiteInstPtr,
			 UINTPTR BaseAddress);
#endif

#endif

