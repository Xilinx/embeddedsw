/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __XV_WARP_INIT_UTILS_H__
#define __XV_WARP_INIT_UTILS_H__

/************************** Constant Definitions *****************************/
#define USE_BUILTIN_CLZ 0
#define DATA_SIZE 16 /* Number of bits for x and y */
#define LUT_BITS 12 /* Look up table size = 2^LUT_BITS*/
#define INV_BIT_SHIFT DATA_SIZE - LUT_BITS - 1
#define LOOP_COUNT	0x7E9000 /*3840x2160*/

/**************************** Function Prototypes *****************************/
unsigned int XVWarpInit_ExponentialPos(unsigned short gval);
unsigned int XVWarpInit_ExponentialNeg(unsigned short gval);
unsigned int XVWarpInit_Sqrt(unsigned int v);
unsigned char XVWarpInit_DominantBit(unsigned int val);
unsigned short XVWarpInit_Inverse(unsigned short x, int M, char* N);
unsigned int XVWarpInit_ExponentialFact(unsigned exp_value, short k_type,
		unsigned int Q_fact, char *Q_exp);

#endif /* __XV_WARP_INIT_UTILS_H__ */
