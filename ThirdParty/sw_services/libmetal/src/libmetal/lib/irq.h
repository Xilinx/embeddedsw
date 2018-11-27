/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	irq.h
 * @brief	Interrupt handling primitives for libmetal.
 */

#ifndef __METAL_IRQ__H__
#define __METAL_IRQ__H__

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup irq Interrupt Handling Interfaces
 *  @{ */

#include <stdlib.h>

/** IRQ handled status */
#define METAL_IRQ_NOT_HANDLED 0
#define METAL_IRQ_HANDLED     1

/**
 * @brief	type of interrupt handler
 * @param[in]   irq interrupt id
 * @param[in]	priv private data
 * @return      irq handled status
 */
typedef int (*metal_irq_handler) (int irq, void *priv);

struct metal_device;

/** Metal IRQ Controller. */
struct metal_irq_controller {
	int (*enable_irq)(struct metal_irq_controller *cntr,
			  unsigned int vector);
	void (*disable_irq)(struct metal_irq_controller *cntr,
			    unsigned int vector);
	void  *arg;
};

/**
 * @brief     Set interrupt controller
 *
 * Set metal global interrupt controller
 *
 * @param[in] controller IRQ controller
 */
void metal_irq_set_controller(struct metal_irq_controller *cntr);

/**
 * @brief      Register interrupt handler for driver ID/device.
 *
 * @param[in]  irq         interrupt id
 * @param[in]  irq_handler interrupt handler, if it is NULL, it will
 *                         unregister the irq handler.
 * @param[in]  dev         metal device this irq belongs to (can be NULL).
 * @param[in]  drv_id      driver id is a unique interrupt handler identifier.
 *                         It can also be used for driver data.
 * @return     0 for success, non-zero on failure
 */
int metal_irq_register(int irq,
		       metal_irq_handler irq_handler,
		       struct metal_device *dev,
		       void *drv_id);

/**
 * @brief      disable interrupts
 * @return     interrupts state
 */
unsigned int metal_irq_save_disable(void);

/**
 * @brief      restore interrupts to their previous state
 * @param[in]  flags previous interrupts state
 */
void metal_irq_restore_enable(unsigned int flags);

/**
 * @brief	metal_irq_enable
 *
 * Enables the given interrupt
 *
 * @param vector   - interrupt vector number
 */
void metal_irq_enable(unsigned int vector);

/**
 * @brief	metal_irq_disable
 *
 * Disables the given interrupt
 *
 * @param vector   - interrupt vector number
 */
void metal_irq_disable(unsigned int vector);

#include <metal/system/@PROJECT_SYSTEM@/irq.h>

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_IRQ__H__ */
