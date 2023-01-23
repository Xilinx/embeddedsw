/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xzdma_intr.c
* @addtogroup zdma_v1_14
* @{
*
* This file contains interrupt related functions of Xilinx ZDMA core.
* Please see xzdma.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   vns     2/27/15  First release
* 1.6   aru     08/18/18 Resolved MISRA-C mandatory violations.(CR#1007757)
* 1.8   aru    07/02/19  Fix coverity warnings.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xzdma.h"

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This function is the interrupt handler for the ZDMA core.
*
* This handler reads the pending interrupt from Status register, determines the
* source of the interrupts and calls the respective callbacks for the
* interrupts that are enabled in IRQ_ENABLE register, and finally clears the
* interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XZDma_SetCallBack() during initialization phase. .
*
* @param	Instance is a pointer to the XZDma instance to be worked on.
*
* @return	None.
*
* @note		To generate interrupt required interrupts should be enabled.
*
******************************************************************************/
void XZDma_IntrHandler(void *Instance)
{
	u32 PendingIntr;
	u32 ErrorStatus;
	XZDma *InstancePtr = (XZDma *)((void *)Instance);

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Get pending interrupts */
	PendingIntr = (u32)(XZDma_IntrGetStatus(InstancePtr));
	PendingIntr &= (~XZDma_GetIntrMask(InstancePtr));

	/* ZDMA transfer has completed */
	ErrorStatus = (PendingIntr) & (XZDMA_IXR_DMA_DONE_MASK);
	if ((ErrorStatus) != 0U) {
		XZDma_DisableIntr(InstancePtr, XZDMA_IXR_ALL_INTR_MASK);
		InstancePtr->ChannelState = XZDMA_IDLE;
		InstancePtr->DoneHandler(InstancePtr->DoneRef);
	}

	/* An error has been occurred */
	ErrorStatus = PendingIntr & (XZDMA_IXR_ERR_MASK);
	if ((ErrorStatus) != 0U) {
		if ((ErrorStatus & XZDMA_IXR_DMA_PAUSE_MASK) ==
				XZDMA_IXR_DMA_PAUSE_MASK) {
			InstancePtr->ChannelState = XZDMA_PAUSE;
		}
		else {
			if ((ErrorStatus & (XZDMA_IXR_AXI_WR_DATA_MASK |
				XZDMA_IXR_AXI_RD_DATA_MASK |
				XZDMA_IXR_AXI_RD_DST_DSCR_MASK |
				XZDMA_IXR_AXI_RD_SRC_DSCR_MASK)) != 0x00U) {
				InstancePtr->ChannelState = XZDMA_IDLE;
			}
		}
		InstancePtr->ErrorHandler(InstancePtr->ErrorRef, ErrorStatus);
	}

	/* Clear pending interrupt(s) */
	XZDma_IntrClear(InstancePtr, PendingIntr);
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
* XZDMA_HANDLER_DONE	   Done handler
* XZDMA_HANDLER_ERROR	   Error handler
*
* </pre>
*
* @param	InstancePtr is a pointer to the XZDma instance to be worked on.
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
s32 XZDma_SetCallBack(XZDma *InstancePtr, XZDma_Handler HandlerType,
	void *CallBackFunc, void *CallBackRef)
{

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);
	Xil_AssertNonvoid((HandlerType == XZDMA_HANDLER_DONE) ||
				(HandlerType == XZDMA_HANDLER_ERROR));
	Xil_AssertNonvoid(InstancePtr->IsReady ==
				(u32)(XIL_COMPONENT_IS_READY));

	/*
	 * Calls the respective callback function corresponding to
	 * the handler type
	 */
	if(HandlerType == XZDMA_HANDLER_DONE) {
		InstancePtr->DoneHandler =
				(XZDma_DoneHandler)((void *)CallBackFunc);
				InstancePtr->DoneRef = CallBackRef;
	} else {
		InstancePtr->ErrorHandler =
				(XZDma_ErrorHandler)((void *)CallBackFunc);
				InstancePtr->ErrorRef = CallBackRef;
	}

	return XST_SUCCESS;
}
/** @} */
