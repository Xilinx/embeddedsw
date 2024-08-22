/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef ZDMA_HEADER_H		/* prevent circular inclusions */
#define ZDMA_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int XZDma_SelfTestExample(u16 DeviceId);
#ifdef XPAR_SCUGIC_0_DEVICE_ID
int XZDma_SimpleExample(XScuGic *IntcInstPtr, XZDma *ZdmaInstPtr,
			u16 DeviceId, u16 IntrId);
#else
#ifdef XPAR_INTC_0_DEVICE_ID
int XZDma_SimpleExample(XIntc *IntcInstPtr, XZDma *ZdmaInstPtr,
			u16 DeviceId, u16 IntrId);
#endif
#endif
#else
int XZDma_SelfTestExample(UINTPTR BaseAddress);
int XZDma_SimpleExample(XZDma *ZdmaInstPtr,
			UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif
