/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf_intr.c
* This file contains functions related to Channel Filter interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     12/10/20 Initial version
*       dc     03/25/21 Device tree item name change
*       dc     04/18/21 Update trigger and event handlers
*       dc     04/20/21 Doxygen documentation update
* 1.1   dc     11/26/21 Correct interrupt mask handler api
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/05/21 Align event handlers
*       dc     11/19/21 Update doxygen documentation
* 1.5   dc     09/12/22 Update handling overflow status
*
* </pre>
* @addtogroup dfeccf Overview
* @{
******************************************************************************/
/**
* @cond nocomments
*/

#include "xdfeccf.h"
#include "xdfeccf_hw.h"

/**************************** Macros Definitions ****************************/

/************************** Function Prototypes *****************************/
extern u32 XDfeCcf_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data);
extern u32 XDfeCcf_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data,
			      u32 Val);
/**
* @endcond
*/

/****************************************************************************/
/**
*
* Gets overflow event status.
*
* @param    InstancePtr Pointer to the channel filter instance.
* @param    Status Pointer to a returned overflow event status.
*
****************************************************************************/
void XDfeCcf_GetOverflowStatus(const XDfeCcf *InstancePtr,
			       XDfeCcf_OverflowStatus *Status)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Status != NULL);

	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_OVERFLOW);
	Status->OverflowBeforeGainReal =
		XDfeCcf_RdBitField(XDFECCF_OVERFLOW_BEFORE_GAIN_REAL_WIDTH,
				   XDFECCF_OVERFLOW_BEFORE_GAIN_REAL_OFFSET,
				   Val);
	Status->OverflowBeforeGainImag =
		XDfeCcf_RdBitField(XDFECCF_OVERFLOW_BEFORE_GAIN_IMAG_WIDTH,
				   XDFECCF_OVERFLOW_BEFORE_GAIN_IMAG_OFFSET,
				   Val);
	Status->OverflowAfterGainReal =
		XDfeCcf_RdBitField(XDFECCF_OVERFLOW_AFTER_GAIN_REAL_WIDTH,
				   XDFECCF_OVERFLOW_AFTER_GAIN_REAL_OFFSET,
				   Val);
	Status->OverflowAfterGainImag =
		XDfeCcf_RdBitField(XDFECCF_OVERFLOW_AFTER_GAIN_IMAG_WIDTH,
				   XDFECCF_OVERFLOW_AFTER_GAIN_IMAG_OFFSET,
				   Val);
	Status->OverflowAntenna = XDfeCcf_RdBitField(
		XDFECCF_ANTENNA_WIDTH, XDFECCF_ANTENNA_OFFSET, Val);
	Status->OverflowCCID = XDfeCcf_RdBitField(XDFECCF_CCID_WIDTH,
						  XDFECCF_CCID_OFFSET, Val);
	Status->OverflowSwitch =
		XDfeCcf_RdBitField(XDFECCF_OVERFLOW_SWITCH_WIDTH,
				   XDFECCF_OVERFLOW_SWITCH_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Gets event status.
*
* @param    InstancePtr Pointer to the channel filter instance.
* @param    Status Pointer to a returned event status.
*
****************************************************************************/
void XDfeCcf_GetEventStatus(const XDfeCcf *InstancePtr, XDfeCcf_Status *Status)
{
	u32 Val;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Status != NULL);

	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_ISR);
	Status->Overflow = XDfeCcf_RdBitField(XDFECCF_OVERFLOW_WIDTH,
					      XDFECCF_OVERFLOW_OFFSET, Val);
	Status->CCUpdate =
		XDfeCcf_RdBitField(XDFECCF_CC_UPDATE_TRIGGERED_WIDTH,
				   XDFECCF_CC_UPDATE_TRIGGERED_OFFSET, Val);
	Status->CCSequenceError =
		XDfeCcf_RdBitField(XDFECCF_CC_SEQUENCE_ERROR_WIDTH,
				   XDFECCF_CC_SEQUENCE_ERROR_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Clears events status.
*
* @param    InstancePtr Pointer to the channel filter instance.
* @param    Status Clear event status container.
*           - 0 - does not clear coresponding event status
*           - 1 - clears coresponding event status
*
****************************************************************************/
void XDfeCcf_ClearEventStatus(const XDfeCcf *InstancePtr,
			      const XDfeCcf_Status *Status)
{
	u32 Val = 0U;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Status != NULL);
	Xil_AssertVoid(Status->Overflow <= 1U);
	Xil_AssertVoid(Status->CCUpdate <= 1U);
	Xil_AssertVoid(Status->CCSequenceError <= 1U);

	Val = XDfeCcf_WrBitField(XDFECCF_OVERFLOW_WIDTH,
				 XDFECCF_OVERFLOW_OFFSET, Val,
				 Status->Overflow);
	Val = XDfeCcf_WrBitField(XDFECCF_CC_UPDATE_TRIGGERED_WIDTH,
				 XDFECCF_CC_UPDATE_TRIGGERED_OFFSET, Val,
				 Status->CCUpdate);
	Val = XDfeCcf_WrBitField(XDFECCF_CC_SEQUENCE_ERROR_WIDTH,
				 XDFECCF_CC_SEQUENCE_ERROR_OFFSET, Val,
				 Status->CCSequenceError);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_ISR, Val);
}

/****************************************************************************/
/**
*
* Sets interrupt masks.
*
* @param    InstancePtr Pointer to the channel filter instance.
* @param    Mask Interrupt mask value.
*           - 0 - does not mask coresponding interrupt
*           - 1 - masks coresponding interrupt
*
****************************************************************************/
void XDfeCcf_SetInterruptMask(const XDfeCcf *InstancePtr,
			      const XDfeCcf_InterruptMask *Mask)
{
	u32 ValIER = 0U;
	u32 ValIDR = 0U;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mask != NULL);
	Xil_AssertVoid(Mask->Overflow <= 1U);
	Xil_AssertVoid(Mask->CCUpdate <= 1U);
	Xil_AssertVoid(Mask->CCSequenceError <= 1U);

	if (Mask->Overflow == XDFECCF_IMR_INTERRUPT) {
		ValIER |= (1U << XDFECCF_OVERFLOW_OFFSET);
	} else {
		ValIDR |= (1U << XDFECCF_OVERFLOW_OFFSET);
	}

	if (Mask->CCUpdate == XDFECCF_IMR_INTERRUPT) {
		ValIER |= (1U << XDFECCF_CC_UPDATE_TRIGGERED_OFFSET);
	} else {
		ValIDR |= (1U << XDFECCF_CC_UPDATE_TRIGGERED_OFFSET);
	}

	if (Mask->CCSequenceError == XDFECCF_IMR_INTERRUPT) {
		ValIER |= (1U << XDFECCF_CC_SEQUENCE_ERROR_OFFSET);
	} else {
		ValIDR |= (1U << XDFECCF_CC_SEQUENCE_ERROR_OFFSET);
	}

	XDfeCcf_WriteReg(InstancePtr, XDFECCF_IER, ValIER);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_IDR, ValIDR);
}

/****************************************************************************/
/**
*
* Gets interrupt masks.
*
* @param    InstancePtr Pointer to the channel filter instance.
* @param    Mask Interrupt mask value.
*
*
****************************************************************************/
void XDfeCcf_GetInterruptMask(const XDfeCcf *InstancePtr,
			      XDfeCcf_InterruptMask *Mask)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mask != NULL);

	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_IMR);
	Mask->Overflow = XDfeCcf_RdBitField(XDFECCF_OVERFLOW_WIDTH,
					    XDFECCF_OVERFLOW_OFFSET, Val);
	Mask->CCUpdate =
		XDfeCcf_RdBitField(XDFECCF_CC_UPDATE_TRIGGERED_WIDTH,
				   XDFECCF_CC_UPDATE_TRIGGERED_OFFSET, Val);
	Mask->CCSequenceError =
		XDfeCcf_RdBitField(XDFECCF_CC_SEQUENCE_ERROR_WIDTH,
				   XDFECCF_CC_SEQUENCE_ERROR_OFFSET, Val);
}

/** @} */
