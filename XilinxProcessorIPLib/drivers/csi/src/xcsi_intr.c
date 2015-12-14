/******************************************************************************
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
/*****************************************************************************/
/**
*
* @file xcsi_intr.c
*
* This file implements the functions which handle the interrupts in the CSI
* Controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a vs   07/28/15 First release
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xcsi_hw.h"
#include "xcsi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Macros Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* XCsi_InterruptEnable will enable the interrupts present in the interrupt mask
* passed onto the function
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
*
****************************************************************************/
void XCsi_InterruptEnable(XCsi *InstancePtr, u32 Mask)
{
	u32 Temp;

	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XCSI_IER_ALLINTR_MASK))) == 0);

	Mask |= XCsi_GetIntrEnable(InstancePtr);

	XCsi_IntrEnable(InstancePtr, Mask);
}

/*****************************************************************************/
/**
* XCsi_InterruptDisable will disable the interrupts present in the
* interrupt mask passed onto the function
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
*
****************************************************************************/
void XCsi_InterruptDisable(XCsi *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XCSI_IER_ALLINTR_MASK))) == 0);

	XCsi_IntrDisable(InstancePtr,
		((~(Mask)) & (XCsi_GetIntrEnable(InstancePtr))));
}

/*****************************************************************************/
/**
* XCsi_InterruptGetEnabled will get the interrupt mask set (enabled) in the
* CSI core
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
* 		Interrupt enable register
*
*
****************************************************************************/
u32 XCsi_InterruptGetEnabled(XCsi *InstancePtr)
{
	u32 Mask;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XCsi_GetIntrEnable(InstancePtr);

	return Mask;
}

/*****************************************************************************/
/**
* XCsi_InterruptGetStatus will get the list of interrupts pending in the
* Interrupt Status Register of the CSI core
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
* 		Interrupt Status register
*
*
****************************************************************************/
u32 XCsi_InterruptGetStatus(XCsi *InstancePtr)
{
	u32 Mask;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XCsi_IntrGetIrq(InstancePtr);

	return Mask;
}


/*****************************************************************************/
/**
* XCsi_InterruptClear will clear the interrupts set in the Interrupt Status
* Register of the CSI core
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @param	Mask is Interrupt Mask with bits set for corresponding interrupt
* 		to be cleared in the Interrupt Status register
*
* @return 	None
*
****************************************************************************/
void XCsi_InterruptClear(XCsi *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XCSI_IER_ALLINTR_MASK))) == 0);

	Mask &= XCsi_IntrGetIrq(InstancePtr);

	XCsi_IntrAckIrq(InstancePtr, Mask);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
