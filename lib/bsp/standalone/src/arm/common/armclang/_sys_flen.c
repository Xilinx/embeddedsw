/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
*
* SPDX-License-Identifier: MIT
*
******************************************************************************/
#include "xil_types.h"

/* Stub for flen sys-call */
__attribute__((weak)) long _sys_flen(__attribute__((unused)) s32 fh)
{
    return 0;
}
