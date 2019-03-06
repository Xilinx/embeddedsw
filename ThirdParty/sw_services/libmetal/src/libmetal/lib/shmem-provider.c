/*
 * Copyright (c) 2019, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	shmem-provider.c
 * @brief	libmetal shared memory provider implementation.
 */

#include <errno.h>
#include <metal/alloc.h>
#include <metal/list.h>
#include <metal/log.h>
#include <metal/shmem.h>
#include <metal/shmem-provider.h>
#include <metal/utilities.h>
#include <string.h>

METAL_DECLARE_LIST(metal_shm_provider_list);

int metal_shm_provider_register(struct metal_shm_provider *provider)
{
	struct metal_list *node;

	if (provider == NULL) {
		return -EINVAL;
	}
	/* check if the provider has already been registered */
	metal_list_for_each(&metal_shm_provider_list, node) {
		if (node == &provider->node) {
			return 0;
		}
	}
	/* need to register the provider */
	metal_log(METAL_LOG_INFO, "Registered shmem provider %s.\n",
		  provider->name);
	metal_list_add_tail(&metal_shm_provider_list, &provider->node);
	return 0;
}

void metal_shm_provider_unregister(struct metal_shm_provider *provider)
{
	if (provider == NULL) {
		return;
	}
	metal_list_del(&provider->node);
}

struct metal_shm_provider *metal_shmem_get_provider(const char *name)
{
	struct metal_list *node;
	struct metal_shm_provider *provider = NULL;

	if (name == NULL) {
		return NULL;
	}
	metal_list_for_each(&metal_shm_provider_list, node) {
		provider = metal_container_of(node,
					      struct metal_shm_provider,
					      node);
		if (strcmp(name, provider->name) == 0) {
			return provider;
		}
	}
	return NULL;
}
