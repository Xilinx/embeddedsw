/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/******************************************************************************
 * RISC-V Trap Handler
 * - Non self-modifying trap handler for the following trap conditions
 *   - Misaligned memory access
 *******************************************************************************/

#include "xparameters.h"

/* RV32E has no F/D extension support; FPU must not be enabled for it */
#if defined(__riscv_32e) && defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && \
    (XPAR_MICROBLAZE_RISCV_USE_FPU > 0)
#error "XPAR_MICROBLAZE_RISCV_USE_FPU must be 0 for an RV32E processor; RV32E does not support the F/D floating-point extension"
#endif

/* 64-bit definitions */
#if __riscv_xlen == 64
#define INTPTR_DATAITEM			.quad
#define REGSIZE				8
#define DATAALIGN			3
#define SHIFT				4
#define STORE				sd
#define LOAD				ld
#else
#define INTPTR_DATAITEM			.long
#define REGSIZE				4
#define DATAALIGN			2
#define SHIFT				3
#define STORE				sw
#define LOAD				lw
#endif /* 64-bit definitions */

/* FPU definitions
 * Note: RV32E does not permit F or D extensions, so FPU is always absent
 * when __riscv_32e is defined. The XPAR guard below naturally evaluates to
 * the "else" branch (FPREGSIZE=0, FPREGCOUNT=0) in that case.
 */
#ifdef __riscv_32e
#define REGCOUNT			16
#else
#define REGCOUNT			32
#endif

#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
#define FPREGSIZE			8
#define FPREGCOUNT			33
#define STOREFP				fsd
#define LOADFP				fld
#elif defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 1)
#define FPREGSIZE			4
#define FPREGCOUNT			33
#define STOREFP				fsw
#define LOADFP				flw
#else
#define FPREGSIZE			0
#define FPREGCOUNT			0
#endif

/*
 * FPU_MISALIGN_SUPPORT (FPU=1 or FPU=2 in xparameters)
 *
 * The trap handler already fixes misaligned integer loads and stores by reading
 * or writing memory one byte at a time. When the FPU is enabled, the same logic
 * also covers misaligned floating-point loads and stores (flw/fsw, and fld/fsd
 * where the core supports them).
 *
 * Integer and FP instructions use the same "size" field in the encoding, so we
 * cannot tell them apart from size alone. We look at the instruction opcode and
 * set t6 on entry: 0 = integer (result goes to xN), 1 = FP (result goes to fN).
 */
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU > 0)
#define FPU_MISALIGN_SUPPORT		1
#define OPCODE_LOAD_FP			0x07	/* LOAD-FP class: flw, fld */
#define OPCODE_STORE_FP			0x27	/* STORE-FP class: fsw, fsd */
#endif

/*
 * RV32D_MISALIGN_SUPPORT (32-bit core, FPU=2 / double-precision)
 *
 * A double is 8 bytes but a general-purpose register is only 4 bytes. For
 * misaligned fld/fsd we rebuild all 8 bytes in the scratch area data_0..data_7,
 * then use fld or fsd to move the value between that buffer and the FP register.
 * We do not use fmv.d.x/fmv.x.d here (those need the D extension in the asm ISA).
 */
