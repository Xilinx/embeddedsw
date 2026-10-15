/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file mode_switch_enter.S
 * @brief M→U handoff via mret; M return via Xil_ReturnToMMode; YIELD resume.
 *
 * Flow — Xil_EnterUserModeAsm
 * ---------------------------
 * 1. Decrement M SP by XIL_MS_MCONT_SIZE (16-byte align frame base); save M
 *    continuation frame (ra, caller sp, s0-s11) on the M stack.
 * 2. Store frame base into global Xil_MContFramePtr (t2 = &Xil_MContFramePtr).
 * 3. mscratch = frame base (M-mode stack anchor for U→M traps); clear MIE.
 * 4. mepc = Entry (_boot); mstatus MPP=U; mret. Guest boot.S sets sp from
 *    __stack — this path does not program the U stack pointer.
 *
 * mtvec is never touched here: U-mode traps go through the same shared
 * _trap_handler (trap_handler.S) that M-mode traps already use — there is no
 * separate mode-switch trap vector to install or restore.
 *
 *   a0 = guest _boot address (mepc target)
 *
 * Permissive PMP is configured in Xil_ModeSwitchInit() before the first handoff.
 *
 * M-continuation frame
 * --------------------
 * Before mret drops into U-mode, M-mode handoff code saves callee-saved M state
 * on the M stack. mscratch holds the frame base pointer so U→M traps can reach
 * the M stack anchor. On EXIT/YIELD/abort, Xil_ReturnToMMode restores that state
 * and returns to the C caller of Xil_EnterUserModeAsm().
 *
 * Slot spacing is REGSIZE (4 B RV32, 8 B RV64). Offsets: mode_switch_frame.h
 * (XIL_MS_MCONT_*). Fixed 4-byte offsets with sd on RV64 would corrupt slots.
 *
 * Flow — Xil_ResumeGuestSessionAsm (YIELD resume)
 * ------------------------------------------------
 * Saves a fresh M continuation frame exactly like Xil_EnterUserModeAsm, then
 * builds a _trap_handler-shaped GPR frame directly from the saved
 * Xil_GuestSession (a plain word-for-word copy - REG_OFFSET(n) in
 * trap_handler.S and gpr[n] here use the same REGSIZE stride), restores mepc
 * and the saved mstatus subset, and jumps straight into trap_restore
 * (trap_handler.S) to finish the restore and mret. No separate resume frame
 * format or GPR restore sequence is duplicated here.
 */

#ifdef RISCV_MODE_SWITCH

#include "xparameters.h"
#include "mode_switch_frame.h"

#if __riscv_xlen == 64
#define REGSIZE		8
#define STORE		sd
#define LOAD		ld
#else
#define REGSIZE		4
#define STORE		sw
#define LOAD		lw
#endif

#define STORE_U32	sw
#define LOAD_U32	lw

/* Must match trap_handler.S's own GPR-frame layout exactly. */
#define REGCOUNT		32
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
#define FPREGSIZE		8
#define FPREGCOUNT		33
#elif defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 1)
#define FPREGSIZE		4
#define FPREGCOUNT		33
#else
#define FPREGSIZE		0
#define FPREGCOUNT		0
#endif
#define HANDLER_FRAME_PAYLOAD	(REGSIZE * REGCOUNT + FPREGSIZE * FPREGCOUNT)
#define HANDLER_STACK_SIZE	(((HANDLER_FRAME_PAYLOAD) + 15) & ~15)
#define FPREG_OFFSET(regnum)	(FPREGSIZE * regnum + REGSIZE * REGCOUNT)

.extern trap_restore

.global Xil_EnterUserModeAsm
.type Xil_EnterUserModeAsm, @function
.global Xil_ReturnToMMode
.type Xil_ReturnToMMode, @function
.global Xil_MContFramePtr

/*
 * Macro: save M-mode continuation frame at sp (already decremented).
 * Records ra, caller SP (pre-handoff M sp), and s0-s11.
 * \caller_sp must hold the M sp before the cont. frame was allocated.
 */
