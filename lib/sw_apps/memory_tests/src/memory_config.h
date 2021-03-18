/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __MEMORY_CONFIG_H_
#define __MEMORY_CONFIG_H_

#include "xil_types.h"
struct memory_range_s {
    char8 *name;
    char8 *ip;
    u64 base;
    u32 size;
};

#define UPPER_4BYTES_MASK	0xFFFFFFFF00000000LL
#define LOWER_4BYTES_MASK	0xFFFFFFFFUL
/* generated memory ranges defined in memory_ranges_g.c */
extern struct memory_range_s memory_ranges[];
extern int n_memory_ranges;

#endif
