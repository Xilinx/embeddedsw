/******************************************************************************
*
* Copyright (C) 2014 - 2018 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the "Software"), to deal in
* the Software without restriction, including without limitation the rights to
* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
* the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software. If you wish to use our Amazon
* FreeRTOS name, please do so in a fair use way that does not cause confusion.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* http://www.FreeRTOS.org
* http://aws.amazon.com/freertos
*
* 1 tab == 4 spaces!
*
******************************************************************************/
/*****************************************************************************/
/**
* @file asm_vectors.s
*
* This file contains the initial vector table for the Cortex A53 processor
* Currently NEON registers are not saved on stack if interrupt is taken.
* It will be implemented.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- ---------------------------------------------------
* 5.00	pkp	5/21/14 Initial version
* </pre>
*
* @note
*
* None.
*
******************************************************************************/



.org 0
.text

.globl _boot
.globl _vector_table
.globl _freertos_vector_table

.globl FIQInterrupt
.globl IRQInterrupt
.globl SErrorInterrupt
.globl SynchronousInterrupt


.org 0

.section .vectors, "a"

_vector_table:

.set	VBAR, _vector_table

.org VBAR
	b	_boot

.org (VBAR + 0x80)
	b	.

.org (VBAR + 0x100)
	b	.

.org (VBAR + 0x180)
	b	.


.org (VBAR + 0x200)
	b	.

.org (VBAR + 0x280)
	b	.

.org (VBAR + 0x300)
	b	.

.org (VBAR + 0x380)
	b	.



.org (VBAR + 0x400)
	b	.

.org (VBAR + 0x480)
	b	.

.org (VBAR + 0x500)
	b	.

.org (VBAR + 0x580)
	b	.

.org (VBAR + 0x600)
	b	.

.org (VBAR + 0x680)
	b	.

.org (VBAR + 0x700)
	b	.

.org (VBAR + 0x780)
	b	.



/******************************************************************************
 * Vector table to use when FreeRTOS is running.
 *****************************************************************************/
.set	FREERTOS_VBAR, (VBAR+0x1000)

.org(FREERTOS_VBAR)
_freertos_vector_table:
	b	FreeRTOS_SWI_Handler

.org (FREERTOS_VBAR + 0x80)
	b	FreeRTOS_IRQ_Handler

.org (FREERTOS_VBAR + 0x100)
	b	.

.org (FREERTOS_VBAR + 0x180)
	b	.

.org (FREERTOS_VBAR + 0x200)
	b	FreeRTOS_SWI_Handler

.org (FREERTOS_VBAR + 0x280)
	b	FreeRTOS_IRQ_Handler

.org (FREERTOS_VBAR + 0x300)
	b	.

.org (FREERTOS_VBAR + 0x380)
	b	.

.org (FREERTOS_VBAR + 0x400)
	b	.

.org (FREERTOS_VBAR + 0x480)
	b	.

.org (FREERTOS_VBAR + 0x500)
	b	.

.org (FREERTOS_VBAR + 0x580)
	b	.

.org (FREERTOS_VBAR + 0x600)
	b	.

.org (FREERTOS_VBAR + 0x680)
	b	.

.org (FREERTOS_VBAR + 0x700)
	b	.

.org (FREERTOS_VBAR + 0x780)
	b	.

.org (FREERTOS_VBAR + 0x800)




