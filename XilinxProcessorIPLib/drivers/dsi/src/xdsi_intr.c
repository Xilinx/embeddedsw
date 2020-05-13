/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi_intr.c
* @addtogroup dsi_v1_2
* @{
*
* This file implements the functions which handle the interrupts in the DSI
* TX Controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0 ram 02/11/16 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdsi_hw.h"
#include "xdsi.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/************************** Macros Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *******************************/

/*****************************************************************************/
/**
* This function will enable the interrupts present in the interrupt mask
* passed onto the function
*
* @param	InstancePtr is the XDsi instance to operate on
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XDsi_InterruptEnable(XDsi *InstancePtr, u32 Mask)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XDSI_IER_ALLINTR_MASK))) == 0);

	Mask |= XDsi_GetIntrEnableStatus(InstancePtr);

	XDsi_IntrEnable(InstancePtr, Mask);
}

/*****************************************************************************/
/**
* This function will disable the interrupts present in the interrupt mask
* passed onto the function
*
* @param	InstancePtr is the XDsi instance to operate on
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XDsi_InterruptDisable(XDsi *InstancePtr, u32 Mask)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XDSI_IER_ALLINTR_MASK))) == 0);

	XDsi_IntrDisable(InstancePtr,
		(Mask & (XDsi_GetIntrEnableStatus(InstancePtr))));
}

/*****************************************************************************/
/**
* This function will get the interrupt mask set (enabled) in the DSI core
*
* @param	InstancePtr is the XDsi instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
* 		Interrupt enable register
*
* @note		None
*
****************************************************************************/
u32 XDsi_InterruptGetEnabled(XDsi *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XDsi_GetIntrEnableStatus(InstancePtr);

	return Mask;
}

/*****************************************************************************/
/**
* This function will get the list of interrupts Invoked in the Interrupt
* Status Register of the DSI core
*
* @param	InstancePtr is the XDsi instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
* 		Interrupt Status register
*
* @note		None
*
****************************************************************************/
u32 XDsi_InterruptGetStatus(XDsi *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XDsi_GetIntrStatus(InstancePtr);

	return Mask;
}

/*****************************************************************************/
/**
* This function will clear the interrupts set in the Interrupt Status
* Register of the DSI core
*
* @param	InstancePtr is the XDsi instance to operate on
* @param	Mask is Interrupt Mask with bits set for corresponding interrupt
* 		to be cleared in the Interrupt Status register
*
* @return 	None
*
* @note		None
*
****************************************************************************/
void XDsi_InterruptClear(XDsi *InstancePtr, u32 Mask)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XDSI_IER_ALLINTR_MASK))) == 0);

	Mask &= XDsi_GetIntrStatus(InstancePtr);

	XDsi_IntrClear(InstancePtr, Mask);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
*
* HandlerType			Invoked by this driver when:
* -----------------------  --------------------------------------------------
* XDSI_HANDLER_UNSUPPORT_DATATYPE Un support data type detected
* XDSI_HANDLER_PIXELDATA_UNDERRUN Byte stream FIFO starves for Pixel during
* 				  HACT transmission
* XDSI_HANDLER_OTHERERROR  Any other type of interrupt has occured like
* 			   Stream Line Buffer Full, Incorrect Lanes, etc
* XDSI_HANDLER_CMDQ_FIFOFULL Command queue FIFO full
*
* </pre>
*
* @param	InstancePtr is the XDsi instance to operate on
* @param 	HandleType is the type of call back to be registered.
* @param	CallbackFunc is the pointer to a call back funtion which
* 		is called when a particular event occurs.
* @param 	CallbackRef is a void pointer to data to be referenced to
* 		by the CallbackFunc
*
* @return
* 		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note 	Invoking this function for a handler that already has been
* 		installed replaces it with the new handler.
*
****************************************************************************/
s32 XDsi_SetCallback(XDsi *InstancePtr, u32 HandleType,
		void *CallbackFunc, void *CallbackRef)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	switch (HandleType) {
	case XDSI_HANDLER_UNSUPPORT_DATATYPE:
		InstancePtr->UnSupportedDataTypeCallback = CallbackFunc;
		InstancePtr->UnsupportDataTypeRef = CallbackRef;
		break;

	case XDSI_HANDLER_PIXELDATA_UNDERRUN:
		InstancePtr->PixelDataUnderrunCallback = CallbackFunc;
		InstancePtr->PixelDataUnderrundRef = CallbackRef;
		break;

	case XDSI_HANDLER_CMDQ_FIFOFULL:
		InstancePtr->CmdQFIFOFullCallback = CallbackFunc;
		InstancePtr->CmdQFIFOFullRef = CallbackRef;
		break;

	case XDSI_HANDLER_OTHERERROR:
		InstancePtr->ErrorCallback = CallbackFunc;
		InstancePtr->ErrRef = CallbackRef;
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
* This function is the interrupt handler for the DSI core.
*
* This handler reads the Invoked interrupt from the Interrupt Status register
* determines the source of the interrupts and calls the respective
* callbacks for the interrupts that are enabled in Interrupt Enable register
* and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this core is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XDsi_SetCallback() during initialization phase.
*
* @param	InstancePtr is a pointer to the XDsi core instance.
*
* @return	None.
*
* @note		Interrupt should be enabled to execute interrupt handler.
*
******************************************************************************/
void XDsi_IntrHandler(void *InstancePtr)
{
	u32 InvokedIntr;
	u32 Mask;

	XDsi *XDsiPtr = (XDsi *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDsiPtr != NULL);
	Xil_AssertVoid(XDsiPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get Invoked interrupts in */
	InvokedIntr = XDsi_InterruptGetStatus(XDsiPtr);

	Mask = InvokedIntr & XDSI_IER_DATA_ID_ERR_MASK;
	if (Mask) {
		/* If UnSupportedDataType then call corresponding callback function */
		XDsiPtr->UnSupportedDataTypeCallback
			(XDsiPtr->UnsupportDataTypeRef, Mask);
	}

	Mask = InvokedIntr & XDSI_IER_PXL_UNDR_RUN_MASK;
	if (Mask) {
		/* If ShortPacket Interrupts then call corresponding
		 * callback function */
		XDsiPtr->PixelDataUnderrunCallback
			(XDsiPtr->PixelDataUnderrundRef, Mask);
	}

	Mask = InvokedIntr & XDSI_IER_CMDQ_FIFO_FULL_MASK;
	if (Mask) {
		/* If Command queue FIFO full Interrupts then call corresponding
		 * callback function */
		XDsiPtr->CmdQFIFOFullCallback
			(XDsiPtr->CmdQFIFOFullRef, Mask);
	}

	/* Clear Invoked interrupt(s) */
	XDsi_InterruptClear(XDsiPtr, InvokedIntr);
}
/** @} */
