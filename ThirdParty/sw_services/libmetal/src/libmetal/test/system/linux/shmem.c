/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "metal-test.h"
#include <metal/log.h>
#include <metal/mutex.h>
#include <metal/shmem.h>
#include <metal/scatterlist.h>
#include <metal/sys.h>
#include <metal/atomic.h>

static atomic_int nb_err = ATOMIC_VAR_INIT(0);

static const int shmem_threads = 10;

static void *shmem_child(void *arg)
{
	const char *name = arg;
	struct {
		metal_mutex_t	mutex;
		int		counter;
	} *virt;
	struct metal_generic_shmem *shm;
	struct metal_io_region *io;
	struct metal_scatter_list *sg;
	unsigned long phys;
	size_t size = 2 * 1024 * 1024;
	int error;

	error = metal_shmem_open(name, size, 0, &shm);
	if (error) {
		metal_log(METAL_LOG_ERROR, "Failed shmem_open: %d.\n", error);
		atomic_fetch_add(&nb_err, 1);
		return NULL;
	}
	sg = metal_shmem_mmap(shm, size);
	if (sg == NULL) {
		metal_log(METAL_LOG_ERROR, "Failed to shmem_mmap %s, %s.\n",
			  shm->name, name);
		return NULL;
	}
	(void)metal_scatterlist_get_ios(sg, &io);
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
	metal_shmem_munmap(shm, sg);
	metal_shmem_close(shm);
	return NULL;
}

static int shmem(void)
{
	return atomic_load(&nb_err) || metal_run(shmem_threads, shmem_child, "linux_shm/foo");
}
METAL_ADD_TEST(shmem);
