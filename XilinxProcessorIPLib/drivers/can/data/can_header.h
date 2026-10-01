/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef CAN_HEADER_H
#define CAN_HEADER_H

/*****************************************************************************/
/**
*
* @file can_header.h
*
* @brief Declares prototypes for the CAN driver examples.
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
int XCanPolledExample(u16 DeviceId);
#else
int XCanPolledExample(XCan *Can, UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif
