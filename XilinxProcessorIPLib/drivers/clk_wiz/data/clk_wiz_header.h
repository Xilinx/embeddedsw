/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XCLK_WIZ_HEADER_H
#define XCLK_WIZ_HEADER_H

/*****************************************************************************/
/**
*
* @file clk_wiz_header.h
*
* @brief Declares prototypes for the CLK_WIZ driver examples.
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/** @cond INTERNAL */
u32 ClkWiz_IntrExample(u32 DeviceId);
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif
