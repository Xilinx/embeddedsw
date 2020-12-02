/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xccm_intr.c
* @addtogroup ccm_v6_1
* @{
*
* This file contains interrupt related functions of Xilinx CCM core.
* Please see xccm.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- ------------------------------------------------------
* 6.0   adk     03/06/14 First release.
*                        Implemented the following functions:
*                        XCcm_IntrHandler and XCcm_SetCallBack.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xccm.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the CCM core.
*
* This handler reads the pending interrupt from Status register,
* determines the source of the interrupts and calls the respective
* callbacks for the interrupts that are enabled in IRQ_ENABLE register,
* and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XCcm_SetCallBack() during initialization phase. .
*
* @param	InstancePtr is a pointer to the XCcm instance that just
*		interrupted.
*
* @return	None.
*
* @note		Interrupt interface should be enabled.
*
******************************************************************************/
void XCcm_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 ErrorStatus;
	XCcm *XCcmPtr = NULL;
	XCcmPtr = (XCcm *)((void *)InstancePtr);

	/* Verify arguments. */
	Xil_AssertVoid(XCcmPtr != NULL);
	Xil_AssertVoid(XCcmPtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(XCcmPtr->Config.HasIntcIf != (u16)0x0);

	/* Get pending interrupts */
	PendingIntr = (u32)(XCcm_IntrGetPending(XCcmPtr));

	/* A Slave error interrupt has happened */
	if (((PendingIntr) & (XCCM_IXR_SE_MASK)) == (XCCM_IXR_SE_MASK)) {
		ErrorStatus = (PendingIntr) & (XCCM_IXR_SE_MASK);
		XCcmPtr->ErrCallBack(XCcmPtr->ErrRef, ErrorStatus);

	}

	/* A processing start has happened */
	if (((PendingIntr) & (XCCM_IXR_PROCS_STARTED_MASK)) ==
					(XCCM_IXR_PROCS_STARTED_MASK)) {
		XCcmPtr->ProcStartCallBack(XCcmPtr->ProcStartRef);
	}

	/* A frame done interrupt has happened */
	if (((PendingIntr) & (XCCM_IXR_EOF_MASK)) ==
					(XCCM_IXR_EOF_MASK)) {
		XCcmPtr->FrameDoneCallBack(XCcmPtr->FrameDoneRef);
	}

	/* Clear pending interrupt(s) */
	XCcm_IntrClear(XCcmPtr, PendingIntr);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType.
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  --------------------------------------------------
* XCCM_HANDLER_FRAMEDONE   FrameDoneCallBack
* XCCM_HANDLER_PROCSTART   ProcStartCallBack
* XCCM_HANDLER_ERROR       ErrCallBack
*
* </pre>
*
* @param	InstancePtr is a pointer to the Xccm instance to be worked on.
* @param	HandlerType specifies which callback is to be attached.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
* 		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
int XCcm_SetCallBack(XCcm *InstancePtr, u32 HandlerType,
	void *CallBackFunc, void *CallBackRef)
{
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);
	Xil_AssertNonvoid((HandlerType >= XCCM_HANDLER_PROCSTART) &&
				(HandlerType <= XCCM_HANDLER_ERROR));
	Xil_AssertNonvoid(InstancePtr->IsReady ==
				(u32)(XIL_COMPONENT_IS_READY));

	/*
	 * Calls the respective callback function corresponding to
	 * the handler type
	 */
	switch (HandlerType) {
		case XCCM_HANDLER_PROCSTART:
			InstancePtr->ProcStartCallBack =
					(XCcm_CallBack)((void *)CallBackFunc);
			InstancePtr->ProcStartRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XCCM_HANDLER_FRAMEDONE:
			InstancePtr->FrameDoneCallBack =
					(XCcm_CallBack)((void *)CallBackFunc);
			InstancePtr->FrameDoneRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XCCM_HANDLER_ERROR:
			InstancePtr->ErrCallBack =
				(XCcm_ErrorCallBack)((void *)CallBackFunc);
			InstancePtr->ErrRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		default:
			Status = (XST_INVALID_PARAM);
			break;

	}

	return Status;
}
/** @} */
