/******************************************************************************
* Copyright (C) 2003 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef GPIO_HEADER_H
#define GPIO_HEADER_H

/*****************************************************************************/
/**
*
* @file gpio_header.h
*
* @brief Declares prototypes for the GPIO driver examples.
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/** @cond INTERNAL */
#ifndef SDT
int GpioOutputExample(u16 DeviceId, u32 GpioWidth);
int GpioInputExample(u16 DeviceId, u32 *DataRead);
#else
int GpioOutputExample(XGpio *GpioOutputPtr, UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif

