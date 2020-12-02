/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef CANPS_HEADER_H		/* prevent circular inclusions */
#define CANPS_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int CanPsPolledExample(u16 DeviceId);

#ifdef XPAR_SCUGIC_0_DEVICE_ID
int CanPsIntrExample(XScuGic *IntcInstPtr, XCanPs *CanInstPtr,
			u16 CanDeviceId, u16 CanIntrId);
#else
#ifdef XPAR_INTC_0_DEVICE_ID
int CanPsIntrExample(XIntc *IntcInstPtr, XCanPs *CanInstPtr,
			u16 CanDeviceId, u16 CanIntrId);
#endif
#endif
#endif
