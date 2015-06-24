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
//! @file semaphore.c
//! Semaphore handling routines.
//----------------------------------------------------------------------------------------------------//
#include <os_config.h>
#include <sys/init.h>
#include <sys/process.h>
#include <sys/ktypes.h>
#include <sys/ksemaphore.h>
#include <sys/decls.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/timer.h>
#include <sys/xtrace.h>

#ifdef CONFIG_SEMA
//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------//
//! Array of Semaphore structures. They are used to keep track of allocated
//! semaphore's. Also used for other bookkeeping operations on Semaphore.

sem_info_t sem_heap[MAX_SEM];	// Memory for allocating semaphores
#ifdef CONFIG_NAMED_SEMA
sem_map_t  sem_map[MAX_SEM];    // Memory for mapping semaphores to symbolic names
#endif
//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//

//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//
void sem_heap_init (void)
{
    unsigned int i = 0 ;

    for (; i < MAX_SEM; i++ ) {
	sem_heap[i].sem_id = -1 ;
	sem_heap[i].owner = -1;
	sem_heap[i].unlink = 0;
	alloc_q (&sem_heap[i].sem_wait_q, MAX_SEM_WAITQ,
		 SEM_Q, sizeof(char), i) ;
#ifdef CONFIG_NAMED_SEMA
	sem_map[i].sem = (sem_t)0;
	bzero ((void*)sem_map[i].name, SEM_NAME_MAX);
#endif
    }
}

sem_info_t* get_sem_by_semt (sem_t* sem)
{
    if (sem == NULL)
	return NULL;
    if ((*sem < MAX_SEM) && (sem_heap[(unsigned int)*sem].sem_id != -1) )
	return &sem_heap[(unsigned int)*sem];

    return NULL;
}

#ifdef CONFIG_NAMED_SEMA
sem_info_t* get_sem_by_name (char* name)
{
    int i;

    for (i=0; i<MAX_SEM; i++) {
	if (! strcmp(sem_map[i].name, name) &&  sem_heap[i].sem_id != -1)
	    return &sem_heap[i];
    }

    return NULL;
}

sem_t* get_semt_by_name (char* name)
{
    int i;

    for (i=0; i<MAX_SEM; i++) {
	if (! strcmp(sem_map[i].name, name) &&  sem_heap[i].sem_id != -1)
	    return &(sem_map[i].sem);
    }

    return NULL;
}
#endif /* CONFIG_NAMED_SEMA */

//----------------------------------------------------------------------------------------------------//
//  @func - sys_sem_init
//! @desc
//!   Create a semaphore
//!   - Allocate a semaphore structure
//!   - Initialize semaphore value
//! @param
//!   - sem is the location to store the created semaphore identifier
//!   - pshared indicates sharing semantics of this semaphore. Unused right now.
//!   - value is the initial value of the semaphore
//! @return
//!   - Returns 0 on success and created semaphore identifier in *sem
//!   - Returns -1 on error
//!     errno set to,
//!     ENOSPC - If the system is out of resources to allocate another semaphore
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
int sys_sem_init (sem_t* sem, int pshared, unsigned value)
{
    signed char i = 0 ;

    for (; i < MAX_SEM; i++) {
	if (sem_heap[i].sem_id == -1) {
	    sem_heap[i].sem_id = i;
	    sem_heap[i].sem_value = value;
	    sem_heap[i].unlink = 0;
	    qinit (&sem_heap[i].sem_wait_q);
	    if (pshared == 0)
		sem_heap[i].owner = current_pid;
	    else
		sem_heap[i].owner = -1;                                 // Any process context can access the semaphore

	    *sem = (sem_t)i;
	    return 0 ;
	}
    }

    kerrno = ENOSPC;
    return -1;
}

