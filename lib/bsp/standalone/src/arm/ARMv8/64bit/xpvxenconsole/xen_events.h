/*
 * xen_events.h
 *
 *  Created on: Sep 14, 2016
 *      Author: jarvis
 */


/*
Copyright DornerWorks 2016
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:
1.	 Redistributions of source code must retain the above copyright notice, this list of conditions and the
following disclaimer.

THIS SOFTWARE IS PROVIDED BY DORNERWORKS FOR USE ON THE CONTRACTED PROJECT, AND ANY EXPRESS OR IMPLIED WARRANTY
IS LIMITED TO THIS USE. FOR ALL OTHER USES THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DORNERWORKS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**
* @file xen_events.h
* @brief Xen event channel interface.
*/

#ifndef _XEN_EVENTS_H_
#define _XEN_EVENTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"

#define PAGE_SHIFT 12 /**< Page size shift (log2 of page size). */
#define PAGE_SIZE (1<<PAGE_SHIFT) /**< Size of one memory page in bytes. */

#define EVENT_IRQ	31 /**< IRQ number used for Xen event channel notifications. */

#define XENMAPSPACE_shared_info  0 /**< Map the Xen shared info page. */
/* ` enum event_channel_op { // EVTCHNOP_* => struct evtchn_* */
#define EVTCHNOP_bind_interdomain 0 /**< Bind an event channel between domains. */
#define EVTCHNOP_bind_virq        1 /**< Bind an event channel to a virtual IRQ. */
#define EVTCHNOP_bind_pirq        2 /**< Bind an event channel to a physical IRQ. */
#define EVTCHNOP_close            3 /**< Close an event channel. */
#define EVTCHNOP_send             4 /**< Send an event on an event channel. */
#define EVTCHNOP_status           5 /**< Query status of an event channel. */
#define EVTCHNOP_alloc_unbound    6 /**< Allocate an unbound event channel. */
#define EVTCHNOP_bind_ipi         7 /**< Bind an event channel to an IPI. */
#define EVTCHNOP_bind_vcpu        8 /**< Bind an event channel to a VCPU. */
#define EVTCHNOP_unmask           9 /**< Unmask an event channel. */
#define EVTCHNOP_reset           10 /**< Reset all event channels. */
#define EVTCHNOP_init_control    11 /**< Initialise FIFO event channel control. */
#define EVTCHNOP_expand_array    12 /**< Expand the FIFO event channel array. */
#define EVTCHNOP_set_priority    13 /**< Set the priority of an event channel. */


/** @brief Send a notification on a Xen event channel.
 * @param[in] evtch_id Event channel port to notify.
 */
void notify_evtch(u32 evtch_id);
/** @brief Initialise the Xen event channel subsystem. */
void init_events(void);
/** @brief Register a handler function for a Xen event channel.
 * @param[in] evtch_id Event channel port to handle.
 * @param[in] fptr Pointer to the handler function.
 * @return 0 on success, negative on failure.
 */
int register_event_handler(u32 evtch_id, void (*fptr)(void));
/** @brief Unmask a Xen event channel.
 * @param[in] port Event channel port to unmask.
 */
void unmask_evtchn(u32 port);
/** @brief Handle a Xen event channel IRQ. */
void handle_event_irq(void* data);

#ifdef __cplusplus
}
#endif

#endif /* _XEN_EVENTS_H_ */
