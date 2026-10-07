/******************************************************************************
* Copyright (c) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file unlink.c
 * @brief unlink stub implementation for standalone BSP.
 */


#include <errno.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) sint32 unlink(char8 *path);

#ifdef __cplusplus
}
#endif
/*
 * unlink -- since we have no file system,
 *           we just return an error.
 */
/**
 * @brief Stub for unlink(); always returns -1 (not supported).
 * @param[in] path Path of file to remove (unused).
 * @return Always -1.
 */
__attribute__((weak)) sint32 unlink(char8 *path)
{
  (void) path;
  errno = EIO;
  return (-1);
}
