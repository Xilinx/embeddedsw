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

int metal_linux_init_shmem(void)
{
	metal_shm_provider_register(&linux_shm_provider_shm);
	return 0;
}

void metal_linux_deinit_shmem(void)
{
	metal_shm_provider_unregister(&linux_shm_provider_shm);
}
