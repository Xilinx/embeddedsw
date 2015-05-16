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
//! @file xtrace.h
//! Kernel level trace API
//----------------------------------------------------------------------------------------------------//

#ifndef _XTRACE_H
#define _XTRACE_H

#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/ktypes.h>
#include <sys/queue.h>

#ifdef __cplusplus
extern "C" {
#endif

enum xtrace_event {
    XSCHED_EVENT,
    XSEM_EVENT,
    XTIMER_EVENT,
    XCUSTOM_EVENT
};

enum xtrace_action {
    XSCHED_RESCHED,
    XSEM_WAIT,
    XSEM_TIMEWAIT,
    XSEM_WAIT_TIMEOUT,
    XSEM_WAIT_UNBLOCK,
    XSEM_ACQUIRE,
    XSEM_POST,
    XSEM_POST_UNBLOCK,
    XTIMER_TIMEOUT,
    XTIMER_TICK,
    XTIMER_ADD,
    XTIMER_REMOVE,
    XCUSTOM_ACTION1,
    XCUSTOM_ACTION2,
    XCUSTOM_ACTION3,
    XCUSTOM_ACTION4,
    XCUSTOM_ACTION5,
    XCUSTOM_ACTION6
};

typedef struct xtrace_pkt_s {
    enum xtrace_event   event;
    pid_t               pid;
    unsigned int        resource;
    enum xtrace_action  action;
    unsigned int        custom[2];
} xtrace_pkt_t;

void xtrace_log_event (enum xtrace_event event, enum xtrace_action action,
                       unsigned int resource, unsigned int custom0, unsigned int custom1);
void xtrace_print_log (void);


#if 0
#define CONFIG_XTRACE
#define CONFIG_XTRACE_MEM_START      0x9a000000
#define CONFIG_XTRACE_MAX_COUNT      100000
#endif


#ifndef CONFIG_XTRACE
#undef xtrace_log_event
#define xtrace_log_event(...) do { } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _XTRACE_H */
