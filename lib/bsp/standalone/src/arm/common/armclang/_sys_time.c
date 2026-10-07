/******************************************************************************
* Copyright (c) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file _sys_time.c
 * @brief Semihosting time stub for ARM compilers.
 */

#include <time.h>

/*
 * clock -- It supposed to return processor time. We are not implementing
 *          this function, as timekeeping is tightly coupled with system, hence
 *          always returning 0. Users can override this with their system
 *          specific implementation.
 *
 */
/**
 * @brief Stub for clock(); always returns 0 as timekeeping is system-specific.
 * @return Always 0.
 */
__attribute__((weak)) clock_t clock(void)
{
    return (0);
}
