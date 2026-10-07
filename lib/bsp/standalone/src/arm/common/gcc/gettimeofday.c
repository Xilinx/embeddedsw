/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file gettimeofday.c
 * @brief gettimeofday stub implementation for standalone BSP.
 */

/* gettimeofday.c -- used to get the time of the day.
 */
#ifndef UNDEFINE_FILE_OPS
#include "xil_types.h"
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Stub for gettimeofday(); always returns 0.
 * @param[in] tp Timeval structure pointer (unused).
 * @param[in] tzp Timezone pointer (unused).
 * @return Always 0.
 */
__attribute__((weak)) int
gettimeofday (struct timeval *restrict tp, void * restrict tzp);

/**
 * @brief Stub for _gettimeofday(); always returns 0.
 * @param[in] tp Timeval structure pointer (unused).
 * @param[in] tzp Timezone pointer (unused).
 * @return Always 0.
 */
__attribute__((weak)) int
_gettimeofday (struct timeval *restrict tp, void * restrict tzp);

#ifdef __cplusplus
}
#endif

/*
 * gettimeofday  -- Used to get the time of the day.
 * Declared as an empty function with __weak attribute to avoid build
 * errors and easy porting.
 */
/**
 * @brief Stub for gettimeofday(); always returns 0.
 * @param[in] tp Timeval structure pointer (unused).
 * @param[in] tzp Timezone pointer (unused).
 * @return Always 0.
 */
__attribute__((weak)) int
gettimeofday (struct timeval *restrict tp, void * restrict tzp)
{
  (void)tp;
  (void)tzp;
  return 0;
}
/**
 * @brief Stub for _gettimeofday(); always returns 0.
 * @param[in] tp Timeval structure pointer (unused).
 * @param[in] tzp Timezone pointer (unused).
 * @return Always 0.
 */
__attribute__((weak)) int
_gettimeofday (struct timeval *restrict tp, void * restrict tzp)
{
  (void)tp;
  (void)tzp;
  return 0;
}
#endif
