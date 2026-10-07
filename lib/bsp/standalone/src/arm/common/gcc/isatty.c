/******************************************************************************
* Copyright (c) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file isatty.c
 * @brief isatty stub implementation for standalone BSP.
 */

#include <unistd.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) sint32 _isatty(sint32 fd);

#ifdef __cplusplus
}
#endif

/*
 * isatty -- returns 1 if connected to a terminal device,
 *           returns 0 if not. Since we're hooked up to a
 *           serial port, we'll say yes _AND return a 1.
 */
/**
 * @brief Stub for isatty(); always returns 1.
 * @param[in] fd File descriptor (unused).
 * @return Always 1.
 */
__attribute__((weak)) sint32 isatty(sint32 fd)
{
  (void)fd;
  return (1);
}

/**
 * @brief Stub for _isatty(); always returns 1.
 * @param[in] fd File descriptor (unused).
 * @return Always 1.
 */
__attribute__((weak)) sint32 _isatty(sint32 fd)
{
  (void)fd;
  return (1);
}
