/******************************************************************************
* Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcan_intr.c
* @addtogroup can Overview
* @{
*
* This file contains functions related to CAN interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a xd   04/12/05  First release
* 1.10a mta  05/13/07 Updated to new coding style
* 2.00a ktn  10/22/09 Updated to use the HAL APIs/macros.
*		      The macros have been renamed to remove _m from the name.
* 3.8   ht   12/13/23 Modify XCan_InterruptEnable to enable the interrupts.
* 3.8	ht   12/13/23 Added support for ECC.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcan.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/


/************************** Function Prototypes ******************************/


/****************************************************************************/
/**
*
* This routine enables interrupt(s). Use the XCAN_IXR_* constants defined in
* xcan_l.h to create the bit-mask to enable interrupts.
*
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	Mask is the mask to enable. Bit positions of 1 will be enabled.
*		Bit positions of 0 will keep the previous setting. This mask is
*		formed by OR'ing XCAN_IXR_* bits defined in xcan_l.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XCan_InterruptEnable(XCan *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Write to IER to enable interrupts */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_IER_OFFSET, Mask);
}

/****************************************************************************/
/**
*
* This routine disables interrupt(s). Use the XCAN_IXR_* constants defined in
* xcan_l.h to create the bit-mask to disable interrupt(s).
*
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	Mask is the mask to disable. Bit positions of 1 will be
*		disabled. Bit positions of 0 will keep the previous setting.
*		This mask is formed by OR'ing XCAN_IXR_* bits defined in
*		xcan_l.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XCan_InterruptDisable(XCan *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read currently enabled interrupts. */
	IntrValue = XCan_InterruptGetEnabled(InstancePtr);

	/* Calculate the new interrupts that should be kept enabled */
	IntrValue &= ~Mask;

	/* Write to IER to enable interrupts */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_IER_OFFSET, IntrValue);
}

/****************************************************************************/
/**
*
* This routine returns enabled interrupt(s). Use the XCAN_IXR_* constants
* defined in xcan_l.h to interpret the returned value.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return	Enabled interrupt(s) in a 32-bit format.
*
* @note		None.
*
*****************************************************************************/
u32 XCan_InterruptGetEnabled(XCan *InstancePtr)
{
	u32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCan_ReadReg(InstancePtr->BaseAddress, XCAN_IER_OFFSET);

	return Result;
}


/****************************************************************************/
/**
*
* This routine returns interrupt status read from Interrupt Status Register.
* Use the XCAN_IXR_* constants defined in xcan_l.h to interpret the returned
* value.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return	The value stored in Interrupt Status Register.
*
* @note		None.
*
*****************************************************************************/
u32 XCan_InterruptGetStatus(XCan *InstancePtr)
{
	u32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCan_ReadReg(InstancePtr->BaseAddress, XCAN_ISR_OFFSET);

	return Result;
}

/****************************************************************************/
/**
*
* This function clears interrupt(s). Every bit set in Interrupt Status
* Register indicates that a specific type of interrupt is occurring, and this
* function clears one or more interrupts by writing a bit mask to Interrupt
* Clear Register.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	Mask is the mask to clear. Bit positions of 1 will be cleared.
*		Bit positions of 0 will not change the previous interrupt
*		status. This mask is formed by OR'ing XCAN_IXR_* bits defined in
*		xcan_l.h.
*
* @note		None.
*
*****************************************************************************/
void XCan_InterruptClear(XCan *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read currently pending interrupts. */
	IntrValue = XCan_InterruptGetStatus(InstancePtr);

	/* Calculate the interrupts that should be cleared. */
	IntrValue &= Mask;

	/* Write to ICR to clear interrupts */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_ICR_OFFSET, IntrValue);
}

