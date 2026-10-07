/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file errno.c
 * @brief Errno variable stub for standalone BSP.
 */

#include <errno.h>
#include <reent.h>
#include "xil_types.h"

/**
 * The errno variable is stored in the reentrancy structure.  This
 * function returns its address for use by the macro errno defined in errno.h.
 * @return Pointer to the errno value in the reentrancy structure.
 */
sint32 * __errno (void);

sint32 *
__errno (void)
{
  return &_REENT->_errno;
}
