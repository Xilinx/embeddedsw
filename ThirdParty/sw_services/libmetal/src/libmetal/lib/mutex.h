/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
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
 * @file	mutex.h
 * @brief	Mutex primitives for libmetal.
 */

#ifndef __METAL_MUTEX__H__
#define __METAL_MUTEX__H__

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup mutex Mutex Interfaces
 *  @{ */

#include <metal/system/@PROJECT_SYSTEM@/mutex.h>

/**
 * @brief	Initialize a libmetal mutex.
 * @param[in]	mutex	Mutex to initialize.
 */
static inline void metal_mutex_init(metal_mutex_t *mutex);

/**
 * @brief	Deinitialize a libmetal mutex.
 * @param[in]	mutex	Mutex to deinitialize.
 */
static inline void metal_mutex_deinit(metal_mutex_t *mutex);

/**
 * @brief	Try to acquire a mutex
 * @param[in]	mutex	Mutex to mutex.
 * @return	0 on failure to acquire, non-zero on success.
 */
static inline int metal_mutex_try_acquire(metal_mutex_t *mutex);

/**
 * @brief	Acquire a mutex
 * @param[in]	mutex	Mutex to mutex.
 */
static inline void metal_mutex_acquire(metal_mutex_t *mutex);

/**
 * @brief	Release a previously acquired mutex.
 * @param[in]	mutex	Mutex to mutex.
 * @see metal_mutex_try_acquire, metal_mutex_acquire
 */
static inline void metal_mutex_release(metal_mutex_t *mutex);

/**
 * @brief	Checked if a mutex has been acquired.
 * @param[in]	mutex	mutex to check.
 * @see metal_mutex_try_acquire, metal_mutex_acquire
 */
static inline int metal_mutex_is_acquired(metal_mutex_t *mutex);


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_MUTEX__H__ */
