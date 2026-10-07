/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file _exit.c
 * @brief exit stub implementation for ARM compilers.
 */


#include <unistd.h>
#include "xil_types.h"

/**
 * @brief Simple implementation of _exit. Does not return.
 * @param[in] status Exit status code (ignored).
 */
__attribute__((weak)) void _exit (sint32 status)
{
  (void)status;
  while (1) {
	;
  }
}
