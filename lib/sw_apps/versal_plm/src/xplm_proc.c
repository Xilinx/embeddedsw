/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_proc.c
*
* This file contains the processor related code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/27/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_proc.h"
#include "xplm_main.h"

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern u32 _stack;
extern u32 _stack_end;

/*****************************************************************************/
/**
 * This function enables the exceptions and interrupts
 * Enable interrupts from the hardware
 *
 * @param
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XPlm_ExceptionInit(void )
{
	u32 Index;
	int Status;
	Xil_ExceptionInit();

	/* Register exception handlers */
	for (Index = XIL_EXCEPTION_ID_FIRST;
	     Index <= XIL_EXCEPTION_ID_LAST; Index++)
	{
		Status = XPLMI_UPDATE_STATUS(XPLM_ERR_EXCEPTION, Index);
		Xil_ExceptionRegisterHandler(Index,
			     (Xil_ExceptionHandler)XPlm_ExceptionHandler,
			     (void *)Status);
	}

	/** Write stack high and low register for stack protection */
	mtslr(&_stack_end);
	mtshr(&_stack);

	microblaze_enable_exceptions();
}

/*****************************************************************************/
/**
 * This is a function handler for exceptions
 *
 * @param
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XPlm_ExceptionHandler(u32 Status)
{
	XPlmi_Printf(DEBUG_GENERAL, "Received Exception \n\r"
		      "MSR: 0x%08x, EAR: 0x%08x, EDR: 0x%08x, ESR: 0x%08x, \n\r"
		      "R14: 0x%08x, R15: 0x%08x, R16: 0x%08x, R17: 0%08x \n\r",
		      mfmsr(), mfear(), mfedr(), mfesr(),
		      mfgpr(r14), mfgpr(r15), mfgpr(r16), mfgpr(r17));

	XPlmi_ErrMgr(Status);

	/* Just in case if it returns */
	while(1);
}

/*****************************************************************************/
/**
 * This function initializes the processor, enables exceptions and start
 * timer
 *
 * @param none
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
int XPlm_InitProc(void )
{
	int Status;

	XPlm_ExceptionInit();
	Status = XPlmi_StartTimer();

	return Status;
}
