/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file fcntl.c
* @brief Newlib fcntl() system call stub.
*/


#include <stdio.h>
#include "xil_types.h"

/************************** Function Prototypes ******************************/

sint32 fcntl (sint32 fd, sint32 cmd, sint32 arg);

/*****************************************************************************/
/**
* @brief Manipulate a file descriptor. We don't have a filesystem, so we do nothing.
*
* @param[in] fd  File descriptor (ignored).
* @param[in] cmd Command to perform (ignored).
* @param[in] arg Argument for the command (ignored).
*
* @return Always returns 0.
*
******************************************************************************/
sint32 fcntl (sint32 fd, sint32 cmd, sint32 arg)
{
  (void) fd;
  (void) cmd;
  (void) arg;
  return 0;
}
