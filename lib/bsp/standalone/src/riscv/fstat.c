/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file fstat.c
 * @brief File status stub implementation for standalone BSP.
 */


#include <sys/stat.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) s32 _fstat(s32 fd, struct stat *buf);

#ifdef __cplusplus
}
#endif
/*
 * fstat -- Since we have no file system, we just return an error.
 */
/**
 * @brief Stub for _fstat(); marks file as character device.
 * @param[in] fd File descriptor (unused).
 * @param[out] buf Stat structure to fill; st_mode is set to S_IFCHR.
 * @return Always 0.
 */
__attribute__((weak)) s32 _fstat(s32 fd, struct stat *buf)
{
  (void)fd;
  buf->st_mode = S_IFCHR; /* Always pretend to be a tty */

  return (0);
}