#if __riscv_xlen == 32 && defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && \
    (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
#define RV32D_MISALIGN_SUPPORT		1
#endif

/*
 * Compressed-instruction builds: FP dispatch tables use 8-byte entries, but the
 * decoded register index in t3 is scaled for 4-byte entries — double it before
 * indexing the FP tables.
 */
#if defined(FPU_MISALIGN_SUPPORT) && __riscv_compressed
#define RESCALE_FP_JUMP_INDEX(reg)	slli reg, reg, 1
#else
#define RESCALE_FP_JUMP_INDEX(reg)
#endif

/* Stack macros: payload matches saved GPR+FPU slots; allocation 16-byte aligned for ABI. */
#define HANDLER_FRAME_PAYLOAD		(REGSIZE * REGCOUNT + FPREGSIZE * FPREGCOUNT)
#define HANDLER_STACK_SIZE		(((HANDLER_FRAME_PAYLOAD) + 15) & ~15)

#define REG_OFFSET(regnum)		(REGSIZE * regnum)
#define NUM_TO_REG(num)			x ## num
#define PUSH_REG(regnum)		STORE NUM_TO_REG(regnum), REG_OFFSET(regnum)(sp)
#define POP_REG(regnum)			LOAD  NUM_TO_REG(regnum), REG_OFFSET(regnum)(sp)

#define FPREG_OFFSET(regnum)		(FPREGSIZE * regnum + REGSIZE * REGCOUNT)
#define NUM_TO_FPREG(num)		f ## num
#define PUSH_FPREG(regnum)		STOREFP NUM_TO_FPREG(regnum), FPREG_OFFSET(regnum)(sp)
#define POP_FPREG(regnum)		LOADFP  NUM_TO_FPREG(regnum), FPREG_OFFSET(regnum)(sp)

/*
 * RV32E register remapping for misaligned handlers
 *
 * RV32E only has x0-x15. The ABI temporaries t3(x28), t4(x29), t5(x30)
 * do not exist. We remap them to a3(x13), a4(x14), a5(x15) which are
 * available caller-saved registers. These are saved on the stack at trap
 * entry and restored at trap exit, exactly as t3/t4/t5 are in RV32I.
 *
 * All uses of t3, t4, t5 throughout handle_load_misalign,
 * handle_store_misalign, and the jump table macros automatically resolve
 * to the remapped registers via these aliases.
 */
#ifdef __riscv_32e
#define t3	a3
#define t4	a4
#define t5	a5
#endif

/* Misaligned handler macros
 *
 * T4_TO_STACK / T4_FROM_STACK:
 *   In RV32I  t4=x29 is used as the data scratch register.
 *   In RV32E  the alias above maps t4 -> a4(x14), so these macros
 *   continue to work correctly without any further changes.
 *
 * T4_TO_REG_V / REG_TO_T4_V:
 *   Used when the destination/source register is one that was saved early
 *   on the stack (t1=x6, t2=x7 in both ISAs; additionally a3/a4/a5 in
 *   RV32E). The value is written to / read from the stack slot so that the
 *   final early-register pop restores the correct value.
 */
#define T4_TO_STACK(regnum)		STORE t4, REG_OFFSET(regnum)(sp)
#define T4_FROM_STACK(regnum)		LOAD  t4, REG_OFFSET(regnum)(sp)

#ifdef RISCV_MODE_SWITCH
/* Synchronous ecall raised by user mode (RISC-V standard mcause encoding). */
#define EXCEPTION_ECALL_UMODE		8
#endif
#define T4_TO_REG_Z(regnum)	   \
	.align 2;		   \
	nop;			   \
	j  misalign_done

#define T4_TO_REG_V(regnum)	   \
	.align 2;		   \
	T4_TO_STACK(regnum);	   \
	j  misalign_done

#define T4_TO_REG(regnum)	   \
	.align 2;		   \
	mv NUM_TO_REG(regnum), t4; \
	j  misalign_done

#define REG_TO_T4_Z(regnum)	   \
	.align 2;		   \
	li t4, 0;		   \
	j  store_tail

#define REG_TO_T4_V(regnum)	   \
	.align 2;		   \
	T4_FROM_STACK(regnum);	   \
	j  store_tail

#define REG_TO_T4(regnum)	   \
	.align 2;		   \
	mv t4, NUM_TO_REG(regnum); \
	j  store_tail

#ifdef FPU_MISALIGN_SUPPORT
/*
 * FP register placement macros (jump-table entries).
 *
 * Single (flw/fsw): 4-byte value in GPR scratch t4, move via fmv.w.x / fmv.x.w
 * Double (fld/fsd on RV32D): 8 bytes in data_0..7. Before indexing
 * load_table_fp_d / store_table_fp_d, set t5 to &data_0; each entry is a
 * single fld/fsd 0(t5) plus j (8-byte stride, same as load_table_fp).
 *
 * t6 must already be 1 (FP instruction) when these tables are used.
 */
#define T4_TO_FPREG(regnum)	   \
	.align 2;		   \
	fmv.w.x NUM_TO_FPREG(regnum), t4; /* 4-byte FP load result into fN */ \
	j  misalign_done

#define REG_TO_T4_FP(regnum)	   \
	.align 2;		   \
	fmv.x.w t4, NUM_TO_FPREG(regnum); /* 4-byte FP store source from fN */ \
	j  store_tail

#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
#define T4_TO_FPREG_D(regnum)	   \
	.align 2;		   \
	fld NUM_TO_FPREG(regnum), 0(t5); /* t5 = &data_0 before table dispatch */ \
	j  misalign_done

#define REG_TO_T4_FP_D(regnum)	   \
	.align 2;		   \
	fsd NUM_TO_FPREG(regnum), 0(t5); \
	j  store_double_scatter
#endif /* double-precision FP */
#endif /* FPU_MISALIGN_SUPPORT */

/* Extern declarations */
.extern XNullHandler
#ifdef RISCV_MODE_SWITCH
.extern Xil_ModeSwitchDispatchFromUser
.extern Xil_ModeSwitchAbort
#endif


#ifndef RISCV_TRAPS_DISABLED				/* If traps are disabled in the processor */

/*
 * trap_handler - Handler for interrupts and exceptions
 *
 * Trap handler notes:
 * - Does not handle traps other than misaligned memory access
 */

.global _trap_handler
.section .text
.align 2
.type _trap_handler, @function
_trap_handler:
#ifdef RISCV_MODE_SWITCH
	/*
	 * Only swap to the M-side scratch anchor for a trap that actually
	 * interrupted U-mode (MPP == U). mscratch is only kept valid while a
	 * session is genuinely active (Enter/Resume set it fresh right before
	 * mret); it is never refreshed while the monitor's own M-mode code
	 * runs afterward. A trap from M's own code - the monitor between
	 * sessions, or a fault inside a registered ISR while U-dispatch is
	 * already running on the anchor - must use its own live sp directly,
	 * exactly like an ordinary (non-mode-switch) build. Swapping
	 * unconditionally here would silently corrupt live M-mode stack
	 * frames once the M SP has drifted since the anchor was last set, or
	 * double-use the same anchor for a nested M-mode fault.
	 * t0 is not yet saved anywhere - safe to use since M-only builds never
	 * reach this block, and a genuine U-mode trap has not touched any GPR
	 * yet either.
	 */
	csrr	t0, mstatus
	srli	t0, t0, 11
	andi	t0, t0, 3
	bnez	t0, 1f
	csrrw	sp, mscratch, sp
1:
#endif
	addi		sp, sp, -(HANDLER_STACK_SIZE)	/* Create stack frame */
	PUSH_REG(6)					/* Save t1 = x6 */
	PUSH_REG(7)					/* Save t2 = x7 */
#ifdef RISCV_MODE_SWITCH
	/* t0 (MPP check from entry) survives PUSH_REG - neither touches x5/t0. */
	bnez	t0, ms_skip_x2_save
	csrr	t2, mscratch			/* U-mode trap: recover interrupted sp */
	STORE	t2, REG_OFFSET(2)(sp)
ms_skip_x2_save:
#endif
	/*
	 * Save the three scratch registers used by the misalign handlers.
	 *
	 * RV32I: t3=x28, t4=x29, t5=x30 are the natural ABI temporaries.
	 * RV32E: those registers do not exist; a3=x13, a4=x14, a5=x15 are
	 *        used instead (via the t3/t4/t5 aliases defined above).
	 *        PUSH_REG(28/29/30) would reference non-existent registers,
	 *        so we push x13/x14/x15 instead. These are NOT pushed again
	 *        in the full-save block below, matching the way x28/x29/x30
	 *        are omitted from the full-save block in RV32I.
	 */
#ifdef __riscv_32e
	PUSH_REG(13)					/* Save a3 = x13 (used as t3 substitute) */
	PUSH_REG(14)					/* Save a4 = x14 (used as t4 substitute) */
	PUSH_REG(15)					/* Save a5 = x15 (used as t5 substitute) */
#else
	PUSH_REG(28)					/* Save t3 = x28 */
	PUSH_REG(29)					/* Save t4 = x29 */
	PUSH_REG(30)					/* Save t5 = x30 */
#endif
#if defined(FPU_MISALIGN_SUPPORT) && !defined(__riscv_32e)
	PUSH_REG(31)					/* Save t6 (x31): 0=int load/store, 1=FP */
#endif
	csrr		t1, mcause
#ifdef RISCV_MODE_SWITCH
	/*
	 * A misaligned load/store taken while the interrupted context was in
	 * U-mode is not emulated (v1 policy) - abort the guest session instead
	 * of running the legacy fixup below against guest-controlled state.
	 * t0 still holds the MPP check from entry (PUSH_REG(28/29/30) don't
	 * touch x5/t0 either); zero means the trap arrived from U.
	 */
	li		t2, 6				/* Store address misaligned */
	bne		t1, t2, ms_check_load_misalign
	beqz	t0, ms_abort_u_fault
ms_check_load_misalign:
	li		t2, 4				/* Load address misaligned */
	bne		t1, t2, ms_no_misalign
	beqz	t0, ms_abort_u_fault
ms_no_misalign:
#endif
	li		t2, 6				/* Store address misaligned */
	beq		t1, t2, handle_store_misalign
	li		t2, 4				/* Load address misaligned */
	beq		t1, t2, handle_load_misalign

	/* Full register save for interrupt/exception dispatch */
	PUSH_REG(1)
	PUSH_REG(3)
	PUSH_REG(4)
	PUSH_REG(5)
	PUSH_REG(8)
	PUSH_REG(9)
	PUSH_REG(10)
	PUSH_REG(11)
	PUSH_REG(12)
	/*
	 * RV32I:  x13/x14/x15 are NOT saved early, push them now.
	 * RV32E:  x13/x14/x15 were already pushed as scratch substitutes
	 *         above; skipping them here avoids double-save and keeps
	 *         each register in its correct, single stack slot.
	 */
#ifndef __riscv_32e
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
	PUSH_REG(31)
	/* x28/x29/x30 already saved as t3/t4/t5 above - not repeated */
#endif /* !__riscv_32e */
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU > 0)
	PUSH_FPREG(0)					/* Save f0 to f31 */
	PUSH_FPREG(1)
	PUSH_FPREG(2)
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
	csrr		t2, fcsr			/* Save fcsr */
	sw		t2, FPREG_OFFSET(32)(sp)
