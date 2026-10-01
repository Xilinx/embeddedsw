/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef WDTPS_HEADER_H
#define WDTPS_HEADER_H

/*****************************************************************************/
/**
*
* @file wdtps_header.h
*
* @brief Declares prototypes for the WDTPS driver examples.
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
int WdtPsSelfTestExample(u16 DeviceId);
#else
int WdtPsSelfTestExample(UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif
