/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file remove.c
 * @brief remove stub implementation for standalone BSP.
 */


/*
 *
 * The "remove" function should remove the file named "filename".  It
 * should return 0 on success and nonzero on failure.
 *
 */

#include <stdio.h>
#include "xil_types.h"

sint32 remove(const char8 * filename);

/**
 * @brief Stub for remove(); always returns 0 (pretend success).
 * @param[in] filename Name of the file to remove (unused).
 * @return Always 0.
 */
sint32 remove(const char8 * filename)
{
  return 0;
}
