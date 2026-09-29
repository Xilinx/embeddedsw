/******************************************************************************
* Copyright (C) 2006 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTLITE_HEADER_H
#define UARTLITE_HEADER_H

/*****************************************************************************/
/**
*
* @file uartlite_header.h
*
* @brief Declares prototypes for the UartLite driver examples.
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
int UartLiteSelfTestExample(u16 DeviceId);
#else
int UartLiteSelfTestExample(UINTPTR BaseAddress);
int UartLiteIntrExample(XUartLite *UartLiteInstPtr,
			 UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif
#endif

