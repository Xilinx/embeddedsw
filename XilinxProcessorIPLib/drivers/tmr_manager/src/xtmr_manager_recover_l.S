/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/******************************************************************************
 * TMR Manager recovery routines:
 *   - Break Handler
 *   - Reset Handler
 *   - Initialize
 *******************************************************************************/

#include "xparameters.h"

#ifdef __riscv

/******************************************************************************
 * RISC-V assembler implementation
 *******************************************************************************/
#if __riscv_xlen == 64
#error "TMR Manager RISC_V driver does not support XLEN 64"
#endif

#define REG_OFFSET(regnum)   (4 * (regnum - 1))
#define NUM_TO_REG(num)      x ## num

#define PUSH_REG(regnum)     sw NUM_TO_REG(regnum), REG_OFFSET(regnum)(sp)
#define POP_REG(regnum)	     lw NUM_TO_REG(regnum), REG_OFFSET(regnum)(sp)

#define PUSH_CSR(name, regnum) \
        csrr    t0, name; \
        sw      t0, REG_OFFSET(regnum)(sp)
#define POP_CSR(name, regnum) \
        lw      t0, REG_OFFSET(regnum)(sp); \
        csrw    name, t0

#define MSTATUS_REGNUM       31
#define MIE_REGNUM           32
#define MTVEC_REGNUM         33
#define MSCRATCH_REGNUM      34
#define MEPC_REGNUM          35
#define MWFI_REGNUM          36
#define MAX_REGNUM           MWFI_REGNUM

#define MWFI                 0x7c4

#define FPREG_OFFSET(regnum) (4 * (regnum + MWFI_REGNUM + 1))
#define NUM_TO_FPREG(num)    f ## num

#define PUSH_FPREG(regnum)   fsw NUM_TO_FPREG(regnum), FPREG_OFFSET(regnum)(sp)
#define POP_FPREG(regnum)    flw NUM_TO_FPREG(regnum), FPREG_OFFSET(regnum)(sp)

#define FCSR_REGNUM          (MWFI_REGNUM + 33)

#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_USE_FPU == 1)
#define HANDLER_STACK_SIZE   (4 * (MWFI_REGNUM + 34))
#else
#define HANDLER_STACK_SIZE   (4 * (MWFI_REGNUM + 1))
#endif

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
 * - There is no need to save exception registers (mcause, mtval), since
 *   when an exception handler is executing, the break interrupt is blocked.
 *
 * STACK FRAME STRUCTURE
 * ---------------------
 *
 *      +-------------+
 *      |     x1      |         + 0
 *      |      .      |
 *      |      .      |
 *      |      .      |
 *      |      .      |
 *      |     x31     |
 *      +-------------+         + 128
 *      |   mstatus   |
 *      |     mie     |
 *      |    mtvec    |
 *      |   mscratch  |
 *      |     mepc    |
 *      |     mwfi    |
 *      +-------------+         + 146
 *      |     f0      |
 *      |      -      |
 *      |     f31     |
 *      |     fcsr    |
 *      +-------------+         + 282
 *      |      .      |
 */

.global _xtmr_manager_break
.section .text
.align 2
.type _xtmr_manager_break, @function
_xtmr_manager_break:
	/* Save context to stack */
	addi  sp, sp, -(HANDLER_STACK_SIZE)
	PUSH_REG(6)

	/* Check for external break interrupt */
	csrr  t1, mcause
	bgez  t1, _xtmr_manager_is_other_trap
	andi  t1, t1, 0xf
	bnez  t1, _xtmr_manager_is_other_trap

