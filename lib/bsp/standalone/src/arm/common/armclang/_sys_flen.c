/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
*
* SPDX-License-Identifier: MIT
*
******************************************************************************/
/**
 * @file _sys_flen.c
 * @brief Semihosting flen stub for ARM compilers.
 */
#include "xil_types.h"

/* Stub for flen sys-call */
/**
 * @brief Stub for semihosting flen; always returns 0.
 * @param[in] fh File handle (unused).
 * @return Always 0.
 */
__attribute__((weak)) long _sys_flen(__attribute__((unused)) s32 fh)
{
    return 0;
}
