/******************************************************************************
* Copyright (c) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file clock.c
 * @brief clock stub implementation for standalone BSP.
 */


/*
 * This is the default implementation of the "clock" function of the
 * standard library.  It can be replaced with a system-specific
 * implementation.
 *
 * The "clock" function should return the processor time used by the
 * program from some implementation-defined start time.  The value
 * should be such that if divided by the macro CLOCKS_PER_SEC the
 * result should yield the time in seconds.
 *
 * The value "0" means that the processor time is not
 * available.
 *
 */

#include <time.h>


/**
 * @brief Stub for clock(); always returns 0.
 * @return Always 0.
 */
clock_t (clock)(void)
{
  return (0);
}
