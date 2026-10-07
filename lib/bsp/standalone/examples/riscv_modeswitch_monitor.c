/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file riscv_modeswitch_monitor.c
 * @brief RISC-V M-mode monitor for the M/U mode-switch handoff example.
 *
 * Test flow (use case)
 * --------------------
 * This ELF is the *monitor* half of a two-ELF mode-switch demo. It:
 *
 *   1. Calls Xil_ModeSwitchInit() once (PMP + mscratch setup for U handoff).
 *   2. Performs a cold handoff to the guest ELF at GUEST_BOOT_PA (_boot).
 *   3. Waits until the guest ECALLs XIL_SYS_EXIT (or faults / YIELD).
 *   4. Verifies M-mode state survived the round-trip, then repeats handoff.
 *
 * The guest must already be present in memory at GUEST_BOOT_PA (XSDB dow of
 * the guest ELF, or a prior loader step). No in-monitor loader is required.
 *
 * BSP requirement
 * ---------------
 * Build against a monitor BSP with standalone_riscv_mode_switch=true. Build
 * the guest separately with standalone_riscv_user_app=true.
 *
 * GUEST_BOOT_PA (guest _boot address — not fixed)
 * ------------------------------------------------
 * GUEST_BOOT_PA must equal the physical address of the guest ELF _boot symbol.
 * It is not always 0x80200000 — that value is only a lab default. The guest
 * may link at any valid address depending on your memory map, linker script, or
 * XSDB load address. The monitor and guest are separate ELFs; Vitis does not
 * pass this value automatically from the guest build.
 *
 * After building the guest application, find _boot:
 *
 *   riscv64-unknown-elf-nm \<guest_app\>.elf | awk '$3 == "_boot" { print $1 }'
 *
 * Example output: 80200000  →  use GUEST_BOOT_PA=0x80200000U
 * Example output: 80040000  →  use GUEST_BOOT_PA=0x80040000U
 *
 * Set it on the *monitor* app in src/UserConfig.cmake (build guest first):
 *
 *   set(USER_COMPILE_DEFINITIONS "GUEST_BOOT_PA=0x80200000U")
 *
 * Replace 0x80200000U with your nm result. Rebuild the monitor after changing
 * this. Load the guest ELF at that same address before running the monitor
 * (for example XSDB dow). If USER_COMPILE_DEFINITIONS is unset, the default below
 * (0x80200000U) is used — only valid when the guest actually links there.
 *
 * Supported guest ECALLs
 * ----------------------
 *   XIL_SYS_EXIT  (1) — end session; monitor sees XIL_MS_STATUS_EXIT, value=a0.
 *   XIL_SYS_YIELD (4) — pause session; monitor sees XIL_MS_STATUS_YIELD and
 *                       may call Xil_ResumeUserSession() (not exercised here).
 * Other ECALL numbers return -1 to the guest and execution continues in U-mode.
 *
 * U-mode interrupts on a single core
 * ------------------------------------
 * Peripheral interrupts taken while the guest runs still enter the monitor's
 * _trap_handler at M-mode. The handler saves the guest context, runs the
 * registered ISR from RISCV_InterruptVectorTable, then mret's back into U-mode.
 * The monitor's M-mode cookie and stack below are preserved across handoff;
 * this test checks that with MONITOR_MSTATE_COOKIE.
 *
 * Expected UART output
 * --------------------
 *   Monitor: init OK (PMP + handoff ready)
 *   Monitor: first handoff to guest _boot @ 0x........  (your GUEST_BOOT_PA)
 *   Guest: running in U-mode
 *   Guest: U-mode checksum=0x........
 *   Guest: ECALL EXIT (code=42)
 *   Monitor: back in M-mode (tag=0xee val=42)
 *   Monitor: continuing in M-mode
 *   Monitor: second handoff ...
 *   PASS: handoff + ECALL EXIT + M-mode continue
 */

#include "xil_printf.h"
#include "xil_mode_switch.h"

#ifndef GUEST_BOOT_PA
#define GUEST_BOOT_PA	0x80200000U /**< Physical address of the guest ELF boot symbol. */
#endif

#define MONITOR_MSTATE_COOKIE	0xA0BE0001U /**< Magic cookie written to mstate by the monitor. */

static volatile u32 g_monitor_cookie;

/** @brief Perform a single monitored handoff to the guest and check exit code. */
static int monitor_handoff_once(const char *label, u32 expect_exit)
{
	int st;
	u32 tag;
	u32 val;

	xil_printf("Monitor: %s to guest _boot @ 0x%08x\r\n", label, (u32)GUEST_BOOT_PA);
	st = Xil_EnterUserMode((void (*)(void))GUEST_BOOT_PA);

	tag = Xil_HandoffStatusTag(st);
	val = Xil_HandoffStatusValue(st);
	xil_printf("Monitor: back in M-mode (tag=0x%02x val=%u)\r\n", tag >> 24U, val);

	if (g_monitor_cookie != MONITOR_MSTATE_COOKIE) {
		xil_printf("FAIL: M-mode state clobbered across handoff\r\n");
		return 1;
	}
	if (tag != XIL_MS_STATUS_EXIT) {
		xil_printf("FAIL: expected EXIT (0x%02x), got 0x%08x\r\n",
			   XIL_MS_STATUS_EXIT >> 24U, tag);
		return 1;
	}
	if (val != expect_exit) {
		xil_printf("FAIL: expected exit code %u, got %u\r\n", expect_exit, val);
		return 1;
	}
	return 0;
}

/**
 * @brief Entry point; runs the RISC-V mode-switch monitor test sequence.
 * @return 0 on success, 1 on initialization or handoff failure.
 */
int main(void)
{
	xil_printf("\r\n=== RISC-V mode-switch handoff example (monitor) ===\r\n");

	if (Xil_ModeSwitchInit() != XST_SUCCESS) {
		xil_printf("FAIL: Xil_ModeSwitchInit\r\n");
		return 1;
	}

	xil_printf("Monitor: init OK (PMP + handoff ready)\r\n");
	g_monitor_cookie = MONITOR_MSTATE_COOKIE;

	if (monitor_handoff_once("first handoff", 42U) != 0) {
		return 1;
	}

	xil_printf("Monitor: continuing in M-mode\r\n");

	/* Cold handoff again — guest boot.S re-inits; same exit code as first run. */
	if (monitor_handoff_once("second handoff", 42U) != 0) {
		return 1;
	}

	xil_printf("PASS: handoff + ECALL EXIT + M-mode continue\r\n");
	return 0;
}
