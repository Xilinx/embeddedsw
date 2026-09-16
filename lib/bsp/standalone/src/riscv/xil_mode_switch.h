/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xil_mode_switch.h
 * @brief RISC-V bare-metal M/U privilege mode switch API (MicroBlaze RISC-V RV32/RV64).
 *
 * @note standalone_riscv_mode_switch (monitor) and standalone_riscv_user_app (guest)
 *       must not both be enabled on the same BSP.
 *
 * M-mode code drops into U-mode via mret, services U-mode traps through the shared
 * _trap_handler (no dedicated mode-switch trap vector), and returns to the M-mode
 * caller when U-mode ECALLs EXIT/YIELD or faults. The YIELD snapshot is XLEN-aware
 * (see mode_switch_frame.h).
 *
 * U-mode trap flow (while Xil_UserActive)
 * ---------------------------------------
 * 1. Trap entry csrrw-swaps sp/mscratch, then saves state on the M stack.
 * 2. ECALL (mcause == 8): Xil_ModeSwitchDispatchFromUser() -> Xil_SyscallDispatch().
 * 3. Interrupts: RISCV_InterruptVectorTable dispatch (same as M-mode).
 * 4. Synchronous U faults: Xil_ModeSwitchUFaultHandler -> Xil_ModeSwitchAbort().
 *
 * Prerequisites
 * -------------
 * 1. Call Xil_ModeSwitchInit() once from M-mode before any handoff.
 * 2. Call Xil_EnterUserMode(Entry) with the guest _boot address of a separately
 *    linked user ELF (standalone_riscv_user_app ON). Guest boot.S sets sp from
 *    __stack. Permissive PMP is programmed during Init.
 *
 * Security note
 * -------------
 * Xil_ModeSwitchInit() programs PMP entry 0 as one NAPOT region over the full
 * address space (R|W|X), so U-mode execution does not fault for lack of a
 * matching PMP entry. This does not isolate U from M memory.
 */

#ifndef XIL_MODE_SWITCH_H
#define XIL_MODE_SWITCH_H

#if defined(RISCV_MODE_SWITCH) && defined(RISCV_USER_APP)
#error "RISCV_MODE_SWITCH and RISCV_USER_APP are mutually exclusive BSP options"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xstatus.h"

/** U-mode ECALL (a7) service number: terminate the guest session. */
#define XIL_SYS_EXIT		1U
/** U-mode ECALL (a7) service number: pause the guest session for a later warm resume. */
#define XIL_SYS_YIELD		4U

#ifdef RISCV_MODE_SWITCH

#include "mode_switch_frame.h"

/** Xil_HandoffStatusTag() value: session ended because U faulted (synchronous exception). */
#define XIL_MS_STATUS_ABORT	0xAB000000U
/** Xil_HandoffStatusTag() value: session paused via XIL_SYS_YIELD; can be warm-resumed. */
#define XIL_MS_STATUS_YIELD	0xBD000000U
/** Xil_HandoffStatusTag() value: session ended via XIL_SYS_EXIT. */
#define XIL_MS_STATUS_EXIT	0xEE000000U

/** High byte of a handoff status: distinguishes ABORT/YIELD/EXIT/the XIL_MS_ERR_* codes. */
#define XIL_MS_STATUS_TAG_MASK	0xFF000000U
/** Low 3 bytes of a handoff status: EXIT's a0 value, or the mcause low bits on ABORT. */
#define XIL_MS_STATUS_VALUE_MASK	0x00FFFFFFU

/** Xil_EnterUserMode()/Xil_ResumeUserSession() error: Xil_ModeSwitchInit() not yet called. */
#define XIL_MS_ERR_NOT_READY	0xE1000000U
/** Xil_EnterUserMode()/Xil_ResumeUserSession() error: a U session is already active. */
#define XIL_MS_ERR_USER_ACTIVE	0xE2000000U
/** Xil_EnterUserMode() error: Entry failed validation; see XIL_MS_ERR_SUB(). */
#define XIL_MS_ERR_VALIDATE	0xE3000000U
/** Xil_EnterUserMode()/Xil_ResumeUserSession() error: the current M stack pointer is unusable. */
#define XIL_MS_ERR_M_STACK	0xE4000000U
/** Xil_ResumeUserSession() error: no saved YIELD frame to resume. */
#define XIL_MS_ERR_NO_RESUME	0xE5000000U

/**
 * Extracts the low-byte sub-code from an XIL_MS_ERR_* status, for example the
 * Xil_ValidateHandoff() reason.
 * @param[in] st Status value returned by Xil_EnterUserMode() or Xil_ResumeUserSession().
 */
#define XIL_MS_ERR_SUB(st)	((u32)(st) & 0xFFU)

/**
 * @brief Returns which kind of result a handoff call returned.
 *
 * @param[in] Status Value returned by Xil_EnterUserMode() or Xil_ResumeUserSession().
 * @return One of XIL_MS_STATUS_ABORT, XIL_MS_STATUS_YIELD, XIL_MS_STATUS_EXIT, or an XIL_MS_ERR_* code.
 */
static inline u32 Xil_HandoffStatusTag(int Status)
{
	return (u32)Status & XIL_MS_STATUS_TAG_MASK;
}

/**
 * @brief Returns the extra detail that comes with a handoff result.
 *
 * @param[in] Status Value returned by Xil_EnterUserMode() or Xil_ResumeUserSession().
 * @return The exit code, fault reason, or error sub-code, depending on the result.
 */
