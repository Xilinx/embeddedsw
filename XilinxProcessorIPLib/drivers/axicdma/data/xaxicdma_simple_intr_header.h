/**
 * @file xaxicdma_simple_intr_header.h
 * @brief Declares the simple-mode interrupt example entry point.
 */
/******************************************************************************
* Copyright (C) 2005 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XAXICDMA_SIMPLE_INTR_HEADER_H		/* prevent circular inclusions */
#define XAXICDMA_SIMPLE_INTR_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
int XAxiCdma_SimpleIntrExample(XIntc *IntcInstancePtr, XAxiCdma *InstancePtr,
	u16 DeviceId,u32 IntrId);
#else
int XAxiCdma_SimpleIntrExample(XScuGic *IntcInstancePtr, XAxiCdma *InstancePtr,
	u16 DeviceId,u32 IntrId);
#endif

#ifdef __cplusplus
}
#endif

#endif