.macro MS_MCONT_SAVE caller_sp
	STORE	ra, XIL_MS_MCONT_SLOT_RA(sp)
	STORE	\caller_sp, XIL_MS_MCONT_SLOT_SP(sp)
	STORE	s0, XIL_MS_MCONT_OFF_S0(sp)
	STORE	s1, XIL_MS_MCONT_OFF_S1(sp)
	STORE	s2, XIL_MS_MCONT_OFF_S2(sp)
	STORE	s3, XIL_MS_MCONT_OFF_S3(sp)
	STORE	s4, XIL_MS_MCONT_OFF_S4(sp)
	STORE	s5, XIL_MS_MCONT_OFF_S5(sp)
	STORE	s6, XIL_MS_MCONT_OFF_S6(sp)
	STORE	s7, XIL_MS_MCONT_OFF_S7(sp)
	STORE	s8, XIL_MS_MCONT_OFF_S8(sp)
	STORE	s9, XIL_MS_MCONT_OFF_S9(sp)
	STORE	s10, XIL_MS_MCONT_OFF_S10(sp)
	STORE	s11, XIL_MS_MCONT_OFF_S11(sp)
.endm

/*
 * Macro: restore M-mode continuation from t2 (frame base from Xil_MContFramePtr).
 * Does not restore sp — caller loads sp from saved slot after mtvec restore.
 */
.macro MS_MCONT_RESTORE_GPRS
	LOAD	ra, XIL_MS_MCONT_SLOT_RA(t2)
	LOAD	s0, XIL_MS_MCONT_OFF_S0(t2)
	LOAD	s1, XIL_MS_MCONT_OFF_S1(t2)
	LOAD	s2, XIL_MS_MCONT_OFF_S2(t2)
	LOAD	s3, XIL_MS_MCONT_OFF_S3(t2)
	LOAD	s4, XIL_MS_MCONT_OFF_S4(t2)
	LOAD	s5, XIL_MS_MCONT_OFF_S5(t2)
	LOAD	s6, XIL_MS_MCONT_OFF_S6(t2)
	LOAD	s7, XIL_MS_MCONT_OFF_S7(t2)
	LOAD	s8, XIL_MS_MCONT_OFF_S8(t2)
	LOAD	s9, XIL_MS_MCONT_OFF_S9(t2)
	LOAD	s10, XIL_MS_MCONT_OFF_S10(t2)
	LOAD	s11, XIL_MS_MCONT_OFF_S11(t2)
.endm

#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU > 0)
/*
 * Sanitize FP state before mret into guest _boot: enable FS and zero f0-f31/fcsr
 * so U-mode does not inherit monitor FPR contents (guest boot also clears FPRs).
 */
.macro MS_GUEST_FP_SCRUB
	li	t0, 0x2000
	csrs	mstatus, t0
	csrw	fcsr, x0
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU == 2)
#if __riscv_xlen == 64
	fmv.d.x	f0, x0
	fmv.d.x	f1, x0
	fmv.d.x	f2, x0
	fmv.d.x	f3, x0
	fmv.d.x	f4, x0
	fmv.d.x	f5, x0
	fmv.d.x	f6, x0
	fmv.d.x	f7, x0
	fmv.d.x	f8, x0
	fmv.d.x	f9, x0
	fmv.d.x	f10, x0
	fmv.d.x	f11, x0
	fmv.d.x	f12, x0
	fmv.d.x	f13, x0
	fmv.d.x	f14, x0
	fmv.d.x	f15, x0
	fmv.d.x	f16, x0
	fmv.d.x	f17, x0
	fmv.d.x	f18, x0
	fmv.d.x	f19, x0
	fmv.d.x	f20, x0
	fmv.d.x	f21, x0
	fmv.d.x	f22, x0
	fmv.d.x	f23, x0
	fmv.d.x	f24, x0
	fmv.d.x	f25, x0
	fmv.d.x	f26, x0
	fmv.d.x	f27, x0
	fmv.d.x	f28, x0
	fmv.d.x	f29, x0
	fmv.d.x	f30, x0
	fmv.d.x	f31, x0
