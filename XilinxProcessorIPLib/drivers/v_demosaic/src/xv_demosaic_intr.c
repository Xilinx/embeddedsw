/******************************************************************************
 * Copyright (C) 2018 - 2021 Xilinx, Inc.	All rights reserved.
 * Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 *
 * @file xv_demosaic_intr.c
 * @addtogroup v_demosaic Overview
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
 * @brief Installs an asynchronous callback function for the given handler type.
 *
 * This function registers callback functions for different interrupt events.
 * Supported handler types include frame done (XVDEMOSAIC_HANDLER_DONE) and
 * frame ready (XVDEMOSAIC_HANDLER_READY) events.
 *
 * @param InstancePtr Pointer to the Demosaic IP instance.
 * @param HandlerType Type of handler (XVDEMOSAIC_HANDLER_DONE or XVDEMOSAIC_HANDLER_READY).
 * @param CallbackFunc Address of the callback function to be invoked.
 * @param CallbackRef User data item passed to the callback function when invoked.
 *
 * @return None
 *
 * @note Invoking this function for a handler that already has been installed
 *       replaces it with the new handler.
 *
 *******************************************************************************/
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
 * @brief Interrupt handler for the Demosaic IP core.
 *
 * This handler clears pending interrupts and determines if the source is a
 * frame done or frame ready signal. If the interrupt is valid, it calls the
 * registered callback function for the corresponding event.
 *
 * @param InstancePtr Pointer to the Demosaic IP instance that just interrupted.
 *
 * @return None
 *
 * @note The application is responsible for connecting this function to the
 *       interrupt system and providing callbacks using XVDemosaic_SetCallback()
 *       during initialization phase.
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
