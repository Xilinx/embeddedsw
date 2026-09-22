/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xil_mode_switch.c
 * @brief RISC-V M/U mode switch: init, handoff, syscalls, abort.
 *
 * U-mode traps are serviced by the shared _trap_handler (trap_handler.S).
 * ECALL from U is dispatched to Xil_ModeSwitchDispatchFromUser(); interrupts
 * and synchronous U faults use the existing vector tables. M-mode faults chain
 * to handlers registered before Xil_ModeSwitchInit() when no U session is active.
 */

#ifdef RISCV_MODE_SWITCH

#include "xil_mode_switch.h"
#include "xreg_riscv.h"
#include "xpseudo_asm.h"
#include "xil_exception.h"
#include <stddef.h>

/** @cond INTERNAL */
extern char _stack_end[];

volatile u32 Xil_ModeSwitchReady;   /**< Set once Xil_ModeSwitchInit() has completed. */
volatile u32 Xil_InTrap;            /**< Reentrancy guard: set while a U trap is being serviced. */
volatile u32 Xil_UserActive;        /**< Set while a U-mode session is running (between mret and return). */
void *Xil_MContFramePtr;            /**< Base of the current M continuation frame (mode_switch_enter.S). */

static volatile u32 Xil_LastStatus;    /**< Status returned to the M caller of the current/last session. */
static volatile u32 Xil_ExitViaSyscall; /**< Set when EXIT/YIELD (not an abort) is unwinding to M. */
static volatile u32 Xil_AbortHalt;     /**< Xil_ModeSwitchAbort() policy: halt (1) or return to M (0). */

static Xil_GuestSession Xil_GuestSavedFrame; /**< Snapshot of the last YIELD'd guest session. */
static volatile u32 Xil_GuestSavedValid;     /**< Non-zero while Xil_GuestSavedFrame holds a resumable session. */
/** @endcond */

/**
 * @brief One per synchronous-fault exception ID that Xil_ModeSwitchInit() takes over.
 *
 * Records what was registered on that ID before Init() overwrote it with
 * Xil_ModeSwitchUFaultHandler, so a fault trapped from M-mode (not U) can still
 * be chained to whatever the application had registered there previously.
 */
typedef struct {
	u32 id;                            /**< Exception ID (matches the raw mcause code). */
	Xil_ExceptionHandler prev_handler; /**< Handler registered on this ID before Init(). */
	void *prev_data;                   /**< CallBackRef registered on this ID before Init(). */
} Xil_ModeSwitchFaultSlot;

static void Xil_HaltForever(void) __attribute__((noreturn));
static int Xil_CurrentMsp(UINTPTR *MspOut);
static u32 Xil_ValidateHandoff(void (*Entry)(void));
static void Xil_EnableUserModeAccess(void);
static void Xil_InvalidateGuestSavedFrame(void);
static void Xil_SaveGuestYieldFrame(UINTPTR *Gpr);
static UINTPTR Xil_AdvanceEpcEcall(void);
static long Xil_SyscallDispatch(UINTPTR *Gpr);
static void Xil_ModeSwitchUFaultHandler(void *Data);

extern void XNullHandler(void *NullParameter);
extern void Xil_EnterUserModeAsm(void (*Entry)(void));
extern void Xil_ResumeGuestSessionAsm(Xil_GuestSession *Session);
extern void Xil_ReturnToMMode(void) __attribute__((noreturn));

/** Masks mcause down to just the synchronous exception code (drops the interrupt bit). */
static u32 Xil_McauseSyncCode(UINTPTR Cause)
{
#if __riscv_xlen == 64
	return (u32)(Cause & 0x7FFFFFFFFFFFFFFFULL);
#else
	return (u32)(Cause & 0x7FFFFFFFU);
#endif
}

/**
 * @brief Chooses what happens after a guest fault is aborted.
 *
 * @param[in] Enable Non-zero to halt the processor after an abort; zero to return to M-mode instead.
 */
void Xil_SetAbortHalt(u32 Enable)
{
	Xil_AbortHalt = Enable;
}

