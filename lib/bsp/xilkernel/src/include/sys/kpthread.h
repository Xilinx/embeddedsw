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
//! @file kpthread.h
//! Kernel pthread declarations and definitions
//----------------------------------------------------------------------------------------------------//
#ifndef _KPTHREAD_H
#define _KPTHREAD_H

#include <os_config.h>
#include <config/config_cparam.h>
#include <config/config_param.h>
#include <sys/ktypes.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// Defines

#define PTHREAD_INVALID          0xFF
#define PTHREAD_MUTEX_INVALID    0xFFFF
// Sched state
#define PTHREAD_STATE_ALIVE     1
#define PTHREAD_STATE_EXIT      2
#define PTHREAD_STATE_DETACHED  3
#define PTHREAD_STATE_BLOCKED   4

void pthread_terminate (pthread_info_t *thread);

// pthreads - Kernel implementation
int sys_pthread_create (pthread_t *thread, const pthread_attr_t *attr,
	 void *(*start_func)(void*), void *param);
void sys_pthread_exit (void *retval) ;
int sys_pthread_join (pthread_t target, void **retval);
pthread_t sys_pthread_self (void);
int sys_pthread_detach (pthread_t thread);
int sys_pthread_equal (pthread_t thread_1, pthread_t thread_2);
int pthread_attr_init (pthread_attr_t *attr);
int pthread_attr_destroy (pthread_attr_t *attr);

#if SCHED_TYPE == SCHED_PRIO
int pthread_attr_setschedparam (pthread_attr_t *attr,
	 const struct sched_param *spar);
int pthread_attr_getschedparam (const pthread_attr_t *attr,
	 struct sched_param *spar);
#endif

int pthread_attr_getdetachstate (const pthread_attr_t *attr, int *dstate);
int pthread_attr_setdetachstate (pthread_attr_t *attr, int dstate);
int pthread_attr_getstack (const pthread_attr_t *attr, void **stackaddr,
	 size_t *stacksize);
int pthread_attr_setstack (pthread_attr_t *attr, void *stackaddr,
	 size_t stacksize);

#ifdef CONFIG_PTHREAD_MUTEX
int sys_pthread_mutex_init (pthread_mutex_t *mutex,
	 const pthread_mutexattr_t *attr);
int sys_pthread_mutex_destroy (pthread_mutex_t *mutex);
int sys_pthread_mutex_lock (pthread_mutex_t *mutex);
int sys_pthread_mutex_trylock (pthread_mutex_t *mutex);
int sys_pthread_mutex_unlock (pthread_mutex_t *mutex);
int pthread_mutexattr_init (pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy (pthread_mutexattr_t *attr);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
#endif

#if SCHED_TYPE == SCHED_PRIO
int       sys_pthread_getschedparam (pthread_t thread, int *policy, struct sched_param *param);
int       sys_pthread_setschedparam (pthread_t thread, int policy, struct sched_param *param);
#endif

#ifdef __cplusplus
}       
#endif 

#endif /* _KPTHREAD_H */
