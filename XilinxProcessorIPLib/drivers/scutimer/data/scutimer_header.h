/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SCUTIMER_HEADER_H		/* prevent circular inclusions */
#define SCUTIMER_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int ScuTimerPolledExample(u16 DeviceId);
#ifdef XPAR_SCUGIC_0_DEVICE_ID
int ScuTimerIntrExample(XScuGic *IntcInstancePtr, XScuTimer *TimerInstancePtr,
			u16 TimerDeviceId, u16 TimerIntrId);
#endif
#else
int ScuTimerPolledExample(XScuTimer *TimerInstancePtr, UINTPTR BaseAddress);
int ScuTimerIntrExample(XScuTimer *TimerInstancePtr, UINTPTR BaseAddress);
#endif
#endif
