/******************************************************************************
*
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
******************************************************************************/

/**
* @file inbyte.c
* @brief Character input byte stub for standalone BSP.
*/

#include "bspconfig.h"

#ifdef __cplusplus
extern "C" {
#endif
char inbyte(void);

#ifdef __cplusplus
}
#endif

#if !defined(STDIN_BASEADDRESS)
char inbyte(void) {
    return (0);
}
#endif
