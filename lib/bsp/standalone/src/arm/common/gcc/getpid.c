/******************************************************************************
* Copyright (c) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file getpid.c
 * @brief getpid stub implementation for standalone BSP.
 */


#include "xil_types.h"
/*
 * getpid -- only one process, so just return 1.
 */
#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) s32 _getpid(void);
__attribute__((weak)) s32 getpid(void);

#ifdef __cplusplus
}
#endif

/**
 * @brief Stub for getpid(); always returns 1.
 * @return Always 1 (single process).
 */
__attribute__((weak)) s32 getpid(void)
{
  return 1;
}

/**
 * @brief Stub for _getpid(); always returns 1.
 * @return Always 1 (single process).
 */
__attribute__((weak)) s32 _getpid(void)
{
  return 1;
}
