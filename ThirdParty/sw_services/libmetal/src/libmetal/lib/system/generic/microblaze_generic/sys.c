/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	generic/microblaze_generic/sys.c
 * @brief	machine specific system primitives implementation.
 */

#include <metal/assert.h>
#include <metal/io.h>
#include <metal/sys.h>
#include <stdint.h>
#include <xil_cache.h>
#include <xil_exception.h>
#include <xparameters.h>

#define MSR_IE  0x2UL /* MicroBlaze status register interrupt enable mask */

#if (XPAR_MICROBLAZE_USE_MSR_INSTR != 0)
unsigned int sys_irq_save_disable(void)
{
	unsigned int state;

	asm volatile("  mfs     %0, rmsr	\n"
		     "  msrclr  r0, %1		\n"
		     :  "=r"(state)
		     :  "i"(MSR_IE)
		     :  "memory");

	return state &= MSR_IE;
}

void sys_irq_restore_enable(unsigned int flags)
{
	unsigned int tmp;
	if (flags)
		asm volatile("  msrset %0, %1	\n"
			 :  "=r"(tmp)
			 :  "i"(MSR_IE)
			 :  "memory");
}

#else /* XPAR_MICROBLAZE_USE_MSR_INSTR == 0 */
unsigned int sys_irq_save_disable(void)
{
	unsigned int tmp, state;

	asm volatile ("  mfs   %0, rmsr		\n"
		      "  andi  %1, %0, %2	\n"
		      "  mts   rmsr, %1		\n"
		      :  "=r"(state), "=r"(tmp)
		      :  "i"(~MSR_IE)
		      :  "memory");

	return state &= MSR_IE;
}

void sys_irq_restore_enable(unsigned int flags)
{
	unsigned int tmp;

	if (flags)
		asm volatile("  mfs    %0, rmsr		\n"
			 "  or      %0, %0, %1	\n"
			 "  mts     rmsr, %0	\n"
			 :  "=r"(tmp)
			 :  "r"(flags)
			 :  "memory");
}
#endif /* XPAR_MICROBLAZE_USE_MSR_INSTR */

void metal_machine_cache_flush(void *addr, unsigned int len)
{
	if (!addr && !len){
		Xil_DCacheFlush();
	}
	else{
		Xil_DCacheFlushRange((intptr_t)addr, len);
	}
}

void metal_machine_cache_invalidate(void *addr, unsigned int len)
{
	if (!addr && !len){
		Xil_DCacheInvalidate();
	}
	else {
		Xil_DCacheInvalidateRange((intptr_t)addr, len);
	}
}

/**
 * @brief make microblaze wait
 */
void metal_weak metal_generic_default_poll(void)
{
	asm volatile("nop");
}

void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
			       size_t size, unsigned int flags)
{
	(void)pa;
	(void)size;
	(void)flags;
	return va;
}
