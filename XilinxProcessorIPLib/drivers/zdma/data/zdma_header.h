/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef ZDMA_HEADER_H		/* prevent circular inclusions */
#define ZDMA_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

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
#endif
