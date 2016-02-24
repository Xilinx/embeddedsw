/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2015 Xilinx, Inc.
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

#include "openamp/hil.h"
#include "machine.h"

/* -- FIX ME: ipi info is to be defined -- */
struct ipi_info {
	uint32_t ipi_base_addr;
	uint32_t ipi_chn_mask;
};

/*--------------------------- Declare Functions ------------------------ */
static int _enable_interrupt(struct proc_vring *vring_hw);
static void _reg_ipi_after_deinit(struct proc_vring *vring_hw);
static void _notify(int cpu_id, struct proc_intr *intr_info);
static int _boot_cpu(int cpu_id, unsigned int load_addr);
static void _shutdown_cpu(int cpu_id);

static void _ipi_handler(int vect_id, void *data);
static void _ipi_handler_deinit(int vect_id, void *data);

/*--------------------------- Globals ---------------------------------- */
struct hil_platform_ops proc_ops = {
	.enable_interrupt     = _enable_interrupt,
	.reg_ipi_after_deinit = _reg_ipi_after_deinit,
	.notify               = _notify,
	.boot_cpu             = _boot_cpu,
	.shutdown_cpu         = _shutdown_cpu,
};

/* Extern functions defined out from OpenAMP lib */
extern void ipi_enable_interrupt(unsigned int vector);
extern void ipi_isr(int vect_id, void *data);

/* static variables */
static struct ipi_info saved_ipi_info;

/*------------------- Extern variable -----------------------------------*/
extern struct hil_proc proc_table[];
extern const int proc_table_size;

void _ipi_handler(int vect_id, void *data)
{
	(void) vect_id;
	struct proc_vring *vring_hw = (struct proc_vring *)(data);
	struct ipi_info *chn_ipi_info =
		(struct ipi_info *)(vring_hw->intr_info.data);
	unsigned int ipi_base_addr = chn_ipi_info->ipi_base_addr;
	unsigned int ipi_intr_status =
	    (unsigned int)HIL_MEM_READ32(ipi_base_addr + IPI_ISR_OFFSET);
	if (ipi_intr_status & chn_ipi_info->ipi_chn_mask) {
		platform_dcache_all_flush();
		platform_isr(vect_id, data);
		HIL_MEM_WRITE32((ipi_base_addr + IPI_ISR_OFFSET),
				chn_ipi_info->ipi_chn_mask);
	}
}

void _ipi_handler_deinit(int vect_id, void *data)
{
	(void) vect_id;
	struct ipi_info *chn_ipi_info =
		(struct ipi_info *)(data);
	unsigned int ipi_base_addr = chn_ipi_info->ipi_base_addr;
	unsigned int ipi_intr_status =
	    (unsigned int)HIL_MEM_READ32(ipi_base_addr + IPI_ISR_OFFSET);
	if (ipi_intr_status & chn_ipi_info->ipi_chn_mask) {
		HIL_MEM_WRITE32((ipi_base_addr + IPI_ISR_OFFSET),
				chn_ipi_info->ipi_chn_mask);
	}
	return;
}

static int _enable_interrupt(struct proc_vring *vring_hw)
{
	struct ipi_info *chn_ipi_info =
	    (struct ipi_info *)(vring_hw->intr_info.data);
	unsigned int ipi_base_addr = chn_ipi_info->ipi_base_addr;

	if (vring_hw->intr_info.vect_id == 0xFFFFFFFF) {
		return 0;
	}

	/* Register ISR */
	env_register_isr_shared(vring_hw->intr_info.vect_id,
			 vring_hw, _ipi_handler, "remoteproc_a53", 1);
	/* Enable IPI interrupt */
	env_enable_interrupt(vring_hw->intr_info.vect_id,
			     vring_hw->intr_info.priority,
			     vring_hw->intr_info.trigger_type);
	HIL_MEM_WRITE32((ipi_base_addr + IPI_IER_OFFSET),
		chn_ipi_info->ipi_chn_mask);
	return 0;
}

/* In case there is an interrupt received after deinit. */
static void _reg_ipi_after_deinit(struct proc_vring *vring_hw)
{
	struct ipi_info *chn_ipi_info =
	    (struct ipi_info *)(vring_hw->intr_info.data);

	if (vring_hw->intr_info.vect_id == 0xFFFFFFFF)
		return;
	saved_ipi_info.ipi_base_addr = chn_ipi_info->ipi_base_addr;
	saved_ipi_info.ipi_chn_mask = chn_ipi_info->ipi_chn_mask;

	env_update_isr(vring_hw->intr_info.vect_id, &saved_ipi_info,
                    _ipi_handler_deinit,
                    "remoteproc_a53", 1);
}

static void _notify(int cpu_id, struct proc_intr *intr_info)
{

	(void)cpu_id;
	struct ipi_info *chn_ipi_info = (struct ipi_info *)(intr_info->data);
	if (chn_ipi_info == NULL)
		return;
	platform_dcache_all_flush();
	env_wmb();

	/* Trigger IPI */
	HIL_MEM_WRITE32((chn_ipi_info->ipi_base_addr + IPI_TRIG_OFFSET),
			chn_ipi_info->ipi_chn_mask);
}

static int _boot_cpu(int cpu_id, unsigned int load_addr)
{
	(void)cpu_id;
	(void)load_addr;
	return -1;
}

static void _shutdown_cpu(int cpu_id)
{
	(void)cpu_id;
	return;
}

/**
 * platform_get_processor_info
 *
 * Copies the target info from the user defined data structures to
 * HIL proc  data structure.In case of remote contexts this function
 * is called with the reserved CPU ID HIL_RSVD_CPU_ID, because for
 * remotes there is only one master.
 *
 * @param proc   - HIL proc to populate
 * @param cpu_id - CPU ID
 *
 * return  - status of execution
 */
int platform_get_processor_info(struct hil_proc *proc , int cpu_id)
{
	int idx;
	unsigned int u_cpu_id = cpu_id;

	for(idx = 0; idx < proc_table_size; idx++) {
		if ((u_cpu_id == HIL_RSVD_CPU_ID) || (proc_table[idx].cpu_id == u_cpu_id)) {
			env_memcpy(proc,&proc_table[idx], sizeof(struct hil_proc));
			return 0;
		}
	}
	return -1;
}

int platform_get_processor_for_fw(char *fw_name)
{
	(void)fw_name;
	return 1;
}
