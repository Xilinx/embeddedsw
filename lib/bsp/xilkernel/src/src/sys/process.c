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
//! @file process.c
//! This contains process management and thread management modules.
//----------------------------------------------------------------------------------------------------//

#include <stdio.h>
#include <string.h>
#include <os_config.h>
#include <sys/init.h>
#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/arch.h>
#include <sys/ktypes.h>
#include <sys/ksched.h>
#include <sys/process.h>
#include <sys/mem.h>
#include <sys/queue.h>
#include <sys/ksemaphore.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/decls.h>
#include <sys/stats.h>
#include <sys/decls.h>
#include <sys/timer.h>
#include <pthread.h>
#ifdef CONFIG_PTHREAD_SUPPORT
#include <sys/kpthread.h>
#endif

//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------//

process_struct  ptable[MAX_PROCESS_CONTEXTS];           //! Process Table
void            *kernel_sp;                             //! Kernel Stack pointer
pid_t           current_pid = -1;                       //! Currently executing processes' ID
pid_t           prev_pid = -1;                          //! ID of process executing immediately before a context switch
process_struct  *current_process = NULL;
process_struct  *ctx_save_process = NULL;
pid_t           idle_task_pid = 0;
reent_t         reent;

//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//
extern unsigned int budget_ticks;
extern signed char resched;
extern unsigned int kernel_ticks;
extern signed char sched_history[SCHED_HISTORY_SIZ];
extern int shp;
void idle_task( void );                 // Idle task prototype

//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//

//----------------------------------------------------------------------------------------------------//
//  @func - idle_task
//! @desc
//!   - This task is run when the system is idle. This task is initialized
//!     during system init and is always run with the least priority.
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void idle_task (void)
{
    unsigned int i ;
    while (1) {
        DBG_PRINT ("Idle Task \r\n");
        for (i=0; i < 0x2fffff; i++)
            ;
    }
}

//----------------------------------------------------------------------------------------------------//
//  @func - proc_create
//! @desc
//!   Process creation primitive.
//!   - Reserves a pid for the process.
//!   - Initializes the process structure (except for the context)
//!   - Places the process in the READY_Q.
//! @param
//!   - priority is the priority of the process
//! @return
//!   - PID of the new process.
//!   - -1 on Error. Max. process exceeded.
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
pid_t proc_create (unsigned int priority)
{
    unsigned char i ;
    process_struct *pcb ;

    pcb = ptable ;
    for (i=0; i<MAX_PROCESS_CONTEXTS; i++){
        if (!pcb->is_allocated) {                                               // Get the first unallocated PCB for the process
            pcb->is_allocated = 1;
            pcb->pid = (pid_t)i;
            pcb->state = PROC_NEW;
            pcb->priority = priority;
            pcb->blockq = NULL;
            // pcb->reent.errno = 0;                                            // POSIX says no one sets errno to 0
            bzero ((void*)&pcb->pcontext, sizeof (process_context));
#ifdef CONFIG_STATS
            pcb->active_ticks = 0;
#endif

#ifdef CONFIG_PTHREAD_SUPPORT
            pcb->thread = NULL;                                                 // No thread associated with this process context currently.
#endif

            if (pcb->pid != 0) {                                                // Do not enqueue the idle_task
#if SCHED_TYPE == SCHED_RR
                penq (&ready_q[0], i, 0);
#else  /* SCHED_TYPE == SCHED_PRIO */
                penq (&ready_q[priority], i, 0);
#endif
            }
            break;
        }
        pcb++ ;
    }

    if (i == MAX_PROCESS_CONTEXTS)
        return -1 ;

#if SCHED_TYPE == SCHED_PRIO
    resched = 1;                                                                // Indicate rescheduling required
#endif
    return (pid_t) (i);
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_kill
//! @desc
//!   Removes the process with pid from the system. No indication is given to other processes that
//!   depend on this process.
//! @param
//!   - pid is the process ID of process to kill
//! @return
//!   - 0 on Success
//!   - -1 on Error
//! @note
//!   - Included only if CONFIG_KILL is defined (os_config.h) or if hardware exceptions are
//!     supported on the given processor
//----------------------------------------------------------------------------------------------------//
#if defined(CONFIG_KILL) || defined(CONFIG_HARDWARE_EXCEPTIONS)
int sys_kill (pid_t pid)
{
    if ((ptable[pid].is_allocated == 0))
        return -1;

    if (pid == current_pid) {
#ifdef CONFIG_PTHREAD_SUPPORT
        if (ptable[pid].thread)
            sys_pthread_exit (NULL);
#ifdef CONFIG_ELF_PROCESS
        else
            sys_elf_exit ();
#endif
#endif

        // Control does not reach here
    }

    if (ptable[pid].state == PROC_READY) {
#if SCHED_TYPE == SCHED_RR
        pdelq (&ready_q[0], pid);
#elif SCHED_TYPE == SCHED_PRIO
        pdelq (&ready_q[ptable[pid].priority], pid);
#endif
    } else if (ptable[pid].state == PROC_WAIT || ptable[pid].state == PROC_TIMED_WAIT) {
#if SCHED_TYPE == SCHED_RR
        pdelq (ptable[pid].blockq, pid);
#elif SCHED_TYPE == SCHED_PRIO
        prio_pdelq (ptable[pid].blockq, pid);
#endif
#ifdef CONFIG_TIME
        if (ptable[pid].state == PROC_TIMED_WAIT)
            remove_tmr (pid);
    } else if (ptable[pid].state == PROC_DELAY)
        remove_tmr (pid);
#else
    }
#endif

#ifdef CONFIG_PTHREAD_SUPPORT
    if (ptable[pid].thread)
        pthread_terminate (ptable[pid].thread);
    else
#endif
        process_invalidate(&ptable[pid]);

    resched = 1;
    return 0;
}
#endif /* CONFIG_KILL */

