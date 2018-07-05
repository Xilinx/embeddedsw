/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/******************************************************************************
 * TMR Manager recovery routines:
 *   - Break Handler
 *   - Reset Handler
 *   - Initialize
 *******************************************************************************/

#include "xparameters.h"

#define REG_OFFSET(regnum) (4 * (regnum - 2))
#define NUM_TO_REG(num)    r ## num

#define PUSH_REG(regnum)   swi NUM_TO_REG(regnum), r1, REG_OFFSET(regnum)
#define POP_REG(regnum)    lwi NUM_TO_REG(regnum), r1, REG_OFFSET(regnum)

#define PUSH_SREG(name, regnum) \
        mfs     r2, name; \
        swi     r2, r1, REG_OFFSET(regnum)
#define POP_SREG(name, regnum) \
        lwi     r2, r1, REG_OFFSET(regnum); \
        mts     name, r2

#define MSR_REGNUM  32
#define SLR_REGNUM  33
#define SHR_REGNUM  34
#define PID_REGNUM  33
#define TLBX_REGNUM 34
#define ZPR_REGNUM  35
#define FSR_REGNUM  36
#define MAX_REGNUM  FSR_REGNUM

/*
 * _xtmr_manager_break - Handler for recovery break from the TMR Manager.
 *
 * Save stack pointer in global register.
 * Save all registers that represent the processor internal state.
 * Flush or invalidate all internal cached data: D-cache, I-cache, BTC and UTLB.
 * Call break handler in C code.
 * Suspend processor to signal TMR Manager that it should perform a reset.
 *
 * Handler notes:
 * - There is no need to save exception registers (EAR, ESR, BIP, EDR), since
 *   when the MSR EIP bit is set, break is blocked.
 *
 * STACK FRAME STRUCTURE
 * ---------------------
 *
 *      +-------------+         + 0
 *      |     r2      |
 *      |      .      |
 *      |      .      |
 *      |      .      |
 *      |      .      |
 *      |     r31     |
 *      +-------------+         + 120
 *      |     MSR     |
 *      +-------------+         + 124
 *      |     SRL     |
 *      |     SHR     |
 *      +-------------+         + 132
 *      |     PID     |
 *      |     TLBX    |
 *      |     ZPR     |
 *      +-------------+         + 144
 *      |     FSR     |
 *      +-------------+         + 148
 *      |      .      |
 *      |      .      |
 */

.global _xtmr_manager_break
.section .text
.align 2
.ent _xtmr_manager_break
.type _xtmr_manager_break, @function
_xtmr_manager_break:
	/* Save context to stack */
	addik r1, r1, -REG_OFFSET(MAX_REGNUM)
	swi r1, r0, XTMR_Manager_StackPointer
        PUSH_REG(2)
	PUSH_REG(3)
	PUSH_REG(4)
	PUSH_REG(5)
	PUSH_REG(6)
	PUSH_REG(7)
	PUSH_REG(8)
	PUSH_REG(9)
	PUSH_REG(10)
	PUSH_REG(11)
	PUSH_REG(12)
	PUSH_REG(13)
	PUSH_REG(14)
	PUSH_REG(15)
	PUSH_REG(16)
	PUSH_REG(17)
	PUSH_REG(18)
	PUSH_REG(19)
	PUSH_REG(20)
	PUSH_REG(21)
	PUSH_REG(22)
	PUSH_REG(23)
	PUSH_REG(24)
	PUSH_REG(25)
	PUSH_REG(26)
	PUSH_REG(27)
	PUSH_REG(28)
	PUSH_REG(29)
	PUSH_REG(30)
	PUSH_REG(31)
#if XPAR_MICROBLAZE_USE_MMU > 0
	/* Save MMU registers and clear UTLB */
	PUSH_SREG(rpid, PID_REGNUM)
	PUSH_SREG(rtlbx, TLBX_REGNUM)
	PUSH_SREG(rzpr, ZPR_REGNUM)
	addik r2,r0,63
utlb_clear_loop:
	mts rtlbx, r2
	mts rtlbhi, r0
	bgtid r2, utlb_clear_loop
	addik r2, r2, -1
#endif
#if XPAR_MICROBLAZE_USE_STACK_PROTECTION > 0
	/* Save stack protection registers */
	PUSH_SREG(rslr, SLR_REGNUM)
	PUSH_SREG(rshr, SHR_REGNUM)
#endif
#if XPAR_MICROBLAZE_USE_FPU > 0
	PUSH_SREG(rfsr, FSR_REGNUM)
#endif
	PUSH_SREG(rmsr, MSR_REGNUM)
#if XPAR_MICROBLAZE_USE_BRANCH_TARGET_CACHE > 0
	/* Invalidate Branch Target Cache */
	mbar 2
#endif
#if XPAR_MICROBLAZE_USE_DCACHE > 0
	/* Flush or invalidate the instruction cache */
#if XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK > 0
	bralid r15, microblaze_flush_dcache
#else
	bralid r15, microblaze_invalidate_dcache
#endif
	nop
#endif
#if XPAR_MICROBLAZE_USE_ICACHE > 0
	/* Invalidate the instruction cache */
	bralid r15, microblaze_invalidate_icache
	nop
