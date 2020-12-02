/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef UNDEFINE_FILE_OPS
#include "xil_types.h"
#ifdef __cplusplus
extern "C" {
	__attribute__((weak)) s32 _close(s32 fd);
}
#endif

/*
 * close -- We don't need to do anything, but pretend we did.
 */

__attribute__((weak)) s32 _close(s32 fd)
{
  (void)fd;
  return (0);
}
#endif
