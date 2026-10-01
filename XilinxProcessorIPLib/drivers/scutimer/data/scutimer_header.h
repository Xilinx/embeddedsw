/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
* @file scutimer_header.h
* @brief Contains prototypes for the scutimer example test functions.
*/

#ifndef SCUTIMER_HEADER_H
#define SCUTIMER_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
