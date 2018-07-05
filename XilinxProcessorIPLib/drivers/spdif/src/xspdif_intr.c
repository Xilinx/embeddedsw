/*******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xspdif_intr.c
 * @addtogroup spdif_v1_0
 * @{
 * ...
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ----- ---------- -----------------------------------------------
 * 1.0   kar   01/25/18    Initial release.
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xspdif.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/*************************** Function Prototypes *****************************/

/************************** Variable Definitions *****************************/

/*************************** Function Definitions ****************************/

/*****************************************************************************/
/**
 *
 * This function is the interrupt handler for the XSpdif driver.
 *
 * This handler reads the pending interrupt from the XSpdif peripheral,
 * determines the source of the interrupts, clears the interrupts and calls
 * call backs accordingly.
 *
 *
 * @param InstancePtr is a pointer to the XSpdif instance.
 *
 * @return None.
 *
 * @note None.
 *
 ******************************************************************************/
void XSpdif_IntrHandler(void *InstancePtr)
{
	u32 Data;
	u32 EnableMask;

	/* Convert the non-typed pointer to a XSpdif instance pointer */
	XSpdif *SpdifPtr = (XSpdif *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(SpdifPtr != NULL);
	Xil_AssertVoid(SpdifPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read the interrupt status register */
	Data = XSpdif_ReadReg(SpdifPtr->Config.BaseAddress,
			XSPDIF_INTERRUPT_STATUS_REGISTER_OFFSET);

	/* Read the interrupt enable register */
	EnableMask = XSpdif_ReadReg(SpdifPtr->Config.BaseAddress,
			XSPDIF_INTERRUPT_ENABLE_REGISTER_OFFSET);

	Data = Data & EnableMask;
	/* Start of Block Detected */
	if (Data & XSPDIF_START_OF_BLOCK_MASK) {
		/* Clear the start of block event */
		XSpdif_IntrClear(SpdifPtr, XSPDIF_START_OF_BLOCK_MASK);

		/* Call the start of block Handler */
		if (SpdifPtr->StartOfBlockHandler)
		SpdifPtr->StartOfBlockHandler(SpdifPtr->StartOfBlockHandlerRef);
	}

	/* Premble error detected */
	if (Data & XSPDIF_PREAMBLE_ERROR_MASK) {
		/* Clear Preamble error event */
		XSpdif_IntrClear(SpdifPtr, XSPDIF_PREAMBLE_ERROR_MASK);

		/* Call the Preamble error Complete Handler */
		if (SpdifPtr->PreambleErrHandler)
		SpdifPtr->PreambleErrHandler(SpdifPtr->PreambleErrHandlerRef);
	}

	/* BMC Error Detected */
	if (Data & XSPDIF_BMC_ERROR_MASK) {
		/* Clear the BMC error event */
		XSpdif_IntrClear(SpdifPtr, XSPDIF_BMC_ERROR_MASK);

		/* Call the BMC Error Detected Handler */
		if (SpdifPtr->BmcErrHandler)
			SpdifPtr->BmcErrHandler(SpdifPtr->BmcErrHandlerRef);
	}

	/* Transmitter or Receiver FIFO Empty Detected */
	if (Data & XSPDIF_TX_OR_RX_FIFO_EMPTY_MASK) {
		/* Clear the Transmitter or Receiver Empty event */
		XSpdif_IntrClear(SpdifPtr, XSPDIF_TX_OR_RX_FIFO_EMPTY_MASK);

		/* Call the Transmitter or Receiver FIFO Empty Handler */
		if (SpdifPtr->TxOrRxFifoEmptyHandler)
			SpdifPtr->TxOrRxFifoEmptyHandler(
					SpdifPtr->TxOrRxFifoEmptyHandlerRef);
	}

	/* Transmitter or Receiver FIFO Full Detected */
	if (Data & XSPDIF_TX_OR_RX_FIFO_FULL_MASK) {
		/* Clear the Transmitter or Receiver Empty event */
		XSpdif_IntrClear(SpdifPtr, XSPDIF_TX_OR_RX_FIFO_FULL_MASK);

		/* Call the transmitter or Receiver Empty Handler */
		if (SpdifPtr->TxOrRxFifoFullHandler)
			SpdifPtr->TxOrRxFifoFullHandler(
					SpdifPtr->TxOrRxFifoFullHandlerRef);
	}
}
/*****************************************************************************/
/**
 *
 * This function installs an asynchronous callback function for the given
 * HandlerType:
 *
 * <pre>
 * HandlerType                              Callback Function
 * --------------------------------         ----------------------------------
 * (XSPDIF_HANDLER_START_OF_BLOCK)          StartOfBlockHandler
 * (XSPDIF_HANDLE_PREAMBLE_ERROR)           PreambleErrHandler
 * (XSPDIF_HANDLE_BMC_ERROR)	            BmcErrHandler
 * (XSPDIF_HANDLER_TX_OR_RX_FIFO_EMPTY)     TxOrRxFifoEmptyHandler
 * (XSPDIF_HANDLER_TX_OR_RX_FIFO_FULL)	    TxOrRxFifoFullHandler
 * </pre>
 *
 * @param InstancePtr is a pointer to the XSpdif core instance.
 * @param HandlerType specifies the type of handler.
 * @param FuncPtr is a pointer to the callback function.
 * @param CallbackRef is a reference pointer passed on actual
 *        calling of the callback function.
 *
 * @return
 *  - XST_SUCCESS if callback function installed successfully.
 *  - XST_INVALID_PARAM when HandlerType is invalid.
 *
 * @note Invoking this function for a handler that already has been
 *       installed replaces it with the new handler.
 *
 ******************************************************************************/
int XSpdif_SetHandler(XSpdif *InstancePtr, XSpdif_HandlerType HandlerType,
		XSpdif_Callback FuncPtr, void *CallbackRef)
{
	int Status = XST_SUCCESS;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType <  XSPDIF_NUM_HANDLERS);
	Xil_AssertNonvoid(FuncPtr != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {
		case XSPDIF_HANDLER_START_OF_BLOCK:
			InstancePtr->StartOfBlockHandler = FuncPtr;
			InstancePtr->StartOfBlockHandlerRef = CallbackRef;
			break;

		case XSPDIF_HANDLER_PREAMBLE_ERROR:
			InstancePtr->PreambleErrHandler = FuncPtr;
			InstancePtr->PreambleErrHandlerRef = CallbackRef;
			break;

		case XSPDIF_HANDLER_BMC_ERROR:
			InstancePtr->BmcErrHandler = FuncPtr;
			InstancePtr->BmcErrHandlerRef = CallbackRef;
			break;

		case XSPDIF_HANDLER_TX_OR_RX_FIFO_EMPTY:
			InstancePtr->TxOrRxFifoEmptyHandler = FuncPtr;
			InstancePtr->TxOrRxFifoEmptyHandlerRef = CallbackRef;
			break;

		case XSPDIF_HANDLER_TX_OR_RX_FIFO_FULL:
			InstancePtr->TxOrRxFifoFullHandler = FuncPtr;
			InstancePtr->TxOrRxFifoFullHandlerRef = CallbackRef;
			break;

		default:
			Status = XST_INVALID_PARAM;
			break;
	}
	return Status;
}
/** @} */

