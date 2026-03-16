/*
 * Copyright (c) 2016-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include <metal/alloc.h>
#include <metal/errno.h>
#include <metal/log.h>
#include <metal/sys.h>
#include <zephyr/sys/printk.h>
#include "metal-test-internal.h"

static int alloc(void)
{
	void *ptr;

	ptr = metal_allocate_memory(1000);
	if (!ptr) {
		metal_log(METAL_LOG_DEBUG, "failed to allocate memory\n");
		return -errno;
	}

	metal_free_memory(ptr);

	return 0;
}
METAL_ADD_TEST(alloc);
