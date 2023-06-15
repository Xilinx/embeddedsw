/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeofdm_intr.c
* This file contains functions related to Orthogonal Frequency Division
* Multiplexing interrupt status handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     11/21/22 Initial version
* 1.1   dc     05/22/23 State and status upgrades
*
* </pre>
* @addtogroup dfeofdm Overview
* @{
******************************************************************************/
/**
* @cond nocomments
*/

#include "xdfeofdm.h"
#include "xdfeofdm_hw.h"

/**************************** Macros Definitions ****************************/

/************************** Function Prototypes *****************************/
extern u32 XDfeOfdm_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data);
extern u32 XDfeOfdm_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data,
			       u32 Val);
/**
* @endcond
*/

/****************************************************************************/
/**
*
* Gets event status.
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    Status Pointer to a returned event status.
*
****************************************************************************/
void XDfeOfdm_GetEventStatus(const XDfeOfdm *InstancePtr,
			     XDfeOfdm_Status *Status)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Status != NULL);

	Val = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_STATUS_ISR_OFFSET);
	Status->CCUpdate =
		XDfeOfdm_RdBitField(XDFEOFDM_STATUS_CC_UPDATE_TRIGGERED_WIDTH,
				    XDFEOFDM_STATUS_CC_UPDATE_TRIGGERED_OFFSET,
				    Val);
	Status->FTCCSequenceError =
		XDfeOfdm_RdBitField(XDFEOFDM_STATUS_FT_CC_SEQUENCE_ERROR_WIDTH,
				    XDFEOFDM_STATUS_FT_CC_SEQUENCE_ERROR_OFFSET,
				    Val);
	Status->Saturation =
		XDfeOfdm_RdBitField(XDFEOFDM_STATUS_BF_SATURATION_WIDTH,
				    XDFEOFDM_STATUS_BF_SATURATION_OFFSET, Val);
	Status->Overflow =
		XDfeOfdm_RdBitField(XDFEOFDM_STATUS_BF_OVERFLOW_WIDTH,
				    XDFEOFDM_STATUS_BF_OVERFLOW_OFFSET, Val);

	Val = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_STATUS_SATURATION_OFFSET);
	Status->SaturationCCID =
		XDfeOfdm_RdBitField(XDFEOFDM_STATUS_SATURATION_CCID_WIDTH,
				    XDFEOFDM_STATUS_SATURATION_CCID_OFFSET,
				    Val);
	Status->SaturationCount = XDfeOfdm_RdBitField(
		XDFEOFDM_STATUS_SATURATION_SATURATION_COUNT_WIDTH,
		XDFEOFDM_STATUS_SATURATION_SATURATION_COUNT_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Clears events status.
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    Status Clear event status container.
*           - 0 - does not clear coresponding event status
*           - 1 - clears coresponding event status
*
****************************************************************************/
void XDfeOfdm_ClearEventStatus(const XDfeOfdm *InstancePtr,
			       const XDfeOfdm_Status *Status)
{
	u32 Val = 0U;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Status != NULL);
	Xil_AssertVoid(Status->CCUpdate <= 1U);
	Xil_AssertVoid(Status->FTCCSequenceError <= 1U);
	Xil_AssertVoid(Status->Saturation <= 1U);

	Val = XDfeOfdm_WrBitField(XDFEOFDM_STATUS_CC_UPDATE_TRIGGERED_WIDTH,
				  XDFEOFDM_STATUS_CC_UPDATE_TRIGGERED_OFFSET,
				  Val, Status->CCUpdate);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_STATUS_FT_CC_SEQUENCE_ERROR_WIDTH,
				  XDFEOFDM_STATUS_FT_CC_SEQUENCE_ERROR_OFFSET,
				  Val, Status->FTCCSequenceError);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_STATUS_BF_SATURATION_WIDTH,
				  XDFEOFDM_STATUS_BF_SATURATION_OFFSET, Val,
				  Status->Saturation);
	Val = XDfeOfdm_WrBitField(XDFEOFDM_STATUS_BF_OVERFLOW_WIDTH,
				  XDFEOFDM_STATUS_BF_OVERFLOW_OFFSET, Val,
				  Status->Overflow);
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_STATUS_ISR_OFFSET, Val);
	Val = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_STATUS_SATURATION_OFFSET);
}

