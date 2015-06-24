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
//! @file libsemaphore.c
//! This contains system call wrapper functions for POSIX Semaphore routines
//----------------------------------------------------------------------------------------------------//
#include <os_config.h>
#include <sys/syscall.h>
#include <semaphore.h>
#include <stdarg.h>
#include <sys/ksemaphore.h>

extern void* make_syscall (void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, int syscall_num);

#ifdef CONFIG_SEMA
int sem_init(sem_t* sem, int pshared, unsigned value)
{
    return (int) make_syscall ((void*)sem, (void*)pshared, (void*)value, NULL, NULL, SC_SEM_INIT);
}

int sem_trywait(sem_t* sem)
{
    return (int) make_syscall ((void*)sem, NULL, NULL, NULL, NULL, SC_SEM_TRYWAIT);
}

int sem_wait(sem_t* sem)
{
    return (int) make_syscall ((void*)sem, NULL, NULL, NULL, NULL, SC_SEM_WAIT);
}

#ifdef CONFIG_TIME
int sem_timedwait (sem_t* sem, unsigned int ticks)
{
    return (int) make_syscall ((void*)sem, (void*)ticks, NULL, NULL, NULL, SC_SEM_TIMED_WAIT);
}
#endif

int sem_getvalue(sem_t* sem, int* sval)
{
    return (int) make_syscall ((void*)sem, (void*)sval, NULL, NULL, NULL, SC_SEM_GETVALUE);
}

int sem_post(sem_t* sem)
{
    return (int) make_syscall ((void*)sem, NULL, NULL, NULL, NULL, SC_SEM_POST);
}

#ifdef CONFIG_NAMED_SEMA
sem_t* sem_open(const char* name, int oflag, ...)
{
    mode_t mode;
    unsigned value;
    va_list varptr;
    
    if( !(oflag & O_CREAT) )                         // Other flags unsupported
	return (sem_t*)SEM_FAILED;

    if (!(oflag & O_EXCL)) {

	va_start( varptr, oflag );
	
	mode = va_arg( varptr, mode_t );
	value = va_arg( varptr, unsigned );
	
	va_end (varptr);
    }

    return (sem_t*) make_syscall ((void*)name, (void*)oflag, (void*)mode, (void*)value, NULL, SC_SEM_OPEN);
}

int sem_unlink(const char* name)
{
    return (int) make_syscall ((void*)name, NULL, NULL, NULL, NULL, SC_SEM_UNLINK);
}

int sem_close(sem_t* sem)
{
    return (int) make_syscall ((void*)sem, NULL, NULL, NULL, NULL, SC_SEM_CLOSE);
}
#endif

int sem_destroy(sem_t* sem)
{
    return (int) make_syscall ((void*)sem, NULL, NULL, NULL, NULL, SC_SEM_DESTROY);
}
#endif /* CONFIG_SEMA */
