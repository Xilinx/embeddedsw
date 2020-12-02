/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_types.h"
/*
 * getpid -- only one process, so just return 1.
 */
#ifdef __cplusplus
extern "C" {
	__attribute__((weak)) s32 _getpid(void);
}
#endif

__attribute__((weak)) s32 getpid(void)
{
  return 1;
}

__attribute__((weak)) s32 _getpid(void)
{
  return 1;
}
