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
#include <metal/event.h>
#include <metal/utilities.h>

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

/** Metal IRQ event */
struct metal_irq_event {
	struct metal_event event; /**< event struct */
	int irq; /**< interrupt ID */
	struct metal_device *dev; /**< pointer to metal device */
};

/** Metal IRQ Controller. */
struct metal_irq_controller {
	int (*enable_irq)(struct metal_irq_controller *cntr,
			  unsigned int vector);
	void (*disable_irq)(struct metal_irq_controller *cntr,
			    unsigned int vector);
	int (*map_irq_event)(struct metal_irq_controller *cntr,
			     int irq, struct metal_irq_event *irqe);
	struct metal_irq_event *(*get_event)(struct metal_irq_controller *cntr,
					     int irq);
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
 * @brief     Map interrupt event
 *
 * Map metal interrupt event to interrupt
 *
 * @param[in] irq interrupt id
 * @param[in] irqe irq event
 *
 * @return 0 for success, otherwise, negative value for failure
 */
void metal_irq_map_event(int irq, struct metal_irq_event *irqe);

/**
 * @brief     Get interrupt event
 *
 * Get metal interrupt event from interrupt
 *
 * @param[in] irq interrupt id
 *
 * @return pointer to metal event mapped to the interrupt
 */
struct metal_event *metal_irq_get_event(int irq);

/**
 * @brief      Initialize metal interrupt event.
 *
 * @param[in]  irqe       interrupt event
 * @param[in]  irq        interrupt id
 * @param[in]  dev        metal device this irq belongs to (can be NULL).
 */
static inline void metal_irq_init_event(struct metal_irq_event *irqe,
					int irq, struct metal_device *dev)
{
	metal_assert(irqe != NULL);
	irqe->irq = irq;
	irqe->dev = dev;
}

/**
 * @brief      get interrupt ID from event
 *
 * get interrupt id from metal event
 *
 * @param[in]  event pointer to metal event
 *
 * @return interrupt ID
 */
static inline int metal_irq_get_vector_from_event(struct metal_event *event)
{
	struct metal_irq_event *irqe;

	metal_assert(event != NULL);
	irqe = metal_container_of(event, struct metal_irq_event, event);
	return irqe->irq;
}

/**
 * @brief      register interrupt
 *
 * @param[in]  irq interrupt id
 * @param[in]  hd interrupt event handler function
 * @param[in]  arg pointer to handler argument
 *
 * @return 0 for success, and negative value for failure
 */
int metal_irq_register(int irq, metal_event_hd_func hd,
		       void *arg);

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
