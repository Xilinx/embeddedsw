/*
 * Copyright (c) 2019, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	linux/shmemi-dma.c
 * @brief	Linux libmetal DMA buf shared memory handling.
 */

#include <linux/dma-buf.h>
#include <metal/dma.h>
#include <metal/log.h>
#include <metal/shmem.h>
#include <metal/sys.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <string.h>
#include "shmem.h"

static int metal_shm_dma_buf_sync_for_device(struct metal_generic_shmem *shm,
					     struct metal_device *dev,
					     unsigned int direction)
{
	int fd, ret;
	struct dma_buf_sync sync;

	(void)dev;
	fd = shm->id;
	if (direction == METAL_DMA_DEV_R) {
		sync.flags = DMA_BUF_SYNC_WRITE;
	} else if (direction == METAL_DMA_DEV_W) {
		sync.flags = DMA_BUF_SYNC_READ;
	} else if (direction == METAL_DMA_DEV_WR) {
		sync.flags = DMA_BUF_SYNC_RW;
	} else {
		metal_log(METAL_LOG_ERROR,
			  "%s: unrecognized direction: 0x%x\n",
			  __func__, direction);
		return -EINVAL;
	}
	ret = ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync);
	if (ret) {
		metal_log(METAL_LOG_ERROR,
			  "%s: failed to sync, %s.\n",
			  __func__, strerror(errno));
	}
	return ret;
}

static int metal_shm_dma_buf_sync_for_cpu(struct metal_generic_shmem *shm,
					  unsigned int direction)
{
	int fd, ret;
	struct dma_buf_sync sync;

	fd = shm->id;
	if (direction == METAL_DMA_DEV_R) {
		sync.flags = DMA_BUF_SYNC_WRITE;
	} else if (direction == METAL_DMA_DEV_W) {
		sync.flags = DMA_BUF_SYNC_READ;
	} else if (direction == METAL_DMA_DEV_WR) {
		sync.flags = DMA_BUF_SYNC_RW;
	} else {
		metal_log(METAL_LOG_ERROR,
			  "%s: unrecognized direction: 0x%x\n",
			  __func__, direction);
		return -EINVAL;
	}
	sync.flags |= DMA_BUF_SYNC_END;
	ret = ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync);
	if (ret) {
		metal_log(METAL_LOG_ERROR,
			  "%s: failed to sync, %s.\n",
			  __func__, strerror(errno));
	}
	return ret;
}

static int
metal_shm_dma_buf_mmap(struct metal_generic_shmem *shmem, size_t size,
		       struct metal_scatter_list *sg)
{
	struct metal_io_region *io;
	int fd;
	void *va;

	if (shmem == NULL || shmem->id < 0) {
		return -EINVAL;
	}
	io = malloc(sizeof(*io));
	if (!io) {
		metal_log(METAL_LOG_ERROR,
			  "%s: failed to allocate memory for I/O region.\n",
			  __func__);
		return -ENOMEM;
	}
	fd = shmem->id;
	va = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (va == MAP_FAILED) {
		metal_log(METAL_LOG_ERROR,
			 "%s: failed to mmap shmem %s, of 0x%lx\n",
			 __func__, shmem->name, size);
		free(io);
		return -EINVAL;
	} else {
		metal_io_init(io, va, NULL, size, -1, 0, NULL);
		sg->ios = io;
		sg->nents = 1;

		return 0;
	}
}

static void metal_shm_dma_buf_munmap(struct metal_generic_shmem *shm,
				     struct metal_scatter_list *sg)
{
	struct metal_io_region *io;

	(void)shm;
	io = sg->ios;
	if (io != NULL) {
		munmap(io->virt, io->size);
		free(io);
		sg->ios = NULL;
	}
}

struct metal_shm_ops metal_shm_dma_buf_ops = {
	.sync_for_device = metal_shm_dma_buf_sync_for_device,
	.sync_for_cpu = metal_shm_dma_buf_sync_for_cpu,
	.mmap = metal_shm_dma_buf_mmap,
	.munmap = metal_shm_dma_buf_munmap,
};
