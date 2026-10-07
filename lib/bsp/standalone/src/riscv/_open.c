/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file _open.c
 * @brief open stub implementation for the RISC-V standalone BSP.
 */

#ifndef UNDEFINE_FILE_OPS
#include <errno.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) s32 _open(const char8 *buf, s32 flags, s32 mode);

#ifdef __cplusplus
}
#endif

/*
 * _open -- open a file descriptor. We don't have a filesystem, so
 *         we return an error.
 */
/**
 * @brief Stub for _open(); always returns -1 (not supported).
 * @param[in] buf Filename string (unused).
 * @param[in] flags Open flags (unused).
 * @param[in] mode File mode (unused).
 * @return Always -1.
 */
__attribute__((weak)) s32 _open(const char8 *buf, s32 flags, s32 mode)
{
  (void)buf;
  (void)flags;
  (void)mode;
  errno = EIO;
  return (-1);
}
#endif
