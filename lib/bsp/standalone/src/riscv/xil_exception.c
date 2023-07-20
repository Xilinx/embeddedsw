/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
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
*
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
* @param    None.
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
* @param    None.
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
* @param    Id: contains the 32 bit ID of the exception source and should be
*           XIL_EXCEPTION_ID_INT or be in the range 0 to XIL_EXCEPTION_ID_LAST.
*           See xil_exception.h for further information.
* @param    Handler: handler function to be registered for exception
* @param    Data: a reference to data that will be passed to the handler
*           when it gets called.
*
****************************************************************************/
void Xil_ExceptionRegisterHandler(u32 Id, Xil_ExceptionHandler Handler,
				  void *Data)
{
	if (Id >= XIL_INTERRUPT_ID_FIRST) {
		RISCV_InterruptVectorTable[Id - XIL_INTERRUPT_ID_FIRST].
		Handler = Handler;
		RISCV_InterruptVectorTable[Id - XIL_INTERRUPT_ID_FIRST].
		CallBackRef = Data;
	} else {
		RISCV_ExceptionVectorTable[Id].Handler = Handler;
		RISCV_ExceptionVectorTable[Id].CallBackRef = Data;
	}
}


/*****************************************************************************/
/**
* @brief    Removes the handler for a specific exception Id. The stub handler
*           is then registered for this exception Id.
*
* @param    Id: contains the 32 bit ID of the exception source and should be
*           XIL_EXCEPTION_ID_INT or in the range 0 to XIL_EXCEPTION_ID_LAST.
*           See xil_exception.h for further information.
*
****************************************************************************/
void Xil_ExceptionRemoveHandler(u32 Id)
{
	if (Id >= XIL_INTERRUPT_ID_FIRST) {
		RISCV_InterruptVectorTable[Id - XIL_INTERRUPT_ID_FIRST].
		Handler = Xil_ExceptionNullHandler;
		RISCV_InterruptVectorTable[Id - XIL_EXCEPTION_ID_FIRST].
		CallBackRef = NULL;
	} else {
		RISCV_ExceptionVectorTable[Id].Handler =
			Xil_ExceptionNullHandler;
		RISCV_ExceptionVectorTable[Id].CallBackRef = NULL;
	}
}
/**
* @} End of "addtogroup riscv_exception_apis".
*/
