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

/**************************************************************************
 * FILE NAME
 *
 *       platform.c
 *
 * DESCRIPTION
 *
 *       This file is the Implementation of IPC hardware layer interface
 *       for Xilinx ZynqMP platform.
 *
 **************************************************************************/

#include "platform.h"
#include "xil_io.h"
#include "xscugic.h"
#include "xil_cache.h"
/*--------------------------- Globals ---------------------------------- */
struct hil_platform_ops proc_ops = {
	.enable_interrupt	= _enable_interrupt,
	.reg_ipi_after_deinit	= _reg_ipi_after_deinit,
	.notify			= _notify,
	.boot_cpu		= _boot_cpu,
	.shutdown_cpu		= _shutdown_cpu,
};

unsigned int old_value = 0;

int _enable_interrupt(struct proc_vring *vring_hw) {

	if (vring_hw->intr_info.vect_id < 0)
		return 0;

	/* Register ISR*/
	env_register_isr(vring_hw->intr_info.vect_id, vring_hw, platform_isr);

	/* Enable IPI interrupt */
	struct ipi_info *chn_ipi_info = (struct ipi_info *)(vring_hw->intr_info.data);
	Xil_Out32((chn_ipi_info->ipi_base_addr + IPI_IER_OFFSET), chn_ipi_info->ipi_chn_mask);
	/* Enable the interrupts */
	env_enable_interrupt(vring_hw->intr_info.vect_id,
		vring_hw->intr_info.priority,
		vring_hw->intr_info.trigger_type);
	return 0;
}

void _reg_ipi_after_deinit(struct proc_vring *vring_hw) {
	struct ipi_info *chn_ipi_info = (struct ipi_info *)(vring_hw->intr_info.data);

	if (vring_hw->intr_info.vect_id < 0) {
		return;
	}
	env_update_isr(vring_hw->intr_info.vect_id, chn_ipi_info, deinit_isr);

}

void _notify(int cpu_id, struct proc_intr *intr_info) {

	struct ipi_info *chn_ipi_info = (struct ipi_info *)(intr_info->data);
	if (chn_ipi_info == NULL)
		return;
	platform_dcache_all_flush();
	/* Trigger IPI */
	Xil_Out32((chn_ipi_info->ipi_base_addr + IPI_TRIG_OFFSET), chn_ipi_info->ipi_chn_mask);
}

int _boot_cpu(int cpu_id, unsigned int load_addr) {
	return -1;
}

void _shutdown_cpu(int cpu_id) {
	return;
}

void platform_isr(int vect_id, void *data) {
	struct proc_vring *vring_hw = (struct proc_vring *) data;
	struct ipi_info *chn_ipi_info = (struct ipi_info *)(vring_hw->intr_info.data);
	unsigned int ipi_intr_status = (unsigned int)Xil_In32(chn_ipi_info->ipi_base_addr + IPI_ISR_OFFSET);
	if ((ipi_intr_status & chn_ipi_info->ipi_chn_mask)) {
		platform_dcache_all_flush();
		hil_isr(vring_hw);
		Xil_Out32((chn_ipi_info->ipi_base_addr + IPI_ISR_OFFSET), chn_ipi_info->ipi_chn_mask);
	}
}

void deinit_isr(int vect_id, void *data) {
	struct ipi_info *chn_ipi_info = (struct ipi_info *)data;
	unsigned int ipi_intr_status = (unsigned int)Xil_In32(chn_ipi_info->ipi_base_addr + IPI_ISR_OFFSET);
	if ((ipi_intr_status & chn_ipi_info->ipi_chn_mask)) {
		Xil_Out32((chn_ipi_info->ipi_base_addr + IPI_ISR_OFFSET), chn_ipi_info->ipi_chn_mask);
	}
}


void platform_interrupt_enable(u32 vector,u32 polarity, u32 priority) {
	XScuGic_EnableIntr(XPAR_SCUGIC_0_DIST_BASEADDR,vector);
}

void platform_interrupt_disable(unsigned int vector) {
	XScuGic_DisableIntr(XPAR_SCUGIC_0_DIST_BASEADDR,vector);
}

void platform_cache_all_flush_invalidate() {
		Xil_DCacheFlush();
		Xil_DCacheInvalidate();
		Xil_ICacheInvalidate();
}

void platform_cache_disable() {
		Xil_DCacheDisable();
		Xil_ICacheDisable();
}

void platform_map_mem_region(unsigned int va,unsigned int pa, unsigned int size,int is_mem_mapped,int cache_type) {
	return;
}

unsigned long platform_vatopa(unsigned long addr) {
	 return ((unsigned long)addr);
 }

void *platform_patova(unsigned long addr) {
	return ((void *)addr);
}

void restore_global_interrupts() {

	ARM_AR_INT_BITS_SET(old_value);

}

void disable_global_interrupts() {

	unsigned int value = 0;

	ARM_AR_INT_BITS_GET(&value);

	if (value != old_value) {

		ARM_AR_INT_BITS_SET(CORTEXR5_CPSR_INTERRUPTS_BITS);

		old_value = value;

	}

}
