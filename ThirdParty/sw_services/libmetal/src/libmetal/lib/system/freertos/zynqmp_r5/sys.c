/*
 * Copyright (c) 2016 - 2019, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	freertos/zynqmp_r5/sys.c
 * @brief	machine specific system primitives implementation.
 */

#include <metal/compiler.h>
#include <metal/io.h>
#include <metal/sys.h>
#include <stdint.h>
#include "xil_cache.h"
#include "xil_exception.h"
#include "xil_mmu.h"
#include "xil_mpu.h"
#include "xreg_cortexr5.h"
#include "xscugic.h"
#include "FreeRTOS.h"
#include "task.h"

void sys_irq_restore_enable(unsigned int flags)
{
	Xil_ExceptionEnableMask(~flags);
}

unsigned int sys_irq_save_disable(void)
{
	unsigned int state = mfcpsr() & XIL_EXCEPTION_ALL;

	if (state != XIL_EXCEPTION_ALL) {
		Xil_ExceptionDisableMask(XIL_EXCEPTION_ALL);
	}
	return state;
}

void metal_machine_cache_flush(void *addr, unsigned int len)
{
	if (!addr && !len)
		Xil_DCacheFlush();
	else
		Xil_DCacheFlushRange((intptr_t)addr, len);
}

void metal_machine_cache_invalidate(void *addr, unsigned int len)
{
	if (!addr && !len)
		Xil_DCacheInvalidate();
	else
		Xil_DCacheInvalidateRange((intptr_t)addr, len);
}

/**
 * @brief poll function until some event happens
 */
void metal_weak metal_generic_default_poll(void)
{
	taskYIELD();
}

/**
 * The code moved to cortexr5/xil_mpu.c:Xil_MemMap()
 * NULL in pa masks possible Xil_MemMap() errors.
 */
void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
			       size_t size, unsigned int flags)
{
	void* __attribute__((unused)) physaddr = Xil_MemMap(pa, size, flags);
	metal_assert(physaddr == (void *)pa);
	return va;
}
