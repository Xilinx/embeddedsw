/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef IICPS_HEADER_H
#define IICPS_HEADER_H

/*****************************************************************************/
/**
*
* @file iicps_header.h
*
* @brief Declares prototypes for the IICPS driver examples.
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
int IicPsSelfTestExample(u16 DeviceId);
#else
int IicPsSelfTestExample(UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif
