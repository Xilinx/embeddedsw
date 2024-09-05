/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XCLK_WIZ_HEADER_H		/* prevent circular inclusions */
#define XCLK_WIZ_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

u32 ClkWiz_IntrExample(u32 DeviceId);

#ifdef __cplusplus
}
#endif

#endif
