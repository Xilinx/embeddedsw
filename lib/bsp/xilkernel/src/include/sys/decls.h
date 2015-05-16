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
//! @file decls.h
//! Gathers declarations for kernel data structures in one place
//----------------------------------------------------------------------------------------------------//
#ifndef _SYS_DECLS_H
#define _SYS_DECLS_H

#ifdef __cplusplus
extern "C" {
#endif

extern char kernel_irq_stack[];
extern char process_startup_stack[];

#include <os_config.h>
#include <sys/arch.h>
#include <sys/types.h>
#include <sys/ktypes.h>

extern pid_t            current_pid, prev_pid, idle_task_pid;
extern process_struct   *current_process;
extern process_struct   ptable[];
extern reent_t          reent;
extern void             *_stack, __stack;
extern struct _queue    ready_q[];
extern void             *kernel_sp;
extern pid_t            idle_task_pid;

extern void _ftext __attribute__((weak)),
            _etext __attribute__((weak)),
            _fdata __attribute__((weak)),
            _edata __attribute__((weak)),
            _frodata __attribute__((weak)),
            _erodata __attribute__((weak)),
            _stack_end __attribute__((weak)),
            __stack __attribute__((weak));

extern void _fstack_guard_top __attribute__((weak)),
            _estack_guard_top __attribute__((weak)),
            _fstack_guard_bottom __attribute__((weak)),
            _estack_guard_bottom __attribute__((weak));

extern void             idle_task (void);

#define kerrno          (current_process->reent._errno)

#ifdef VERBOSE
#define DBG_PRINT(string) print(string)
#define DPUTNUM(num)      putnum(num)
#define DPRINTF           xil_printf
#else
#define DBG_PRINT(string) // nothing
#define DPUTNUM(num)      // nothing
#define DPRINTF(...)      // nothing
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SYS_DECLS_H */
