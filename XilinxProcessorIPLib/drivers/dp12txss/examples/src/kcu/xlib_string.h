/*******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


//#define SIMULATION
#define DEBUG_LEVEL 1
#define DEVICE_ID 1 //0->k7;1->v7;2->a7

#include "xil_types.h"

#define DP_INTERRUPT_ENABLE 1
#define NORMAL_MODE 1
#define PRINT_TS 0
#define PRINT_STATUS 1//make it '1' to know detailed Lane Status during training
#define PRINT_EDID 0
#define LLC_TEST_MODE 0
#define AUDIO_RESET_TESTS 0
#define ENABLE_DP159 1

#if (DEBUG_LEVEL >= 4)
#define dbg4_printf xil_printf
#else
#define dbg4_printf do_nothing
#endif

#if (DEBUG_LEVEL >= 3)
#define dbg3_printf xil_printf
#else
#define dbg3_printf do_nothing
#endif

#if (DEBUG_LEVEL >= 2)
#define dbg2_printf xil_printf
#else
#define dbg2_printf do_nothing
#endif

#if (DEBUG_LEVEL >= 1)
#define dbg_printf xil_printf
#define dbg1_printf xil_printf
#else
#define dbg_printf do_nothing
#define dbg1_printf do_nothing
#endif

#if (DEBUG_LEVEL == 0)
#define dbg_printf do_nothing
#endif

#define dbg_llc_printf do_nothing //xil_printf
#define dbg1_llc_printf do_nothing //xil_printf
#define dbg2_llc_printf do_nothing

#define XIL_MAX_LINE_LENGTH 256

void do_nothing();

char xil_getc(u32 timeout_ms);

u32 xil_gethex(u8 num_chars);

u32 xil_getdec(u8 num_chars);

void xil_getline (char s[]);
