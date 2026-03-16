/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	freertos/xlnx/sys.h
 * @brief	freertos Xilinx common system primitives for libmetal.
 */

#ifndef __METAL_FREERTOS_SYS__H__
#error "Include metal/sys.h instead of metal/freertos/@PROJECT_MACHINE@/sys.h"
#endif

#ifndef __METAL_FREERTOS_XLNX_SYS__H__
#define __METAL_FREERTOS_XLNX_SYS__H__

#include "xscugic.h"
#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SDT) && defined(PLATFORM_ZYNQ)
#define XPAR_SCUGIC_0_DIST_BASEADDR XPAR_SCUGIC_DIST_BASEADDR
#endif

#ifndef XLNX_MAXIRQS
#define XLNX_MAXIRQS XSCUGIC_MAX_NUM_INTR_INPUTS
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

static inline void sys_irq_enable(unsigned int vector)
{
#ifdef PLATFORM_ZYNQ
	XScuGic_EnableIntr(XPAR_SCUGIC_0_DIST_BASEADDR, vector);
#else
	vPortEnableInterrupt(vector);
#endif
}

static inline void sys_irq_disable(unsigned int vector)
{
#ifdef PLATFORM_ZYNQ
	XScuGic_DisableIntr(XPAR_SCUGIC_0_DIST_BASEADDR, vector);
#else
	vPortDisableInterrupt(vector);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_FREERTOS_XLNX_SYS__H__ */