#endif
	/* Call Break Handler in C code */
	lwi r5, r0, XTMR_Manager_InstancePtr
	bralid r15, XTMR_Manager_BreakHandler
	nop

	/* Suspend MicroBlaze to signal that a recovery reset should be done */
	suspend
	nop
	nop
	nop
	nop
.end _xtmr_manager_break


/*
 * _xtmr_manager_reset - Handler for recovery reset issued by TMR Manager.
 *
 * Restore stack pointer from global register.
 * Restore MSR to turn on caches.
 * Call reset handler in C code.
 * If C code returns 0, represnting cold reset, jump to saved cold reset vector.
 * Restore all registers that represent the processor internal state.
 * Return from break to resume execution.
 *
 */

.global _xtmr_manager_reset
.section .text
.align 2
.ent _xtmr_manager_reset
.type _xtmr_manager_reset, @function
_xtmr_manager_reset:
	/* Restore stack pointer */
	lwi r1, r0, XTMR_Manager_StackPointer

	/* Turn on caches if they are used */
	POP_SREG(rmsr, MSR_REGNUM)
	bri 4

	/* Call C-level reset handler */
	lwi r5, r0, XTMR_Manager_InstancePtr
	bralid r15, XTMR_Manager_ResetHandler
	nop

	/* Jump to cold reset vector */
	lwi r4, r0, XTMR_Manager_ColdResetVector
	beq r3, r4

	/* Restore context from stack and return from break */
#ifdef XPAR_MICROBLAZE_USE_MMU
	POP_SREG(rpid, PID_REGNUM)
	POP_SREG(rtlbx, TLBX_REGNUM)
	POP_SREG(rzpr, ZPR_REGNUM)
#endif
#ifdef XPAR_MICROBLAZE_USE_STACK_PROTECTION
	POP_SREG(rslr, SLR_REGNUM)
	POP_SREG(rshr, SHR_REGNUM)
#endif
#if XPAR_MICROBLAZE_USE_FPU > 0
	POP_SREG(rfsr, FSR_REGNUM)
#endif
	POP_SREG(rmsr, MSR_REGNUM)
	bri 4
        POP_REG(2)
	POP_REG(3)
	POP_REG(4)
	POP_REG(5)
	POP_REG(6)
	POP_REG(7)
	POP_REG(8)
	POP_REG(9)
	POP_REG(10)
	POP_REG(11)
	POP_REG(12)
	POP_REG(13)
	POP_REG(14)
	POP_REG(15)
	POP_REG(16)
	POP_REG(17)
	POP_REG(18)
	POP_REG(19)
	POP_REG(20)
	POP_REG(21)
	POP_REG(22)
	POP_REG(23)
	POP_REG(24)
	POP_REG(25)
	POP_REG(26)
	POP_REG(27)
	POP_REG(28)
	POP_REG(29)
	POP_REG(30)
	POP_REG(31)
	addik r1, r1, REG_OFFSET(MAX_REGNUM)

	/* Return from break to resume execution */
	rtbd r16, 0
	nop
.end _xtmr_manager_reset


/*
 * _xtmr_manager_initialize - Initialize break and reset vector.
 *
 * Save original cold reset vector to global variables.
 * Set up reset vector to branch to _xtmr_manager_reset.
 * Set up break vector to branch to _xtmr_manager_break.
 *
 */

.global _xtmr_manager_initialize
.section .text
.align 2
.ent _xtmr_manager_initialize
.type _xtmr_manager_initialize, @function
_xtmr_manager_initialize:
	addik r1, r1, -12
	swi r6, r1, 0
	swi r7, r1, 4
	swi r8, r1, 8

	/* Save instance pointer */
	swi r5, r0, XTMR_Manager_InstancePtr

	/* Save cold reset vector */
	addik r8, r0, XPAR_MICROBLAZE_BASE_VECTORS
	lwi r6, r8, 0
	lwi r7, r8, 4
	swi r6, r0, XTMR_Manager_ColdResetVector+0
	swi r7, r0, XTMR_Manager_ColdResetVector+4

	/* Change reset vector */
	ori r6, r0, _xtmr_manager_reset
	bsrli r6, r6, 16
	ori r6, r6, 0xb0000000
	ori r7, r0, _xtmr_manager_reset
	andi r7, r7, 0xffff
	ori r7, r7, 0xb8080000
	swi r6, r8, 0
	swi r7, r8, 4

	/* Initialize break vector */
	ori r6, r0, _xtmr_manager_break
	bsrli r6, r6, 16
	ori r6, r6, 0xb0000000
	ori r7, r0, _xtmr_manager_break
	andi r7, r7, 0xffff
	ori r7, r7, 0xb8080000
	swi r6, r8, 0x14
	swi r7, r8, 0x18

	lwi r6, r1, 0
	lwi r7, r1, 4
	lwi r8, r1, 8
	addik r1, r1, 12

	/* Clear MSR BIP by performing an RTBD instead of RTSD */
	rtbd r15, 8
	nop
.end _xtmr_manager_initialize

/* Declarations of global variables used by the recovery functionality */
.section .data
.align 2
.global XTMR_Manager_ColdResetVector
.global XTMR_Manager_StackPointer
.global XTMR_Manager_InstancePtr
XTMR_Manager_ColdResetVector:
	.long 0
	.long 0
XTMR_Manager_StackPointer:
	.long 0
XTMR_Manager_InstancePtr:
	.long 0