#endif
#ifdef RISCV_MODE_SWITCH
	li		t2, EXCEPTION_ECALL_UMODE
	beq		t1, t2, handle_ecall_umode
#endif
	la		t2, RISCV_InterruptVectorTable	/* Load interrupt vector table base address */
	bltz		t1, is_interrupt
	la		t2, RISCV_ExceptionVectorTable	/* Load exception vector table base address */
is_interrupt:
	slli		t1, t1, SHIFT			/* Calculate vector offset = t1 * SHIFT */
	add		t2, t2, t1			/* Get pointer to vector */
	LOAD		a0, REGSIZE(t2)			/* Load argument to handler from vector */
	LOAD		t2, 0(t2)			/* Load vector itself from vector */
	jalr		t2				/* Jump to handler: ra = x1 = link address */
.global trap_restore
.type trap_restore, @function
trap_restore:
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU > 0)
	lw		t2, FPREG_OFFSET(32)(sp)	/* Restore fcsr */
	csrw		fcsr, t2
	POP_FPREG(31)					/* Restore f0 to f31 */
	POP_FPREG(30)
	POP_FPREG(29)
	POP_FPREG(28)
	POP_FPREG(27)
	POP_FPREG(26)
	POP_FPREG(25)
	POP_FPREG(24)
	POP_FPREG(23)
	POP_FPREG(22)
	POP_FPREG(21)
	POP_FPREG(20)
	POP_FPREG(19)
	POP_FPREG(18)
	POP_FPREG(17)
	POP_FPREG(16)
	POP_FPREG(15)
	POP_FPREG(14)
	POP_FPREG(13)
	POP_FPREG(12)
	POP_FPREG(11)
	POP_FPREG(10)
	POP_FPREG(9)
	POP_FPREG(8)
	POP_FPREG(7)
	POP_FPREG(6)
	POP_FPREG(5)
	POP_FPREG(4)
	POP_FPREG(3)
	POP_FPREG(2)
	POP_FPREG(1)
	POP_FPREG(0)
#endif
	/* Full register restore - mirror of full save above */
#ifndef __riscv_32e
	POP_REG(31)
	POP_REG(27)
	POP_REG(26)
	POP_REG(25)
	POP_REG(24)
	POP_REG(23)
	POP_REG(22)
	POP_REG(21)
	POP_REG(20)
	POP_REG(19)
	POP_REG(18)
	POP_REG(17)
	POP_REG(16)
	POP_REG(15)
	POP_REG(14)
	POP_REG(13)
	/* x28/x29/x30 restored below in the common early-reg pop */
#endif /* !__riscv_32e */
	POP_REG(12)
	POP_REG(11)
	POP_REG(10)
	POP_REG(9)
	POP_REG(8)
	POP_REG(5)
	POP_REG(4)
	POP_REG(3)
	POP_REG(1)
misalign_done:
#ifdef RISCV_MODE_SWITCH
	/*
	 * Re-derive the entry-time MPP check fresh - t0 does not reliably
	 * survive a registered handler/dispatch call in between, unlike the
	 * earlier reuses which only cross PUSH_REG (which never clobbers it).
	 * A U-mode trap swapped at entry and must swap back; an M-mode trap
	 * never swapped and must not swap back either.
	 */
	csrr	t0, mstatus
	srli	t0, t0, 11
	andi	t0, t0, 3
	bnez	t0, ms_epilogue_no_recover
	LOAD	t1, REG_OFFSET(2)(sp)			/* Recover interrupted sp = x2 */
	csrw	mscratch, t1
ms_epilogue_no_recover:
#endif
	/*
	 * Restore the three scratch registers saved at entry.
	 * RV32I:  t5=x30, t4=x29, t3=x28
	 * RV32E:  a5=x15, a4=x14, a3=x13 (via the aliases above)
	 */
#ifdef __riscv_32e
	POP_REG(15)					/* Restore a5 = x15 (t5 substitute) */
	POP_REG(14)					/* Restore a4 = x14 (t4 substitute) */
	POP_REG(13)					/* Restore a3 = x13 (t3 substitute) */
#else
	POP_REG(30)					/* Restore t5 = x30 */
	POP_REG(29)					/* Restore t4 = x29 */
	POP_REG(28)					/* Restore t3 = x28 */
#endif
#if defined(FPU_MISALIGN_SUPPORT) && !defined(__riscv_32e)
	POP_REG(31)					/* Restore t6 (integer vs FP flag) */
#endif
	POP_REG(7)					/* Restore t2 = x7 */
	POP_REG(6)					/* Restore t1 = x6 */
	addi		sp, sp, (HANDLER_STACK_SIZE)	/* Restore stack frame */
#ifdef RISCV_MODE_SWITCH
	bnez	t0, ms_epilogue_no_swapback
	csrrw	sp, mscratch, sp			/* Restore application stack */
ms_epilogue_no_swapback:
#endif
	mret

#ifdef RISCV_MODE_SWITCH
/*
 * Unhandled synchronous fault (illegal instruction, access fault, or a
 * misaligned load/store per the ms_no_misalign check above) taken from
 * U-mode: abort the guest session and return to the M-mode caller instead
 * of falling through to the ordinary vector-table dispatch. mcause/mepc/
 * mtval are still live CSRs at this point - nothing has overwritten them.
 */
ms_abort_u_fault:
	mv	a0, t1
	csrr	a1, mepc
	csrr	a2, mtval
	call	Xil_ModeSwitchAbort
1:	j	1b				/* Xil_ModeSwitchAbort is noreturn */

handle_ecall_umode:
	mv		a0, sp			/* a0 points to the saved x0-x31 GPR frame */
	call	Xil_ModeSwitchDispatchFromUser
	j	trap_restore
#endif

