/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include <stdio.h>
#include "xil_types.h"

/************************** Function Prototypes ******************************/

sint32 fcntl (sint32 fd, sint32 cmd, sint32 arg);

/*****************************************************************************/
/**
* fcntl -- Manipulate a file descriptor.
* We don't have a filesystem, so we do nothing.
******************************************************************************/
sint32 fcntl (sint32 fd, sint32 cmd, sint32 arg)
{
  (void) fd;
  (void) cmd;
  (void) arg;
  return 0;
}
