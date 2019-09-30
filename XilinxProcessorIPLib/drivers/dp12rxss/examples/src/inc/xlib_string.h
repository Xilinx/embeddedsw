/*******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