/****************************************************************************/
/**
*
* Sets interrupt masks.
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    Mask Interrupt mask value.
*           - 0 - does not mask coresponding interrupt
*           - 1 - masks coresponding interrupt
*
****************************************************************************/
void XDfeOfdm_SetInterruptMask(const XDfeOfdm *InstancePtr,
			       const XDfeOfdm_InterruptMask *Mask)
{
	u32 ValIER = 0U;
	u32 ValIDR = 0U;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mask != NULL);
	Xil_AssertVoid(Mask->CCUpdate <= 1U);
	Xil_AssertVoid(Mask->FTCCSequenceError <= 1U);
	Xil_AssertVoid(Mask->Saturation <= 1U);

	if (Mask->CCUpdate == XDFEOFDM_IMR_INTERRUPT) {
		ValIER |= (1U << XDFEOFDM_STATUS_CC_UPDATE_TRIGGERED_OFFSET);
	} else {
		ValIDR |= (1U << XDFEOFDM_STATUS_CC_UPDATE_TRIGGERED_OFFSET);
	}

	if (Mask->FTCCSequenceError == XDFEOFDM_IMR_INTERRUPT) {
		ValIER |= (1U << XDFEOFDM_STATUS_FT_CC_SEQUENCE_ERROR_OFFSET);
	} else {
		ValIDR |= (1U << XDFEOFDM_STATUS_FT_CC_SEQUENCE_ERROR_OFFSET);
	}

	if (Mask->Saturation == XDFEOFDM_IMR_INTERRUPT) {
		ValIER |= (1U << XDFEOFDM_STATUS_BF_SATURATION_OFFSET);
	} else {
		ValIDR |= (1U << XDFEOFDM_STATUS_BF_SATURATION_OFFSET);
	}

	if (Mask->Overflow == XDFEOFDM_IMR_INTERRUPT) {
		ValIER |= (1U << XDFEOFDM_STATUS_BF_OVERFLOW_OFFSET);
	} else {
		ValIDR |= (1U << XDFEOFDM_STATUS_BF_OVERFLOW_OFFSET);
	}

	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_STATUS_IER_OFFSET, ValIER);
	XDfeOfdm_WriteReg(InstancePtr, XDFEOFDM_STATUS_IDR_OFFSET, ValIDR);
}

/****************************************************************************/
/**
*
* Gets interrupt masks.
*
* @param    InstancePtr Pointer to the OFDM instance.
* @param    Mask Interrupt mask value.
*
*
****************************************************************************/
void XDfeOfdm_GetInterruptMask(const XDfeOfdm *InstancePtr,
			       XDfeOfdm_InterruptMask *Mask)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mask != NULL);

	Val = XDfeOfdm_ReadReg(InstancePtr, XDFEOFDM_STATUS_IMR_OFFSET);
	Mask->CCUpdate =
		XDfeOfdm_RdBitField(XDFEOFDM_STATUS_CC_UPDATE_TRIGGERED_WIDTH,
				    XDFEOFDM_STATUS_CC_UPDATE_TRIGGERED_OFFSET,
				    Val);
	Mask->FTCCSequenceError =
		XDfeOfdm_RdBitField(XDFEOFDM_STATUS_FT_CC_SEQUENCE_ERROR_WIDTH,
				    XDFEOFDM_STATUS_FT_CC_SEQUENCE_ERROR_OFFSET,
				    Val);
	Mask->Saturation =
		XDfeOfdm_RdBitField(XDFEOFDM_STATUS_BF_SATURATION_WIDTH,
				    XDFEOFDM_STATUS_BF_SATURATION_OFFSET, Val);
	Mask->Overflow =
		XDfeOfdm_RdBitField(XDFEOFDM_STATUS_BF_OVERFLOW_WIDTH,
				    XDFEOFDM_STATUS_BF_OVERFLOW_OFFSET, Val);
}

/** @} */
