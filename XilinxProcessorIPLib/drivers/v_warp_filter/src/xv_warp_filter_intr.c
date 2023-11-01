/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xv_warp_filter_intr.c
 * @addtogroup v_warp_filter Overview
 * @{
 *
 * The functions in this file provides interrupt handler and associated
 * functions that are consumed by layer-2
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xv_warp_filter_hw.h"
#include "xv_warp_filter.h"

/*****************************************************************************/
/**
 **
 ** This function installs an asynchronous callback function:
 **
 ** @param    InstancePtr is a pointer to the v_warp_filter IP instance.
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
void XVWarpFilter_SetCallback(XV_warp_filter *InstancePtr,
		void *CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->FrameDoneCallback = (XV_warp_filter_Callback)CallbackFunc;
	InstancePtr->CallbackRef = CallbackRef;
}

/*****************************************************************************/
/**
 **
 ** This function is the interrupt handler for the Warp core
 ** driver.
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
void XVWarpFilter_IntrHandler(void *InstancePtr)
{
	XV_warp_filter *WarpPtr = (XV_warp_filter *) InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(WarpPtr);
	Xil_AssertVoid(WarpPtr->IsReady);

	if (XV_warp_filter_InterruptGetStatus(WarpPtr)) {
		XV_warp_filter_InterruptClear(WarpPtr, XV_WARP_FILTER_INTR_AP_DONE_MASK);
		if (WarpPtr->FrameDoneCallback)
			WarpPtr->FrameDoneCallback(WarpPtr->CallbackRef);
	}
}
/** @} */
