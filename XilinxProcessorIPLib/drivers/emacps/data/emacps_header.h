/******************************************************************************
* Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef EMACPS_HEADER_H		/* prevent circular inclusions */
#define EMACPS_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef SDT
int EmacPsDmaIntrExample(XEmacPs *EmacPsInstancePtr, UINTPTR BaseAddress);
#elif XPAR_INTC_0_DEVICE_ID
int EmacPsDmaIntrExample(XIntc *IntcInstancePtr,
			  XEmacPs *EmacPsInstancePtr,
			  u16 EmacPsDeviceId);
#else
int EmacPsDmaIntrExample(XScuGic *IntcInstancePtr,
			  XEmacPs *EmacPsInstancePtr,
			  u16 EmacPsDeviceId);
#endif

#endif
