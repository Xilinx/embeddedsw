/******************************************************************************
* Copyright (c) 2008 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __MEMORY_CONFIG_H_
#define __MEMORY_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
struct memory_range_s {
	char8 *name;
	char8 *ip;
	UINTPTR base;
	u64 size;
};

#define UPPER_4BYTES_MASK	0xFFFFFFFF00000000LL
#define LOWER_4BYTES_MASK	0xFFFFFFFFUL
/* generated memory ranges defined in memory_ranges_g.c */
extern struct memory_range_s memory_ranges[];
extern int n_memory_ranges;

#ifdef __cplusplus
}
#endif

#endif
