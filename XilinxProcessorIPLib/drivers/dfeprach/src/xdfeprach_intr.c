/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeprach_intr.c
* @addtogroup xdfeprach_v1_1
* @{
*
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
*
* </pre>
*
******************************************************************************/

#include "xdfeprach.h"
#include "xdfeprach_hw.h"

/**************************** Macros Definitions ****************************/

/************************** Function Prototypes *****************************/
extern u32 XDfePrach_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data);
extern u32 XDfePrach_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data,
				u32 Val);

/****************************************************************************/
/**
*
* Gets interrupt mask status.
*
* @param    InstancePtr is a pointer to the PRACH instance.
* @param    Flags is an interrupt flags container.
*
****************************************************************************/
void XDfePrach_GetInterruptMask(const XDfePrach *InstancePtr,
				XDfePrach_InterruptMask *Flags)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Val = XDfePrach_ReadReg(InstancePtr, XDFEPRACH_IMR);
	Flags->DecimatorOverflow =
		XDfePrach_RdBitField(XDFEPRACH_DECIMATOR_OVERFLOW_WIDTH,
				     XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET, Val);
	Flags->MixerOverflow =
		XDfePrach_RdBitField(XDFEPRACH_MIXER_OVERFLOW_WIDTH,
				     XDFEPRACH_MIXER_OVERFLOW_OFFSET, Val);
	Flags->DecimatorOverrun =
		XDfePrach_RdBitField(XDFEPRACH_DECIMATOR_OVERRUN_WIDTH,
				     XDFEPRACH_DECIMATOR_OVERRUN_OFFSET, Val);
	Flags->SelectorOverrun =
		XDfePrach_RdBitField(XDFEPRACH_SELECTOR_OVERRUN_WIDTH,
				     XDFEPRACH_SELECTOR_OVERRUN_OFFSET, Val);
	Flags->RachUpdate =
		XDfePrach_RdBitField(XDFEPRACH_RACH_UPDATE_TRIGGERED_WIDTH,
				     XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET,
				     Val);
	Flags->CCSequenceError =
		XDfePrach_RdBitField(XDFEPRACH_CC_SEQUENCE_ERROR_WIDTH,
				     XDFEPRACH_CC_SEQUENCE_ERROR_OFFSET, Val);
	Flags->SFSequenceUpdate =
		XDfePrach_RdBitField(XDFEPRACH_FRAME_INIT_TRIGGERED_WIDTH,
				     XDFEPRACH_FRAME_INIT_TRIGGERED_OFFSET,
				     Val);
}
/****************************************************************************/
/**
*
* Sets interrupt mask.
*
* @param    InstancePtr is a pointer to the PRACH instance.
* @param    Flags is an interrupt flags container.
*
****************************************************************************/
void XDfePrach_SetInterruptMask(const XDfePrach *InstancePtr,
				const XDfePrach_InterruptMask *Flags)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Data = XDfePrach_WrBitField(XDFEPRACH_DECIMATOR_OVERFLOW_WIDTH,
				    XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET, Data,
				    Flags->DecimatorOverflow);
	Data = XDfePrach_WrBitField(XDFEPRACH_MIXER_OVERFLOW_WIDTH,
				    XDFEPRACH_MIXER_OVERFLOW_OFFSET, Data,
				    Flags->MixerOverflow);
	Data = XDfePrach_WrBitField(XDFEPRACH_DECIMATOR_OVERRUN_WIDTH,
				    XDFEPRACH_DECIMATOR_OVERRUN_OFFSET, Data,
				    Flags->DecimatorOverrun);
	Data = XDfePrach_WrBitField(XDFEPRACH_SELECTOR_OVERRUN_WIDTH,
				    XDFEPRACH_SELECTOR_OVERRUN_OFFSET, Data,
				    Flags->SelectorOverrun);
	Data = XDfePrach_WrBitField(XDFEPRACH_RACH_UPDATE_TRIGGERED_WIDTH,
				    XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET,
				    Data, Flags->RachUpdate);
	Data = XDfePrach_WrBitField(XDFEPRACH_CC_SEQUENCE_ERROR_WIDTH,
				    XDFEPRACH_CC_SEQUENCE_ERROR_OFFSET, Data,
				    Flags->CCSequenceError);
	Data = XDfePrach_WrBitField(XDFEPRACH_FRAME_INIT_TRIGGERED_WIDTH,
				    XDFEPRACH_FRAME_INIT_TRIGGERED_OFFSET, Data,
				    Flags->SFSequenceUpdate);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_IMR, Data);
}

