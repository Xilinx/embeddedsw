/******************************************************************************
*
* Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
//----------------------------------------------------------------------------------------------------//
//! @file ppc-hw.c
//! PPC Hardware initialization
//----------------------------------------------------------------------------------------------------//
#include <os_config.h>
#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/ktypes.h>
#include <xparameters.h>
#include <sys/process.h>
#include <xil_exception.h>
#include <xtime_l.h>
#include <xpseudo_asm.h>
#include <sys/syscall.h>
#ifdef CONFIG_INTC
#include <xintc.h>
#endif
#include <sys/decls.h>
#ifndef PPC_CPU_440
#include <xreg405.h>
#else
#include <xreg440.h>
#endif
#include <stdio.h>

//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------//
#ifdef CONFIG_INTC
XIntc sys_intc;
#endif
struct t_syscall_data system_call_data;
// this pointer points to data shared between user and system
struct t_syscall_data *system_call_data_ptr = &system_call_data;
struct t_syscall_data **system_call_data_ptr_ptr = &system_call_data_ptr;
unsigned int pit_timer_interval = 1; // default interval
extern char did_resched;
extern char timer_need_refresh;
extern unsigned char sched_partial_tick;

//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//
void hw_init(void);
void int_system_init(void);
unsigned int pit_get_value(void);
void pit_initialize(unsigned int interval);
void pit_set_interval( unsigned int interval);
void pit_reset(void);
void pit_continue(void);
void pit_disable(void);
void setup_initial_context (process_struct *pcb, pid_t parent, unsigned int start_addr, unsigned int stackaddr, unsigned int stacksize);
void init_idle_task (void);

extern void timer_int_handler(int) ;
extern void system_call_handler(void *baseaddr);

#ifndef PPC_CPU_440

#define PIT_INT                 XIL_EXCEPTION_ID_PIT_INT
#define PIT_SET_INTERVAL        XTime_PITSetInterval
#define PIT_ENABLE_AUTO_RELOAD  XTime_PITEnableAutoReload
#define PIT_DISABLE_INTR        XTime_PITDisableInterrupt
#define PIT_ENABLE_INTR         XTime_PITEnableInterrupt
#define PIT_CLEAR_STATUS()      XTime_TSRClearStatusBits(XREG_TSR_PIT_INTERRUPT_STATUS)

#else

#define PIT_INT                 XIL_EXCEPTION_ID_DEC_INT
#define PIT_SET_INTERVAL        XTime_DECSetInterval
#define PIT_ENABLE_AUTO_RELOAD  XTime_DECEnableAutoReload
#define PIT_DISABLE_INTR        XTime_DECDisableInterrupt
#define PIT_ENABLE_INTR         XTime_DECEnableInterrupt
#define PIT_CLEAR_STATUS()      XTime_TSRClearStatusBits(XREG_TSR_DEC_INTERRUPT_STATUS)

#endif
//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//

//----------------------------------------------------------------------------------------------------//
//  @func - hw_init
//! @desc
//!   PPC405 hardware specific initialization
//!   - Initialize PPC exception handling mechanism
//!   - If an interrupt controller is present, register interrupt controller handler
//!     as the external interrupt handler.
//!   - Register PPC timer interrupt handler as the handler for the PPC PIT interrupt
//! @param
//!   - none
//! @return
//!   - nothing
//! @note
//!   - none
//----------------------------------------------------------------------------------------------------//
void hw_init(void)
{
    Xil_ExceptionInit();                                                                                // Initialize exception handling
    Xil_ExceptionDisable();

#ifdef CONFIG_INTC
    int_system_init();
#endif

    Xil_ExceptionRegisterHandler(PIT_INT, (Xil_ExceptionHandler)timer_int_handler, (void *)0);          // Register PIT interrupt handler
    pit_initialize (SYSTMR_INTERVAL);                                                                   // use SYSTMR_INTERVAL as configured in MSS
}

#ifdef CONFIG_INTC
//----------------------------------------------------------------------------------------------------//
//  @func - int_system_init
//! @desc
//!   Initialize PPC405 interrupt handling system in the prescence of an interrupt controller.
//! @param
//!   - none
//! @return
//!   - nothing
//! @note
//!   - none
//----------------------------------------------------------------------------------------------------//
void int_system_init (void)
{
    XStatus status;

    status = XIntc_Initialize (&sys_intc, SYSINTC_DEVICE_ID);
    if (status != XST_SUCCESS) {
	DBG_PRINT ("XMK: init_int_system: XIntc_Initialize failed.\r\n");
	return;
    }

    XIntc_SetIntrSvcOption(SYSINTC_BASEADDR, XIN_SVC_ALL_ISRS_OPTION);  // IntC ISR should service all interrupts
    Xil_ExceptionRegisterHandler (XIL_EXCEPTION_ID_NON_CRITICAL_INT,    // Now tie the INTC's interrupt handler to the PPC Non-Critical Interrupt Handler

                                  (Xil_ExceptionHandler)XIntc_DeviceInterruptHandler,
                                  (void*)SYSINTC_DEVICE_ID);

    status = XIntc_Start (&sys_intc, XIN_REAL_MODE);                    // Start the interrupt controller

    if (status != XST_SUCCESS) {
	DBG_PRINT ("XMK: init_int_system: XIntc_Start failed.\r\n");
	return;
    }
}
#endif /* CONFIG_INTC */

