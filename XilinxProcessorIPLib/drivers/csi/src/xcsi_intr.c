/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi_intr.c
* @addtogroup csi_v1_5
* @{
*
* This file implements the functions which handle the interrupts and callbacks
* in the CSI2 Rx Controller.
* The callbacks are registered for events which are interrupts clubbed together
* on the basis of the CSI specification.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 07/28/15 Initial release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xcsi.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/************************** Macros Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions ******************************/

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
void XCsi_IntrEnable(XCsi *InstancePtr, u32 Mask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XCSI_IER_ALLINTR_MASK))) == 0);

	Mask |= XCsi_ReadReg(InstancePtr->Config.BaseAddr, XCSI_IER_OFFSET);

	XCsi_WriteReg(InstancePtr->Config.BaseAddr,XCSI_IER_OFFSET,
			Mask & XCSI_IER_ALLINTR_MASK);
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
void XCsi_IntrDisable(XCsi *InstancePtr, u32 Mask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XCSI_IER_ALLINTR_MASK))) == 0);

	XCsi_WriteReg(InstancePtr->Config.BaseAddr, XCSI_IER_OFFSET,
			~ Mask & XCSI_IER_ALLINTR_MASK);
}

/*****************************************************************************/
/**
* This function will get the interrupt mask set (enabled) in the CSI2 Rx core
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
* 		Interrupt enable register
*
* @note		None
*
****************************************************************************/
u32 XCsi_GetIntrEnable(XCsi *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XCsi_ReadReg(InstancePtr->Config.BaseAddr, XCSI_IER_OFFSET);

	return Mask;
}

/*****************************************************************************/
/**
* This function will get the list of interrupts pending in the
* Interrupt Status Register of the CSI2 Rx core
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
* 		Interrupt Status register
*
* @note		None
*
****************************************************************************/
u32 XCsi_GetIntrStatus(XCsi *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XCsi_ReadReg(InstancePtr->Config.BaseAddr, XCSI_ISR_OFFSET);

	return Mask;
}

