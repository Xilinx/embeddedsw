/******************************************************************************
* Copyright (c) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file fcntl.c
 * @brief File control stub implementation for standalone BSP.
 */


#include <stdio.h>
#include "xil_types.h"

__attribute__((weak)) sint32 fcntl (sint32 fd, sint32 cmd, LONG arg);

/**
 * @brief Manipulate a file descriptor. We don't have a filesystem, so we do nothing.
 * @param[in] fd  File descriptor (ignored).
 * @param[in] cmd Command to perform (ignored).
 * @param[in] arg Argument for the command (ignored).
 * @return Always returns 0.
 */
__attribute__((weak)) sint32 fcntl (sint32 fd, sint32 cmd, LONG arg)
{
  (void)fd;
  (void)cmd;
  (void)arg;
  return 0;
}
