/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf_intr.c
* @addtogroup dfeccf_v1_2
* @{
* @cond nocomments
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
*
* </pre>
* @endcond
******************************************************************************/

#include "xdfeccf.h"
#include "xdfeccf_hw.h"

/**************************** Macros Definitions ****************************/

/************************** Function Prototypes *****************************/
/**
* @cond nocomments
*/
extern u32 XDfeCcf_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data);
extern u32 XDfeCcf_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data,
			      u32 Val);
/**
* @endcond
*/

/****************************************************************************/
/**
*
* Gets event status.
*
* @param    InstancePtr is a pointer to the channel filter instance.
* @param    Status is a pointer to a returned event status.
*
****************************************************************************/
void XDfeCcf_GetEventStatus(const XDfeCcf *InstancePtr, XDfeCcf_Status *Status)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Status != NULL);

	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_ISR);
	Status->CCUpdate =
		XDfeCcf_RdBitField(XDFECCF_CC_UPDATE_TRIGGERED_WIDTH,
				   XDFECCF_CC_UPDATE_TRIGGERED_OFFSET, Val);
	Status->CCSequenceError =
		XDfeCcf_RdBitField(XDFECCF_CC_SEQUENCE_ERROR_WIDTH,
				   XDFECCF_CC_SEQUENCE_ERROR_OFFSET, Val);

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
}

/****************************************************************************/
/**
*
* Clears all event statuses.
*
* @param    InstancePtr is a pointer to the channel filter instance.
*
****************************************************************************/
void XDfeCcf_ClearEventStatus(const XDfeCcf *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XDfeCcf_WriteReg(InstancePtr, XDFECCF_ISR, XDFECCF_IRQ_FLAGS_MASK);
}

/****************************************************************************/
/**
*
* Sets interrupt masks.
*
* @param    InstancePtr is a pointer to the channel filter instance.
* @param    Mask is an interrupt mask value.
*
****************************************************************************/
void XDfeCcf_SetInterruptMask(const XDfeCcf *InstancePtr,
			      const XDfeCcf_InterruptMask *Mask)
{
	u32 ValIER = 0U;
	u32 ValIDR = 0U;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mask != NULL);

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

/** @} */
