/******************************************************************************
* Copyright (C) 2008 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SPI_INTR_HEADER_H		/* prevent circular inclusions */
#define SPI_INTR_HEADER_H		/* by using protection macros */


#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
int SpiIntrExample(XIntc *IntcInstancePtr, \
                       XSpi *SpiInstancePtr, \
                       u16 SpiDeviceId, \
                       u16 SpiIntrId);
#else
int SpiIntrExample(XScuGic *IntcInstancePtr, \
                       XSpi *SpiInstancePtr, \
                       u16 SpiDeviceId, \
                       u16 SpiIntrId);

#endif
#endif

