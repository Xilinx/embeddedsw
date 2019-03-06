/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "metal-test.h"
#include <metal/log.h>
#include <metal/scatterlist.h>
#include <metal/shmem.h>
#include <stdlib.h>

#define MB(n) (0x100000U * (n))
#define SHM_SIZE MB(4)

static int ion_shmem(void)
{
	struct metal_generic_shmem *shm;
	struct metal_scatter_list *sg;
	char *shm_name = "ion.reserved/shm0";
	struct metal_io_region *io;
	size_t size;
	int ret;
	void *va;
	unsigned int *a;

	size = SHM_SIZE;
	ret = metal_shmem_open(shm_name,
			       SHM_SIZE, METAL_SHM_NOTCACHED, &shm);
	if (ret) {
		metal_log(METAL_LOG_ERROR,
			  "failed to open shared memory %s.\n",
			  shm_name);
		return -EINVAL;
	}

	sg = metal_shmem_mmap(shm, size);
	if (sg == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "failed to mmap shared memory from pool.\n");
		return -EINVAL;
	}
	io = sg->ios;
	va = metal_io_virt(io, 0);
	a = va;
	*a = 0xdeadbeff;
	metal_shm_sync_for_cpu(shm, METAL_SHM_DIR_DEV_RW);
	metal_shmem_munmap(shm, sg);
	metal_shmem_close(shm);
	return 0;
}
METAL_ADD_TEST(ion_shmem);
