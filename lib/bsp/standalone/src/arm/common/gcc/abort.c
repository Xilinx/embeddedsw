/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file abort.c
 * @brief Abort stub implementation for standalone BSP.
 */


#include <stdlib.h>
#include <unistd.h>

/*
 * abort -- go out via exit...
 */
/** @brief Stub for abort(); loops indefinitely as there is no OS signal support. */
__attribute__((weak)) void abort(void)
{
  _exit(1);
}