/*****************************************************************************/
/**
*
* This routine is the interrupt handler for the CAN driver.
*
* This handler reads the interrupt status from the ISR, determines the source of
* the interrupts, calls according callbacks, and finally clears the interrupts.
*
* Application beyond this driver is responsible for providing callbacks to
* handle interrupts and installing the callbacks using XCan_SetHandler() during
* initialization phase. An example delivered with this driver demonstrates how
* this could be done.
*
* @param	InstancePtr is a pointer to the XCan instance that just
*		interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCan_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr, EventIntr, ErrorStatus;
	XCan *CanPtr = (XCan *) InstancePtr;

	Xil_AssertVoid(CanPtr != NULL);
	Xil_AssertVoid(CanPtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Get pending interrupts
	 */
	PendingIntr = XCan_InterruptGetStatus(CanPtr);
	PendingIntr &= XCan_InterruptGetEnabled(CanPtr);

	/*
	 * An error interrupt is occurring
	 */
	if ((PendingIntr & XCAN_IXR_ERROR_MASK)) {
		ErrorStatus = XCan_GetBusErrorStatus(CanPtr);
		CanPtr->ErrorHandler(CanPtr->ErrorRef, ErrorStatus);

		/* Clear Error Status Register */
		XCan_ClearBusErrorStatus(CanPtr, ErrorStatus);
	}

	/*
	 * Check if any following event interrupt is pending:
	 *      - RX FIFO Overflow
	 *      - RX FIFO Underflow
	 *      - TX High Priority Buffer full
	 *      - TX FIFO Full
	 *      - Wake up from sleep mode
	 *      - Enter sleep mode
	 *      - Enter Bus off status
	 *      - Arbitration is lost
	 *
	 * If so, call event callback provided by upper level.
	 */
	EventIntr = PendingIntr & (XCAN_IXR_RXOFLW_MASK | XCAN_IXR_RXUFLW_MASK |
                                   XCAN_IXR_TXBFLL_MASK | XCAN_IXR_TXFLL_MASK |
                                   XCAN_IXR_WKUP_MASK | XCAN_IXR_SLP_MASK |
                                   XCAN_IXR_BSOFF_MASK | XCAN_IXR_ARBLST_MASK);

	if(CanPtr->EnableECC == 1) {
		EventIntr |= (PendingIntr & XCAN_IXR_ECC_MASK);

		/* Get ECC count and reset the counter because it reaches its maximum at 0xffff and does not overflow. */
		XCan_GetECCCount(CanPtr);
		XCan_ResetECC(CanPtr);
	}
	if (EventIntr) {
		CanPtr->EventHandler(CanPtr->EventRef, EventIntr);

		if ((EventIntr & XCAN_IXR_BSOFF_MASK)) {
			/*
			 * Event callback should reset whole device if "Enter
			 * Bus Off Status" interrupt occurred. All pending
			 * interrupts are cleared and no further checking and
			 * handling of other interrupts is needed any more.
			 */
			return;
		}
	}

	/*
	 * A frame was received and is sitting in RX FIFO.
	 *
	 * XCAN_IXR_RXOK_MASK is not used because the bit is set just once
	 * even if there are multiple frames sitting in RX FIFO.
	 *
	 * XCAN_IXR_RXNEMP_MASK is used because the bit can be set again and
	 * again automatically as long as there is at least one frame in RX
	 * FIFO.
	 */
	if ((PendingIntr & XCAN_IXR_RXNEMP_MASK)) {
		CanPtr->RecvHandler(CanPtr->RecvRef);
	}

	/*
	 * A frame was transmitted successfully
	 */
	if ((PendingIntr & XCAN_IXR_TXOK_MASK)) {
		CanPtr->SendHandler(CanPtr->SendRef);
	}

	/* Clear all pending interrupts */
	XCan_InterruptClear(CanPtr, PendingIntr);
}


/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  ---------------------------
* XCAN_HANDLER_SEND        XCan_SendRecvHandler
* XCAN_HANDLER_RECV        XCan_SendRecvHandler
* XCAN_HANDLER_ERROR       XCan_ErrorHandler
* XCAN_HANDLER_EVENT       XCan_EventHandler
*
* HandlerType              Invoked by this driver when:
* -----------------------  --------------------------------------------------
* XCAN_HANDLER_SEND        A frame transmitted by a call to
*                          XCan_Send() has been sent successfully.
*
* XCAN_HANDLER_RECV        A frame has been received and is sitting in
*                          the RX FIFO.
*
* XCAN_HANDLER_ERROR       An error interrupt is occurring.
*
* XCAN_HANDLER_EVENT       Any other kind of interrupt is occurring.
* </pre>
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	HandlerType specifies which handler is to be attached.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note
* Invoking this function for a handler that already has been installed replaces
* it with the new handler.
*
******************************************************************************/
int XCan_SetHandler(XCan *InstancePtr, u32 HandlerType,
		    void *CallBackFunc, void *CallBackRef)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	switch (HandlerType) {
	case XCAN_HANDLER_SEND:
		InstancePtr->SendHandler = (XCan_SendRecvHandler) CallBackFunc;
		InstancePtr->SendRef = CallBackRef;
		break;

	case XCAN_HANDLER_RECV:
		InstancePtr->RecvHandler = (XCan_SendRecvHandler) CallBackFunc;
		InstancePtr->RecvRef = CallBackRef;
		break;

	case XCAN_HANDLER_ERROR:
		InstancePtr->ErrorHandler = (XCan_ErrorHandler) CallBackFunc;
		InstancePtr->ErrorRef = CallBackRef;
		break;

	case XCAN_HANDLER_EVENT:
		InstancePtr->EventHandler = (XCan_EventHandler) CallBackFunc;
		InstancePtr->EventRef = CallBackRef;
		break;

	default:
		return (XST_INVALID_PARAM);

	}
	return (XST_SUCCESS);
}

/** @} */