static inline u32 Xil_HandoffStatusValue(int Status)
{
	return (u32)Status & XIL_MS_STATUS_VALUE_MASK;
}

/** Xil_GetTrapPrivilege() return value: the trapped context was running in U-mode. */
#define XIL_MODE_U		0U
/** Xil_GetTrapPrivilege() return value: the trapped context was running in M-mode. */
#define XIL_MODE_M		3U

/**
 * @brief Gets the mode switch feature ready to use.
 *
 * Call this once from M-mode before switching to U-mode for the first time.
 *
 * @return XST_SUCCESS if ready to use, or XST_FAILURE if the M stack is not usable.
 */
int Xil_ModeSwitchInit(void);

/**
 * @brief Switches the processor from M-mode to U-mode and starts running the guest.
 *
 * Entry must be the guest ELF _boot symbol (not main). The guest BSP boot path
 * initializes sp from __stack. Control returns to the caller only after the guest
 * exits, pauses with YIELD, or faults.
 *
 * @param[in] Entry Guest _boot address to mret into.
 * @return A status value showing why control came back: exit, yield, abort, or an error.
 */
int Xil_EnterUserMode(void (*Entry)(void));

/**
 * @brief Continues a guest that previously paused itself with a YIELD call.
 *
 * Restores the guest's saved registers and switches back to U-mode from where it left off.
 *
 * @return A status value showing why control came back: exit, yield, abort, or an error.
 */
int Xil_ResumeUserSession(void);

/**
 * @brief Checks whether a paused guest session is available to continue.
 *
 * @return Non-zero if Xil_ResumeUserSession() can be called, zero otherwise.
 */
int Xil_CanResumeUserSession(void);

/** @brief Throws away any paused guest session so it can no longer be resumed. */
void Xil_InvalidateUserSession(void);

/**
 * @brief Reports which privilege mode was interrupted by the current trap.
 *
 * @return XIL_MODE_U if U-mode was interrupted, or XIL_MODE_M if M-mode was interrupted.
 */
u32 Xil_GetTrapPrivilege(void);

/**
 * @brief Checks whether a U-mode guest is currently running.
 *
 * @return Non-zero while the guest is running, zero otherwise.
 */
int Xil_IsUserModeActive(void);

/**
 * @brief Ends the current guest session and returns control to M-mode.
 *
 * Called by the guest's own ECALL EXIT handling; never returns to its own caller.
 *
 * @param[in] Status Value the M-mode caller receives back from the ended session.
 */
void Xil_ExitUserMode(int Status) __attribute__((noreturn));

/**
 * @brief Stops a guest session after a fault and returns control to M-mode.
 *
 * Called automatically when a synchronous fault happens in U-mode; never returns to its own caller.
 *
 * @param[in] Mcause Fault reason (raw mcause value) that caused the abort.
 * @param[in] Mepc   Program counter at the time of the fault.
 * @param[in] Mtval  Extra fault detail from the mtval CSR.
 */
void Xil_ModeSwitchAbort(UINTPTR Mcause, UINTPTR Mepc, UINTPTR Mtval)
	__attribute__((noreturn));

/**
 * @brief Chooses what happens after a guest fault is aborted.
 *
 * @param[in] Enable Non-zero to halt the processor after an abort; zero to return to M-mode instead.
 */
void Xil_SetAbortHalt(u32 Enable);

#endif /* RISCV_MODE_SWITCH */

/**
 * @brief Makes a syscall request to the M-mode monitor from U-mode code.
 *
 * @param[in] Service Syscall number, for example XIL_SYS_EXIT or XIL_SYS_YIELD.
 * @param[in] Arg0    First argument to pass with the syscall.
 * @param[in] Arg1    Second argument to pass with the syscall.
 * @return The value the monitor sends back.
 */
static inline long Xil_Syscall(u32 Service, long Arg0, long Arg1)
{
	register long a0 asm("a0") = Arg0;
	register long a1 asm("a1") = Arg1;
	register long a7 asm("a7") = (long)Service;

	asm volatile ("ecall" : "+r"(a0) : "r"(a1), "r"(a7) : "memory");

	return a0;
}

/**
 * @brief Makes a syscall request with no arguments.
 *
 * @param[in] Nr Syscall number to request.
 * @return The value the monitor sends back.
 */
static inline long Xil_Syscall0(u32 Nr)
{
	return Xil_Syscall(Nr, 0L, 0L);
}

/**
 * @brief Makes a syscall request with one argument.
 *
 * @param[in] Nr   Syscall number to request.
 * @param[in] Arg0 Argument to pass with the syscall.
 * @return The value the monitor sends back.
 */
static inline long Xil_Syscall1(u32 Nr, long Arg0)
{
	return Xil_Syscall(Nr, Arg0, 0L);
}

/**
 * @brief Makes a syscall request with two arguments.
 *
 * @param[in] Nr   Syscall number to request.
 * @param[in] Arg0 First argument to pass with the syscall.
 * @param[in] Arg1 Second argument to pass with the syscall.
 * @return The value the monitor sends back.
 */
static inline long Xil_Syscall2(u32 Nr, long Arg0, long Arg1)
{
	return Xil_Syscall(Nr, Arg0, Arg1);
}

#ifdef __cplusplus
}
#endif

#endif /* XIL_MODE_SWITCH_H */
