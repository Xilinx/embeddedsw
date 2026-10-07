/**
 *@cond nocomments
 */
/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_exception.c
*
* This file contains implementation of exception related driver functions.
*
* @addtogroup riscv_exception_apis RISC-V exception APIs
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 9.0   sa   08/27/22 Initial release
* 9.0   sa   07/20/23 Update Xil_ExceptionInit to access MEDELEG and MIDELEG
*                     CSRs only when supervisory mode is implemented.
* 9.2   ml   17/01/24 Modified description and code for Xil_ExceptionRegisterHandler and
*                     Xil_ExceptionRemoveHandler API's to fix doxygen warnings.
* 9.3   ml   02/19/25 Add support for RISC-V Interrupt Handling
* 9.4   ml   11/06/25 Add support for RISC-V Exception Handling
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
* @note
*
* None.
*
******************************************************************************/

#include "xil_types.h"
#include "xil_exception.h"

#include "xpseudo_asm.h"
#include "xparameters.h"

#ifndef XPAR_MICROBLAZE_RISCV_USE_MMU
#define XPAR_MICROBLAZE_RISCV_USE_MMU 0 /**< Default: MMU not present (0=no, 1=S-mode, 2=S+U-mode). */
#endif

extern void _trap_handler(void); /**< Low-level trap handler entry point (defined in assembly). */

/****************************************************************************/
/**
 * @brief   Default stub exception handler that gets
 *          called if the application has not setup a handler for a specific
 *          exception. The function interface has to match the interface
 *          specified for a handler even though none of the arguments are used.
 *
 * @param[in]	Data Unused by this function.
 *
 *****************************************************************************/
static void Xil_ExceptionNullHandler(void *Data)
{
	(void) Data;
}

/****************************************************************************/
/**
* @brief   Initialize exception handling for the processor. The exception
*          vector table is setup with the stub handler for all exceptions.
*

*****************************************************************************/
void Xil_ExceptionInit(void)
{
#ifdef RISCV_USER_APP
	/*
	 * Guest (U-mode) build: mtvec/mie/mstatus are M-only CSRs. The guest never
	 * owns a trap vector or interrupt-enable state - all of that lives in the
	 * monitor, which services the guest's traps/interrupts/ECALLs via the
	 * shared _trap_handler. No-op here.
	 */
	return;
#else
	/*
	 * mtvec always points at the shared _trap_handler, mode-switch monitor
	 * builds included - it never diverges to a separate U-mode vector, so
	 * there is nothing session-dependent to handle here.
	 */
	csrwi(XREG_MIE, 0);
#if XPAR_MICROBLAZE_RISCV_USE_MMU > 1
	csrwi(XREG_MEDELEG, 0);
	csrwi(XREG_MIDELEG, 0);
#endif
	csrw(XREG_MTVEC, &_trap_handler);
#endif
}

/****************************************************************************/
/**
* @brief   Initialize mtvec with valid trap handler address
*

*****************************************************************************/
void __attribute__ ((constructor)) Xil_RegMtvecInit(void)
{
#ifndef RISCV_USER_APP
        csrw(XREG_MTVEC, &_trap_handler);
#endif
}

/****************************************************************************/
/**
* @brief    Enable external, timer, and software interrupts.
*

******************************************************************************/
void Xil_ExceptionEnable(void)
{
#ifndef RISCV_USER_APP
	csrw(XREG_MIE, XREG_MIE_MEIE_MASK | XREG_MIE_MTIE_MASK | XREG_MIE_MSIE_MASK);
	csrsi(XREG_MSTATUS, XREG_MSTATUS_MIE_MASK);
#endif
}

/****************************************************************************/
/**
* @brief    Disable external, timer, and software interrupts.
*

******************************************************************************/
void Xil_ExceptionDisable(void)
{
#ifndef RISCV_USER_APP
	csrwi(XREG_MIE, 0);
	csrci(XREG_MSTATUS, XREG_MSTATUS_MIE_MASK);
#endif
}

/*****************************************************************************/
/**
*@brief     Makes the connection between the Id of the exception source and the
*           associated handler that is to run when the exception is recognized.
*           The argument provided in this call as the DataPtr is used as the
*           argument for the handler when it is called.
*
*           in the range XIL_INTERRUPT_ID_FIRST to XIL_INTERRUPT_ID_LAST.
*           See xil_exception.h for further information.
*           when it gets called.
*
****************************************************************************/
/**
 *@endcond
 */
void Xil_ExceptionRegisterHandler(u32 Exception_id, Xil_ExceptionHandler Handler,
				  void *Data)
{
	/* Reject NULL only; XNullHandler is a valid public no-op stub. */
	if (Handler == NULL) {
		return;
	}
	if (Exception_id >= XIL_INTERRUPT_ID_FIRST && Exception_id <= XIL_INTERRUPT_ID_LAST) {
		RISCV_InterruptVectorTable[Exception_id - XIL_INTERRUPT_ID_FIRST].
		Handler = Handler;
		RISCV_InterruptVectorTable[Exception_id - XIL_INTERRUPT_ID_FIRST].
		CallBackRef = Data;
	} else if (Exception_id <= XIL_EXCEPTION_ID_LAST) {
		RISCV_ExceptionVectorTable[Exception_id].Handler = Handler;
		RISCV_ExceptionVectorTable[Exception_id].CallBackRef = Data;
	}
}


/**
 *@cond nocomments
 */
/*****************************************************************************/
/**
* @brief    Removes the handler for a specific exception Id. The stub handler
*           is then registered for this exception Id.
*
*           in the range XIL_INTERRUPT_ID_FIRST to XIL_INTERRUPT_ID_LAST.
*           See xil_exception.h for further information.
*
****************************************************************************/
/**
 *@endcond
 */
void Xil_ExceptionRemoveHandler(u32 Exception_id)
{
	if (Exception_id >= XIL_INTERRUPT_ID_FIRST && Exception_id <= XIL_INTERRUPT_ID_LAST) {
		RISCV_InterruptVectorTable[Exception_id - XIL_INTERRUPT_ID_FIRST].
		Handler = Xil_ExceptionNullHandler;
		RISCV_InterruptVectorTable[Exception_id - XIL_INTERRUPT_ID_FIRST].
		CallBackRef = NULL;
	} else if (Exception_id <= XIL_EXCEPTION_ID_LAST) {
		RISCV_ExceptionVectorTable[Exception_id].Handler = Xil_ExceptionNullHandler;
		RISCV_ExceptionVectorTable[Exception_id].CallBackRef = NULL;
	}
}
/**
 *@cond nocomments
 */
/**
* @} End of "addtogroup riscv_exception_apis".
*/
/**
 *@endcond
 */