//----------------------------------------------------------------------------------------------------//
//  @func - process_invalidate
//! @desc
//!   Remove the Process with pid.
//!   This is an internal proc that is called by
//!   sys_kill() and also by the schedulers to remove "DEAD" processes
//! @param
//!   - proc is the process structure of process to invalidate
//! @return
//!   - 0 on Success
//!   - -1 on Error
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
int process_invalidate (process_struct *proc)
{
#ifdef CONFIG_STATS
    budget_ticks++;
#endif
    proc->is_allocated = 0 ;
    proc->state = PROC_DEAD;
    return 0;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_process_status
//! @desc
//!   Return the status of the process.
//! @param
//!   - pid is the Process ID of the process.
//!   - ps is the structure where the status is returned.
//! @return
//!   - The status of the process is returned on ps
//!   - If pid not an active process, ps->pid is assigned -1
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
int sys_process_status (pid_t pid, p_stat *ps)
{
    if (ptable[pid].is_allocated) {
        ps->pid = pid;
        ps->state = ptable[pid].state ;
    } else {
        ps->pid = -1;
        return -1 ;
    }

    return 0 ;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_yield
//! @desc
//!   Yield the processor to the next process.
//!   - The current process changes to READY state and is enqueued in the
//!     ready queue.
//! @return
//!   - None
//! @note
//!   - Included only if CONFIG_YIELD is defined. (os_config.h)
//----------------------------------------------------------------------------------------------------//
#ifdef CONFIG_YIELD
int sys_yield(void)
{
    resched = 1;
    return 0 ;
}
#endif  /* CONFIG_YIELD */

//----------------------------------------------------------------------------------------------------//
//  @func - sys_get_currentPID
//! @desc
//!   Return the PID of the currently running process context
//! @return
//!   - PID of the currently running process context.
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
pid_t sys_get_currentPID (void)
{
    return current_pid ;
}

//----------------------------------------------------------------------------------------------------//
//  @func - process_block
//! @desc
//!   Block the process.
//!   - Place the process into the specified wait queue, change the state of the process and
//!     set the kernel flag to PROCESS_BLOCK.
//!   - Call the process_scheduler
//! @param
//!   - queue is the queue where the process is enqueued.
//!   - state is the state of the process in queue.
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void process_block (queuep queue, unsigned int state)
{
#if SCHED_TYPE == SCHED_RR
    penq (queue, current_pid, 0);
#else
    prio_penq (queue, current_pid, 0);
#endif
    current_process->blockq = queue;
    current_process->state = state;
    suspend ();
}

//----------------------------------------------------------------------------------------------------//
//  @func - process_unblock
//! @desc
//!   Unblock the first process in queue and place it onto the ready queue.
//!   Call the process_scheduler only if this is PRIO scheduling. This is because we do not want the
//!   current process to prematurely lose its time slice. Change the state of the process to
//!   PROC_READY
//! @param
//!   - queue is the queue where the process is enqueued.
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void process_unblock (queuep queue)
{
    pid_t pid;

#if SCHED_TYPE == SCHED_RR
    pdeq (queue, &pid, 0);
#else
    prio_pdeq (queue, &pid, 0);
#endif

    if (pid == -1)
        return;

#ifdef CONFIG_TIME
    if (ptable[pid].state == PROC_TIMED_WAIT)                                                   // This process is also on a list of timers. Remove it.
        ptable[pid].remain = remove_tmr (pid);

#endif

    ptable[pid].state = PROC_READY;
    ptable[pid].blockq = NULL;

#if SCHED_TYPE == SCHED_RR
    penq (&ready_q[0], pid, 0);
#else /* SCHED_TYPE == SCHED_PRIO */
    penq (&ready_q[ptable[pid].priority], pid, 0);
    resched = 1;
#endif
}

void proc_restore_state (void)
{
    reent._errno = current_process->reent._errno;
}

#ifdef CONFIG_STATS
//----------------------------------------------------------------------------------------------------//
//  @func - get_kernel_stats
//! @desc
//!   Retrieve statistics about executing processes, ticks, history of processes scheduled since last
//!   last call etc.
//! @param
//!   - stats is the structure to store the statistics information in.
//! @return
//!   - 0 on success
//!   - -1 if error
//!   - 1 if history overflowed
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
int sys_get_kernel_stats (kstats_t *stats)
{
    int i, count, ret = 0;

    if (stats == NULL)
        return -1;

    if (stats->pstat_count <= 0)
        return -1;

    count = 0;
    for (i=0; i<MAX_PROCESS_CONTEXTS; i++) {
        if (count == stats->pstat_count)
            break;

        if (!ptable[i].is_allocated)
            continue;

        stats->pstats[count].pid = ptable[i].pid;
        stats->pstats[count].state = ptable[i].state;
        stats->pstats[count].priority = ptable[i].priority;
        stats->pstats[count].aticks = ptable[i].active_ticks;

        count++;
    }

    stats->pstat_count = count;
    stats->kernel_ticks = budget_ticks;

    for (i=0; i<shp; i++)
        stats->sched_history[i] = sched_history[i];

    if (shp == SCHED_HISTORY_SIZ)
        ret = 1;
    else
        stats->sched_history[i] = -1;

    shp = 0;       // Reset history pointer
    return ret;
}
#endif

//----------------------------------------------------------------------------------------------------//
//  @func - sys_get_reentrancy
//! @desc
//!   Return the kernel re-entrancy structure. This structure holds program level state
//!   information for the current process. Upon each context switch this structure is updated.
//!   to minimize the run-time efficiency, each application ELF file can query a pointer to this
//!   structure once and then use it for all subsequent state information queries.
//! @param
//!   - None
//! @return
//!   - Pointer to kernel re-entrancy structure
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
reent_t* sys_get_reentrancy (void)
{
    return &reent;
}
