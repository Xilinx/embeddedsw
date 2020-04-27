/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include <errno.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
	__attribute__((weak)) sint32 unlink(char8 *path);
}
#endif
/*
 * unlink -- since we have no file system,
 *           we just return an error.
 */
__attribute__((weak)) sint32 unlink(char8 *path)
{
  (void) path;
  errno = EIO;
  return (-1);
}
