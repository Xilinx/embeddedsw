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
//! @file kmsg.h
//! Kernel message queue definitions and declarations
//----------------------------------------------------------------------------------------------------//

#ifndef _KMSG_H
#define _KMSG_H

#include <sys/ktypes.h>
#include <sys/ksemaphore.h>
#include <sys/msg.h>
#include <sys/queue.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSGQ_MAX_BYTES  100

// System calls
int sys_msgctl(int msqid, int cmd, struct msqid_ds *buf);
int sys_msgget(key_t key, int msgflg); 
ssize_t sys_msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
int sys_msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

#ifdef __cplusplus
}       
#endif 

#endif /* _KMSG_H */
