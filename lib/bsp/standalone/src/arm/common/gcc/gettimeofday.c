/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/* gettimeofday.c -- used to get the time of the day.
 */
#ifndef UNDEFINE_FILE_OPS
#include "xil_types.h"
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) int
gettimeofday (struct timeval *restrict tp, void * restrict tzp);

#ifdef __cplusplus
}
#endif

/*
 * gettimeofday  -- Used to get the time of the day.
 * Declared as an empty function with __weak attribute to avoid build
 * errors and easy porting.
 */
__attribute__((weak)) int
gettimeofday (struct timeval *restrict tp, void * restrict tzp)
{
  (void)tp;
  (void)tzp;
  return 0;
}
#endif
