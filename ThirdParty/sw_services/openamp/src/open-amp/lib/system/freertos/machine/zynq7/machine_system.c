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
#include <stdio.h>
#include <string.h>
#include "baremetal.h"
#include "machine_system.h"
#include "openamp/env.h"
#include "task.h"

static inline unsigned int get_cpu_id_arm(void)
{
	unsigned long cpu_id = 0;

	asm volatile ("MRC p15 ,"
		      "0," "%0," "c0," "c0," "5":[cpu_id] "=&r"(cpu_id)
		      : /* No inputs */ );

	/*
	 * Return cpu id to caller, extract last two bits from Multiprocessor
	 * Affinity Register */
	return (cpu_id & 0x03);
}

int platform_interrupt_enable(unsigned int vector_id, unsigned int polarity,
			      unsigned int priority)
{
	unsigned long reg_offset;
	unsigned long bit_shift;
	unsigned long temp32 = 0;
	unsigned long targ_cpu;

	temp32 = get_cpu_id_arm();

	/* Determine the necessary bit shift in this target / priority register
	   for this interrupt vector ID */
	bit_shift = ((vector_id) % 4) * 8;

	/* Build a target value based on the bit shift calculated above and the CPU core
	   that this code is executing on */
	targ_cpu = (1 << temp32) << bit_shift;

	/* Determine the Global interrupt controller target / priority register
	   offset for this interrupt vector ID
	   NOTE:  Each target / priority register supports 4 interrupts */
	reg_offset = ((vector_id) / 4) * 4;

	/* Read-modify-write the priority register for this interrupt */
	temp32 = MEM_READ32(INT_GIC_DIST_BASE + INT_GIC_DIST_PRI + reg_offset);

	/* Set new priority. */
	temp32 |= (priority << (bit_shift + 4));
	MEM_WRITE32(INT_GIC_DIST_BASE + INT_GIC_DIST_PRI + reg_offset, temp32);

	/* Read-modify-write the target register for this interrupt to allow this
	   cpu to accept this interrupt */
	temp32 =
	    MEM_READ32(INT_GIC_DIST_BASE + INT_GIC_DIST_TARGET + reg_offset);
	temp32 |= targ_cpu;
	MEM_WRITE32(INT_GIC_DIST_BASE + INT_GIC_DIST_TARGET + reg_offset,
		    temp32);

	/* Determine the Global interrupt controller enable set register offset
	   for this vector ID
	   NOTE:  There are 32 interrupts in each enable set register */
	reg_offset = (vector_id / 32) * 4;

	/* Write to the appropriate bit in the enable set register for this
	   vector ID to enable the interrupt */

	temp32 = (1UL << (vector_id - (reg_offset * 0x08)));
	MEM_WRITE32(INT_GIC_DIST_BASE + INT_GIC_DIST_ENABLE_SET + reg_offset,
		    temp32);

	/* Return the vector ID */
	return (vector_id);
}

int platform_interrupt_disable(unsigned int vector_id)
{
	unsigned long reg_offset;
	unsigned long bit_shift;
	unsigned long temp32 = 0;
	unsigned long targ_cpu;

	temp32 = get_cpu_id_arm();

	/* Determine the Global interrupt controller enable set register offset
	   for this vector ID
	   NOTE:  There are 32 interrupts in each enable set register */
	reg_offset = (vector_id / 32) * 4;

	/* Write to the appropriate bit in the enable clear register for this
	   vector ID to disable the interrupt */

	MEM_WRITE32(INT_GIC_DIST_BASE + INT_GIC_DIST_ENABLE_CLEAR + reg_offset,
		    (1UL << (vector_id - (reg_offset * 0x08))));

	/* Determine the Global interrupt controller target register offset for
	   this interrupt vector ID
	   NOTE:  Each target register supports 4 interrupts */
	reg_offset = (vector_id / 4) * 4;

	/* Determine the necessary bit shift in this target register for this
	   vector ID */
	bit_shift = (vector_id % 4) * 8;

	/* Build a value based on the bit shift calculated above and the CPU core
	   that this code is executing on */
	targ_cpu = (1 << temp32) << bit_shift;

	/* Read-modify-write the target register for this interrupt and remove this cpu from
	   accepting this interrupt */
	temp32 =
	    MEM_READ32(INT_GIC_DIST_BASE + INT_GIC_DIST_TARGET + reg_offset);
	temp32 &= ~targ_cpu;

	MEM_WRITE32(INT_GIC_DIST_BASE + INT_GIC_DIST_TARGET + reg_offset,
		    temp32);

	/* Return the vector ID */
	return (vector_id);
}

void restore_global_interrupts()
{
	taskENABLE_INTERRUPTS();
}

void disable_global_interrupts()
{
	taskDISABLE_INTERRUPTS();
}

void platform_map_mem_region(unsigned int vrt_addr, unsigned int phy_addr,
			     unsigned int size, unsigned int flags)
{
	int is_mem_mapped = 0;
	int cache_type = 0;

	if ((flags & (0x0f << 4)) == MEM_MAPPED) {
		is_mem_mapped = 1;
	}

	if ((flags & 0x0f) == WB_CACHE) {
		cache_type = WRITEBACK;
	} else if ((flags & 0x0f) == WT_CACHE) {
		cache_type = WRITETHROUGH;
	} else {
		cache_type = NOCACHE;
	}

	arm_ar_map_mem_region(vrt_addr, phy_addr, size, is_mem_mapped,
			      cache_type);
}

void platform_cache_all_flush_invalidate()
{
	ARM_AR_MEM_DCACHE_ALL_OP(1);
}

void platform_cache_disable()
{
	ARM_AR_MEM_CACHE_DISABLE();
}

unsigned long platform_vatopa(void *addr)
{
	return (((unsigned long)addr & (~(0x0fff << 20))) | (0x08 << 24));
}

void *platform_patova(unsigned long addr)
{
	return ((void *)addr);

}
