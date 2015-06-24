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
//! @file stats.h
//! Kernel statistics collection definitions and structures
//----------------------------------------------------------------------------------------------------//

#ifndef _STATS_H
#define _STATS_H

#include <os_config.h>
#include <sys/ktypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCHED_HISTORY_SIZ    25

//! Process Status structure used for process_status
typedef struct _ps_stat{
    pid_t 	        pid ;           //! Process ID 
    unsigned char 	state ;		//! State of process 
    unsigned long       aticks;         //! Active ticks
    signed char         priority;       //! Process priority
} p_stat;

//! Kernel statistics structure
typedef struct kstats_s {
    unsigned int kernel_ticks;
    pid_t        sched_history[SCHED_HISTORY_SIZ];
    int          pstat_count;
    p_stat       *pstats;
} kstats_t;

#ifdef __cplusplus
}       
#endif 

#endif /* _STATS_H */
