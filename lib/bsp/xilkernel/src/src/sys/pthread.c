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
//! @file pthread.c
//! The kernel pthreads implementation
//----------------------------------------------------------------------------------------------------//

#include <stdio.h>
#include <os_config.h>
#include <config/config_cparam.h>
#include <config/config_param.h>
#include <sys/arch.h>
#include <sys/kpthread.h>
#include <sys/ksched.h>
#include <sys/process.h>
#include <sys/queue.h>
#include <sys/mem.h>
#include <sys/ksemaphore.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/init.h>
#include <sys/decls.h>
#include <errno.h>


#ifdef CONFIG_PTHREAD_SUPPORT
//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------//
pthread_attr_t default_attr;
pthread_info_t thread_info_list[MAX_PTHREADS];
//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//
extern process_struct ptable[MAX_PROCESS_CONTEXTS] ;
extern process_struct *current_process;
extern signed char current_pid;
extern signed char resched;
extern struct _queue ready_q[] ;        // Ready Queue
extern void setup_initial_context (process_struct *pcb, pid_t parent, unsigned int startaddr, unsigned int stackaddr, unsigned int stacksize);

void invalidate_thread_info (pthread_info_t *thread);
void* pthread_wrapper (void *arg);
pthread_info_t* pthread_get_info (pthread_t thr);

#ifdef CONFIG_PTHREAD_MUTEX
extern void pthread_mutex_heap_init(void);
#endif
//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//
void pthread_init (void)
{
    int i;

    bss_mem_init ();
    default_attr.schedparam.sched_priority = PRIO_LOWEST;       // Defaults to the lowest priority.
    default_attr.contentionscope = PTHREAD_SCOPE_SYSTEM;        // Only system scope supported.
    default_attr.detachstate = PTHREAD_CREATE_DETACHED;         // Detach on thread exit
    default_attr.stackaddr = NULL;
    default_attr.stacksize = 0;

    for (i=0; i<MAX_PTHREADS; i++) {
        thread_info_list[i].is_allocated = 0;
        thread_info_list[i].state = PTHREAD_STATE_DETACHED;
        thread_info_list[i].join_thread = NULL;
        thread_info_list[i].tid = (pthread_t)i;                 // Can statically allocate thread ids.
        thread_info_list[i].mem_id = -1;                        // BSS memory unallocated.
        thread_info_list[i].thread_attr = default_attr;
        thread_info_list[i].parent = NULL;
        alloc_q (&thread_info_list[i].joinq,1,PTHREAD_JOIN_Q,sizeof(char),0);
        thread_info_list[i].joinq.items = &(thread_info_list[i].joinq_mem);
    }

#ifdef CONFIG_PTHREAD_MUTEX
    pthread_mutex_heap_init();
#endif
}

void invalidate_thread_info (pthread_info_t *thread)
{
    thread->is_allocated = 0;
    thread->join_thread = NULL;
    thread->start_func = NULL;
    thread->param = NULL;
    thread->retval = NULL;
    thread->parent = NULL;
    thread->state = PTHREAD_STATE_DETACHED;
    alloc_q (&(thread->joinq),1,PTHREAD_JOIN_Q,sizeof(char),0);         // Realloc Q afresh
    qinit (&(thread->joinq));
    thread->joinq.items = &(thread->joinq_mem);

    if( thread->mem_id != -1 ) {
        free_bss_mem (thread->mem_id) ;
        thread->mem_id = -1;                                            // BSS memory unallocated.
    }

    thread->thread_attr = default_attr;
}

void* pthread_wrapper (void *arg)
{
    void* status = NULL;
    pthread_t cur = sys_pthread_self ();
    pthread_info_t *self = pthread_get_info (cur);

    arg = 0;
    status = (*(self->start_func))(self->param);

    xmk_enter_kernel ();
    sys_pthread_exit (status);          // Implicit exit at the end of the thread's execution
    xmk_leave_kernel ();                // We should never get here. There will be a leave_kernel done on our behalf by newly scheduled process or kernel

    return status;
}

