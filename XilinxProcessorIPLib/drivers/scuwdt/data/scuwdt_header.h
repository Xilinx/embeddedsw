/******************************************************************************
* Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SCUWDT_HEADER_H		/* prevent circular inclusions */
#define SCUWDT_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int ScuWdtIntrExample(XScuGic *IntcInstancePtr, XScuWdt *WdtInstancePtr,
		   u16 WdtDeviceId, u16 WdtIntrId);
#else
int ScuWdtIntrExample(XScuWdt *WdtInstancePtr, UINTPTR BaseAddress);
int ScuWdtPolledExample(XScuWdt *WdtInstancePtr, UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif
#endif
