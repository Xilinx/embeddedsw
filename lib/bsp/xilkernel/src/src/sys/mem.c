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
//! @file mem.c
//! This contains the Memory allocation routines.
//! - All the memory initialization routines for various system modules is
//!   defined here. The memory for all the modules are allocated statically,
//!   the size of which can be configured by the user. This can be specified
//!   in structures defined in sys/init.c. Based on this the total memory
//!   requirements for each module is calculated.
//! - This also contains routines that allocate thread context and
//!   buffer management.
//----------------------------------------------------------------------------------------------------//

#include <stdio.h>
#include <os_config.h>
#include <config/config_cparam.h>
#include <config/config_param.h>
#include <sys/arch.h>
#include <sys/process.h>
#include <sys/mem.h>
#include <sys/init.h>
#include <sys/shm.h>
#include <sys/kmsg.h>
#include <sys/kpthread.h>
#include <sys/ksched.h>
#include <sys/decls.h>
#include <sys/bufmalloc.h>
#include <errno.h>

//! Memory allocated for storing Process IDs. This memory is used for Ready
//! Queue and Semaphore Queue. The memory is split as follows:
//! -# Memory for Ready Queue's.
//! -# Memory for Semaphore Queue's.
pid_t pid_queue_mem[PID_QUEUE_MSIZE] ;
extern process_struct ptable[MAX_PROCESS_CONTEXTS] ;

#ifdef CONFIG_PTHREAD_SUPPORT
//! Total Memory allocated for thread context. This depends on the number of
//! threads in the system and size of the thread context. This is user
//! customizable.

char thread_stack_mem[PTHREAD_STACK_MSIZE] ;

//! Array of table used to keep track of Memory ID for the allocated thread
//! context.
//! 0 - unallocated
//! 1 - allocated

char thread_stack_memid[MAX_PTHREADS] ;
#endif

#ifdef CONFIG_PTHREAD_MUTEX
pid_t pthread_mutex_queue_mem[PTHREAD_MUTEX_QUEUE_MSIZE];
#endif

#ifdef CONFIG_MSGQ
//! Memory allocated for storing Message pointers in Message Queues.
//! The total memory is user customizable.
//! Each msg_t structure contains a pointer to the actual message and the size of the message
#define MSG_QUEUE_MSIZE (NUM_MSGQS * MSGQ_CAPACITY * sizeof(msg_t))
char msg_queue_mem[MSG_QUEUE_MSIZE] ;
extern msgid_ds msgq_heap[] ;
#endif /* CONFIG_MSGQ */

#ifdef CONFIG_SHM

//! Memory allocated for Shared Memory modules. The total memory is user
//! customizable.

char shm_mem[SHM_MSIZE] ;
extern struct _shm_init shm_config[] ;
extern shm_info_t shm_heap[] ;
#endif


#ifdef CONFIG_BUFMALLOC
membuf_t        smbufs[N_STATIC_BUFS];
//! Total memory allocated for buffer management. The memory is divided among
//! different memory sized blocks based on the user configuration.
char bufmallocmem[BUFMALLOC_MSIZE] ;
extern struct bufmalloc_init_s bufmalloc_cfg[];
#endif /* CONFIG_BUFMALLOC */

//----------------------------------------------------------------------------------------------------//
//  @func - alloc_pidq_mem
//! @desc
//!   Allocate memory to the process ID queue's.
//!   - This includes the Ready_Q and Semaphore_Q. Initial memory is allocated
//!     for Ready queue, followed by the semaphore queue.
//! @param
//!  - queue is the queue of PIDs
//!  - qtype is the queue type. Can be READY_Q or SEMA_Q
//!  - qno is the Queue number. (eg) There are N_PRIO number of
//!    Ready Queue. The qno denotes this number. Memory for
//!    queues is allocated sequentially.
//! @return
//!   - None
//! @note
//!   - SEMA_Q is valid only if CONFIG_SEMA is defined.
//----------------------------------------------------------------------------------------------------//
void alloc_pidq_mem (queuep queue, unsigned int qtype, unsigned int qno)
{
    if (qtype == READY_Q) {
	queue->items = pid_queue_mem + (qno * MAX_READYQ * sizeof (pid_t)) ;
    }
#ifdef CONFIG_SEMA
    else if (qtype == SEM_Q) {
	queue->items = (pid_queue_mem + SEMQ_START) + (qno * MAX_SEM_WAITQ * sizeof (pid_t));
    }
#endif
#ifdef CONFIG_PTHREAD_MUTEX
    else if (qtype == PTHREAD_MUTEX_Q) {
	queue->items = pthread_mutex_queue_mem + (qno * MAX_PTHREAD_MUTEX_WAITQ * sizeof (pid_t));
    }
#endif
}

