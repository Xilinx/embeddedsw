/******************************************************************************
*
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
******************************************************************************/
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
