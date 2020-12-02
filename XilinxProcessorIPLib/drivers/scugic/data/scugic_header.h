/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SCUGIC_HEADER_H		/* prevent circular inclusions */
#define SCUGIC_HEADER_H		/* by using protection macros */

#include "xil_assert.h"
#include "xil_types.h"
#include "xstatus.h"

int ScuGicSelfTestExample(u16 DeviceId);
int ScuGicInterruptSetup(XScuGic *IntcInstancePtr, u16 DeviceId);

#endif
