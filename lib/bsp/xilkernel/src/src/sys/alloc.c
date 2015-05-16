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
//! @file alloc.c
//! Kernel data structures to be allocated in the BSS section.
//----------------------------------------------------------------------------------------------------//

#include <os_config.h>
#include <config/config_param.h>
#include <sys/arch.h>

#if PTHREAD_STACK_SIZE < 1024
#define KERNEL_IRQ_STACKSZ 1024
#else
#define KERNEL_IRQ_STACKSZ PTHREAD_STACK_SIZE
#endif

//! kernel_irq_stack - Stack to be used on IRQs
char kernel_irq_stack[KERNEL_IRQ_STACKSZ] __attribute__ ((aligned (STACK_ALIGN)));
const char *kernel_irq_stack_ptr = (const char *)(kernel_irq_stack + KERNEL_IRQ_STACKSZ + SSTACK_PTR_ADJUST);
const char *kernel_irq_stack_ptr_end = (const char *)(kernel_irq_stack + SSTACK_PTR_ADJUST);

#ifdef CONFIG_ELF_PROCESS
//! process_startup_stack - Temporary initial stack to be used by processes
//! before their own elf stacks are setup. This is potentially dangerous
char process_startup_stack[PROCESS_STARTUP_STACKSZ] __attribute__ ((aligned (STACK_ALIGN)));
#endif