/*
 * Load address misalign exception handler
 *
 * Register roles during emulation:
 *   t1 = faulting memory address (from mtval)
 *   t2 = transfer size in bytes (from instruction funct3)
 *   t3 = destination register index * stride (for jump table)
 *   t4 = reconstructed data value / temporary address
 *   t5 = scratch (instruction fetch, data_0..3 byte store base)
 *   t6 = is_fp_insn (1 = flw/fld or compressed FP load, 0 = lw/lh/…)
 *
 * Data path (after instruction decode):
 *   1. Byte-read from fault address into data_0.. (width = t2)
 *   2. Assemble into t4 (4-byte) or data_0..7 (8-byte)
 *   3. load_tail: if t6==0 write t4 to xN (load_table), else to fN (load_table_fp*)
 *
 * RV32I: t3=x28, t4=x29, t5=x30
 * RV32E: t3/t4/t5 aliased to a3/a4/a5 — see remapping above.
 */

handle_load_misalign:
	csrr	t1, mtval				/* Get faulting virtual address */
	csrr	t3, mepc				/* Get faulting instruction address */
#if __riscv_compressed
	lhu	t5, 0(t3)				/* Fetch 16 low bits of faulting instruction */
	andi	t2, t5, 3				/* Extract op */
	slli	t2, t2, SHIFT-1
	la	t4, load_op_table			/* Compute address to load op label */
	add	t4, t4, t2
	LOAD	t4, 0(t4)				/* Load label itself from address */
	jr	t4					/* Jump to load op label */
load_00: /* Compressed instruction C.LW, C.LD, C.FLW, C.FLD with rd' (bit 4:2) */
	addi	t3, t3, 2				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */

	srli	t2, t5, 13				/* Extract funct3 two LSB */
	andi	t2, t2, 3
	la	t4, transfer_size_table			/* Compute address to transfer size */
	add	t4, t4, t2
	lbu	t2, 0(t4)				/* Fetch transfer size */

#ifdef FPU_MISALIGN_SUPPORT
	/* Compressed FP load: C.FLD (funct3=001), C.FLW on RV32 (funct3=011) */
	srli	t6, t5, 13
	andi	t6, t6, 7
	li	t4, 1
	beq	t6, t4, 1f				/* C.FLD / C.FLDSP */
#if __riscv_xlen == 32
	li	t4, 3
	beq	t6, t4, 1f				/* C.FLW / C.FLWSP */
#endif
	li	t6, 0					/* C.LW / C.LWSP — integer */
	j	2f
1:	li	t6, 1					/* FP compressed load */
2:
#endif /* FPU_MISALIGN_SUPPORT */
	addi	t3, t5, 8*4				/* Extract rd destination register * 4 */
	j	load_continue
load_01: /* Non-load compressed instruction */
	j	misalign_done				/* Should never be misaligned so just return */
load_10: /* Compressed instruction C.LWSP, C.LDSP, C.FLWSP, C.FLDSP with rd (bit 11:7) */
	addi	t3, t3, 2				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */

	srli	t2, t5, 13				/* Extract funct3 two LSB */
	andi	t2, t2, 3
	la	t4, transfer_size_table			/* Compute address to transfer size */
	add	t4, t4, t2
	lbu	t2, 0(t4)				/* Fetch transfer size */

#ifdef FPU_MISALIGN_SUPPORT
	/* Same FP vs integer decode as load_00 (C.LWSP / C.FLWSP / C.FLDSP) */
	srli	t6, t5, 13
	andi	t6, t6, 7
	li	t4, 1
	beq	t6, t4, 1f
#if __riscv_xlen == 32
	li	t4, 3
	beq	t6, t4, 1f
#endif
	li	t6, 0
	j	2f
1:	li	t6, 1
2:
#endif /* FPU_MISALIGN_SUPPORT */
	srli	t3, t5, 5				/* Extract rd destination register * 4 */
	j	load_continue
load_11: /* Uncompressed instruction */
	addi	t3, t3, 4				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */
#ifdef FPU_MISALIGN_SUPPORT
	/* Uncompressed: opcode 0x07 = LOAD-FP (flw/fld), else integer load */
	andi	t6, t5, 0x7f
	li	t4, OPCODE_LOAD_FP
	beq	t6, t4, 1f
	li	t6, 0					/* Integer load (lw, lh, …) */
	j	2f
1:	li	t6, 1					/* FP load (flw or fld) */
2:
#endif /* FPU_MISALIGN_SUPPORT */
	srli	t2, t5, 12				/* Extract transfer size */
	andi	t2, t2, 7
	srli	t3, t5, 5				/* Extract rd destination register * 4 */
load_continue:
	andi	t3, t3, 0x7c
#else
	lw	t5, (t3)				/* Fetch faulting instruction */
	addi	t3, t3, 4				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */
#ifdef FPU_MISALIGN_SUPPORT
	/* Uncompressed (no C extension): same opcode check as load_11 */
	andi	t6, t5, 0x7f
	li	t4, OPCODE_LOAD_FP
	beq	t6, t4, 1f
	li	t6, 0
	j	2f
1:	li	t6, 1
2:
#endif /* FPU_MISALIGN_SUPPORT */
	srli	t2, t5, 12				/* Extract transfer size */
	andi	t2, t2, 7
	srli	t3, t5, 4				/* Extract rd destination register * 8 */
	andi	t3, t3, 0xf8
#endif /* __riscv_compressed */

	/*
	 * How many bytes to reconstruct from the faulting address (funct3 in t2).
	 *
	 * On RV32 with double FP (RV32D), funct3=2 means 4 bytes (lw or flw) and
	 * funct3=3 means 8 bytes (fld only — there is no 8-byte integer load on RV32).
	 * We must handle funct3=2 before the 8-byte path, or plain lw would be treated
	 * as an 8-byte access and break integer misalign fixups.
	 *
	 * Whether the value lands in xN or fN is decided later at load_tail using t6.
	 */
	li	t4, 1
	beq	t2, t4, load_half_size
	li	t4, 5
	beq	t2, t4, load_half_size_unsigned
#if __riscv_xlen == 64
	li	t4, 2
	beq	t2, t4, load_word_size		/* lw / flw (4 bytes) */
	li	t4, 6
	beq	t2, t4, load_word_size_unsigned
#elif defined(RV32D_MISALIGN_SUPPORT)
	li	t4, 2
	beq	t2, t4, load_word_size		/* RV32D: lw / flw — must not fall into 8-byte path */
#endif
#if __riscv_xlen == 64 || defined(RV32D_MISALIGN_SUPPORT)
#if __riscv_xlen == 32
	li	t4, 3
	beq	t2, t4, load_double_size	/* RV32D: fld only (8 bytes) */
	j	misalign_done			/* Unknown size on RV32 — nothing to emulate */
