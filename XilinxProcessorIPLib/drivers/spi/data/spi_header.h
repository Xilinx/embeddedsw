/******************************************************************************
* Copyright (C) 2007 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SPI_HEADER_H		/* prevent circular inclusions */
#define SPI_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int SpiSelfTestExample(u16 DeviceId);

#ifdef __cplusplus
}
#endif
#endif

