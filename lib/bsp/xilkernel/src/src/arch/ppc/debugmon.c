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
//! @file debugmon.c
//! Kernel inbuilt debug monitor routines for PPC405
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

#ifdef CONFIG_DEBUGMON

void debugmon_dump_proc_info (void)
{
    int i, j;

    xmk_enter_kernel ();
    DBG_PRINT ("kernel_dump_proc_info ----> \r\n");
    for (i = 0; i<MAX_PROCESS_CONTEXTS; i++) {
        if (!ptable[i].is_allocated)
            continue;

        DBG_PRINT ("=============================>\r\n");
        DBG_PRINT ("pid: ");
        putnum (ptable[i].pid);
        DBG_PRINT ("\r\nstate: ");
        putnum (ptable[i].state);
        DBG_PRINT ("\r\nisrflag: ");
        putnum (ptable[i].pcontext.isrflag);

        for (j=0; j<44; j++) {
            DBG_PRINT ("\r\nregs[ ");
            putnum (j);
            DBG_PRINT ("]: ");
            putnum (ptable[i].pcontext.regs[j]);
        }
        DBG_PRINT ("\r\n=============================>\r\n\r\n");
    }

    while (1);
}

void debugmon_stack_check (void)
{

}

void debugmon_dump_sched_info (void)
{

}
#endif /* CONFIG_DEBUGMON */
