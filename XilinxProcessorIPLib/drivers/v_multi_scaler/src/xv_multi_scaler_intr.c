/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xv_multi_scaler_intr.c
 * @addtogroup v_multi_scaler Overview
 * @{
 *
 * The functions in this file provides interrupt handler and associated
 * functions that are consumed by layer-2
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xv_multi_scaler_hw.h"
#include "xv_multi_scaler.h"

/*****************************************************************************/
/**
 **
 ** This function installs an asynchronous callback function:
 **
 ** @param    InstancePtr is a pointer to the MultiScaler IP instance.
 ** @param    CallbackFunc is the address of the callback function.
 ** @param    CallbackRef is a user data item that will be passed to the
 **       callback function when it is invoked.
 **
 ** @return      None.
 **
 ** @note     Invoking this function for a handler that already has been
 **       installed replaces it with the new handler.
 **
 ******************************************************************************/
void XVMultiScaler_SetCallback(XV_multi_scaler *InstancePtr, void *CallbackFunc,
	void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->FrameDoneCallback = (XVMultiScaler_Callback)CallbackFunc;
	InstancePtr->CallbackRef = CallbackRef;
}

/*****************************************************************************/
/**
 **
 ** This function is the interrupt handler for the MultiScaler core driver.
 **
 ** This handler clears the pending interrupt and determines if the source is
 ** frame done signal. If yes, calls the registered callback function.
 **
 ** The application is responsible for connecting this function to the interrupt
 ** system.
 **
 ** @param    InstancePtr is a pointer to the core instance that just
 **           interrupted.
 **
 ** @return   None.
 **
 ** @note     None.
 **
 *****************************************************************************/
void *XV_MultiScalerIntrHandler(void *InstancePtr)
{
	XV_multi_scaler *MscPtr = (XV_multi_scaler *) InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(MscPtr != NULL);
	Xil_AssertVoid(MscPtr->IsReady == XIL_COMPONENT_IS_READY);

	if (XV_multi_scaler_InterruptGetStatus(MscPtr)) {
		XV_multi_scaler_InterruptClear(MscPtr,
			XV_MULTI_SCALER_ISR_DONE_BIT_MASK |
			XV_MULTI_SCALER_ISR_READY_BIT_MASK);
		if (MscPtr->FrameDoneCallback)
			MscPtr->FrameDoneCallback(MscPtr);
	}
}
/** @} */
