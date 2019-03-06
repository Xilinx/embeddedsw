/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
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

extern int metal_ion_shm_provider_init(void);
extern void metal_ion_shm_provider_deinit(void);

int metal_linux_init_shmem(void)
{
	int ret = 0;

	metal_shm_provider_register(&linux_shm_provider_shm);
#ifdef HAVE_DMA_BUF_H
	ret = metal_ion_shm_provider_init();
#endif
	return ret;
}

void metal_linux_deinit_shmem(void)
{
	metal_shm_provider_unregister(&linux_shm_provider_shm);
#ifdef HAVE_DMA_BUF_H
	metal_ion_shm_provider_deinit();
#endif
}
