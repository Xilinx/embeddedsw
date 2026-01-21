/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
*
* SPDX-License-Identifier: MIT
*
******************************************************************************/
#include "xil_types.h"

/* Stub for ensure sys-call */
__attribute__((weak)) s32 _sys_ensure(__attribute__((unused)) s32 fh)
{
    return 0;
}