/****************************************************************************/
/**
*
* Enables interrupts.
*
* @param    InstancePtr is a pointer to the PRACH instance.
* @param    Flags is an interrupt flags container.
*
****************************************************************************/
void XDfePrach_InterruptEnable(const XDfePrach *InstancePtr,
			       const XDfePrach_InterruptMask *Flags)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Data = XDfePrach_WrBitField(XDFEPRACH_DECIMATOR_OVERFLOW_WIDTH,
				    XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET, Data,
				    Flags->DecimatorOverflow);
	Data = XDfePrach_WrBitField(XDFEPRACH_MIXER_OVERFLOW_WIDTH,
				    XDFEPRACH_MIXER_OVERFLOW_OFFSET, Data,
				    Flags->MixerOverflow);
	Data = XDfePrach_WrBitField(XDFEPRACH_DECIMATOR_OVERRUN_WIDTH,
				    XDFEPRACH_DECIMATOR_OVERRUN_OFFSET, Data,
				    Flags->DecimatorOverrun);
	Data = XDfePrach_WrBitField(XDFEPRACH_SELECTOR_OVERRUN_WIDTH,
				    XDFEPRACH_SELECTOR_OVERRUN_OFFSET, Data,
				    Flags->SelectorOverrun);
	Data = XDfePrach_WrBitField(XDFEPRACH_RACH_UPDATE_TRIGGERED_WIDTH,
				    XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET,
				    Data, Flags->RachUpdate);
	Data = XDfePrach_WrBitField(XDFEPRACH_CC_SEQUENCE_ERROR_WIDTH,
				    XDFEPRACH_CC_SEQUENCE_ERROR_OFFSET, Data,
				    Flags->CCSequenceError);
	Data = XDfePrach_WrBitField(XDFEPRACH_FRAME_INIT_TRIGGERED_WIDTH,
				    XDFEPRACH_FRAME_INIT_TRIGGERED_OFFSET, Data,
				    Flags->SFSequenceUpdate);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_IER, Data);
}

/****************************************************************************/
/**
*
* Disables interrupts.
*
* @param    InstancePtr is a pointer to the PRACH instance.
* @param    Flags is an interrupt flags container.
*
****************************************************************************/
void XDfePrach_InterruptDisable(const XDfePrach *InstancePtr,
				const XDfePrach_InterruptMask *Flags)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Data = XDfePrach_WrBitField(XDFEPRACH_DECIMATOR_OVERFLOW_WIDTH,
				    XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET, Data,
				    Flags->DecimatorOverflow);
	Data = XDfePrach_WrBitField(XDFEPRACH_MIXER_OVERFLOW_WIDTH,
				    XDFEPRACH_MIXER_OVERFLOW_OFFSET, Data,
				    Flags->MixerOverflow);
	Data = XDfePrach_WrBitField(XDFEPRACH_DECIMATOR_OVERRUN_WIDTH,
				    XDFEPRACH_DECIMATOR_OVERRUN_OFFSET, Data,
				    Flags->DecimatorOverrun);
	Data = XDfePrach_WrBitField(XDFEPRACH_SELECTOR_OVERRUN_WIDTH,
				    XDFEPRACH_SELECTOR_OVERRUN_OFFSET, Data,
				    Flags->SelectorOverrun);
	Data = XDfePrach_WrBitField(XDFEPRACH_RACH_UPDATE_TRIGGERED_WIDTH,
				    XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET,
				    Data, Flags->RachUpdate);
	Data = XDfePrach_WrBitField(XDFEPRACH_CC_SEQUENCE_ERROR_WIDTH,
				    XDFEPRACH_CC_SEQUENCE_ERROR_OFFSET, Data,
				    Flags->CCSequenceError);
	Data = XDfePrach_WrBitField(XDFEPRACH_FRAME_INIT_TRIGGERED_WIDTH,
				    XDFEPRACH_FRAME_INIT_TRIGGERED_OFFSET, Data,
				    Flags->SFSequenceUpdate);
	Data = ~Data &
	       XDFEPRACH_IRQ_FLAGS_MASK; /* Invert flags to set IRQ disable */
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_IDR, Data);
}
/****************************************************************************/
/**
*
* Gets interrupt status.
*
* @param    InstancePtr is a pointer to the PRACH instance.
* @param    Flags is an interrupt flags container.
*
****************************************************************************/
void XDfePrach_GetInterruptStatus(const XDfePrach *InstancePtr,
				  XDfePrach_InterruptMask *Flags)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Val = XDfePrach_ReadReg(InstancePtr, XDFEPRACH_ISR);
	Flags->DecimatorOverflow =
		XDfePrach_RdBitField(XDFEPRACH_DECIMATOR_OVERFLOW_WIDTH,
				     XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET, Val);
	Flags->MixerOverflow =
		XDfePrach_RdBitField(XDFEPRACH_MIXER_OVERFLOW_WIDTH,
				     XDFEPRACH_MIXER_OVERFLOW_OFFSET, Val);
	Flags->DecimatorOverrun =
		XDfePrach_RdBitField(XDFEPRACH_DECIMATOR_OVERRUN_WIDTH,
				     XDFEPRACH_DECIMATOR_OVERRUN_OFFSET, Val);
	Flags->SelectorOverrun =
		XDfePrach_RdBitField(XDFEPRACH_SELECTOR_OVERRUN_WIDTH,
				     XDFEPRACH_SELECTOR_OVERRUN_OFFSET, Val);
	Flags->RachUpdate =
		XDfePrach_RdBitField(XDFEPRACH_RACH_UPDATE_TRIGGERED_WIDTH,
				     XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET,
				     Val);
	Flags->CCSequenceError =
		XDfePrach_RdBitField(XDFEPRACH_CC_SEQUENCE_ERROR_WIDTH,
				     XDFEPRACH_CC_SEQUENCE_ERROR_OFFSET, Val);
	Flags->SFSequenceUpdate =
		XDfePrach_RdBitField(XDFEPRACH_FRAME_INIT_TRIGGERED_WIDTH,
				     XDFEPRACH_FRAME_INIT_TRIGGERED_OFFSET,
				     Val);
}

