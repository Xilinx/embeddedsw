/******************************************************************************
* Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTNS550_INTR_HEADER_H		/* prevent circular inclusions */
#define UARTNS550_INTR_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
int UartNs550IntrExample(XIntc* IntcInstancePtr, \
                             XUartNs550* UartLiteInstancePtr, \
                             u16 UartNs550DeviceId, \
                             u16 UartNs550IntrId);
#else
int UartNs550IntrExample(XScuGic* IntcInstancePtr, \
                             XUartNs550* UartLiteInstancePtr, \
                             u16 UartNs550DeviceId, \
                             u16 UartNs550IntrId);

#endif
#endif
