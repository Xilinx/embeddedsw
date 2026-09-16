/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file riscv_modeswitch_guest.c
 * @brief RISC-V U-mode guest for the M/U mode-switch handoff example.
 *
 * Test flow (use case)
 * --------------------
 * This ELF is the *guest* half of a two-ELF mode-switch demo. A separate
 * monitor ELF (riscv_modeswitch_monitor.c) runs in M-mode, calls
 * Xil_EnterUserMode() with this image's _boot address, and regains control
 * only when the guest ends the session via an ECALL.
 *
 * Typical bring-up (XSDB / lab):
 *   1. Load the guest ELF at its link address (must match GUEST_BOOT_PA in the
 *      monitor build).
 *   2. Load the monitor ELF and run.
 *   3. Monitor handoffs to guest _boot; guest runs the U-mode work below.
 *   4. Guest ECALLs XIL_SYS_EXIT; monitor continues in M-mode and checks status.
 *
 * BSP requirement
 * ---------------
 * Build against a guest BSP with standalone_riscv_user_app=true. The guest has
 * no access to M-mode CSRs (mtvec, mie, mstatus.MPP, PMP programming, etc.).
 * Xil_ExceptionInit() is a no-op in this BSP; trap/interrupt state is owned
 * by the monitor.
 *
 * Supported ECALLs (a7 service number)
 * ------------------------------------
 * Only two syscalls are handled by the monitor's Xil_SyscallDispatch():
 *
 *   XIL_SYS_EXIT  (1) — terminate the guest session. a0 carries the exit code
 *                       returned to the monitor (XIL_MS_STATUS_EXIT).
 *   XIL_SYS_YIELD (4) — pause the guest; monitor may warm-resume later via
 *                       Xil_ResumeUserSession() (XIL_MS_STATUS_YIELD).
 *
 * Any other ECALL number returns -1 (XST_FAILURE) in a0 and the guest resumes
 * after mepc is advanced past the ECALL instruction.
 *
 * U-mode interrupt handling (single core)
 * ---------------------------------------
 * On this single-hart MicroBlaze RISC-V core there is no separate U-mode trap
 * vector. While the guest runs, mtvec still points at the monitor's shared
 * _trap_handler in M-mode. When a peripheral interrupt fires during U-mode
 * execution:
 *
 *   1. Hardware traps to _trap_handler (still at M privilege).
 *   2. Entry swaps to the M stack anchor (mscratch) and saves the guest frame.
 *   3. The interrupt is dispatched through RISCV_InterruptVectorTable — the
 *      same handlers registered by the monitor BSP (e.g. UART/timer ISRs).
 *   4. trap_restore returns to U-mode at the interrupted guest PC (mret).
 *
 * So guest ISRs are really monitor ISRs running on behalf of the guest; the
 * guest cannot register its own vector table or manipulate MIE/MSTATUS.MIE.
 *
 * Expected outcome
 * ----------------
 * UART prints guest progress, then the monitor prints that it returned from
 * handoff with exit code 42 and PASS on the second cold handoff.
 */

#include "xil_mode_switch.h"
#include "xil_printf.h"

#ifndef __riscv
#error "riscv_modeswitch_guest.c is for RISC-V (MicroBlaze RISC-V) only"
#endif

/** Small U-mode-only routine: iterative checksum over a buffer (no CSRs). */
static u32 guest_u_mode_checksum(const u8 *buf, u32 len)
{
	u32 sum = 0U;
	u32 i;

	for (i = 0U; i < len; i++) {
		sum = (sum + (u32)buf[i]) ^ (i * 17U);
	}
	return sum;
}

/** Exit status passed to the monitor via XIL_SYS_EXIT. */
__attribute__((weak)) long guest_exit_code(void)
{
	return 42L;
}

int main(void)
{
	static const u8 payload[] = "mode-switch-guest-u-mode";
	u32 cksum;
	long code = guest_exit_code();

	print("Guest: running in U-mode\r\n");

	/* U-mode work: compute a checksum, then report (still in U-mode). */
	cksum = guest_u_mode_checksum(payload, sizeof(payload) - 1U);
	xil_printf("Guest: U-mode checksum=0x%08x\r\n", cksum);

	xil_printf("Guest: ECALL EXIT (code=%ld)\r\n", code);
	(void)Xil_Syscall1(XIL_SYS_EXIT, code);

	/* Unreachable — XIL_SYS_EXIT ends the session in the monitor. */
	return 0;
}
