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
//! @file sched.c
//! This file contains routines for process scheduling
//----------------------------------------------------------------------------------------------------//

#include <stdio.h>
#include <os_config.h>
#include <sys/ksched.h>
#include <sys/entry.h>
#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/process.h>
#include <sys/decls.h>

//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------//


// Ready Queue - Array of N_PRIO process queues
struct _queue ready_q[N_PRIO] ;
signed char entry_mode = ENTRY_MODE_USER;       // Current entry mode into kernel
signed char resched = 0;                        // Indicates if rescheduling occurred elsewhere
char did_resched = 0;                           // Indicates if the kernel completed the rescheduling

#ifdef CONFIG_STATS
signed char sched_history[SCHED_HISTORY_SIZ];
int shp = 0;
#endif
//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//
extern unsigned int budget_ticks;
extern process_struct *ctx_save_process;
extern int save_context (process_struct *);
extern void restore_context (void);

int scheduler (void);

//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//

void readyq_init(void)
{
    unsigned int i = 0 ;

    for (;i < N_PRIO; i++ ) {
	alloc_q (&ready_q[i], MAX_READYQ, READY_Q, sizeof(char), i);
    }
}
int scheduler (void)
{

#ifdef CONFIG_DEBUGMON
    debugmon_dump_sched_info ();
#endif

    if (current_process->state == PROC_DEAD) {
	ctx_save_process = NULL;
	prev_pid = -1;
    }
    else if (current_process->pcontext.isrflag == 1) {    // If entered the kernel through an ISR, context saved in ISR itself
	ctx_save_process = NULL;
	prev_pid = current_pid;
    }
    else {
	ctx_save_process = current_process;
	prev_pid = current_pid;
    }

#if SCHED_TYPE == SCHED_RR
    sched_rr ();
#elif SCHED_TYPE == SCHED_PRIO
    sched_prio ();
#endif

    if( current_pid == -1 ) {
	DBG_PRINT ("XMK: Unable to find schedulable process. Kernel Halt.\r\n") ;
	while(1);
    }

    did_resched = 1;    // scheduler indicates that it completed the rescheduling
    resched = 0;        // scheduler always resets resched flag

#ifdef CONFIG_DEBUGMON
    debugmon_stack_check ();
#endif

#ifdef CONFIG_STATS
    if (shp != SCHED_HISTORY_SIZ)
	sched_history[shp++] = current_pid;
#endif

    if (prev_pid == current_pid)
	return 1;

    return 0;
}

#if SCHED_TYPE == SCHED_RR
//----------------------------------------------------------------------------------------------------//
//  @func - sched_rr
//! @desc
//!   Round Robin Scheduler.
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void sched_rr (void)
{
    signed char ready = -1;

    // Enqueue only currently running processes. Else,
    // If PROC_DEAD, no need to enqueue
    // If PROC_WAIT or PROC_TIMED_WAIT, already enqueued in appropriate wait queue
    // If PROC_DELAY, is in one of the timer queues
    // If idle_task, then does not need to enter the queue
    if (current_process->state == PROC_RUN) {
        ptable[current_pid].state = PROC_READY;
	if(current_pid != idle_task_pid)
	    penq (&ready_q[0], current_pid, 0);
    }

    SET_CURRENT_PROCESS (-1);

    while (ready_q[0].item_count != 0) {
	pdeq (&ready_q[0], &ready, 0);
	if (ptable[ready].state == PROC_DEAD) {   // Flush out dead processes
	    ready = -1;
	    continue;
	}
	else break;
    }

    if (ready == -1)
	ready = idle_task_pid;

#if 0
    DBG_PRINT ("XMK: Scheduler: scheduled pid: ");
    putnum (ready);
    DBG_PRINT ("\r\n");
#endif

    ptable[ready].state = PROC_RUN;
    SET_CURRENT_PROCESS (ready);
}
#endif /* SCHED_TYPE == SCHED_RR */

#if SCHED_TYPE == SCHED_PRIO
//----------------------------------------------------------------------------------------------------//
//  @func - sched_prio
//! @desc
//!   Pre-emptive strict priority scheduler
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void sched_prio (void)
{
    int i;
    signed char ready = -1;

    // Enqueue only currently running processes. Else,
    // If PROC_DEAD, no need to enqueue
    // If PROC_WAIT or PROC_TIMED_WAIT, already enqueued in appropriate wait queue
    // If PROC_DELAY, is in one of the timer queues
    // If idle_task, then does not need to enter the queue
    if (current_process->state == PROC_RUN) {
	ptable[current_pid].state = PROC_READY;
	if (current_pid != idle_task_pid)
	    penq (&ready_q[ptable[current_pid].priority], current_pid, 0);
    }

    SET_CURRENT_PROCESS (-1);

    for (i=0; i <= PRIO_LOWEST; i++) {
	while (ready_q[i].item_count != 0) {
	    pdeq (&ready_q[i], &ready, 0);
	    if (ptable[ready].state == PROC_DEAD) {   // Flush out dead processes
		ready = -1;
		continue;
	    }
	    else break;
	}

	if (ready != -1)
	    break;
    }

    if (ready == -1)
	ready = idle_task_pid;

#if 0
    DBG_PRINT ("XMK: Scheduler: scheduled pid: ");
    putnum (ready);
    DBG_PRINT ("\r\n");
#endif

    ptable[ready].state = PROC_RUN;
    SET_CURRENT_PROCESS (ready);
}
#endif /* SCHED_TYPE == SCHED_PRIO */

//----------------------------------------------------------------------------------------------------//
//  @func - suspend
//! @desc
//!   Suspend a process inside the kernel. A rescheduling followed by the corresponding context
//!   switch occurs within this routine.
//! @return
//!   - Nothing
//! @note
//!   - This routine is not expected to be invoked from within an ISR. i.e no suspension allowed in
//!     an ISR.
//----------------------------------------------------------------------------------------------------//
void suspend (void)
{
#ifdef CONFIG_STATS
    current_process->active_ticks++;
    budget_ticks++;
#endif
    if (scheduler ()) {
	DBG_PRINT ("XMK: Scheduling error. Cannot suspend current process.\r\n");
	while (1)
	    ;
    }

    if (ctx_save_process != NULL)
	if (save_context (ctx_save_process))    // Save context returns 0 during save
	    return;                             // When saved context is restored, returns a 1

    restore_context ();
}



