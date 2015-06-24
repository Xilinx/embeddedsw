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
//! @file os_config.h 
//! This file caontains all the configuration #define's for the system. The
//! different modules of the kernel can be configured by changing this file
//! and recompiling the kernel. Some of the modules that are configurable are:
//!  - Process Management
//!  - Thread Management
//!  - Scheduling type
//!  - Semaphore
//!  - Message Queue
//!  - Shared Memory
//!  - Dynamic Buffer Management
//!
//!  By recompiling, the kernel gets compiled with the defaults for each 
//!  modules. To further configure the kernel changes may have to be made in the
//!  following files:
//!  - config_param.h
//!  - config_cparam.h
//!  - config_init.h
//----------------------------------------------------------------------------------------------------//

#ifndef _OS_CONFIG_H
#define _OS_CONFIG_H

//! Timer ticks value used for scheduling.
#define TIMER_TICKS 2	

//! Process Management.
//! Further configuration of this module can be done by modifying 
//! config_param.h. The initial system processes are specified in sys/init.c file.
#define MAX_PROCS 10	// Max. number of Processesing contexts in the system 

//! Max. number of Processes in each Priority ready queue. This determines the size
//! of the Ready Queue. This is determined by the type of application.
#define MAX_READYQ 10	

#define CONFIG_KILL     1           //! Include kill() function 
#define CONFIG_YIELD    1           //! Include yield() function 

//! Type of process scheduling. There two types of scheduling supported and 
//! can be configured during system build.
#define SCHED_TYPE 3	//! SCHED_PRIO 


//! ELF Process Management
//! Further configuration of this module can be done by modifying 
//! config_param.h. 
#define CONFIG_ELF_PROCESS      1	// Support ELF process functionality

//! Thread Management
//! Further configuration of this module can be done by modifying 
//! config_param.h. 
#define CONFIG_PTHREAD_SUPPORT  1	//! Support thread functionality

//! Semaphore
//! Further configuration of this module can be done by modifying 
//! config_param.h. 

//! Include the Semapore module 
#define CONFIG_SEMA 	1

//! pthread mutex
//! Further configuration of this module can be done by modifying 
//! config_param.h. 

//! Include the Mutex module 
#define CONFIG_PTHREAD_MUTEX 	1

//! Message Queue
//! Further configuration of this module can be done by modifying 
//! config_cparam.h and sys/init.c.

//! Include the Message Queue functionality. 
#define CONFIG_MSGQ 	1

//! Shared Memory
//! Further configuration of this module can be done by modifying 
//! config_cparam.h and sys/init.c.
//! Include the Shared Memory Functionality 
#define CONFIG_SHM 	1

//! Dynamic Buffer Management
//! Further configuration of this module can be done by modifying 
//! config_cparam.h and sys/init.c.
//! Include the Dynamic buffer management functionality.
#define CONFIG_MALLOC 	1

//! Cache Enable
//! For PPC only
//! Further configuration can be done by modifying sys/main.c
//! Enable caches 
#define CONFIG_CACHE 1

//! Configure timer functionality
#define CONFIG_TIMERS 1

//! Vector base address
#define CONFIG_BASE_VECTORS 0x00000000

#endif /* _OS_CONFIG_H */
