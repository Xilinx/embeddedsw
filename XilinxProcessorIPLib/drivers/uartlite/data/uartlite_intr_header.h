/******************************************************************************
* Copyright (C) 2006 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTLITE_INTR_HEADER_H		/* prevent circular inclusions */
#define UARTLITE_INTR_HEADER_H		/* by using protection macros */


#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
int UartLiteIntrExample(XIntc* IntcInstancePtr, \
                            XUartLite* UartLiteInstancePtr, \
                            u16 UartLiteDeviceId, \
                            u16 UartLiteIntrId);
#else
int UartLiteIntrExample(XScuGic* IntcInstancePtr, \
                            XUartLite* UartLiteInstancePtr, \
                            u16 UartLiteDeviceId, \
                            u16 UartLiteIntrId);

#endif
#endif
