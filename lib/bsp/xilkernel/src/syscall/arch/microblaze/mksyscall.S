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
/*----------------------------------------------------------------------------------------//
//! @file
//! mksyscall.S - System call stub	
-----------------------------------------------------------------------------------------//*/
	
#include <os_config.h>
#include <sys/arch.h>

	
/*  make_syscall (param1, param2, param3, param4, param5, syscall_num);  
 *		   r5,     r6,     r7,     r8,     r9,     r10
 *               
 */
		
	.global make_syscall
	.section .text
	.align 2	
	.ent make_syscall
make_syscall:
	addi	r1, r1, -4;				
	sw	r15, r0, r1;			/* Save link register */
	bralid	r15, (CONFIG_BASE_VECTORS + 8);	/* Make system call. Parameters in registers. System call number in r10 */
	nop; 
	lw	r15, r0, r1;			/* Restore link register */
	rtsd	r15, 8;
	addik	r1, r1, 4;
	.end make_syscall
