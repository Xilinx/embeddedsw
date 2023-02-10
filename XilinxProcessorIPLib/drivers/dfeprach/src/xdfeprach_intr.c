/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeprach_intr.c
* This file contains functions related to PRACH interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     03/08/21 Initial version
*       dc     04/18/21 Update trigger and event handlers
* 1.1   dc     06/30/21 Doxygen documentation update
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/05/21 Align event handlers
*       dc     11/19/21 Update doxygen documentation
* 1.4   dc     08/18/22 Correct event status read
* 1.5   dc     01/02/23 Multiband registers update
*
* </pre>
* @addtogroup dfeprach Overview
* @{
******************************************************************************/

#include "xdfeprach.h"
#include "xdfeprach_hw.h"

/**************************** Macros Definitions ****************************/

/************************** Function Prototypes *****************************/
/**
* @cond nocomments
*/
extern u32 XDfePrach_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data);
extern u32 XDfePrach_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data,
				u32 Val);
/**
* @endcond
*/

/****************************************************************************/
/**
*
* Gets interrupt mask status.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Mask Interrupt masks container.
*
****************************************************************************/
void XDfePrach_GetInterruptMask(const XDfePrach *InstancePtr,
				XDfePrach_InterruptMask *Mask)
{
	u32 Val;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mask != NULL);

	Val = XDfePrach_ReadReg(InstancePtr, XDFEPRACH_IMR);
	Mask->DecimatorOverflow =
		XDfePrach_RdBitField(XDFEPRACH_DECIMATOR_OVERFLOW_WIDTH,
				     XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET, Val);
	Mask->MixerOverflow =
		XDfePrach_RdBitField(XDFEPRACH_MIXER_OVERFLOW_WIDTH,
				     XDFEPRACH_MIXER_OVERFLOW_OFFSET, Val);
	Mask->DecimatorOverrun =
		XDfePrach_RdBitField(XDFEPRACH_DECIMATOR_OVERRUN_WIDTH,
				     XDFEPRACH_DECIMATOR_OVERRUN_OFFSET, Val);
	Mask->SelectorOverrun =
		XDfePrach_RdBitField(XDFEPRACH_SELECTOR_OVERRUN_WIDTH,
				     XDFEPRACH_SELECTOR_OVERRUN_OFFSET, Val);
	Mask->RachUpdate =
		XDfePrach_RdBitField(XDFEPRACH_RACH_UPDATE_TRIGGERED_WIDTH,
				     XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET,
				     Val);
	for (Index = 0; Index < XDFEPRACH_BAND_ID_MAX; Index++) {
		Mask->CCSequenceError[Index] = XDfePrach_RdBitField(
			XDFEPRACH_CC_SEQUENCE_ERROR_WIDTH,
			XDFEPRACH_CC_SEQUENCE_ERROR_OFFSET(Index), Val);
		Mask->FrameInitTrigger[Index] = XDfePrach_RdBitField(
			XDFEPRACH_FRAME_INIT_TRIGGERED_WIDTH,
			XDFEPRACH_FRAME_INIT_TRIGGERED_OFFSET(Index), Val);
		Mask->FrameError[Index] = XDfePrach_RdBitField(
			XDFEPRACH_FRAME_ERROR_WIDTH,
			XDFEPRACH_FRAME_ERROR_OFFSET(Index), Val);
	}
}
/****************************************************************************/
/**
*
* Sets interrupt mask.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Mask Interrupt mask flags container.
*           - 0 - does not mask corresponding interrupt
*           - 1 - masks corresponding interrupt
*
****************************************************************************/
void XDfePrach_SetInterruptMask(const XDfePrach *InstancePtr,
				const XDfePrach_InterruptMask *Mask)
{
	u32 ValIER = 0U;
	u32 ValIDR = 0U;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mask != NULL);
	Xil_AssertVoid(Mask->DecimatorOverflow <= 1U);
	Xil_AssertVoid(Mask->MixerOverflow <= 1U);
	Xil_AssertVoid(Mask->DecimatorOverrun <= 1U);
	Xil_AssertVoid(Mask->SelectorOverrun <= 1U);
	Xil_AssertVoid(Mask->RachUpdate <= 1U);
	for (Index = 0; Index < XDFEPRACH_BAND_ID_MAX; Index++) {
		Xil_AssertVoid(Mask->CCSequenceError[Index] <= 1U);
		Xil_AssertVoid(Mask->FrameInitTrigger[Index] <= 1U);
		Xil_AssertVoid(Mask->FrameError[Index] <= 1U);
	}

	if (Mask->DecimatorOverflow == XDFEPRACH_IMR_INTERRUPT) {
		ValIER |= (1U << XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET);
	} else {
		ValIDR |= (1U << XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET);
	}

	if (Mask->MixerOverflow == XDFEPRACH_IMR_INTERRUPT) {
		ValIER |= (1U << XDFEPRACH_MIXER_OVERFLOW_OFFSET);
	} else {
		ValIDR |= (1U << XDFEPRACH_MIXER_OVERFLOW_OFFSET);
	}

	if (Mask->DecimatorOverrun == XDFEPRACH_IMR_INTERRUPT) {
		ValIER |= (1U << XDFEPRACH_DECIMATOR_OVERRUN_OFFSET);
	} else {
		ValIDR |= (1U << XDFEPRACH_DECIMATOR_OVERRUN_OFFSET);
	}

	if (Mask->SelectorOverrun == XDFEPRACH_IMR_INTERRUPT) {
		ValIER |= (1U << XDFEPRACH_SELECTOR_OVERRUN_OFFSET);
	} else {
		ValIDR |= (1U << XDFEPRACH_SELECTOR_OVERRUN_OFFSET);
	}

	if (Mask->RachUpdate == XDFEPRACH_IMR_INTERRUPT) {
		ValIER |= (1U << XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET);
	} else {
		ValIDR |= (1U << XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET);
	}

	for (Index = 0; Index < XDFEPRACH_BAND_ID_MAX; Index++) {
		if (Mask->CCSequenceError[Index] == XDFEPRACH_IMR_INTERRUPT) {
			ValIER |= (1U << XDFEPRACH_CC_SEQUENCE_ERROR_OFFSET(
					   Index));
		} else {
			ValIDR |= (1U << XDFEPRACH_CC_SEQUENCE_ERROR_OFFSET(
					   Index));
		}

		if (Mask->FrameInitTrigger[Index] == XDFEPRACH_IMR_INTERRUPT) {
			ValIER |= (1U << XDFEPRACH_FRAME_INIT_TRIGGERED_OFFSET(
					   Index));
		} else {
			ValIDR |= (1U << XDFEPRACH_FRAME_INIT_TRIGGERED_OFFSET(
					   Index));
		}

		if (Mask->FrameError[Index] == XDFEPRACH_IMR_INTERRUPT) {
			ValIER |= (1U << XDFEPRACH_FRAME_ERROR_OFFSET(Index));
		} else {
			ValIDR |= (1U << XDFEPRACH_FRAME_ERROR_OFFSET(Index));
		}
	}

	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_IER, ValIER);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_IDR, ValIDR);
}