_xtmr_manager_is_break:
	sw    sp, XTMR_Manager_StackPointer, t1
        PUSH_REG(1)
	PUSH_REG(3)
	PUSH_REG(4)
	PUSH_REG(5)
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
	PUSH_CSR(mstatus,  MSTATUS_REGNUM)
	PUSH_CSR(mie,      MIE_REGNUM)
	PUSH_CSR(mtvec,    MTVEC_REGNUM)
	PUSH_CSR(mscratch, MSCRATCH_REGNUM)
	PUSH_CSR(MWFI,     MWFI_REGNUM)
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_USE_FPU == 1)
        PUSH_FPREG(0)
        PUSH_FPREG(1)
	PUSH_FPREG(3)
	PUSH_FPREG(4)
	PUSH_FPREG(5)
	PUSH_FPREG(6)
	PUSH_FPREG(7)
	PUSH_FPREG(8)
	PUSH_FPREG(9)
	PUSH_FPREG(10)
	PUSH_FPREG(11)
	PUSH_FPREG(12)
	PUSH_FPREG(13)
	PUSH_FPREG(14)
	PUSH_FPREG(15)
	PUSH_FPREG(16)
	PUSH_FPREG(17)
	PUSH_FPREG(18)
	PUSH_FPREG(19)
	PUSH_FPREG(20)
	PUSH_FPREG(21)
	PUSH_FPREG(22)
	PUSH_FPREG(23)
	PUSH_FPREG(24)
	PUSH_FPREG(25)
	PUSH_FPREG(26)
	PUSH_FPREG(27)
	PUSH_FPREG(28)
	PUSH_FPREG(29)
	PUSH_FPREG(30)
	PUSH_FPREG(31)
	PUSH_CSR(fcsr,     FCSR_REGNUM)
#endif
#if XPAR_MICROBLAZE_RISCV_USE_BRANCH_TARGET_CACHE > 0
	/* Invalidate Branch Target Cache */
	fence.i
#endif
#if XPAR_MICROBLAZE_RISCV_USE_DCACHE > 0
	/* Flush or invalidate the data cache */
#if XPAR_MICROBLAZE_RISCV_DCACHE_USE_WRITEBACK > 0
	jal microblaze_riscv_flush_dcache
#else
	jal microblaze_riscv_invalidate_dcache
#endif
#endif
#if XPAR_MICROBLAZE_RISCV_USE_ICACHE > 0
	/* Invalidate the instruction cache */
	jal microblaze_riscv_invalidate_icache
#endif
	/* Call Break Handler in C code */
	lw a0, XTMR_Manager_InstancePtr
	jal XTMR_Manager_BreakHandler

	/* Suspend to signal that a recovery reset should be done */
	csrwi MWFI, 4
	wfi
	nop
	nop
	nop
	nop

	/* Should never get here due to recovery reset */
	j     0

_xtmr_manager_is_other_trap:
	/* Jump to original trap handler */
	PUSH_CSR(mepc, MSCRATCH_REGNUM)
	la    t1, _xtmr_manager_other_trap_end
	csrw  mepc, t1
	addi  sp, sp, HANDLER_STACK_SIZE
	lw    t1, XTMR_Manager_ColdResetVector+4
	jr    t1
_xtmr_manager_other_trap_end:
	POP_CSR(mepc, MEPC_REGNUM)
	POP_REG(6)
	addi  sp, sp, HANDLER_STACK_SIZE
	mret
/* end _xtmr_manager_break */


/*
 * _xtmr_manager_reset - Handler for recovery reset issued by TMR Manager.
 *
 * Restore stack pointer from global register.
 * Call reset handler in C code.
 * If C code returns 0, represnting cold reset, jump to saved cold reset vector.
 * Restore all registers that represent the processor internal state.
 * Return from break to resume execution.
 *
 */

.global _xtmr_manager_reset
.section .text
.align 2
.type _xtmr_manager_reset, @function
_xtmr_manager_reset:
	/* Restore stack pointer */
	lw   sp, XTMR_Manager_StackPointer

	/* Call C-level reset handler */
	lw   a0, XTMR_Manager_InstancePtr
	jal  XTMR_Manager_ResetHandler

	/* Jump to cold reset vector */
	lw   t1, XTMR_Manager_ColdResetVector
	bnez t1, _xtmr_manager_is_warm_reset
	jr   t1

