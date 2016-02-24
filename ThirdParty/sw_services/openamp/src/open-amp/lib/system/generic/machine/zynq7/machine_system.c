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
#include "machine.h"
#include "machine_system.h"
#include "openamp/env.h"
#include "openamp/hil.h"
#include "xscugic.h"
#include "xil_cache_l.h"
#include "xil_mmu.h"



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
	unsigned long bit_shift;
	unsigned long temp32 = 0;

	(void)polarity;

	/* Determine the necessary bit shift in this target / priority register
	  for this interrupt vector ID */
	bit_shift = ((vector_id) % 4) * 8;

	/* Read-modify-write the priority register for this interrupt */
	temp32 = XScuGic_ReadReg(XPAR_SCUGIC_0_DIST_BASEADDR, XSCUGIC_PRIORITY_OFFSET_CALC(vector_id));

	/* Set new priority. */
	temp32 |= (priority << (bit_shift + 4));
	XScuGic_WriteReg(XPAR_SCUGIC_0_DIST_BASEADDR, XSCUGIC_PRIORITY_OFFSET_CALC(vector_id), temp32);

	/* Write to the appropriate bit in the enable set register for this
	 vector ID to enable the interrupt */

	XScuGic_EnableIntr(XPAR_SCUGIC_0_DIST_BASEADDR, vector_id);
	/* Return the vector ID */
	return (vector_id);
}

int platform_interrupt_disable(unsigned int vector_id)
{
	XScuGic_DisableIntr(XPAR_SCUGIC_0_DIST_BASEADDR, vector_id);
	return (vector_id);
}

int old_value = 0;

void restore_global_interrupts()
{
	ARM_AR_INT_BITS_SET(old_value);
}

void disable_global_interrupts()
{
	int value = 0;
	ARM_AR_INT_BITS_GET(&value);
	if (value != old_value) {
		ARM_AR_INT_BITS_SET(ARM_AR_INTERRUPTS_DISABLE_BITS);
		old_value = value;
	}
}

/***********************************************************************
 *
 *
 * arm_ar_map_mem_region
 *
 *
 * This function sets-up the region of memory based on the given
 * attributes
 *
 * @param vrt_addr       - virtual address of region
 * @param phy_addr       - physical address of region
 * @parma size           - size of region
 * @param is_mem_mapped  - memory mapped or not
 * @param cache_type     - cache type of region
 *
 *
 *   OUTPUTS
 *
 *       None
 *
 ***********************************************************************/
void arm_ar_map_mem_region(unsigned int vrt_addr, unsigned int phy_addr,
			   unsigned int size, int is_mem_mapped,
			   CACHE_TYPE cache_type)
{
	unsigned int ttb_value;
	phy_addr &= ARM_AR_MEM_TTB_SECT_SIZE_MASK;
	vrt_addr &= ARM_AR_MEM_TTB_SECT_SIZE_MASK;
	ttb_value = ARM_AR_MEM_TTB_DESC_ALL_ACCESS;

	(void)size;

	if (!is_mem_mapped) {

		/* Set cache related bits in translation table entry.
		 NOTE: Default is uncached instruction and data. */
		if (cache_type == WRITEBACK) {
			/* Update translation table entry value */
			ttb_value |= (ARM_AR_MEM_TTB_DESC_B | ARM_AR_MEM_TTB_DESC_C);
		} else if (cache_type == WRITETHROUGH) {
			/* Update translation table entry value */
			ttb_value |= ARM_AR_MEM_TTB_DESC_C;
		}
		/* In case of un-cached memory, set TEX 0 bit to set memory
		 attribute to normal. */
		else if (cache_type == NOCACHE) {
			ttb_value |= ARM_AR_MEM_TTB_DESC_TEX;
		}
	}

	Xil_SetTlbAttributes(phy_addr,ttb_value);

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

void platform_cache_all_flush_invalidate(void)
{
	Xil_L1DCacheFlush();
}

void platform_cache_disable(void)
{
	Xil_L1DCacheDisable();
}

unsigned long platform_vatopa(void *addr)
{
	return (((unsigned long)addr & (~(0x0fff << 20))) | (0x08 << 24));
}

void *platform_patova(unsigned long addr)
{
	return ((void *)addr);

}

void platform_isr(int vect_id, void * data)
{
    (void)vect_id;
    hil_isr(((struct proc_vring *)data));
}
