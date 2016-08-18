
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
#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "metal/sys.h"
#include "metal/device.h"
#include "metal/io.h"
#include "metal/shmem.h"

/** Device base address */
#define SCUGIC_PERIPH_BASE      0xF8F00000
#define SCUGIC_DIST_BASE        (SCUGIC_PERIPH_BASE + 0x00001000)
#define UNDEFINE_BASE_ADDR      0x0

#define UNDEFINE_MEM_SIZE       0xFFFFFFFF

/** IPI IRQ ID */
#define IPI0_IRQ_VECT_ID         15
#define IPI1_IRQ_VECT_ID         14

#define IPI0_DEV_NAME           "ipi0"
#define IPI1_DEV_NAME           "ipi1"
#define VRING_DEV_NAME          "vrings"
#define SHM_NAME                "shm"

const metal_phys_addr_t undefine_mem_addr = UNDEFINE_BASE_ADDR;
const metal_phys_addr_t ipi_base_addr = SCUGIC_DIST_BASE;

struct metal_device metal_dev_table[] = {
	{
		/* IPI0 device */
		IPI0_DEV_NAME,
		NULL,
		1,
		{
			{
				(void *)SCUGIC_DIST_BASE,
				&ipi_base_addr,
				0x1000,
				(sizeof(metal_phys_addr_t) << 3),
				(unsigned long)(-1),
				METAL_UNCACHED,
				{NULL},
			}
		},
		{NULL},
		1,
		(void *)IPI0_IRQ_VECT_ID,

	},
	{
		/* IPI device */
		IPI1_DEV_NAME,
		NULL,
		1,
		{
			{
				(void *)SCUGIC_DIST_BASE,
				&ipi_base_addr,
				0x1000,
				(sizeof(metal_phys_addr_t) << 3),
				(unsigned long)(-1),
				METAL_UNCACHED,
				{NULL},
			}
		},
		{NULL},
		1,
		(void *)IPI1_IRQ_VECT_ID,

	},
	{
		/* Shared memory management device */
		VRING_DEV_NAME,
		NULL,
		1,
		{
			{
				(void *)UNDEFINE_BASE_ADDR,
				&undefine_mem_addr,
				UNDEFINE_MEM_SIZE,
				(sizeof(metal_phys_addr_t) << 3),
				(unsigned long)(-1),
				METAL_UNCACHED | METAL_SHARED_MEM,
				{NULL},
			}
		},
		{NULL},
		0,
		NULL,

	},
};

struct metal_generic_shmem shm = {
	SHM_NAME,
	{
		(void *)UNDEFINE_BASE_ADDR,
		&undefine_mem_addr,
		UNDEFINE_MEM_SIZE,
		(sizeof(metal_phys_addr_t) << 3),
		(unsigned long)(-1),
		0,
		{NULL},
	},
	{NULL, NULL},
};

/**
 * This funciton is to install baremeta/RTOS libmetal devices.
 */
extern char RPROC_DEBUG[];
int platform_register_metal_device(void)
{
	int i;
	int ret;
	struct metal_device *dev;

	/* Register device */
	metal_bus_register(&metal_generic_bus);
	for (i = 0;
	     i < (int)(sizeof(metal_dev_table)/sizeof(struct metal_device));
	     i++) {
		dev = &metal_dev_table[i];
		ret = metal_register_generic_device(dev);
		if (ret)
			return ret;
	}

	/* Register shared memory */
	metal_shmem_register_generic(&shm);

	return 0;
}
