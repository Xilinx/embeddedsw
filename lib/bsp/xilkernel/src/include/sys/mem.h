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
//! @file mem.h
//! Kernel level memory allocation definitions and declarations
//----------------------------------------------------------------------------------------------------//
#ifndef _SYS_MEM_H
#define _SYS_MEM_H

#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/ktypes.h>
#include <sys/queue.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEMQ_START 	(N_PRIO*MAX_READYQ)

//! This is used by buffer management routines.
//! Each Memory Block of different sizes is associated with each structure
struct _malloc_info {
    unsigned int mem_bsize ;	//! Memory Block Size
    unsigned char max_blks ;	//! Max. number of mem blocks
    unsigned char n_blks ;	//! No. of mem blocks allocated
    unsigned short start_blk ;	//! The starting mem. blk number
    signed char *start_addr ;	//! Starting memory location for this bll
};

void    alloc_pidq_mem( queuep queue, unsigned int qtype, unsigned int qno ) ;
int     se_process_init(void) ;
int     kb_pthread_init(void);
void    alloc_msgq_mem( queuep queue, unsigned int qno ) ;
void    msgq_init(void) ;
void    shm_init(void) ;
void    bss_mem_init(void) ;
int     alloc_bss_mem( unsigned int *start, unsigned int *end ) ;
void    free_bss_mem( unsigned int memid ) ;
void    malloc_init(void) ;
void    sem_heap_init(void);
void    bufmalloc_mem_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_MEM_H */
