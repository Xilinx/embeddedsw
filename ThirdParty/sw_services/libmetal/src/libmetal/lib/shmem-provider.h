/*
 * Copyright (c) 2019, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	shmem-pool.h
 * @brief	Shared memory primitives for libmetal.
 */

#ifndef __METAL_SHMEM_PROVIDER__H__
#define __METAL_SHMEM_PROVIDER__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <metal/list.h>
#include <stdlib.h>

/** \defgroup shmem-provider Shared Memory Provider Interfaces
 *  @{ */

struct metal_generic_shmem;
struct metal_shm_provider;

/**
 * @brief type of shmem allocation function
 * @param[in]	provider shmem provider
 * @param[in]	shm	shmem handle
 * @param[in]	size	shmem size
 * @return 0 for success, or negative value for failure.
 */
typedef int (*metal_shm_alloc)(struct metal_shm_provider *provider,
			       struct metal_generic_shmem *shm,
			       size_t size);

/**
 * @brief type of shmem free function
 * @param[in]	provider shmem provider
 * @param[in]	shm	shmem handle
 */
typedef void (*metal_shm_free)(struct metal_shm_provider *provider,
			       struct metal_generic_shmem *shm);

/** Shared memory provider data structure. */
struct metal_shm_provider {
	const char		*name; /**< name of shmem provider */
	void			*priv; /**< private data */
	metal_shm_alloc		alloc; /**< shmem allocation function */
	metal_shm_free		free; /**< shmem free function */
	struct metal_list	node; /**< node */
};

#define METAL_SHM_PROVIDER_DEFINE(_name, _priv, \
				  _alloc, _free) \
	{ \
		.name = _name, \
		.priv = _priv, \
		.alloc = _alloc, \
		.free = _free, \
	}

#define METAL_SHM_PROVIDER_DECLARE(_provider, _name, _priv, \
				   _alloc, _free) \
	struct metal_shm_provider _provider = \
		METAL_SHM_PROVIDER_DEFINE(_name, _priv, \
					  _alloc, _free);

/**
 * @brief metal shared memory provider registration
 *
 * Register the metal shared memory provider to the system.
 * After the registration, user can get the shared
 * memory provider with its name.
 *
 * @param[in]	provider pointer to a shared memory provider
 *
 * @return 0 for success, negative value for failure
 */
int metal_shm_provider_register(struct metal_shm_provider *provider);

/**
 * @brief metal shared memory provider unregistration
 *
 * Unregister the metal shared memory provider to the system.
 *
 * @param[in]	provider pointer to a shared memory provider
 */
void metal_shm_provider_unregister(struct metal_shm_provider *provider);

/**
 * @brief get metal shared memory provider
 *
 * Retrieve a previous registered shared memory provider.
 *
 * @param[in]	name name of the shared memory provider
 *
 * @return point to a shared memory provider for success, NULL for failure.
 */
struct metal_shm_provider *metal_shmem_get_provider(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* __METAL_SHMEM_PROVIDER__H__ */
