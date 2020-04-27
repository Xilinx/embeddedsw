/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef UNDEFINE_FILE_OPS
#include <errno.h>
#include "xil_types.h"
#include <time.h>
struct tms* tms;
#ifdef __cplusplus
extern "C" {
	__attribute__((weak)) clock_t _times(struct tms* tms);
}
#endif

__attribute__((weak)) clock_t _times(struct tms* tms)
{
  (void)tms;

  errno = EIO;
  return (-1);
}


#endif
