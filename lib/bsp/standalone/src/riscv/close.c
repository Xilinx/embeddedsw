/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file close.c
 * @brief File close stub implementation for standalone BSP.
 */

#ifndef UNDEFINE_FILE_OPS
#include "xil_types.h"
#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) s32 _close(s32 fd);

#ifdef __cplusplus
}
#endif

/*
 * close -- We don't need to do anything, but pretend we did.
 */

/**
 * @brief Stub for _close(); always returns 0 (pretend success).
 * @param[in] fd File descriptor (unused).
 * @return Always 0.
 */
__attribute__((weak)) s32 _close(s32 fd)
{
  (void)fd;
  return (0);
}
#endif
