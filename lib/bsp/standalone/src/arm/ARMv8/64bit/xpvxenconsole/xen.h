/******************************************************************************
 * xen.h
 *
 * Guest OS interface to Xen.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Copyright (c) 2004, K A Fraser
* * Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 */

/**
* @file xen.h
* @brief Guest OS interface to the Xen hypervisor.
*/

/*
Copyright DornerWorks 2016

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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __XEN_H_
/** @brief Include guard for xen.h. */
#define __XEN_H_

/*
 * HYPERCALLS
 */

/* `incontents 100 hcalls List of hypercalls
 * ` enum hypercall_num { // __HYPERVISOR_* => HYPERVISOR_*()
 */

#define __HYPERVISOR_set_trap_table        0 /**< Set processor trap/exception table. */
#define __HYPERVISOR_mmu_update            1 /**< Perform MMU page-table updates. */
#define __HYPERVISOR_set_gdt               2 /**< Set the Global Descriptor Table. */
#define __HYPERVISOR_stack_switch          3 /**< Switch the guest kernel stack. */
#define __HYPERVISOR_set_callbacks         4 /**< Set event/failsafe callback addresses. */
#define __HYPERVISOR_fpu_taskswitch        5 /**< Notify Xen of FPU task-switch. */
#define __HYPERVISOR_sched_op_compat       6 /**< Compatibility scheduler operations. */
#define __HYPERVISOR_platform_op           7 /**< Platform-specific operations. */
#define __HYPERVISOR_set_debugreg          8 /**< Set a hardware debug register. */
#define __HYPERVISOR_get_debugreg          9 /**< Get a hardware debug register. */
#define __HYPERVISOR_update_descriptor    10 /**< Update a segment descriptor. */
#define __HYPERVISOR_memory_op            12 /**< Memory management operations. */
#define __HYPERVISOR_multicall            13 /**< Execute multiple hypercalls atomically. */
#define __HYPERVISOR_update_va_mapping    14 /**< Update a virtual-to-physical mapping. */
#define __HYPERVISOR_set_timer_op         15 /**< Set a one-shot domain timer. */
#define __HYPERVISOR_event_channel_op_compat 16 /**< Compatibility event channel operations. */
#define __HYPERVISOR_xen_version          17 /**< Query Xen version information. */
#define __HYPERVISOR_console_io           18 /**< Console I/O operations. */
#define __HYPERVISOR_physdev_op_compat    19 /**< Compatibility physical device operations. */
#define __HYPERVISOR_grant_table_op       20 /**< Grant table operations. */
#define __HYPERVISOR_vm_assist            21 /**< Request VM assist features. */
#define __HYPERVISOR_update_va_mapping_otherdomain 22 /**< Update VA mapping in another domain. */
#define __HYPERVISOR_iret                 23 /**< Return from interrupt/exception (x86 only). */
#define __HYPERVISOR_vcpu_op              24 /**< VCPU management operations. */
#define __HYPERVISOR_set_segment_base     25 /**< Set segment base address registers (x86/64 only). */
#define __HYPERVISOR_mmuext_op            26 /**< Extended MMU operations. */
#define __HYPERVISOR_xsm_op               27 /**< Xen security module operations. */
#define __HYPERVISOR_nmi_op               28 /**< NMI handling operations. */
#define __HYPERVISOR_sched_op             29 /**< Scheduler operations. */
#define __HYPERVISOR_callback_op          30 /**< Callback registration operations. */
#define __HYPERVISOR_xenoprof_op          31 /**< Xenoprof profiling operations. */
#define __HYPERVISOR_event_channel_op     32 /**< Event channel operations. */
#define __HYPERVISOR_physdev_op           33 /**< Physical device operations. */
#define __HYPERVISOR_hvm_op               34 /**< Hardware Virtual Machine operations. */
#define __HYPERVISOR_sysctl               35 /**< System control operations. */
#define __HYPERVISOR_domctl               36 /**< Domain control operations. */
#define __HYPERVISOR_kexec_op             37 /**< Kernel exec operations. */
#define __HYPERVISOR_tmem_op              38 /**< Transcendent memory operations. */
#define __HYPERVISOR_xc_reserved_op       39 /**< Reserved Xen client operations. */

/* Architecture-specific hypercall definitions. */
#define __HYPERVISOR_arch_0               48 /**< Architecture-specific hypercall 0. */
#define __HYPERVISOR_arch_1               49 /**< Architecture-specific hypercall 1. */
#define __HYPERVISOR_arch_2               50 /**< Architecture-specific hypercall 2. */
#define __HYPERVISOR_arch_3               51 /**< Architecture-specific hypercall 3. */
#define __HYPERVISOR_arch_4               52 /**< Architecture-specific hypercall 4. */
#define __HYPERVISOR_arch_5               53 /**< Architecture-specific hypercall 5. */
#define __HYPERVISOR_arch_6               54 /**< Architecture-specific hypercall 6. */
#define __HYPERVISOR_arch_7               55 /**< Architecture-specific hypercall 7. */

/* ` } */

/*
 * Commands to HYPERVISOR_console_io().
 */
#define CONSOLEIO_write         0 /**< Write data to the Xen console. */
#define CONSOLEIO_read          1 /**< Read data from the Xen console. */

/*
 * Commands to HYPERVISOR_hvm_op
 */
#define HVMOP_set_param           0 /**< Set an HVM domain parameter. */
#define HVMOP_get_param           1 /**< Get an HVM domain parameter. */

/* DOMID_SELF is used in certain contexts to refer to oneself. */
#define DOMID_SELF (0x7FF0U) /**< Pseudo-domain ID referring to the calling domain. */


#define XENMEM_add_to_physmap      7 /**< Add a page to the physical memory map. */


/* Console debug shared memory ring and event channel */
#define HVM_PARAM_CONSOLE_PFN    17 /**< HVM parameter: console page frame number. */
#define HVM_PARAM_CONSOLE_EVTCHN 18 /**< HVM parameter: console event channel. */


#endif /* __XEN_PUBLIC_XEN_H__ */

#ifdef __cplusplus
}
#endif
