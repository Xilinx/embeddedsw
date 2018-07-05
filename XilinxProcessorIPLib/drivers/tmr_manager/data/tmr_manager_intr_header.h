/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef TMRMANAGER_INTR_HEADER_H		/* prevent circular inclusions */
#define TMRMANAGER_INTR_HEADER_H		/* by using protection macros */


#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
int TMRManagerIntrExample(XIntc* IntcInstancePtr, \
                            XTMRManager* TMRManagerInstancePtr, \
                            u16 TMRManagerDeviceId, \
                            u16 TMRManagerIntrId);
#else
int TMRManagerIntrExample(XScuGic* IntcInstancePtr, \
                            XTMRManager* TMRManagerInstancePtr, \
                            u16 TMRManagerDeviceId, \
                            u16 TMRManagerIntrId);

#endif
#endif
