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
//! @file syscall.h
//! The System call numbers for the system are defined here. These numbers
//! are then used to index into the system call vector table by the system
//! call handler, when a system call is invoked.
//!
//! The system calls are available for following routines
//!  - Process Management
//!  - Thread Managament
//!  - Semaphore
//!  - Message Queue
//!  - Shared Memory
//!  - Dynamic Buffer Management
//!  - XilNet
//----------------------------------------------------------------------------------------------------//
#ifndef _SYSCALL_H
#define _SYSCALL_H

#ifdef __cplusplus
extern "C" {
#endif

#define SC_PROCESS_CREATE		1	// Process Management
#define SC_PROCESS_EXIT			2
#define SC_PROCESS_KILL			3
#define SC_PROCESS_STATUS		4
#define SC_GET_REENTRANCY		5
#define SC_PROCESS_YIELD		6
#define SC_PROCESS_GETPID		7
#define SC_PROCESS_GETPRIORITY		8
#define SC_PROCESS_SETPRIORITY		9

#define SC_PTHREAD_CREATE		10	// Thread Management
#define SC_PTHREAD_EXIT			11
#define SC_PTHREAD_JOIN			12
#define SC_PTHREAD_SELF			13
#define SC_PTHREAD_DETACH		14
#define SC_PTHREAD_EQUAL		15
#define SC_PTHREAD_GETSCHEDPARAM	16
#define SC_PTHREAD_SETSCHEDPARAM	17
#define SC_PTHREAD_MUTEX_INIT		18	// Pthread Mutex handling
#define SC_PTHREAD_MUTEX_LOCK		19
#define SC_PTHREAD_MUTEX_TRYLOCK        20
#define SC_PTHREAD_MUTEX_UNLOCK	        21
#define SC_PTHREAD_MUTEX_DESTROY	22
#define SC_PTHREAD_RESERVED_0           23
#define SC_PTHREAD_RESERVED_1           24
#define SC_PTHREAD_RESERVED_2           25
#define SC_PTHREAD_RESERVED_3           26
#define SC_PTHREAD_RESERVED_4           27
#define SC_PTHREAD_RESERVED_5           28
#define SC_PTHREAD_RESERVED_6           29

#define SC_SEM_INIT			30	// Semaphore
#define SC_SEM_WAIT			31
#define SC_SEM_TRYWAIT			32
#define SC_SEM_POST			33
#define SC_SEM_DESTROY			34
#define SC_SEM_OPEN			35
#define SC_SEM_CLOSE			36
#define SC_SEM_UNLINK			37
#define SC_SEM_GETVALUE			38
#define SC_SEM_TIMED_WAIT               39
#define SC_SEM_RESERVED_1               40

#define SC_MSGGET			41	// Message Queue
#define SC_MSGCTL			42
#define SC_MSGSND			43
#define SC_MSGRCV			44
#define SC_MSG_RESERVED_0               45

#define SC_SHMGET			46	// Shared Memory
#define SC_SHMCTL			47
#define SC_SHMAT			48
#define SC_SHMDT			49


#define SC_BUFCREATE			50	// Dynamic Buffer Management
#define SC_BUFDESTROY    		51
#define SC_BUFMALLOC                    52
#define SC_BUFFREE			53
#define SC_MALLOC_RESERVED_2            54

#define SC_TMR_GETCLOCKTICKS		55
#define SC_TMR_SLEEP			56      // Software timers
#define SC_TMR_TIME			57

#define SC_REGISTER_INT_HANDLER         58      // User level interrupt handlers
#define SC_UNREGISTER_INT_HANDLER       59
#define SC_ENABLE_INTERRUPT             60
#define SC_DISABLE_INTERRUPT            61
#define SC_ACKNOWLEDGE_INTERRUPT        62

//#define SC_RESERVED_0                 63
#define SC_GET_KERNEL_STATS             63
#define SC_RESERVED_1                   64
#define SC_RESERVED_2                   65
#define SC_RESERVED_3                   66
#define SC_RESERVED_4                   67
#define SC_RESERVED_5                   68
#define SC_RESERVED_6                   69
#define SC_RESERVED_7                   70
#define SC_RESERVED_8                   71
#define SC_RESERVED_9                   72

#define SC_XILSOCK_SOCKET	        73	// Xil Net
#define SC_XILSOCK_BIND	                74
#define SC_XILSOCK_LISTEN 	        75
#define SC_XILSOCK_ACCEPT	        76
#define SC_XILSOCK_RECV	                77
#define SC_XILSOCK_SEND	                78
#define SC_XILSOCK_RECVFROM	        79
#define SC_XILSOCK_SENDTO	        80
#define SC_XILSOCK_CLOSE	        81

struct t_syscall_data {
    int system_call_num; // predefined system call number
    // actual function to be called will be passed these params
    void *param1;
    void *param2;
    void *param3;
    void *param4;
    void *param5;
    void *param6;
};

#ifdef __cplusplus
}
#endif

#endif  /* _SYSCALL_H */
