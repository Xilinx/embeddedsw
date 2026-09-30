/******************************************************************************
* Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SPIPS_HEADER_H
#define SPIPS_HEADER_H

/******************************************************************************/
/**
*
* @file spips_header.h
*
* @brief Declares prototypes for the SPIPS driver examples.
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
int SpiPsSelfTestExample(u16 DeviceId);
#else
int SpiPsSelfTestExample(UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif
#endif