#endif
load_double_size:
	/*
	 * 8-byte misaligned load: rebuild data_0..data_7, then:
	 *   RV64 — ld into t4, then load_table (integer) or load_table_fp_d (fld, FPU=2 only)
	 *   RV32D — fld from data_0 into fN (no 64-bit GPR)
	 */
	lb	t4, 0(t1)				/* Load double byte-by-byte from destination address and save */
	sb	t4, data_0, t5
	lb	t4, 1(t1)
	sb	t4, data_1, t5
	lb	t4, 2(t1)
	sb	t4, data_2, t5
	lb	t4, 3(t1)
	sb	t4, data_3, t5
	lb	t4, 4(t1)
	sb	t4, data_4, t5
	lb	t4, 5(t1)
	sb	t4, data_5, t5
	lb	t4, 6(t1)
	sb	t4, data_6, t5
	lb	t4, 7(t1)
	sb	t4, data_7, t5
#if __riscv_xlen == 64
	ld	t4, data_0				/* RV64: stage 8 bytes in GPR scratch */
	/*
	 * RV64 with FPU=1 (single-precision only): misaligned ld still uses load_table;
	 * there is no fld and load_table_fp_d is not assembled. Only FPU=2 adds fld/fsd.
	 */
#if defined(FPU_MISALIGN_SUPPORT) && \
    defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
	beqz	t6, load_tail			/* Integer ld -> xN */
	la	t5, data_0
	la	t2, load_table_fp_d		/* Misaligned fld -> fN from scratch buffer */
	RESCALE_FP_JUMP_INDEX(t3)
	add	t2, t2, t3
	jr	t2
#endif
	j	load_tail
#else /* RV32D: 8-byte path is fld only — no GPR wide enough for 64 bits */
#ifdef FPU_MISALIGN_SUPPORT
	la	t5, data_0
	la	t2, load_table_fp_d		/* fld from data_0..7 into fN */
	RESCALE_FP_JUMP_INDEX(t3)
	add	t2, t2, t3
	jr	t2
#endif
#endif /* __riscv_xlen */
#endif /* RV64 or RV32D 8-byte staging */
#if __riscv_xlen == 64
load_word_size_unsigned:
	lb	t4, 0(t1)				/* Load word byte-by-byte from destination address and save */
	sb	t4, data_0, t5
	lb	t4, 1(t1)
	sb	t4, data_1, t5
	lb	t4, 2(t1)
	sb	t4, data_2, t5
	lb	t4, 3(t1)
	sb	t4, data_3, t5
	lwu	t4, data_0				/* Load unsigned word */
	j	load_tail
#endif
load_word_size:
	lb	t4, 0(t1)				/* Load word byte-by-byte from destination address and save */
	sb	t4, data_0, t5
	lb	t4, 1(t1)
	sb	t4, data_1, t5
	lb	t4, 2(t1)
	sb	t4, data_2, t5
	lb	t4, 3(t1)
	sb	t4, data_3, t5
	lw	t4, data_0				/* Load word */
	j	load_tail
load_half_size:
	lb	t4, 0(t1)				/* Load halfword byte-by-byte from destination address and save */
	sb	t4, data_0, t5
	lb	t4, 1(t1)
	sb	t4, data_1, t5
	lh	t4, data_0				/* Load halfword */
	j	load_tail
load_half_size_unsigned:
	lb	t4, 0(t1)				/* Load halfword byte-by-byte from destination address and save */
	sb	t4, data_0, t5
	lb	t4, 1(t1)
	sb	t4, data_1, t5
	lhu	t4, data_0				/* Load unsigned halfword */
load_tail:
#ifdef FPU_MISALIGN_SUPPORT
	/* 4-byte loads: t6 picks integer (xN) vs single FP (fN) jump table */
	beqz	t6, load_tail_gpr
	la	t2, load_table_fp			/* flw result: t4 -> fN via fmv.w.x */
	RESCALE_FP_JUMP_INDEX(t3)
	add	t2, t2, t3
	jr	t2
load_tail_gpr:
#endif
	la	t2, load_table				/* lw/lh result: t4 -> xN via mv */
	add	t2, t2, t3
	jr	t2
	.align 2
load_table:
	T4_TO_REG_Z (0)
	T4_TO_REG   (1)
	T4_TO_REG   (2)
	T4_TO_REG   (3)
	T4_TO_REG   (4)
	T4_TO_REG   (5)
	T4_TO_REG_V (6)				/* t1=x6: saved early -> write to stack slot */
	T4_TO_REG_V (7)				/* t2=x7: saved early -> write to stack slot */
	T4_TO_REG   (8)
	T4_TO_REG   (9)
	T4_TO_REG   (10)
	T4_TO_REG   (11)
	T4_TO_REG   (12)
#ifdef __riscv_32e
	/*
	 * In RV32E, x13/x14/x15 are saved early on the stack as scratch
	 * substitutes for t3/t4/t5. A misaligned load targeting one of
	 * these registers must write the result to the stack slot so that
	 * the early-register pop at misalign_done restores the correct
	 * value. Using T4_TO_REG_V writes to the stack slot safely.
	 * Note: t4 here is aliased to a4=x14, so T4_TO_STACK stores a4
	 * into the slot for the target register.
	 */
	T4_TO_REG_V (13)			/* a3=x13: saved early -> write to stack slot */
	T4_TO_REG_V (14)			/* a4=x14: saved early -> write to stack slot */
	T4_TO_REG_V (15)			/* a5=x15: saved early -> write to stack slot */
#else
	T4_TO_REG   (13)
	T4_TO_REG   (14)
	T4_TO_REG   (15)
	T4_TO_REG   (16)
	T4_TO_REG   (17)
	T4_TO_REG   (18)
	T4_TO_REG   (19)
	T4_TO_REG   (20)
	T4_TO_REG   (21)
	T4_TO_REG   (22)
	T4_TO_REG   (23)
	T4_TO_REG   (24)
	T4_TO_REG   (25)
	T4_TO_REG   (26)
	T4_TO_REG   (27)
	T4_TO_REG_V (28)			/* t3=x28: saved early -> write to stack slot */
	T4_TO_REG_V (29)			/* t4=x29: saved early -> write to stack slot */
	T4_TO_REG_V (30)			/* t5=x30: saved early -> write to stack slot */
#if defined(FPU_MISALIGN_SUPPORT) && !defined(__riscv_32e)
	T4_TO_REG_V (31)			/* t6=x31: FP flag during handler; value on stack */
#else
	T4_TO_REG   (31)
#endif
#endif /* __riscv_32e */

#ifdef FPU_MISALIGN_SUPPORT
	.align 2