SynchronousInterruptHandler:
	stp	X0,X1, [sp,#-0x10]!
	stp	X2,X3, [sp,#-0x10]!
	stp	X4,X5, [sp,#-0x10]!
	stp	X6,X7, [sp,#-0x10]!
	stp	X8,X9, [sp,#-0x10]!
	stp	X10,X11, [sp,#-0x10]!
	stp	X12,X13, [sp,#-0x10]!
	stp	X14,X15, [sp,#-0x10]!
	stp	X16,X17, [sp,#-0x10]!
	stp     X18,X19, [sp,#-0x10]!
	stp     X29,X30, [sp,#-0x10]!

        bl      SynchronousInterrupt

	ldp     X29,X30, [sp], #0x10
        ldp     X18,X19, [sp], #0x10
	ldp	X16,X17, [sp], #0x10
	ldp	X14,X15, [sp], #0x10
	ldp	X12,X13, [sp], #0x10
	ldp	X10,X11, [sp], #0x10
	ldp	X8,X9, [sp], #0x10
	ldp	X6,X7, [sp], #0x10
	ldp	X4,X5, [sp], #0x10
	ldp	X2,X3, [sp], #0x10
	ldp	X0,X1, [sp], #0x10

	eret

IRQInterruptHandler:
	stp	X0,X1, [sp,#-0x10]!
	stp	X2,X3, [sp,#-0x10]!
	stp	X4,X5, [sp,#-0x10]!
	stp	X6,X7, [sp,#-0x10]!
	stp	X8,X9, [sp,#-0x10]!
	stp	X10,X11, [sp,#-0x10]!
	stp	X12,X13, [sp,#-0x10]!
	stp	X14,X15, [sp,#-0x10]!
	stp	X16,X17, [sp,#-0x10]!
	stp     X18,X19, [sp,#-0x10]!
	stp     X29,X30, [sp,#-0x10]!

	bl      IRQInterrupt

	ldp     X29,X30, [sp], #0x10
	ldp     X18,X19, [sp], #0x10
	ldp	X16,X17, [sp], #0x10
	ldp	X14,X15, [sp], #0x10
	ldp	X12,X13, [sp], #0x10
	ldp	X10,X11, [sp], #0x10
	ldp	X8,X9, [sp], #0x10
	ldp	X6,X7, [sp], #0x10
	ldp	X4,X5, [sp], #0x10
	ldp	X2,X3, [sp], #0x10
	ldp	X0,X1, [sp], #0x10

	eret

FIQInterruptHandler:

	stp	X0,X1, [sp,#-0x10]!
	stp	X2,X3, [sp,#-0x10]!
	stp	X4,X5, [sp,#-0x10]!
	stp	X6,X7, [sp,#-0x10]!
	stp	X8,X9, [sp,#-0x10]!
	stp	X10,X11, [sp,#-0x10]!
	stp	X12,X13, [sp,#-0x10]!
	stp	X14,X15, [sp,#-0x10]!
	stp	X16,X17, [sp,#-0x10]!
	stp     X18,X19, [sp,#-0x10]!
	stp     X29,X30, [sp,#-0x10]!

        bl      FIQInterrupt

	ldp     X29,X30, [sp], #0x10
        ldp     X18,X19, [sp], #0x10
	ldp	X16,X17, [sp], #0x10
	ldp	X14,X15, [sp], #0x10
	ldp	X12,X13, [sp], #0x10
	ldp	X10,X11, [sp], #0x10
	ldp	X8,X9, [sp], #0x10
	ldp	X6,X7, [sp], #0x10
	ldp	X4,X5, [sp], #0x10
	ldp	X2,X3, [sp], #0x10
	ldp	X0,X1, [sp], #0x10

	eret

SErrorInterruptHandler:

	stp	X0,X1, [sp,#-0x10]!
	stp	X2,X3, [sp,#-0x10]!
	stp	X4,X5, [sp,#-0x10]!
	stp	X6,X7, [sp,#-0x10]!
	stp	X8,X9, [sp,#-0x10]!
	stp	X10,X11, [sp,#-0x10]!
	stp	X12,X13, [sp,#-0x10]!
	stp	X14,X15, [sp,#-0x10]!
	stp	X16,X17, [sp,#-0x10]!
	stp     X18,X19, [sp,#-0x10]!
	stp     X29,X30, [sp,#-0x10]!

        bl      SErrorInterrupt

	ldp     X29,X30, [sp], #0x10
        ldp     X18,X19, [sp], #0x10
	ldp	X16,X17, [sp], #0x10
	ldp	X14,X15, [sp], #0x10
	ldp	X12,X13, [sp], #0x10
	ldp	X10,X11, [sp], #0x10
	ldp	X8,X9, [sp], #0x10
	ldp	X6,X7, [sp], #0x10
	ldp	X4,X5, [sp], #0x10
	ldp	X2,X3, [sp], #0x10
	ldp	X0,X1, [sp], #0x10

	eret

.end
