/******************************************************************************
* Copyright (C) 2005 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef AXIVDMA_HEADER_H		/* prevent circular inclusions */
#define AXIVDMA_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int AxiVDMASelfTestExample(u16 DeviceId);
#else
int AxiVDMASelfTestExample(UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif
