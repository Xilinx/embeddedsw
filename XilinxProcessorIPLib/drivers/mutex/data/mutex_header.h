/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef MUTEX_HEADER_H
#define MUTEX_HEADER_H

/*****************************************************************************/
/**
*
* @file mutex_header.h
*
* @brief Declares prototypes for the Mutex driver examples.
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xstatus.h"

/** @cond INTERNAL */
#ifndef SDT
int MutexExample(u16 MutexDeviceID);
#else
int MutexExample (XMutex *MutexInstPtr, UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif

