/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SCUTIMER_HEADER_H		/* prevent circular inclusions */
#define SCUTIMER_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int ScuTimerPolledExample(u16 DeviceId);
int ScuTimerIntrExample(XScuGic *IntcInstancePtr, XScuTimer *TimerInstancePtr,
			u16 TimerDeviceId, u16 TimerIntrId);

#endif
