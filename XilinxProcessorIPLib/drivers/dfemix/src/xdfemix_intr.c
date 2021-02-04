/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfexmix_intr.c
* @addtogroup xdfemix_v1_0
* @{
*
* This file contains functions related to Equalizer interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     10/21/20 Initial version
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
* Get interrupt mask status.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags is interrupt flags container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_GetInterruptMask(const XDfeMix *InstancePtr,
			      XDfeMix_InterruptMask *Flags)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

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
* Set interrupt mask.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags is interrupt flags container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_SetInterruptMask(XDfeMix *InstancePtr,
			      XDfeMix_InterruptMask *Flags)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

	XDfeMix_InterruptDisable(InstancePtr, Flags);
	XDfeMix_InterruptEnable(InstancePtr, Flags);
}

/****************************************************************************/
/**
*
* Enable interrupts.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags is interrupt flags container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_InterruptEnable(const XDfeMix *InstancePtr,
			     const XDfeMix_InterruptMask *Flags)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

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
* Disable interrupts.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags is interrupt flags container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_InterruptDisable(const XDfeMix *InstancePtr,
			      const XDfeMix_InterruptMask *Flags)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

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
* Get interrupt status.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags is interrupt flags container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_GetInterruptStatus(const XDfeMix *InstancePtr,
				XDfeMix_InterruptMask *Flags)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

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
* Clear interrupt status.
*
* @param    InstancePtr is a pointer to the Mixer instance.
* @param    Flags is interrupt flags container.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeMix_ClearInterruptStatus(const XDfeMix *InstancePtr,
				  const XDfeMix_InterruptMask *Flags)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flags != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFEMIX_STATE_OPERATIONAL);

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