*
* HandlerType              Invoked by this driver when:
* -----------------------  --------------------------------------------------
* XCSI_HANDLER_DPHY        A DPHY Level Error has been detected.
*
* XCSI_HANDLER_PROTLVL     A Protocol Level Error has been detected.
*
* XCSI_HANDLER_PKTLVL      A Packet Level Error has been detected.
*
* XCSI_HANDLER_SHORTPACKET A Short packet has been received or the
* 			   Short Packet FIFO is full.
*
* XCSI_HANDLER_FRAMERECVD  A Frame has been received
*
* XCSI_HANDLER_OTHERERROR  Any other type of interrupt has occured like
* 			   Stream Line Buffer Full, Incorrect Lanes, etc
*
* </pre>
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @param 	HandleType is the type of call back to be registered.
*
* @param	CallBackFunc is the pointer to a call back funtion which
* 		is called when a particular event occurs.
*
* @param 	CallBackRef is a void pointer to data to be referenced to
* 		by the CallBackFunc
*
* @return
* 		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note 	Invoking this function for a handler that already has been
* 		installed replaces it with the new handler.
*
****************************************************************************/
int XCsi_SetCallBack(XCsi *InstancePtr, u32 HandleType,
		void *CallBackFunc, void *CallBackRef)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	switch (HandleType) {
		case XCSI_HANDLER_DPHY:
			InstancePtr->DPhyLvlErrCallBack = CallBackFunc;
			InstancePtr->DPhyLvlErrRef = CallBackRef;
			break;
		case XCSI_HANDLER_PROTLVL:
			InstancePtr->ProtDecodeErrCallBack = CallBackFunc;
			InstancePtr->ProtDecErrRef = CallBackRef;
			break;
		case XCSI_HANDLER_PKTLVL:
			InstancePtr->PktLvlErrCallBack = CallBackFunc;
			InstancePtr->PktLvlErrRef = CallBackRef;
			break;
		case XCSI_HANDLER_SHORTPACKET:
			InstancePtr->ShortPacketCallBack = CallBackFunc;
			InstancePtr->ShortPacketRef = CallBackRef;
			break;
		case XCSI_HANDLER_FRAMERECVD:
			InstancePtr->FrameRecvdCallBack = CallBackFunc;
			InstancePtr->FrameRecvdRef = CallBackRef;
			break;
		case XCSI_HANDLER_OTHERERROR:
			InstancePtr->ErrorCallBack = CallBackFunc;
			InstancePtr->ErrRef = CallBackRef;
			break;
		default:
			/* Invalid value of HandleType */
			Xil_AssertVoidAlways();
			return XST_INVALID_PARAM;
	}

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function is the interrupt handler for the CSI core.
*
* This handler reads the pending interrupt from the Interrupt Status register,
* determines the source of the interrupts and calls the respective
* callbacks for the interrupts that are enabled in Interrupt Enable register,
* and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this core is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XCsi_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XCsi core instance.
*
* @return	None.
*
* @note		Interrupt should be enabled to execute interrupt handler.
*
******************************************************************************/
void XCsi_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 ErrorStatus;
	u32 Mask;

	XCsi *XCsiPtr = (XCsi *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XCsiPtr != NULL);
	Xil_AssertVoid(XCsiPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get pending interrupts */
	PendingIntr = XCsi_InterruptGetStatus(XCsiPtr);

	Mask = PendingIntr & XCSI_INTR_FRAMERCVD_MASK;
	if (Mask) {
		/* If Frame received then call corresponding callback function */
		XCsiPtr->FrameRecvdCallBack(XCsiPtr->FrameRecvdRef, Mask);
	}

	Mask = PendingIntr & XCSI_INTR_ERR_MASK;
	if (Mask) {
		/* If ShortPacket Interrupts then call corresponding
		 * callback function */
		XCsiPtr->ErrorCallBack(XCsiPtr->ErrRef,	Mask);
	}

	Mask = PendingIntr & XCSI_INTR_SPKT_MASK;
	if (Mask) {
		/* If ShortPacket Interrupts then call corresponding
		 * callback function */
		XCsiPtr->ShortPacketCallBack(XCsiPtr->ShortPacketRef, Mask);
	}

	Mask = PendingIntr & XCSI_INTR_DPHY_MASK;
	if (Mask) {
		/* Handle DPHY Level Errors */
		XCsiPtr->DPhyLvlErrCallBack(XCsiPtr->DPhyLvlErrRef, Mask);
	}

	Mask = PendingIntr & XCSI_INTR_PROT_MASK;
	if (Mask) {
		/* Handle Protocol Decoding Level Errors */
		XCsiPtr->ProtDecodeErrCallBack(XCsiPtr->ProtDecErrRef, Mask);
	}

	Mask = PendingIntr & XCSI_INTR_PKTLVL_MASK;
	if (Mask) {
		/* Handle Packet Level Errors */
		XCsiPtr->PktLvlErrCallBack(XCsiPtr->PktLvlErrRef, Mask);
	}

	/* Clear pending interrupt(s) */
	XCsi_InterruptClear(XCsiPtr, PendingIntr);

	return;
}
