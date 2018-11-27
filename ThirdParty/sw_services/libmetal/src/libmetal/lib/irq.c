/*
 * Copyright (c) 2018, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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
int metal_irq_register(int irq,
		       metal_irq_handler irq_handler,
		       struct metal_device *dev,
		       void *drv_id)
{
	metal_assert(metal_irq_controller != NULL);
	return metal_irq_controller->register_irq(metal_irq_controller,
						  irq, irq_handler, dev,
						  drv_id);
}