#else
	fcvt.d.w f0, x0
	fcvt.d.w f1, x0
	fcvt.d.w f2, x0
	fcvt.d.w f3, x0
	fcvt.d.w f4, x0
	fcvt.d.w f5, x0
	fcvt.d.w f6, x0
	fcvt.d.w f7, x0
	fcvt.d.w f8, x0
	fcvt.d.w f9, x0
	fcvt.d.w f10, x0
	fcvt.d.w f11, x0
	fcvt.d.w f12, x0
	fcvt.d.w f13, x0
	fcvt.d.w f14, x0
	fcvt.d.w f15, x0
	fcvt.d.w f16, x0
	fcvt.d.w f17, x0
	fcvt.d.w f18, x0
	fcvt.d.w f19, x0
	fcvt.d.w f20, x0
	fcvt.d.w f21, x0
	fcvt.d.w f22, x0
	fcvt.d.w f23, x0
	fcvt.d.w f24, x0
	fcvt.d.w f25, x0
	fcvt.d.w f26, x0
	fcvt.d.w f27, x0
	fcvt.d.w f28, x0
	fcvt.d.w f29, x0
	fcvt.d.w f30, x0
	fcvt.d.w f31, x0
#endif
#else
	fmv.w.x	f0, x0
	fmv.w.x	f1, x0
	fmv.w.x	f2, x0
	fmv.w.x	f3, x0
	fmv.w.x	f4, x0
	fmv.w.x	f5, x0
	fmv.w.x	f6, x0
	fmv.w.x	f7, x0
	fmv.w.x	f8, x0
	fmv.w.x	f9, x0
	fmv.w.x	f10, x0
	fmv.w.x	f11, x0
	fmv.w.x	f12, x0
	fmv.w.x	f13, x0
	fmv.w.x	f14, x0
	fmv.w.x	f15, x0
	fmv.w.x	f16, x0
	fmv.w.x	f17, x0
	fmv.w.x	f18, x0
	fmv.w.x	f19, x0
	fmv.w.x	f20, x0
	fmv.w.x	f21, x0
	fmv.w.x	f22, x0
	fmv.w.x	f23, x0
	fmv.w.x	f24, x0
	fmv.w.x	f25, x0
	fmv.w.x	f26, x0
	fmv.w.x	f27, x0
	fmv.w.x	f28, x0
	fmv.w.x	f29, x0
	fmv.w.x	f30, x0
	fmv.w.x	f31, x0
#endif
.endm
#endif /* FPU */

/*
 * void Xil_EnterUserModeAsm(entry)
 *   a0 = guest _boot address (mepc target)
 *
 * Own section (matches -ffunction-sections on the C side): only reachable
 * from Xil_EnterUserMode() - a plain app that never
 * calls those does not pay for this function even though it links
 * Xil_ReturnToMMode from the same file.
 */
.section .text.Xil_EnterUserModeAsm,"ax",@progbits
.align 2
Xil_EnterUserModeAsm:
	mv	t1, sp
	addi	sp, sp, -XIL_MS_MCONT_SIZE
	andi	sp, sp, -16
	la	t2, Xil_MContFramePtr
	STORE	sp, 0(t2)			/* *Xil_MContFramePtr = frame base */

	MS_MCONT_SAVE t1

	csrci	mstatus, 0x8			/* clear MIE before mret */
	csrw	mscratch, sp			/* M stack anchor = cont. frame base */
	csrw	mepc, a0
#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU > 0)
	MS_GUEST_FP_SCRUB
#endif
	/*
	 * ~0x1888 clears MPP/MPIE/MIE/MPP-related bits in mstatus subset before
	 * U entry; ori 0x80 sets MPIE so mret enables interrupts in U-mode.
	 */
	csrr	t0, mstatus
	li	t1, ~0x1888
	and	t0, t0, t1
	ori	t0, t0, 0x80
	csrw	mstatus, t0
	mret