//----------------------------------------------------------------------------------------------------//
//  @func - xmk_enter_kernel
//! @desc
//!   Lock kernel by turning off ALL system interrupts
//! @return
//!   - Nothing
//! @note
//!   - Locks kernel against ALL interrupts.
//!   - Does not lock against critical interrupts.
//----------------------------------------------------------------------------------------------------//
inline void xmk_enter_kernel (void)
{
    Xil_ExceptionDisable();                         // Does not disable critical interrupt though
}

//----------------------------------------------------------------------------------------------------//
//  @func - xmk_leave_kernel
//! @desc
//!   Unlock kernel by turning on system timer interrupts
//! @return
//!   - Nothing
//! @note
//!   - Unlocks kernel with respect to ALL interrupts except critical interrupts
//----------------------------------------------------------------------------------------------------//
inline void xmk_leave_kernel( void )
{
    Xil_ExceptionEnable();
}



//----------------------------------------------------------------------------------------------------//
//  @func - pit_initialize
//! @desc
//!   Initialize the PIT timer in the system
//! @param
//!   - interval is the interval to interrupt the system at.
//! @return
//!   - nothing
//! @note
//!  - none
//----------------------------------------------------------------------------------------------------//
void pit_initialize (unsigned int interval)
{
    pit_timer_interval = interval;
    PIT_DISABLE_INTR ();
    PIT_SET_INTERVAL (interval);
    PIT_ENABLE_AUTO_RELOAD ();
    PIT_ENABLE_INTR ();
}

//----------------------------------------------------------------------------------------------------//
//  @func - pit_reset
//! @desc
//!   Enable and start PIT timer with configured PIT interval loaded to the PIT counter
//! @param
//!   - none
//! @return
//!   - nothing
//! @note
//!   - none
//----------------------------------------------------------------------------------------------------//
void pit_reset (void)
{
    // Refresh budget only if the timer ran out
    if (timer_need_refresh) {
        PIT_SET_INTERVAL (pit_timer_interval) ;
        PIT_ENABLE_AUTO_RELOAD () ;
        PIT_CLEAR_STATUS ();

        // Reset scheduler/timer state
        did_resched = 0;
        timer_need_refresh = 0;
        sched_partial_tick = 0;
    } else {
        // FIXME!
        //  - did_resched seems bogus. We will be here only if we do a resched
        //    seems redundant. Leaving it in, because it is harmless if redundant
        if (did_resched) {
            did_resched = 0;
            sched_partial_tick = 1;
        }
    }

    // Enable the timer.
    PIT_ENABLE_INTR();
}

void pit_disable (void)
{
    PIT_DISABLE_INTR ();
}


void setup_initial_context (process_struct *pcb, pid_t parent, unsigned int start_addr, unsigned int stackaddr, unsigned int stacksize)
{
    pcb->pcontext.isrflag = ISRFLAG_NEW_PROC;
#ifndef PPC_CPU_440
    pcb->pcontext.regs[CTX_INDEX_MSR] = ptable[parent].pcontext.regs[CTX_INDEX_MSR] | XIL_EXCEPTION_NON_CRITICAL;
#else
    // We set MSR[DS] = 1 here, because that is the TLB scheme we use to
    // separate instruction and data space on the 440.
    pcb->pcontext.regs[CTX_INDEX_MSR] = ptable[parent].pcontext.regs[CTX_INDEX_MSR] | XIL_EXCEPTION_NON_CRITICAL | XREG_MSR_TLB_DATA_TS;
#endif
    pcb->pcontext.regs[CTX_INDEX_PC] = start_addr;
    pcb->pcontext.regs[CTX_INDEX_GPR(1)] = stackaddr;
    pcb->pcontext.regs[CTX_INDEX_GPR(2)]  = ptable[parent].pcontext.regs[CTX_INDEX_GPR(2)];
    pcb->pcontext.regs[CTX_INDEX_GPR(13)] = ptable[parent].pcontext.regs[CTX_INDEX_GPR(13)];
    stacksize = 0; /* Dummy to remove compilation warnings */
}

void init_idle_task (void)
{
    idle_task_pid = proc_create (PRIO_LOWEST);                          // Idle task (PID 0).
    ptable[idle_task_pid].state = PROC_RUN;                             // Idle task assumed to be running as soon as the kernel starts
#ifndef PPC_CPU_440
    ptable[idle_task_pid].pcontext.regs[CTX_INDEX_MSR]    = mfmsr () | XIL_EXCEPTION_NON_CRITICAL;
#else
    // We set MSR[DS] = 1 here, because that is the TLB scheme we use to
    // separate instruction and data space on the 440.
    ptable[idle_task_pid].pcontext.regs[CTX_INDEX_MSR]    = mfmsr () | XIL_EXCEPTION_NON_CRITICAL | XREG_MSR_TLB_DATA_TS;
#endif
    ptable[idle_task_pid].pcontext.regs[CTX_INDEX_PC]     = (unsigned int)idle_task;
    ptable[idle_task_pid].pcontext.regs[CTX_INDEX_GPR(1)] = mfgpr (1);
    ptable[idle_task_pid].pcontext.regs[CTX_INDEX_GPR(2)] = mfgpr (2);
    ptable[idle_task_pid].pcontext.regs[CTX_INDEX_GPR(13)]= mfgpr (13);
    SET_CURRENT_PROCESS (idle_task_pid);
}
