/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanfd_intr.c
* @brief Implements interrupt handling functions for the XCanFd driver.
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
* 2.12  vlt  01/27/26 Fixed codespell issues
* 2.13  vn   07/30/26 Fix Doxygen warnings
* 2.13  vn   09/08/26 Align Doxygen comments with AMD standards
* 2.13  vn   09/30/26 Update Doxygen comments to AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup canfd Overview
 * @{
 */


/***************************** Include Files *********************************/

#include "xcanfd.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/


/************************** Function Prototypes ******************************/


/****************************************************************************/
/**
* @brief Enables one or more interrupts.
*
* Use the XCANFD_IXR_* constants defined in xcanfd_hw.h to create the
* bit-mask to enable interrupts.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance.
* @param[in]    Mask Mask to enable. Bit positions of 1 are
*               enabled. Bit positions of 0 keep the previous setting.
*               This mask is formed as a bitwise OR of XCANFD_IXR_* bits
*               defined in xcanfd_hw.h.
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
* @brief Enables one or more TxBuffer Ready Request interrupts.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance.
* @param[in]    Mask Mask to enable. Bit positions of 1 are
*               enabled. Bit positions of 0 keep the previous setting.
*               Form this mask by bitwise-ORing XCANFD_TXBUFFER*_RDY_RQT_MASK
*               values defined in xcanfd_hw.h.
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
* @brief Enables one or more TxBuffer Cancellation interrupts.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance.
* @param[in]    Mask Mask to enable. Bit positions of 1 are
*               enabled. Bit positions of 0 keep the previous setting.
*               Form this mask by bitwise-ORing XCANFD_TXBUFFER*_CANCEL_RQT_MASK
*               values defined in xcanfd_hw.h.
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
* @brief Disables one or more TxBuffer Ready Request interrupts.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance.
* @param[in]    Mask Mask to disable. Bit positions of 1 are
*               disabled. Bit positions of 0 keep the previous setting.
*               Form this mask by bitwise-ORing XCANFD_TXBUFFER*_RDY_RQT_MASK
*               values defined in xcanfd_hw.h.
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
* @brief Disables one or more TxBuffer Cancel Request interrupts.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance.
* @param[in]    Mask Mask to disable. Bit positions of 1 are
*               disabled. Bit positions of 0 keep the previous setting.
*               Form this mask by bitwise-ORing XCANFD_TXBUFFER*_CANCEL_RQT_MASK
*               values defined in xcanfd_hw.h.
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
* @brief Enables one or more RxBuffer Full interrupts in MailBox Mode.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance.
* @param[in]    Mask Mask to enable. Bit positions of 1 are
*               enabled. Bit positions of 0 keep the previous setting.
*               Each set bit selects the corresponding Rx buffer interrupt.
* @param[in]    RxBuffNumber Has two values:
*		if 0 -> Access RxBufferFull0 Reg,
*		else -> Access RxBufferFull1 Reg.
*
****************************************************************************/
void XCanFd_InterruptEnable_RxBuffFull(XCanFd *InstancePtr,
		u32 Mask,u32 RxBuffNumber)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (RxBuffNumber == (u32)0) {
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
* @brief Disables one or more RxBuffer Full interrupts in MailBox Mode.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance.
* @param[in]    Mask Mask to disable. Bit positions of 1 are
*               disabled. Bit positions of 0 keep the previous setting.
*               Each set bit selects the corresponding Rx buffer interrupt.
* @param[in]    RxBuffNumber Has two values:
*		if 0 -> Access RxBufferFull0 Reg,
*		else -> Access RxBufferFull1 Reg.
*
*****************************************************************************/
void XCanFd_InterruptDisable_RxBuffFull(XCanFd *InstancePtr,
		u32 Mask,u32 RxBuffNumber)
{

	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (RxBuffNumber == (u32)0) {
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
* @brief Disables one or more interrupts.
*
* Use the XCANFD_IXR_* constants defined in xcanfd_hw.h to create the
* bit-mask to disable interrupts.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance.
* @param[in]    Mask Mask to disable. Bit positions of 1 are
*               disabled. Bit positions of 0 keep the previous setting.
*               This mask is formed as a bitwise OR of XCANFD_IXR_* bits
*               defined in xcanfd_hw.h.
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
* @brief Clears one or more interrupts.
*
* Every bit set in Interrupt Status Register indicates that a specific type of
* interrupt is occurring. This function clears one or more interrupts by writing
* a bit mask to Interrupt Clear Register.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance.
* @param[in]    Mask Mask to clear. Bit positions of 1 are
*               cleared. Bit positions of 0 do not change the previous
*               interrupt status. This mask is formed as a bitwise OR of
*		XCANFD_IXR_* bits defined in xcanfd_hw.h.
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
* @brief Handles CAN FD driver interrupts.
*
* Reads the interrupt status from the ISR, determines the source of
* the interrupts, calls according callbacks, and finally clears the
* interrupts.
*
* Application beyond this driver is responsible for providing callbacks to
* handle interrupts and installing the callbacks using XCanFd_SetHandler()
* during initialization phase. An example delivered with this driver
* demonstrates how this could be done.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance that just
*		interrupted.
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
	 *      - Transmit Cancellation Request served
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
* @brief Installs an asynchronous callback function.
*
* Installs a callback for the given HandlerType:
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
* @param[in,out] InstancePtr Pointer to the XCanFd instance.
* @param[in]     HandlerType Specifies which handler is to be attached.
* @param[in]     CallBackFunc Address of the callback function.
* @param[in]     CallBackRef User data item that is passed to the
*                callback function when it is invoked.
*
* @return        - XST_SUCCESS when handler is installed.
*                - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note          Invoking this function for a handler that already has been
*                installed replaces it with the new handler.
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
