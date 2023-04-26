/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanfd_intr.c
* @addtogroup canfd Overview
* @{
*
* This file contains functions related to CAN interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date      Changes
* ----- ---- --------- -------------------------------------------------------
* 1.0   nsk  06/04/15  First release
* 1.0	nsk  16/06/15  Updated XCanFd_IntrHandler() since RTL has
*		       changed. RTL Changes,Added new bits to MSR,SR,ISR,
*		       IER,ICR Registers and modified TS2 bits in
*		       BTR and F_SJW bits in F_BTR Registers.
* 1.2   mi   09/22/16  Fixed compilation warnings.
* 2.1   ask  07/03/18 Added support for canfd 2.0 spec sequential mode.
*					   XCanFd_SetRxIntrWatermark : This function has been
*					   moved to xcanfd_intr.c
*       ask  07/03/18 Fix for Sequential recv CR# 992606,CR# 1004222.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcanfd.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/


/************************** Function Prototypes ******************************/


/****************************************************************************/
/**
*
* This routine enables interrupt(s). Use the XCANFD_IXR_* constants defined in
* xcanfd_hw.h to create the bit-mask to enable interrupts.
*
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Mask is the mask to enable. Bit positions of 1 will be enabled.
*		Bit positions of 0 will keep the previous setting. This mask is
*		formed by OR'ing XCANFD_IXR_* bits defined in xcanfd_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XCanFd_InterruptEnable(XCanFd *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	IntrValue = XCanFd_InterruptGetEnabled(InstancePtr);

	IntrValue |= Mask & XCANFD_IXR_ALL;

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_IER_OFFSET,
			IntrValue);
}

/****************************************************************************/
/**
*
* This routine enables TxBuffer Ready Request interrupt(s).
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Mask is the mask to enable. Bit positions of 1 will be enabled.
*		Bit positions of 0 will keep the previous setting. This mask is
*		formed by OR'ing XCANFD_IETRS_OFFSET* bits defined in xcanfd_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XCanFd_InterruptEnable_ReadyRqt(XCanFd *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	IntrValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_IETRS_OFFSET);
	IntrValue |= Mask;
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_IETRS_OFFSET, IntrValue);
}

/****************************************************************************/
/**
*
* This routine enables TxBuffer Cancellation interrupt(s).
*
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Mask is the mask to enable. Bit positions of 1 will be enabled.
*		Bit positions of 0 will keep the previous setting. This mask is
*		formed by OR'ing XCANFD_IETCS_OFFSET* bits defined in xcanfd_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XCanFd_InterruptEnable_CancelRqt(XCanFd *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	IntrValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_IETCS_OFFSET);
	IntrValue |= Mask;
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_IETCS_OFFSET, IntrValue);
}

/****************************************************************************/
/**
*
* This routine disables TxBuffer Ready Request interrupt(s).
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Mask is the mask to disable. Bit positions of 1 will be
*		disabled. Bit positions of 0 will keep the previous setting.
*		This mask is formed by AND'ing XCANFD_IETRS_OFFSET* bits
*		defined in xcanfd_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XCanFd_InterruptDisable_ReadyRqt(XCanFd *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	IntrValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_IETRS_OFFSET);
	IntrValue &= ~Mask;
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_IETRS_OFFSET, IntrValue);
}

/****************************************************************************/
/**
*
* This routine disables the TxBuffer Cancel Request interrupt(s).
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Mask is the mask to disable. Bit positions of 1 will be
*		disabled. Bit positions of 0 will keep the previous setting.
*		This mask is formed by AND'ing XCANFD_IETCS_OFFSET* bits defined in
*		xcanfd_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XCanFd_InterruptDisable_CancelRqt(XCanFd *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	IntrValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_IETCS_OFFSET);
	IntrValue &= ~Mask;
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_IETCS_OFFSET, IntrValue);
}

/****************************************************************************/
/**
*
* This routine Enables the RxBuffer Full interrupt(s) in MailBox Mode.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Mask is the mask to disable. Bit positions of 1 will be
*		disabled. Bit positions of 0 will keep the previous setting.
*		This mask is formed by AND'ing XCANFD_RXBFLL*_OFFSET bits
*		defined	in xcanfd_hw.h.
* @param	RxBuffNumber has two values
*		if 0 -> Access RxBufferFull0 Reg
*		else -> Access RxBufferFull1 Reg
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XCanFd_InterruptEnable_RxBuffFull(XCanFd *InstancePtr,
		u32 Mask,u32 RxBuffNumber)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if ((RxBuffNumber == (u32)0)) {
		IntrValue = XCanFd_ReadReg(
				InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RXBFLL1_OFFSET);
		IntrValue |= Mask;
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RXBFLL1_OFFSET, IntrValue);
	}
	else {
		IntrValue = XCanFd_ReadReg(
				InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RXBFLL2_OFFSET);
		IntrValue |= Mask;
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RXBFLL2_OFFSET, IntrValue);
	}
}

/****************************************************************************/
/**
*
* This routine disables the RxBuffer Full interrupt(s) in MailBox Mode.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Mask is the mask to disable. Bit positions of 1 will be
*		disabled. Bit positions of 0 will keep the previous setting.
*		This mask is formed by AND'ing XCANFD_RXBFLL*_OFFSET bits
*		defined in xcanfd_hw.h.
*
*@param		RxBuffNumber has two values
*		if 0 -> Access RxBufferFull0 Reg.
*		else -> Access RxBufferFull1 Reg.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XCanFd_InterruptDisable_RxBuffFull(XCanFd *InstancePtr,
		u32 Mask,u32 RxBuffNumber)
{

	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if ((RxBuffNumber == (u32)0)) {
		IntrValue =
			XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RXBFLL1_OFFSET);
		IntrValue &= ~Mask;
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RXBFLL1_OFFSET, IntrValue);
	}

	else {
		IntrValue =
			XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RXBFLL2_OFFSET);
		IntrValue &= ~Mask;
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RXBFLL2_OFFSET, IntrValue);
	}
}

/****************************************************************************/
/**
*
* This routine disables interrupt(s). Use the XCANFD_IXR_* constants defined in
* xcanfd_hw.h to create the bit-mask to disable interrupt(s).
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Mask is the mask to disable. Bit positions of 1 will be
*		disabled. Bit positions of 0 will keep the previous setting.
*		This mask is formed by OR'ing XCANFD_IXR_* bits defined in
*		xcanfd_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XCanFd_InterruptDisable(XCanFd *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	IntrValue = XCanFd_InterruptGetEnabled(InstancePtr);

	IntrValue &= ~Mask;

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_IER_OFFSET, IntrValue);
}

/****************************************************************************/
/**
*
* This function clears interrupt(s). Every bit set in Interrupt Status
* Register indicates that a specific type of interrupt is occurring, and this
* function clears one or more interrupts by writing a bit mask to Interrupt
* Clear Register.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Mask is the mask to clear. Bit positions of 1 will be cleared.
*		Bit positions of 0 will not change the previous interrupt
*		status. This mask is formed by OR'ing XCANFD_IXR_* bits defined
*		in xcanfd_hw.h.
*
* @note		None.
*
*****************************************************************************/
void XCanFd_InterruptClear(XCanFd *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	IntrValue = XCanFd_InterruptGetStatus(InstancePtr);

	IntrValue &= Mask;

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_ICR_OFFSET, IntrValue);
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
* handle interrupts and installing the callbacks using XCanFd_SetHandler() during
* initialization phase. An example delivered with this driver demonstrates how
* this could be done.
*
* @param	InstancePtr is a pointer to the XCanFd instance that just
*		interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCanFd_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr, EventIntr, ErrorStatus;
	XCanFd *CanPtr = (XCanFd *) InstancePtr;

	Xil_AssertVoid(CanPtr != NULL);
	Xil_AssertVoid(CanPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get pending interrupts */
	PendingIntr = XCanFd_InterruptGetStatus(CanPtr);
	PendingIntr &= XCanFd_InterruptGetEnabled(CanPtr);

	/* An error interrupt is occurring */
	if ((PendingIntr & XCANFD_IXR_ERROR_MASK) != (u32)0) {
		ErrorStatus = XCanFd_GetBusErrorStatus(CanPtr);
		CanPtr->ErrorHandler(CanPtr->ErrorRef, ErrorStatus);

		/* Clear Error Status Register */
		XCanFd_ClearBusErrorStatus(CanPtr, ErrorStatus);
	}

	/*
	 * Check if any following event interrupt is pending:
	 *      - RX FIFO Overflow
	 *      - RX FIFO Underflow
	 *      - Rx Match Not Finished
	 *      - Rx Buffer Overflow for Buffer Index(Mail Box)
	 *      - Transmit Cancelation Request served
	 *      - Transmit Ready Request Served
	 *      - Wake up from sleep mode
	 *      - Rx Buffer Full(Mail Box)
	 *      - TX FIFO Full
	 *      - Enter sleep mode
	 *      - Enter Bus off status
	 *      - Arbitration is lost
	 *	- Protocol Exception Event
	 *
	 * If so, call event callback provided by upper level.
	 */
	EventIntr = PendingIntr & (XCANFD_IXR_RXBOFLW_BI_MASK |
			XCANFD_IXR_RXMNF_MASK |
			XCANFD_IXR_RXBOFLW_MASK |
			XCANFD_IXR_TXCRS_MASK | XCANFD_IXR_TXRRS_MASK |
			XCANFD_IXR_WKUP_MASK |
			XCANFD_IXR_SLP_MASK | XCANFD_IXR_BSOFF_MASK |
			XCANFD_IXR_RXFOFLW_MASK | XCANFD_IXR_ARBLST_MASK |
			XCANFD_IXR_RXRBF_MASK |
			XCANFD_IXR_PEE_MASK | XCANFD_IXR_TSCNT_OFLW_MASK |
			XCANFD_IXR_BSRD_MASK |
			XCANFD_IXR_TXEOFLW_MASK);
	if (EventIntr != (u32)0) {

		CanPtr->EventHandler(CanPtr->EventRef, EventIntr);
	}

	/*
	 * A frame was received and is sitting in RX FIFO.
	 *
	 * XCANFD_IXR_RXOK_MASK is used because the bit is set when a frame
	 * is sit in the buffer
	 *
	 * XCANFD_IXR_RXFWMFLL_MASK is used because the bit can be set
	 * when configured level of frames are set
	 *
	 * XCANFD_IXR_RXRBF_MASK is used because the bit is set in MAIL BOX Mode
	 * when a message is received and becomes FULL.
	 */
	if ((PendingIntr & (XCANFD_IXR_RXFWMFLL_MASK | XCANFD_IXR_RXOK_MASK \
			| XCANFD_IXR_RXRBF_MASK | XCANFD_IXR_RXFWMFLL_1_MASK)) !=(u32)0) {

		CanPtr->RecvHandler(CanPtr->RecvRef);
	}

	/* A frame was transmitted successfully */
	if ((PendingIntr & (XCANFD_IXR_TXOK_MASK | XCANFD_IXR_TXEWMFLL_MASK)) != (u32)0) {
		CanPtr->SendHandler(CanPtr->SendRef);
	}

	/* Clear all pending interrupts */
	XCanFd_InterruptClear(CanPtr, PendingIntr);
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
* XCANFD_HANDLER_SEND        XCanFd_SendRecvHandler
* XCANFD_HANDLER_RECV        XCanFd_SendRecvHandler
* XCANFD_HANDLER_ERROR       XCanFd_ErrorHandler
* XCANFD_HANDLER_EVENT       XCanFd_EventHandler
*
* HandlerType              Invoked by this driver when:
* -----------------------  --------------------------------------------------
* XCANFD_HANDLER_SEND        A frame transmitted by a call to
*                          XCanFd_Send() has been sent successfully.
*
* XCANFD_HANDLER_RECV        A frame has been received and is sitting in
*                          the RX FIFO.
*
* XCANFD_HANDLER_ERROR       An error interrupt is occurring.
*
* XCANFD_HANDLER_EVENT       Any other kind of interrupt is occurring.
* </pre>
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	HandlerType specifies which handler is to be attached.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return	- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
int XCanFd_SetHandler(XCanFd *InstancePtr, u32 HandlerType,
		    void *CallBackFunc, void *CallBackRef)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	switch (HandlerType) {
	case XCANFD_HANDLER_SEND:
		InstancePtr->SendHandler = (XCanFd_SendRecvHandler) CallBackFunc;
		InstancePtr->SendRef = CallBackRef;
		Status = XST_SUCCESS;
		break;

	case XCANFD_HANDLER_RECV:
		InstancePtr->RecvHandler = (XCanFd_SendRecvHandler) CallBackFunc;
		InstancePtr->RecvRef = CallBackRef;
		Status = XST_SUCCESS;
		break;

	case XCANFD_HANDLER_ERROR:
		InstancePtr->ErrorHandler = (XCanFd_ErrorHandler) CallBackFunc;
		InstancePtr->ErrorRef = CallBackRef;
		Status = XST_SUCCESS;
		break;

	case XCANFD_HANDLER_EVENT:
		InstancePtr->EventHandler = (XCanFd_EventHandler) CallBackFunc;
		InstancePtr->EventRef = CallBackRef;
		Status = XST_SUCCESS;
		break;

	default:
		Status = XST_INVALID_PARAM;
		break;

	}
	return (s32)Status;
}
/** @} */
