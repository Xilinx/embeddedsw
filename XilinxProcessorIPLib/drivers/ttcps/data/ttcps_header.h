/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef TTCPS_HEADER_H		/* prevent circular inclusions */
#define TTCPS_HEADER_H		/* by using protection macros */

#include "xil_assert.h"
#include "xil_types.h"
#include "xstatus.h"

int TmrInterruptExample(XTtcPs *TtcPsInst,u16 DeviceID,u16 TtcTickIntrID,
						XScuGic *InterruptController);
#endif
