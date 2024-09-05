/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef CANPS_HEADER_H		/* prevent circular inclusions */
#define CANPS_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int CanPsPolledExample(u16 DeviceId);

#ifdef XPAR_SCUGIC_0_DEVICE_ID
int CanPsIntrExample(XScuGic *IntcInstPtr, XCanPs *CanInstPtr,
			u16 CanDeviceId, u16 CanIntrId);
#else
#ifdef XPAR_INTC_0_DEVICE_ID
int CanPsIntrExample(XIntc *IntcInstPtr, XCanPs *CanInstPtr,
			u16 CanDeviceId, u16 CanIntrId);
#endif
#endif

#else
int CanPsPolledExample(XCanPs *CanInstPtr, UINTPTR BaseAddress);
int CanPsIntrExample(XCanPs *CanInstPtr, UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif
