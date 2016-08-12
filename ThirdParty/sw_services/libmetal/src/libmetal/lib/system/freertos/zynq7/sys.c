/*
 * Copyright (c) 2014, Mentor Graphics Corporation
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
 * @file	freertos/sys.c
 * @brief	machine specific system primitives implementation.
 */

#include <stdint.h>
#include "xil_cache.h"
#include "xil_mmu.h"
#include "metal/io.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "metal/sys.h"

/* Translation table is 16K in size */
#define     ARM_AR_MEM_TTB_SIZE                    16*1024

/* Each TTB descriptor covers a 1MB region */
#define     ARM_AR_MEM_TTB_SECT_SIZE               1024*1024

/* Mask off lower bits of addr */
#define     ARM_AR_MEM_TTB_SECT_SIZE_MASK          (~(ARM_AR_MEM_TTB_SECT_SIZE-1UL))

/* Define shift to convert memory address to index of translation table entry (descriptor).
 Shift 20 bits (for a 1MB section) - 2 bits (for a 4 byte TTB descriptor) */
#define     ARM_AR_MEM_TTB_SECT_TO_DESC_SHIFT      (20-2)

/* Macro used to make a 32-bit value with the specified bit set */
#define             ESAL_GE_MEM_32BIT_SET(bit_num)      (1UL<<(bit_num))

#define     ARM_AR_MEM_TTB_DESC_BACKWARDS          ESAL_GE_MEM_32BIT_SET(4)
#define     ARM_AR_MEM_TTB_DESC_AP_MANAGER        (ESAL_GE_MEM_32BIT_SET(10)        |          \
                                                    ESAL_GE_MEM_32BIT_SET(11))
#define     ARM_AR_MEM_TTB_DESC_SECT               ESAL_GE_MEM_32BIT_SET(1)

/* Define translation table descriptor bits */
#define     ARM_AR_MEM_TTB_DESC_B                  ESAL_GE_MEM_32BIT_SET(2)
#define     ARM_AR_MEM_TTB_DESC_C                  ESAL_GE_MEM_32BIT_SET(3)
#define     ARM_AR_MEM_TTB_DESC_TEX                ESAL_GE_MEM_32BIT_SET(12)
#define     ARM_AR_MEM_TTB_DESC_S                  ESAL_GE_MEM_32BIT_SET(16)

/* Define all access  (manager access permission / not cachable / not bufferd)  */
#define     ARM_AR_MEM_TTB_DESC_ALL_ACCESS         (ARM_AR_MEM_TTB_DESC_AP_MANAGER |          \
                                                     ARM_AR_MEM_TTB_DESC_SECT)


static unsigned int int_old_val = 0;

void sys_irq_restore_enable(void)
{
	Xil_ExceptionEnableMask(int_old_val);
}

void sys_irq_save_disable(void)
{
	unsigned int value = 0;

	value = mfcpsr() & XIL_EXCEPTION_ALL;

	if (value != int_old_val) {
		Xil_ExceptionDisableMask(XIL_EXCEPTION_ALL);
		int_old_val = value;
	}
}

void metal_machine_cache_flush(void *addr, unsigned int len)
{
	if (!addr & !len)
		Xil_DCacheFlush();
	else
		Xil_DCacheFlushRange((intptr_t)addr, len);
}

void metal_machine_cache_invalidate(void *addr, unsigned int len)
{
	if (!addr & !len)
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

void metal_machine_io_mem_map(metal_phys_addr_t pa,
				      size_t size, unsigned int flags)
{
	unsigned int section_offset;
	unsigned int ttb_addr;
	unsigned int ttb_value;

	/* Ensure the virtual and physical addresses are aligned on a
	   section boundary */
	pa &= ARM_AR_MEM_TTB_SECT_SIZE_MASK;
	ttb_value = ARM_AR_MEM_TTB_DESC_ALL_ACCESS;

	/* Loop through entire region of memory (one MMU section at a time).
	   Each section requires a TTB entry. */
	for (section_offset = 0; section_offset < size;
	     section_offset += ARM_AR_MEM_TTB_SECT_SIZE) {

		/* Calculate translation table entry for this memory section */
		ttb_addr = (pa + section_offset);

		/* Build translation table entry value */

		/* Set cache related bits in translation table entry.
		   NOTE: Default is uncached instruction and data. */
		if ((flags & METAL_CACHE_WB)) {
			/* Update translation table entry value */
			ttb_value |=
			    (ARM_AR_MEM_TTB_DESC_B |
			     ARM_AR_MEM_TTB_DESC_C);
		} else if ((flags & METAL_CACHE_WT)) {
			/* Update translation table entry value */
			ttb_value |= ARM_AR_MEM_TTB_DESC_C;
		}
		/* In case of un-cached memory, set TEX 0 bit to set memory
		   attribute to normal. */
		else if ((flags & METAL_UNCACHED)) {
			ttb_value |= ARM_AR_MEM_TTB_DESC_TEX;
		}

		/* Write translation table entry value to entry address */
		Xil_SetTlbAttributes(ttb_addr, ttb_value);

	}
}
