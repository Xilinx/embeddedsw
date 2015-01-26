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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file xdp_intr.c
 *
 * This file contains functions related to XDp interrupt handling.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp.h"

/**************************** Function Prototypes *****************************/

static void XDp_TxInterruptHandler(XDp *InstancePtr);
static void XDp_RxInterruptHandler(XDp *InstancePtr);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the interrupt handler for the XDp driver.
 * When an interrupt happens, this interrupt handler will check which TX/RX mode
 * of operation the core is running in, and will call the appropriate interrupt
 * handler. The called interrupt handler will first detect what kind of
 * interrupt happened, then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_InterruptHandler(XDp *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (XDp_CfgGetCoreType(&InstancePtr->Config) == XDP_TX) {
		XDp_TxInterruptHandler(InstancePtr);
	}
	else {
		XDp_RxInterruptHandler(InstancePtr);
	}
}

/******************************************************************************/
/**
 * This function generates a pulse on the hot-plug-detect (HPD) line of the
 * specified duration.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	DurationUs is the duration of the HPD pulse, in microseconds.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxGenerateHpdInterrupt(XDp *InstancePtr, u16 DurationUs)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_HPD_INTERRUPT,
			(DurationUs << XDP_RX_HPD_INTERRUPT_LENGTH_US_SHIFT) |
			XDP_RX_HPD_INTERRUPT_ASSERT_MASK);
}

/******************************************************************************/
/**
 * This function enables interrupts associated with the specified mask.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Mask specifies which interrupts should be enabled. Bits set to
 *		1 will enable the corresponding interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxInterruptEnable(XDp *InstancePtr, u32 Mask)
{
	u32 MaskVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	MaskVal = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_CAUSE);
	MaskVal &= ~Mask;
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_INTERRUPT_MASK,
								MaskVal);
}

/******************************************************************************/
/**
 * This function disables interrupts associated with the specified mask.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Mask specifies which interrupts should be disabled. Bits set to
 *		1 will disable the corresponding interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxInterruptDisable(XDp *InstancePtr, u32 Mask)
{
	u32 MaskVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	MaskVal = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_CAUSE);
	MaskVal |= Mask;
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_INTERRUPT_MASK,
								MaskVal);
}


/******************************************************************************/
/**
 * This function installs a callback function for when a hot-plug-detect event
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxSetHpdEventHandler(XDp *InstancePtr,
				XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->TxInstance.HpdEventHandler = CallbackFunc;
	InstancePtr->TxInstance.HpdEventCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a hot-plug-detect pulse
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxSetHpdPulseHandler(XDp *InstancePtr,
				XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->TxInstance.HpdPulseHandler = CallbackFunc;
	InstancePtr->TxInstance.HpdPulseCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a video mode change
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrVmChangeHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrVmChangeHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrVmChangeCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when the power state interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrPowerStateHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrPowerStateHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrPowerStateCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a no video interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrNoVideoHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrNoVideoHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrNoVideoCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a vertical blanking
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrVBlankHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrVBlankHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrVBlankCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training lost interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrTrainingLostHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrTrainingLostHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrTrainingLostCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a valid video interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrVideoHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrVideoHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrVideoCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training done interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrTrainingDoneHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrTrainingDoneHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrTrainingDoneCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a bandwidth change
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrBwChangeHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrBwChangeHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrBwChangeCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 1
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrTp1Handler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrTp1Handler = CallbackFunc;
	InstancePtr->RxInstance.IntrTp1CallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 2
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrTp2Handler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrTp2Handler = CallbackFunc;
	InstancePtr->RxInstance.IntrTp2CallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 3
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrTp3Handler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrTp3Handler = CallbackFunc;
	InstancePtr->RxInstance.IntrTp3CallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function is the interrupt handler for the XDp driver operating in TX
 * mode.
 *
 * When an interrupt happens, it first detects what kind of interrupt happened,
 * then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDp_TxInterruptHandler(XDp *InstancePtr)
{
	u32 IntrStatus;
	u8 HpdEventDetected;
	u8 HpdPulseDetected;
	u32 HpdDuration;
	u32 IntrMask;

	/* Determine what kind of interrupt occurred.
	 * Note: XDP_TX_INTERRUPT_STATUS is an RC (read-clear) register. */
	IntrStatus = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_STATUS);
	IntrStatus &= ~XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_MASK);
	IntrMask = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_MASK);

	HpdEventDetected = IntrStatus & XDP_TX_INTERRUPT_STATUS_HPD_EVENT_MASK;
	HpdPulseDetected = IntrStatus &
				XDP_TX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK;

	if (HpdEventDetected) {
		/* Mask interrupts while event handling is taking place. API
		 * will error out in case of a disconnection event anyway. */
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, IntrMask |
			XDP_TX_INTERRUPT_MASK_HPD_EVENT_MASK);

		InstancePtr->TxInstance.HpdEventHandler(
				InstancePtr->TxInstance.HpdEventCallbackRef);
	}
	else if (HpdPulseDetected && XDp_TxIsConnected(InstancePtr)) {
		/* Mask interrupts while event handling is taking place. */
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, IntrMask |
			XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK);

		/* The source device must debounce the incoming HPD signal by
		 * sampling the value at an interval greater than 0.500 ms. An
		 * HPD pulse should be of width 0.5 ms - 1.0 ms. */
		HpdDuration = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_TX_HPD_DURATION);
		if (HpdDuration >= 500) {
			InstancePtr->TxInstance.HpdPulseHandler(
				InstancePtr->TxInstance.HpdPulseCallbackRef);
		}
	}

	/* Unmask previously masked interrupts once handling is done. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_INTERRUPT_MASK,
								IntrMask);
}

/******************************************************************************/
/**
 * This function is the interrupt handler for the XDp driver operating in RX
 * mode.
 *
 * When an interrupt happens, it first detects what kind of interrupt happened,
 * then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDp_RxInterruptHandler(XDp *InstancePtr)
{
	u32 IntrStatus;
	u8 IntrVmChange, IntrPowerState, IntrNoVideo, IntrVBlank,
		IntrTrainingLost, IntrVideo, IntrTrainingDone, IntrBwChange,
		IntrTp1, IntrTp2, IntrTp3;

	/* Determine what kind of interrupt(s) occurred.
	 * Note: XDP_RX_INTERRUPT_CAUSE is an RC (read-clear) register. */
	IntrStatus = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_CAUSE);
	IntrVmChange = (IntrStatus & XDP_RX_INTERRUPT_CAUSE_VM_CHANGE_MASK);
	IntrPowerState = (IntrStatus & XDP_RX_INTERRUPT_CAUSE_POWER_STATE_MASK);
	IntrNoVideo = (IntrStatus & XDP_RX_INTERRUPT_CAUSE_NO_VIDEO_MASK);
	IntrVBlank = (IntrStatus & XDP_RX_INTERRUPT_CAUSE_VBLANK_MASK);
	IntrTrainingLost = (IntrStatus &
				XDP_RX_INTERRUPT_CAUSE_TRAINING_LOST_MASK);
	IntrVideo = (IntrStatus & XDP_RX_INTERRUPT_CAUSE_VIDEO_MASK);
	IntrTrainingDone = (IntrStatus &
				XDP_RX_INTERRUPT_CAUSE_TRAINING_DONE_MASK);
	IntrBwChange = (IntrStatus & XDP_RX_INTERRUPT_CAUSE_BW_CHANGE_MASK);
	IntrTp1 = (IntrStatus & XDP_RX_INTERRUPT_CAUSE_TP1_MASK);
	IntrTp2 = (IntrStatus & XDP_RX_INTERRUPT_CAUSE_TP2_MASK);
	IntrTp3 = (IntrStatus & XDP_RX_INTERRUPT_CAUSE_TP3_MASK);

	/* Training pattern 1 has started. */
	if (IntrTp1) {
		InstancePtr->RxInstance.IntrTp1Handler(
			InstancePtr->RxInstance.IntrTp1CallbackRef);
	}
	/* Training pattern 2 has started. */
	if (IntrTp2) {
		InstancePtr->RxInstance.IntrTp2Handler(
			InstancePtr->RxInstance.IntrTp2CallbackRef);
	}
	/* Training pattern 3 has started. */
	if (IntrTp3) {
		InstancePtr->RxInstance.IntrTp3Handler(
			InstancePtr->RxInstance.IntrTp3CallbackRef);
	}
	/* Training lost - the link has been lost. */
	if (IntrTrainingLost) {
		InstancePtr->RxInstance.IntrTrainingLostHandler(
			InstancePtr->RxInstance.IntrTrainingLostCallbackRef);
	}
	/* The link has been trained. */
	else if (IntrTrainingDone) {
		InstancePtr->RxInstance.IntrTrainingDoneHandler(
			InstancePtr->RxInstance.IntrTrainingDoneCallbackRef);
	}

	/* A change has been detected in the current video transmitted on the
	 * link as indicated by the main stream attributes (MSA) fields. The
	 * horizontal and vertical resolution parameters are monitored for
	 * changes. */
	if (IntrVmChange) {
		InstancePtr->RxInstance.IntrVmChangeHandler(
			InstancePtr->RxInstance.IntrVmChangeCallbackRef);
	}
	/* The VerticalBlanking_Flag in the VB-ID field of the received stream
	 * indicates the start of the vertical blanking interval. */
	if (IntrVBlank) {
		InstancePtr->RxInstance.IntrVBlankHandler(
			InstancePtr->RxInstance.IntrVBlankCallbackRef);
	}
	/* The receiver has detected the no-video flags in the VB-ID field after
	 * active video has been received. */
	if (IntrNoVideo) {
		InstancePtr->RxInstance.IntrNoVideoHandler(
			InstancePtr->RxInstance.IntrNoVideoCallbackRef);
	}
	/* A valid video frame is detected on the main link. */
	else if (IntrVideo) {
		InstancePtr->RxInstance.IntrVideoHandler(
			InstancePtr->RxInstance.IntrVideoCallbackRef);
	}

	/* The transmitter has requested a change in the current power state of
	 * the receiver core. */
	if (IntrPowerState) {
		InstancePtr->RxInstance.IntrPowerStateHandler(
			InstancePtr->RxInstance.IntrPowerStateCallbackRef);
	}
	/* A change in the bandwidth has been detected. */
	if (IntrBwChange) {
		InstancePtr->RxInstance.IntrBwChangeHandler(
			InstancePtr->RxInstance.IntrBwChangeCallbackRef);
	}
}
