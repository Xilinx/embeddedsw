/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ_intr.c
* This file contains functions related to Equalizer interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     12/10/20 Initial version
*       dc     02/22/21 align driver to current specification
*       dc     04/18/21 Update trigger and event handlers
*       dc     04/20/21 Doxygen documentation update
* 1.2   dc     10/29/21 Update doxygen comments
*       dc     11/05/21 Align event handlers
*       dc     11/19/21 Update doxygen documentation
*
* </pre>
* @addtogroup Overview
* @{
******************************************************************************/
/**
* @cond nocomments
*/

#include "xdfeequ.h"
#include "xdfeequ_hw.h"

/**************************** Macros Definitions ****************************/

/************************** Function Prototypes *****************************/
extern u32 XDfeEqu_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data);
extern u32 XDfeEqu_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data,
			      u32 Val);
/**
* @endcond
*/

/****************************************************************************/
/**
*
* Gets event status.
*
* @param    InstancePtr Pointer to the Equalizer instance.
* @param    Status Equalizer status container.
*
****************************************************************************/
void XDfeEqu_GetEventStatus(const XDfeEqu *InstancePtr, XDfeEqu_Status *Status)
{
	u32 Index;
	u32 Data;
	u32 Offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Status != NULL);

	/* Reads the relevant bits of the Channel_Id Status register. Populates
	   Status.i_status and Status.q_status. */
	Offset = XDFEEQU_CHANNEL_0_STATUS_OFFSET;
	for (Index = 0; Index < XDFEEQU_CHANNEL_NUM; Index++) {
		Data = XDfeEqu_ReadReg(
			InstancePtr,
			Offset + (XDFEEQU_CHANNEL_STATUS_OFFSET * Index));
		Status->IStatus[Index] = Data & XDFEEQU_CHANNEL_I_STATUS_MASK;
		Status->QStatus[Index] =
			(Data & XDFEEQU_CHANNEL_Q_STATUS_MASK) >>
			XDFEEQU_CHANNEL_Q_STATUS_OFFSET;
	}
}

/****************************************************************************/
/**
*
* Clears Equalizer status. The channel status will be cleared for any IStatus
* or QStatus not equal 0.
*
* @param    InstancePtr Pointer to the Equalizer instance.
* @param    Status Equalizer status container.
*
* @note     The Status registers are only present for a given channel when it
*           is present in the configured IP. The number of channels present is
*           given by CONFIG.NUM_CHANNELS.
*
****************************************************************************/
void XDfeEqu_ClearEventStatus(const XDfeEqu *InstancePtr,
			      const XDfeEqu_Status *Status)
{
	u32 Index;
	u32 Offset;
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Status != NULL);

	/* Clears the Status register if IStatus or Qstatus are not 0. */
	Offset = XDFEEQU_CHANNEL_0_STATUS_OFFSET;
	for (Index = 0; Index < XDFEEQU_CHANNEL_NUM; Index++) {
		Data = (Status->IStatus[Index] & 1U) |
		       ((Status->QStatus[Index] & 1U)
			<< XDFEEQU_CHANNEL_Q_STATUS_OFFSET);
		XDfeEqu_WriteReg(
			InstancePtr,
			Offset + (XDFEEQU_CHANNEL_STATUS_OFFSET * Index), Data);
	}
}

/****************************************************************************/
/**
*
* Enables an Equalizer status for channel ID.
*
* @param    InstancePtr Pointer to the Equalizer instance.
* @param    ChannelField Bits indicating which channel is enabled.
* @param    InterruptMask Equalizer interrupt mask container.
*
* @note     The Status Mask registers are only present for a given channel when
*           it is present in the configured IP. The number of channels present
*           is given by CONFIG.NUM_CHANNELS.
*
****************************************************************************/
void XDfeEqu_SetInterruptMask(const XDfeEqu *InstancePtr,
			      const XDfeEqu_InterruptMask *InterruptMask)
{
	u32 Index;
	u32 Offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InterruptMask != NULL);

	/* Sets the Status Mask register */
	Offset = XDFEEQU_CHANNEL_0_STATUS_MASK_OFFSET;
	for (Index = 0; Index < XDFEEQU_CHANNEL_NUM; Index++) {
		XDfeEqu_WriteReg(
			InstancePtr,
			Offset + (XDFEEQU_CHANNEL_STATUS_OFFSET * Index),
			InterruptMask->Mask[Index]);
	}
}

/****************************************************************************/
/**
*
* Gets interrupt mask.
*
* @param    InstancePtr Pointer to the Equalizer instance.
* @param    InterruptMask Equalizer interrupt mask container.
*
****************************************************************************/
void XDfeEqu_GetInterruptMask(const XDfeEqu *InstancePtr,
			      XDfeEqu_InterruptMask *InterruptMask)
{
	u32 Index;
	u32 Offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InterruptMask != NULL);

	/* Reads the Status Mask register */
	Offset = XDFEEQU_CHANNEL_0_STATUS_MASK_OFFSET;
	for (Index = 0; Index < XDFEEQU_CHANNEL_NUM; Index++) {
		InterruptMask->Mask[Index] = XDfeEqu_ReadReg(
			InstancePtr,
			Offset + (XDFEEQU_CHANNEL_STATUS_OFFSET * Index));
	}
}

/** @} */
