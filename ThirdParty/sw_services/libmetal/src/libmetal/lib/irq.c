
/*
 * Copyright (c) 2018, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <metal/irq.h>

static struct metal_irq_controller *metal_irq_controller = NULL;

void metal_irq_set_controller(struct metal_irq_controller *cntr)
{
	metal_irq_controller = cntr;
}
