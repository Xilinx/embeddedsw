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
//! @file init.h
//! This files contains structures, that are used for configuring the system.
//! The values are specified in sys/init.c
//----------------------------------------------------------------------------------------------------//

#ifndef _INIT_H
#define _INIT_H

#include <config/config_cparam.h>
#include <config/config_param.h>
#include <sys/ktypes.h>
#ifdef __MICROBLAZE__
#include <sys/mpu.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------------------------------//
//! Processes to be initialised at the start of the system are defined here.
//----------------------------------------------------------------------------------------------------//
struct _process_init {
    unsigned int p_start_addr ;	 // Start address of the process
    int priority ;		 // Priority of the process
} ;

//----------------------------------------------------------------------------------------------------//
//! Threads to be a part of kernel executable to be initialised at the start of the
//! system are defined here.
//----------------------------------------------------------------------------------------------------//
struct _elf_pthread_init {
    void (*start_func)(void);	 // Start address of the thread
    int priority ;	         // Priority of the thread
} ;

//----------------------------------------------------------------------------------------------------//
//! The shared memory in the system are defined here.
//! There is a entry for each Shared Memory
//----------------------------------------------------------------------------------------------------//
struct _shm_init {
    unsigned int shm_size ;		// Size of the Shared Memory
} ;

//----------------------------------------------------------------------------------------------------//
//! The dynamic memory (buffer) management module is configured here.
//! The system can have memory blocks of different sizes. Memory blocks of
//! different size and the number of memory blocks is specified here.
//----------------------------------------------------------------------------------------------------//
typedef struct bufmalloc_init_s {
    unsigned int bsiz;                  // Memory Block size
    char         nblks;                 // Number of blocks of the size
} bufmalloc_init_t;

void soft_tmr_init(void);
void pthread_init(void);
void bufmalloc_init(void);
void hw_init(void);
int xmk_add_static_thread(void* (*start_routine)(void *), int sched_priority);
void xilkernel_init (void);
void xilkernel_start (void);

#ifdef __cplusplus
}
#endif

#endif /* _INIT_H */
