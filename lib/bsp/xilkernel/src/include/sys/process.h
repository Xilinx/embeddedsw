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
//! @file process.h
//! Process management declarations and definitions
//----------------------------------------------------------------------------------------------------//
#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/queue.h>
#include <sys/ktypes.h>
#include <sys/stats.h>
#include <sys/kpthread.h>

#ifdef __cplusplus
extern "C" {
#endif

//! Process States.
#define PROC_NEW                0   //! Process is new and has not been scheduled yet
#define PROC_RUN                1   //! Process is currently executing
#define PROC_READY              2   //! Process that is ready and considered for scheduling
#define PROC_WAIT               3   //! Process is waiting for a resource. Is out of ready queues and in some wait queue
#define PROC_TIMED_WAIT         4   //! Process is waiting for a resource with a timeout. Is out of ready queues and in some wait queue
#define PROC_DELAY              5   //! Process is waiting for a timeout
#define PROC_DEAD               6   //! State of process that has called exit and is now "dead"

//----------------------------------------------------------------------------------------------------//
// Function prototypes - defined in sys/process.h
//----------------------------------------------------------------------------------------------------//
// Internal Kernel functions. These routines are not directly called from any
// process. They don't have reentrant code.

void xmk_enter_kernel (void);
void xmk_leave_kernel (void);
void process_block (queuep queue, unsigned int state) ;
void process_unblock( queuep queue ) ;
pid_t proc_create (unsigned int priority);
int  process_invalidate (process_struct *proc);

pid_t   sys_elf_process_create (void* pstart_addr, unsigned int priority);
int     sys_elf_exit (void);
int     sys_kill (pid_t pid);
int     sys_process_status (pid_t pid, p_stat *ps);
int     sys_yield(void);
pid_t   sys_get_currentPID(void);
int     sys_get_kernel_stats (kstats_t *stats);


pid_t   get_currentPID(void);
int     kill (pid_t pid);
int     process_status (pid_t pid, p_stat *ps);
int     yield(void);

#ifdef __cplusplus
}
#endif

#endif /* _PROCESS_H_ */
