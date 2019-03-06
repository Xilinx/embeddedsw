/*
 * Copyright (c) 2019, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	linux/shmem.c
 * @brief	Linux libmetal shared memory handling.
 */

#include <metal/shmem.h>
#include <metal/sys.h>
#include <metal/utilities.h>
#include "shmem.h"

static int metal_linux_shm_try_map(struct metal_page_size *ps, int fd, size_t size,
				   struct metal_io_region **result)
{
	size_t pages, page, phys_size;
	struct metal_io_region *io;
	metal_phys_addr_t *phys;
	uint8_t *virt;
	void *mem;
	int error;

	size = metal_align_up(size, ps->page_size);
	pages = size / ps->page_size;

	error = metal_map(fd, 0, size, 1, ps->mmap_flags, &mem);
	if (error) {
		metal_log(METAL_LOG_WARNING,
			  "failed to mmap shmem %ld,0x%x - %s\n",
			  size, ps->mmap_flags, strerror(-error));
		return error;
	}

	error = metal_mlock(mem, size);
	if (error) {
		metal_log(METAL_LOG_WARNING, "failed to mlock shmem - %s\n",
			  strerror(-error));
	}

	phys_size = sizeof(*phys) * pages;
	phys = malloc(phys_size);
	if (!phys) {
		metal_unmap(mem, size);
		return -ENOMEM;
	}

	io = malloc(sizeof(*io));
	if (!io) {
		free(phys);
		metal_unmap(mem, size);
		return -ENOMEM;
	}

	if (_metal.pagemap_fd < 0) {
		phys[0] = 0;
		metal_log(METAL_LOG_WARNING,
		"shmem - failed to get va2pa mapping. use offset as pa.\n");
		metal_io_init(io, mem, phys, size, -1, 0, NULL);
	} else {
		for (virt = mem, page = 0; page < pages; page++) {
			size_t offset = page * ps->page_size;
			error = metal_virt2phys(virt + offset, &phys[page]);
			if (error < 0)
				phys[page] = METAL_BAD_OFFSET;
		}
		metal_io_init(io, mem, phys, size, ps->page_shift, 0, NULL);
	}
	*result = io;

	return 0;
}

static int
metal_linux_shm_mmap(struct metal_generic_shmem *shmem, size_t size,
		     struct metal_scatter_list *sg)
{
	struct metal_page_size *ps;
	struct metal_io_region *io;
	char subname[128];
	int fd, ret;

	if (shmem == NULL || shmem->id < 0) {
		return -EINVAL;
	}
	ret = metal_shmem_get_subname(shmem->name, subname);
	if (ret <= 0) {
		return -EINVAL;
	}
	ret = metal_open(subname, 1);
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR,
			  "Failed to shm_open() file :%s\n", shmem->name);
		return ret;
	}
	fd = ret;
	/* Iterate through page sizes in decreasing order. */
	ret = -EINVAL;
	metal_for_each_page_size_down(ps) {
		if (ps->page_size > 2 * size)
			continue;
		ret = metal_linux_shm_try_map(ps, fd, size, &io);
		if (!ret)
			break;
	}
	close(fd);
	if (ret) {
		return ret;
	} else {
		sg->ios = io;
		sg->nents = 1;

		return 0;
	}
}

static void metal_linux_shm_munmap(struct metal_generic_shmem *shm,
				   struct metal_scatter_list *sg)
{
	struct metal_io_region *io;

	(void)shm;
	io = sg->ios;
	if (io != NULL) {
		metal_unmap(io->virt, io->size);
		free((void *)io->physmap);
		free(io);
		sg->ios = NULL;
	}
}

static struct metal_shm_ops metal_linux_shm_ops = {
	.sync_for_device = NULL,
	.sync_for_cpu = NULL,
	.mmap = metal_linux_shm_mmap,
	.munmap = metal_linux_shm_munmap,
};

static int metal_linux_shm_alloc(struct metal_shm_provider *provider,
				 struct metal_generic_shmem *shm,
				 size_t size)
{
	(void)provider;
	(void)size;
	if (shm == NULL || shm->name == NULL) {
		return -EINVAL;
	}
	/* Will open the Linux inter processes shmem file during mmap */
	shm->ops = &metal_linux_shm_ops;
	shm->size = size;
	return 0;
}

static void metal_linux_shm_free(struct metal_shm_provider *provider,
				 struct metal_generic_shmem *shm)
{
	(void)provider;
	if (shm == NULL) {
		return;
	}
	if (shm->id >= 0) {
		close(shm->id);
	}
}

METAL_SHM_PROVIDER_DECLARE(linux_shm_provider_shm, "linux_shm", NULL,
			   metal_linux_shm_alloc, metal_linux_shm_free)
