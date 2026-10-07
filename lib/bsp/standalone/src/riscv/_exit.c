/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file _exit.c
 * @brief exit stub implementation for the RISC-V standalone BSP.
 */

#include <unistd.h>
#include "xil_types.h"
void _exit (sint32 status);

/**
 * @brief Simple implementation of _exit. Does not return.
 * @param[in] status Exit status code (ignored).
 */
void _exit (sint32 status)
{
  (void) status;
  while (1)
    {
		;
    }
}