/****************************************************************************/
/**
*
* Gets event status.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Status Event status container.
*
****************************************************************************/
void XDfePrach_GetEventStatus(const XDfePrach *InstancePtr,
			      XDfePrach_StatusMask *Status)
{
	u32 Val;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Status != NULL);

	Val = XDfePrach_ReadReg(InstancePtr, XDFEPRACH_ISR);
	Status->DecimatorOverflow =
		XDfePrach_RdBitField(XDFEPRACH_DECIMATOR_OVERFLOW_WIDTH,
				     XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET, Val);
	Status->MixerOverflow =
		XDfePrach_RdBitField(XDFEPRACH_MIXER_OVERFLOW_WIDTH,
				     XDFEPRACH_MIXER_OVERFLOW_OFFSET, Val);
	Status->DecimatorOverrun =
		XDfePrach_RdBitField(XDFEPRACH_DECIMATOR_OVERRUN_WIDTH,
				     XDFEPRACH_DECIMATOR_OVERRUN_OFFSET, Val);
	Status->SelectorOverrun =
		XDfePrach_RdBitField(XDFEPRACH_SELECTOR_OVERRUN_WIDTH,
				     XDFEPRACH_SELECTOR_OVERRUN_OFFSET, Val);
	Status->RachUpdate =
		XDfePrach_RdBitField(XDFEPRACH_RACH_UPDATE_TRIGGERED_WIDTH,
				     XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET,
				     Val);
	for (Index = 0; Index < XDFEPRACH_BAND_ID_MAX; Index++) {
		Status->CCSequenceError[Index] = XDfePrach_RdBitField(
			XDFEPRACH_CC_SEQUENCE_ERROR_WIDTH,
			XDFEPRACH_CC_SEQUENCE_ERROR_OFFSET(Index), Val);
		Status->FrameInitTrigger[Index] = XDfePrach_RdBitField(
			XDFEPRACH_FRAME_INIT_TRIGGERED_WIDTH,
			XDFEPRACH_FRAME_INIT_TRIGGERED_OFFSET(Index), Val);
		Status->FrameError[Index] = XDfePrach_RdBitField(
			XDFEPRACH_FRAME_ERROR_WIDTH,
			XDFEPRACH_FRAME_ERROR_OFFSET(Index), Val);
	}
}

