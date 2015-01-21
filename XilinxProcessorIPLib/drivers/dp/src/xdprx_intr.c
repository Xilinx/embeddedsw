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
 * @file xdprx_intr.c
 *
 * This file contains functions related to XDprx interrupt handling.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/14 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdprx.h"

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the interrupt handler for the XDprx driver.
 *
 * When an interrupt happens, it first detects what kind of interrupt happened,
 * then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_InterruptHandler(XDprx *InstancePtr)
{
	u32 IntrStatus;
	u8 IntrVmChange, IntrPowerState, IntrNoVideo, IntrVBlank,
		IntrTrainingLost, IntrVideo, IntrTrainingDone, IntrBwChange,
		IntrTp1, IntrTp2, IntrTp3;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Determine what kind of interrupt(s) occurred.
	 * Note: XDPRX_INTERRUPT_CAUSE is an RC (read-clear) register. */
	IntrStatus = XDprx_ReadReg(InstancePtr->Config.BaseAddr,
							XDPRX_INTERRUPT_CAUSE);
	IntrVmChange = (IntrStatus & XDPRX_INTERRUPT_CAUSE_VM_CHANGE_MASK);
	IntrPowerState = (IntrStatus & XDPRX_INTERRUPT_CAUSE_POWER_STATE_MASK);
	IntrNoVideo = (IntrStatus & XDPRX_INTERRUPT_CAUSE_NO_VIDEO_MASK);
	IntrVBlank = (IntrStatus & XDPRX_INTERRUPT_CAUSE_VBLANK_MASK);
	IntrTrainingLost = (IntrStatus &
				XDPRX_INTERRUPT_CAUSE_TRAINING_LOST_MASK);
	IntrVideo = (IntrStatus & XDPRX_INTERRUPT_CAUSE_VIDEO_MASK);
	IntrTrainingDone = (IntrStatus &
				XDPRX_INTERRUPT_CAUSE_TRAINING_DONE_MASK);
	IntrBwChange = (IntrStatus & XDPRX_INTERRUPT_CAUSE_BW_CHANGE_MASK);
	IntrTp1 = (IntrStatus & XDPRX_INTERRUPT_CAUSE_TP1_MASK);
	IntrTp2 = (IntrStatus & XDPRX_INTERRUPT_CAUSE_TP2_MASK);
	IntrTp3 = (IntrStatus & XDPRX_INTERRUPT_CAUSE_TP3_MASK);

	/* Training pattern 1 has started. */
	if (IntrTp1) {
		InstancePtr->IntrTp1Handler(InstancePtr->IntrTp1CallbackRef);
	}
	/* Training pattern 2 has started. */
	if (IntrTp2) {
		InstancePtr->IntrTp2Handler(InstancePtr->IntrTp2CallbackRef);
	}
	/* Training pattern 3 has started. */
	if (IntrTp3) {
		InstancePtr->IntrTp3Handler(InstancePtr->IntrTp3CallbackRef);
	}
	/* Training lost - the link has been lost. */
	if (IntrTrainingLost) {
		InstancePtr->IntrTrainingLostHandler(
				InstancePtr->IntrTrainingLostCallbackRef);
	}
	/* The link has been trained. */
	else if (IntrTrainingDone) {
		InstancePtr->IntrTrainingDoneHandler(
				InstancePtr->IntrTrainingDoneCallbackRef);
	}

	/* A change has been detected in the current video transmitted on the
	 * link as indicated by the main stream attributes (MSA) fields. The
	 * horizontal and vertical resolution parameters are monitored for
	 * changes. */
	if (IntrVmChange) {
		InstancePtr->IntrVmChangeHandler(
					InstancePtr->IntrVmChangeCallbackRef);
	}
	/* The VerticalBlanking_Flag in the VB-ID field of the received stream
	 * indicates the start of the vertical blanking interval. */
	if (IntrVBlank) {
		InstancePtr->IntrVBlankHandler(
					InstancePtr->IntrVBlankCallbackRef);
	}
	/* The receiver has detected the no-video flags in the VB-ID field after
	 * active video has been received. */
	if (IntrNoVideo) {
		InstancePtr->IntrNoVideoHandler(
					InstancePtr->IntrNoVideoCallbackRef);
	}
	/* A valid video frame is detected on the main link. */
	else if (IntrVideo) {
		InstancePtr->IntrVideoHandler(
					InstancePtr->IntrVideoCallbackRef);
	}

	/* The transmitter has requested a change in the current power state of
	 * the receiver core. */
	if (IntrPowerState) {
		InstancePtr->IntrPowerStateHandler(
					InstancePtr->IntrPowerStateCallbackRef);
	}
	/* A change in the bandwidth has been detected. */
	if (IntrBwChange) {
		InstancePtr->IntrBwChangeHandler(
					InstancePtr->IntrBwChangeCallbackRef);
	}
}

