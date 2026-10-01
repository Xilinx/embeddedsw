/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
* @file tmr_inject_header.h
* @brief Contains prototypes for the tmr_inject example test functions.
*/

#ifndef TMRINJECT_HEADER_H
#define TMRINJECT_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int TMR_InjectSelfTestExample(u16 DeviceId);
#else
int TMR_InjectSelfTestExample(UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif
#endif
