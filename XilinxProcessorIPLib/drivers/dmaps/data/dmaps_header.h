/******************************************************************************
* Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef DMAPS_HEADER_H		/* prevent circular inclusions */
#define DMAPS_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int XDmaPs_Example_W_Intr(XScuGic *GicPtr, u16 DeviceId);
#else
int XDmaPs_Example_W_Intr(XDmaPs *DmapsInstPtr, UINTPTR BaseAddress);
#endif
#endif

