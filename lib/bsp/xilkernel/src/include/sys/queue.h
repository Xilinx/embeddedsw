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
//! @file queue.h
//! Queue management declarations and defines
//----------------------------------------------------------------------------------------------------//
#ifndef _QUEUE_H
#define _QUEUE_H

#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/ktypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//! Queue Types 
#define RUN_Q 	        1	//! Used to store Processes in PROC_RUN state 
#define READY_Q         2 	//! Used to store Processes in PROC_READY state 
#define SEM_Q	        3	//! Used for storing processes waiting for semaphore resource 
#define MSG_Q 	        4	//! Used for storing Messages in Message Q 
#define PTHREAD_JOIN_Q  5       //! Used for storing Threads waiting to join in a join Q 
#define PTHREAD_EXIT_Q  6       //! Used for storing Threads in state PTHREAD_STATE_EXIT
#define PTHREAD_MUTEX_Q 7       //! Used for storing processes waiting for mutex resource

// Function prototypes defined in sys/queue.c
void    alloc_q (queuep queue, unsigned char max_items, 
                 unsigned char qtype, unsigned short size, unsigned char qno);
void    qinit (queuep queue);
void    enq (queuep queue, const void *item, unsigned short key);
void    deq (queuep queue, void *item, unsigned short key);
int     pdelq (queuep queue, pid_t item);
void    pdeq (queuep queue, pid_t *item, unsigned short key);
void    penq (queuep queue, pid_t item, unsigned short key);
void    prio_penq (queuep queue, pid_t item, unsigned short key); 
void    prio_pdeq (queuep queue, pid_t *item, unsigned short key); 
int     prio_pdelq (queuep queue, pid_t item);

#ifdef __cplusplus
}       
#endif 

#endif  /* _QUEUE_H */
