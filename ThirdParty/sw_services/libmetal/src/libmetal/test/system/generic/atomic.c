/*
 * Copyright (c) 2016-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include <metal-test.h>
#include <metal/atomic.h>
#include <metal/log.h>
#include <metal/sys.h>

static const int atomic_test_count = 1000;

static int atomic(void)
{
	atomic_int counter = ATOMIC_VAR_INIT(0);
	int value, error=0, i;

	for (i = 0; i < atomic_test_count; i++) {
		atomic_fetch_add(&counter, 1);
	}

	value = atomic_load(&counter);
	value -= atomic_test_count;
	if (value) {
		metal_log(METAL_LOG_DEBUG, "counter mismatch, delta = %d\n", value);
		error = -1;
	}

	return error;
}
METAL_ADD_TEST(atomic);