load_table_fp:
	/* Single-precision misalign load: place reconstructed word into f0..f31 */
	T4_TO_FPREG (0)
	T4_TO_FPREG (1)
	T4_TO_FPREG (2)
	T4_TO_FPREG (3)
	T4_TO_FPREG (4)
	T4_TO_FPREG (5)
	T4_TO_FPREG (6)
	T4_TO_FPREG (7)
	T4_TO_FPREG (8)
	T4_TO_FPREG (9)
	T4_TO_FPREG (10)
	T4_TO_FPREG (11)
	T4_TO_FPREG (12)
	T4_TO_FPREG (13)
	T4_TO_FPREG (14)
	T4_TO_FPREG (15)
	T4_TO_FPREG (16)
	T4_TO_FPREG (17)
	T4_TO_FPREG (18)
	T4_TO_FPREG (19)
	T4_TO_FPREG (20)
	T4_TO_FPREG (21)
	T4_TO_FPREG (22)
	T4_TO_FPREG (23)
	T4_TO_FPREG (24)
	T4_TO_FPREG (25)
	T4_TO_FPREG (26)
	T4_TO_FPREG (27)
	T4_TO_FPREG (28)
	T4_TO_FPREG (29)
	T4_TO_FPREG (30)
	T4_TO_FPREG (31)

#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
	.align 2
load_table_fp_d:
	/* FPU=2 only: after 8-byte scratch fill, fld into f0..f31 (RV32D and RV64D) */
	T4_TO_FPREG_D (0)
	T4_TO_FPREG_D (1)
	T4_TO_FPREG_D (2)
	T4_TO_FPREG_D (3)
	T4_TO_FPREG_D (4)
	T4_TO_FPREG_D (5)
	T4_TO_FPREG_D (6)
	T4_TO_FPREG_D (7)
	T4_TO_FPREG_D (8)
	T4_TO_FPREG_D (9)
	T4_TO_FPREG_D (10)
	T4_TO_FPREG_D (11)
	T4_TO_FPREG_D (12)
	T4_TO_FPREG_D (13)
	T4_TO_FPREG_D (14)
	T4_TO_FPREG_D (15)
	T4_TO_FPREG_D (16)
	T4_TO_FPREG_D (17)
	T4_TO_FPREG_D (18)
	T4_TO_FPREG_D (19)
	T4_TO_FPREG_D (20)
	T4_TO_FPREG_D (21)
	T4_TO_FPREG_D (22)
	T4_TO_FPREG_D (23)
	T4_TO_FPREG_D (24)
	T4_TO_FPREG_D (25)
	T4_TO_FPREG_D (26)
	T4_TO_FPREG_D (27)
	T4_TO_FPREG_D (28)
	T4_TO_FPREG_D (29)
	T4_TO_FPREG_D (30)
	T4_TO_FPREG_D (31)
#endif /* double-precision FP */
#endif /* FPU_MISALIGN_SUPPORT */

/*
 * Store address misalign exception handler
 *
 * Register roles (same scratch mapping as load handler):
 *   t1 = faulting memory address (from mtval)
 *   t2 = transfer size in bytes (from instruction funct3)
 *   t3 = source register index * stride (for jump table)
 *   t4 = value to write (read from xN or fN before scatter)
 *   t5 = scratch
 *   t6 = is_fp_insn (1 = fsw/fsd or compressed FP store, 0 = sw/sh/…)
 *
 * Data path:
 *   1. store_table*: read source value into t4 (GPR or FP macro)
 *   2. store_tail: byte-scatter t4 (or data_0..7) to fault address
 *
 * FP stores branch to store_table_fp / store_table_fp_d before store_tail
 * when t6==1. Integer stores use store_table only.
 */

handle_store_misalign:
	csrr	t1, mtval				/* Get faulting virtual address */
	csrr	t3, mepc				/* Get faulting instruction address */
#if __riscv_compressed
	lhu	t5, 0(t3)				/* Fetch 16 low bits of faulting instruction */
	andi	t2, t5, 3				/* Extract op */
	slli	t2, t2, SHIFT-1
	la	t4, store_op_table			/* Compute address to store op label */
	add	t4, t4, t2
	LOAD	t4, 0(t4)				/* Load label itself from address */
	jr	t4					/* Jump to store op label */
store_00: /* Compressed instruction C.SW, C.SD, C.FSW, C.FSD with rs2' (bit 4:2) */
	addi	t3, t3, 2				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */

	srli	t2, t5, 13				/* Extract funct3 two LSB */
	andi	t2, t2, 3
	la	t4, transfer_size_table			/* Compute address to transfer size */
	add	t4, t4, t2
	lbu	t2, 0(t4)				/* Fetch transfer size */

#ifdef FPU_MISALIGN_SUPPORT
	/*
	 * Compressed FP store funct3 [15:13] differs from FP loads (CL format):
	 * C.FSD=101, C.FSW=111 (RV32). Loads use C.FLD=001, C.FLW=011.
	 */
	srli	t6, t5, 13
	andi	t6, t6, 7
	li	t4, 5
	beq	t6, t4, 1f				/* C.FSD / C.FSDSP */
#if __riscv_xlen == 32
	li	t4, 7
	beq	t6, t4, 1f				/* C.FSW / C.FSWSP */
#endif
	li	t6, 0					/* C.SW / C.SD — integer */
	j	2f
1:	li	t6, 1
2:
#endif /* FPU_MISALIGN_SUPPORT */
	addi	t3, t5, 8*4				/* Extract rs2 source register * 4 */
	j	store_continue
store_01: /* Non-store compressed instruction */
	j	misalign_done				/* Should never be misaligned so just return */
store_10: /* Compressed instruction C.SWSP, C.SDSP, C.FSWSP, C.FSDSP with rs2 (bit 6:2) */
	addi	t3, t3, 2				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */

	srli	t2, t5, 13				/* Extract funct3 two LSB */
	andi	t2, t2, 3
	la	t4, transfer_size_table			/* Compute address to transfer size */
	add	t4, t4, t2
	lbu	t2, 0(t4)				/* Fetch transfer size */

#ifdef FPU_MISALIGN_SUPPORT
	/* Same compressed FP store decode as store_00 (C.FSDSP / C.FSWSP on RV32) */
	srli	t6, t5, 13
	andi	t6, t6, 7
	li	t4, 5
	beq	t6, t4, 1f
#if __riscv_xlen == 32
	li	t4, 7
	beq	t6, t4, 1f
#endif
	li	t6, 0
	j	2f
1:	li	t6, 1
2:
#endif /* FPU_MISALIGN_SUPPORT */
	mv	t3, t5					/* Extract rs2 source register * 4 */
	j	store_continue
store_11: /* Uncompressed instruction with rs2 (bit 24:20) */
#ifdef FPU_MISALIGN_SUPPORT
	/* Must run before t5 is overwritten with the instruction high half */
	andi	t6, t5, 0x7f
	li	t4, OPCODE_STORE_FP
	beq	t6, t4, 1f
	li	t6, 0					/* Integer store (sw, sh, …) */
	j	2f
1:	li	t6, 1					/* FP store (fsw or fsd) */
2:
#endif /* FPU_MISALIGN_SUPPORT */
	srli	t2, t5, 12				/* Extract transfer size */
	andi	t2, t2, 7
	lhu	t5, 2(t3)				/* Fetch 16 high bits of faulting instruction */
	addi	t3, t3, 4				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */
	srli	t3, t5, 2				/* Extract rs2 source register * 4 */
