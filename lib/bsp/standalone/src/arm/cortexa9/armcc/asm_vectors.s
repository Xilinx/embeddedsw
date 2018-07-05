;******************************************************************************
;
; Copyright (C) 2009 - 2015 Xilinx, Inc.  All rights reserved.
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
; XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
; OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.
;
; Except as contained in this notice, the name of the Xilinx shall not be used
; in advertising or otherwise to promote the sale, use or other dealings in
; this Software without prior written authorization from Xilinx.
;
;*****************************************************************************
;****************************************************************************
;**
; @file asm_vectors.s
;
; This file contains the initial vector table for the Cortex A9 processor
;
; <pre>
; MODIFICATION HISTORY:
;
; Ver	Who     Date	  Changes
; ----- ------- -------- ---------------------------------------------------
; 1.00a ecm/sdm 10/20/09 Initial version
; 3.11a asa	9/17/13	 Added support for neon.
; 4.00  pkp	01/22/14 Modified return addresses for interrupt
;			 handlers
; 5.1	pkp	05/13/15 Saved the addresses of instruction causing data
;			 abort and prefetch abort into DataAbortAddr and
;			 PrefetchAbortAddr for further use to fix CR#854523
; 5.4	pkp	12/03/15 Added handler for undefined exception
;</pre>
;
; @note
;
; None.
;
;****************************************************************************

	EXPORT _vector_table
	EXPORT IRQHandler

	IMPORT _boot
	IMPORT _prestart
	IMPORT IRQInterrupt
	IMPORT FIQInterrupt
	IMPORT SWInterrupt
	IMPORT DataAbortInterrupt
	IMPORT PrefetchAbortInterrupt
	IMPORT UndefinedException
	IMPORT DataAbortAddr
	IMPORT PrefetchAbortAddr
	IMPORT UndefinedExceptionAddr

	AREA |.vectors|, CODE
	REQUIRE8     {TRUE}
	PRESERVE8    {TRUE}
	ENTRY ; define this as an entry point
_vector_table
	B	_boot
	B	Undefined
	B	SVCHandler
	B	PrefetchAbortHandler
	B	DataAbortHandler
	NOP	; Placeholder for address exception vector
	B	IRQHandler
	B	FIQHandler


IRQHandler					; IRQ vector handler

	stmdb	sp!,{r0-r3,r12,lr}		; state save from compiled code
	vpush {d0-d7}
	vpush {d16-d31}
	vmrs r1, FPSCR
	push {r1}
	vmrs r1, FPEXC
	push {r1}
	bl	IRQInterrupt			; IRQ vector
	pop 	{r1}
	vmsr    FPEXC, r1
	pop 	{r1}
	vmsr    FPSCR, r1
	vpop    {d16-d31}
	vpop    {d0-d7}
	ldmia	sp!,{r0-r3,r12,lr}		; state restore from compiled code
	subs	pc, lr, #4			; adjust return


FIQHandler					; FIQ vector handler
	stmdb	sp!,{r0-r3,r12,lr}		; state save from compiled code
	vpush {d0-d7}
	vpush {d16-d31}
	vmrs r1, FPSCR
	push {r1}
	vmrs r1, FPEXC
	push {r1}
FIQLoop
	bl	FIQInterrupt			; FIQ vector
	pop 	{r1}
	vmsr    FPEXC, r1
	pop 	{r1}
	vmsr    FPSCR, r1
	vpop    {d16-d31}
	vpop    {d0-d7}
	ldmia	sp!,{r0-r3,r12,lr}		; state restore from compiled code
	subs	pc, lr, #4			; adjust return


Undefined					; Undefined handler
	stmdb	sp!,{r0-r3,r12,lr}		; state save from compiled code
	ldr     r0, =UndefinedExceptionAddr
	sub     r1, lr,#4
	str     r1, [r0]			; Address of instruction causing undefined exception
	bl	UndefinedException		; UndefinedException: call C function here
	ldmia	sp!,{r0-r3,r12,lr}		; state restore from compiled code

	movs	pc, lr


SVCHandler					; SWI handler
	stmdb	sp!,{r0-r3,r12,lr}		; state save from compiled code
	tst	r0, #0x20			; check the T bit
	ldrneh	r0, [lr,#-2]			; Thumb mode
	bicne	r0, r0, #0xff00			; Thumb mode
	ldreq	r0, [lr,#-4]			; ARM mode
	biceq	r0, r0, #0xff000000		; ARM mode
	bl	SWInterrupt			; SWInterrupt: call C function here
	ldmia	sp!,{r0-r3,r12,lr}		; state restore from compiled code
	movs	pc, lr				; adjust return

DataAbortHandler				; Data Abort handler
	stmdb	sp!,{r0-r3,r12,lr}		; state save from compiled code
	ldr     r0, =DataAbortAddr
	sub     r1, lr,#8
	str     r1, [r0]			;Address of instruction causing data abort
	bl	DataAbortInterrupt		;DataAbortInterrupt :call C function here
	ldmia	sp!,{r0-r3,r12,lr}		; state restore from compiled code
	subs	pc, lr, #8			; adjust return

PrefetchAbortHandler				; Prefetch Abort handler
	stmdb	sp!,{r0-r3,r12,lr}		; state save from compiled code
	ldr     r0, =PrefetchAbortAddr
	sub     r1, lr,#4
	str     r1, [r0]			;Address of instruction causing prefetch abort
	bl	PrefetchAbortInterrupt		; PrefetchAbortInterrupt: call C function here
	ldmia	sp!,{r0-r3,r12,lr}		; state restore from compiled code
	subs	pc, lr, #4			; adjust return

	END
