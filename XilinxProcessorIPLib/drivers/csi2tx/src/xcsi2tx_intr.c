/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* @file xcsi2tx_intr.c
* @addtogroup csi2tx_v1_0
* @{
*
* This file implements the functions which handle the interrupts and callbacks
* in the CSI2 Tx Controller.
* The callbacks are registered for events which are interrupts clubbed together
* on the basis of the CSI specification.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 sss 07/28/16 Initial release
* 1.1 vsa 02/28/18 Added Frame End Generation feature
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsi2tx.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/


/*************************** Macros Definitions ******************************/


/************************** Function Prototypes ******************************/

/************************* Function Definitions ******************************/

/*****************************************************************************/
/**
* This function will enable the interrupts present in the interrupt mask
* passed onto the function
*
* @param	InstancePtr is the XCsi instance to operate on
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XCsi2Tx_IntrEnable(XCsi2Tx *InstancePtr, u32 Mask)
{
	u32 AllMask = XCSI2TX_IER_ALLINTR_MASK;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->Config.FEGenEnabled) {
		AllMask |= XCSITX_LCSTAT_VC0_IER_MASK;
		AllMask |= XCSITX_LCSTAT_VC1_IER_MASK;
		AllMask |= XCSITX_LCSTAT_VC2_IER_MASK;
		AllMask |= XCSITX_LCSTAT_VC3_IER_MASK;
	}
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & ~AllMask) == 0);

	Mask |= XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
						XCSI2TX_IER_OFFSET);

	XCsi2Tx_WriteReg(InstancePtr->Config.BaseAddr, XCSI2TX_IER_OFFSET,
			Mask & AllMask);
}

/*****************************************************************************/
/**
* This function will disable the interrupts present in the
* interrupt mask passed onto the function
*
* @param	InstancePtr is the XCsi instance to operate on
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XCsi2Tx_IntrDisable(XCsi2Tx *InstancePtr, u32 Mask)
{
	u32 AllMask = XCSI2TX_IER_ALLINTR_MASK;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->Config.FEGenEnabled) {
		AllMask |= XCSITX_LCSTAT_VC0_IER_MASK;
		AllMask |= XCSITX_LCSTAT_VC1_IER_MASK;
		AllMask |= XCSITX_LCSTAT_VC2_IER_MASK;
		AllMask |= XCSITX_LCSTAT_VC3_IER_MASK;
	}
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & ~AllMask) == 0);

	XCsi2Tx_WriteReg(InstancePtr->Config.BaseAddr, XCSI2TX_IER_OFFSET,
			~Mask & AllMask);
}

/*****************************************************************************/
/**
* This function will get the interrupt mask set (enabled) in the CSI2 Tx core
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
* 		Interrupt enable register
*
* @note		None
*
****************************************************************************/
u32 XCsi2Tx_GetIntrEnable(XCsi2Tx *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
						XCSI2TX_IER_OFFSET);

	return Mask;
}

/*****************************************************************************/
/**
* This function will get the list of interrupts pending in the
* Interrupt Status Register of the CSI2 Tx core
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
* 		Interrupt Status register
*
* @note		None
*
****************************************************************************/
u32 XCsi2Tx_GetIntrStatus(XCsi2Tx *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
						XCSI2TX_ISR_OFFSET);

	return Mask;
}

/*****************************************************************************/
/**
* This function will clear the interrupts set in the Interrupt Status
* Register of the CSI2 Tx core
*
* @param	InstancePtr is the XCsi instance to operate on
* @param	Mask is Interrupt Mask with bits set for corresponding interrupt
* 		to be cleared in the Interrupt Status register
*
* @return 	None
*
* @note		None
*
****************************************************************************/
void XCsi2Tx_InterruptClear(XCsi2Tx *InstancePtr, u32 Mask)
{
	u32 AllMask = XCSI2TX_ISR_ALLINTR_MASK;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->Config.FEGenEnabled) {
		AllMask |= XCSITX_LCSTAT_VC0_ISR_MASK;
		AllMask |= XCSITX_LCSTAT_VC1_ISR_MASK;
		AllMask |= XCSITX_LCSTAT_VC2_ISR_MASK;
		AllMask |= XCSITX_LCSTAT_VC3_ISR_MASK;
	}
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & ~AllMask) == 0);

	Mask &= XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
						XCSI2TX_ISR_OFFSET);

	XCsi2Tx_WriteReg(InstancePtr->Config.BaseAddr, XCSI2TX_ISR_OFFSET,
				Mask & AllMask);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType				Callback Function Type
