/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xclk_wiz_intr.c
* @addtogroup Overview
* @{
*
* This file implements the functions which handle the interrupts in the CLK_WIZ
* Controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0 ram 02/12/16 Initial version for Clock Wizard
* 1.2 ms  03/02/17 Fixed compilation errors. Fix for CR-970507.
* 1.6 sd  07/07/23 Added SDT support.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xclk_wiz_hw.h"
#include "xclk_wiz.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/************************** Macros Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* XClk_Wiz_InterruptEnable will enable the interrupts present in the interrupt
* mask passed onto the function
*
* @param	InstancePtr is the XClk_Wiz instance to operate on
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XClk_Wiz_InterruptEnable(XClk_Wiz *InstancePtr, u32 Mask)
{

	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XCLK_WIZ_IER_ALLINTR_MASK))) == 0);

	Mask |= XClk_Wiz_GetIntrEnable(InstancePtr);

	XClk_Wiz_IntrEnable(InstancePtr, Mask);
}

/*****************************************************************************/
/**
* XClk_Wiz_InterruptDisable will disable the interrupts present in the
* interrupt mask passed onto the function
*
* @param	InstancePtr is the XClk_Wiz instance to operate on
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XClk_Wiz_InterruptDisable(XClk_Wiz *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XCLK_WIZ_IER_ALLINTR_MASK))) == 0);

	XClk_Wiz_IntrDisable(InstancePtr, \
			     (Mask & (XClk_Wiz_GetIntrEnable(InstancePtr))));
}

/*****************************************************************************/
/**
* XClk_Wiz_InterruptGetEnabled will get the interrupt mask set (enabled) in the
* CLK_WIZ core
*
* @param	InstancePtr is the XClk_Wiz instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
*		Interrupt enable register
*
* @note		None
*
****************************************************************************/
u32 XClk_Wiz_InterruptGetEnabled(XClk_Wiz *InstancePtr)
{
	u32 Mask;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XClk_Wiz_GetIntrEnable(InstancePtr);

	return Mask;
}

/*****************************************************************************/
/**
* XClk_Wiz_InterruptGetStatus will get the list of interrupts pending in the
* Interrupt Status Register of the CLK_WIZ core
*
* @param	InstancePtr is the XClk_Wiz instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
*		Interrupt Status register
*
* @note		None
*
****************************************************************************/
u32 XClk_Wiz_InterruptGetStatus(XClk_Wiz *InstancePtr)
{
	u32 Mask;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XClk_Wiz_IntrGetIrq(InstancePtr);

	return Mask;
}

/*****************************************************************************/
/**
* XClk_Wiz_InterruptClear will clear the interrupts set in the Interrupt Status
* Register of the CLK_WIZ core
*
* @param	InstancePtr is the XClk_Wiz instance to operate on
* @param	Mask is Interrupt Mask with bits set for corresponding interrupt
*		to be cleared in the Interrupt Status register
*
* @return 	None
*
* @note		None
*
****************************************************************************/
void XClk_Wiz_InterruptClear(XClk_Wiz *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XCLK_WIZ_IER_ALLINTR_MASK))) == 0);

	Mask &= XClk_Wiz_IntrGetIrq(InstancePtr);

	XClk_Wiz_IntrAckIrq(InstancePtr, Mask);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
