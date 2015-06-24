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
//! @file xtrace.c
//!
//! XMK API for tracing kernel level and custom user events. 
//! Requires trace memory in the system accessible to program.
//! @note - This feature is for internal use only.
//----------------------------------------------------------------------------------------------------//
#include <os_config.h>
#include <sys/types.h>
#include <sys/decls.h>
#include <sys/xtrace.h>

#ifdef CONFIG_XTRACE

//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//
const char *xtrace_event_name[] = {
    "SCHED",
    "SEM",
    "TIMER",
    "CUSTOM"
};

const char *xtrace_action_name[] = {
    "resched",
    "wait",
    "timewait",
    "wait_timeout",
    "wait_unblock",
    "acquire",
    "post",
    "post_unblock",
    "timeout",
    "tick",
    "add",
    "remove",
    "action1",
    "action2",
    "action3",
    "action4",
    "action5",
    "action6"
};

extern pid_t current_pid;

//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------// 
unsigned int    xtrace_first, xtrace_last, xtrace_count;
xtrace_pkt_t    *xtracebuf    = (xtrace_pkt_t *)(CONFIG_XTRACE_MEM_START + 8);
unsigned int    xtrace_off;

//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//

//----------------------------------------------------------------------------------------------------//
//  @func - xtrace_log_event 
//!
//! 
//! @param
//!   - 
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void xtrace_log_event (enum xtrace_event event, enum xtrace_action action, 
                       unsigned int resource, unsigned int custom0, unsigned int custom1)
{
    if (xtrace_off)
        return;

    xtracebuf[xtrace_last].event = event;
    xtracebuf[xtrace_last].pid   = current_pid;
    xtracebuf[xtrace_last].resource = resource;
    xtracebuf[xtrace_last].action = action;
    xtracebuf[xtrace_last].custom[0] = custom0;
    xtracebuf[xtrace_last].custom[1] = custom1;
    
    if (xtrace_count != CONFIG_XTRACE_MAX_COUNT)
        xtrace_count++;
    
    xtrace_last++;
    if (xtrace_last == CONFIG_XTRACE_MAX_COUNT) 
        xtrace_last = 0;
    
    if (xtrace_last == xtrace_first) {
        xtrace_first++;
        if (xtrace_first == CONFIG_XTRACE_MAX_COUNT)
            xtrace_first = 0;
    } 
}

//----------------------------------------------------------------------------------------------------//
//  @func - xtrace_print_log 
//!
//! 
//! @param
//!   - 
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void xtrace_print_log (int count, int last)
{
    int i, start, end;
    
    DPRINTF ("XTrace Log\r\n");
    DPRINTF ("\tTotal events recorded: %d.\r\n", xtrace_count);

    if (count == 0)
        count = xtrace_count;
    
    if (last) {
        start = (xtrace_last - count - 1);
        if (start < 0)
            start = (CONFIG_XTRACE_MAX_COUNT + start);
    } else 
        start = xtrace_first;
    
    end = (start + count) % CONFIG_XTRACE_MAX_COUNT;
    
    DPRINTF ("\tDumping (%d) %s events next --\r\n", count, (last ? "trailing" : "beginning"));
    
    i = start;
    while (i != end) {
        if (xtracebuf[i].event >= XCUSTOM_EVENT) {
            DPRINTF ("E%04d: [%6s]-(%15s) PID(%03d) on (%03d). custom - [0x%x], [0x%x]\r\n",
                     i,
                     xtrace_event_name[xtracebuf[i].event],
                     xtrace_action_name[xtracebuf[i].action],
                     xtracebuf[i].pid,
                     xtracebuf[i].resource,
                     xtracebuf[i].custom[0],
                     xtracebuf[i].custom[1]);
        } else {
            DPRINTF ("E%04d: [%6s]-(%15s) PID(%03d) on (%03d).\r\n",
                     i,
                     xtrace_event_name[xtracebuf[i].event],
                     xtrace_action_name[xtracebuf[i].action],
                     xtracebuf[i].pid,
                     xtracebuf[i].resource);
        }
        
        if (++i == CONFIG_XTRACE_MAX_COUNT)
            i = 0;
    }
    
    while (1);
}
    
#endif /* CONFIG_XTRACE */

