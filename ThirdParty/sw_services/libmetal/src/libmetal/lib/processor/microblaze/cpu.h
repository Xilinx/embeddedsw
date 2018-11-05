/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
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
 * @file	cpu.h
 * @brief	CPU specific primatives on microblaze platform.
 */

#ifndef __METAL_MICROBLAZE__H__
#define __METAL_MICROBLAZE__H__

#include <stdint.h>
#include <metal/atomic.h>

#define metal_cpu_yield()

static inline void metal_processor_io_write64(void *ptr, uint64_t value,
					      memory_order order)
{
	void *tmp = &value;

	atomic_store_explicit((atomic_ulong *)ptr, *((atomic_ulong *)tmp), order);
	tmp += sizeof(atomic_ulong);
	ptr += sizeof(atomic_ulong);
	atomic_store_explicit((atomic_ulong *)ptr, *((atomic_ulong *)tmp), order);
}

static inline uint64_t metal_processor_io_read64(void *ptr, memory_order order)
{
	uint64_t long_ret;
	void *tmp = &long_ret;

	*((atomic_ulong *)tmp) = atomic_load_explicit((atomic_ulong *)ptr, order);
	tmp += sizeof(atomic_ulong);
	ptr += sizeof(atomic_ulong);
	*((atomic_ulong *)tmp) = atomic_load_explicit((atomic_ulong *)ptr, order);

	return long_ret;
}

#endif /* __METAL_MICROBLAZE__H__ */
