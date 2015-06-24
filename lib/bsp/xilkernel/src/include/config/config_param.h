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
//! @file config_param.h 
//! This file contains configuration parameter's for process Management, 
//! thread Management and semaphore routines.
//----------------------------------------------------------------------------------------------------//

#ifndef _CONFIG_PARAM_H
#define _CONFIG_PARAM_H

#include <os_config.h>
#include <sys/ksched.h>

#if SCHED_TYPE == SCHED_RR 
#ifdef N_PRIO             
#undef N_PRIO
#endif
#define N_PRIO 1 	// N_PRIO always 1 for SCHED_RR 
#define PRIO_HIGHEST 0 // Highest priority 
#define PRIO_LOWEST  0 // Lowest  priority 
#endif /* SCHED_TYPE == SCHED_RR */

#if SCHED_TYPE == SCHED_PRIO 	// SCHED_PRIO 
#ifndef N_PRIO                  // was not defined in os_config.h 
#define N_PRIO 32	        // Number of priority levels 
#define PRIO_HIGHEST  0         // Highest priority 
#define PRIO_LOWEST  31	        // Least Priority 
#else                           
#define PRIO_HIGHEST 0
#define PRIO_LOWEST (N_PRIO-1)
#endif
#endif	/* SCHED_TYPE == SCHED_PRIO */


/************************************************************************/
/*	InterProcess Communication options				*/
/************************************************************************/

// Semaphore Specific Configs 
#ifdef CONFIG_SEMA

#ifndef MAX_SEM 
#define MAX_SEM 20		// Max Semaphore count 
#endif
#ifndef MAX_SEM_WAITQ 
#define MAX_SEM_WAITQ 10	// Max semaphore wait Q length 
#endif
#endif 	/* CONFIG_SEMA */


#ifdef CONFIG_MSGQ 
#ifndef CONFIG_SEMA
#error "Message queues require semaphores. Please define CONFIG_SEMA"
#endif
#ifndef NUM_MSGQS 
#define NUM_MSGQS     10
#endif
#ifndef MSGQ_CAPACITY 
#define MSGQ_CAPACITY 10
#endif
#endif 	/* CONFIG_MSGQ */

#ifdef CONFIG_PTHREAD_MUTEX
#ifndef CONFIG_PTHREAD_SUPPORT
#error "Pthread mutex requires pthread support. Please define CONFIG_PTHREAD_SUPPORT"
#endif
#ifndef MAX_PTHREAD_MUTEX 
#define MAX_PTHREAD_MUTEX  5
#endif

#ifndef MAX_PTHREAD_MUTEX_WAITQ      
#define MAX_PTHREAD_MUTEX_WAITQ 10  // Max pthread_mutex wait Q length 
#endif
#endif

/************************************************************************/
/* Memory Management related options.					*/
/************************************************************************/

#ifdef CONFIG_PTHREAD_SUPPORT
#ifndef MAX_PTHREADS          
#define MAX_PTHREADS	5	// Max number of threads
#endif

#ifndef PTHREAD_STACK_SIZE 
#define PTHREAD_STACK_SIZE	600	// Should be a multiple of 4
					// for word alignment	
#endif
#endif	/* CONFIG_PTHREAD_SUPPORT */

#ifdef CONFIG_ELF_PROCESS
#define MAX_PROCESS_CONTEXTS (MAX_PROCS + MAX_PTHREADS)
#else
#ifdef CONFIG_PTHREAD_SUPPORT
#define MAX_PROCESS_CONTEXTS (MAX_PTHREADS)
#else
#define MAX_PROCESS_CONTEXTS 0
#endif
#endif  /* CONFIG_PTHREAD_SUPPORT */

#endif	/* _CONFIG_PARAM_H */
