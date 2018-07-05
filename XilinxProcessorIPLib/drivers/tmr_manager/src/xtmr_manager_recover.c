/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xtmr_manager_recover.c
* @addtogroup tmr_manager_v1_0
* @{
*
* This file contains the recovery handling functions for the TMR Manager
* component (XTMR_Manager).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xtmr_manager.h"
#include "xtmr_manager_l.h"
#include "xil_io.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/

extern u32 XTMR_Manager_StackPointer;
extern XTMR_Manager *XTMR_Manager_InstancePtr;

/************************** Function Prototypes *****************************/


/****************************************************************************/
/**
*
* Break occurred signalling that a recovery should be performed. Call the
* prerecovery user handler, and then suspend the processor, to signal to
* the TMR Manager hardware that it should reset the TMR sub-system.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
*
* @return	None.
*
* @note		Called from break vector, with interrupts disabled.
*
****************************************************************************/

void XTMR_Manager_BreakHandler (XTMR_Manager *InstancePtr)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((InstancePtr->Cr & XTM_CR_RIR) != 0);
	Xil_AssertVoid((InstancePtr->Cr & XTM_CR_MAGIC1_MASK) ==
		       XPAR_TMR_MANAGER_0_MAGIC1);

	/* Call user defined pre-recovery handler, if any */
	if (InstancePtr->PreResetHandler != NULL)
		InstancePtr->PreResetHandler(
				InstancePtr->PreResetCallBackRef);
}

/****************************************************************************/
/**
*
* Detect and handle recovery reset or cold reset.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
*
* @return	Whether the reset was recovery reset (1) or cold reset (0).
*
* @note		Called from assembler context. The assembler routine ensures
*               that the context is restored when doing a recovery reset.
*
****************************************************************************/
int XTMR_Manager_ResetHandler (XTMR_Manager *InstancePtr)
{
        u32 ffr;
	int rec_reset_12_13, rec_reset_12_23, rec_reset_13_23, rec_reset;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
         * Determine reset cause from TMR Manager First Failing Register. To
         * be considered a valid recovery reset, all fatal bits must be 0,
	 * the recovery bit must be 1, and two out of three lockstep
	 * mismatch bits must be set.
         */
	ffr = XTMR_Manager_GetFirstFailingReg(InstancePtr->RegBaseAddress);

	rec_reset_12_13 = (ffr & XTM_FFR_REC_12_13_MASK) ==
				(XTM_FFR_REC_MASK | XTM_FFR_LM12_LM13_MASK);
	rec_reset_12_23 = (ffr & XTM_FFR_REC_12_23_MASK) ==
				(XTM_FFR_REC_MASK | XTM_FFR_LM12_LM23_MASK);
	rec_reset_13_23 = (ffr & XTM_FFR_REC_13_23_MASK) ==
				(XTM_FFR_REC_MASK | XTM_FFR_LM13_LM23_MASK);
	rec_reset = rec_reset_12_13 || rec_reset_12_23 || rec_reset_13_23;

	if (rec_reset) {
		/* Treat as recovery reset: Handle recovery */

		/* Call user defined postrecovery handler, if any */
		if (InstancePtr->PostResetHandler != NULL)
			InstancePtr->PostResetHandler(
				InstancePtr->PostResetCallBackRef);

		/* Clear First Failing Status after successful recovery */
		XTMR_Manager_ClearFirstFailingReg(InstancePtr->RegBaseAddress);
		InstancePtr->Stats.RecoveryCount++;

		/* Restore saved context and resume execution */
		return 1;
	} else {
		/* Treat as cold reset: Do a complete program restart */
		return 0;
	}
}


/****************************************************************************/
/**
*
* Set the user recovery handler, which can replace the pre-defined handler
* completely, to do custom recovery.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
*
* @note		This function is run in interrupt context.
*
******************************************************************************/
void XTMR_Manager_SetRecoveryHandler(XTMR_Manager *InstancePtr,
				     XTMR_Manager_Handler FuncPtr,
				     void *CallBackRef)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->RecoveryHandler = FuncPtr;
	InstancePtr->RecoveryCallBackRef = CallBackRef;
}


/****************************************************************************/
/**
*
* Set the user pre-reset handler, which can be used to save context before
* reset in a recovery sequence.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
*
* @note		This function is run in interrupt context.
*
******************************************************************************/
void XTMR_Manager_SetPreResetHandler(XTMR_Manager *InstancePtr,
				     XTMR_Manager_Handler FuncPtr,
				     void *CallBackRef)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->PreResetHandler = FuncPtr;
	InstancePtr->PreResetCallBackRef = CallBackRef;
}


/****************************************************************************/
/**
*
* Set the user post-reset handler, which can be used to restore context after
* reset in a recovery sequence.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
*
* @note		None.
*
******************************************************************************/
void XTMR_Manager_SetPostResetHandler(XTMR_Manager *InstancePtr,
				      XTMR_Manager_Handler FuncPtr,
				      void *CallBackRef)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->PostResetHandler = FuncPtr;
	InstancePtr->PostResetCallBackRef = CallBackRef;
}


/** @} */
