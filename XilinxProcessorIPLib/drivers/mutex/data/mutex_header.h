/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef MUTEX_HEADER_H		/* prevent circular inclusions */
#define MUTEX_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xstatus.h"

#ifndef SDT
int MutexExample(u16 MutexDeviceID);
#else
int MutexExample (XMutex *MutexInstPtr, UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif

