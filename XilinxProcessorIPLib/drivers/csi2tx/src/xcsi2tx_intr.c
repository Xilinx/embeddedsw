/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi2tx_intr.c
* @addtogroup csi2tx Overview
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
* 1.2 ai  09/17/26 Added multi-VC (up to 16) line count interrupt handling
*                  with banked ISR sweep and VC4-VC15 callbacks for the
*                  CSI2 TX subsystem v3.0 IP
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
		/* VC4-VC7 (and, in bank 1, VC12-VC15) line count enables */
		AllMask |= XCSITX_LCSTAT_VC4_7_IER_MASK;
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
		/* VC4-VC7 (and, in bank 1, VC12-VC15) line count enables */
		AllMask |= XCSITX_LCSTAT_VC4_7_IER_MASK;
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
		/* VC4-VC7 (and, in bank 1, VC12-VC15) line count status */
		AllMask |= XCSITX_LCSTAT_VC4_7_ISR_MASK;
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
* (XCSI2TX_HANDLER_LCERRVC0..VC15)	LineCountErrVC[0] .. LineCountErrVC[15]
* </pre>
*
* @param	InstancePtr is the XCsi2Tx instance to operate on
* @param 	HandleType is the type of call back to be registered.
* @param	Callbackfunc is the pointer to a call back function which
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

	/* The per-VC Line Count Status error handlers use contiguous handler
	 * IDs (VC0 to VC15) and are stored in an array indexed by VC, so map
	 * them directly instead of one case per VC.
	 */
	if ((HandleType >= XCSI2TX_HANDLER_LCERRVC0) &&
	    (HandleType <= XCSI2TX_HANDLER_LCERRVC15)) {
		u32 Vc = HandleType - XCSI2TX_HANDLER_LCERRVC0;

		InstancePtr->LineCountErrVC[Vc] = Callbackfunc;
		InstancePtr->LCErrVCRef[Vc] = Callbackref;
		return XST_SUCCESS;
	}

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

		default:
			/* Invalid value of HandleType */
			return XST_INVALID_PARAM;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This static helper processes the Line Count Status for one bank of up to
* eight Virtual Channels. Each VC line count status is a 2-bit field; within a
* bank VC0..VC7 occupy ISR bits [9:8], [11:10], ... [23:22]. For every VC that
* reports a less/more lines condition the corresponding callback is invoked and
* the bit position is accumulated into the returned mask so the caller can
* clear the w1c status bits.
*
* @param	XCsiPtr is a pointer to the XCsi2Tx core instance.
* @param	BankIntr is the Interrupt Status Register value for the bank.
* @param	BankBaseVc is the physical VC of the bank's first channel
*		(0 for bank 0, XCSI2TX_VC_PER_BANK for bank 1).
*
* @return	Mask of the w1c Line Count status bits that were handled and
*		should be cleared for this bank.
*
* @note		None.
*
****************************************************************************/
static u32 Csi2Tx_HandleLcBank(XCsi2Tx *XCsiPtr, u32 BankIntr, u8 BankBaseVc)
{
	u32 ClearMask = 0;
	u32 Shift;
	u32 Status;
	u8 Local;
	u8 Vc;

	for (Local = 0U; Local < XCSI2TX_VC_PER_BANK; Local++) {
		Shift = XCSITX_LCSTAT_VC0_ISR_OFFSET + ((u32)Local * 2U);
		Status = (BankIntr >> Shift) & 0x3U;
		if (Status == XCSI2TX_LC_LESS_LINES ||
		    Status == XCSI2TX_LC_MORE_LINES) {
			Vc = BankBaseVc + Local;
			XCsiPtr->LineCountErrVC[Vc](XCsiPtr->LCErrVCRef[Vc],
						   Status);
			ClearMask |= (0x3U << Shift);
		}
	}

	return ClearMask;
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
	u32 BankIntr;
	u32 ClearMask;

	XCsi2Tx *XCsiPtr = (XCsi2Tx *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XCsiPtr != NULL);
	Xil_AssertVoid(XCsiPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get Active interrupts (bank 0) */
	ActiveIntr = XCsi2Tx_GetIntrStatus(XCsiPtr);

	/* If Frame End Generation is Enabled then check for Line Count Status
	 * per Virtual Channel. The Line Count status fields are banked: bank 0
	 * maps VC0-VC7 and bank 1 maps VC8-VC15. Bank 1 only exists on
	 * NUM_VC=16 cores.
	 */
	if (XCsiPtr->Config.FEGenEnabled) {
		/* Bank 0 (VC0-VC7) from the snapshot read above */
		ClearMask = Csi2Tx_HandleLcBank(XCsiPtr, ActiveIntr, 0U);
		if (ClearMask != 0U) {
			/* ISR[6] and the base interrupt bits are 0 in ClearMask,
			 * so only the bank-0 LC status bits are cleared.
			 */
			XCsi2Tx_WriteReg(XCsiPtr->Config.BaseAddr,
					 XCSI2TX_ISR_OFFSET, ClearMask);
		}

		/* Select bank 1 with a direct write (does not disturb w1c bits)
		 * and confirm the core actually supports it via read-back.
		 */
		XCsi2Tx_WriteReg(XCsiPtr->Config.BaseAddr, XCSI2TX_ISR_OFFSET,
				 XCSI2TX_ISR_VCBANKSEL_MASK);
		if (XCsi2Tx_GetVcIsrBank(XCsiPtr)) {
			BankIntr = XCsi2Tx_GetIntrStatus(XCsiPtr);
			ClearMask = Csi2Tx_HandleLcBank(XCsiPtr, BankIntr,
							XCSI2TX_VC_PER_BANK);
			if (ClearMask != 0U) {
				/* Keep ISR[6]=1 in the write so the w1c clear
				 * targets bank 1.
				 */
				XCsi2Tx_WriteReg(XCsiPtr->Config.BaseAddr,
						 XCSI2TX_ISR_OFFSET,
						 ClearMask |
						 XCSI2TX_ISR_VCBANKSEL_MASK);
			}

			/* Restore bank 0 (direct write, no w1c clears) */
			XCsi2Tx_WriteReg(XCsiPtr->Config.BaseAddr,
					 XCSI2TX_ISR_OFFSET, 0U);
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

	/* Clear handled base interrupt(s). The per-VC line count status bits
	 * were already cleared per bank in the loop above.
	 */
	XCsi2Tx_InterruptClear(XCsiPtr, ActiveIntr & XCSI2TX_ISR_ALLINTR_MASK);

	return;
}
/** @} */