* <pre>
*
* HandlerType				Invoked by this driver when:
* -----------------------  --------------------------------------------------
* XCLK_WIZ_HANDLER_CLK_OUTOF_RANGE	Clock under flow/over flow
* XCLK_WIZ_HANDLER_CLK_GLITCH		Clock Glitch
* XCLK_WIZ_HANDLER_CLK_STOP		Clock Stop
* XCLK_WIZ_HANDLER_OTHERERROR		Any other type of interrupts
* </pre>
*
* @param	InstancePtr is the XClk_Wiz instance to operate on
*
* @param 	HandleType is the type of call back to be registered.
*
* @param	CallBackFunc is the pointer to a call back funtion which
* 		is called when a particular event occurs.
*
* @param 	CallBackRef is a void pointer to data to be referenced to
* 		by the CallBackFunc
*
* @return
* 		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note 	Invoking this function for a handler that already has been
* 		installed replaces it with the new handler.
*
****************************************************************************/
int XClk_Wiz_SetCallBack(XClk_Wiz *InstancePtr, u32 HandleType,
			 void *CallBackFunc, void *CallBackRef)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	switch (HandleType) {
		case XCLK_WIZ_HANDLER_CLK_OUTOF_RANGE:
			InstancePtr->ClkOutOfRangeCallBack = (XClk_Wiz_CallBack)CallBackFunc;
			InstancePtr->ClkOutOfRangeRef = CallBackRef;
			break;
		case XCLK_WIZ_HANDLER_CLK_GLITCH:
			InstancePtr->ClkGlitchCallBack = (XClk_Wiz_CallBack)CallBackFunc;
			InstancePtr->ClkGlitchRef = CallBackRef;
			break;
		case XCLK_WIZ_HANDLER_CLK_STOP:
			InstancePtr->ClkStopCallBack = (XClk_Wiz_CallBack)CallBackFunc;
			InstancePtr->ClkStopRef = CallBackRef;
			break;
		case XCLK_WIZ_HANDLER_CLK_OTHER_ERROR:
			InstancePtr->ErrorCallBack = (XClk_Wiz_CallBack)CallBackFunc;
			InstancePtr->ErrRef = CallBackRef;
			break;
		default:
			/* Invalid value of HandleType */
			return XST_INVALID_PARAM;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the CLK_WIZ core.
*
* This handler reads the pending interrupt from the Interrupt Status register
* determines the source of the interrupts and calls the respective
* callbacks for the interrupts that are enabled in Interrupt Enable register
* and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this core is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XClk_Wiz_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XClk_Wiz core instance.
*
* @return	None
*
* @note		Interrupt should be enabled to execute interrupt handler.
*
******************************************************************************/
void XClk_Wiz_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 Mask;

	XClk_Wiz *XClk_WizPtr = (XClk_Wiz *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XClk_WizPtr != NULL);
	Xil_AssertVoid(XClk_WizPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get pending interrupts */
	PendingIntr = XClk_Wiz_InterruptGetStatus(XClk_WizPtr);

	Mask = PendingIntr & XCLK_WIZ_ISR_CLKALL_MAXFREQ_MASK;
	if (Mask) {
		/* If Clock out of max range then call corresponding
		 * callback function
		 */
		XClk_WizPtr->ClkOutOfRangeCallBack\
		(XClk_WizPtr->ClkOutOfRangeRef, Mask);
	}

	Mask = PendingIntr & XCLK_WIZ_ISR_CLKALL_MINFREQ_MASK;
	if (Mask) {
		/* If Clock out of min range then call corresponding
		 * callback function
		 */
		XClk_WizPtr->ClkOutOfRangeCallBack\
		(XClk_WizPtr->ClkOutOfRangeRef, Mask);
	}

	Mask = PendingIntr & XCLK_WIZ_ISR_CLKALL_GLITCH_MASK;
	if (Mask) {
		/* If clock glitch then call corresponding
		 * callback function */
		XClk_WizPtr->ClkGlitchCallBack\
		(XClk_WizPtr->ClkGlitchRef, Mask);
	}
	Mask = PendingIntr & XCLK_WIZ_ISR_CLKALL_STOP_MASK;
	if (Mask) {
		/* If clock stops then call corresponding
		 * callback function */
		XClk_WizPtr->ClkStopCallBack\
		(XClk_WizPtr->ClkStopRef, Mask);
	}

	/* Clear pending interrupt(s) */
	XClk_Wiz_InterruptClear(XClk_WizPtr, PendingIntr);
}
/** @} */