/** Registered handler for the fault IDs in Xil_ModeSwitchInit(); aborts U, chains M. */
static void Xil_ModeSwitchUFaultHandler(void *Data)
{
	Xil_ModeSwitchFaultSlot *slot = (Xil_ModeSwitchFaultSlot *)Data;

	if (Xil_GetTrapPrivilege() != XIL_MODE_U) {
		if ((slot->prev_handler != NULL) && (slot->prev_handler != XNullHandler)) {
			slot->prev_handler(slot->prev_data);
		}
		return;
	}
	Xil_ModeSwitchAbort((UINTPTR)slot->id, (UINTPTR)csrr(XREG_MEPC),
			    (UINTPTR)csrr(XREG_MTVAL));
}

/** Reads the live M stack pointer and rejects it if it has underflowed past _stack_end. */
static int Xil_CurrentMsp(UINTPTR *MspOut)
{
	UINTPTR msp = (UINTPTR)mfgpr(2);

	if (msp <= (UINTPTR)&_stack_end) {
		return XST_FAILURE;
	}
	if (MspOut != NULL) {
		*MspOut = msp;
	}
	return XST_SUCCESS;
}

/**
 * @brief Gets the mode switch feature ready to use.
 *
 * Call this once from M-mode before switching to U-mode for the first time.
 *
 * @return XST_SUCCESS if ready to use, or XST_FAILURE if the M stack is not usable.
 */
