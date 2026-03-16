/*
 * Copyright (c) 2016-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "metal-test-internal.h"
#include <zephyr/kernel.h>
#include <metal/log.h>
#include <zephyr/sys/printk.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define BLK_SIZE_MAX 1024
#define BLK_NUM_MAX 16

K_HEAP_DEFINE(kmpool, BLK_SIZE_MAX * BLK_NUM_MAX);
static void *block[BLK_NUM_MAX];

extern int init_system(void);
extern void metal_generic_default_poll(void);

extern void metal_test_add_alloc();
extern void metal_test_add_atomic();
extern void metal_test_add_irq();
extern void metal_test_add_mutex();

extern void *metal_zephyr_allocate_memory(unsigned int size)
{
	int i;

	for (i = 0; i < sizeof(block)/sizeof(block[0]); i++) {
		if (!block[i]) {
			block[i] = k_heap_alloc(&kmpool, size, K_NO_WAIT);
			if (!block[i])
				printk("Failed to alloc 0x%x memory.\n", size);
			return block[i];
		}
	}

	printk("Failed to alloc 0x%x memory, no free blocks.\n", size);
	return NULL;
}

extern void metal_zephyr_free_memory(void *ptr)
{
	int i;

	for (i = 0; i < sizeof(block)/sizeof(block[0]); i++) {
		if (block[i] == ptr) {
			k_heap_free(&kmpool, block[i]);
			block[i] = NULL;
			return;
		}
	}
	printk("Failed to free %p, no block matches.\n", ptr);
}

void metal_test_add_functions()
{
	metal_test_add_alloc();
	metal_test_add_atomic();
	metal_test_add_mutex();
}

int main(void)
{
	metal_test_add_functions();
	(void)metal_tests_run(NULL);

	while (1)
               metal_generic_default_poll();

	/* will not return, but quiet the compiler */
	return 0;
}
