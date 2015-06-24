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
//! @file ksemaphore.h
//! Kernel level semaphore definitions and declarations
//----------------------------------------------------------------------------------------------------//

#ifndef _KSEMAPHORE_H
#define _KSEMAPHORE_H

#include <sys/types.h>
#include <sys/ktypes.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

int    sys_sem_init(sem_t* sem, int pshared, unsigned value);
sem_t* sys_sem_open(const char* name, int oflag, mode_t mode, unsigned value);
int    sys_sem_close(sem_t* sem);
int    sys_sem_unlink(const char* name);
int    sys_sem_getvalue(sem_t* sem, int* sval);
int    sys_sem_wait_x(sem_t* sem);
int    sys_sem_trywait(sem_t *sem);
int    sys_sem_post(sem_t* sem);
int    sys_sem_destroy(sem_t* sem);
int    sem_force_destroy (sem_t* sem);

void sem_heap_init(void);
sem_info_t* get_sem_by_semt( sem_t* sem);
sem_info_t* get_sem_by_name( char*  sem);
sem_t* get_semt_by_name( char* name);

#ifdef __cplusplus
}       
#endif 

#endif /* _KSEMAPHORE_H */
