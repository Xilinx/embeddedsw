/*
 * Copyright (c) 2019, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	linux/shmem-provider-ion.c
 * @brief	Linux libmetal shared memory provider with ION implementation.
 */

#include <metal/alloc.h>
#include <metal/shmem.h>
#include <metal/shmem-provider.h>
#include <metal/sys.h>
#include <metal/utilities.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ion.h"
#include "shmem.h"

struct metal_linux_ion {
	int fd;		/**< ION file descriptor */
	struct ion_heap_query query;	/**< ION heap query */
};

struct metal_linux_shm_provider_ion {
	struct metal_shm_provider provider;
	struct metal_linux_ion *ion;
	uint32_t heap_id;
	char name[64];
};

static struct metal_linux_shm_provider_ion *ion_providers;

extern struct metal_shm_ops metal_shm_dma_buf_ops;

static struct metal_linux_ion linux_ion = {
	.fd = -1,
};

static int _metal_linux_ion_ioctl_alloc(int fd,
					struct ion_allocation_data *alloc_data)
{
	int ret;

	ret = ioctl(fd, ION_IOC_ALLOC, alloc_data);
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR,
			  "%s: failed to alloc from ion heap 0x%x, %s.\n",
			  __func__, alloc_data->heap_id_mask, strerror(errno));
	}
	return ret;
}

static int _metal_linux_ion_alloc(struct metal_shm_provider *provider,
				  size_t size, unsigned int flags)
{
	struct ion_allocation_data alloc_data;
	struct metal_linux_shm_provider_ion *ion_provider;
	struct metal_linux_ion *ion;
	struct ion_heap_data *heaps;
	int ret, i;

	if (provider == NULL) {
		return -EINVAL;
	}
	ion_provider = (struct metal_linux_shm_provider_ion *)provider;
	ion = ion_provider->ion;
	if (ion->fd < 0) {
		metal_log(METAL_LOG_ERROR,
			 "%s: ion is not opened.\n", __func__);
		return -EINVAL;
	}
	alloc_data.len = size;
	alloc_data.heap_id_mask = 0;
	if (flags != METAL_SHM_NOTCACHED) {
		alloc_data.flags = ION_FLAG_CACHED;
	}

	heaps = (struct ion_heap_data *)(uintptr_t)(ion->query.heaps);
	ret = -EINVAL;
	for (i = 0; i < (int)ion->query.cnt; i++) {
		if (heaps[i].heap_id == ion_provider->heap_id) {
			alloc_data.heap_id_mask = 1 << heaps[i].heap_id;
			ret = _metal_linux_ion_ioctl_alloc(ion->fd,
							   &alloc_data);
			if (ret != 0) {
				ret = -EINVAL;
			}
			break;
		}
	}
	if (ret != 0) {
		return -EINVAL;
	} else {
		return alloc_data.fd;
	}
}

static int metal_linux_ion_alloc(struct metal_shm_provider *provider,
				 struct metal_generic_shmem *shm,
				 size_t size)
{
	int dma_buf_fd;

	if (shm == NULL) {
		metal_log(METAL_LOG_DEBUG,
			  "%s: shm pointer is NULL.\n", __func__);
		return -EINVAL;
	}
	dma_buf_fd = _metal_linux_ion_alloc(provider, size, shm->flags);
	if (dma_buf_fd < 0) {
		return dma_buf_fd;
	}
	shm->id = dma_buf_fd;
	shm->ops = &metal_shm_dma_buf_ops;
	shm->size = size;

	return 0;
}

static void metal_linux_ion_free(struct metal_shm_provider *provider,
				 struct metal_generic_shmem *shm)
{
	int dma_buf_fd;

	(void)provider;
	if (shm == NULL) {
		metal_log(METAL_LOG_DEBUG,
			  "%s: shm pointer is NULL.\n", __func__);
		return;
	}
	dma_buf_fd = shm->id;
	close(dma_buf_fd);
}

int metal_ion_shm_provider_init(void)
{
	struct ion_heap_query *query;
	struct ion_heap_data *heaps;
	int fd, ret, i;
	size_t size;

	/* Open ION */
	fd = open("/dev/ion", O_RDONLY);
	if (fd < 0) {
		metal_log(METAL_LOG_ERROR,
			  "%s: failed to open ion dev file.\n", __func__);
		return fd;
	}
	linux_ion.fd = fd;

	/* Enquire for heaps informaiton */
	query = &linux_ion.query;
	query->heaps = 0;
	ret = ioctl(fd, ION_IOC_HEAP_QUERY, query);
	if (ret != 0) {
		metal_log(METAL_LOG_ERROR,
			  "%s: failed to enquire ION heaps\n", __func__);
		return ret;
	}
	heaps = calloc(query->cnt, sizeof(*heaps));
	if (heaps == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "%s: failed to allocate mem for ion heaps\n",
			  __func__);
		return -ENOMEM;
	}
	query->heaps = (__u64)(uintptr_t)heaps;
	ret = ioctl(fd, ION_IOC_HEAP_QUERY, query);
	if (ret != 0) {
		metal_log(METAL_LOG_ERROR,
			  "%s: failed to enquire ION heaps\n", __func__);
		return ret;
	}
	metal_log(METAL_LOG_DEBUG,
		  "%s: successfully initialze ion shm provider.\n", __func__);
	size = query->cnt * sizeof(*ion_providers);
	ion_providers = metal_allocate_memory(size);
	if (ion_providers == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "Failed to allocate memory to create "
			  "ion shared memory provider.\n");
		return -EINVAL;
	}
	memset(ion_providers, 0, size);
	for (i = 0; i < (int)query->cnt; i++) {
		struct metal_linux_shm_provider_ion *provider;

		provider = &ion_providers[i];
		provider->provider.priv = provider;
		provider->ion = &linux_ion;
		provider->heap_id = heaps[i].heap_id;
		sprintf(provider->name, "ion.%s", heaps[i].name);
		provider->provider.name = provider->name;
		provider->provider.alloc = metal_linux_ion_alloc;
		provider->provider.free = metal_linux_ion_free;
		metal_shm_provider_register(&provider->provider);
	}

	return 0;
}

void metal_ion_shm_provider_deinit(void)
{
	int i;

	/* Deregister ION providers */
	if (ion_providers != NULL) {
		for (i = 0; i < (int)linux_ion.query.cnt; i++) {
			struct metal_shm_provider *provider;

			provider = &ion_providers[i].provider;
			metal_shm_provider_unregister(provider);
		}
		metal_free_memory(ion_providers);
	}
	if (linux_ion.query.heaps != 0) {
		free((void *)(uintptr_t)linux_ion.query.heaps);
		linux_ion.query.heaps = 0;
	}
	if (linux_ion.fd >= 0) {
		close(linux_ion.fd);
		linux_ion.fd = -1;
	}
}
