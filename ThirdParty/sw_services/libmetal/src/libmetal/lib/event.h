/*
 * Copyright (c) 2018, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	event.h
 * @brief	Event handling primitives for libmetal.
 */

#ifndef __METAL_EVENT__H__
#define __METAL_EVENT__H__

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup event Event Handling Interfaces
 *  @{ */

#include <metal/compiler.h>
#include <metal/list.h>

/** Event states */
#define METAL_EVENT_IDLE        0x0UL /**< Initial state of an event instance */
#define METAL_EVENT_ENABLED     0x1UL /**< Event is enabled, producer can post
					   the envent to event queue */
#define METAL_EVENT_PENDING     0x2UL /**< Event has happened, but it has not
					   been posted to the event queue yet */
#define METAL_EVENT_ENQUEUED    0x4UL /**< Event has been posted in the event
					   queue */
/** Event priority */
#define METAL_EVENT_PRIORITY_ANY 0xFFFFFFFFUL

/** Event handled status */
#define METAL_EVENT_NOT_HANDLED 0x0UL
#define METAL_EVENT_HANDLED     0x1UL

/** Event wait unlimited */
#define METAL_EVENT_WAIT_UNLIMITED ((unsigned long long)-1)

struct metal_event;

/**
 * @brief	type of metal event handler
 * @param[in]   event pointer to the event
 * @param[in]	arg argument to event handler
 * @return      event handled status
 */
typedef int (*metal_event_hd_func) (struct metal_event *event, void *arg);

/** Libmetal event handler structure */
struct metal_event_handler {
	metal_event_hd_func func; /**< pointer to event handler funcion */
	void *arg; /**< argement of the handler function */
};

/** Libmetal event structure */
struct metal_event {
	unsigned int state; /**< event state */
	unsigned int priority; /**< event priority. Reserved. It can be used
				    when posting the event. The default event
				    posting implementation from libmetal will
				    not consider the priority. But the user
				    can overwrite the event posting function to
				    make use of the priority property. */
	struct metal_event_handler hd; /**< event handler */
	struct metal_list node; /**< event node */
};

#define METAL_EVENT_INIT(p) \
	{ \
		.state = METAL_EVENT_IDLE, \
		.hd = { \
			.func = NULL, \
			.arg = NULL, \
		}, \
		.priority = p, \
		.node = { \
			.prev = NULL, \
			.next = NULL, \
		}, \
	}
#define METAL_EVENT_DEFINE(e, p) \
	struct metal_event e = METAL_EVENT_INIT(p)

/**
 * @brief     initialize event
 *
 * @param[in] event pointer to the event
 * @param[in] priority event priority
 */
static inline void metal_event_init(struct metal_event *event,
				    unsigned int priority)
{
	if (!event)
		return;
	event->state = METAL_EVENT_IDLE;
	event->priority = priority;
	event->hd.func = NULL;
	event->hd.arg = NULL;
	metal_list_init(&event->node);
}

/**
 * @brief     uninitialize event
 *
 * @param[in] event pointer to the event
 */
void metal_event_uninit(struct metal_event *event);

/**
 * @brief     post event
 *
 * Post event to indicate an event has happened.
 *
 * @param[in] event pointer to the event
 *
 * @return    0 if event is successfully posted, otherwise, negative
 *            value for failure.
 */
int metal_event_post(struct metal_event *event);

/**
 * @brief     pop event
 *
 * Remove the event so that it will not be scheduled.
 *
 * @param[in] event pointer to the event
 */
void metal_event_pop(struct metal_event *event);

/**
 * @brief      Register event handler
 *
 * @param[in]  event pointer to the event
 * @param[in]  func pointer to the event handler
 * @param[in]  arg arugment data to event handler
 * @return     0 for success, negative value for failure
 */
int metal_event_register_handler(struct metal_event *event,
				 metal_event_hd_func func,
				 void *arg);

/**
 * @brief         handle event
 *
 * call the event handler
 *
 * @param[in]     event pointer to the event
 * @return        METAL_EVENT_HANDLED if the event has been handled
 *                successfully, otherwise METAL_EVENT_NOT_HANDLED
 */
static inline int metal_event_handle(struct metal_event *event)
{
	if (event->hd.func)
		return event->hd.func(event, event->hd.arg);
	else
		return METAL_EVENT_NOT_HANDLED;
}

/**
 * @brief         enable event
 *
 * enable event handling to enable posting the event if the event
 * has happened.
 * If the event is pending, post it.
 *
 * @param[in]     event pointer to the event
 */
void metal_event_enable(struct metal_event *event);

/**
 * @brief         disable event
 *
 * disable event from posting it.
 *
 * @param[in]     event pointer to the event
 */
void metal_event_disable(struct metal_event *event);

/**
 * @brief        discard event
 *
 * Discard even. If the event has been enqueued, it will
 * pop the event from the event queue. If the event is pending,
 * it will discard the pending bit.
 *
 * @param[in]     event pointer to the event
 */
void metal_event_discard(struct metal_event *event);

#ifdef METAL_INTERNAL
/**
 * @brief     post event implementation which can be overwrittten
 *
 * libmetal provides default implementation to enqueue the event
 * to its default event queue, application overwrite this implementation.
 *
 * @param[in] event pointer to the event
 *
 * @return    0 if event is successfully posted, otherwise, negative
 *            value for failure.
 */
metal_weak int _metal_event_post(struct metal_event *event);

/**
 * @brief     pop event implementation which can be overwrittten
 *
 * libmetal provides default implementation to dequeue the event
 * from its default event queue, application overwrite this implementation.
 *
 * @param[in] event pointer to the event
 */
metal_weak void _metal_event_pop(struct metal_event *event);
#endif /* METAL_INTERNAL */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_EVENT__H__ */