pthread_info_t* pthread_get_info (pthread_t thr)
{
    if ((int)thr >=0 && (int)thr < MAX_PTHREADS)
        return &thread_info_list[thr];
    return NULL;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_create
//! @desc
//!   Thread creation routine
//!   - Verify pthread attributes
//!   - Allocate a pthread info structure
//!   - Allocate a context (bss memory) for the new thread.
//!   - Call sys_process_create to allocate a pid.
//!   - If SCHED_PRIO, call the process_scheduler
//! @param
//!   - thread is location where the ID of the created thread is stored.
//!   - attr is the reference to thread initialization attributes.
//!   - start_func is the address of start routine.
//!   - param is the pointer argument to the thread.
//! @return
//!   - 0 on success and thread ID of created thread in *'thread'
//!   - EINVAL, EAGAIN on errors
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
int sys_pthread_create (pthread_t *thread, const pthread_attr_t *attr,
                        void *(*start_func)(void*), void *param)
{
    int pid, i;
    unsigned int  bss_start_addr, bss_end_addr, stackaddr, stacksize;
    pthread_info_t *cur_thread;

    if (thread == NULL)                                                   // Should actually just throw an illegal memory access exception
        return -1;                                                        // Rather returning an undefined error code.

    if (attr != NULL) {                                                   // Examine attr to determine validity
        if ((attr->schedparam.sched_priority > PRIO_LOWEST)               // attr priority parameters out of bounds
            || (attr->schedparam.sched_priority < PRIO_HIGHEST)           // attr priority parameters out of bounds
            || (attr->contentionscope != PTHREAD_SCOPE_SYSTEM)            // Only system contention scope supported
            || ((attr->detachstate != PTHREAD_CREATE_DETACHED)
                && (attr->detachstate != PTHREAD_CREATE_JOINABLE))
    )
        {
            return EINVAL;
        }
    }
    else attr = &default_attr;

    cur_thread = thread_info_list;
    for (i=0; i < MAX_PTHREADS; i++) {
        if (!(cur_thread->is_allocated)){                                 // Get the first unallocated
            cur_thread->is_allocated = 1 ;                                // Thread info structure for this thread
            cur_thread->state = PTHREAD_STATE_ALIVE;
            cur_thread->start_func = start_func;
            cur_thread->param = param;
            cur_thread->join_thread = NULL;
            cur_thread->thread_attr = *attr;
            qinit (&(cur_thread->joinq));
            break;
        }
        cur_thread++;
    }

    if (i == MAX_PTHREADS)                                                // No more resources to create new threads.
        return EAGAIN;

    if (attr->stackaddr == NULL) {                                        // Allocate a stack from our BSS pool only if no stack has been specified in attr
        cur_thread->mem_id = alloc_bss_mem (&bss_start_addr, &bss_end_addr);
        if (cur_thread->mem_id == -1) {
            invalidate_thread_info (cur_thread);
            return EAGAIN ;
        }
    }

    pid = proc_create (attr->schedparam.sched_priority);
    if( pid == -1 ) {
        invalidate_thread_info (cur_thread);                              // No more resources to create new threads.
        return EAGAIN;
    }

    if (attr->stackaddr == NULL) {
        stackaddr = (bss_end_addr + SSTACK_PTR_ADJUST);
        stacksize = PTHREAD_STACK_SIZE;
    }
    else {
        stackaddr = (unsigned int)(attr->stackaddr + attr->stacksize + SSTACK_PTR_ADJUST);
        stacksize = attr->stacksize;
    }

    setup_initial_context (&ptable[pid], current_pid, (unsigned int)pthread_wrapper, stackaddr, stacksize);

    ptable[pid].thread = cur_thread;
    cur_thread->parent = &ptable[pid];
    *thread = cur_thread->tid;

#ifdef CONFIG_DEBUGMON
    cur_thread->thread_attr.stackaddr = (void*)stackaddr;
    cur_thread->thread_attr.stacksize = stacksize;
#endif

    return 0; // Success
}


//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_exit
//! @desc
//!   Thread termination routine.
//!   - Store retval for reclamation by other threads. Set thread state.
//!   - If thread waiting to join, then unblock that thread and terminate.
//!   - If detachstate is set, then detach self.
//!   - Block and terminate.
//! @param
//!   - retval is the value returned by thread's main routine
//! @return
//!   - Does not return
//! @note
//!   - Irrespective of whether detachstate is PTHREAD_CREATE_DETACHED or not, thread waiting to join
//!     is unblocked.
//----------------------------------------------------------------------------------------------------//
void sys_pthread_exit (void *retval)
{
    pthread_info_t *self = current_process->thread;

    self->retval = retval;                              // Store the retval for use by joining threads
    self->state = PTHREAD_STATE_EXIT;

    if (self->join_thread != NULL)
        process_unblock (&self->joinq);                 // Unblock the thread that is waiting to join with self.

    if (self->thread_attr.detachstate == PTHREAD_CREATE_DETACHED) {
        process_invalidate (self->parent);
        invalidate_thread_info (self);
    }
    else current_process->state = PROC_DEAD;

    suspend ();
}

void pthread_terminate (pthread_info_t *thread)
{
    thread->retval = NULL;                              // Store the retval for use by joining threads
    thread->state  = PTHREAD_STATE_EXIT;

    if (thread->join_thread != NULL)
        process_unblock (&thread->joinq);               // Unblock the thread that is waiting to join with self.

    if (thread->thread_attr.detachstate == PTHREAD_CREATE_DETACHED) {
        process_invalidate (thread->parent);
        invalidate_thread_info (thread);
    }
    else thread->parent->state = PROC_DEAD;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_join
//! @desc
//!   Suspend current thread till target thread terminates. Then completely detach target thread.
//!   - Verify target is present and is joinable
//!   - Block onto target's join queue
//!   - When unblocked, if thread not already detached, detach it
//!   - if retval is not NULL, then store target's return value in *retval.
//! @param
//!   - target is the thread to join with
//!   - retval is the location to store return value of target thread.
//! @return
//!   - Return 0 on success and return value of target thread in location referenced by retval.
//!   - Return ESRCH, EINVAL as appropriate
//! @note
//!   - none
//----------------------------------------------------------------------------------------------------//
int sys_pthread_join (pthread_t target, void **retval)
{
    pthread_info_t *target_info = pthread_get_info (target);
    pthread_t cur  = sys_pthread_self ();
    pthread_info_t *self = pthread_get_info (cur);

    if (target_info == NULL)
        return ESRCH;

    // @note - Can possibly detect deadlocks here
    if (target_info->state == PTHREAD_STATE_ALIVE) {
        if (target_info->join_thread != NULL)                   // Some other thread already waiting to join
            return EINVAL;                                      // Therefore invalid to join with this target

        target_info->join_thread = self;                        // Block and yield execution to some other context
        process_block (&(target_info->joinq), PROC_WAIT);       // Indicate that self wants to join with target.

        if (retval != NULL)
            *retval = target_info->retval;
    } else if (target_info->state == PTHREAD_STATE_DETACHED)    // Can potentially return success here.
        return ESRCH;                                           // POSIX is not specific about behavior on multiple joins to an already terminated thread.

    if (target_info->state != PTHREAD_STATE_DETACHED)  {        // Target thread already in state PTHREAD_STATE_EXIT. Detach target thread.
        if (retval != NULL)                                     // Thread already detached if detachstate was PTHREAD_STATE_DETACHED,
            *retval = target_info->retval;
        process_invalidate (target_info->parent);               // Clear up corresponding parent structure
        invalidate_thread_info (target_info);
    }

    return 0; // Success
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_self
//! @desc
//!   Get handle to self
//! @param
//!   - none
//! @return
//!   - return pthread_t tid of current thread
//!   - -1 on error
//! @note
//!   - none
//----------------------------------------------------------------------------------------------------//
pthread_t sys_pthread_self (void)
{
    if (current_pid == -1)
        return (pthread_t)-1;
    return (pthread_t)current_process->thread->tid;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_detach
//! @desc
//!   Change detachstate of target thread to PTHREAD_CREATE_DETACHED
//!   - If thread already terminated, then detach target, else change detachstate
//! @param
//!   - thread is the target thread
//! @return
//!   - Return 0 on success
//!   - ESRCH if target not found.
//! @note
//!   - none
//----------------------------------------------------------------------------------------------------//
int sys_pthread_detach (pthread_t thread)
{
    pthread_info_t *thread_info;
    thread_info = pthread_get_info (thread);

    if (thread_info == NULL)
        return ESRCH;

    if (thread_info->state == PTHREAD_STATE_EXIT) {
        process_invalidate (thread_info->parent);
        invalidate_thread_info (thread_info);
    }
    else
        thread_info->thread_attr.detachstate = PTHREAD_STATE_DETACHED;

    return 0;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_equal
//! @desc
//!   Compare two pthread_t structures
//! @param
//!   - thread_1 and thread_2 are the thread ID structures of the two threads
//! @return
//!   - Return 1 if equal, elze 0
//! @note
//!   - none
//----------------------------------------------------------------------------------------------------//
int sys_pthread_equal (pthread_t thread_1, pthread_t thread_2)
{
    if ((pthread_get_info (thread_1) == pthread_get_info (thread_2)))
        return 1;
    return 0;                               // Else
}

#if SCHED_TYPE == SCHED_PRIO
//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_getschedparam
//! @desc
//!   Form and return a sched_param structure of the thread identified by 'thread'
//! @param
//!   - thread is the identifier of the target thread
//!   - policy is a pointer to the structure where the current scheduling policy for this thread is saved
//!   - param is the sched_param structure to store the schedule attributes in
//! @return
//!   - Return 0 on success
//!   - Return ESRCH if target thread not found
//!   - Return EINVAL if param/policy point to invalid structures
//! @note
//!   - Currently sched_param contains only priority
//!   - The policy returned indicates the global scheduling policy (there is no per thread policy)
//----------------------------------------------------------------------------------------------------//
int sys_pthread_getschedparam (pthread_t thread, int *policy, struct sched_param *param)
{
    pthread_info_t *thread_info;
    thread_info = pthread_get_info (thread);

    if (thread_info == NULL)
        return ESRCH;

    if ((param == NULL || policy == NULL))
        return EINVAL;

    param->sched_priority = thread_info->parent->priority;
    *policy = SCHED_PRIO;
    return 0;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_setschedparam
//! @desc
//!   Change the scheduling parameters of the thread identified by 'thread'
//! @param
//!   - thread is the identifier of the target thread
//!   - policy is the target's new required scheduling policy
//!   - param is the target's new required scheduling parameters
//! @return
//!   - Return 0 on success
//!   - Return ESRCH if target thread not found
//!   - Return EINVAL if scheduling parameters are invalid
//!   - Return -1 on unhandled errors
//! @note
//!   - policy is ignored
//----------------------------------------------------------------------------------------------------//
int sys_pthread_setschedparam (pthread_t thread, int policy, struct sched_param *param)
{
    pthread_info_t *thread_info;
    thread_info = pthread_get_info (thread);

    if (thread_info == NULL)
        return ESRCH;

    if ((param->sched_priority > PRIO_LOWEST)                                           // Check requested priority change
        || (param->sched_priority < PRIO_HIGHEST))
        return EINVAL;

    if (thread_info->parent->pid == current_pid ||
        thread_info->parent->state == PROC_DELAY) {                                     // Just need to change the priority
        thread_info->parent->priority = param->sched_priority;
    } else if (thread_info->parent->state == PROC_READY) {                              // cannot handle processes which are blocked
        if (pdelq (&ready_q[thread_info->parent->priority],
                   thread_info->parent->pid) < 0)                                       // Remove from corresponding priority queue
            return -1;
        thread_info->parent->priority = param->sched_priority;                          // Change priority and enqueue in new queue
        penq (&ready_q[thread_info->parent->priority], thread_info->parent->pid, 0);
    } else if (thread_info->parent->state == PROC_WAIT ||
               thread_info->parent->state == PROC_TIMED_WAIT) {                         // Thread currently blocked
        if (prio_pdelq (thread_info->parent->blockq,                                    // Remove from corresponding wait queue
                        thread_info->parent->pid) < 0)
            return -1;
        thread_info->parent->priority = param->sched_priority;                          // Change priority and enqueue in new queue
        prio_penq (thread_info->parent->blockq, thread_info->parent->pid, 0);
    }

    resched = 1;
    return 0;
}
#endif  /* SCHED_TYPE == SCHED_PRIO     */
#endif  /* CONFIG_PTHREAD_SUPPORT       */


