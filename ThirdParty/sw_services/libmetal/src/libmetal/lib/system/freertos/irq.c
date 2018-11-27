/*
 * Copyright (c) 2016 - 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	freertos/irq.c
 * @brief	FreeRTOS libmetal irq definitions.
 */

#include <errno.h>
#include <metal/irq.h>
#include <metal/sys.h>
#include <metal/log.h>
#include <metal/mutex.h>
#include <metal/list.h>
#include <metal/utilities.h>
#include <metal/alloc.h>

/** IRQ descriptor structure */
struct metal_irq_desc {
	int irq;                  /**< interrupt number */
	metal_irq_handler hd;     /**< irq handler */
	void *drv_id;             /**< id to identify the driver
	                               of the irq handler */
	struct metal_device *dev; /**< device identifier */
	struct metal_list node;   /**< node on irqs list */
};

/** IRQ state structure */
struct metal_irqs_state {
	struct metal_list irqs;   /**< interrupt descriptors */
	metal_mutex_t irq_lock;   /**< access lock */
};

/** IRQ state structure */
struct metal_irqs_state {
	struct metal_list irqs;    /**< interrupt descriptors */
	metal_mutex_t irq_lock;    /**< access lock */
};

static struct metal_irqs_state _irqs = {
	.irqs = METAL_INIT_LIST(_irqs.irqs),
	.irq_lock = METAL_MUTEX_INIT(_irqs.irq_lock),
};

int metal_irq_register(int irq,
                       metal_irq_handler hd,
                       struct metal_device *dev,
                       void *drv_id)
{
	struct metal_irq_desc *irq_p = NULL;
	struct metal_list *node;
	unsigned int irq_flags_save;

	if (irq < 0) {
		metal_log(METAL_LOG_ERROR,
			  "%s: irq %d need to be a positive number\n",
		          __func__, irq);
		return -EINVAL;
	}

	/* Search for irq in list */
	metal_mutex_acquire(&_irqs.irq_lock);
	metal_list_for_each(&_irqs.irqs, node) {
		irq_p = metal_container_of(node, struct metal_irq_desc, node);

		if (irq_p->irq == irq) {
			/* Check if handler has already registered */
			if (irq_p->hd != NULL && irq_p->hd != hd) {
				metal_log(METAL_LOG_ERROR,
					  "%s: irq %d already registered."
					  "Will not register again.\n",
					  __func__, irq);
				metal_mutex_release(&_irqs.irq_lock);
				return -EINVAL;
			} else {
				metal_mutex_release(&_irqs.irq_lock);
				return 0;
			}
		}
	}

	/* Either need to add handler to an existing list or to a new one */
	irq_p = metal_allocate_memory(sizeof(*irq_p));
	if (irq_p == NULL) {
		metal_log(METAL_LOG_ERROR,
		          "%s: irq %d cannot allocate mem for drv_id %d.\n",
		          __func__, irq, drv_id);
		metal_mutex_release(&_irqs.irq_lock);
		return -ENOMEM;
	}
	irq_p->hd = hd;
	irq_p->drv_id = drv_id;
	irq_p->dev = dev;
	irq_p->irq = irq;

	irq_flags_save = metal_irq_save_disable();
	metal_list_add_tail(&_irqs.irqs, &irq_p->node);
	metal_irq_restore_enable(irq_flags_save);
	metal_mutex_release(&_irqs.irq_lock);

	metal_log(METAL_LOG_DEBUG, "%s: success, irq %d add drv_id %p \n",
	          __func__, irq, drv_id);
	return 0;
}

int metal_irq_unregister(int irq,
                         metal_irq_handler hd,
                         struct metal_device *dev,
                         void *drv_id)
{
	struct metal_irq_desc *irq_p;
	struct metal_list *node;

	(void)hd;
	(void)dev;
	(void)drv_id;
	if (irq < 0) {
		metal_log(METAL_LOG_ERROR, "%s: irq %d need to be a positive number\n",
		          __func__, irq);
		return -EINVAL;
	}

	/* Search for irq in list */
	metal_mutex_acquire(&_irqs.irq_lock);
	metal_list_for_each(&_irqs.irqs, node) {

		irq_p = metal_container_of(node, struct metal_irq_desc, node);

		if (irq_p->irq == irq) {
			unsigned int irq_flags_save;

			irq_flags_save=metal_irq_save_disable();
			metal_list_del(node);
			metal_irq_restore_enable(irq_flags_save);
			metal_free_memory(irq_p);
			metal_mutex_release(&_irqs.irq_lock);
			return 0;
		}
	}
	metal_log(METAL_LOG_DEBUG, "%s: No matching IRQ entry\n", __func__);

	metal_mutex_release(&_irqs.irq_lock);
	return -ENOENT;
}

unsigned int metal_irq_save_disable(void)
{
	return sys_irq_save_disable();
}

void metal_irq_restore_enable(unsigned int flags)
{
	sys_irq_restore_enable(flags);
}

/**
 * @brief default handler
 */
void metal_irq_isr(unsigned int vector)
{
	struct metal_list *node;
	struct metal_irq_desc *irq_p;

	metal_list_for_each(&_irqs.irqs, node) {
		irq_p = metal_container_of(node, struct metal_irq_desc, node);

		if ((unsigned int)irq_p->irq == vector) {
			if (irq_p->hd)
				(irq_p->hd)(vector, irq_p->drv_id);
			return;
		}
	}
}
