/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanps_intr.c
* @brief Implements interrupt handling functions for the XCanPs CAN driver.
*
* This file contains functions related to CAN interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00a xd/sv  01/12/10 First release
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.1   nsk    12/21/15 Updated XCanPs_IntrHandler to handle error
*			interrupts correctly. CR#925615
* 3.5	sne    07/01/20 Fixed MISRAC warnings.
* 3.13  vn     07/30/26 Fix Doxygen warnings
* 3.13  vn   09/08/26 Align Doxygen comments with AMD standards
* 3.13  vn   09/30/26 Update Doxygen comments to AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup canps Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xcanps.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/****************************************************************************/
/**
* @brief Enables the specified interrupts.
*
* Use the XCANPS_IXR_* constants defined in xcanps_hw.h to create the
* bit-mask to enable interrupts.
*
* @param[in]   InstancePtr Pointer to the XCanPs instance.
* @param[in]   Mask Mask to enable. Bit positions of 1 are enabled.
*              Bit positions of 0 keep the previous setting. This mask is
*              formed by a bitwise OR of XCANPS_IXR_* bits defined in
*              xcanps_hw.h.
*
*****************************************************************************/
void XCanPs_IntrEnable(XCanPs *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the IER to enable the specified interrupts.
	 */
	IntrValue = XCanPs_IntrGetEnabled(InstancePtr);
	IntrValue |= Mask;
	XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
			XCANPS_IER_OFFSET, IntrValue);
}

/****************************************************************************/
/**
* @brief Disables the specified interrupts.
*
* Use the XCANPS_IXR_* constants defined in xcanps_hw.h to create the
* bit-mask to disable interrupts.
*
* @param[in]   InstancePtr Pointer to the XCanPs instance.
* @param[in]   Mask Mask to disable. Bit positions of 1 are
*              disabled. Bit positions of 0 keep the previous setting.
*              This mask is formed by a bitwise OR of XCANPS_IXR_* bits
*              defined in xcanps_hw.h.
*
*****************************************************************************/
void XCanPs_IntrDisable(XCanPs *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the IER to disable the specified interrupts.
	 */
	IntrValue = XCanPs_IntrGetEnabled(InstancePtr);
	IntrValue &= ~Mask;
	XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
			XCANPS_IER_OFFSET, IntrValue);
}

/****************************************************************************/
/**
* @brief Returns the currently enabled interrupts.
*
* Use the XCANPS_IXR_* constants defined in xcanps_hw.h to interpret the
* returned value.
*
* @param[in]    InstancePtr Pointer to the XCanPs instance.
*
* @return	Enabled interrupts in a 32-bit format.
*
*****************************************************************************/
u32 XCanPs_IntrGetEnabled(XCanPs *InstancePtr)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_IER_OFFSET);
}

/****************************************************************************/
/**
* @brief Returns the interrupt status.
*
* Reads the Interrupt Status Register. Use the XCANPS_IXR_* constants
* defined in xcanps_hw.h to interpret the
* returned value.
*
* @param[in]    InstancePtr Pointer to the XCanPs instance.
*
* @return	The value stored in Interrupt Status Register.
*
*****************************************************************************/
u32 XCanPs_IntrGetStatus(XCanPs *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_ISR_OFFSET);
}

/****************************************************************************/
/**
* @brief Clears the specified interrupts.
*
* Every bit set in Interrupt Status Register indicates that a specific type of
* interrupt is occurring. This function clears one or more interrupts by writing
* a bit mask to Interrupt Clear Register.
*
* @param[in]   InstancePtr Pointer to the XCanPs instance.
* @param[in]   Mask Mask to clear. Bit positions of 1 are cleared.
*              Bit positions of 0 do not change the previous interrupt
*              status. This mask is formed by a bitwise OR of XCANPS_IXR_*
*              bits defined in xcanps_hw.h.
*
*****************************************************************************/
void XCanPs_IntrClear(XCanPs *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Clear the currently pending interrupts.
	 */
	IntrValue = XCanPs_IntrGetStatus(InstancePtr);
	IntrValue &= Mask;
	XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr, XCANPS_ICR_OFFSET,
				IntrValue);
}

