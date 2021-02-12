/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTPS_INTR_HEADER_H		/* prevent circular inclusions */
#define UARTPS_INTR_HEADER_H		/* by using protection macros */


#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
int UartPsIntrExample(XIntc *IntcInstPtr, XUartPs *UartInstPtr,
			u16 DeviceId, u16 UartIntrId);
#else
int UartPsIntrExample(XScuGic *IntcInstPtr, XUartPs *UartInstPtr,
			u16 DeviceId, u16 UartIntrId);
#endif

#endif
