/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include <unistd.h>
#include "xil_types.h"

/************************** Function Prototypes ******************************/

void _exit (sint32 status);

/*****************************************************************************/
/**
*_exit - Simple implementation. Does not return.
*
******************************************************************************/
void _exit (sint32 status)
{
  (void) status;
  while (1)
    {
		;
    }
}