int Xil_ModeSwitchInit(void)
{
	static const u32 FaultIds[] = {
		XIL_EXCEPTION_ID_IACCESS_FAULT,
		XIL_EXCEPTION_ID_ILLEGAL_INSTR,
		XIL_EXCEPTION_ID_BREAKPOINT,
		XIL_EXCEPTION_ID_LACCESS_FAULT,
		XIL_EXCEPTION_ID_SACCESS_FAULT,
		XIL_EXCEPTION_ID_IPAGE_FAULT,
		XIL_EXCEPTION_ID_LPAGE_FAULT,
		XIL_EXCEPTION_ID_SPAGE_FAULT,
	};
	static Xil_ModeSwitchFaultSlot FaultSlots[sizeof(FaultIds) / sizeof(FaultIds[0])];
	u32 i;

	if (Xil_ModeSwitchReady != 0U) {
		return XST_SUCCESS;
	}

	if (Xil_CurrentMsp(NULL) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Xil_MContFramePtr = NULL;
	Xil_UserActive = 0U;
	Xil_InTrap = 0U;
	Xil_LastStatus = 0U;
	Xil_ExitViaSyscall = 0U;
	Xil_AbortHalt = 0U;
	Xil_InvalidateGuestSavedFrame();
	Xil_EnableUserModeAccess();

	for (i = 0U; i < (u32)(sizeof(FaultIds) / sizeof(FaultIds[0])); i++) {
		FaultSlots[i].id = FaultIds[i];
		FaultSlots[i].prev_handler = RISCV_ExceptionVectorTable[FaultIds[i]].Handler;
		FaultSlots[i].prev_data = RISCV_ExceptionVectorTable[FaultIds[i]].CallBackRef;
		Xil_ExceptionRegisterHandler(FaultIds[i], Xil_ModeSwitchUFaultHandler,
					     (void *)&FaultSlots[i]);
	}

	Xil_ModeSwitchReady = 1U;
	return XST_SUCCESS;
}

/**
 * @brief Switches the processor from M-mode to U-mode and starts running the guest.
 *
 * Control returns to the caller only after the guest exits, pauses with YIELD, or faults.
 *
 * @param[in] Entry Guest _boot address; guest boot.S sets sp from __stack.
 * @return A status value showing why control came back: exit, yield, abort, or an error.
 */
int Xil_EnterUserMode(void (*Entry)(void))
{
	u32 verr;

	if (Xil_ModeSwitchReady == 0U) {
		return (int)XIL_MS_ERR_NOT_READY;
	}
	if (Xil_UserActive != 0U) {
		return (int)XIL_MS_ERR_USER_ACTIVE;
	}

	if (Xil_CurrentMsp(NULL) != XST_SUCCESS) {
		return (int)(XIL_MS_ERR_M_STACK | 1U);
	}

	verr = Xil_ValidateHandoff(Entry);
	if (verr != 0U) {
		return (int)(XIL_MS_ERR_VALIDATE | verr);
	}

	Xil_InvalidateGuestSavedFrame();

	Xil_UserActive = 1U;
	Xil_ExitViaSyscall = 0U;
	Xil_InTrap = 0U;
	Xil_EnterUserModeAsm(Entry);
	Xil_MContFramePtr = NULL;
	Xil_UserActive = 0U;
	Xil_InTrap = 0U;
	return (int)Xil_LastStatus;
}

/**
 * @brief Checks whether a paused guest session is available to continue.
 *
 * @return Non-zero if Xil_ResumeUserSession() can be called, zero otherwise.
 */
int Xil_CanResumeUserSession(void)
{
	return (Xil_GuestSavedValid != 0U) ? 1 : 0;
}

/** @brief Throws away any paused guest session so it can no longer be resumed. */
void Xil_InvalidateUserSession(void)
{
	Xil_InvalidateGuestSavedFrame();
}

/**
 * @brief Continues a guest that previously paused itself with a YIELD call.
 *
 * Restores the guest's saved registers and switches back to U-mode from where it left off.
 *
 * @return A status value showing why control came back: exit, yield, abort, or an error.
 */
int Xil_ResumeUserSession(void)
{
	int st;

	if (Xil_ModeSwitchReady == 0U) {
		return (int)XIL_MS_ERR_NOT_READY;
	}
	if (Xil_UserActive != 0U) {
		return (int)XIL_MS_ERR_USER_ACTIVE;
	}
	if (Xil_CanResumeUserSession() == 0) {
		return (int)XIL_MS_ERR_NO_RESUME;
	}

	if (Xil_CurrentMsp(NULL) != XST_SUCCESS) {
		return (int)(XIL_MS_ERR_M_STACK | 1U);
	}

	Xil_UserActive = 1U;
	Xil_ExitViaSyscall = 0U;
	Xil_InTrap = 0U;
	Xil_ResumeGuestSessionAsm(&Xil_GuestSavedFrame);
	Xil_MContFramePtr = NULL;
	Xil_UserActive = 0U;
	Xil_InTrap = 0U;
	st = (int)Xil_LastStatus;
	return st;
}

/**
 * @brief Reports which privilege mode was interrupted by the current trap.
 *
 * @return XIL_MODE_U if U-mode was interrupted, or XIL_MODE_M if M-mode was interrupted.
 */
u32 Xil_GetTrapPrivilege(void)
{
	u32 mstatus;

	mstatus = csrr(XREG_MSTATUS);
	return (mstatus & XREG_MSTATUS_MPP_MASK) >> XREG_MSTATUS_MPP_SHIFT;
}

/**
 * @brief Checks whether a U-mode guest is currently running.
 *
 * @return Non-zero while the guest is running, zero otherwise.
 */
int Xil_IsUserModeActive(void)
{
	return (Xil_UserActive != 0U) ? 1 : 0;
}

/**
 * @brief Ends the current guest session and returns control to M-mode.
 *
 * Called by the guest's own ECALL EXIT handling; never returns to its own caller.
 *
 * @param[in] Status Value the M-mode caller receives back from the ended session.
 */
void Xil_ExitUserMode(int Status)
{
	Xil_UserActive = 0U;
	Xil_LastStatus = XIL_MS_STATUS_EXIT |
			 ((u32)Status & XIL_MS_STATUS_VALUE_MASK);
	if (Xil_ExitViaSyscall != 0U) {
		Xil_InTrap = 0U;
		Xil_ReturnToMMode();
	}
	Xil_HaltForever();
}

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
{
	(void)Mepc;
	(void)Mtval;

	Xil_InvalidateGuestSavedFrame();
	Xil_UserActive = 0U;
	Xil_InTrap = 0U;
	Xil_LastStatus = XIL_MS_STATUS_ABORT |
			 (Xil_McauseSyncCode(Mcause) & XIL_MS_STATUS_VALUE_MASK);
	if (Xil_AbortHalt != 0U) {
		Xil_HaltForever();
	}
	Xil_ReturnToMMode();
}

/**
 * @brief Handles a U-mode ECALL trap; called from the shared trap handler.
 *
 * Reads the syscall number and arguments from the trapped guest registers and runs it.
 *
 * @param[in] Gpr Pointer to the trapped guest's saved general-purpose registers.
 */
void Xil_ModeSwitchDispatchFromUser(UINTPTR *Gpr)
{
	if (Xil_InTrap != 0U) {
		Xil_ModeSwitchAbort((UINTPTR)csrr(XREG_MCAUSE), (UINTPTR)csrr(XREG_MEPC),
				    (UINTPTR)csrr(XREG_MTVAL));
	}
	Xil_InTrap = 1U;

	(void)Xil_SyscallDispatch(Gpr);
	Xil_InTrap = 0U;
}

/** Advances mepc past the ECALL that trapped, so a serviced/resumed guest does not re-trap on it. */
static UINTPTR Xil_AdvanceEpcEcall(void)
{
	UINTPTR pc = (UINTPTR)csrr(XREG_MEPC);

	pc += 4U;
	csrw(XREG_MEPC, pc);
	return pc;
}

/** Programs PMP entry 0 as one full-address-space NAPOT region, R|W|X, for U-mode access. */
static void Xil_EnableUserModeAccess(void)
{
	/*
	 * One NAPOT R|W|X region over the full address space so U-mode fetches
	 * and loads/stores do not fault for lack of a matching PMP entry.
	 * Programmed once at Init; not tied to C_USE_MMU (PMP is independent).
	 */
	csrw(XREG_PMPADDR0, (UINTPTR)(-1));
	csrw(XREG_PMPCFG0, XREG_PMPCFG_A_NAPOT | XREG_PMPCFG_R_MASK |
	     XREG_PMPCFG_W_MASK | XREG_PMPCFG_X_MASK);
}

/** Decodes a7/a0 from the trapped GPR frame and services EXIT/YIELD; anything else returns -1. */
static long Xil_SyscallDispatch(UINTPTR *Gpr)
{
	UINTPTR nr = Gpr[17];
	long a0 = (long)Gpr[10];

	switch ((u32)nr) {
	case XIL_SYS_EXIT:
		Xil_InvalidateGuestSavedFrame();
		Xil_ExitViaSyscall = 1U;
		Xil_ExitUserMode((int)a0);
		break;
	case XIL_SYS_YIELD:
		Xil_SaveGuestYieldFrame(Gpr);
		Xil_ExitViaSyscall = 1U;
		Xil_LastStatus = XIL_MS_STATUS_YIELD;
		Xil_UserActive = 0U;
		Xil_InTrap = 0U;
		Xil_ReturnToMMode();
		break;
	default:
		Gpr[10] = (UINTPTR)(-(long)XST_FAILURE);
		(void)Xil_AdvanceEpcEcall();
		break;
	}
	return (long)Gpr[10];
}

/** Rejects a NULL Entry (guest _boot address). */
static u32 Xil_ValidateHandoff(void (*Entry)(void))
{
	if (Entry == NULL) {
		return 1U;
	}
	return 0U;
}

/** Marks the saved YIELD frame dead; a later Xil_ResumeUserSession() fails. */
static void Xil_InvalidateGuestSavedFrame(void)
{
	Xil_GuestSavedValid = 0U;
}

/** Snapshots the trapped GPRs/mepc/mstatus into Xil_GuestSavedFrame for a later warm resume. */
static void Xil_SaveGuestYieldFrame(UINTPTR *Gpr)
{
	UINTPTR i;

	for (i = 0U; i < 32U; i++) {
		Xil_GuestSavedFrame.gpr[i] = Gpr[i];
	}
	Xil_GuestSavedFrame.mepc = Xil_AdvanceEpcEcall();
	Xil_GuestSavedFrame.mstatus = (u32)csrr(XREG_MSTATUS) & XIL_MS_MSTATUS_SAVE_MASK;
	Xil_GuestSavedValid = 1U;
}

/** Disables interrupts and spins forever; the terminal state for an unrecoverable exit/abort. */
static void Xil_HaltForever(void)
{
	csrci(XREG_MSTATUS, XREG_MSTATUS_MIE_MASK);
	for (;;) {
		asm volatile("nop");
	}
}

#endif /* RISCV_MODE_SWITCH */
