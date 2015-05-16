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
//! @file libprocess.c
//! This contains system call wrapper functions for Process Management.
//----------------------------------------------------------------------------------------------------//
#include <os_config.h>
#include <sys/process.h>
#include <sys/syscall.h>
#include <sys/stats.h>
#include <sys/ktypes.h>

reent_t  *lreent = NULL;

extern void* make_syscall (void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, int syscall_num);

reent_t* get_reentrancy (void)
{
    return (reent_t*) make_syscall (NULL, NULL, NULL, NULL, NULL, SC_GET_REENTRANCY);
}

#ifdef CONFIG_ELF_PROCESS
pid_t elf_process_create (void* start, int prio)
{
    return (int) make_syscall (start, (void*)prio, NULL, NULL, NULL, SC_PROCESS_CREATE);
}

int elf_process_exit (void)
{
    return (int) make_syscall ( NULL, NULL, NULL, NULL, NULL, SC_PROCESS_EXIT);
}
#endif

#ifdef CONFIG_KILL
int kill (pid_t pid)
{
    return (int) make_syscall ((void*)(int)pid, NULL, NULL, NULL, NULL, SC_PROCESS_KILL);
}
#endif

int process_status (pid_t pid, p_stat *ps)
{
    return (int) make_syscall ((void*)(int)pid, (void*)ps, NULL, NULL, NULL, SC_PROCESS_STATUS);
}

pid_t get_currentPID (void)
{
    return (int) make_syscall (NULL, NULL, NULL, NULL, NULL, SC_PROCESS_GETPID);
}

#ifdef CONFIG_YIELD
int yield (void)
{
    return (int) make_syscall (NULL, NULL, NULL, NULL, NULL, SC_PROCESS_YIELD);
}
#endif /* CONFIG_YIELD */

#ifdef CONFIG_STATS
int get_kernel_stats (kstats_t *stats)
{
    return (int) make_syscall ((void*)stats, NULL, NULL, NULL, NULL, SC_GET_KERNEL_STATS);
}
#endif


int*   __errno (void)
{
    if (lreent == NULL)
        lreent = get_reentrancy ();

    return &(lreent->_errno);
}
