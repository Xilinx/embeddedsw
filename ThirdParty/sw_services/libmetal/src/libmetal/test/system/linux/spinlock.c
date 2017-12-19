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

#include <pthread.h>

#include "metal-test.h"
#include <metal/log.h>
#include <metal/sys.h>
#include <metal/spinlock.h>

static const int spinlock_test_count = 1000;
static unsigned int total = 0;

static void *spinlock_thread(void *arg)
{
	struct metal_spinlock *l = arg;
	int i;

	for (i = 0; i < spinlock_test_count; i++) {
		metal_spinlock_acquire(l);
		total++;
		metal_spinlock_release(l);
	}

	return NULL;
}

static int spinlock(void)
{
	struct metal_spinlock lock = METAL_SPINLOCK_INIT;
	const int threads = 10;
	int value, error;

	error = metal_run(threads, spinlock_thread, &lock);
	if (!error) {
		value = total;
		value -= spinlock_test_count * threads;
		if (value) {
			metal_log(METAL_LOG_DEBUG, "counter mismatch, delta = %d\n",
				  value);
			error = -EINVAL;
		}
	}

	return error;
}
METAL_ADD_TEST(spinlock);
