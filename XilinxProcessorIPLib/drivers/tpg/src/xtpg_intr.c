/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpg_intr.c
* @addtogroup tpg Overview
* @{
*
* This file contains interrupt related functions of the TPG core.
* Please see xtpg.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date    Changes
* ----- ----- -------  --------------------------------------------------
* 3.0   adk   02/19/14  First Release
*                       Implemented the following functions:
*                       XTpg_IntrHandler and XTpg_SetCallBack.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtpg.h"

/************************** Constant Definitions *****************************/


/***************** Marcos (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the TPG driver.
*
* This handler reads the pending interrupt from the STATUS register,
* determines the source of the interrupts and calls the respective
* callbacks for the interrupts that are enabled in IRQ_ENABLE register,
* and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to	handle interrupts and installing the callbacks using
* XTpg_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XTpg instance that
*		just interrupted.
*
* @return	None.
*
* @note		Interrupt interface should be enabled.
*
******************************************************************************/
void XTpg_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 ErrorStatus;
	XTpg *XTpgPtr = NULL;
	XTpgPtr = (XTpg *) ((void *)InstancePtr);

	/* Verify Arguments. */
	Xil_AssertVoid(XTpgPtr != NULL);
	Xil_AssertVoid(XTpgPtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(XTpgPtr->Config.HasIntcIf != (u16)0x0);

	/* Get pending interrupts */
	PendingIntr = (u32)XTpg_IntrGetPending(XTpgPtr);

	/* A Slave error interrupt has been happened */
	if (((PendingIntr) & (XTPG_IXR_SE_MASK)) ==
			(XTPG_IXR_SE_MASK)) {
		ErrorStatus = (PendingIntr) & ((u32)(XTPG_IXR_SE_MASK));
		XTpgPtr->ErrCallBack(XTpgPtr->ErrRef, ErrorStatus);
	}

	/* A Processing Start interrupt has occurred */
	if (((PendingIntr) & (XTPG_IXR_PROCS_STARTED_MASK)) ==
					(XTPG_IXR_PROCS_STARTED_MASK)) {
		XTpgPtr->ProcStartCallBack(XTpgPtr->ProcStartRef);
	}

	/* A Frame Done interrupt has occurred */
	if (((PendingIntr) & (XTPG_IXR_EOF_MASK)) == (XTPG_IXR_EOF_MASK)) {
		XTpgPtr->FrameDoneCallBack(XTpgPtr->FrameDoneRef);
	}

	/* Clear pending interrupts */
	XTpg_IntrClear(XTpgPtr, PendingIntr);

}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType.
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  ----------------------
* XTPG_HANDLER_FRAMEDONE   FrameDoneCallBack
* XTPG_HANDLER_PROCSTART   ProcStartCallBack
* XTPG_HANDLER_ERROR       ErrCallBack
*
* </pre>
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
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
* 		installed replaces it with the new handler.
*
******************************************************************************/
int XTpg_SetCallBack(XTpg *InstancePtr, u32 HandlerType,
			void *CallBackFunc, void *CallBackRef)
{
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady ==
				(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);
	Xil_AssertNonvoid((HandlerType >= XTPG_HANDLER_PROCSTART) &&
				(HandlerType <= XTPG_HANDLER_ERROR));

	/* Setting the handlerType */
	switch (HandlerType) {
		case XTPG_HANDLER_PROCSTART:
			InstancePtr->ProcStartCallBack =
					(XTpg_CallBack)((void *)CallBackFunc);
			InstancePtr->ProcStartRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XTPG_HANDLER_FRAMEDONE:
			InstancePtr->FrameDoneCallBack =
					(XTpg_CallBack)((void *)CallBackFunc);
			InstancePtr->FrameDoneRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XTPG_HANDLER_ERROR:
			InstancePtr->ErrCallBack =
				(XTpg_ErrorCallBack)((void *)CallBackFunc);
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
