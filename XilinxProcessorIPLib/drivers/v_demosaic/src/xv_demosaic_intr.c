/******************************************************************************
 * Copyright (C) 2018 - 2021 Xilinx, Inc.	All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xv_demosaic_intr.c
 * @addtogroup v_demosaic_v1_3
 * @{
 *
 * The functions in this file provides interrupt handler and associated
 * functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.30  pg   05/07/21   Added Interrupt handler functions.
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xv_demosaic_hw.h"
#include "xv_demosaic.h"

/*****************************************************************************/
/**
 *
 * This function installs an asynchronous callback function for the given
 * HandlerType:
 *
 * <pre>
 * HandlerType                     Callback Function Type
 * -----------------------         --------------------------------------------------
 * (XVDEMOSAIC_HANDLER_DONE)       DoneCallback
 * (XVDEMOSAIC_HANDLER_READY)      ReadyCallback
 *
 * @param    InstancePtr is a pointer to the Demosaic IP instance.
 * @param    CallbackFunc is the address of the callback function.
 * @param    CallbackRef is a user data item that will be passed to the
 *       callback function when it is invoked.
 *
 * @return	None.
 *
 * @note     Invoking this function for a handler that already has been
 *       installed replaces it with the new handler.
 *
 ******************************************************************************/
void XVDemosaic_SetCallback(XV_demosaic *InstancePtr, u32 HandlerType,
		void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType >= (XVDEMOSAIC_HANDLER_DONE));
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {
		case (XVDEMOSAIC_HANDLER_DONE):
			InstancePtr->FrameDoneCallback = (XVDemosaic_Callback)CallbackFunc;
			InstancePtr->CallbackDoneRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;
		case (XVDEMOSAIC_HANDLER_READY):
			InstancePtr->FrameReadyCallback = (XVDemosaic_Callback)CallbackFunc;
			InstancePtr->CallbackReadyRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;
		default:
			Status = (XST_INVALID_PARAM);
			break;
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the Sensor Demosaic core driver.
*
* This handler clears the pending interrupt and determined if the source is
* frame done signal. If yes, starts the next frame processing and calls the
* registered callback function
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XVDemosaic_SetCallback() during initialization phase.
*
* @param    InstancePtr is a pointer to the core instance that just
*           interrupted.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XVDemosaic_InterruptHandler(XV_demosaic *InstancePtr)
{
	u32 Status;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get the interrupt source */
	Status = XV_demosaic_InterruptGetStatus(InstancePtr);

	/* Check for Done Signal */
	if(Status & XVDEMOSAIC_IRQ_DONE_MASK) {
		/* Clear the interrupt */
		XV_demosaic_InterruptClear(InstancePtr, XVDEMOSAIC_IRQ_DONE_MASK);
		//Call user registered callback function, if any
		if(InstancePtr->FrameDoneCallback) {
			InstancePtr->FrameDoneCallback(InstancePtr->CallbackDoneRef);
		}
	}

	/* Check for Ready Signal */
	if(Status & XVDEMOSAIC_IRQ_READY_MASK) {
		/* Clear the interrupt */
		XV_demosaic_InterruptClear(InstancePtr, XVDEMOSAIC_IRQ_READY_MASK);
		//Call user registered callback function, if any
		if(InstancePtr->FrameReadyCallback) {
			InstancePtr->FrameReadyCallback(InstancePtr->CallbackReadyRef);
		}
	}
}
/** @} */