_xtmr_manager_is_warm_reset:
	/* Restore context from stack and return from break */
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_USE_FPU == 1)
	POP_CSR(fcsr, FCSR_REGNUM)
        POP_FPREG(0)
        POP_FPREG(1)
	POP_FPREG(3)
	POP_FPREG(4)
	POP_FPREG(5)
	POP_FPREG(6)
	POP_FPREG(7)
	POP_FPREG(8)
	POP_FPREG(9)
	POP_FPREG(10)
	POP_FPREG(11)
	POP_FPREG(12)
	POP_FPREG(13)
	POP_FPREG(14)
	POP_FPREG(15)
	POP_FPREG(16)
	POP_FPREG(17)
	POP_FPREG(18)
	POP_FPREG(19)
	POP_FPREG(20)
	POP_FPREG(21)
	POP_FPREG(22)
	POP_FPREG(23)
	POP_FPREG(24)
	POP_FPREG(25)
	POP_FPREG(26)
	POP_FPREG(27)
	POP_FPREG(28)
	POP_FPREG(29)
	POP_FPREG(30)
	POP_FPREG(31)
#endif
	POP_CSR(mstatus,  MSTATUS_REGNUM)
	POP_CSR(mtvec,    MTVEC_REGNUM)
	POP_CSR(mie,      MIE_REGNUM)
	POP_CSR(mscratch, MSCRATCH_REGNUM)
	POP_CSR(MWFI,     MWFI_REGNUM)
        POP_REG(1)
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
	addi  sp, sp, (HANDLER_STACK_SIZE)

	/* Return from external break interrupt to resume execution */
	mret
/* end _xtmr_manager_reset */


/*
 * _xtmr_manager_initialize - Initialize break and reset vector.
 *
 * Save original cold reset vector to global variables.
 * Set up reset vector to branch to _xtmr_manager_reset.
 * Set up break interrupt to branch to _xtmr_manager_break.
 *
 */

.global _xtmr_manager_initialize
.section .text
.align 2
.type _xtmr_manager_initialize, @function
_xtmr_manager_initialize:
	addi sp, sp, -16
	sw   t1, 0(sp)
	sw   t2, 4(sp)
	sw   t3, 8(sp)
	sw   t4, 8(sp)

	/* Save instance pointer */
	sw   a0, XTMR_Manager_InstancePtr, t3

	/* Save cold reset vector */
	la   t2, XPAR_MICROBLAZE_RISCV_BASE_VECTORS
	lw   t1, 0(t2)
	sw   t1, XTMR_Manager_ColdResetVector+0, t3

	/* Save mtvec */
	csrr t1, mtvec
	sw   t1, XTMR_Manager_ColdResetVector+4, t3

	/* Change reset vector */
	la   t3, _xtmr_manager_reset
	/*   offset bit 19:12 = imm[19:12] */
	li   t4, 0xff000
	and  t1, t3, t4
	/*   offset bit 20 = imm[11] */
	li   t4, 0x800
	and  t4, t3, t4
        slli t4, t4, 9
	or   t1, t1, t4
        /*   offset bit 30:21 = imm[10:1] */
	andi t4, t3, 0x7fe
        slli t4, t4, 20
	or   t1, t1, t4
	/*   offset bit 31 = imm[20] */
	li   t4, 0x100000
	and  t4, t3, t4
        slli t4, t4, 11
	or   t1, t1, t4
        /*   "jal x0, offset" = offset | 0x6f */
	ori  t1, t1, 0x6f
	sw   t1, 0(t2)

	/* Initialize external break interrupt */
	la   t1, _xtmr_manager_break
	csrw mtvec, t1

	/* Enable external break interrupt */
	csrr   t1, mie
	li     t4, 0x10000
	add    t1, t1, t4
	csrw   mie, t1

	addi sp, sp, 16
	ret
/* end _xtmr_manager_initialize */

#else /* __riscv */

/******************************************************************************
 * MicroBlaze assembler implementation
 *******************************************************************************/

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
	/* Flush or invalidate the data cache */
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
	swi r6, r8, 0x18
	swi r7, r8, 0x1c

	lwi r6, r1, 0
	lwi r7, r1, 4
	lwi r8, r1, 8
	addik r1, r1, 12

	/* Clear MSR BIP by performing an RTBD instead of RTSD */
	rtbd r15, 8
	nop
.end _xtmr_manager_initialize

#endif /* ! __riscv */

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
