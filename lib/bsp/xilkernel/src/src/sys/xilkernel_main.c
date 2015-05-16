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
//! @file xilkernel_main.c
//! Initialises the system by calling sys_init(), hw_init() and blocks the kernel
//! On first timer interrupt the first process gets scheduled.
//----------------------------------------------------------------------------------------------------//

#include <xmk.h>
#include <os_config.h>

#include <xil_exception.h>

#ifdef MB_XILKERNEL
#include <sys/process.h>
#endif

#ifdef PPC_XILKERNEL
#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/ktypes.h>
#include <xparameters.h>
#include <sys/process.h>
#include <xpseudo_asm.h>
#include <xtime_l.h>
#include <sys/syscall.h>
#endif /* PPC_XILKERNEL */
#include <sys/decls.h>
#include <sys/init.h>
#include <sys/mem.h>
#include <pthread.h>
#include <stdio.h>

//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//
#ifdef CONFIG_STATIC_ELF_PROCESS_SUPPORT
extern struct _process_init se_process_table[] ;
#endif

#ifdef CONFIG_PTHREAD_SUPPORT
extern pthread_attr_t default_attr;

#ifdef CONFIG_STATIC_PTHREAD_SUPPORT
extern struct _elf_pthread_init kb_pthread_table[];
#endif
#endif

extern void idle_task (void);
extern void init_idle_task (void);

//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//


//----------------------------------------------------------------------------------------------------//
//  @func - xilkernel_main
//! @desc
//!   Entry point of the kernel
//! @return
//!   - Nothing.
//! @note
//!   - Routine does not return. (Caller does not get back control)
//----------------------------------------------------------------------------------------------------//
void xilkernel_main(void)
{
    DBG_PRINT("XMK: Starting kernel.\r\n");

    xilkernel_init ();
    xilkernel_start ();
}

//----------------------------------------------------------------------------------------------------//
//  @func - xilkernel_init
//! @desc
//!   Initialize the system - This function is called at the start of system.
//!   It initializes the system.
//!   - Initializes the process vector table.
//!   - Creates the Idle process (pid - 0).
//!   - Creates the static set of processes.
//! @return
//!   - Nothing.
//----------------------------------------------------------------------------------------------------//
void xilkernel_init(void)
{
    unsigned int i = 0 ;

    DBG_PRINT("XMK: Initializing Hardware.\r\n");
    hw_init();                                                  // Do hardware specific initialization

    DBG_PRINT("XMK: System initialization.\r\n");
    for( ; i < MAX_PROCESS_CONTEXTS; i++ ) {
        ptable[i].is_allocated = 0 ;
        ptable[i].pcontext.isrflag = 0;
    }

#ifdef MB_XILKERNEL
    kernel_sp = (void*)((unsigned int)&_stack + SSTACK_PTR_ADJUST);
#elif defined(PPC_XILKERNEL)
    kernel_sp = (void*)((unsigned int)&__stack + SSTACK_PTR_ADJUST);
#endif
    readyq_init();

#ifdef CONFIG_PTHREAD_SUPPORT
    pthread_init();
#endif
#ifdef CONFIG_SEMA
    sem_heap_init();
#endif
#ifdef CONFIG_MSGQ
    msgq_init();
#endif
#ifdef CONFIG_SHM
    shm_init();
#endif
#ifdef CONFIG_BUFMALLOC
    bufmalloc_init ();
#endif

    init_idle_task ();

#ifdef CONFIG_STATIC_ELF_PROCESS_SUPPORT
    se_process_init() ;                                           // Create statically specified separate executable processes
#endif

#ifdef CONFIG_STATIC_PTHREAD_SUPPORT
    kb_pthread_init ();                                           // Create statically specified kernel bundled threads
#endif

#ifdef CONFIG_TIME
    soft_tmr_init ();
#endif
}

//----------------------------------------------------------------------------------------------------//
//  @func - xilkernel_start
//! @desc
//!   Start the kernel by enabling interrupts and starting to execute the idle task.
//! @return
//!   - Nothing.
//! @note
//!   - Routine does not return.
//! @desc
//----------------------------------------------------------------------------------------------------//
void xilkernel_start (void)
{
    DBG_PRINT("XMK: Process scheduling starts.\r\n");
    Xil_ExceptionEnable();
    idle_task ();                                                       // Does not return
}

#ifdef CONFIG_STATIC_ELF_PROCESS_SUPPORT
//----------------------------------------------------------------------------------------------------//
//  @func - se_process_init
//! @desc
//!   Create the statically specified set of separate executable processes.
//! @return
//!   - 0 on success
//!   - -1 on error
//! @note
//!   - Used only in the case of separate executable process support
//----------------------------------------------------------------------------------------------------//
int se_process_init(void)
{
    struct _process_init *pinit ;
    unsigned int i = 0 ;

    // Atleast a single process should be loaded during initialisation.
    pinit = se_process_table;
    for( i = 0; i < N_INIT_PROCESS; i++) {
	if (sys_elf_process_create((void*)pinit->p_start_addr, pinit->priority) < 0) {
	    DBG_PRINT ("XMK: se_process_init: sys_process_create failed.\r\n");
	    return -1;
	}
	pinit++ ;
    }
    return 0;
}
#endif /* CONFIG_STATIC_ELF_PROCESS_SUPPORT */

#ifdef CONFIG_PTHREAD_SUPPORT
#ifdef CONFIG_STATIC_PTHREAD_SUPPORT
//----------------------------------------------------------------------------------------------------//
//  @func - kb_pthread_init
//! @desc
//!   Create the statically specified pthreads that do not have an ELF file associated with it.
//!   Threads in kernel space. Stack allocated from BSS memory pool.
//! @return
//!   - 0 on success
//!   - error code from sys_pthread_create
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
int kb_pthread_init(void)
{
    struct _elf_pthread_init *pinit ;
    unsigned int i = 0 ;
    pthread_t tid;
    pthread_attr_t attr = default_attr;
    int ret;

    // Load the system processes to run during init.
    pinit = kb_pthread_table;

    for( i=0; i<N_INIT_SELF_PTHREADS; i++) {
#if SCHED_TYPE == SCHED_PRIO
	attr.schedparam.sched_priority = pinit->priority;
#endif
	ret = sys_pthread_create (&tid, &attr, (void*)pinit->start_func, NULL);
	if (ret != 0) {
	    DBG_PRINT("XMK: kb_pthread_init: sys_pthread_create failed.\r\n");
	    return -1;
	}
	pinit++;
    }

    return 0;
}
#endif /* CONFIG_STATIC_PTHREAD_SUPPORT */


int xmk_add_static_thread(void* (*start_routine)(void *), int sched_priority)
{
    pthread_t tid;
    pthread_attr_t attr = default_attr;
    int ret;

#if SCHED_TYPE == SCHED_PRIO
    attr.schedparam.sched_priority = sched_priority;
#endif
    sched_priority = 0; /* Dummy to remove compilation issues */

    ret = sys_pthread_create (&tid, &attr, start_routine, NULL);
    if (ret != 0) {
        DBG_PRINT("XMK: xmk_add_static_thread: sys_pthread_create failed.\r\n");
        return -1;
    }

    return 0;
}

#endif /* CONFIG_PTHREAD_SUPPORT */