#ifdef CONFIG_NAMED_SEMA
//----------------------------------------------------------------------------------------------------//
//  @func - sys_sem_open
//! @desc
//!   Creates a mapping between a name and a semaphore
//!   - Check whether named semaphore already exists, if so return a pointer to corresponding sem_t.
//!   - If the semaphore exists and O_EXCL also specified, return error.
//!   - If the semaphore does not exist, create it and map name to the semaphore, return sem_t*.
//! @param
//!   - name is the name of the semaphore
//!   - oflag is the flag specifying open characteristics
//!   - mode is unused
//!   - value is the initial value of the semaphore
//! @return
//!   - Returns a pointer to a sem_t corresponding to newly created or previously existing semaphore
//!   - Returns SEM_FAILED on errors
//!     errno set to,
//!     ENOSPC - If the system is out of resources to create a new semaphore (or mapping)
//!     EEXIST if O_EXCL has been requested and the named semaphore already exists.
//!     EINVAL if the parameters are invalid.
//! @note
//!   - POSIX is vague about the behaviour of sem_open without O_CREAT in the flag. Therefore
//!     such oflags are not supported and SEM_FAILED returned.
//!   - Defined only if named semaphores are configured.
//----------------------------------------------------------------------------------------------------//
sem_t* sys_sem_open (const char* name, int oflag, mode_t mode, unsigned value)
{
    sem_t *sem;
    signed char i;

    if (name == NULL || oflag != O_CREAT) {
        kerrno = EINVAL;
	return (sem_t*)SEM_FAILED;
    }

    sem = get_semt_by_name ((char*)name);

    if (sem != NULL) {
	if (oflag & O_EXCL) {
            kerrno = EEXIST;
	    return (sem_t*)SEM_FAILED;
        }
	return sem;
    }

    for (i=0; i<MAX_SEM; i++) {                                         // No such named semaphore exists. Create one.
	if (sem_heap[i].sem_id == -1) {
	    sem_heap[i].sem_id = i;
	    sem_heap[i].sem_value = value;
	    sem_heap[i].unlink = 0;
	    qinit( &sem_heap[i].sem_wait_q);
	    sem_heap[i].owner = -1;                                     // Any process context can access the semaphore
	    strcpy (sem_map[i].name, name);                             // Setup the name mapping
	    sem_map[i].sem = (sem_t)i;
	    return &(sem_map[i].sem) ;
	}
    }

    kerrno = ENOSPC;
    return (sem_t*)SEM_FAILED;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_sem_close
//! @desc
//!   Close the semaphore referenced by sem
//! @param
//!   - sem is the reference to the semaphore
//! @return
//!   - 0 on success, -1 on error
//!     errno set to,
//!     EINVAL - If the semaphore identifier is invalid
//!     ENOTSUP - If the semaphore is currently locked and processes are blocked on the semaphore
//! @note
//!   - Defined only if named semaphores are configured.
//!   - Returns error if semaphore currently locked.
//----------------------------------------------------------------------------------------------------//
int sys_sem_close (sem_t* sem)
{
    sem_info_t *seminfo = get_sem_by_semt (sem);
    if (seminfo == NULL) {
        kerrno = EINVAL;
	return -1;
    }

    if (seminfo->sem_value <= 0 && (seminfo->sem_wait_q.item_count > 0)) { // POSIX does not define semantics of destroying a currently locked semaphore
        kerrno = ENOTSUP;                                                  // We indicate error on such an operation.
	return -1;
    }
    bzero ((void*) sem_map[(int)seminfo->sem_id].name, SEM_NAME_MAX);
    sem_map[(int)seminfo->sem_id].sem = (sem_t)0;

    seminfo->sem_id = -1;
    seminfo->sem_value = 0;
    seminfo->owner = -1;
    seminfo->unlink = 0;

    return 0 ;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_sem_unlink
//! @desc
//!   Unlink named semaphore
//!   - Check if named semaphore can be unlinked immediately
//!   - If yes, unlink, else indicate unlink required
//! @param
//!   - name is the name of the semaphore
//! @return
//!   - 0 on success, -1 on error
//!     errno set to,
//!     ENOENT - If an entry for name cannot be located
//! @note
//!   - Defined only if named semaphores are configured.
//----------------------------------------------------------------------------------------------------//
int sys_sem_unlink (const char* name)
{
    sem_info_t *seminfo = get_sem_by_name ((char*)name);
    if (seminfo == NULL) {
        kerrno = ENOENT;
	return -1;
    }

    if (seminfo->sem_value <= 0) {                                      // Some process locking the semaphore currently.
	seminfo->unlink = 1;                                            // Postpone unlink until all locks are released.
	return 0;
    }

    bzero ((void*) sem_map[(int)seminfo->sem_id].name, SEM_NAME_MAX);           // Else, unlink immediately
    sem_map[(int)seminfo->sem_id].sem = (sem_t)0;
    seminfo->sem_id = -1;
    seminfo->sem_value = 0;
    seminfo->owner = -1;
    seminfo->unlink = 0;

    return 0;
}
#endif  /* CONFIG_NAMED_SEMA */

//----------------------------------------------------------------------------------------------------//
//  @func - sys_sem_getvalue
//! @desc
//!   Get the current value of the semaphore
//! @param
//!   - sem is the semaphore reference
//!   - sval is the location to store the semaphore value in
//! @return
//!   - 0 on success, -1 on error.
//!     errno set to,
//!     EINVAL - If the semaphore identifier does not refer to a valid semaphore
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
int sys_sem_getvalue (sem_t* sem, int* sval)
{
    sem_info_t *seminfo = get_sem_by_semt (sem);
    if( seminfo == NULL) {
        kerrno = EINVAL;
	return -1;
    }

    *sval = seminfo->sem_value;
    return 0;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_sem_wait_x
//! @desc
//!   Semaphore wait operation
//!   - Decrement semaphore value
//!   - If value < 0, block
//! @param
//!   - sem is the semaphore reference
//! @return
//!   - 0 on success, -1 on failure
//!     errno set to,
//!     EINVAL - If the semaphore identifier does not refer to a valid semaphore
//!     EIDRM  - If the semaphore was removed forcibly
//! @note
//!   - Renamed away from sys_sem_wait because it conflicts with functions in LWIP.
//----------------------------------------------------------------------------------------------------//
int sys_sem_wait_x (sem_t* sem)
{
    sem_info_t* seminfo = get_sem_by_semt (sem);
    if (seminfo == NULL) {
        kerrno = EINVAL;
	return -1;
    }

    seminfo->sem_value-- ;                                              // Decrement the resource count
    if (seminfo->sem_value < 0)                                         // If resource unavailable
	process_block (&(seminfo->sem_wait_q), PROC_WAIT);

    // Return here on unblock

    // Special. Not part of posix specification. If the semaphore was force_destroy'ed
    // then the process has not really acquired the semaphore when it was unblocked,
    // but rather it is in an interrupted situation. Signal error in this case

    if (seminfo->sem_id == -1) {                                         // If sem invalidated by now
        kerrno = EIDRM;
	return -1;
    }

    return 0 ;
}

#ifdef CONFIG_TIME
//----------------------------------------------------------------------------------------------------//
//  @func - sys_sem_timedwait
//! @desc
//!   Semaphore timedwait operation
//!   - Decrement semaphore value
//!   - If value < 0, block with a timeout, as specified by the second parameters 'ms'
//! @param
//!   - sem is the semaphore reference
//!   - ms is the amount of time to be blocked on the semaphore in milliseconds
//! @return
//!   - 0 on success, -1 on failure
//!     errno set to,
//!     EINVAL - If the semaphore identifier does not refer to a valid semaphore
//!     ETIMEDOUT - The semaphore could not be locked before the specified timeout expired.
//!     EIDRM - If the semaphore was forcibly removed from the system
//! @note
//!   - Depends on CONFIG_TIME being true
//----------------------------------------------------------------------------------------------------//
int sys_sem_timedwait (sem_t* sem, unsigned int ms)
{
    sem_info_t* seminfo = get_sem_by_semt (sem);
    process_struct *self;

    if (seminfo == NULL) {
        kerrno = EINVAL;
	return -1;
    }

    seminfo->sem_value-- ;                                              // Decrement the resource count
    if (seminfo->sem_value < 0)  {                                      // If resource unavailable
        self = current_process;
        add_tmr (self->pid, ms);                                        // Add a timer for self
        process_block (&(seminfo->sem_wait_q), PROC_TIMED_WAIT);

                                                                        // Return here on unblock
        if (self->timeout) {                                            // Timeout during semaphore wait. Return with error
            seminfo->sem_value++;                                       // Restore sem value
            self->timeout = 0;
            kerrno = ETIMEDOUT;
            return -1;
        } else {
            remove_tmr (self->pid);                                     // We managed to acquire the semaphore. Remove associated timer
        }
    }

    // Special. Not part of posix specification. If the semaphore was force_destroy'ed
    // then the process has not really acquired the semaphore when it was unblocked,
    // but rather it is in an interrupted situation. Signal error in this case

    if (seminfo->sem_id == -1) {                                        // If sem invalidated by now
        kerrno = EIDRM;
	return -1;
    }

    return 0 ;
}
#endif /* CONFIG_TIME */

//----------------------------------------------------------------------------------------------------//
//  @func - sys_sem_trywait
//! @desc
//!   Try wait operation on a semaphore
//!   - Check if semaphore can be locked. If so, lock it, else return -1
//! @param
//!   - sem is the semaphore reference
//! @return
//!   - Return 0 on success, -1 on failure
//!     errno set to,
//!     EINVAL - The semaphore identifier does not refer to a valid semaphore
//!     EAGAIN - The semaphore could not be immediately locked by trywait
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
int sys_sem_trywait (sem_t *sem)
{
    sem_info_t *seminfo = get_sem_by_semt (sem);
    if (seminfo == NULL) {
        kerrno = EINVAL;
	return -1;
    }

    if (seminfo->sem_value > 0) {
	seminfo->sem_value-- ;
        return 0 ;
    }

    kerrno = EAGAIN;
    return -1;
}


//----------------------------------------------------------------------------------------------------//
//  @func - sys_sem_post
//! @desc
//!   Semaphore post operation
//!   - Increment semaphore value
//!   - If semaphore value <= 0, unblock a waiting process (depending on scheduling type)
//!   - If semaphore value > 0 and if unlink requested, destroy the semaphore
//! @param
//!   - sem is the semaphore reference
//! @return
//!   - Return 0 on success, -1 on failure
//!     errno set to,
//!     EINVAL - If the semaphore identifier does not refer to a valid semaphore
//! @note
//!   - process_unblock unblocks the highest prio process in the queue in case of SCHED_PRIO.
//!     Else, the first process in the queue is unblocked.
//----------------------------------------------------------------------------------------------------//
int sys_sem_post (sem_t* sem)
{
    sem_info_t *seminfo = get_sem_by_semt (sem);
    if (seminfo == NULL) {
        kerrno = EINVAL;
	return -1;
    }

    seminfo->sem_value++;
    if ((seminfo->sem_value <= 0) && (seminfo->sem_wait_q.item_count > 0))
	process_unblock (&(seminfo->sem_wait_q));
    else if ((seminfo->sem_value > 0) && (seminfo->unlink))             // Additionally, unlink the semaphore, if unlink is set
	sys_sem_destroy (sem);

    return 0 ;
}


//----------------------------------------------------------------------------------------------------//
//  @func - sys_sem_destroy
//! @desc
//!   Destroy a semaphore
//! @param
//!   - sem is the semaphore reference
//! @return
//!   - Return 0 on success, -1 on failure
//!     errno set to,
//!     EINVAL - If the semaphore identifier does not refer to a valid semaphore
//!     EBUSY - If the semaphore is currently locked and processes are blocked on it
//! @note
//!   - Returns error if semaphore currently locked.
//----------------------------------------------------------------------------------------------------//
int sys_sem_destroy (sem_t* sem)
{
    sem_info_t *seminfo = get_sem_by_semt (sem);
    if (seminfo == NULL) {
        kerrno = EINVAL;
	return -1;
    }

    if ((seminfo->sem_value <= 0) && (seminfo->sem_wait_q.item_count > 0)) {
                                                                // POSIX does not define semantics of destroying a semaphore that has blocked processes
        kerrno = EBUSY;                                         // We indicate error on such an operation
	return -1;
    }

    seminfo->sem_id = -1;
    seminfo->sem_value = 0;
    seminfo->owner = -1;
    seminfo->unlink = 0;
    return 0 ;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sem_force_destroy
//! @desc
//!   Destroy a semaphore and force waiting processes out of the semaphore
//! @param
//!   - sem is the semaphore reference
//! @return
//!   - Return 0 on success, -1 on failure
//! @note
//!   - This is not a system call and is for kernel use only.
//----------------------------------------------------------------------------------------------------//
int sem_force_destroy (sem_t* sem)
{
    sem_info_t *seminfo = get_sem_by_semt (sem);
    if (seminfo == NULL)
	return -1;

    while (seminfo->sem_value <= 0 && (seminfo->sem_wait_q.item_count > 0)) {                           // Unblock waiting processes and continue
	process_unblock (&seminfo->sem_wait_q);
	seminfo->sem_value++;
    }

    seminfo->sem_id = -1;
    seminfo->sem_value = 0;
    seminfo->owner = -1;
    seminfo->unlink = 0;
    return 0 ;
}
#endif	/* CONFIG_SEMA */
