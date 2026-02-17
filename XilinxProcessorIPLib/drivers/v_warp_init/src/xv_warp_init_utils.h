/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xv_warp_init_utils.h
 * @addtogroup v_warp_init Overview
 */

#ifndef __XV_WARP_INIT_UTILS_H__
#define __XV_WARP_INIT_UTILS_H__

/************************** Constant Definitions *****************************/
/** Use built-in compiler intrinsic for count leading zeros (0=disabled, 1=enabled) */
#define USE_BUILTIN_CLZ 0

/** Number of bits for x and y coordinates */
#define DATA_SIZE 16

/** Look up table size = 2^LUT_BITS */
#define LUT_BITS 12

/** Inverse bit shift calculation for lookup table indexing */
#define INV_BIT_SHIFT DATA_SIZE - LUT_BITS - 1

/** Loop count for 3840x2160 resolution */
#define LOOP_COUNT	0x7E9000

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Function Prototypes *****************************/
unsigned int XVWarpInit_ExponentialPos(unsigned short gval);
unsigned int XVWarpInit_ExponentialNeg(unsigned short gval);
unsigned int XVWarpInit_Sqrt(unsigned int v);
unsigned char XVWarpInit_DominantBit(unsigned int val);
unsigned short XVWarpInit_Inverse(unsigned short x, int M, char* N);
unsigned int XVWarpInit_ExponentialFact(unsigned exp_value, short k_type,
		unsigned int Q_fact, char *Q_exp);

#endif /* __XV_WARP_INIT_UTILS_H__ */
