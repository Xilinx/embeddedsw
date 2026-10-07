/******************************************************************************
* Copyright (c) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file _profile_clean.c
* @brief Profiling cleanup for standalone BSP.
*/



#include "profile.h"
#include "_profile_timer_hw.h"
#include "xil_exception.h"

/** @brief Disable profiling interrupts and the profiling timer. */
void _profile_clean( void );

/*
 * This function is the exit routine and is called by the crtinit, when the
 * program terminates. The name needs to be changed later..
 */
/** @brief Disable profiling interrupts and the profiling timer. */
void _profile_clean( void )
{
	Xil_ExceptionDisable();
	disable_timer();
}
