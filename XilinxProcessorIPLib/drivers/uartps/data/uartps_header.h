/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTPS_HEADER_H		/* prevent circular inclusions */
#define UARTPS_HEADER_H		/* by using protection macros */


#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
int UartPsIntrExample(XIntc *IntcInstPtr, XUartPs *UartInstPtr,
			u16 DeviceId, u16 UartIntrId);
#else
int UartPsIntrExample(XScuGic *IntcInstPtr, XUartPs *UartInstPtr,
			u16 DeviceId, u16 UartIntrId);
#endif
int UartPsPolledExample(u16 DeviceId);
#else
int UartPsPolledExample(XUartPs *UartInstPtr, UINTPTR BaseAddress);
int UartPsIntrExample(XUartPs *UartInstPtr, UINTPTR BaseAddress);
#endif

#endif
