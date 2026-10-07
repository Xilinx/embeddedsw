/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file _exit.c
* @brief Newlib _exit() system call stub.
*/



#include <unistd.h>
#include "xil_types.h"

/************************** Function Prototypes ******************************/

void _exit (sint32 status);

/*****************************************************************************/
/**
* @brief Simple implementation of _exit. Does not return.
*
* @param[in] status Exit status code (ignored).
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
