/*******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xhdmiphy1_intr.c
 *
 * This file contains functions related to XHdmiphy1 interrupt handling.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/12/18 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xhdmiphy1.h"
#include "xhdmiphy1_i.h"

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function enables interrupts associated with the specified interrupt type
 *
 * @param	InstancePtr is a pointer to the XHdmiphy1 instance.
 * @param	Intr is the interrupt type/mask to enable.
 *
 * @return	None.
 *
 * @note	None.
*
*******************************************************************************/
void XHdmiphy1_IntrEnable(XHdmiphy1 *InstancePtr,
		XHdmiphy1_IntrHandlerType Intr)
{
	u32 RegVal;

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
                XHDMIPHY1_INTR_EN_REG);
	RegVal |= Intr;
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_INTR_EN_REG,
        RegVal);
}

/******************************************************************************/
/**
 * This function disabled interrupts associated with the specified interrupt
 * type.
 *
 * @param	InstancePtr is a pointer to the XHdmiphy1 instance.
 * @param	Intr is the interrupt type/mask to disable.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XHdmiphy1_IntrDisable(XHdmiphy1 *InstancePtr,
		XHdmiphy1_IntrHandlerType Intr)
{
	u32 RegVal;

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_INTR_DIS_REG);
	RegVal |= Intr;
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_INTR_DIS_REG,
			RegVal);
}

/******************************************************************************/
/**
 * This function installs a callback function for the specified handler type.
 *
 * @param	InstancePtr is a pointer to the XHdmiphy1 instance.
 * @param	HandlerType is the interrupt handler type which specifies which
 *		interrupt event to attach the callback for.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XHdmiphy1_SetIntrHandler(XHdmiphy1 *InstancePtr,
        XHdmiphy1_IntrHandlerType HandlerType,
        XHdmiphy1_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_TXRESET_DONE) ||
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_RXRESET_DONE) ||
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_CPLL_LOCK) ||
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_QPLL_LOCK) ||
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_TXALIGN_DONE) ||
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_QPLL1_LOCK) ||
#else
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_LCPLL_LOCK) ||
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_RPLL_LOCK) ||
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_TX_GPO_RISING_EDGE) ||
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_RX_GPO_RISING_EDGE) ||
#endif
		(HandlerType ==
			XHDMIPHY1_INTR_HANDLER_TYPE_TX_CLKDET_FREQ_CHANGE) ||
		(HandlerType ==
			XHDMIPHY1_INTR_HANDLER_TYPE_RX_CLKDET_FREQ_CHANGE) ||
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_TX_MMCM_LOCK_CHANGE) ||
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_RX_MMCM_LOCK_CHANGE) ||
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_TX_TMR_TIMEOUT) ||
		(HandlerType == XHDMIPHY1_INTR_HANDLER_TYPE_RX_TMR_TIMEOUT));
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	switch (HandlerType) {
	case XHDMIPHY1_INTR_HANDLER_TYPE_TXRESET_DONE:
		InstancePtr->IntrTxResetDoneHandler = CallbackFunc;
		InstancePtr->IntrTxResetDoneCallbackRef = CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_RXRESET_DONE:
		InstancePtr->IntrRxResetDoneHandler = CallbackFunc;
		InstancePtr->IntrRxResetDoneCallbackRef = CallbackRef;
		break;
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
	case XHDMIPHY1_INTR_HANDLER_TYPE_CPLL_LOCK:
		InstancePtr->IntrCpllLockHandler = CallbackFunc;
		InstancePtr->IntrCpllLockCallbackRef = CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_QPLL_LOCK:
		InstancePtr->IntrQpllLockHandler = CallbackFunc;
		InstancePtr->IntrQpllLockCallbackRef = CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_TXALIGN_DONE:
		InstancePtr->IntrTxAlignDoneHandler = CallbackFunc;
		InstancePtr->IntrTxAlignDoneCallbackRef = CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_QPLL1_LOCK:
		InstancePtr->IntrQpll1LockHandler = CallbackFunc;
		InstancePtr->IntrQpll1LockCallbackRef = CallbackRef;
		break;
#else
	case XHDMIPHY1_INTR_HANDLER_TYPE_TX_GPO_RISING_EDGE:
		InstancePtr->IntrTxGpoRisingEdgeHandler = CallbackFunc;
		InstancePtr->IntrTxGpoRisingEdgeCallbackRef = CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_RX_GPO_RISING_EDGE:
		InstancePtr->IntrRxGpoRisingEdgeHandler = CallbackFunc;
		InstancePtr->IntrRxGpoRisingEdgeCallbackRef = CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_LCPLL_LOCK:
		InstancePtr->IntrLcpllLockHandler = CallbackFunc;
		InstancePtr->IntrLcpllLockCallbackRef = CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_RPLL_LOCK:
		InstancePtr->IntrRpllLockHandler = CallbackFunc;
		InstancePtr->IntrRpllLockCallbackRef = CallbackRef;
		break;
#endif
	case XHDMIPHY1_INTR_HANDLER_TYPE_TX_CLKDET_FREQ_CHANGE:
		InstancePtr->IntrTxClkDetFreqChangeHandler =
							CallbackFunc;
		InstancePtr->IntrTxClkDetFreqChangeCallbackRef =
							CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_RX_CLKDET_FREQ_CHANGE:
		InstancePtr->IntrRxClkDetFreqChangeHandler =
							CallbackFunc;
		InstancePtr->IntrRxClkDetFreqChangeCallbackRef =
							CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_TX_MMCM_LOCK_CHANGE:
		InstancePtr->IntrTxMmcmLockHandler = CallbackFunc;
		InstancePtr->IntrTxMmcmLockCallbackRef = CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_RX_MMCM_LOCK_CHANGE:
		InstancePtr->IntrRxMmcmLockHandler = CallbackFunc;
		InstancePtr->IntrRxMmcmLockCallbackRef = CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_TX_TMR_TIMEOUT:
		InstancePtr->IntrTxTmrTimeoutHandler = CallbackFunc;
		InstancePtr->IntrTxTmrTimeoutCallbackRef = CallbackRef;
		break;
	case XHDMIPHY1_INTR_HANDLER_TYPE_RX_TMR_TIMEOUT:
		InstancePtr->IntrRxTmrTimeoutHandler = CallbackFunc;
		InstancePtr->IntrRxTmrTimeoutCallbackRef = CallbackRef;
		break;
	default:
		break;
	}
}

/******************************************************************************/
/**
 * This function is the interrupt handler for the XHdmiphy1 driver. It will
 * detect what kind of interrupt has happened, and will invoke the appropriate
 * callback function.
 *
 * @param	InstancePtr is a pointer to the XHdmiphy1 instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XHdmiphy1_InterruptHandler(XHdmiphy1 *InstancePtr)
{
	u32 IntrStatus;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Determine what kind of interrupts have occurred. */
	IntrStatus = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_INTR_STS_REG);

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
	if (IntrStatus & XHDMIPHY1_INTR_CPLL_LOCK_MASK) {
		InstancePtr->IntrCpllLockHandler(
				InstancePtr->IntrCpllLockCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_QPLL_LOCK_MASK) {
		InstancePtr->IntrQpllLockHandler(
				InstancePtr->IntrQpllLockCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_QPLL1_LOCK_MASK) {
		InstancePtr->IntrQpll1LockHandler(
				InstancePtr->IntrQpll1LockCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_TXRESETDONE_MASK) {
		InstancePtr->IntrTxResetDoneHandler(
				InstancePtr->IntrTxResetDoneCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_TXALIGNDONE_MASK) {
		InstancePtr->IntrTxAlignDoneHandler(
				InstancePtr->IntrTxAlignDoneCallbackRef);
	}
#else
	if (IntrStatus & XHDMIPHY1_INTR_LCPLL_LOCK_MASK) {
		InstancePtr->IntrLcpllLockHandler(
				InstancePtr->IntrLcpllLockCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_RPLL_LOCK_MASK) {
		InstancePtr->IntrRpllLockHandler(
				InstancePtr->IntrRpllLockCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_TXRESETDONE_MASK) {
		InstancePtr->IntrTxResetDoneHandler(
				InstancePtr->IntrTxResetDoneCallbackRef);
	}
#endif
	if (IntrStatus & XHDMIPHY1_INTR_RXRESETDONE_MASK) {
		InstancePtr->IntrRxResetDoneHandler(
				InstancePtr->IntrRxResetDoneCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_TXCLKDETFREQCHANGE_MASK) {
		InstancePtr->IntrTxClkDetFreqChangeHandler(
				InstancePtr->IntrTxClkDetFreqChangeCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_RXCLKDETFREQCHANGE_MASK) {
		InstancePtr->IntrRxClkDetFreqChangeHandler(
				InstancePtr->IntrRxClkDetFreqChangeCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_TXMMCMUSRCLK_LOCK_MASK) {
		InstancePtr->IntrTxMmcmLockHandler(
				InstancePtr->IntrTxMmcmLockCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_RXMMCMUSRCLK_LOCK_MASK) {
		InstancePtr->IntrRxMmcmLockHandler(
				InstancePtr->IntrRxMmcmLockCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_TXTMRTIMEOUT_MASK) {
		InstancePtr->IntrTxTmrTimeoutHandler(
				InstancePtr->IntrTxTmrTimeoutCallbackRef);
	}
	if (IntrStatus & XHDMIPHY1_INTR_RXTMRTIMEOUT_MASK) {
		InstancePtr->IntrRxTmrTimeoutHandler(
				InstancePtr->IntrRxTmrTimeoutCallbackRef);
	}
}
