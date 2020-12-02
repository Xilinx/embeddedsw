/******************************************************************************
* Copyright (c) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdeint_intr.c
* @addtogroup deinterlacer_v3_4
* @{
*
* This code contains interrupt related functions of Xilinx Video
* Deinterlacer (DEINT) core. Please see xdeint.h for more details of the core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who  Date     Changes
* ----- ----- -------- -------------------------------------------------------
* 1.00a rjh   07/10/11 First release.
* 2.00a rjh   18/01/12 Updated for v_deinterlacer 2.00.
* 3.2   adk   02/13/14 Adherence to Xilinx coding, Doxygen guidelines.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdeint.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the Deinterlacer core.
*
* This handler reads the pending interrupt from the IER/ISR, determines the
* source of the interrupts, calls according callback, and finally clears the
* interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XDeint_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	None.
*
* @note		The Error interrupt callback invoked in case an error interrupt
*		or spurious interrupt happens should reset the Deinterlacer
*		core that just interrupted.
*
******************************************************************************/
void XDeint_IntrHandler(void *InstancePtr)
{
	XDeint *XDeintPtr;
	u32 PendingIntr;

	/* Verify arguments. */
	XDeintPtr = (XDeint *)((void *)InstancePtr);
	Xil_AssertVoid(XDeintPtr != NULL);
	Xil_AssertVoid(XDeintPtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));

	/* Get pending interrupts. */
	PendingIntr = XDeint_IntrGetPending(XDeintPtr);

	/* A known interrupt has happened. */
	if (PendingIntr != 0) {
		XDeintPtr->IntCallBack(PendingIntr);
	}

	/* Clear pending interrupt(s). */
	XDeint_IntrClear(XDeintPtr, PendingIntr);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	CallBackFunc is the address of the callback function.
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
int XDeint_SetCallBack(XDeint *InstancePtr, void *CallBackFunc)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady ==
					(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertNonvoid(CallBackFunc != NULL);

	InstancePtr->IntCallBack = (XDeint_CallBack)((void *)CallBackFunc);

	return (XST_SUCCESS);
}
/** @} */
