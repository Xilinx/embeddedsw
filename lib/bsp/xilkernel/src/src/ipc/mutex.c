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
//! @file mutex.c
//! POSIX Mutex locks implementation
//----------------------------------------------------------------------------------------------------//
#include <os_config.h>
#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/kpthread.h>
#include <sys/process.h>
#include <sys/queue.h>
#include <sys/decls.h>
#include <errno.h>


#ifdef CONFIG_PTHREAD_MUTEX

//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------//

//! Array of mutex structures. They are used to keep track of allocated
//! mutex. Also used for other bookkeeping operations on Mutex.
pthread_mutexinfo_t pthread_mutex_heap[MAX_PTHREAD_MUTEX] ;	// Memory for allocating mutex
pthread_mutexattr_t default_mutex_attr;

//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//
int  pthread_mutex_lock_basic (pthread_mutex_t *mutex, int trylock);
void pthread_mutex_heap_init (void);

//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//
void pthread_mutex_heap_init(void)
{
    unsigned int i = 0 ;

    for (; i < MAX_PTHREAD_MUTEX; i++ ) {
	pthread_mutex_heap[i].is_allocated = 0;
	alloc_q (&(pthread_mutex_heap[i].mutex_wait_q), MAX_PTHREAD_MUTEX_WAITQ,
		 PTHREAD_MUTEX_Q, sizeof(char), i) ;
    }
    default_mutex_attr.type = PTHREAD_MUTEX_DEFAULT;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_mutex_init
//! @desc
//!   Initialize a pthread mutex
//!   - Allocate a pthread mutex structure.
//!   - Initialize the pthread mutex structure
//! @param
//!   - mutex is the location to store mutex identifier in
//!   - attr is the mutex creation attributes. Unused currently.
//! @return
//!   - Returns 0 on success and mutex identifier in *mutex.
//!   - Returns EAGAIN if no more mutex structures can be allocated.
//!   - Returns EINVAL if attr is invalid
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
int sys_pthread_mutex_init (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    int i;

    if (mutex == NULL)
	return -1;

    if (attr == NULL)
        attr = &default_mutex_attr;

    if (attr->type != PTHREAD_MUTEX_DEFAULT && attr->type != PTHREAD_MUTEX_RECURSIVE) {
        kerrno = EINVAL;
        return -1;
    }

    for (i=0; i < MAX_PTHREAD_MUTEX; i++) {
	if (pthread_mutex_heap[i].is_allocated == 0)
	    break;
    }

    if (i == MAX_PTHREAD_MUTEX)
	return EAGAIN;

    *mutex = (pthread_mutex_t)i;
    pthread_mutex_heap[i].is_allocated = 1;
    pthread_mutex_heap[i].locked = 0;    // Unlocked
    pthread_mutex_heap[i].owner = -1;
    pthread_mutex_heap[i].attr.type = attr->type;

    qinit (&(pthread_mutex_heap[i].mutex_wait_q));
    return 0;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_mutex_destroy
//! @desc
//!   Destroy a pthread mutex
//!   - Check if a valid pthread mutex is being referenced
//!   - Deallocate it and set *mutex to an invalid value
//! @param
//!   - mutex is the pointer to mutex identifier that is being referenced
//! @return
//!   - Returns 0 on success
//!   - Returns EINVAL on invalid mutex reference
//! @note
//!   - Mutex lock/unlock state disregarded during destroy. No consideration is given for
//!     waiting processes.
//----------------------------------------------------------------------------------------------------//
int sys_pthread_mutex_destroy (pthread_mutex_t *mutex)
{
    unsigned int index;

    if (mutex == NULL)                                                                  // Verify parameter
	return EINVAL;

    if (*mutex == PTHREAD_MUTEX_INITIALIZER) {
	*mutex = (pthread_mutex_t)PTHREAD_MUTEX_INVALID;
	return 0;
    }

    if (*mutex >= MAX_PTHREAD_MUTEX)
	return EINVAL;

    index = (unsigned int)*mutex;
    if (pthread_mutex_heap[index].is_allocated != 1)
	return EINVAL;

    if (pthread_mutex_heap[index].locked != 0)
        return EBUSY;

    pthread_mutex_heap[index].is_allocated = 0;
    *mutex = (pthread_mutex_t)PTHREAD_MUTEX_INVALID;                                    // An invalid value

    return 0;
}

//----------------------------------------------------------------------------------------------------//
//  @func - pthread_mutex_lock_basic
//! @desc
//!   Lock a mutex
//!   - If mutex has only been statically initialized, initialize with a call to pthread_mutex_init
//!   - If mutex is not already locked, lock it.
//!   - If mutex is locked, then if trylock is true, then return immediately with EBUSY. Block onto
//!     mutex wait queue if trylock is not true.
//!   - When the call returns the mutex is locked by the process that made the call
//!   - Ownership checks are performed if mutex type is PTHREAD_MUTEX_RECURSIVE
//!   - Recursive locks, cause the mutex lock count to be incremented.
//! @param
//!   - mutex is the pointer to mutex identifier
//!   - trylock indicates if lock should just be tried for instead of committing to acquire the lock
//! @return
//!   - Returns 0 on success and the mutex in a locked state.
//!   - Returns EBUSY if mutex locked and trylock requested. -1 on unhandled errors.
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
int pthread_mutex_lock_basic (pthread_mutex_t *mutex, int trylock)
{
    unsigned int i;

    if ((mutex == NULL) ||                                                                      // Verify parameters
        ((*mutex != PTHREAD_MUTEX_INITIALIZER) &&
         (*mutex >= MAX_PTHREAD_MUTEX)))
        return EINVAL;

    if (*mutex == PTHREAD_MUTEX_INITIALIZER)  {                                                 // Initialize statically allocated mutex lock
	if (sys_pthread_mutex_init(mutex, &default_mutex_attr) != 0)
	    return -1;                                                                          // Undefined error code
    }

    i = (unsigned int)*mutex;
    if (pthread_mutex_heap[i].is_allocated != 1)
        return EINVAL;

    while (pthread_mutex_heap[i].locked) {
        if ((pthread_mutex_heap[i].attr.type != PTHREAD_MUTEX_RECURSIVE ||                      // Suspend this request only if either of
             pthread_mutex_heap[i].owner != current_pid)) {                                     //    a. MUTEX type is not recursive
            if (trylock)                                                                        //    b. Requestor is not owner
                return EBUSY;                                                                   // or both are true.
            else
                process_block (&(pthread_mutex_heap[i].mutex_wait_q), PROC_WAIT);
        } else
            break;
    }


    if (pthread_mutex_heap[i].attr.type == PTHREAD_MUTEX_RECURSIVE)
        pthread_mutex_heap[i].locked++;                                                         // Lock with lock count incremented
    else
        pthread_mutex_heap[i].locked = 1;                                                       // Simple lock

    pthread_mutex_heap[i].owner = current_pid;
    return 0;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_mutex_lock
//! @desc
//!   Wrapper function for pthread_mutex_lock_basic routine, with trylock forced to 0.
//! @param
//!   Same as pthread_mutex_lock_basic
//! @return
//!   Same as pthread_mutex_lock_basic
//! @note
//!   - Save code space by using same underlying implementation as sys_pthread_mutex_trylock
//----------------------------------------------------------------------------------------------------//
int sys_pthread_mutex_lock (pthread_mutex_t *mutex)
{
    return pthread_mutex_lock_basic (mutex, 0);
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_mutex_lock
//! @desc
//!   Wrapper function for pthread_mutex_lock_basic routine, with trylock forced to 1.
//! @param
//!   Same as pthread_mutex_lock_basic
//! @return
//!   Same as pthread_mutex_lock_basic
//! @note
//!   - Save code space by using same underlying implementation as sys_pthread_mutex_lock
//----------------------------------------------------------------------------------------------------//
int sys_pthread_mutex_trylock (pthread_mutex_t *mutex)
{
    return pthread_mutex_lock_basic (mutex, 1);
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_pthread_mutex_unlock
//! @desc
//!   Unlock a pthread mutex
//!   - If mutex has only been statically initialized, initialize with a call to pthread_mutex_init
//!   - Unlock the mutex. If there are processes waiting on the mutex, unblock a process
//!     according to the scheduling scheme
//!   - Ownership checks are performed if mutex type is PTHREAD_MUTEX_RECURSIVE
//!   - Recursive unlocks, cause the mutex lock count to be decremented.
//! @param
//!   - mutex is the pointer to the mutex identifier
//! @return
//!   - Returns 0 on success and mutex in unlocked state.
//!   - Returns -1 on undefined errors
//! @note
//!   - No ownership checks, lock status etc. associated with XSI mutex types performed. This is the
//!     barebones mutex implementation
//!   - In SCHED_PRIO processes are unblocked 'highest priority first'.
//!     In SCHED_RR processes are unblocked in FIFO order.
//----------------------------------------------------------------------------------------------------//
int sys_pthread_mutex_unlock (pthread_mutex_t *mutex)
{
    unsigned int i;

    if ((mutex == NULL) ||                                                                      // Verify parameter
	((*mutex != PTHREAD_MUTEX_INITIALIZER) &&
	 (*mutex >= MAX_PTHREAD_MUTEX)))
	return EINVAL;

    if (*mutex == PTHREAD_MUTEX_INITIALIZER)                                                    // Initialize statically allocated mutex lock
	if (sys_pthread_mutex_init(mutex, &default_mutex_attr) != 0)
	    return -1;

    i = (unsigned int)*mutex;
    if (pthread_mutex_heap[i].is_allocated != 1)
        return EINVAL;

    if (pthread_mutex_heap[i].attr.type == PTHREAD_MUTEX_RECURSIVE) {
        if (pthread_mutex_heap[i].owner != current_pid)
            return EPERM;
        pthread_mutex_heap[i].locked--;                                                         // Unlock
    }
    else
        pthread_mutex_heap[i].locked = 0;

    if (pthread_mutex_heap[i].locked == 0) {
        pthread_mutex_heap[i].owner = -1;
        if (pthread_mutex_heap[i].mutex_wait_q.item_count > 0)
            process_unblock (&(pthread_mutex_heap[i].mutex_wait_q));
    }

    return 0;
}
#endif /* CONFIG_PTHREAD_MUTEX */
