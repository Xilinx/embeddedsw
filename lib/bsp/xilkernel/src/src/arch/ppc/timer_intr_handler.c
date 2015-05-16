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
//! @file timer_intr_handler.c
//! Contains PPC system timer interrupt handler routine
//----------------------------------------------------------------------------------------------------//

#include <os_config.h>
#include <sys/ksched.h>
#include <xpseudo_asm.h>
#include <sys/decls.h>
#include <xtime_l.h>

extern void soft_tmr_handler (void);
extern signed char resched;
extern process_struct *current_process;
unsigned int kernel_ticks;      //! Ticks since kernel startup
char timer_need_refresh  = 0;   //! Do we need a reset?
#ifdef CONFIG_STATS
unsigned int budget_ticks;
#endif
unsigned char sched_partial_tick;

//----------------------------------------------------------------------------------------------------//
//  @func - timer_int_handler
//! @desc
//!   System timer interrupt handler
//!   - Do a rescheduling operation
//!   - If a context switch occurs within this routine,
//!     - When returning out of process_scheduler, process executing will return in the appropriate
//!       context when it was switched out on a previous flow through this execution path.
//!   - Reset PIT interval to start a full time slice (if PIT in the system) (if INTC not present)
//! @param
//!   - none
//! @return
//!   - nothing
//! @note
//!   - May NOT return from this routine if a NEW process context is scheduled in the scheduler
//!   - A context switch does not occur within this routine if an INTC is present. The switch
//!     occurs at the end of the INTC ISR.
//----------------------------------------------------------------------------------------------------//
void timer_int_handler (int irq_num)
{
    irq_num = 0;
    // Update global kernel ticks so far
    kernel_ticks++;
#ifdef CONFIG_STATS
    current_process->active_ticks++;
    budget_ticks++;
#endif
#ifdef CONFIG_TIME
    soft_tmr_handler ();
#endif

    if (sched_partial_tick) {
#if SCHED_TYPE == SCHED_RR
        // Avoid one more starvation case. On a partial schedule tick,
        // you want to give the next full time quantum to the thread
        // which was scheduled partially and not any thread that
        // timed out in soft_tmr_handler.
        // Note: We do this only for RR scheduling where we don't
        //       want starvation.
        resched = 0;
#endif
        sched_partial_tick = 0;
    } else {
        resched = 1;
    }

    timer_need_refresh = 1;
}
