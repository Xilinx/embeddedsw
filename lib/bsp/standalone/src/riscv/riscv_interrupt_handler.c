/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file riscv_interrupt_handler.c
*
* This file contains the standard interrupt handler for the RISC-V processor.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   mus  11/21/22 First release
* 9.3   ml   02/19/25 Add support for RISC-V Interrupt Handling
*
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/

#include "xil_exception.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
void riscv_register_handler(XInterruptHandler Handler, void *DataPtr);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* @brief  Registers a top-level interrupt handler for the processor. The
*         argument provided in this call as the DataPtr is used as the argument
* 	  for the handler when it is called.
*
* @param  Handler: Top level handler.
* @param  DataPtr: a reference to data that will be passed to the handler
*         when it gets called.

* @return None.
*
*
****************************************************************************/
void riscv_register_handler(XInterruptHandler Handler, void *DataPtr)
{
	RISCV_InterruptVectorTable[XIL_INTERRUPT_ID_MACHINE_EXTERNAL - XIL_INTERRUPT_ID_FIRST].
		Handler = Handler;
	RISCV_InterruptVectorTable[XIL_INTERRUPT_ID_MACHINE_EXTERNAL - XIL_INTERRUPT_ID_FIRST].
		CallBackRef = DataPtr;
}