.size Xil_EnterUserModeAsm, .-Xil_EnterUserModeAsm

.global Xil_ResumeGuestSessionAsm
.type Xil_ResumeGuestSessionAsm, @function

/*
 * void Xil_ResumeGuestSessionAsm(Xil_GuestSession *Session)
 *   a0 = persistent YIELD snapshot. Saves a fresh M continuation frame, then
 *   builds a _trap_handler-shaped GPR frame directly from Session and jumps
 *   into trap_restore to finish the restore and mret.
 *
 * Own section: only reachable from Xil_ResumeUserSession().
 */
.section .text.Xil_ResumeGuestSessionAsm,"ax",@progbits
.align 2
Xil_ResumeGuestSessionAsm:
	mv	t1, sp
	addi	sp, sp, -XIL_MS_MCONT_SIZE
	andi	sp, sp, -16
	la	t2, Xil_MContFramePtr
	STORE	sp, 0(t2)

	MS_MCONT_SAVE t1

	csrci	mstatus, 0x8			/* clear MIE */
	csrw	mscratch, sp			/* M stack anchor = fresh cont. frame base */

	/* Restore mepc and the saved MPIE/MPP mstatus subset from the session. */
	LOAD	t0, XIL_MS_SESSION_OFF_MEPC(a0)
	csrw	mepc, t0
	LOAD_U32 t0, XIL_MS_SESSION_OFF_MSTATUS(a0)
	csrr	t1, mstatus
	li	t2, ~0x1880
	and	t1, t1, t2
	or	t1, t1, t0
	csrw	mstatus, t1

	/* Build a _trap_handler-shaped GPR frame below the fresh M anchor. */
	addi	sp, sp, -(HANDLER_STACK_SIZE)
	mv	t2, a0				/* &Session->gpr[0] */
	mv	t3, sp				/* base of the new GPR frame */
	li	t4, 32
1:
	LOAD	t5, 0(t2)
	STORE	t5, 0(t3)
	addi	t2, t2, REGSIZE
	addi	t3, t3, REGSIZE
	addi	t4, t4, -1
	bnez	t4, 1b

#if defined(XPAR_MICROBLAZE_RISCV_USE_FPU) && (XPAR_MICROBLAZE_RISCV_USE_FPU > 0)
	/*
	 * FPU state is not preserved across YIELD/resume - zero the frame's FPU
	 * region rather than let trap_restore load stale stack contents into
	 * the real f0-f31/fcsr registers.
	 */
	addi	t2, sp, FPREG_OFFSET(0)
	li	t4, (FPREGSIZE * FPREGCOUNT)
2:
	sb	zero, 0(t2)
	addi	t2, t2, 1
	addi	t4, t4, -1
	bnez	t4, 2b
#endif

	j	trap_restore

.size Xil_ResumeGuestSessionAsm, .-Xil_ResumeGuestSessionAsm

/*
 * void Xil_ReturnToMMode(void)
 * Restore M callee-saved state saved by Enter/Resume, restore EnterUserMode()
 * caller SP into sp and mscratch.
 *
 * Own section: this is the one function here a plain app (never calling
 * Enter/Resume) still forces in, via Xil_ExitUserMode/Xil_ModeSwitchAbort/
 * the YIELD case in Xil_SyscallDispatch (xil_mode_switch.c).
 */
.section .text.Xil_ReturnToMMode,"ax",@progbits
.align 2
Xil_ReturnToMMode:
	csrci	mstatus, 0x8
	la	t2, Xil_MContFramePtr
	LOAD	t2, 0(t2)
	beqz	t2, ms_return_halt

	MS_MCONT_RESTORE_GPRS
	LOAD	sp, XIL_MS_MCONT_SLOT_SP(t2)
	csrw	mscratch, sp
	ret

ms_return_halt:
1:	j	1b				/* local label 1: spin if no cont. frame */

.size Xil_ReturnToMMode, .-Xil_ReturnToMMode

#endif /* RISCV_MODE_SWITCH */
