/******************************************************************************
* Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UARTNS550_HEADER_H
#define UARTNS550_HEADER_H

/*****************************************************************************/
/**
*
* @file uartns550_header.h
*
* @brief Declares prototypes for the UartNs550 driver examples.
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
int UartNs550SelfTestExample(u16 DeviceId);
#else
int UartNs550SelfTestExample(UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif
#endif
