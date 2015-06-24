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

/*
//----------------------------------------------------------------------------------------//
//! @file
//! syscall.S - Contains the system call table
//-----------------------------------------------------------------------------------------//
*/
#include <os_config.h>

.global syscall_table        

/*
 * System call table - System call number defined in syscall.h
 * If the module is not included in the system, then the location
 * in the table is replaced by 0.
 */
        .section .rodata
        .align 2
syscall_table:                                          /* System call table */ 
        .long 0
#ifdef CONFIG_ELF_PROCESS        
        .long sys_elf_process_create                    /*  1 */
        .long sys_elf_exit                              /*  2 */
#else   
        .long 0
        .long 0                                        
#endif
        
#ifdef CONFIG_KILL
        .long sys_kill                                  /*  3 */
#else
        .long 0
#endif
        .long sys_process_status                        /*  4 */
        .long sys_get_reentrancy                        /*  5 */
#ifdef CONFIG_YIELD
        .long sys_yield                                 /*  6 */
#else
        .long 0
#endif

        .long sys_get_currentPID                        /*  7 */
        .long 0                                         /*  8 */
        .long 0                                         /*  9 */

#ifdef CONFIG_PTHREAD_SUPPORT
        .long sys_pthread_create                        /* 10 */
        .long sys_pthread_exit                          /* 11 */
        .long sys_pthread_join                          /* 12 */
        .long sys_pthread_self                          /* 13 */
        .long sys_pthread_detach                        /* 14 */
        .long sys_pthread_equal                         /* 15 */
#ifdef CONFIG_PRIOSCHED
        .long sys_pthread_getschedparam                 /* 16 */
        .long sys_pthread_setschedparam                 /* 17 */
#else
        .long 0
        .long 0
#endif
#else   
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
#endif

#ifdef CONFIG_PTHREAD_MUTEX
        .long sys_pthread_mutex_init                    /* 18 */
        .long sys_pthread_mutex_lock                    /* 19 */
        .long sys_pthread_mutex_trylock                 /* 20 */
        .long sys_pthread_mutex_unlock                  /* 21 */
        .long sys_pthread_mutex_destroy                 /* 22 */
#else
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
#endif
        .long 0                                         /* 23 */
        .long 0                                         /* 24 */
        .long 0                                         /* 25 */
        .long 0                                         /* 26 */
        .long 0                                         /* 27 */
        .long 0                                         /* 28 */
        .long 0                                         /* 29 */
        
#ifdef CONFIG_SEMA
        .long sys_sem_init                              /* 30 */                
        .long sys_sem_wait_x                            /* 31 */
        .long sys_sem_trywait                           /* 32 */
        .long sys_sem_post                              /* 33 */
        .long sys_sem_destroy                           /* 34 */
#ifdef CONFIG_NAMED_SEMA        
        .long sys_sem_open                              /* 35 */
        .long sys_sem_close                             /* 36 */
        .long sys_sem_unlink                            /* 37 */
#else
        .long 0
        .long 0
        .long 0        
#endif        
        .long sys_sem_getvalue                          /* 38 */
#ifdef CONFIG_TIME
        .long sys_sem_timedwait                         /* 39 */
#else           
        .long 0
#endif
        .long 0                                         /* 40 */
#else
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0                                 
        .long 0                                 
#endif
        
#ifdef CONFIG_MSGQ
        .long sys_msgget                                /* 41 */
        .long sys_msgctl                                /* 42 */
        .long sys_msgsnd                                /* 43 */
        .long sys_msgrcv                                /* 44 */
        .long 0                                         /* 45 */
#else                   
        .long 0                                 
        .long 0
        .long 0
        .long 0
        .long 0 
#endif

#ifdef CONFIG_SHM
        .long sys_shmget                                /* 46 */
        .long sys_shmctl                                /* 47 */
        .long sys_shmat                                 /* 48 */
        .long sys_shmdt                                 /* 49 */
#else
        .long 0
        .long 0
        .long 0
        .long 0
#endif
        
#ifdef CONFIG_BUFMALLOC
        .long sys_bufcreate                             /* 50 */
        .long sys_bufdestroy                            /* 51 */
        .long sys_bufmalloc                             /* 52 */
        .long sys_buffree                               /* 53 */
        .long 0                                         /* 54 */
#else
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
#endif
        
#ifdef CONFIG_TIME
        .long sys_xget_clock_ticks                      /* 55 */
        .long sys_sleep                                 /* 56 */
        .long sys_time                                  /* 57 */
#else
        .long 0                 
        .long 0
        .long 0
#endif
        
#ifdef CONFIG_INTC
        .long sys_register_int_handler                  /* 58 */
        .long sys_unregister_int_handler                /* 59 */
        .long sys_enable_interrupt                      /* 60 */
        .long sys_disable_interrupt                     /* 61 */
        .long sys_acknowledge_interrupt                 /* 62 */
#else
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
#endif
        
#ifdef CONFIG_STATS
        .long sys_get_kernel_stats                      /* 63 */
#else
        .long 0                                 
#endif
        .long 0                                         /* 64 */
        .long 0                                         /* 65 */
        .long 0                                         /* 66 */
        .long 0                                         /* 67 */
        .long 0                                         /* 68 */
        .long 0                                         /* 69 */
        .long 0                                         /* 70 */        
        .long 0                                         /* 71 */
        .long 0                                         /* 72 */
        
#ifdef CONFIG_NET
        .long xilsock_socket                            /* 73 */
        .long xilsock_bind                              /* 74 */
        .long xilsock_listen                            /* 75 */
        .long xilsock_accept                            /* 76 */
        .long xilsock_recv                              /* 77 */
        .long xilsock_send                              /* 78 */        
        .long xilsock_recv                              /* 79 */
        .long xilsock_sendto                            /* 80 */
        .long xilsock_close                             /* 81 */
#else
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
        .long 0
#endif
