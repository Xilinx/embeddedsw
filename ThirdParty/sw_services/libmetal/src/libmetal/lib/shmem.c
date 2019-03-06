/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	generic/shmem.c
 * @brief	Generic libmetal shared memory handling.
 */

#include <errno.h>
#include <metal/alloc.h>
#include <metal/assert.h>
#include <metal/mutex.h>
#include <metal/shmem.h>
#include <metal/shmem-provider.h>
#include <metal/sys.h>
#include <metal/utilities.h>
#include <string.h>

static METAL_MUTEX_DEFINE(metal_shmem_lock);

int metal_shmem_register_generic(struct metal_generic_shmem *shmem)
{
	/* Make sure that we can be found. */
	metal_assert(shmem->name && strlen(shmem->name) != 0);

	metal_list_add_tail(&_metal.common.generic_shmem_list,
			    &shmem->node);
	return 0;
}

int metal_shmem_open_generic(const char *name, size_t size,
			     struct metal_generic_shmem **result)
{
	struct metal_generic_shmem *shmem;
	struct metal_list *node;

	metal_list_for_each(&_metal.common.generic_shmem_list, node) {
		size_t shmem_size;

		shmem = metal_container_of(node,
					   struct metal_generic_shmem, node);
		if (strcmp(shmem->name, name) != 0)
			continue;
		shmem_size = shmem->size;
		if (shmem_size != 0 && size != 0 && size > shmem_size)
			continue;
		*result = shmem;
		return 0;
	}

	return -ENOENT;
}

int metal_shmem_get_provider_name(const char *name, char *result)
{
	int pname_size;
	char *pname_end;

	if (name == NULL || result == NULL) {
		return -EINVAL;
	}
	pname_end = strchr(name, '/');
	if (pname_end != NULL) {
		pname_size = pname_end - name;
	} else {
		pname_size = 0;
	}
	if (pname_size <= 0) {
		metal_log(METAL_LOG_ERROR,
			  "No shmem provider is specified from: %s.\n"
			  "Please use '/' to separate shmem provider and "
			  "shmem name: <provider>/<shmem>.\n",
			  name);
		return -EINVAL;
	}
	memset(result, 0, pname_size + 1);
	strncpy(result, name, pname_size);
	return pname_size;
}

int metal_shmem_get_subname(const char *name, char *result)
{
	int subname_size;
	char *pname_end;

	if (name == NULL || result == NULL) {
		return -EINVAL;
	}
	pname_end = strchr(name, '/');
	if (pname_end != NULL) {
		subname_size = name + strlen(name) - pname_end;
	} else {
		subname_size = 0;
	}
	if (subname_size <= 0) {
		metal_log(METAL_LOG_ERROR,
			  "No shmem subname is specified from: %s.\n"
			  "Please use '/' to separate shmem provider and"
			  "shmem name.\n",
			  name);
		return -EINVAL;
	}
	memset(result, 0, subname_size + 1);
	strncpy(result, pname_end + 1, subname_size);
	return subname_size;
}

int metal_shmem_open(const char *name,
		     size_t size, unsigned int flags,
		     struct metal_generic_shmem **result)
{
	struct metal_shm_provider *provider;
	struct metal_generic_shmem *shmem;
	char provider_name[128];
	int ret;

	if (result == NULL || name == NULL) {
		return -EINVAL;
	}
	metal_mutex_acquire(&metal_shmem_lock);
	ret = metal_shmem_open_generic(name, size, result);
	if (ret == 0) {
		metal_mutex_release(&metal_shmem_lock);
		return ret;
	}
	ret = metal_shmem_get_provider_name(name, provider_name);
	if (ret <= 0) {
		metal_mutex_release(&metal_shmem_lock);
		return -EINVAL;
	}

	provider = metal_shmem_get_provider(provider_name);
	if (provider == NULL) {
		metal_log(METAL_LOG_ERROR, "Failed to get shm provider %s.\n",
			  provider_name);
		metal_mutex_release(&metal_shmem_lock);
		return -EINVAL;
	}
	shmem = metal_allocate_memory(sizeof(*shmem));
	if (shmem == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "Failed to allocate memory for shm handle.\n");
		metal_mutex_release(&metal_shmem_lock);
		return -ENOMEM;
	}
	memset(shmem, 0, sizeof(*shmem));
	strncpy(shmem->name, name, sizeof(shmem->name) - 1);
	shmem->provider = provider;
	shmem->flags = flags;
	metal_list_init(&shmem->refs);
	metal_mutex_init(&shmem->lock);
	ret = provider->alloc(provider, shmem, size);
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR,
			  "Failed to get shared memory from %s.\n",
			  provider_name);
		metal_free_memory(shmem);
		metal_mutex_release(&metal_shmem_lock);
		return ret;
	}
	metal_shmem_register_generic(shmem);
	metal_mutex_release(&metal_shmem_lock);
	*result = shmem;
	return 0;
}

int metal_shmem_close(struct metal_generic_shmem *shmem)
{
	struct metal_shm_provider *provider;
	struct metal_list *node;
	int need_to_close;

	if (shmem == NULL) {
		return -EINVAL;
	}
	metal_mutex_acquire(&metal_shmem_lock);
	/* Check if the the shared memory is already closed */
	need_to_close = 0;
	metal_list_for_each(&_metal.common.generic_shmem_list, node) {
		struct metal_generic_shmem *lshmem;
		lshmem = metal_container_of(node,
					    struct metal_generic_shmem, node);
		if (lshmem == shmem) {
			need_to_close = 1;
			break;
		}
	}
	if (need_to_close == 0) {
		/* It is already closed */
		metal_mutex_release(&metal_shmem_lock);
		return 0;
	}
	metal_mutex_acquire(&shmem->lock);
	if (shmem->refcount != 0) {
		metal_log(METAL_LOG_ERROR,
			  "Failed to close shmem with non-zero refcount %d.\n",
			  shmem->refcount);
		metal_mutex_release(&shmem->lock);
		metal_mutex_release(&metal_shmem_lock);
		return -EINVAL;
	}
	provider = shmem->provider;
	if (provider && provider->free) {
		provider->free(provider, shmem);
		metal_list_del(&shmem->node);
		metal_mutex_release(&shmem->lock);
		metal_mutex_deinit(&shmem->lock);
		metal_free_memory(shmem);
	} else {
		metal_mutex_release(&shmem->lock);
	}
	metal_mutex_release(&metal_shmem_lock);
	return 0;
}

