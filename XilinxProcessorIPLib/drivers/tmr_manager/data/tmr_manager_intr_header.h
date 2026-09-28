/**
 * @file tmr_manager_intr_header.h
 * @brief Header file for the TMR Manager interrupt example application.
 */
/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef TMRMANAGER_INTR_HEADER_H		/* prevent circular inclusions */
#define TMRMANAGER_INTR_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
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
#ifdef __cplusplus
}
#endif
#endif
