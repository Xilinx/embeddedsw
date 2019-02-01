/*
 * Copyright (c) 2018, Pinecone Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	nuttx/irq.c
 * @brief	NuttX libmetal irq definitions.
 */

#include <errno.h>
#include <metal/irq.h>
#include <metal/alloc.h>
#include <nuttx/irq.h>

/** IRQ handler descriptor structure */
struct metal_irq_hddesc {
	metal_irq_handler hd;     /**< irq handler */
	void *drv_id;             /**< id to identify the driver
	                               of the irq handler */
};

static int metal_irq_isr(int irq, void *context, void *arg);

int metal_irq_register(int irq,
		       metal_irq_handler hd,
		       struct metal_device *dev,
		       void *drv_id)
{
	struct metal_irq_hddesc *desc;
	int ret;

	if ((drv_id == NULL) || (hd == NULL))
		return -EINVAL;

	desc = metal_allocate_memory(sizeof(struct metal_irq_hddesc));
	if (desc == NULL)
		return -ENOMEM;

	desc->hd = hd;
	desc->drv_id = drv_id;

	ret = irq_attach(irq, metal_irq_isr, desc);
	if (ret < 0)
		metal_free_memory(desc);

	return ret;
}

int metal_irq_unregister(int irq,
			 metal_irq_handler hd,
			 struct metal_device *dev,
			 void *drv_id)
{
	unsigned int irq_flags_save;

	if (irq < 0)
		return -EINVAL;

	irq_flags_save = metal_irq_save_disable();
	/* context == drv_id mean unregister */
	irq_dispatch(irq, drv_id); /* fake a irq request */
	metal_irq_restore_enable(irq_flags_save);

	return 0;
}

unsigned int metal_irq_save_disable(void)
{
	return enter_critical_section();
}

void metal_irq_restore_enable(unsigned int flags)
{
	leave_critical_section(flags);
}

void metal_irq_enable(unsigned int vector)
{
	up_enable_irq(vector);
}

void metal_irq_disable(unsigned int vector)
{
	up_disable_irq(vector);
}

/**
  * @brief       IRQ handler
 */
static int metal_irq_isr(int irq, void *context, void *arg)
{
	struct metal_irq_hddesc *desc = arg;

	if (context != desc->drv_id)
		return desc->hd(irq, desc->drv_id);

	/* context == drv_id mean unregister */
	irqchain_detach(irq, metal_irq_isr, arg);
	sched_kfree(arg);
	return 0;
}