/*****************************************************************************/
/**
* This function will clear the interrupts set in the Interrupt Status
* Register of the CSI2 Rx core
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
void XCsi_InterruptClear(XCsi *InstancePtr, u32 Mask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XCSI_IER_ALLINTR_MASK))) == 0);

	Mask &= XCsi_ReadReg(InstancePtr->Config.BaseAddr, XCSI_ISR_OFFSET);

	XCsi_WriteReg(InstancePtr->Config.BaseAddr, XCSI_ISR_OFFSET,
				Mask & XCSI_ISR_ALLINTR_MASK);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType			Callback Function Type
* ----------------------------  --------------------------------------------
* (XCSI_HANDLER_DPHY)		DPhyLvlErrCallBack
* (XCSI_HANDLER_PROTLVL)	ProtDecodeErrCallBack
* (XCSI_HANDLER_PKTLVL)		PktLvlErrCallBack
* (XCSI_HANDLER_SHORTPACKET)	ShortPacketCallBack
* (XCSI_HANDLER_FRAMERECVD)	FrameRecvdCallBack
* (XCSI_HANDLER_VCXERR)		VCXErrCallBack
* (XCSI_HANDLER_OTHERERROR)	ErrorCallBack
* </pre>
*
* @param	InstancePtr is the XCsi instance to operate on
* @param 	HandleType is the type of call back to be registered.
* @param	Callbackfunc is the pointer to a call back funtion which
* 		is called when a particular event occurs.
* @param 	Callbackref is a void pointer to data to be referenced to
* 		by the Callbackfunc
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
		void *Callbackfunc, void *Callbackref)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Callbackref != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	switch (HandleType) {
		case XCSI_HANDLER_DPHY:
			InstancePtr->DPhyLvlErrCallBack = Callbackfunc;
			InstancePtr->DPhyLvlErrRef = Callbackref;
			break;

		case XCSI_HANDLER_PROTLVL:
			InstancePtr->ProtDecodeErrCallBack = Callbackfunc;
			InstancePtr->ProtDecErrRef = Callbackref;
			break;

		case XCSI_HANDLER_PKTLVL:
			InstancePtr->PktLvlErrCallBack = Callbackfunc;
			InstancePtr->PktLvlErrRef = Callbackref;
			break;

		case XCSI_HANDLER_SHORTPACKET:
			InstancePtr->ShortPacketCallBack = Callbackfunc;
			InstancePtr->ShortPacketRef = Callbackref;
			break;

		case XCSI_HANDLER_FRAMERECVD:
			InstancePtr->FrameRecvdCallBack = Callbackfunc;
			InstancePtr->FrameRecvdRef = Callbackref;
			break;

		case XCSI_HANDLER_VCXERR:
			InstancePtr->VCXErrCallBack = Callbackfunc;
			InstancePtr->VCXErrRef = Callbackref;

			break;
		case XCSI_HANDLER_OTHERERROR:
			InstancePtr->ErrorCallBack = Callbackfunc;
			InstancePtr->ErrRef = Callbackref;
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
* This function is the interrupt handler for the CSI2 Rx core.
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
	u32 ActiveIntr;
	u32 Mask;

	XCsi *XCsiPtr = (XCsi *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XCsiPtr != NULL);
	Xil_AssertVoid(XCsiPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get Active interrupts */
	ActiveIntr = XCsi_GetIntrStatus(XCsiPtr) & XCsi_GetIntrEnable(XCsiPtr);

	Mask = ActiveIntr & XCSI_INTR_FRAMERCVD_MASK;
	if (Mask) {
		/* If Frame received then call corresponding callback function */
		XCsiPtr->FrameRecvdCallBack(XCsiPtr->FrameRecvdRef, Mask);
	}

	Mask = ActiveIntr & XCSI_INTR_ERR_MASK;
	if (Mask) {
		/* If ShortPacket Interrupts then call corresponding
		 * callback function */
		XCsiPtr->ErrorCallBack(XCsiPtr->ErrRef,	Mask);
	}

	Mask = ActiveIntr & XCSI_INTR_SPKT_MASK;
	if (Mask) {
		/* If ShortPacket Interrupts then call corresponding
		 * callback function */
		XCsiPtr->ShortPacketCallBack(XCsiPtr->ShortPacketRef, Mask);
	}

	Mask = ActiveIntr & XCSI_INTR_DPHY_MASK;
	if (Mask) {
		/* Handle DPHY Level Errors */
		XCsiPtr->DPhyLvlErrCallBack(XCsiPtr->DPhyLvlErrRef, Mask);
	}

	Mask = ActiveIntr & XCSI_INTR_VCXFE_MASK;
	if (Mask) {
		/* Handle VCx Errors */
		Mask = XCsi_ReadReg(XCsiPtr->Config.BaseAddr,
				    XCSI_VCX_FE_OFFSET);
		Mask &= XCSI_INTR_VCFE_MASK;
		XCsiPtr->VCXErrCallBack(XCsiPtr->VCXErrRef, Mask);
		XCsi_WriteReg(XCsiPtr->Config.BaseAddr, XCSI_VCX_FE_OFFSET, Mask);
	}

	Mask = ActiveIntr & XCSI_INTR_PROT_MASK;
	if (Mask) {
		/* Handle Protocol Decoding Level Errors */
		XCsiPtr->ProtDecodeErrCallBack(XCsiPtr->ProtDecErrRef, Mask);
	}

	Mask = ActiveIntr & XCSI_INTR_PKTLVL_MASK;
	if (Mask) {
		/* Handle Packet Level Errors */
		XCsiPtr->PktLvlErrCallBack(XCsiPtr->PktLvlErrRef, Mask);
	}

	/* Clear handled interrupt(s) */
	XCsi_InterruptClear(XCsiPtr, ActiveIntr);

	return;
}
/** @} */
