/*
 * Copyright (c) 2018, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <metal/event.h>
#include <metal/irq.h>
#include <metal/list.h>

static METAL_DECLARE_LIST(metal_default_event_queue);

void metal_event_uninit(struct metal_event *event)
{
	unsigned int flags;

	if (!event)
		return;
	flags = metal_irq_save_disable();
	_metal_event_pop(event);
	event->state = METAL_EVENT_IDLE;
	event->hd.func = NULL;
	event->hd.arg = NULL;
	metal_irq_restore_enable(flags);
}

int metal_event_post(struct metal_event *event)
{
	int ret;
	unsigned int flags;

	if (!event)
		return -EINVAL;
	flags = metal_irq_save_disable();
	if ((event->state & METAL_EVENT_ENABLED) &&
	    (event->state & METAL_EVENT_ENQUEUED) == 0) {
		ret = _metal_event_post(event);
		if (ret == 0)
			event->state |= METAL_EVENT_ENQUEUED;
	} else if ((event->state & METAL_EVENT_ENABLED) == 0) {
		ret = 0;
		event->state |= METAL_EVENT_PENDING;
	} else {
		ret = 0;
	}
	metal_irq_restore_enable(flags);
	return ret;
}

void metal_event_pop(struct metal_event *event)
{
	unsigned int flags;

	if (!event)
		return;
	flags = metal_irq_save_disable();
	_metal_event_pop(event);
	event->state &= ~METAL_EVENT_ENQUEUED;
	metal_irq_restore_enable(flags);
}

int metal_event_register_handler(struct metal_event *event,
				 metal_event_hd_func func,
				 void *arg)
{
	unsigned int flags;

	if (!event)
		return -EINVAL;
	flags = metal_irq_save_disable();
	event->hd.func = func;
	event->hd.arg = arg;
	metal_irq_restore_enable(flags);
	return 0;
}

void metal_event_enable(struct metal_event *event)
{
	unsigned int flags;

	if (!event)
		return;
	flags = metal_irq_save_disable();
	event->state |= METAL_EVENT_ENABLED;
	if (event->state & METAL_EVENT_PENDING)
		event->state &= ~METAL_EVENT_PENDING;
	metal_irq_restore_enable(flags);
	metal_event_post(event);
}

void metal_event_disable(struct metal_event *event)
{
	unsigned int flags;

	if (!event)
		return;
	flags = metal_irq_save_disable();
	if ((event->state & METAL_EVENT_ENQUEUED) != 0) {
		_metal_event_pop(event);
		event->state &= ~METAL_EVENT_ENQUEUED;
		event->state |= METAL_EVENT_PENDING;
	}
	event->state &= ~METAL_EVENT_ENABLED;
	metal_irq_restore_enable(flags);
}

void metal_event_discard(struct metal_event *event)
{
	unsigned int flags;

	if (!event)
		return;
	flags = metal_irq_save_disable();
	if ((event->state & METAL_EVENT_ENQUEUED) != 0) {
		_metal_event_pop(event);
		event->state &= ~METAL_EVENT_ENQUEUED;
	} else if ((event->state & METAL_EVENT_PENDING) != 0) {
		event->state &= ~METAL_EVENT_PENDING;
	}
	metal_irq_restore_enable(flags);

}

metal_weak int _metal_event_post(struct metal_event *event)
{
	metal_list_add_tail(&metal_default_event_queue, &event->node);
	return 0;
}

metal_weak void _metal_event_pop(struct metal_event *event)
{
	metal_list_del(&event->node);
}
