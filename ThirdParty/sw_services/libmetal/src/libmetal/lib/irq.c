/*
 * Copyright (c) 2018, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <metal/assert.h>
#include <metal/irq.h>

static struct metal_irq_controller *metal_irq_controller = NULL;

void metal_irq_set_controller(struct metal_irq_controller *cntr)
{
	metal_irq_controller = cntr;
}

void metal_irq_enable(unsigned int vector)
{
	metal_assert(metal_irq_controller != NULL);
	metal_irq_controller->enable_irq(metal_irq_controller, vector);
}

void metal_irq_disable(unsigned int vector)
{
	metal_assert(metal_irq_controller != NULL);
	metal_irq_controller->disable_irq(metal_irq_controller, vector);
}

void metal_irq_map_event(int irq, struct metal_irq_event *irqe)
{
	metal_assert(metal_irq_controller != NULL);
	metal_assert(irqe != NULL);
	metal_assert(metal_irq_controller->map_irq_event != NULL);
	metal_irq_controller->map_irq_event(metal_irq_controller,
					    irq, irqe);
}

struct metal_event *metal_irq_get_event(int irq)
{
	struct metal_irq_event *irqe;

	metal_assert(metal_irq_controller != NULL);
	metal_assert(metal_irq_controller->get_event != NULL);
	irqe =  metal_irq_controller->get_event(metal_irq_controller,
					        irq);
	if (irqe != NULL)
		return &irqe->event;
	else
		return NULL;
}

int metal_irq_register(int irq, metal_event_hd_func hd,
		       void *arg)
{
	struct metal_event *event;

	metal_assert(metal_irq_controller != NULL);
	event = metal_irq_get_event(irq);
	if (event == NULL)
		return -EINVAL;
	return metal_event_register_handler(event, hd, arg);
}
