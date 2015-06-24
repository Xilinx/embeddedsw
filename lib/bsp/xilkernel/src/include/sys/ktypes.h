/******************************************************************************
*
* Copyright (C) 2004 - 2014 Xilinx, Inc.  All rights reserved.
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
//! @file ktypes.h
//! Kernel equivalent of sys/types.h
//----------------------------------------------------------------------------------------------------//

#ifndef _KTYPES_H
#define _KTYPES_H

#include <os_config.h>
#include <sys/arch.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/msg.h>
#include <sys/shm.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SET_CURRENT_PROCESS(pid)  do {                                                         \
                                        current_pid = pid;                                     \
                                        current_process = (pid == -1) ? NULL:(&ptable[pid]);   \
                                  } while(0)                                                  
                                  
typedef struct _process_struct process_struct ;
typedef struct _process_context process_context ;

struct pthread_info_s;
// Queue Management Data Structures
typedef struct _queue *queuep ;

struct _queue {
    unsigned short item_size ;	//! Size of Queue item 
    unsigned char max_items ;	//! Max. number of items in Q 
    unsigned char item_count ;	//! Items in the Queue 
    unsigned char qfront ;	//! Queue front pointer 
    unsigned char qend ;	//! Queue end pointer 	
    void *items ;		//! Queue of item's 
};	

typedef struct reent_s {
    int _errno;
} reent_t;

//! Process Control Block - Information about the Process
struct _process_struct {
    process_context     pcontext;	//! Context switch store. Keep this at the top of the structure. Context switch relies on this.
    unsigned char 	state;		//! Process State 
    pid_t               pid;		//! Unique Process Identifier 
    signed char 	priority;	//! Process Priority 
    signed char 	is_allocated;	//! If the PCB has been allocated  
                                        //! 0 - Unallocated, 1 - Allocated 
    queuep              blockq;         //! Queue on which this process is currently blocked
#ifdef CONFIG_PTHREAD_SUPPORT 
    struct pthread_info_s* thread;      //! Pointer to the thread that is currently using this PCB. 
                                        //! Kludge ! Used because process and threads are essentially the same now.
#endif
#ifdef CONFIG_TIME  
    unsigned char       timeout;        //! Flag set if this process had a recent timeout
    unsigned int        remain;         //! Number of milliseconds of timeout remaining when this process was unblocked
#endif
#ifdef CONFIG_STATS
    unsigned int        active_ticks;   //! Number of quantums which this process has executed (approx)
#endif    
    reent_t             reent;          //! Re-entrancy information for this process
} __attribute__ ((aligned (4), packed));

//! Kernel pthread info structure
typedef struct pthread_info_s {
    char                        is_allocated;
    process_struct              *parent;                //! pointer back to base process structure
    pthread_t                   tid;                    //! ID of this thread.
    void                        *retval;                //! Return value of thread when it terminates.
    void*                       (*start_func)(void*);   //! Starting function
    void                        *param;                 //! Parameter to starting function
    unsigned char               state;                  //! Thread state 
    struct pthread_info_s       *join_thread;           //! Joining thread
    char                        joinq_mem;              //! Just a single byte of memory for the queue
    struct _queue               joinq;                  //! Queue of size 1.
    signed char                 mem_id ;                //! Mem ID of thread context
    pthread_attr_t              thread_attr;            //! Thread attributes
} pthread_info_t; 


//! Kernel mutex info structure
typedef struct pthread_mutexinfo_s {
    int                 is_allocated;    
    int                 locked;
    pthread_mutexattr_t attr;                   //! Mutex attributes
    struct _queue       mutex_wait_q ;          //! Queue of waiting threads
    pid_t               owner;                  //! ID of currently locking process
} pthread_mutexinfo_t;


//! Kernel semaphore info structure
typedef struct sem_info_s {
    signed char   sem_id;	//! Semaphore ID. -1 -> uninitialized
    signed char   sem_value;	//! Resource count 
    struct _queue sem_wait_q ;	//! Queue of waiting processes 
    pid_t         owner;        //! PID of process owning this semaphore. if -1 ,  
                                //! then any process can access this sem.
    unsigned char unlink;       //! Unlink status
} sem_info_t;

//! Semaphore symbolic name mapping table
typedef struct sem_map_s {
    char name[SEM_NAME_MAX];
    sem_t sem;
} sem_map_t;

//! Message queue buffer structure
typedef struct msg_s {
    char*  msg_buf;
    size_t msg_len;
} msg_t;

//! Kernel message queue info structure
typedef struct  msgid_s {
    int    msgid ;             //! Message Queue ID 
    key_t  key ;	       //! Key used to identify the MsgQ 
    //unsigned char msgsize ;  //! The size of the Message 
    unsigned char msgq_len ;   //! Message Queue depth
    sem_t full ;               //! Semaphore used by the producer 
    sem_t empty ;              //! Semaphore used by the consumer 
    struct _queue msg_q ;      //! Queue of Messages 
    struct msqid_ds stats;     //! Statistics about message queue
} msgid_ds;


//! Kernel shared memory info structure
typedef struct shm_info_s {
    size_t     shm_segsz;           //! Size of segment in bytes. 
    pid_t      shm_lpid;            //! Process ID of last shared memory operation. 
    pid_t      shm_cpid;            //! Process ID of creator. 
    shmatt_t   shm_nattch;          //! Number of current attaches. 
    void*      shm_addr;            //! Shared memory pointer
    int        shm_id;              //! ID of this SHM struct
    key_t      shm_key;             //! Associated key
} shm_info_t;

#ifdef __cplusplus
}       
#endif 

#endif // _KTYPES_H
