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

/*
 * @file	spinlock.h
 * @brief	Spinlock primitives for libmetal.
 */

#ifndef __METAL_SPINLOCK__H__
#define __METAL_SPINLOCK__H__

#include <metal/atomic.h>
#include <metal/cpu.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup spinlock Spinlock Interfaces
 *  @{ */
struct metal_spinlock {
	atomic_int v;
};

/** Static metal spinlock initialization. */
#define METAL_SPINLOCK_INIT		{ATOMIC_VAR_INIT(0)}

/**
 * @brief	Initialize a libmetal spinlock.
 * @param[in]	slock	Spinlock to initialize.
 */
static inline void metal_spinlock_init(struct metal_spinlock *slock)
{
	atomic_store(&slock->v, 0);
}

/**
 * @brief	Acquire a spinlock.
 * @param[in]	slock   Spinlock to acquire.
 * @see metal_spinlock_release
 */
static inline void metal_spinlock_acquire(struct metal_spinlock *slock)
{
	while (atomic_flag_test_and_set(&slock->v)) {
		metal_cpu_yield();
	}
}

/**
 * @brief	Release a previously acquired spinlock.
 * @param[in]	slock	Spinlock to release.
 * @see metal_spinlock_acquire
 */
static inline void metal_spinlock_release(struct metal_spinlock *slock)
{
	atomic_flag_clear(&slock->v);
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_SPINLOCK__H__ */
