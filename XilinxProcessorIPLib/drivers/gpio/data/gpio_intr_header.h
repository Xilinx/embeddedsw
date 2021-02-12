/******************************************************************************
* Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef GPIO_INTR_HEADER_H		/* prevent circular inclusions */
#define GPIO_INTR_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
int GpioIntrExample(XIntc* IntcInstancePtr,
                        XGpio* InstancePtr,
                        u16 DeviceId,
                        u16 IntrId,
                        u16 IntrMask,
                        u32 *DataRead);
#else
int GpioIntrExample(XScuGic* IntcInstancePtr,
                        XGpio* InstancePtr,
                        u16 DeviceId,
                        u16 IntrId,
                        u16 IntrMask,
                        u32 *DataRead);

#endif
#endif

