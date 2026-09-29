/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTPSV_HEADER_H
#define UARTPSV_HEADER_H

/*****************************************************************************/
/**
*
* @file uartpsv_header.h
*
* @brief Declares prototypes for the UartPsv driver examples.
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
int UartPsvPolledExample(u16 DeviceId);
#else
int UartPsvPolledExample(XUartPsv *UartInstPtr, UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif
#endif
