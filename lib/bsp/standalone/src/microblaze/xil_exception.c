/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law:
* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL FAULTS, AND
* XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS, IMPLIED,
* OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage of
* any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits, goodwill,
* or any type of  loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was  reasonably foreseeable
* or Xilinx had been advised of the  possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe  performance, such as life-support or
* safety devices or  systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any  other applications
* that could lead to death, personal  injury, or severe property or environmental
* damage  (individually and collectively, "Critical  Applications").
* Customer assumes the sole risk and liability of any use of Xilinx products in
* Critical  Applications, subject only to applicable laws and  regulations
* governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xil_exception.c
*
* This file contains implementation of exception related driver functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  hbm  07/28/09 Initial release
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

#include "microblaze_exceptions_g.h"
#include "microblaze_interrupts_i.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void microblaze_enable_exceptions(void);
extern void microblaze_disable_exceptions(void);
extern void microblaze_enable_interrupts(void);
extern void microblaze_disable_interrupts(void);

/**
* Currently HAL is an augmented part of standalone BSP, so the old definition
* of MB_ExceptionVectorTableEntry is used here.
*/

typedef struct {
   Xil_ExceptionHandler Handler;
   void *CallBackRef;
} MB_ExceptionVectorTableEntry;

/*typedef struct {
   Xil_ExceptionHandler Handler,
   void *CallBackRef,
} MB_InterruptVectorTableEntry, */

#ifdef __cplusplus
}
#endif


/************************** Variable Definitions *****************************/
extern MB_ExceptionVectorTableEntry MB_ExceptionVectorTable[XIL_EXCEPTION_ID_INT];
extern MB_InterruptVectorTableEntry MB_InterruptVectorTable;

/**
 *
 * This function is a stub handler that is the default handler that gets called
 * if the application has not setup a handler for a specific  exception. The
 * function interface has to match the interface specified for a handler even
 * though none of the arguments are used.
 *
 * @param	Data is unused by this function.
 *
 * @return
 *
 * None.
 *
 * @note
 *
 * None.
 *
 *****************************************************************************/
static void Xil_ExceptionNullHandler(void *Data)
{
	(void *) Data;
}

/****************************************************************************/
/**
*
* Initialize exception handling for the processor. The exception vector table
* is setup with the stub handler for all exceptions.
*
* @param    None.
*
* @return   None.
*
* @note
*
* None.
*
*****************************************************************************/
void Xil_ExceptionInit(void)
{
	/*
	 * there is no need to setup the exception table here
	 */

}

/****************************************************************************/
/**
* Enable Exceptions.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void Xil_ExceptionEnable(void)
{
#ifdef MICROBLAZE_EXCEPTIONS_ENABLED
	microblaze_enable_exceptions();
#endif
	microblaze_enable_interrupts();
}

/****************************************************************************/
/**
* Disable Exceptions.
*
* @param    None.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void Xil_ExceptionDisable(void)
{
#ifdef MICROBLAZE_EXCEPTIONS_ENABLED
	microblaze_disable_exceptions();
#endif
	microblaze_disable_interrupts();
}

/*****************************************************************************/
/**
*
* Makes the connection between the Id of the exception source and the
* associated handler that is to run when the exception is recognized. The
* argument provided in this call as the DataPtr is used as the argument
* for the handler when it is called.
*
* @param    Id contains the ID of the exception source and should
*           be XIL_EXCEPTION_INT or be in the range of 0 to XIL_EXCEPTION_LAST.
*	    See xil_mach_exception.h for further information.
* @param    Handler to the handler for that exception.
* @param    Data is a reference to data that will be passed to the handler
*           when it gets called.
*
* @return   None.
*
* @note
*
* None.
*
****************************************************************************/
void Xil_ExceptionRegisterHandler(u32 Id, Xil_ExceptionHandler Handler,
				  void *Data)
{
	if (Id == XIL_EXCEPTION_ID_INT) {
		MB_InterruptVectorTable.Handler = Handler;
		MB_InterruptVectorTable.CallBackRef = Data;
	}
	else {
#ifdef MICROBLAZE_EXCEPTIONS_ENABLED
		MB_ExceptionVectorTable[Id].Handler = Handler;
		MB_ExceptionVectorTable[Id].CallBackRef = Data;
#endif
	}
}


/*****************************************************************************/
/**
*
* Removes the handler for a specific exception Id. The stub handler is then
* registered for this exception Id.
*
* @param    Id contains the ID of the exception source and should
*           be XIL_EXCEPTION_INT or in the range of 0 to XIL_EXCEPTION_LAST.
*	    See xexception_l.h for further information.
*
* @return   None.
*
* @note
*
* None.
*
****************************************************************************/
void Xil_ExceptionRemoveHandler(u32 Id)
{
	if (Id == XIL_EXCEPTION_ID_INT) {
		MB_InterruptVectorTable.Handler = Xil_ExceptionNullHandler;
		MB_InterruptVectorTable.CallBackRef = NULL;
	}
	else {

#ifdef MICROBLAZE_EXCEPTIONS_ENABLED
		MB_ExceptionVectorTable[Id].Handler =
			Xil_ExceptionNullHandler;
		MB_ExceptionVectorTable[Id].CallBackRef = NULL;
#endif
	}
}

