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
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * @file	linux/condition.h
 * @brief	Linux condition variable primitives for libmetal.
 */

#ifndef __METAL_CONDITION__H__
#error "Include metal/condition.h instead of metal/linux/condition.h"
#endif

#ifndef __METAL_LINUX_CONDITION__H__
#define __METAL_LINUX_CONDITION__H__

#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <metal/atomic.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

struct metal_condition {
	metal_mutex_t *m; /**< mutex.
	                       The condition variable is attached to
	                       this mutex when it is waiting.
	                       It is also used to check correctness
	                       in case there are multiple waiters. */

	atomic_int waiters;    /**< number of waiters. */
	atomic_int wakeups;    /**< number of wakeups. */
};

/** Static metal condition variable initialization. */
#define METAL_CONDITION_INIT		{ NULL, ATOMIC_VAR_INIT(0), ATOMIC_VAR_INIT(0) }

static inline void metal_condition_init(struct metal_condition *cv)
{
	cv->m = NULL;
	atomic_init(&cv->waiters, 0);
	atomic_init(&cv->wakeups, 0);
}

static inline int metal_condition_signal(struct metal_condition *cv)
{
	if (!cv)
		return -EINVAL;

	atomic_fetch_add(&cv->wakeups, 1);
	if (atomic_load(&cv->waiters) > 0)
		syscall(SYS_futex, &cv->wakeups, FUTEX_WAKE, 1, NULL, NULL, 0);
	return 0;
}

static inline int metal_condition_broadcast(struct metal_condition *cv)
{
	if (!cv)
		return -EINVAL;

	atomic_fetch_add(&cv->wakeups, 1);
	if (atomic_load(&cv->waiters) > 0)
		syscall(SYS_futex, &cv->wakeups, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
	return 0;
}

#endif /* __METAL_LINUX_CONDITION__H__ */
