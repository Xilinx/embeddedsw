/******************************************************************************
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XAXICDMA_HEADER_H		/* prevent circular inclusions */
#define XAXICDMA_HEADER_H               /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int XAxiCdma_SgIntrExample(XAxiCdma *InstancePtr, UINTPTR BaseAddress);
int XAxiCdma_SgPollExample(XAxiCdma *InstancePtr, UINTPTR BaseAddress);
int XAxiCdma_SimpleIntrExample(XAxiCdma *InstancePtr, UINTPTR BaseAddress);
int XAxiCdma_SimplePollExample(XAxiCdma *InstancePtr, UINTPTR BaseAddress);

#ifdef __cplusplus
}
#endif

#endif
