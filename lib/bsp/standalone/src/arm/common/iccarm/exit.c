/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file exit.c
 * @brief exit stub implementation for standalone BSP.
 */


#include <stdlib.h>
#include <yfuns.h>
#include "xil_types.h"

/**
 * @brief Stub for exit(); loops indefinitely as there is no OS to return to.
 * @param[in] arg Exit status code (unused).
 */
void exit(sint32 arg)
{
  while(1) {
	;
  }
}
