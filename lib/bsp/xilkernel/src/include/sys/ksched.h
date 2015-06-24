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
//! @file ksched.h
//! Kernel level scheduling definitions and declarations
//----------------------------------------------------------------------------------------------------//

#ifndef _KSCHED_H
#define _KSCHED_H

#include <os_config.h>
#include <sys/ktypes.h>
#include <sys/sched.h>

#ifdef __cplusplus
extern "C" {
#endif

// Scheduling algorithm.
#ifndef SCHED_OTHER
#define SCHED_OTHER     0    // Other 
#endif
#ifndef SCHED_FIFO
#define SCHED_FIFO      1    // FIFO scheduling 
#endif
#ifndef SCHED_RR
#define SCHED_RR 	2    // Round Robin Scheduling type 
#endif

// This will go away in the future
#define SCHED_PRIO 	3    // Priority Preemptive Scheduling type 


void readyq_init(void) ;
void process_scheduler(void) ;
void process_scheduler_and_switch (void);
void suspend (void);

#if SCHED_TYPE == SCHED_RR
void sched_rr (void);
#elif SCHED_TYPE == SCHED_PRIO
void sched_prio(void);
#endif


#ifdef MB_XILKERNEL
#define XMK_CONTEXT_SWITCH(prevpid)  microblaze_context_switch(prevpid)
#else // PPC
#define XMK_CONTEXT_SWITCH(prevpid)  ppc_context_switch(prevpid)
#endif // MB_XILKERNEL

#ifdef __cplusplus
}       
#endif 

#endif /* _KSCHED_H */
