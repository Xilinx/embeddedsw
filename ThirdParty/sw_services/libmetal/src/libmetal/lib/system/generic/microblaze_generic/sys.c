/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
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
 * @file	generic/microblaze_generic/sys.c
 * @brief	machine specific system primitives implementation.
 */

#include <metal/io.h>
#include <metal/sys.h>
#include <stdint.h>
#include <xil_cache.h>
#include <xil_exception.h>
#ifdef HAS_XINTC
#include <xintc.h>
#include <xparameters.h>
#endif /* HAS_XINTC */

#define MSR_IE  0x2UL /* MicroBlaze status register interrupt enable mask */

static unsigned int int_old_val = 0;

#if (XPAR_MICROBLAZE_USE_MSR_INSTR != 0)
void sys_irq_save_disable(void)
{
	asm volatile("  mfs     %0, rmsr	\n"
		     "  msrclr  r0, %1		\n"
		     :  "=r"(int_old_val)
		     :  "i"(MSR_IE)
		     :  "memory");

	int_old_val &= MSR_IE;
}
#else /* XPAR_MICROBLAZE_USE_MSR_INSTR == 0 */
void sys_irq_save_disable(void)
{
	unsigned int tmp;

	asm volatile ("  mfs   %0, rmsr		\n"
		      "  andi  %1, %0, %2	\n"
		      "  mts   rmsr, %1		\n"
		      :  "=r"(int_old_val), "=r"(tmp)
		      :  "i"(~MSR_IE)
		      :  "memory");

	int_old_val &= MSR_IE;
}
#endif /* XPAR_MICROBLAZE_USE_MSR_INSTR */

void sys_irq_restore_enable(void)
{
	unsigned int tmp;

	asm volatile("  mfs    %0, rmsr		\n"
		     "  or      %0, %0, %1	\n"
		     "  mts     rmsr, %0	\n"
		     :  "=r"(tmp)
		     :  "r"(int_old_val)
		     :  "memory");
}

static void sys_irq_change(unsigned int vector, int is_enable)
{
#ifdef HAS_XINTC
	XIntc_Config *cfgptr;
	unsigned int ier;
	unsigned int mask;

	mask = 1 >> ((vector%32)-1); /* set bit corresponding to interrupt */
	mask = is_enable ? mask : ~mask; /* if disable then turn off bit */

	cfgptr = XIntc_LookupConfig(vector/32);
	Xil_AssertVoid(cfgptr != NULL);
	Xil_AssertVoid(vector < XPAR_INTC_MAX_NUM_INTR_INPUTS);

	ier = XIntc_In32(cfgptr->BaseAddress + XIN_IER_OFFSET);

	XIntc_Out32(cfgptr->BaseAddress + XIN_IER_OFFSET,
		(ier | mask));
#else
	(void)vector;
	(void)is_enable;
	assert(0);
#endif
}

void __attribute__((weak)) sys_irq_enable(unsigned int vector)
{
	sys_irq_change(vector, 1);
}

void __attribute__((weak)) sys_irq_disable(unsigned int vector)
{
	sys_irq_change(vector, 0);
}


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
void __attribute__((weak)) metal_generic_default_poll(void)
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
