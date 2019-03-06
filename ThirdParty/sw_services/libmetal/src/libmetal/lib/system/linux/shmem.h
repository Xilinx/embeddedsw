/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	shmem.h
 * @brief	Linux Shared memory primitives for libmetal.
 */

#ifndef __METAL_LINUX_SHMEM__H__
#define __METAL_LINUX_SHMEM__H__

#include <metal/shmem.h>
#include <metal/shmem-provider.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup shmem Shared Memory Interfaces
 *  @{ */

extern struct metal_shm_provider linux_shm_provider_shm;

int metal_linux_shmem_mmap(struct metal_generic_shmem *shm,
			   size_t size,
			   struct metal_scatter_list *sg);

void metal_linux_shmem_munmap(struct metal_generic_shmem *shm,
			     struct metal_scatter_list *sg);

int metal_linux_init_shmem(void);

void metal_linux_deinit_shmem(void);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_LINUX_SHMEM__H__ */
