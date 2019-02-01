/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Xilinx nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "metal-test-internal.h"
#include <kernel.h>
#include <metal/log.h>
#include <misc/printk.h>
#include <stdarg.h>
#include <stdio.h>

#define BLK_SIZE_MIN 128
#define BLK_SIZE_MAX 1024
#define BLK_NUM_MIN 64
#define BLK_NUM_MAX 16
#define BLK_ALIGN BLK_SIZE_MIN

K_MEM_POOL_DEFINE(kmpool, BLK_SIZE_MIN, BLK_SIZE_MAX, BLK_NUM_MAX, BLK_ALIGN);
struct k_mem_block block[BLK_NUM_MIN];

extern int init_system(void);
extern void metal_generic_default_poll(void);

extern void metal_test_add_alloc();
extern void metal_test_add_atomic();
extern void metal_test_add_irq();
extern void metal_test_add_mutex();

extern void *metal_zephyr_allocate_memory(unsigned int size)
{
	int i;
	struct k_mem_block *blk;

	for (i = 0; i < sizeof(block)/sizeof(block[0]); i++) {
		blk = &block[i];
		if (!blk->data) {
			if (k_mem_pool_alloc(&kmpool, blk,size, K_NO_WAIT))
				printk("Failed to alloc 0x%x memory.\n", size);
			return blk->data;
		}
	}

	printk("Failed to alloc 0x%x memory, no free blocks.\n", size);
	return NULL;
}

extern void metal_zephyr_free_memory(void *ptr)
{
	int i;
	struct k_mem_block *blk;

	for (i = 0; i < sizeof(block)/sizeof(block[0]); i++) {
		blk = &block[i];
		if (blk->data == ptr) {
			k_mem_pool_free(blk);
			blk->data = NULL;
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

void metal_zephyr_log_handler(enum metal_log_level level,
			       const char *format, ...)
{
	char msg[1024];
	va_list args;
	static const char *level_strs[] = {
		"metal: emergency: ",
		"metal: alert:     ",
		"metal: critical:  ",
		"metal: error:     ",
		"metal: warning:   ",
		"metal: notice:    ",
		"metal: info:      ",
		"metal: debug:     ",
	};

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	if (level <= METAL_LOG_EMERGENCY || level > METAL_LOG_DEBUG)
		level = METAL_LOG_EMERGENCY;

	printk("%s%s", level_strs[level], msg);
}

int main(void)
{
	struct metal_init_params params;

	metal_test_add_functions();

	params.log_handler = metal_zephyr_log_handler;
	(void)metal_tests_run(&params);

	while (1)
               metal_generic_default_poll();

	/* will not return, but quiet the compiler */
	return 0;
}
