/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef CSUDMA_HEADER_H		/* prevent circular inclusions */
#define CSUDMA_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int XCsuDma_SelfTestExample(u16 DeviceId);
#ifdef XPAR_SCUGIC_0_DEVICE_ID
int XCsuDma_IntrExample(XScuGic *IntcInstancePtr, XCsuDma *CsuDmaInstance,
			u16 DeviceId, u16 IntrId);
#else
#ifdef XPAR_INTC_0_DEVICE_ID
int XCsuDma_IntrExample(XIntc *IntcInstancePtr, XCsuDma *CsuDmaInstance,
			u16 DeviceId, u16 IntrId);
#endif
#endif
#endif
