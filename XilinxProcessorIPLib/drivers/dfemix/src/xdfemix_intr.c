/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfexmix_intr.c
* @addtogroup xdfemix_v1_1
* @{
*
* This file contains functions related to Mixer interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     10/21/20 Initial version
*       dc     02/15/21 align driver to curent specification
*       dc     03/18/21 New model parameter list
*       dc     04/18/21 Update trigger and event handlers
*       dc     04/20/21 Doxygen documentation update
* 1.1   dc     07/13/21 Update to common latency requirements
*
* </pre>
*
******************************************************************************/

#include "xdfemix.h"
#include "xdfemix_hw.h"

/**************************** Macros Definitions ****************************/

/************************** Function Prototypes *****************************/
extern u32 XDfeMix_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data);
extern u32 XDfeMix_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data,
			      u32 Val);

/****************************************************************************/
/**
*
* Gets interrupt mask status.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags are interrupted flags container.
*
****************************************************************************/
void XDfeMix_GetInterruptMask(const XDfeMix *InstancePtr,
			      XDfeMix_InterruptMask *Flags)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_IMR);
	Flags->DUCDDCOverflow =
		XDfeMix_RdBitField(XDFEMIX_DUC_DDC_OVERFLOW_WIDTH,
				   XDFEMIX_DUC_DDC_OVERFLOW_OFFSET, Val);
	Flags->MixerOverflow =
		XDfeMix_RdBitField(XDFEMIX_MIXER_OVERFLOW_WIDTH,
				   XDFEMIX_MIXER_OVERFLOW_OFFSET, Val);
	Flags->DLCCUpdate =
		XDfeMix_RdBitField(XDFEMIX_CC_UPDATE_TRIGGERED_WIDTH,
				   XDFEMIX_CC_UPDATE_TRIGGERED_OFFSET, Val);
	Flags->DLCCSequenceError =
		XDfeMix_RdBitField(XDFEMIX_CC_SEQUENCE_ERROR_WIDTH,
				   XDFEMIX_CC_SEQUENCE_ERROR_OFFSET, Val);
}
/****************************************************************************/
/**
*
* Sets interrupt mask.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags are interrupted flags container.
*
****************************************************************************/
void XDfeMix_SetInterruptMask(const XDfeMix *InstancePtr,
			      const XDfeMix_InterruptMask *Flags)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Data = XDfeMix_WrBitField(XDFEMIX_DUC_DDC_OVERFLOW_WIDTH,
				  XDFEMIX_DUC_DDC_OVERFLOW_OFFSET, Data,
				  Flags->DUCDDCOverflow);
	Data = XDfeMix_WrBitField(XDFEMIX_MIXER_OVERFLOW_WIDTH,
				  XDFEMIX_MIXER_OVERFLOW_OFFSET, Data,
				  Flags->MixerOverflow);
	Data = XDfeMix_WrBitField(XDFEMIX_CC_UPDATE_TRIGGERED_WIDTH,
				  XDFEMIX_CC_UPDATE_TRIGGERED_OFFSET, Data,
				  Flags->DLCCUpdate);
	Data = XDfeMix_WrBitField(XDFEMIX_CC_SEQUENCE_ERROR_WIDTH,
				  XDFEMIX_CC_SEQUENCE_ERROR_OFFSET, Data,
				  Flags->DLCCSequenceError);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_IMR, Data);
}

/****************************************************************************/
/**
*
* Enables interrupts.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags are interrupted flags container.
*
****************************************************************************/
void XDfeMix_InterruptEnable(const XDfeMix *InstancePtr,
			     const XDfeMix_InterruptMask *Flags)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Data = XDfeMix_WrBitField(XDFEMIX_DUC_DDC_OVERFLOW_WIDTH,
				  XDFEMIX_DUC_DDC_OVERFLOW_OFFSET, Data,
				  Flags->DUCDDCOverflow);
	Data = XDfeMix_WrBitField(XDFEMIX_MIXER_OVERFLOW_WIDTH,
				  XDFEMIX_MIXER_OVERFLOW_OFFSET, Data,
				  Flags->MixerOverflow);
	Data = XDfeMix_WrBitField(XDFEMIX_CC_UPDATE_TRIGGERED_WIDTH,
				  XDFEMIX_CC_UPDATE_TRIGGERED_OFFSET, Data,
				  Flags->DLCCUpdate);
	Data = XDfeMix_WrBitField(XDFEMIX_CC_SEQUENCE_ERROR_WIDTH,
				  XDFEMIX_CC_SEQUENCE_ERROR_OFFSET, Data,
				  Flags->DLCCSequenceError);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_IER, Data);
}

