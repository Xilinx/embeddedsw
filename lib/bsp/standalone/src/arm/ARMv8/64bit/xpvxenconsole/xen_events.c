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
* @file xen_events.c
* @brief Xen event channel implementation.
*/


#include "xen.h"
#include "hypercall.h"
#include "arm64_ops.h"
#include "xen_events.h"
#include "xen_console.h"

#define VA_TO_GUEST_PAGE(x) 	((u64)x >> PAGE_SHIFT)		/**< Convert a virtual address to a guest page frame number (direct VA:PA mapping). */

#define active_evtchns(cpu,sh,idx)              \
    ((sh)->evtchn_pending[idx] &                \
     ~(sh)->evtchn_mask[idx]) /**< Compute the set of active (pending and unmasked) event channels for the given index. */

/** @brief Structure for sharing a page with Xen via add_to_physmap hypercall. */
struct xen_add_to_physmap
{
    u16 domid;           /**< Domain to change the mapping for. */
    u16 size;            /**< Number of pages for gmfn_range. */
    unsigned int space;  /**< Physical map space type (enum phys_map_space). */
    u64 idx;             /**< Index into the space being mapped. */
    u64 gpfn;            /**< GPFN in domid where the source mapping page appears. */
};
typedef struct xen_add_to_physmap xen_add_to_physmap_t; /**< Xen hypercall structure for mapping a guest physical frame. */

/** @brief Per-vCPU information shared between guest and Xen hypervisor. */
struct vcpu_info {
    u8 evtchn_upcall_pending;  /**< Event channel upcall pending flag. */
    u8 pad0;                   /**< Padding byte. */
    u64 evtchn_pending_sel;    /**< Bitmask of pending event channels. */
    u8 pad[30];                /**< Padding to align to 64 bytes. */
}; /**< 64 bytes (x86) */
typedef struct vcpu_info vcpu_info_t; /**< Per-vCPU information shared between guest and hypervisor. */

/** @brief Shared info page between guest and Xen hypervisor. */
struct shared_info
{
    vcpu_info_t vcpu_info[1];                  /**< Per-vCPU info array. */
    u64 evtchn_pending[sizeof(u64) * 8];       /**< Pending event channel bitmap. */
    u64 evtchn_mask[sizeof(u64) * 8];          /**< Masked event channel bitmap. */
};
typedef struct shared_info shared_info_t; /**< Shared info page between guest and Xen hypervisor. */

/** @brief Event handler entry mapping an event channel to its handler. */
struct event_handler
{
    u32 evtch_id;               /**< Event channel identifier. */
    void (*handler_fptr)(void); /**< Pointer to the event handler function. */
};
typedef struct event_handler event_handler_t; /**< Event handler entry mapping an event channel to its handler. */

static event_handler_t event_handler_table[1] = {{0}};
static __attribute__((aligned(0x1000))) u8 shared_info_page[1<<PAGE_SHIFT];
static struct shared_info* shared_info = NULL;


// This function is used to register a callback function for a particular event channel
/**
 * @brief Register a handler function for a Xen event channel.
 * @param[in] evtch_id Event channel port to handle.
 * @param[in] fptr Pointer to the handler function.
 * @return 0 on success, negative on failure.
 */
int register_event_handler(u32 evtch_id, void (*fptr)(void))
{

	if(0 != event_handler_table[0].evtch_id && evtch_id != event_handler_table[0].evtch_id)
	{
		return -1;
	}

	event_handler_table[0].evtch_id = evtch_id;
	event_handler_table[0].handler_fptr = fptr;

	return 0;
}

// This function is used by the guest to notify Xen that an event has occurred
/**
 * @brief Send a notification on a Xen event channel.
 * @param[in] evtch_id Event channel port to notify.
 */
void notify_evtch(u32 evtch_id)
{
    u32 op = (u32)evtch_id;
    HYPERVISOR_event_channel_op(EVTCHNOP_send, &op);
}

// This function makes it so an event can cause an interrupt to the guest
/**
 * @brief Unmask a Xen event channel.
 * @param[in] port Event channel port to unmask.
 */
void unmask_evtchn(uint32_t port)
{
	struct shared_info* s = shared_info;
    vcpu_info_t *vcpu_info = &s->vcpu_info[0];

    synch_clear_bit(port, &s->evtchn_mask[0]);

    /*
     * The following is basically the equivalent of 'hw_resend_irq'. Just like
     * a real IO-APIC we 'lose the interrupt edge' if the channel is masked.
     */
    if (  synch_test_bit        (port,    &s->evtchn_pending[0]) &&
         !synch_test_and_set_bit(port / (sizeof(unsigned long) * 8),
              &vcpu_info->evtchn_pending_sel) )
    {
        vcpu_info->evtchn_upcall_pending = 1;
#ifdef XEN_HAVE_PV_UPCALL_MASK
        if ( !vcpu_info->evtchn_upcall_mask )
#endif
       //     force_evtchn_callback();				//todo, figure if this is needed
    }
}

// Clears the event flag so Xen can raise it again later
/**
 * @brief Clear a pending Xen event channel notification.
 * @param[in] port Event channel port to clear.
 */
static inline void clear_evtchn(uint32_t port)
{
    shared_info_t *s = shared_info;
    synch_clear_bit(port, &s->evtchn_pending[0]);
}

// This is where we would call the handler for the specific event, but for now only supporting 1 event
/**
 * @brief Dispatch and handle a pending event on the given channel port.
 * @param[in] port Event channel port to dispatch.
 * @return 0 on success.
 */
static int do_event(u32 port)
{
    clear_evtchn(port);

    /**< call the handler */
    if(port == event_handler_table[0].evtch_id)
    {
	event_handler_table[0].handler_fptr();
    }

    return 1;
}

// The IRQ handler for the interrupt that Xen uses for events (IRQ 31).
/**
 * @brief Top-level IRQ handler for Xen event channels.
 * @param[in] data Opaque pointer to event data (unused).
 */
void handle_event_irq(void* data)
{
	unsigned long  l1, l2, l1i, l2i;
	unsigned int   port;
	int            cpu = 0;
	shared_info_t *s = shared_info;
	vcpu_info_t   *vcpu_info = &s->vcpu_info[cpu];

	vcpu_info->evtchn_upcall_pending = 0;
	wmb();

	l1 = xchg(&vcpu_info->evtchn_pending_sel, 0);
	while ( l1 != 0 )
	{
		l1i = __ffs(l1);
		l1 &= ~(1UL << l1i);
		while ( (l2 = active_evtchns(cpu, s, l1i)) != 0 )
		{
			l2i = __ffs(l2);
			l2 &= ~(1UL << l2i);

			port = (l1i * (sizeof(unsigned long) * 8)) + l2i;
			do_event(port);

		}
	}
}

// Initialize the guest's event framework
/** @brief Initialise the Xen event channel subsystem. */
void init_events(void)
{
    xen_add_to_physmap_t xatp;

    /**< Map shared_info page */
    xatp.domid = DOMID_SELF;
    xatp.idx = 0;
    xatp.space = XENMAPSPACE_shared_info;
    xatp.gpfn = VA_TO_GUEST_PAGE(shared_info_page);

    if(0 != HYPERVISOR_memory_op(XENMEM_add_to_physmap, &xatp))
    {
	printk("ERROR setting up shared memory!\r\n");
    }

    shared_info = (struct shared_info *)shared_info_page;
}
