
/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the <ORGANIZATION> nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <string.h>
#include <stdio.h>
#include "metal/sys.h"
#include "metal/shmem.h"
#include "metal/device.h"
#include "metal/io.h"

#define SHM_DEV_NAME "3ed10000.shm"
#define SHM_BUS_NAME "platform"

static struct metal_device *shm_dev;
static struct metal_generic_shmem shm;

void init_system()
{
	int ret;
	struct metal_io_region *io;
	struct metal_init_params metal_param = METAL_INIT_DEFAULTS;

	metal_init(&metal_param);

	/* Register the UIO shared memory */
	/* This will be a temporary solution, we should
	 * use DMA memory in fugure.
	 */
	ret = metal_device_open(SHM_BUS_NAME, SHM_DEV_NAME, &shm_dev);
	if (ret) {
		fprintf(stderr, "ERROR: Failed to open shared memory device.\n");
		return;
	}
	io = metal_device_io_region(shm_dev, 0);
	if (!io) {
		fprintf(stderr, "ERROR: Failed to get the I/O region of shared memory.\n");
		return;
	}

	memset(&shm, 0, sizeof(struct metal_generic_shmem));
	shm.name = "shm";
	memcpy((void *)&shm.io, io, sizeof(struct metal_io_region));
	ret = metal_shmem_register_generic(&shm);
	if (ret) {
		fprintf(stderr, "ERROR: Failed to registered shared memory.\n");
		return;
	}
	return;
}

void cleanup_system()
{
	if (shm_dev)
		metal_device_close(shm_dev);
	metal_finish();
}
