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
//! @file semaphore.h
//! XSI semaphore definitions and declarations
//----------------------------------------------------------------------------------------------------//
#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#include <sys/fcntl.h>
#include <os_config.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEM_NAME_MAX    25      // Length of symbolic name that can be associated with a semaphore

#define SEM_FAILED      -1      // Error indication

#ifndef SEM_NSEMS_MAX
#define SEM_NSEMS_MAX   10
#endif
 
#ifndef SEM_VALUE_MAX 
#define SEM_VALUE_MAX  200
#endif

typedef unsigned int sem_t;

int    sem_init(sem_t* sem, int pshared, unsigned value);
int    sem_wait(sem_t* sem);
/*int    sem_timedwait (sem_t *sem, const struct timespec *abs_timeout);*/
int    sem_timedwait (sem_t *sem, unsigned int ticks);
int    sem_trywait(sem_t* sem);
int    sem_getvalue(sem_t* sem, int* sval);
int    sem_post(sem_t* sem);
sem_t* sem_open(const char* name, int oflag, ...);
int    sem_unlink(const char* name);
int    sem_close(sem_t* sem);
int    sem_destroy(sem_t* sem);

#ifdef __cplusplus
}       
#endif 

#endif /* _SEMAPHORE_H */