/****************************************************************************/
/**
*
* Clears event status.
*
* @param    InstancePtr Pointer to the PRACH instance.
* @param    Status Clear event status container.
*           - 0 - does not clear corresponding event status
*           - 1 - clear corresponding event status
*
****************************************************************************/
void XDfePrach_ClearEventStatus(const XDfePrach *InstancePtr,
				const XDfePrach_StatusMask *Status)
{
	u32 Data = 0;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Status != NULL);
	Xil_AssertVoid(Status->DecimatorOverflow <= 1U);
	Xil_AssertVoid(Status->MixerOverflow <= 1U);
	Xil_AssertVoid(Status->DecimatorOverrun <= 1U);
	Xil_AssertVoid(Status->SelectorOverrun <= 1U);
	Xil_AssertVoid(Status->RachUpdate <= 1U);
	for (Index = 0; Index < XDFEPRACH_BAND_ID_MAX; Index++) {
		Xil_AssertVoid(Status->CCSequenceError[Index] <= 1U);
		Xil_AssertVoid(Status->FrameInitTrigger[Index] <= 1U);
		Xil_AssertVoid(Status->FrameError[Index] <= 1U);
	}
	Data = XDfePrach_WrBitField(XDFEPRACH_DECIMATOR_OVERFLOW_WIDTH,
				    XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET, Data,
				    Status->DecimatorOverflow);
	Data = XDfePrach_WrBitField(XDFEPRACH_MIXER_OVERFLOW_WIDTH,
				    XDFEPRACH_MIXER_OVERFLOW_OFFSET, Data,
				    Status->MixerOverflow);
	Data = XDfePrach_WrBitField(XDFEPRACH_DECIMATOR_OVERRUN_WIDTH,
				    XDFEPRACH_DECIMATOR_OVERRUN_OFFSET, Data,
				    Status->DecimatorOverrun);
	Data = XDfePrach_WrBitField(XDFEPRACH_SELECTOR_OVERRUN_WIDTH,
				    XDFEPRACH_SELECTOR_OVERRUN_OFFSET, Data,
				    Status->SelectorOverrun);
	Data = XDfePrach_WrBitField(XDFEPRACH_RACH_UPDATE_TRIGGERED_WIDTH,
				    XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET,
				    Data, Status->RachUpdate);
	for (Index = 0; Index < XDFEPRACH_BAND_ID_MAX; Index++) {
		Data = XDfePrach_WrBitField(
			XDFEPRACH_CC_SEQUENCE_ERROR_WIDTH,
			XDFEPRACH_CC_SEQUENCE_ERROR_OFFSET(Index), Data,
			Status->CCSequenceError[Index]);
		Data = XDfePrach_WrBitField(
			XDFEPRACH_FRAME_INIT_TRIGGERED_WIDTH,
			XDFEPRACH_FRAME_INIT_TRIGGERED_OFFSET(Index), Data,
			Status->FrameInitTrigger[Index]);
		Data = XDfePrach_WrBitField(XDFEPRACH_FRAME_ERROR_WIDTH,
					    XDFEPRACH_FRAME_ERROR_OFFSET(Index),
					    Data, Status->FrameError[Index]);
	}
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_ISR, Data);
}

/** @} */
