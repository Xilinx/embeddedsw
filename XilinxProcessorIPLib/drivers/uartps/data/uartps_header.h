/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTPS_HEADER_H
#define UARTPS_HEADER_H

/*****************************************************************************/
/**
*
* @file uartps_header.h
*
* @brief Declares prototypes for the UartPs driver examples.
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
int UartPsPolledExample(u16 DeviceId);
#else
int UartPsPolledExample(XUartPs *UartInstPtr, UINTPTR BaseAddress);
int UartPsIntrExample(XUartPs *UartInstPtr, UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif
#endif
