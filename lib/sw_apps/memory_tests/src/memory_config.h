/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __MEMORY_CONFIG_H_
#define __MEMORY_CONFIG_H_

struct memory_range_s {
    char *name;
    char *ip;
    unsigned long base;
    unsigned long size;
};

/* generated memory ranges defined in memory_ranges_g.c */
extern struct memory_range_s memory_ranges[];
extern int n_memory_ranges;

#endif