store_continue:
	andi	t3, t3, 0x7c
#else
	lw	t5, (t3)				/* Fetch faulting instruction */
	addi	t3, t3, 4				/* Increment faulting instruction address */
	csrw	mepc, t3				/* Set incremented faulting instruction address */
#ifdef FPU_MISALIGN_SUPPORT
	/* Uncompressed (no C extension): opcode 0x27 = STORE-FP (fsw/fsd) */
	andi	t6, t5, 0x7f
	li	t4, OPCODE_STORE_FP
	beq	t6, t4, 1f
	li	t6, 0
	j	2f
1:	li	t6, 1
2:
#endif /* FPU_MISALIGN_SUPPORT */
	srli	t3, t5, 17				/* Extract rs2 source register * 8 */
	andi	t3, t3, 0xf8
	srli	t2, t5, 12				/* Extract transfer size */
	andi	t2, t2, 7
#endif /* __riscv_compressed */

#ifdef FPU_MISALIGN_SUPPORT
	/*
	 * Stores: copy the source register into t4 or data_0..7 before byte-scatter.
	 * Integer (t6=0) uses store_table (xN). FP uses store_table_fp (fsw / fmv.x.w).
	 * Double FP stores (fsd) use store_table_fp_d only when FPU=2 (same rule as load).
	 */
	beqz	t6, store_dispatch_gpr
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
	li	t4, 3
	beq	t2, t4, store_dispatch_fp_d	/* 8-byte fsd: fN -> scratch via fsd */
#endif
	la	t4, store_table_fp			/* 4-byte fsw: fN -> t4 via fmv.x.w */
	j	store_dispatch_fp
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
store_dispatch_fp_d:
	la	t5, data_0
	la	t4, store_table_fp_d
	j	store_dispatch_fp
#endif
store_dispatch_fp:
	RESCALE_FP_JUMP_INDEX(t3)
	add	t4, t4, t3
	jr	t4
store_dispatch_gpr:
#endif
	la	t4, store_table				/* Integer: xN -> t4 */
	add	t4, t4, t3
	jr	t4
store_tail:
	/*
	 * Write the staged value to the misaligned address, one byte at a time.
	 * Word/halfword paths use t4; 8-byte paths use data_0..data_7 (RV64 sd/fsd
	 * or RV32D fsd). Same funct3=2 vs 3 routing as on the load side for RV32D.
	 */
	li	t3, 1
	beq	t2, t3, store_half_size
#if __riscv_xlen == 64
	li	t3, 2
	beq	t2, t3, store_word_size		/* sw / fsw (4 bytes) */
#elif defined(RV32D_MISALIGN_SUPPORT)
	li	t3, 2
	beq	t2, t3, store_word_size		/* RV32D: sw / fsw — not 8-byte path */
#endif
#if __riscv_xlen == 64 || defined(RV32D_MISALIGN_SUPPORT)
#if __riscv_xlen == 32
	li	t3, 3
	beq	t2, t3, store_double_size	/* RV32D: fsd only (via store_table_fp_d) */
	j	misalign_done
#endif
store_double_size:
#if __riscv_xlen == 64
	sd	t4, data_0, t3				/* RV64: integer sd or FP fsd staging */
#endif
store_double_scatter:
	/* Write data_0..7 byte-by-byte to misaligned address (fsd / sd) */
	lbu	t4, data_0				/* Store double byte-by-byte into destination address */
	sb	t4, 0(t1)
	lbu	t4, data_1
	sb	t4, 1(t1)
	lbu	t4, data_2
	sb	t4, 2(t1)
	lbu	t4, data_3
	sb	t4, 3(t1)
	lbu	t4, data_4
	sb	t4, 4(t1)
	lbu	t4, data_5
	sb	t4, 5(t1)
	lbu	t4, data_6
	sb	t4, 6(t1)
	lbu	t4, data_7
	sb	t4, 7(t1)
	j	misalign_done
#endif /* RV64 or RV32D 8-byte staging */
store_word_size:
	sw	t4, data_0, t3
	lbu	t4, data_0				/* Store word byte-by-byte into destination address */
	sb	t4, 0(t1)
	lbu	t4, data_1
	sb	t4, 1(t1)
	lbu	t4, data_2
	sb	t4, 2(t1)
	lbu	t4, data_3
	sb	t4, 3(t1)
	j	misalign_done
store_half_size:
	sw	t4, data_0, t3
	lbu	t4, data_0				/* Store halfword byte-by-byte into destination address */
	sb	t4, 0(t1)
	lbu	t4, data_1
	sb	t4, 1(t1)
	j	misalign_done
	.align 2
store_table:
	REG_TO_T4_Z (0)
	REG_TO_T4   (1)
	REG_TO_T4   (2)
	REG_TO_T4   (3)
	REG_TO_T4   (4)
	REG_TO_T4   (5)
	REG_TO_T4_V (6)				/* t1=x6: saved early -> read from stack slot */
	REG_TO_T4_V (7)				/* t2=x7: saved early -> read from stack slot */
	REG_TO_T4   (8)
	REG_TO_T4   (9)
	REG_TO_T4   (10)
	REG_TO_T4   (11)
	REG_TO_T4   (12)
#ifdef __riscv_32e
	/*
	 * In RV32E, x13/x14/x15 are saved early as scratch substitutes.
	 * A misaligned store sourcing one of these registers must read the
	 * original value from the stack slot (not the clobbered register),
	 * so REG_TO_T4_V is used to load from the saved stack slot.
	 */
	REG_TO_T4_V (13)			/* a3=x13: saved early -> read from stack slot */
	REG_TO_T4_V (14)			/* a4=x14: saved early -> read from stack slot */
	REG_TO_T4_V (15)			/* a5=x15: saved early -> read from stack slot */
#else
	REG_TO_T4   (13)
	REG_TO_T4   (14)
	REG_TO_T4   (15)
	REG_TO_T4   (16)
	REG_TO_T4   (17)
	REG_TO_T4   (18)
	REG_TO_T4   (19)
	REG_TO_T4   (20)
	REG_TO_T4   (21)
	REG_TO_T4   (22)
	REG_TO_T4   (23)
	REG_TO_T4   (24)
	REG_TO_T4   (25)
	REG_TO_T4   (26)
	REG_TO_T4   (27)
	REG_TO_T4_V (28)			/* t3=x28: saved early -> read from stack slot */
	REG_TO_T4_V (29)			/* t4=x29: saved early -> read from stack slot */
	REG_TO_T4_V (30)			/* t5=x30: saved early -> read from stack slot */
#if defined(FPU_MISALIGN_SUPPORT) && !defined(__riscv_32e)
	REG_TO_T4_V (31)			/* t6=x31: read original t6 from stack slot */
#else
	REG_TO_T4   (31)
#endif
#endif /* __riscv_32e */

#ifdef FPU_MISALIGN_SUPPORT
	.align 2