/******************************************************************************/
/**
 * This function generates a pulse on the hot-plug-detect (HPD) line of the
 * specified duration.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	DurationUs is the duration of the HPD pulse, in microseconds.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_GenerateHpdInterrupt(XDprx *InstancePtr, u16 DurationUs)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XDprx_WriteReg(InstancePtr->Config.BaseAddr, XDPRX_HPD_INTERRUPT,
			(DurationUs << XDPRX_HPD_INTERRUPT_LENGTH_US_SHIFT) |
			XDPRX_HPD_INTERRUPT_ASSERT_MASK);
}

/******************************************************************************/
/**
 * This function enables interrupts associated with the specified mask.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	Mask specifies which interrupts should be enabled. Bits set to
 *		1 will enable the corresponding interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_InterruptEnable(XDprx *InstancePtr, u32 Mask)
{
	u32 MaskVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	MaskVal = XDprx_ReadReg(InstancePtr->Config.BaseAddr,
							XDPRX_INTERRUPT_CAUSE);
	MaskVal &= ~Mask;
	XDprx_WriteReg(InstancePtr->Config.BaseAddr, XDPRX_INTERRUPT_MASK,
								MaskVal);
}

/******************************************************************************/
/**
 * This function disables interrupts associated with the specified mask.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	Mask specifies which interrupts should be disabled. Bits set to
 *		1 will disable the corresponding interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_InterruptDisable(XDprx *InstancePtr, u32 Mask)
{
	u32 MaskVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	MaskVal = XDprx_ReadReg(InstancePtr->Config.BaseAddr,
							XDPRX_INTERRUPT_CAUSE);
	MaskVal |= Mask;
	XDprx_WriteReg(InstancePtr->Config.BaseAddr, XDPRX_INTERRUPT_MASK,
								MaskVal);
}

/******************************************************************************/
/**
 * This function installs a callback function for when a video mode change
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrVmChangeHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrVmChangeHandler = CallbackFunc;
	InstancePtr->IntrVmChangeCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when the power state interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrPowerStateHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrPowerStateHandler = CallbackFunc;
	InstancePtr->IntrPowerStateCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a no video interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrNoVideoHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrNoVideoHandler = CallbackFunc;
	InstancePtr->IntrNoVideoCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a vertical blanking
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrVBlankHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrVBlankHandler = CallbackFunc;
	InstancePtr->IntrVBlankCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training lost interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrTrainingLostHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrTrainingLostHandler = CallbackFunc;
	InstancePtr->IntrTrainingLostCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a valid video interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrVideoHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrVideoHandler = CallbackFunc;
	InstancePtr->IntrVideoCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training done interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrTrainingDoneHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrTrainingDoneHandler = CallbackFunc;
	InstancePtr->IntrTrainingDoneCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a bandwidth change
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrBwChangeHandler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrBwChangeHandler = CallbackFunc;
	InstancePtr->IntrBwChangeCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 1
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrTp1Handler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrTp1Handler = CallbackFunc;
	InstancePtr->IntrTp1CallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 2
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrTp2Handler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrTp2Handler = CallbackFunc;
	InstancePtr->IntrTp2CallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 3
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDprx_SetIntrTp3Handler(XDprx *InstancePtr,
			XDprx_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->IntrTp3Handler = CallbackFunc;
	InstancePtr->IntrTp3CallbackRef = CallbackRef;
}