/****************************************************************************/
/**
*
* Disables interrupts.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags are interrupted flags container.
*
****************************************************************************/
void XDfeMix_InterruptDisable(const XDfeMix *InstancePtr,
			      const XDfeMix_InterruptMask *Flags)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Data = XDfeMix_WrBitField(XDFEMIX_DUC_DDC_OVERFLOW_WIDTH,
				  XDFEMIX_DUC_DDC_OVERFLOW_OFFSET, Data,
				  Flags->DUCDDCOverflow);
	Data = XDfeMix_WrBitField(XDFEMIX_MIXER_OVERFLOW_WIDTH,
				  XDFEMIX_MIXER_OVERFLOW_OFFSET, Data,
				  Flags->MixerOverflow);
	Data = XDfeMix_WrBitField(XDFEMIX_CC_UPDATE_TRIGGERED_WIDTH,
				  XDFEMIX_CC_UPDATE_TRIGGERED_OFFSET, Data,
				  Flags->DLCCUpdate);
	Data = XDfeMix_WrBitField(XDFEMIX_CC_SEQUENCE_ERROR_WIDTH,
				  XDFEMIX_CC_SEQUENCE_ERROR_OFFSET, Data,
				  Flags->DLCCSequenceError);
	Data = ~Data &
	       XDFEMIX_IRQ_FLAGS_MASK; /* Invert flags to set IRQ disable */
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_IDR, Data);
}
/****************************************************************************/
/**
*
* Gets interrupt status.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags are interrupted flags container.
*
****************************************************************************/
void XDfeMix_GetInterruptStatus(const XDfeMix *InstancePtr,
				XDfeMix_InterruptMask *Flags)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Val = XDfeMix_ReadReg(InstancePtr, XDFEMIX_ISR);
	Flags->DUCDDCOverflow =
		XDfeMix_RdBitField(XDFEMIX_DUC_DDC_OVERFLOW_WIDTH,
				   XDFEMIX_DUC_DDC_OVERFLOW_OFFSET, Val);
	Flags->MixerOverflow =
		XDfeMix_RdBitField(XDFEMIX_MIXER_OVERFLOW_WIDTH,
				   XDFEMIX_MIXER_OVERFLOW_OFFSET, Val);
	Flags->DLCCUpdate =
		XDfeMix_RdBitField(XDFEMIX_CC_UPDATE_TRIGGERED_WIDTH,
				   XDFEMIX_CC_UPDATE_TRIGGERED_OFFSET, Val);
	Flags->DLCCSequenceError =
		XDfeMix_RdBitField(XDFEMIX_CC_SEQUENCE_ERROR_WIDTH,
				   XDFEMIX_CC_SEQUENCE_ERROR_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Clears interrupt status.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags are interrupted flags container.
*
****************************************************************************/
void XDfeMix_ClearInterruptStatus(const XDfeMix *InstancePtr,
				  const XDfeMix_InterruptMask *Flags)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);

	Data = XDfeMix_WrBitField(XDFEMIX_DUC_DDC_OVERFLOW_WIDTH,
				  XDFEMIX_DUC_DDC_OVERFLOW_OFFSET, Data,
				  Flags->DUCDDCOverflow);
	Data = XDfeMix_WrBitField(XDFEMIX_MIXER_OVERFLOW_WIDTH,
				  XDFEMIX_MIXER_OVERFLOW_OFFSET, Data,
				  Flags->MixerOverflow);
	Data = XDfeMix_WrBitField(XDFEMIX_CC_UPDATE_TRIGGERED_WIDTH,
				  XDFEMIX_CC_UPDATE_TRIGGERED_OFFSET, Data,
				  Flags->DLCCUpdate);
	Data = XDfeMix_WrBitField(XDFEMIX_CC_SEQUENCE_ERROR_WIDTH,
				  XDFEMIX_CC_SEQUENCE_ERROR_OFFSET, Data,
				  Flags->DLCCSequenceError);
	XDfeMix_WriteReg(InstancePtr, XDFEMIX_ISR, Data);
}

/** @} */
