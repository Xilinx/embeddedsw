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
//! @file timer.h
//! Declarations and defines for timer related functionality
//----------------------------------------------------------------------------------------------------//
#ifndef _TIMER_H
#define _TIMER_H

#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/ktypes.h>
#include <sys/unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct soft_tmr_s {
    unsigned int timeout;
    pid_t pid;
} soft_tmr_t;

void     soft_tmr_init(void) ;
int      add_tmr (pid_t pid, unsigned ticks);
unsigned 
int      remove_tmr (pid_t pid);
void     soft_tmr_handler (void);
unsigned 
int      xget_clock_ticks (void);

unsigned 
int      sys_xget_clock_ticks (void);
time_t   sys_time (time_t *timer);
unsigned sys_sleep (unsigned ticks);

#ifdef __cplusplus
}       
#endif 

#endif /* _TIMER_H */