#ifdef CONFIG_MSGQ
//----------------------------------------------------------------------------------------------------//
//  @func - alloc_msgq_mem
//! @desc
//!   Allocate memory to the Message Queues from statically allocated memory pool.
//! @param
//!   - queue is the queue of PIDs
//!   - qno is the Queue number.
//! @return
//!   - None
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void alloc_msgq_mem( queuep queue, unsigned int qno )
{
    // It is always called in Sequence during system initialization !!
    static unsigned int msgq_start_ptr = 0 ;

    queue->items = (msg_queue_mem + msgq_start_ptr) ;
    msgq_start_ptr += (sizeof(msg_t) * MSGQ_CAPACITY);
}

//----------------------------------------------------------------------------------------------------//
//  @func - msgq_init
//! @desc
//!   The message Queue structures are initialized.
//!   - Based on the system configuration the message queues are created and
//!     defaults assigned to them.
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void msgq_init(void)
{
    unsigned int i = 0 ;
    for( ; i < NUM_MSGQS; i++ ){
	msgq_heap[i].msgid = -1;
	msgq_heap[i].msgq_len = MSGQ_CAPACITY;
	msgq_heap[i].stats.msg_qbytes = MSGQ_MAX_BYTES;
	msgq_heap[i].stats.msg_lspid = -1;
	msgq_heap[i].stats.msg_lrpid = -1;
	// Allocate space in the queue to contain msgq_len number of pointers
	alloc_q (&msgq_heap[i].msg_q, MSGQ_CAPACITY, MSG_Q, sizeof(msg_t),i);
    }
}
#endif	/* CONFIG_MSGQ */


#ifdef CONFIG_SHM
//----------------------------------------------------------------------------------------------------//
//  @func - shm_init
//! @desc
//!   The shared memory structures are initialized based on the system configuration.
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void shm_init(void)
{
    unsigned int i = 0 ;
    unsigned int offset = 0 ;

    for( ; i < N_SHM; i++ ) {
	shm_heap[i].shm_id = -1;
	shm_heap[i].shm_segsz = shm_config[i].shm_size ;
	shm_heap[i].shm_addr = (char *)(shm_mem+ offset) ;
	offset += shm_config[i].shm_size;
    }
}
#endif 	/* CONFIG_SHM */

#ifdef CONFIG_PTHREAD_SUPPORT
//----------------------------------------------------------------------------------------------------//
//  @func - bss_mem_init
//! @desc
//!   Initialize the bss memory allocation array. Used for thread stack allocation
//! @return
//!   - Nothing
//! @note
//!   - Included only if pthread support is required
//----------------------------------------------------------------------------------------------------//
void bss_mem_init (void)
{
    unsigned int i ;
    for(i = 0; i < MAX_PTHREADS; i++)
	thread_stack_memid[i] = 0 ;
}
//----------------------------------------------------------------------------------------------------//
//  @func - alloc_bss_mem
//! @desc
//!   Allocate memory for the bss memory, from the memory pool.
//! @param
//!   - start is the start address of the bss memory
//!   - end is the end address of the bss memory
//! @return
//!   -	On Success, start and end are assigned the start and end address of
//! 	bss memory respectively. Mem ID is returned.
//!   - -1 on Error
//! @note
//!   - Included only if pthread support is required
//----------------------------------------------------------------------------------------------------//
int alloc_bss_mem (unsigned int *start, unsigned int *end)
{
    unsigned int i ;

    for(i = 0; i < MAX_PTHREADS; i++ ){
	if( thread_stack_memid[i] == 0 ){
	    /* Need to align along the boundary */
	    *start = (unsigned int)thread_stack_mem + (PTHREAD_STACK_SIZE*i) ;
	    *end = (*start + PTHREAD_STACK_SIZE) ;
	    thread_stack_memid[i] = 1 ;
	    return i ;
	}
    }

    return -1 ;
}

//----------------------------------------------------------------------------------------------------//
//  @func - free_bss_mem
//! @desc
//!   Free the bss memory allocated for the thread.
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void free_bss_mem (unsigned int memid)
{
    thread_stack_memid[memid] = 0 ;
}
#endif		/* CONFIG_PTHREAD_SUPPORT */


#ifdef CONFIG_BUFMALLOC
//----------------------------------------------------------------------------------------------------//
//  @func - bufmalloc_mem_init
//! @desc
//!   Create buffer memory pools for statically specified mem configurations
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void bufmalloc_mem_init (void)
{
    int i;
    void *ptr = bufmallocmem;
    unsigned int bufsiz;

    for (i = 0; i < N_STATIC_BUFS; i++) {
        bufsiz = bufmalloc_cfg[i].bsiz * bufmalloc_cfg[i].nblks;
        if (sys_bufcreate (&smbufs[i], ptr, bufmalloc_cfg[i].nblks, bufmalloc_cfg[i].bsiz) != 0) {
            DBG_PRINT ("XMK: Error while initializing statically allocated block memory resources.\r\n");
            return;
        }
        ptr = ptr + bufsiz;
    }
}

#endif /* CONFIG_BUFMALLOC */