store_table_fp:
	/* Single-precision misalign store: read source word from f0..f31 into t4 */
	REG_TO_T4_FP (0)
	REG_TO_T4_FP (1)
	REG_TO_T4_FP (2)
	REG_TO_T4_FP (3)
	REG_TO_T4_FP (4)
	REG_TO_T4_FP (5)
	REG_TO_T4_FP (6)
	REG_TO_T4_FP (7)
	REG_TO_T4_FP (8)
	REG_TO_T4_FP (9)
	REG_TO_T4_FP (10)
	REG_TO_T4_FP (11)
	REG_TO_T4_FP (12)
	REG_TO_T4_FP (13)
	REG_TO_T4_FP (14)
	REG_TO_T4_FP (15)
	REG_TO_T4_FP (16)
	REG_TO_T4_FP (17)
	REG_TO_T4_FP (18)
	REG_TO_T4_FP (19)
	REG_TO_T4_FP (20)
	REG_TO_T4_FP (21)
	REG_TO_T4_FP (22)
	REG_TO_T4_FP (23)
	REG_TO_T4_FP (24)
	REG_TO_T4_FP (25)
	REG_TO_T4_FP (26)
	REG_TO_T4_FP (27)
	REG_TO_T4_FP (28)
	REG_TO_T4_FP (29)
	REG_TO_T4_FP (30)
	REG_TO_T4_FP (31)

#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
	.align 2
store_table_fp_d:
	/* FPU=2 only: fsd from fN into data_0..7, then store_tail scatters bytes */
	REG_TO_T4_FP_D (0)
	REG_TO_T4_FP_D (1)
	REG_TO_T4_FP_D (2)
	REG_TO_T4_FP_D (3)
	REG_TO_T4_FP_D (4)
	REG_TO_T4_FP_D (5)
	REG_TO_T4_FP_D (6)
	REG_TO_T4_FP_D (7)
	REG_TO_T4_FP_D (8)
	REG_TO_T4_FP_D (9)
	REG_TO_T4_FP_D (10)
	REG_TO_T4_FP_D (11)
	REG_TO_T4_FP_D (12)
	REG_TO_T4_FP_D (13)
	REG_TO_T4_FP_D (14)
	REG_TO_T4_FP_D (15)
	REG_TO_T4_FP_D (16)
	REG_TO_T4_FP_D (17)
	REG_TO_T4_FP_D (18)
	REG_TO_T4_FP_D (19)
	REG_TO_T4_FP_D (20)
	REG_TO_T4_FP_D (21)
	REG_TO_T4_FP_D (22)
	REG_TO_T4_FP_D (23)
	REG_TO_T4_FP_D (24)
	REG_TO_T4_FP_D (25)
	REG_TO_T4_FP_D (26)
	REG_TO_T4_FP_D (27)
	REG_TO_T4_FP_D (28)
	REG_TO_T4_FP_D (29)
	REG_TO_T4_FP_D (30)
	REG_TO_T4_FP_D (31)
#endif /* double-precision FP */
#endif /* FPU_MISALIGN_SUPPORT */


.section .data
#if __riscv_xlen == 64 || defined(RV32D_MISALIGN_SUPPORT)
.align 3
#else
.align DATAALIGN
#endif

/*
 * Scratch bytes used while emulating a misaligned access.
 *   data_0..3 — halfword and 32-bit word paths (integer and flw/fsw).
 *   data_4..7 — extra bytes for 64-bit paths on RV64 (ld/sd) or RV32D (fld/fsd).
 * The buffer is 8-byte aligned on RV64 and RV32D so fld/fsd to data_0 is valid.
 */

data_0:
	.byte 0
data_1:
	.byte 0
data_2:
	.byte 0
data_3:
	.byte 0
#if __riscv_xlen == 64 || defined(RV32D_MISALIGN_SUPPORT)
data_4:
	.byte 0
data_5:
	.byte 0
data_6:
	.byte 0
data_7:
	.byte 0
#endif

/*
 * Compressed transfer size and op tables.
 * Size only — FP vs integer for compressed ops is decoded separately into t6.
 */
#if __riscv_compressed
transfer_size_table:
	.byte 0
	.byte 3						/* 01: C.FLD/C.FSD family — 8 bytes */
	.byte 2						/* 10: C.LW/C.SW family — 4 bytes */
#if __riscv_xlen == 64
	.byte 3						/* 11: C.LD/C.SD family — 8 bytes */
#else
	.byte 2						/* 11: C.FLW/C.FSW family — 4 bytes (RV32) */
#endif
	.align DATAALIGN
load_op_table:
	INTPTR_DATAITEM load_00				/* OP0 compressed instruction */
	INTPTR_DATAITEM load_01			 	/* OP1 compressed instruction */
	INTPTR_DATAITEM load_10				/* OP2 compressed instruction */
	INTPTR_DATAITEM load_11				/* Uncompressed instruction   */
store_op_table:
	INTPTR_DATAITEM store_00			/* OP0 compressed instruction */
	INTPTR_DATAITEM store_01			/* OP1 compressed instruction */
	INTPTR_DATAITEM store_10			/* OP2 compressed instruction */
	INTPTR_DATAITEM store_11			/* Uncompressed instruction   */
#endif /* __riscv_compressed */

/*
 * Exception and interrupt vector tables
 */
.align DATAALIGN
.global RISCV_ExceptionVectorTable
RISCV_ExceptionVectorTable:
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 0					/* Instruction access misaligned */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 1					/* Instruction access fault */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 2					/* Illegal instruction */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 3					/* Breakpoint */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 4					/* Load address misaligned */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 5					/* Load access fault */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 6					/* Store/AMO address misaligned */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 7					/* Store/AMO access fault */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 8					/* Environment call from U-mode */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 9					/* Environment call from S-mode */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 10					/* Reserved */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 11					/* Environment call from M-mode */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 12                                  /* Instruction page fault */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 13                                  /* Load page fault */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 14                                  /* Reserved */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 15                                  /* Store/AMO page fault */

.global RISCV_InterruptVectorTable
RISCV_InterruptVectorTable:
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 0					/* Unimplemented */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 1					/* Supervisor software interrupt */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 2					/* Reserved for future standard use */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 3					/* Machine software interrupt */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 4					/* Unimplemented */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 5					/* Supervisor timer interrupt */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 6					/* Reserved for future standard use */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 7					/* Machine timer interrupt */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 8					/* Unimplemented */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 9					/* Supervisor external interrupt */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 10					/* Reserved for future standard use */
    INTPTR_DATAITEM XNullHandler
    INTPTR_DATAITEM 11					/* Machine external interrupt */
.end _trap_handler

#else

/*
 * Dummy trap handler, in case traps are not present in the processor
 */

.global _trap_handler
.section .text
.align 2
_trap_handler:
        j     0
.end _trap_handler

#endif  /* RISCV_TRAPS_DISABLED */
