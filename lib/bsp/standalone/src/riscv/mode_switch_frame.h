/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file mode_switch_frame.h
 * @brief Saved-session layout for a YIELD'd RISC-V M/U mode-switch guest.
 *
 * U-mode traps are serviced by the shared _trap_handler (trap_handler.S) using
 * its own existing GPR/FPU save area on the M stack - there is no dedicated
 * mode-switch trap frame or vector. This header only defines the small
 * snapshot needed to persist a guest's register state across a YIELD ECALL
 * until Xil_ResumeUserSession() restores it (mode_switch_enter.S).
 *
 * mepc and the MPIE/MPP mstatus bits are captured because the guest is not
 * resumed immediately (unlike an ordinary ECALL, which advances mepc via CSR
 * write and falls straight through to _trap_handler's mret). mcause/mtval and
 * FPU state are not part of the snapshot: mcause/mtval only matter at the
 * instant of the trap (already consumed before YIELD is recognized as such),
 * and FPU register state is not preserved across a YIELD/resume cycle.
 */

#ifndef MODE_SWITCH_FRAME_H
#define MODE_SWITCH_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ASSEMBLER__
#include "xil_types.h"
#endif

/** Mask for the mstatus fields captured in a saved session: MPIE (7), MPP (12:11). */
#define XIL_MS_MSTATUS_SAVE_MASK	0x00001880U

/**
 * @name Xil_GuestSession byte offsets
 *
 * Byte offsets into Xil_GuestSession, for mode_switch_enter.S's resume helper
 * (which cannot use offsetof()). gpr[] is REGSIZE-strided (matches
 * trap_handler.S's REG_OFFSET); mepc follows the 32 GPR slots; mstatus (u32)
 * follows mepc. Must be kept in sync with the struct below.
 * @{
 */
#if defined(__riscv_xlen) && (__riscv_xlen == 64)
#define XIL_MS_SESSION_REGSIZE		8U /**< XLEN register width in bytes (RV64). */
#else
#define XIL_MS_SESSION_REGSIZE		4U /**< XLEN register width in bytes (RV32). */
#endif
#define XIL_MS_SESSION_OFF_GPR		0U /**< Offset of gpr[0]. */
#define XIL_MS_SESSION_OFF_MEPC		(32U * XIL_MS_SESSION_REGSIZE) /**< Offset of mepc. */
#define XIL_MS_SESSION_OFF_MSTATUS	(33U * XIL_MS_SESSION_REGSIZE) /**< Offset of mstatus. */
/** @} */

/**
 * @name M-mode continuation frame offsets
 *
 * M-mode continuation frame (mode_switch_enter.S):
 * Saved across U execution so Xil_ReturnToMMode can restore ra, sp, and
 * s0-s11. Slot spacing is REGSIZE; total size is 16-byte aligned (64 B on
 * RV32, 112 B on RV64). mtvec is not saved/restored here - it never diverges
 * from _trap_handler, so there is nothing to reinstall on return.
 *
 * Layout: ra, caller_sp, s0..s11 => 14 XLEN slots.
 * @{
 */
#if defined(__riscv_xlen) && (__riscv_xlen == 64)
#define XIL_MS_MCONT_SLOT_RA		0U   /**< Offset of the saved ra. */
#define XIL_MS_MCONT_SLOT_SP		8U   /**< Offset of the saved caller sp. */
#define XIL_MS_MCONT_OFF_S0		16U  /**< Offset of the saved s0. */
#define XIL_MS_MCONT_OFF_S1		24U  /**< Offset of the saved s1. */
#define XIL_MS_MCONT_OFF_S2		32U  /**< Offset of the saved s2. */
#define XIL_MS_MCONT_OFF_S3		40U  /**< Offset of the saved s3. */
#define XIL_MS_MCONT_OFF_S4		48U  /**< Offset of the saved s4. */
#define XIL_MS_MCONT_OFF_S5		56U  /**< Offset of the saved s5. */
#define XIL_MS_MCONT_OFF_S6		64U  /**< Offset of the saved s6. */
#define XIL_MS_MCONT_OFF_S7		72U  /**< Offset of the saved s7. */
#define XIL_MS_MCONT_OFF_S8		80U  /**< Offset of the saved s8. */
#define XIL_MS_MCONT_OFF_S9		88U  /**< Offset of the saved s9. */
#define XIL_MS_MCONT_OFF_S10		96U  /**< Offset of the saved s10. */
#define XIL_MS_MCONT_OFF_S11		104U /**< Offset of the saved s11. */
#define XIL_MS_MCONT_SIZE_RAW		112U /**< Unaligned frame size (RV64), before 16-byte rounding. */
#else
#define XIL_MS_MCONT_SLOT_RA		0U   /**< Offset of the saved ra. */
#define XIL_MS_MCONT_SLOT_SP		4U   /**< Offset of the saved caller sp. */
#define XIL_MS_MCONT_OFF_S0		8U   /**< Offset of the saved s0. */
#define XIL_MS_MCONT_OFF_S1		12U  /**< Offset of the saved s1. */
#define XIL_MS_MCONT_OFF_S2		16U  /**< Offset of the saved s2. */
#define XIL_MS_MCONT_OFF_S3		20U  /**< Offset of the saved s3. */
#define XIL_MS_MCONT_OFF_S4		24U  /**< Offset of the saved s4. */
#define XIL_MS_MCONT_OFF_S5		28U  /**< Offset of the saved s5. */
#define XIL_MS_MCONT_OFF_S6		32U  /**< Offset of the saved s6. */
#define XIL_MS_MCONT_OFF_S7		36U  /**< Offset of the saved s7. */
#define XIL_MS_MCONT_OFF_S8		40U  /**< Offset of the saved s8. */
#define XIL_MS_MCONT_OFF_S9		44U  /**< Offset of the saved s9. */
#define XIL_MS_MCONT_OFF_S10		48U  /**< Offset of the saved s10. */
#define XIL_MS_MCONT_OFF_S11		52U  /**< Offset of the saved s11. */
#define XIL_MS_MCONT_SIZE_RAW		56U  /**< Unaligned frame size (RV32), before 16-byte rounding. */
#endif
/** @} */

/** M-mode continuation frame size, rounded up to a 16-byte boundary. */
#define XIL_MS_MCONT_SIZE		(((XIL_MS_MCONT_SIZE_RAW) + 15U) & ~15U)

#ifndef __ASSEMBLER__

/**
 * @brief Snapshot of a YIELD'd guest, restored verbatim by Xil_ResumeUserSession().
 */
typedef struct {
	UINTPTR gpr[32]; /**< Guest GPRs x0-x31. gpr[0] is unused; gpr[2] is the
			  *   guest's saved stack pointer. */
	UINTPTR mepc;    /**< Guest mepc at YIELD, already advanced past the ECALL. */
	u32 mstatus;     /**< Guest mstatus MPIE/MPP bits captured at YIELD. */
} Xil_GuestSession;

#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif

#endif /* MODE_SWITCH_FRAME_H */
