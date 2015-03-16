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
 * 3. Neither the name of Mentor Graphics Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdio.h>
#include "hil.h"
#include "xil_cache.h"
/* ------------------------- Macros --------------------------*/

/********************/
/* Register offsets */
/********************/

/* -- FIX ME: ipi info is to be defined -- */
struct ipi_info {
	uint32_t ipi_base_addr;
	uint32_t ipi_chn_mask;
};

int _enable_interrupt(struct proc_vring *vring_hw);
void _reg_ipi_after_deinit(struct proc_vring *vring_hw);
void _notify(int cpu_id, struct proc_intr *intr_info);
int _boot_cpu(int cpu_id, unsigned int load_addr);
void _shutdown_cpu(int cpu_id);
void platform_isr(int vect_id, void *data);
void deinit_isr(int vect_id, void *data);

/* define function macros for OpenAMP API */
#define platform_cache_all_flush_invalidate() \
	{ \
		Xil_DCacheFlush(); \
		Xil_DCacheInvalidate(); \
		Xil_ICacheInvalidate(); \
	}

#define platform_cache_disable() \
	{ \
		Xil_DCacheDisable(); \
		Xil_ICacheDisable(); \
	}

#define platform_dcache_all_flush() { Xil_DCacheFlush(); }

#define platform_dcache_flush_range(addr, len) { Xil_DCacheFlushRange(addr, len); }

#define platform_interrupt_enable(...) zynqMP_r5_gic_interrupt_enable(__VA_ARGS__)
#define platform_interrupt_disable(...) zynqMP_r5_gic_interrupt_disable(__VA_ARGS__)
#define platform_map_mem_region(...)

#define platform_vatopa(addr) ((unsigned long)addr)
#define platform_patova(addr) ((void *)addr)
/* IPI REGs OFFSET */
#define IPI_TRIG_OFFSET          0x00000000 /* IPI trigger register offset */
#define IPI_OBS_OFFSET           0x00000004 /* IPI observation register offset */
#define IPI_ISR_OFFSET           0x00000010 /* IPI interrupt status register offset */
#define IPI_IMR_OFFSET           0x00000014 /* IPI interrupt mask register offset */
#define IPI_IER_OFFSET           0x00000018 /* IPI interrupt enable register offset */
#define IPI_IDR_OFFSET           0x0000001C /* IPI interrupt disable register offset */

/* IPC Device parameters */
#define SHM_ADDR                          (void *)0x3ED08000
#define SHM_SIZE                          0x00200000
#define IPI_BASEADDR                      0xff310000
#define IPI_CHN_BITMASK                   0x00000001 /* IPI channel bit mask APU<->RPU0 */
#define VRING0_IPI_INTR_VECT              -1
#define VRING1_IPI_INTR_VECT              79
#define MASTER_CPU_ID                     0
#define REMOTE_CPU_ID                     1

#endif /* PLATFORM_H_ */
