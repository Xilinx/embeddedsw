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
#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "platform_info.h"


#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID

extern void bm_env_isr(int vector);
extern XScuGic xInterruptController;


/*-----------------------------------------------------------------------*/
/* The OpenAMP library is currently using env_allocate_memory()          */
/* which cannot be used inside ISR with FreeRTOS.                         */
/* We re-implement interrupt processing with deferred call              */
/*-----------------------------------------------------------------------*/
#include "openamp/env.h"
#include "rsc_table.h"
#include "openamp/machine/zynqmp_r5/machine.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

//#define IPI_ISR_OFFSET           0x00000010 /* IPI interrupt status register offset */
//#define IPI_TOTAL 11

typedef void (*ipi_handler_t)(unsigned long ipi_base_addr, unsigned int intr_mask, void *data);

extern struct hil_platform_ops proc_ops;

/* proc_ops set to override the library _enable_interrupt() */
//static int app_enable_interrupt(struct proc_vring *vring_hw)
//{
//	struct ipi_info *chn_ipi_info =
//	    (struct ipi_info *)(vring_hw->intr_info.data);
//	unsigned int ipi_base_addr = chn_ipi_info->ipi_base_addr;
//
//	if (vring_hw->intr_info.vect_id == 0xFFFFFFFF) {
//		return 0;
//	}
//
//	/* Register ISR */
//	env_register_isr_shared(vring_hw->intr_info.vect_id,
//			 vring_hw, _ipi_handler, "remoteproc_a53", 1);
//	/* Enable IPI interrupt */
//	env_enable_interrupt(vring_hw->intr_info.vect_id,
//			     vring_hw->intr_info.priority,
//			     vring_hw->intr_info.trigger_type);
//	Xil_Out32((ipi_base_addr + IPI_IER_OFFSET),
//		chn_ipi_info->ipi_chn_mask);
//	return 0;
//}

/*--------------------------------------------------------------------------------*/

/* wrapper around generic ISR from library */
static void app_irq_isr(void *intr_id_ptr)
{
    bm_env_isr(*(unsigned int *)intr_id_ptr);
}

/* Complete Interrupt Controller setup, FreeRTOS is doing the pre-init */
static int app_gic_initialize(void)
{
	void *intr_id;

	/* Connect Interrupt ID with ISR */
	intr_id = (void *)VRING1_IPI_INTR_VECT;
	XScuGic_Connect(&xInterruptController, VRING1_IPI_INTR_VECT,
			   (Xil_ExceptionHandler)app_irq_isr,
			   intr_id);

	return 0;
}

/* Main hw machinery initialization entry point, called from main()*/
/* return 0 on success */
int init_system(void)
{
	/* configure the global interrupt controller */
	app_gic_initialize();

	/* override default _enable_interrupt() to use deferred call */
	//proc_ops.enable_interrupt = app_enable_interrupt;

    return 0;
}
