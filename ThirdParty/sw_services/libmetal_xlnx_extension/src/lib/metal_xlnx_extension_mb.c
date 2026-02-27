/*
 * Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	metal_xlnx_extension.c
 * @brief	machine specific system primitives implementation.
 */

#include <metal/assert.h>
#include <metal/device.h>
#include <metal/irq_controller.h>
#include <metal/irq.h>
#include <metal/sys.h>
#include <metal/log.h>
#include <metal/utilities.h>

#include "xil_cache.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xparameters.h"

#ifdef HAS_XINTC
#include <xintc.h>
#endif /* HAS_XINTC */

#define MSR_IE  0x2UL /* MicroBlaze status register interrupt enable mask */

#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID

#ifndef XLNX_MAXIRQS
#define XLNX_MAXIRQS 32
#endif

#define MAX_IRQS XLNX_MAXIRQS

extern struct metal_state _metal;
static struct metal_irq irqs[MAX_IRQS]; /**< Linux IRQs array */

static void metal_xlnx_irq_set_enable(struct metal_irq_controller *irq_cntr,
				      int irq, unsigned int state);

/*
 * Forward declaration of functions used
 * in metal_xlnx_irq_set_enable.
 */
void sys_irq_enable(unsigned int vector);
void sys_irq_disable(unsigned int vector);

/**< Xilinx common platform IRQ controller */
METAL_IRQ_CONTROLLER_DECLARE(xlnx_irq_cntr,
			     0, MAX_IRQS,
			     NULL,
			     metal_xlnx_irq_set_enable, NULL,
			     irqs);

static void metal_xlnx_irq_set_enable(struct metal_irq_controller *irq_cntr,
				     int irq, unsigned int state)
{
	if (irq < irq_cntr->irq_base ||
	    irq >= irq_cntr->irq_base + irq_cntr->irq_num) {
		metal_log(METAL_LOG_ERROR, "%s: invalid irq %d\n",
			  __func__, irq);
		return;
	} else if (state == METAL_IRQ_ENABLE) {
		sys_irq_enable((unsigned int)irq);
	} else {
		sys_irq_disable((unsigned int)irq);
	}
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
	metal_assert(0);
#endif
}

void sys_irq_enable(unsigned int vector)
{
	sys_irq_change(vector, 1);
}

void sys_irq_disable(unsigned int vector)
{
	sys_irq_change(vector, 0);
}

void metal_machine_cache_flush(void *addr, unsigned int len)
{
	if (!addr && !len) {
		Xil_DCacheFlush();
	} else {
		Xil_DCacheFlushRange((intptr_t)addr, len);
	}
}

void metal_machine_cache_invalidate(void *addr, unsigned int len)
{
	if (!addr && !len) {
		Xil_DCacheInvalidate();
	} else {
		Xil_DCacheInvalidateRange((intptr_t)addr, len);
	}
}

/**
 * @brief poll function until some event happens
 */
void metal_weak metal_generic_default_poll(void)
{
	metal_asm volatile("nop");
}

void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
			       size_t size, unsigned int flags)
{
	(void)pa;
	(void)size;
	(void)flags;
	return va;
}

void sys_irq_restore_enable(unsigned int flags)
{
	(void)flags;
	Xil_ExceptionEnable();
}


#define MSR_IE  0x2UL /* MicroBlaze status register interrupt enable mask */

#if (XPAR_MICROBLAZE_USE_MSR_INSTR != 0)
unsigned int sys_irq_save_disable(void)
{
	unsigned int state;

	metal_asm volatile("  mfs     %0, rmsr\n"
		     "  msrclr  r0, %1\n"
		     :  "=r"(state)
		     :  "i"(MSR_IE)
		     :  "memory");

	return state &= MSR_IE;
}

#else /* XPAR_MICROBLAZE_USE_MSR_INSTR == 0 */
unsigned int sys_irq_save_disable(void)
{
	unsigned int tmp, state;

	metal_asm volatile ("  mfs   %0, rmsr\n"
		      "  andi  %1, %0, %2\n"
		      "  mts   rmsr, %1\n"
		      :  "=r"(state), "=r"(tmp)
		      :  "i"(~MSR_IE)
		      :  "memory");

	return state &= MSR_IE;
}

#endif /* XPAR_MICROBLAZE_USE_MSR_INSTR */
