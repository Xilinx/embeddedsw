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
//! @file elf.c
//! ELF file related routines (Creating processes out of ELF files)
//----------------------------------------------------------------------------------------------------//

#include <stdio.h>
#include <os_config.h>
#include <sys/init.h>
#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/arch.h>
#include <sys/ktypes.h>
#include <sys/ksched.h>
#include <sys/process.h>
#include <sys/mem.h>
#include <sys/queue.h>
#include <sys/ksemaphore.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/decls.h>
#include <sys/stats.h>

#ifdef CONFIG_ELF_PROCESS

#ifdef CONFIG_PTHREAD_SUPPORT
#include <sys/kpthread.h>
#endif

//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------//

//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//
extern process_struct ptable[] ;
extern signed char current_pid;
extern char process_startup_stack[];

//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//

//----------------------------------------------------------------------------------------------------//
//  @func - sys_elf_process_create
//! @desc
//!   Process creation.
//!   - Reserves a pid for the process.
//!   - Initilaizes the process structure. Also loads the r15 with the start
//!     address of the process.
//!   - Places the process in the READY_Q.
//!   - If SCHED_PRIO, calls the process_scheduler for scheduling.
//!
//! @param
//!   - pstart_addr is the start address of the process
//!   - priority is the priority of the process
//! @return
//!   - PID of the new process.
//!   - -1 on Error. Max. process exceeded.
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
pid_t sys_elf_process_create (void* pstart_addr, unsigned int priority)
{
    pid_t pid;
    process_struct *pcb;
    unsigned int stackaddr;

    pid = proc_create (priority);

    if (pid == -1)
        return -1;

    pcb = &ptable[pid];

    stackaddr = ((unsigned int)process_startup_stack                    // Temporary stack for new launched process in case it is interrupted
                 + PROCESS_STARTUP_STACKSZ + SSTACK_PTR_ADJUST);        // before its own stack is setup

    setup_initial_context (&ptable[pid], current_pid, pstart_addr, stackaddr, 0);

    return pid;
}


//----------------------------------------------------------------------------------------------------//
//  @func - sys_elf_exit
//! @desc
//!   Remove the process entry from the process table.
//!   - Set the is_allocated flag to '0'. Set the current_pid to -1. so that the
//!     next process gets scheduled.
//! @return
//!   - 0 on Success
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
int sys_elf_exit()
{
    process_invalidate (&ptable[current_pid]);
    suspend ();
    return 0 ;
}

#endif /* CONFIG_ELF_PROCESS */
