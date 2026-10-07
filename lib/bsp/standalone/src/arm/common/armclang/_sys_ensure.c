/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
*
* SPDX-License-Identifier: MIT
*
******************************************************************************/
/**
 * @file _sys_ensure.c
 * @brief Semihosting ensure stub for ARM compilers.
 */
#include "xil_types.h"

/* Stub for ensure sys-call */
/**
 * @brief Stub for semihosting ensure; always returns 0 (success).
 * @param[in] fh File handle (unused).
 * @return Always 0.
 */
__attribute__((weak)) s32 _sys_ensure(__attribute__((unused)) s32 fh)
{
    return 0;
}
