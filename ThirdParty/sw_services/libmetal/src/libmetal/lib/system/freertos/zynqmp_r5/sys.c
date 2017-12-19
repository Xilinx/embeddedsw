/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Xilinx nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
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

/*
 * @file	freertos/zynqmp_r5/sys.c
 * @brief	machine specific system primitives implementation.
 */

#include <stdint.h>
#include "xil_cache.h"
#include "xreg_cortexr5.h"
#include "xil_mmu.h"
#include "xil_mpu.h"
#include "xscugic.h"
#include "xil_exception.h"
#include <metal/io.h>
#include <metal/sys.h>

#define MPU_REGION_SIZE_MIN 0x20

/* default value setting for disabling interrupts */
static unsigned int int_old_val = XIL_EXCEPTION_ALL;

void sys_irq_restore_enable(void)
{
	Xil_ExceptionEnableMask(~int_old_val);
}

void sys_irq_save_disable(void)
{
	int_old_val = mfcpsr() & XIL_EXCEPTION_ALL;

	if (XIL_EXCEPTION_ALL != int_old_val) {
		Xil_ExceptionDisableMask(XIL_EXCEPTION_ALL);
	}
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
void __attribute__((weak)) metal_generic_default_poll(void)
{
	asm volatile("wfi");
}

void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
			       size_t size, unsigned int flags)
{
	size_t rsize = MPU_REGION_SIZE_MIN;
	metal_phys_addr_t base_pa;

	if (!flags)
		return va;
	while(1) {
		if (rsize < size) {
			rsize <<= 1;
			continue;
		} else {
			base_pa = pa & ~(rsize - 1);
			if ((base_pa + rsize) < (pa + size)) {
				rsize <<= 1;
				continue;
			}
			break;
		}
	}
	Xil_SetMPURegion(base_pa, rsize, flags);
	return va;
}
