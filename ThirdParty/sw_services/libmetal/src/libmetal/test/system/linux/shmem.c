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

#include "metal-test.h"
#include <metal/log.h>
#include <metal/mutex.h>
#include <metal/shmem.h>
#include <metal/sys.h>
#include <metal/atomic.h>

static atomic_int nb_err = ATOMIC_VAR_INIT(0);

static const int shmem_threads = 10;

static void *shmem_child(void *arg)
{
	const char *name = arg;
	struct {
		metal_mutex_t	mutex;
		int			counter;
	} *virt;
	struct metal_io_region *io;
	unsigned long phys;
	size_t size = 2 * 1024 * 1024;
	int error;

	error = metal_shmem_open(name, size, &io);
	if (error) {
		metal_log(METAL_LOG_ERROR, "Failed shmem_open: %d.\n", error);
		atomic_fetch_add(&nb_err, 1);
		return NULL;
	}

	virt = metal_io_virt(io, 0);
	phys = metal_io_phys(io, 0);
	if (phys != METAL_BAD_OFFSET) {
		if (virt != metal_io_phys_to_virt(io, phys)) {
			atomic_fetch_add(&nb_err, 1);
			metal_log(METAL_LOG_ERROR, "Failed virt != phys.\n");
		}
		if (phys != metal_io_virt_to_phys(io, virt)) {
			atomic_fetch_add(&nb_err, 1);
			metal_log(METAL_LOG_ERROR, "Failed phys != virt.\n");
		}
	}

	metal_io_finish(io);
	return NULL;
}

static int shmem(void)
{
	return atomic_load(&nb_err) || metal_run(shmem_threads, shmem_child, "/foo");
}
METAL_ADD_TEST(shmem);
