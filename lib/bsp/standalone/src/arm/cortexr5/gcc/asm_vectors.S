/******************************************************************************
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file asm_vectors.S
*
* This file contains the initial vector table for the Cortex R5 processor
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- ---------------------------------------------------
* 5.00  pkp	02/10/14 Initial version
* 6.0   mus     27/07/16 Added UndefinedException handler
* 6.3	pkp	02/13/17 Added support for hard float
* 9.4   vmt     14/07/25 Extended FPU register support to 32 for Cortex-R52
* </pre>
*
******************************************************************************/
.org 0
.text

.globl _boot
.globl _vector_table

.globl FIQInterrupt
.globl IRQInterrupt
.globl SWInterrupt
.globl DataAbortInterrupt
.globl PrefetchAbortInterrupt

.globl IRQHandler
.globl prof_pc

.section .vectors, "a"
_vector_table:
	ldr	pc,=_boot
	ldr	pc,=Undefined
	ldr	pc,=SVCHandler
	ldr	pc,=PrefetchAbortHandler
	ldr	pc,=DataAbortHandler
	NOP	/* Placeholder for address exception vector*/
	ldr	pc,=IRQHandler
	ldr	pc,=FIQHandler

.text
IRQHandler:					/* IRQ vector handler */
	stmdb	sp!,{r0-r3,r12,lr}		/* state save from compiled code*/
#ifndef __SOFTFP__
#if defined (ARMR52)
        vpush {d0-d15}
        vpush {d16-d31}
#else
	vpush {d0-d7}				/* Store floating point registers */
#endif
	vmrs r1, FPSCR
	push {r1}
	vmrs r1, FPEXC
	push {r1}
#endif
	bl	IRQInterrupt			/* IRQ vector */
#ifndef __SOFTFP__

	pop 	{r1}				/* Restore floating point registers */
	vmsr    FPEXC, r1
	pop 	{r1}
	vmsr    FPSCR, r1
#if defined (ARMR52)
        vpop    {d16-d31}
        vpop    {d0-d15}
#else
	vpop    {d0-d7}
#endif
#endif
	ldmia	sp!,{r0-r3,r12,lr}		/* state restore from compiled code */
	subs	pc, lr, #4			/* adjust return */
FIQHandler:					/* FIQ vector handler */
	stmdb	sp!,{r0-r3,r12,lr}		/* state save from compiled code */
FIQLoop:
	bl	FIQInterrupt			/* FIQ vector */
	ldmia	sp!,{r0-r3,r12,lr}		/* state restore from compiled code */
	subs	pc, lr, #4			/* adjust return */

Undefined:					/* Undefined handler */
	stmdb	sp!,{r0-r3,r12,lr}		/* state save from compiled code */
	ldr     r0, =UndefinedExceptionAddr
	sub     r1, lr, #4
	str     r1, [r0]            		/* Store address of instruction causing undefined exception */

	bl	UndefinedException		/* UndefinedException: call C function here */
	ldmia	sp!,{r0-r3,r12,lr}		/* state restore from compiled code */
	movs	pc, lr

SVCHandler:					/* SWI handler */
	stmdb	sp!,{r0-r3,r12,lr}		/* state save from compiled code */
	tst	r0, #0x20			/* check the T bit */
	ldrneh	r0, [lr,#-2]			/* Thumb mode */
	bicne	r0, r0, #0xff00			/* Thumb mode */
	ldreq	r0, [lr,#-4]			/* ARM mode */
	biceq	r0, r0, #0xff000000		/* ARM mode */
	bl	SWInterrupt			/* SWInterrupt: call C function here */
	ldmia	sp!,{r0-r3,r12,lr}		/* state restore from compiled code */
	movs	pc, lr				/* adjust return */

DataAbortHandler:				/* Data Abort handler */
	stmdb	sp!,{r0-r3,r12,lr}		/* state save from compiled code */
	ldr     r0, =DataAbortAddr
	sub     r1, lr, #8
	str     r1, [r0]            		/* Stores instruction causing data abort */
	bl	DataAbortInterrupt		/*DataAbortInterrupt :call C function here */
	ldmia	sp!,{r0-r3,r12,lr}		/* state restore from compiled code */
	subs	pc, lr, #8			/* adjust return */

PrefetchAbortHandler:				/* Prefetch Abort handler */
	stmdb	sp!,{r0-r3,r12,lr}		/* state save from compiled code */
	ldr     r0, =PrefetchAbortAddr
	sub     r1, lr, #4
	str     r1, [r0]            		/* Stores instruction causing prefetch abort */
	bl	PrefetchAbortInterrupt		/* PrefetchAbortInterrupt: call C function here */
	ldmia	sp!,{r0-r3,r12,lr}		/* state restore from compiled code */
	subs	pc, lr, #4			/* adjust return */


.end
