/******************************************************************************
* Copyright (c) 2004 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file microblaze_exception_handler.c
*
* This file contains exception handler registration routines for
* the MicroBlaze processor.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Date     Changes
* ----- -------- -----------------------------------------------
* 1.00b 06/24/04 First release
* 8.0	03/17/22 Move MB_ExceptionVectorTableEntry declaration to
* 		 header file to fix misra_c_2012_rule_8_5 violation.
* 8.0   08/02/22 Update description for microblaze_register_exception_handler
*                to fix documentation issue with its arguments.
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "microblaze_exceptions_i.h"
#include "bspconfig.h"
#ifndef SDT
#include "microblaze_exceptions_g.h"
#endif

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/****************************************************************************/

/*****************************************************************************/
/**
*
* Registers an exception handler for the MicroBlaze. The
* argument provided in this call as the DataPtr is used as the argument
* for the handler when it is called.
*
* @param    ExceptionId: ExceptionId is the id of the exception to register
*           this handler for. It can be one of the value between XIL_EXCEPTION_ID_FIRST
*           and XIL_EXCEPTION_ID_LAST as defined in xil_exception.h.
* @param    Handler: Top level Exception handler.
* @param    DataPtr: DataPtr is a reference to data that will be passed to the handler
*           when it gets called.
* @return   None.
*
****************************************************************************/
#if defined(MICROBLAZE_EXCEPTIONS_ENABLED) || defined(SDT)
void microblaze_register_exception_handler(u32 ExceptionId, Xil_ExceptionHandler Handler, void *DataPtr)
{
   MB_ExceptionVectorTable[ExceptionId].Handler = Handler;
   MB_ExceptionVectorTable[ExceptionId].CallBackRef = DataPtr;
}
#endif