static void metal_shm_add_ref(struct metal_generic_shmem *shmem,
			      struct metal_shm_ref *ref)
{
	metal_mutex_acquire(&shmem->lock);
	metal_list_add_tail(&shmem->refs, &ref->node);
	shmem->refcount++;
	metal_mutex_release(&shmem->lock);
}

static void metal_shm_remove_ref(struct metal_generic_shmem *shmem,
				 struct metal_shm_ref *ref)
{
	metal_mutex_acquire(&shmem->lock);
	metal_list_del(&ref->node);
	shmem->refcount--;
	metal_mutex_release(&shmem->lock);
	metal_free_memory(ref);
}

struct metal_scatter_list *
metal_shmem_mmap(struct metal_generic_shmem *shmem, size_t size)
{
	struct metal_shm_ref *ref;
	int ret;

	if (shmem == NULL) {
		return NULL;
	}
	ref = metal_allocate_memory(sizeof(*ref));
	if (ref == NULL) {
		return NULL;
	}
	ref->flags = 0;
	ref->dev = NULL;
	if (shmem->ops->mmap) {
		ret = shmem->ops->mmap(shmem, size, &ref->sg);
		if (ret != 0) {
			metal_log(METAL_LOG_ERROR,
				  "Failed to mmap shmem %s.\n",
				  shmem->name);
			return NULL;
		}
	} else {
		/* Use the default shmem scatter list */
		memcpy(&ref->sg, &shmem->sg, sizeof(ref->sg));
	}
	metal_shm_add_ref(shmem, ref);
	return &ref->sg;
}

void metal_shmem_munmap(struct metal_generic_shmem *shmem,
			struct metal_scatter_list *sg)
{
	struct metal_shm_ref *ref;
	struct metal_list *node;

	if (shmem == NULL) {
		return;
	}

	metal_mutex_acquire(&shmem->lock);
	metal_list_for_each(&shmem->refs, node) {
		ref = metal_container_of(node,
					 struct metal_shm_ref, node);
		if (&ref->sg == sg) {
			metal_mutex_release(&shmem->lock);
			if (shmem->ops->munmap) {
				shmem->ops->munmap(shmem, sg);
			}
			metal_shm_remove_ref(shmem, ref);
			return;
		}
	}
	metal_mutex_release(&shmem->lock);
}

struct metal_scatter_list *
metal_shm_attach(struct metal_generic_shmem *shmem,
		 struct metal_device *dev,
		 unsigned int direction)
{
	struct metal_shm_ref *ref;
	struct metal_bus *bus;
	int ret;

	if (shmem == NULL || dev == NULL) {
		return NULL;
	}

	ref = metal_allocate_memory(sizeof(*ref));
	if (ref == NULL) {
		metal_log(METAL_LOG_ERROR,
			  "Failed to allocate memory for ref for shm attachment.\n");
		metal_free_memory(ref);
		return NULL;
	}
	ref->dev = dev;
	bus = dev->bus;
	if (bus && bus->ops.dev_shm_attach) {
		ret = bus->ops.dev_shm_attach(bus, shmem, dev, direction, ref);
		if (ret < 0) {
			metal_log(METAL_LOG_ERROR,
				  "Failed to attach shmem %s to dev %s.\n",
				  shmem->name, dev->name);
			metal_free_memory(ref);
			return NULL;
		}
	} else {
		memcpy(&ref->sg, &shmem->sg, sizeof(ref->sg));
	}
	metal_shm_add_ref(shmem, ref);
	return &ref->sg;
}

void metal_shm_detach(struct metal_generic_shmem *shmem,
		     struct metal_device *dev)
{
	struct metal_shm_ref *ref;
	struct metal_list *node;

	if (shmem == NULL) {
		return;
	}

	metal_mutex_acquire(&shmem->lock);
	metal_list_for_each(&shmem->refs, node) {
		ref = metal_container_of(node,
					 struct metal_shm_ref, node);
		if (ref->dev == dev) {
			struct metal_bus *bus;

			metal_mutex_release(&shmem->lock);
			bus = dev->bus;
			if (bus->ops.dev_shm_detach) {
				bus->ops.dev_shm_detach(bus, shmem, dev, ref);
			}
			metal_shm_remove_ref(shmem, ref);
			break;
		}
	}
	metal_mutex_release(&shmem->lock);
}

int metal_shm_sync_for_device(struct metal_generic_shmem *shmem,
			      struct metal_device *dev,
			      unsigned int direction)
{
	if (shmem == NULL) {
		return -EINVAL;
	}
	if (shmem->ops->sync_for_device) {
		return shmem->ops->sync_for_device(shmem, dev, direction);
	}
	return 0;
}

int metal_shm_sync_for_cpu(struct metal_generic_shmem *shmem,
			    unsigned int direction)
{
	if (shmem == NULL) {
		return -EINVAL;
	}
	if (shmem->ops->sync_for_cpu) {
		return shmem->ops->sync_for_cpu(shmem, direction);
	}
	return 0;
}
