/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All rights reserved.
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
#define XPAR_MICROBLAZE_RISCV_USE_MMU 0
#endif

extern void _trap_handler(void);

/****************************************************************************/
/**
 * @brief   This function is a stub handler that is the default handler that gets
 *          called if the application has not setup a handler for a specific
 *          exception. The function interface has to match the interface
 *          specified for a handler even though none of the arguments are used.
 *
 * @param	Data: unused by this function.
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
* @return   None.
*
*****************************************************************************/
void Xil_ExceptionInit(void)
{
	csrwi(XREG_MIE, 0);
#if XPAR_MICROBLAZE_RISCV_USE_MMU > 1
	csrwi(XREG_MEDELEG, 0);
	csrwi(XREG_MIDELEG, 0);
#endif
	csrw(XREG_MTVEC, &_trap_handler);
}

/****************************************************************************/
/**
* @brief   Initialize mtvec with valid trap handler address
*
* @param    None.
*
* @return   None.
*
*****************************************************************************/
void __attribute__ ((constructor)) Xil_RegMtvecInit(void)
{
        csrw(XREG_MTVEC, &_trap_handler);
}

/****************************************************************************/
/**
* @brief    Enable external, timer, and software interrupts.
*
* @return   None.
*
******************************************************************************/
void Xil_ExceptionEnable(void)
{
	csrw(XREG_MIE, XREG_MIE_MEIE_MASK | XREG_MIE_MTIE_MASK | XREG_MIE_MSIE_MASK);
	csrsi(XREG_MSTATUS, XREG_MSTATUS_MIE_MASK);
}

/****************************************************************************/
/**
* @brief    Disable external, timer, and software interrupts.
*
* @return   None.
*
******************************************************************************/
void Xil_ExceptionDisable(void)
{
	csrwi(XREG_MIE, 0);
	csrci(XREG_MSTATUS, XREG_MSTATUS_MIE_MASK);
}

/*****************************************************************************/
/**
*@brief     Makes the connection between the Id of the exception source and the
*           associated handler that is to run when the exception is recognized.
*           The argument provided in this call as the DataPtr is used as the
*           argument for the handler when it is called.
*
* @param    Exception_id : contains the 32 bit ID of the exception source and should be
*           in the range XIL_INTERRUPT_ID_FIRST to XIL_INTERRUPT_ID_LAST.
*           See xil_exception.h for further information.
* @param    Handler : handler function to be registered for exception
* @param    Data : a reference to data that will be passed to the handler
*           when it gets called.
*
****************************************************************************/
void Xil_ExceptionRegisterHandler(u32 Exception_id, Xil_ExceptionHandler Handler,
				  void *Data)
{
	if (Exception_id >= XIL_INTERRUPT_ID_FIRST && Exception_id <= XIL_INTERRUPT_ID_LAST) {
		RISCV_InterruptVectorTable[Exception_id - XIL_INTERRUPT_ID_FIRST].
		Handler = Handler;
		RISCV_InterruptVectorTable[Exception_id - XIL_INTERRUPT_ID_FIRST].
		CallBackRef = Data;
	}
}


/*****************************************************************************/
/**
* @brief    Removes the handler for a specific exception Id. The stub handler
*           is then registered for this exception Id.
*
* @param    Exception_id : contains the 32 bit ID of the exception source and should be
*           in the range XIL_INTERRUPT_ID_FIRST to XIL_INTERRUPT_ID_LAST.
*           See xil_exception.h for further information.
*
****************************************************************************/
void Xil_ExceptionRemoveHandler(u32 Exception_id)
{
	if (Exception_id >= XIL_INTERRUPT_ID_FIRST && Exception_id <= XIL_INTERRUPT_ID_LAST) {
		RISCV_InterruptVectorTable[Exception_id - XIL_INTERRUPT_ID_FIRST].
		Handler = Xil_ExceptionNullHandler;
		RISCV_InterruptVectorTable[Exception_id - XIL_INTERRUPT_ID_FIRST].
		CallBackRef = NULL;
	}
}
/**
* @} End of "addtogroup riscv_exception_apis".
*/