/*****************************************************************************/
/**
* @brief Handles CAN interrupts.
*
* This handler reads the interrupt status from the ISR, determines the source of
* the interrupts, calls according callbacks, and finally clears the interrupts.
*
* The application beyond this driver is responsible for providing callbacks to
* handle interrupts and installing the callbacks using XCanPs_SetHandler()
* during initialization phase. An example delivered with this driver
* demonstrates how this could be done.
*
* @param[in]   InstancePtr Pointer to the XCanPs instance that just
*              interrupted.
*
******************************************************************************/
void XCanPs_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 EventIntr;
	u32 ErrorStatus;
	XCanPs *CanPtr = (XCanPs *) InstancePtr;

	Xil_AssertVoid(CanPtr != NULL);
	Xil_AssertVoid(CanPtr->IsReady == XIL_COMPONENT_IS_READY);

	PendingIntr = XCanPs_IntrGetStatus(CanPtr);
	PendingIntr &= XCanPs_IntrGetEnabled(CanPtr);

	/*
	 * Clear all pending interrupts.
	 * Rising Edge interrupt
	 */
	XCanPs_IntrClear(CanPtr, PendingIntr);

	/*
	 * An error interrupt is occurring.
	 */
	if (((PendingIntr & XCANPS_IXR_ERROR_MASK) != (u32)0) &&
		(CanPtr->ErrorHandler != NULL)) {
			ErrorStatus = XCanPs_GetBusErrorStatus(CanPtr);
			CanPtr->ErrorHandler(CanPtr->ErrorRef,ErrorStatus);
		/*
		 * Clear Error Status Register.
		 */
		XCanPs_ClearBusErrorStatus(CanPtr,ErrorStatus);
	}

	/*
	 * Check if any following event interrupt is pending:
	 *	  - RX FIFO Overflow
	 *	  - RX FIFO Underflow
	 *	  - TX High Priority Buffer full
	 *	  - TX FIFO Full
	 *	  - Wake up from sleep mode
	 *	  - Enter sleep mode
	 *	  - Enter Bus off status
	 *	  - Arbitration is lost
	 *
	 * If so, call event callback provided by upper level.
	 */
	EventIntr = PendingIntr & ((u32)XCANPS_IXR_RXOFLW_MASK |
				(u32)XCANPS_IXR_RXUFLW_MASK |
				(u32)XCANPS_IXR_TXBFLL_MASK |
				(u32)XCANPS_IXR_TXFLL_MASK |
				(u32)XCANPS_IXR_WKUP_MASK |
				(u32)XCANPS_IXR_SLP_MASK |
				(u32)XCANPS_IXR_BSOFF_MASK |
				(u32)XCANPS_IXR_ARBLST_MASK);
	if ((EventIntr != (u32)0) && (CanPtr->EventHandler != NULL)) {
		CanPtr->EventHandler(CanPtr->EventRef, EventIntr);

		if ((EventIntr & XCANPS_IXR_BSOFF_MASK) != (u32)0) {
			/*
			 * Event callback should reset whole device if "Enter
			 * Bus Off Status" interrupt occurred. All pending
			 * interrupts are cleared and no further checking and
			 * handling of other interrupts is needed any more.
			 */
			return;
		}
	}

	if (((PendingIntr & (XCANPS_IXR_RXFWMFLL_MASK |
			XCANPS_IXR_RXNEMP_MASK)) != (u32)0) &&
		(CanPtr->RecvHandler != NULL)) {

		/*
		 * This case happens when
		 * A number of frames depending on the Rx FIFO Watermark
		 * threshold are received.
		 * And  also when frame was received and is sitting in RX FIFO.
		 *
		 * XCANPS_IXR_RXOK_MASK is not used because the bit is set
		 * just once even if there are multiple frames sitting
		 * in the RX FIFO.
		 *
		 * XCANPS_IXR_RXNEMP_MASK is used because the bit can be
		 * set again and again automatically as long as there is
		 * at least one frame in RX FIFO.
		 */
		CanPtr->RecvHandler(CanPtr->RecvRef);
	}

	/*
	 * A frame was transmitted successfully.
	 */
	if (((PendingIntr & (XCANPS_IXR_TXOK_MASK | XCANPS_IXR_TXFWMEMP_MASK)) != (u32)0) &&
		(CanPtr->SendHandler != NULL)) {
		CanPtr->SendHandler(CanPtr->SendRef);
	}
}


/*****************************************************************************/
/**
* @brief Installs an asynchronous callback function.
*
* Installs a callback for the given HandlerType:
*
* <pre>
* HandlerType			Callback Function Type
* -----------------------	------------------------
* XCANPS_HANDLER_SEND		XCanPs_SendRecvHandler
* XCANPS_HANDLER_RECV		XCanPs_SendRecvHandler
* XCANPS_HANDLER_ERROR		XCanPs_ErrorHandler
* XCANPS_HANDLER_EVENT		XCanPs_EventHandler
*
* HandlerType			Invoked by this driver when:
* -------------------------------------------------------------------------
* XCANPS_HANDLER_SEND		A frame transmitted by a call to
*				XCanPs_Send() has been sent successfully.
*
* XCANPS_HANDLER_RECV		A frame has been received and is sitting in
*				the RX FIFO.
*
* XCANPS_HANDLER_ERROR		An error interrupt is occurring.
*
* XCANPS_HANDLER_EVENT		Any other kind of interrupt is occurring.
* </pre>
*
* @param[in,out] InstancePtr Pointer to the XCanPs instance.
* @param[in]     HandlerType Specifies which handler is to be attached.
* @param[in]     CallBackFunc Address of the callback function.
* @param[in]     CallBackRef User data item that is passed to the
*                callback function when it is invoked.
*
* @return
*                - XST_SUCCESS when handler is installed.
*                - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note          Invoking this function for a handler that already has been
*                installed replaces it with the new handler.
*
******************************************************************************/
s32 XCanPs_SetHandler(XCanPs *InstancePtr, u32 HandlerType,
			void *CallBackFunc, void *CallBackRef)
{
	s32 Status;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	switch (HandlerType) {
		case XCANPS_HANDLER_SEND:
			InstancePtr->SendHandler =
				(XCanPs_SendRecvHandler) CallBackFunc;
			InstancePtr->SendRef = CallBackRef;
			Status = (s32)XST_SUCCESS;
			break;

		case XCANPS_HANDLER_RECV:
			InstancePtr->RecvHandler =
				(XCanPs_SendRecvHandler) CallBackFunc;
			InstancePtr->RecvRef = CallBackRef;
			Status = (s32)XST_SUCCESS;
			break;

		case XCANPS_HANDLER_ERROR:
			InstancePtr->ErrorHandler =
				(XCanPs_ErrorHandler) CallBackFunc;
			InstancePtr->ErrorRef = CallBackRef;
			Status = (s32)XST_SUCCESS;
			break;

		case XCANPS_HANDLER_EVENT:
			InstancePtr->EventHandler =
				(XCanPs_EventHandler) CallBackFunc;
			InstancePtr->EventRef = CallBackRef;
			Status = (s32)XST_SUCCESS;
			break;

		default:
			Status = (s32)XST_INVALID_PARAM;
			break;
	}
	return Status;
}

/** @} */
