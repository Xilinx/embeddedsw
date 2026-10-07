/******************************************************************************
* Copyright (c) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file errno.c
 * @brief Errno variable stub for standalone BSP.
 */


#include <errno.h>
#include <reent.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The errno variable is stored in the reentrancy structure.  This
 * function returns its address for use by the macro errno defined in errno.h.
 * @return Pointer to the errno value in the reentrancy structure.
 */
__attribute__((weak)) sint32 * __errno (void);

#ifdef __cplusplus
}
#endif

__attribute__((weak)) sint32 *
__errno (void)
{
  return &_REENT->_errno;
}
