/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file cpputest_time.c
 * @brief CppUTest time implementation for standalone BSP.
 */


#ifndef UNDEFINE_FILE_OPS
#include <errno.h>
#include "xil_types.h"
#include <time.h>
struct tms* tms; /**< Pointer to tms structure (unused stub variable). */
#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) clock_t _times(struct tms* tms);

#ifdef __cplusplus
}
#endif

/**
 * @brief Stub for _times(); reports that timekeeping is unsupported.
 * @param[out] tms Pointer to tms structure (unused).
 * @return Always -1.
 */
__attribute__((weak)) clock_t _times(struct tms* tms)
{
  (void)tms;

  errno = EIO;
  return (-1);
}


#endif
