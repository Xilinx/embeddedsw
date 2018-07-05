/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UNDEFINE_FILE_OPS
#include <errno.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
	__attribute__((weak)) s32 _open(const char8 *buf, s32 flags, s32 mode);
}
#endif

/*
 * _open -- open a file descriptor. We don't have a filesystem, so
 *         we return an error.
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
