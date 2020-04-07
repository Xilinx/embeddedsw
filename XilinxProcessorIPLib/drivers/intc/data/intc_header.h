/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef INTC_HEADER_H		/* prevent circular inclusions */
#define INTC_HEADER_H		/* by using protection macros */

#include "xil_assert.h"
#include "xil_types.h"
#include "xstatus.h"

int IntcSelfTestExample(u16 DeviceId);
int IntcInterruptSetup(XIntc *IntcInstancePtr, u16 DeviceId);

#endif
