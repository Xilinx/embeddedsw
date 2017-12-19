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
 * @file	condition.h
 * @brief	Condition variable for libmetal.
 */

#ifndef __METAL_CONDITION__H__
#define __METAL_CONDITION__H__

#include <metal/mutex.h>
#include <metal/utilities.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup condition Condition Variable Interfaces
 *  @{ */

/** Opaque libmetal condition variable data structure. */
struct metal_condition;

/**
 * @brief        Initialize a libmetal condition variable.
 * @param[in]	 cv	condition variable to initialize.
 */
static inline void metal_condition_init(struct metal_condition *cv);

/**
 * @brief        Notify one waiter.
 *               Before calling this function, the caller
 *               should have acquired the mutex.
 * @param[in]    cv    condition variable
 * @return       zero on no errors, non-zero on errors
 * @see metal_condition_wait, metal_condition_broadcast
 */
static inline int metal_condition_signal(struct metal_condition *cv);

/**
 * @brief        Notify all waiters.
 *               Before calling this function, the caller
 *               should have acquired the mutex.
 * @param[in]    cv    condition variable
 * @return       zero on no errors, non-zero on errors
 * @see metal_condition_wait, metal_condition_signal
 */
static inline int metal_condition_broadcast(struct metal_condition *cv);

/**
 * @brief        Block until the condition variable is notified.
 *               Before calling this function, the caller should
 *               have acquired the mutex.
 * @param[in]    cv    condition variable
 * @param[in]    m     mutex
 * @return	 0 on success, non-zero on failure.
 * @see metal_condition_signal
 */
int metal_condition_wait(struct metal_condition *cv, metal_mutex_t *m);

#include <metal/system/@PROJECT_SYSTEM@/condition.h>

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_CONDITION__H__ */
