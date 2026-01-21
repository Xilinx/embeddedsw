/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
*
* SPDX-License-Identifier: MIT
*
******************************************************************************/
#include "xil_types.h"

/* Stub for seek sys-call */
__attribute__((weak)) s32 _sys_seek(__attribute__((unused)) s32 fh,
                                    __attribute__((unused)) long pos)
{
    return 0;
}