/****************************************************************************/
/**
*
* Clears interrupt status.
*
* @param    InstancePtr is a pointer to the PRACH instance.
* @param    Flags is an interrupt flags container.
*
****************************************************************************/
void XDfePrach_ClearInterruptStatus(const XDfePrach *InstancePtr,
				    const XDfePrach_InterruptMask *Flags)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Data = XDfePrach_WrBitField(XDFEPRACH_DECIMATOR_OVERFLOW_WIDTH,
				    XDFEPRACH_DECIMATOR_OVERFLOW_OFFSET, Data,
				    Flags->DecimatorOverflow);
	Data = XDfePrach_WrBitField(XDFEPRACH_MIXER_OVERFLOW_WIDTH,
				    XDFEPRACH_MIXER_OVERFLOW_OFFSET, Data,
				    Flags->MixerOverflow);
	Data = XDfePrach_WrBitField(XDFEPRACH_DECIMATOR_OVERRUN_WIDTH,
				    XDFEPRACH_DECIMATOR_OVERRUN_OFFSET, Data,
				    Flags->DecimatorOverrun);
	Data = XDfePrach_WrBitField(XDFEPRACH_SELECTOR_OVERRUN_WIDTH,
				    XDFEPRACH_SELECTOR_OVERRUN_OFFSET, Data,
				    Flags->SelectorOverrun);
	Data = XDfePrach_WrBitField(XDFEPRACH_RACH_UPDATE_TRIGGERED_WIDTH,
				    XDFEPRACH_RACH_UPDATE_TRIGGERED_OFFSET,
				    Data, Flags->RachUpdate);
	Data = XDfePrach_WrBitField(XDFEPRACH_CC_SEQUENCE_ERROR_WIDTH,
				    XDFEPRACH_CC_SEQUENCE_ERROR_OFFSET, Data,
				    Flags->CCSequenceError);
	Data = XDfePrach_WrBitField(XDFEPRACH_FRAME_INIT_TRIGGERED_WIDTH,
				    XDFEPRACH_FRAME_INIT_TRIGGERED_OFFSET, Data,
				    Flags->SFSequenceUpdate);
	XDfePrach_WriteReg(InstancePtr, XDFEPRACH_ISR, Data);
}

/** @} */
