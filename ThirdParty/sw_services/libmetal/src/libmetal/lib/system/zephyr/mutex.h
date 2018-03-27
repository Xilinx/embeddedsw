/*
 * Copyright (c) 2017, Linaro Limited. and Contributors. All rights reserved.
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
 * 3. Neither the name of Linaro nor the names of its contributors may be used
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

/*
 * @file	zephyr/mutex.h
 * @brief	Zephyr mutex primitives for libmetal.
 */

#ifndef __METAL_MUTEX__H__
#error "Include metal/mutex.h instead of metal/zephyr/mutex.h"
#endif

#ifndef __METAL_ZEPHYR_MUTEX__H__
#define __METAL_ZEPHYR_MUTEX__H__

#include <metal/atomic.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	atomic_int v;
} metal_mutex_t;

#define METAL_MUTEX_INIT		{ ATOMIC_VAR_INIT(0) }

static inline void metal_mutex_init(metal_mutex_t *mutex)
{
	atomic_store(&mutex->v, 0);
}

static inline void metal_mutex_deinit(metal_mutex_t *mutex)
{
	(void)mutex;
}

static inline int metal_mutex_try_acquire(metal_mutex_t *mutex)
{
	return 1 - atomic_flag_test_and_set(&mutex->v);
}

static inline void metal_mutex_acquire(metal_mutex_t *mutex)
{
	while (atomic_flag_test_and_set(&mutex->v)) {
		;
	}
}

static inline void metal_mutex_release(metal_mutex_t *mutex)
{
	atomic_flag_clear(&mutex->v);
}

static inline int metal_mutex_is_acquired(metal_mutex_t *mutex)
{
	return atomic_load(&mutex->v);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_ZEPHYR_MUTEX__H__ */
