/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file close.c
 * @brief File close stub implementation for standalone BSP.
 */


#include <yfuns.h>
#include "xil_types.h"

/**
 * @brief Stub for __close(); always returns 0 (pretend success).
 * @param[in] fd File descriptor (unused).
 * @return Always 0.
 */
sint32 __close(sint32 fd)
{
  (void)fd;
  return (0);

}
