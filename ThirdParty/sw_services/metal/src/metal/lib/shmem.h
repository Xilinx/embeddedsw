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
 * @file	shmem.h
 * @brief	Shared memory primitives for libmetal.
 */

#ifndef __METAL_SHMEM__H__
#define __METAL_SHMEM__H__

#include <metal/io.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup shmem Shared Memory Interfaces
 *  @{ */

/** Generic shared memory data structure. */
struct metal_generic_shmem {
	const char		*name;
	struct metal_io_region	io;
	struct metal_list	node;
};

/**
 * @brief	Open a libmetal shared memory segment.
 *
 * Open a shared memory segment.
 *
 * @param[in]		name	Name of segment to open.
 * @param[in]		size	Size of segment.
 * @param[out]		io	I/O region handle, if successful.
 * @return	0 on success, or -errno on failure.
 *
 * @see metal_shmem_create
 */
extern int metal_shmem_open(const char *name, size_t size,
			    struct metal_io_region **io);

/**
 * @brief	Statically register a generic shared memory region.
 *
 * Shared memory regions may be statically registered at application
 * initialization, or may be dynamically opened.  This interface is used for
 * static registration of regions.  Subsequent calls to metal_shmem_open() look
 * up in this list of pre-registered regions.
 *
 * @param[in]	shmem	Generic shmem structure.
 * @return 0 on success, or -errno on failure.
 */
extern int metal_shmem_register_generic(struct metal_generic_shmem *shmem);

#ifdef METAL_INTERNAL

/**
 * @brief	Open a statically registered shmem segment.
 *
 * This interface is meant for internal libmetal use within system specific
 * shmem implementations.
 *
 * @param[in]		name	Name of segment to open.
 * @param[in]		size	Size of segment.
 * @param[out]		io	I/O region handle, if successful.
 * @return	0 on success, or -errno on failure.
 */
int metal_shmem_open_generic(const char *name, size_t size,
			     struct metal_io_region **result);

#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_SHMEM__H__ */
