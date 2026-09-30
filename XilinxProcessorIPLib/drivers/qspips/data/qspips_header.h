/******************************************************************************
* Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef QSPIPS_HEADER_H
#define QSPIPS_HEADER_H

/******************************************************************************/
/**
*
* @file qspips_header.h
*
* @brief Declares prototypes for the QSPIPS driver examples.
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
int QspiPsSelfTestExample(u16 DeviceId);
#else
int QspiPsSelfTestExample(UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif
#endif
