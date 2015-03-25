/*
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
 *       for Xilinx Zynq ZC702EVK platform.
 *
 **************************************************************************/

#include "platform.h"
#include "baremetal.h"

/*--------------------------- Globals ---------------------------------- */
struct hil_platform_ops proc_ops = {
	.enable_interrupt	= _enable_interrupt,
	.reg_ipi_after_deinit	= _reg_ipi_after_deinit,
	.notify			= _notify,
	.boot_cpu		= _boot_cpu,
	.shutdown_cpu		= _shutdown_cpu,
};

extern void ipi_enable_interrupt(unsigned int vector);
extern void ipi_isr(int vect_id, void *data);

extern void ipi_register_interrupt(unsigned long ipi_base_addr, unsigned int intr_mask, void *data, void *ipi_handler);

void _ipi_handler (unsigned long ipi_base_addr, unsigned int intr_mask, void *data) {
	struct proc_vring *vring_hw = (struct proc_vring *) data;
	platform_dcache_all_flush();
	hil_isr(vring_hw);
}

void _ipi_handler_deinit (unsigned long ipi_base_addr, unsigned int intr_mask, void *data) {
	return;
}

int _enable_interrupt(struct proc_vring *vring_hw) {

	struct ipi_info *chn_ipi_info = (struct ipi_info *)(vring_hw->intr_info.data);

	if (vring_hw->intr_info.vect_id < 0)
		return 0;
	/* Register IPI handler */
	ipi_register_handler(chn_ipi_info->ipi_base_addr, chn_ipi_info->ipi_chn_mask, vring_hw, _ipi_handler);
	/* Register ISR*/
	env_register_isr(vring_hw->intr_info.vect_id, &(chn_ipi_info->ipi_base_addr), ipi_isr);
	/* Enable IPI interrupt */
	env_enable_interrupt(vring_hw->intr_info.vect_id,
		vring_hw->intr_info.priority,
		vring_hw->intr_info.trigger_type);
	return 0;
}

void _reg_ipi_after_deinit(struct proc_vring *vring_hw) {
	struct ipi_info *chn_ipi_info = (struct ipi_info *)(vring_hw->intr_info.data);
	env_disable_interrupts();
	ipi_register_handler(chn_ipi_info->ipi_base_addr, chn_ipi_info->ipi_chn_mask, 0, _ipi_handler_deinit);
	env_restore_interrupts();
}

void _notify(int cpu_id, struct proc_intr *intr_info) {

	struct ipi_info *chn_ipi_info = (struct ipi_info *)(intr_info->data);
	if (chn_ipi_info == NULL)
		return;
	platform_dcache_all_flush();
	/* Trigger IPI */
	ipi_trigger(chn_ipi_info->ipi_base_addr, chn_ipi_info->ipi_chn_mask);
}

int _boot_cpu(int cpu_id, unsigned int load_addr) {
	return -1;
}

void _shutdown_cpu(int cpu_id) {
	return;
}

