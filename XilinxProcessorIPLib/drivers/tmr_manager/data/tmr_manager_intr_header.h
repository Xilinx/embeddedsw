/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef TMRMANAGER_INTR_HEADER_H		/* prevent circular inclusions */
#define TMRMANAGER_INTR_HEADER_H		/* by using protection macros */


#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
int TMR_ManagerIntrExample(XIntc* IntcInstancePtr, \
                            XTMR_Manager* TMRManagerInstancePtr, \
                            u16 TMRManagerDeviceId, \
                            u16 TMRManagerIntrId);
#elif defined(XPAR_IOMODULE_0_DEVICE_ID)
int TMR_ManagerIntrExample(XIOModule* IntcInstancePtr, \
                            XTMR_Manager* TMRManagerInstancePtr, \
                            u16 TMRManagerDeviceId, \
                            u16 TMRManagerIntrId);
#else
int TMR_ManagerIntrExample(XScuGic* IntcInstancePtr, \
                            XTMR_Manager* TMRManagerInstancePtr, \
                            u16 TMRManagerDeviceId, \
                            u16 TMRManagerIntrId);

#endif
#endif
