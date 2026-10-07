/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
*
* SPDX-License-Identifier: MIT
*
******************************************************************************/
/**
 * @file _sys_seek.c
 * @brief Semihosting seek stub for ARM compilers.
 */
#include "xil_types.h"

/* Stub for seek sys-call */
/**
 * @brief Stub for semihosting seek; always returns 0.
 * @param[in] fh File handle (unused).
 * @param[in] pos Seek position (unused).
 * @return Always 0.
 */
__attribute__((weak)) s32 _sys_seek(__attribute__((unused)) s32 fh,
                                    __attribute__((unused)) long pos)
{
    return 0;
}
