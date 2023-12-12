/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	generic/xlnx/sys.h
 * @brief	generic xlnx system primitives for libmetal.
 */

#ifndef __METAL_GENERIC_SYS__H__
#error "Include metal/sys.h instead of metal/generic/@PROJECT_MACHINE@/sys.h"
#endif

#ifndef __MICROBLAZE__
#include "xscugic.h"
#endif

#include <metal/assert.h>
#include <metal/io.h>
#include <metal/sys.h>
#include <stdint.h>
#include <metal/compiler.h>
#include <metal/utilities.h>

#include <xil_cache.h>
#include <xil_exception.h>
#include <xparameters.h>

#ifndef __METAL_GENERIC_XLNX_SYS__H__
#define __METAL_GENERIC_XLNX_SYS__H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MICROBLAZE__
#define XLNX_MAXIRQS XSCUGIC_MAX_NUM_INTR_INPUTS
#endif

#if defined(SDT) && !defined(__MICROBLAZE__)
#define XPAR_SCUGIC_0_DIST_BASEADDR XPAR_SCUGIC_DIST_BASEADDR
#endif

/**
 * @brief	metal_xlnx_irq_isr
 *
 * Xilinx interrupt ISR can be registered to the Xilinx embeddedsw
 * IRQ controller driver.
 *
 * @param[in] arg input argument, interrupt vector id.
 */
void metal_xlnx_irq_isr(void *arg);

/**
 * @brief	metal_xlnx_irq_int
 *
 * Xilinx interrupt controller initialization. It will initialize
 * the metal Xilinx IRQ controller data structure.
 *
 * @return 0 for success, or negative value for failure
 */
int metal_xlnx_irq_init(void);

/* Microblaze defines these routines */
#ifdef __MICROBLAZE__
void metal_weak sys_irq_enable(unsigned int vector);

void metal_weak sys_irq_disable(unsigned int vector);
#else
static inline void sys_irq_enable(unsigned int vector)
{
	XScuGic_EnableIntr(XPAR_SCUGIC_0_DIST_BASEADDR, vector);
}

static inline void sys_irq_disable(unsigned int vector)
{
	XScuGic_DisableIntr(XPAR_SCUGIC_0_DIST_BASEADDR, vector);
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* __METAL_GENERIC_XLNX_SYS__H__ */
