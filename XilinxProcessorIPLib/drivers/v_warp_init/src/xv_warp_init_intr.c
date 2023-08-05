/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xv_warp_init_intr.c
 * @addtogroup v_warp_init Overview
 * @{
 *
 * The functions in this file provides interrupt handler and associated
 * functions that are consumed by layer-2
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xv_warp_init.h"
#include "xv_warp_init_hw.h"

/*****************************************************************************/
/**
 **
 ** This function installs an asynchronous callback function:
 **
 ** @param    InstancePtr is a pointer to the v_warp_init IP instance.
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
void XVWarpInit_SetCallback(XV_warp_init *InstancePtr,
		void *CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(CallbackFunc);
	Xil_AssertVoid(CallbackRef);

	InstancePtr->FrameDoneCallback = (XV_warp_init_Callback)CallbackFunc;
	InstancePtr->CallbackRef = CallbackRef;
}

/*****************************************************************************/
/**
 **
 ** This function is the interrupt handler for the GenerateRemapVector core
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
void XVWarpInit_IntrHandler(void *InstancePtr)
{
	XV_warp_init *GenRemapVectPtr = (XV_warp_init *) InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(GenRemapVectPtr);
	Xil_AssertVoid(GenRemapVectPtr->IsReady);

	if (XV_warp_init_InterruptGetStatus(GenRemapVectPtr)) {
		if (XV_warp_init_IsDone(InstancePtr)) {
			XV_warp_init_InterruptClear(GenRemapVectPtr,
					XV_WARP_INIT_INTR_AP_DONE_MASK);
			if (GenRemapVectPtr->FrameDoneCallback)
				GenRemapVectPtr->FrameDoneCallback(GenRemapVectPtr->CallbackRef);
		}
	}
}
/** @} */
