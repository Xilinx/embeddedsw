/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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

#ifndef _BAREMETAL_H
#define _BAREMETAL_H

#include "amp_os.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xreg_cortexr5.h"
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#endif

struct XOpenAMPInstPtr{
	unsigned int IntrID;
	unsigned int IPI_Status;
	void *lock;
};

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID

#define VRING1_IPI_INTR_VECT              65

/* IPI REGs OFFSET */
#define IPI_TRIG_OFFSET          0x00000000 /* IPI trigger register offset */
#define IPI_OBS_OFFSET           0x00000004 /* IPI observation register offset */
#define IPI_ISR_OFFSET           0x00000010 /* IPI interrupt status register offset */
#define IPI_IMR_OFFSET           0x00000014 /* IPI interrupt mask register offset */
#define IPI_IER_OFFSET           0x00000018 /* IPI interrupt enable register offset */
#define IPI_IDR_OFFSET           0x0000001C /* IPI interrupt disable register offset */

#define platform_dcache_all_flush() { Xil_DCacheFlush(); }

#define platform_dcache_flush_range(addr, len) { Xil_DCacheFlushRange(addr, len); }

#define CORTEXR5_CPSR_INTERRUPTS_BITS (XREG_CPSR_IRQ_ENABLE | XREG_CPSR_FIQ_ENABLE)

/* This macro writes the current program status register (CPSR - all fields) */
#define ARM_AR_CPSR_CXSF_WRITE(cpsr_cxsf_value) \
	{ \
		asm volatile("    MSR     CPSR_cxsf, %0" \
			: /* No outputs */ \
			: "r" (cpsr_cxsf_value) ); \
	}

/* This macro sets the interrupt related bits in the status register / control
 register to the specified value. */
#define ARM_AR_INT_BITS_SET(set_bits) \
	{ \
		int     tmp_val; \
		tmp_val = mfcpsr(); \
		tmp_val &= ~((unsigned int)CORTEXR5_CPSR_INTERRUPTS_BITS); \
		tmp_val |= set_bits; \
		ARM_AR_CPSR_CXSF_WRITE(tmp_val); \
	}

/* This macro gets the interrupt related bits from the status register / control
 register. */
#define ARM_AR_INT_BITS_GET(get_bits_ptr) \
	{ \
		int     tmp_val; \
		tmp_val = mfcpsr(); \
		tmp_val &= CORTEXR5_CPSR_INTERRUPTS_BITS; \
		*get_bits_ptr = tmp_val; \
	}
void zynqMP_r5_map_mem_region(u32 addr, u32 size, u32 attrib);

int zynqMP_r5_gic_initialize();
void zynqMP_r5_irq_isr();

void restore_global_interrupts();
void disable_global_interrupts();
int platform_interrupt_enable(unsigned int vector,unsigned int polarity, unsigned int priority);
int platform_interrupt_disable(unsigned int vector);
void platform_cache_all_flush_invalidate();
void platform_cache_disable();
void platform_map_mem_region(unsigned int va,unsigned int pa, unsigned int size, unsigned int flags);
unsigned long platform_vatopa(void *addr);
void *platform_patova(unsigned long addr);
void process_communication(struct XOpenAMPInstPtr OpenAMPInstance);
void ipi_register_handler(unsigned long ipi_base_addr, unsigned int intr_mask, void *data, void *ipi_handler);
void ipi_trigger(unsigned long ipi_base_addr, unsigned int trigger_mask);

#endif /* _BAREMETAL_H */
