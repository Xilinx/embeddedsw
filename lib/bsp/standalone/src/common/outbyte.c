/******************************************************************************
*
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
******************************************************************************/
#include "bspconfig.h"
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif
void outbyte(char c);

#ifdef __cplusplus
}
#endif

#if !defined(VERSAL_PLM) && !defined(STDOUT_BASEADDRESS) && !defined(SPARTANUP_PLM) && !defined(ASUFW)
void outbyte(char c)
{
    (void) c;
}
#endif
