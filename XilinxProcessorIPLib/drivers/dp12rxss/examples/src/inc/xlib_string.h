/* ***************************************************************************
 * ** Copyright (c) 2012-2013 Xilinx, Inc.  All rights reserved.            **
 * **   ____  ____                                                          **
 * **  /   /\/   /                                                          **
 * ** /___/  \  /   Vendor: Xilinx                                          **
 * ** \   \   \/                                                            **
 * **  \   \                                                                **
 * **  /   /                                                                **
 * ** /___/   /\                                                            **
 * ** \   \  /  \                                                           **
 * **  \___\/\___\                                                          **
 * **                                                                       **
 * ** XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"         **
 * ** AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND       **
 * ** SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,        **
 * ** OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,        **
 * ** APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION           **
 * ** THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,     **
 * ** AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE      **
 * ** FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY              **
 * ** WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE               **
 * ** IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR        **
 * ** REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF       **
 * ** INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       **
 * ** FOR A PARTICULAR PURPOSE.                                             **
 * **                                                                       **
 * **************************************************************************/


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