* ----------------------------  --------------------------------------------
* (XCSI2TX_HANDLER_WRG_LANE)		IncorrectLaneCallBack
* (XCSI2TX_HANDLER_GSPFIFO_FULL)	GSPFIFOCallBack
* (XCSI2TX_HANDLER_ULPS)		DPhyUlpsCallBack
* (XCSI2TX_HANDLER_LINEBUF_FULL)	LineBufferCallBack
* (XCSI2TX_HANDLER_WRG_DATATYPE)	WrgDataTypeCallBack
* (XCSI2TX_HANDLER_UNDERRUN_PIXEL)	UnderrunPixelCallBack
* (XCSI2TX_HANDLER_LCERRVC0)		LineCountErrVC0
* (XCSI2TX_HANDLER_LCERRVC1)		LineCountErrVC1
* (XCSI2TX_HANDLER_LCERRVC2)		LineCountErrVC2
* (XCSI2TX_HANDLER_LCERRVC3)		LineCountErrVC3
* </pre>
*
* @param	InstancePtr is the XCsi2Tx instance to operate on
* @param 	HandleType is the type of call back to be registered.
* @param	Callbackfunc is the pointer to a call back funtion which
* 		is called when a particular event occurs.
* @param 	Callbackref is a void pointer to data to be referenced to
* 		by the Callbackfunc
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note 	Invoking this function for a handler that already has been
* 		installed replaces it with the new handler.
*
****************************************************************************/
int XCsi2Tx_SetCallBack(XCsi2Tx *InstancePtr, u32 HandleType,
		void *Callbackfunc, void *Callbackref)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Callbackref != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	switch (HandleType) {
		case XCSI2TX_HANDLER_WRG_LANE:
			InstancePtr->IncorrectLaneCallBack = Callbackfunc;
			InstancePtr->IncorrectLaneRef = Callbackref;
			break;

		case XCSI2TX_HANDLER_GSPFIFO_FULL:
			InstancePtr->GSPFIFOCallBack = Callbackfunc;
			InstancePtr->GSPFIFORef = Callbackref;
			break;

		case XCSI2TX_HANDLER_ULPS:
			InstancePtr->DPhyUlpsCallBack = Callbackfunc;
			InstancePtr->DPhyUlpsRef = Callbackref;
			break;

		case XCSI2TX_HANDLER_LINEBUF_FULL:
			InstancePtr->LineBufferCallBack = Callbackfunc;
			InstancePtr->LineBufferRef = Callbackref;
			break;

		case XCSI2TX_HANDLER_WRG_DATATYPE:
			InstancePtr->WrgDataTypeCallBack = Callbackfunc;
			InstancePtr->WrgDataTypeRef = Callbackref;
			break;

		case XCSI2TX_HANDLER_UNDERRUN_PIXEL:
			InstancePtr->UnderrunPixelCallBack = Callbackfunc;
			InstancePtr->UnderrunPixelRef = Callbackref;
			break;

		case XCSI2TX_HANDLER_LCERRVC0:
			InstancePtr->LineCountErrVC0 = Callbackfunc;
			InstancePtr->LCErrVC0Ref = Callbackref;
			break;

		case XCSI2TX_HANDLER_LCERRVC1:
			InstancePtr->LineCountErrVC1 = Callbackfunc;
			InstancePtr->LCErrVC1Ref = Callbackref;
			break;

		case XCSI2TX_HANDLER_LCERRVC2:
			InstancePtr->LineCountErrVC2 = Callbackfunc;
			InstancePtr->LCErrVC2Ref = Callbackref;
			break;

		case XCSI2TX_HANDLER_LCERRVC3:
			InstancePtr->LineCountErrVC3 = Callbackfunc;
			InstancePtr->LCErrVC3Ref = Callbackref;
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
* This function is the interrupt handler for the CSI2 Tx core.
*
* This handler reads the pending interrupt from the Interrupt Status register,
* determines the source of the interrupts and calls the respective
* callbacks for the interrupts that are enabled in Interrupt Enable register,
* and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this core is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XCsi2Tx_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XCsi2Tx core instance.
*
* @return	None.
*
* @note		Interrupt should be enabled to execute interrupt handler.
*
******************************************************************************/
void XCsi2Tx_IntrHandler(void *InstancePtr)
{
	u32 ActiveIntr;
	u32 Mask;

	XCsi2Tx *XCsiPtr = (XCsi2Tx *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XCsiPtr != NULL);
	Xil_AssertVoid(XCsiPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get Active interrupts */
	ActiveIntr = XCsi2Tx_GetIntrStatus(XCsiPtr);

	/* If Frame End Generation is Enabled then check for
	 * Line Count Status per Virtual Channel.
	 */
	if (XCsiPtr->Config.FEGenEnabled) {
		/* Check if the no. of lines recieved is different from
		 * the value configured in corresponding VC register.
		 * To clear the interrupt, we need to write 0x3 in the
		 * corresponding bitfield of ISR.
		 */
		Mask = ActiveIntr & XCSITX_LCSTAT_VC0_ISR_MASK;
		Mask >>= XCSITX_LCSTAT_VC0_ISR_OFFSET;
		if (Mask == XCSI2TX_LC_LESS_LINES ||
		    Mask == XCSI2TX_LC_MORE_LINES) {
			XCsiPtr->LineCountErrVC0(XCsiPtr->LCErrVC0Ref, Mask);
			ActiveIntr |= XCSITX_LCSTAT_VC0_ISR_MASK;
		}

		Mask = ActiveIntr & XCSITX_LCSTAT_VC1_ISR_MASK;
		Mask >>= XCSITX_LCSTAT_VC1_ISR_OFFSET;
		if (Mask == XCSI2TX_LC_LESS_LINES ||
		    Mask == XCSI2TX_LC_MORE_LINES) {
			XCsiPtr->LineCountErrVC1(XCsiPtr->LCErrVC1Ref, Mask);
			ActiveIntr |= XCSITX_LCSTAT_VC1_ISR_MASK;
		}

		Mask = ActiveIntr & XCSITX_LCSTAT_VC2_ISR_MASK;
		Mask >>= XCSITX_LCSTAT_VC2_ISR_OFFSET;
		if (Mask == XCSI2TX_LC_LESS_LINES ||
		    Mask == XCSI2TX_LC_MORE_LINES) {
			XCsiPtr->LineCountErrVC2(XCsiPtr->LCErrVC2Ref, Mask);
			ActiveIntr |= XCSITX_LCSTAT_VC2_ISR_MASK;
		}

		Mask = ActiveIntr & XCSITX_LCSTAT_VC3_ISR_MASK;
		Mask >>= XCSITX_LCSTAT_VC3_ISR_OFFSET;
		if (Mask == XCSI2TX_LC_LESS_LINES ||
		    Mask == XCSI2TX_LC_MORE_LINES) {
			XCsiPtr->LineCountErrVC3(XCsiPtr->LCErrVC3Ref, Mask);
			ActiveIntr |= XCSITX_LCSTAT_VC3_ISR_MASK;
		}
	}

	Mask = ActiveIntr & XCSI2TX_UNDERRUN_PIXEL_MASK;
	if (Mask) {
		/* If pixel data underrun then call
		 * corresponding callback function */
		XCsiPtr->UnderrunPixelCallBack(XCsiPtr->UnderrunPixelRef, Mask);
	}

	Mask = ActiveIntr & XCSI2TX_WRONG_DATATYPE_MASK;
	if (Mask) {
		/* If Wrong data Interrupts then call corresponding
		 * callback function */
		XCsiPtr->WrgDataTypeCallBack(XCsiPtr->WrgDataTypeRef,	Mask);
	}

	Mask = ActiveIntr & XCSI2TX_LINE_BUFF_FULL_MASK;
	if (Mask) {
		/* If Linebuffer interrupts, then call
		* corresponding callback function */
		XCsiPtr->LineBufferCallBack(XCsiPtr->LineBufferRef, Mask);
	}

	Mask = ActiveIntr & XCSI2TX_DPHY_ULPS_MASK;
	if (Mask) {
		/* Handle DPHY ULPS Errors */
		XCsiPtr->DPhyUlpsCallBack(XCsiPtr->DPhyUlpsRef, Mask);
	}


	Mask = ActiveIntr & XCSI_GPSFIFO_MASK;
	if (Mask) {
		/* Handle GPS FIFO full Errors */
		XCsiPtr->GSPFIFOCallBack(XCsiPtr->GSPFIFORef, Mask);
	}

	Mask = ActiveIntr & XCSI_INCORT_LANE_MASK;
	if (Mask) {
		/* Handle Incorrect Lane config Errors */
		XCsiPtr->IncorrectLaneCallBack(XCsiPtr->IncorrectLaneRef, Mask);
	}

	/* Clear handled interrupt(s) */
	XCsi2Tx_InterruptClear(XCsiPtr, ActiveIntr);

	return;
}
/** @} */
