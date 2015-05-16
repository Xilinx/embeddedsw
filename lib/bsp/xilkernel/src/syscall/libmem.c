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
//! @file libmem.c
//! This contains system call wrapper functions for Dynamic buffer management.
//----------------------------------------------------------------------------------------------------//
#include <os_config.h>
#include <sys/syscall.h>
#include <sys/bufmalloc.h>

extern void* make_syscall (void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, int syscall_num);

#ifdef CONFIG_BUFMALLOC
int bufcreate (membuf_t *mbuf, void *memptr, int nblks, size_t blksiz)
{
    return (int)make_syscall ((void*)mbuf, memptr, (void*)nblks, (void*)blksiz, NULL, SC_BUFCREATE);
}

int bufdestroy (membuf_t mbuf)
{
    return (int)make_syscall ((void*)mbuf, NULL, NULL, NULL, NULL, SC_BUFDESTROY);
}

void* bufmalloc (membuf_t mbuf, size_t siz)
{
    return (void*)make_syscall ((void*)mbuf, (void*)siz, NULL, NULL, NULL, SC_BUFMALLOC);
}

void buffree (membuf_t mbuf, void *mem)
{
    make_syscall ((void*)mbuf, (void*)mem, NULL, NULL, NULL, SC_BUFFREE);
}
#endif /* CONFIG_MALLOC */
