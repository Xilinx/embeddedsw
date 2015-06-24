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
//! @file config_cparam.h
//! This contains the configuration parameter's for Message Queue, 
//! Shared Memory and Dynamic Buffer mgmt routines. The following fields are 
//! defined  based on the values in sys/init.c
//----------------------------------------------------------------------------------------------------//
#ifndef CONFIG_CPARAM_H
#define CONFIG_CPARAM_H

#include <os_config.h>
#include <config/config_param.h>

/************************************************************************/
/* Memory sizes for the various memory blocks			        */
/************************************************************************/

// The total Memory needed for all PID queue's in the system. This includes
// ready queue and semaphore wait queues.

#ifdef CONFIG_SEMA
#define PID_QUEUE_MSIZE  ((N_PRIO*MAX_READYQ)+(MAX_SEM*MAX_SEM_WAITQ))
#else
#define PID_QUEUE_MSIZE  (N_PRIO*MAX_READYQ)
#endif

#ifdef CONFIG_PTHREAD_MUTEX
#define PTHREAD_MUTEX_QUEUE_MSIZE (MAX_PTHREAD_MUTEX * MAX_PTHREAD_MUTEX_WAITQ)
#endif

#ifdef CONFIG_PTHREAD_SUPPORT
// The total memory needed for all the thread context. Calculated based on
// definition in config_param.h
#define PTHREAD_STACK_MSIZE (MAX_PTHREADS * PTHREAD_STACK_SIZE)
#endif

#ifdef CONFIG_SHM
// Total Memory size for the various Shared Memory
// This is how it is calculated from (struct _shm_init):	
// -# Add msize of all the shared memory to get the total msize
#ifndef SHM_MSIZE 
#define SHM_MSIZE 	 100
#endif
#endif

#ifdef CONFIG_MALLOC
// Total Memory size for the Dynamic buffer management	
// This is how it is calculated from (struct _malloc_init):
//  -#	msize for a single memory block = mem_bsize * n_blocks
//  -#	Add msize of all the Memory blocks to get the total msize
 
#ifndef MALLOC_MSIZE 
#define MALLOC_MSIZE 	 120
#endif
#endif

/************************************************************************/
/* Maximum number of various elements - See config_init.h		*/
/************************************************************************/
#ifndef N_INIT_PROCESS 
#define N_INIT_PROCESS 	0	
#endif

#ifndef N_INIT_SELF_PTHREADS
#define N_INIT_SELF_PTHREADS   0
#endif
#ifndef N_INIT_MELF_PTHREADS
#define N_INIT_MELF_PTHREADS   0
#endif

#ifdef CONFIG_SHM
#ifndef N_SHM 
#define N_SHM		1	
#endif
#endif

#ifdef CONFIG_MALLOC
#ifndef N_MALLOC_BLOCKS 
#define N_MALLOC_BLOCKS	2	
#endif
#ifndef TOT_MALLOC_BLOCKS 
#define TOT_MALLOC_BLOCKS 20	// Total number of memory blocks in the 
				// system. This is:			
				//  - Sum of n_blocks field of all elements  in malloc_config[]	 
#endif
#endif

#endif
