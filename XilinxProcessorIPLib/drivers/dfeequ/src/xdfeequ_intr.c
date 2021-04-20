/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeequ_intr.c
* @addtogroup xdfeequ_v1_0
* @{
*
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
*
* </pre>
*
******************************************************************************/

#include "xdfeequ.h"
#include "xdfeequ_hw.h"

/**************************** Macros Definitions ****************************/

/************************** Function Prototypes *****************************/
extern u32 XDfeEqu_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data);
extern u32 XDfeEqu_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data,
			      u32 Val);

/****************************************************************************/
/**
*
* Gets Equalizer event status for channel ID.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    ChannelId is a channel ID.
* @param    Status is an Equalizer status container.
*
*
****************************************************************************/
void XDfeEqu_GetEventStatus(const XDfeEqu *InstancePtr, u32 ChannelId,
			    XDfeEqu_Status *Status)
{
	u32 Data;
	u32 Offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ChannelId < XDFEEQU_CHANNEL_NUM);
	Xil_AssertVoid(Status != NULL);

	/* Reads the relevant bits of the Channel_Id Status register. Populates
	   Status.i_status and Status.q_status. */
	Offset = XDFEEQU_CHANNEL_0_STATUS_OFFSET +
		 (XDFEEQU_CHANNEL_STATUS_OFFSET * ChannelId);
	Data = XDfeEqu_ReadReg(InstancePtr, Offset);
	Status->IStatus = Data & XDFEEQU_CHANNEL_I_STATUS_MASK;
	Status->QStatus = (Data & XDFEEQU_CHANNEL_Q_STATUS_MASK) >>
			  XDFEEQU_CHANNEL_Q_STATUS_OFFSET;
}

/****************************************************************************/
/**
*
* Clears Equalizer status for channel ID.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    ChannelId bits indicate which channel is enabled.
*
****************************************************************************/
void XDfeEqu_ClearEventStatus(const XDfeEqu *InstancePtr, u32 ChannelId)
{
	u32 Index;
	u32 Offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ChannelId < ((u32)1U << XDFEEQU_CHANNEL_NUM));

	/* Clears the relevant bits of the Channel_Id Status register. */
	for (Index = 0; Index < XDFEEQU_CHANNEL_NUM; Index++) {
		if (0U != (ChannelId & ((u32)1U << Index))) {
			Offset = XDFEEQU_CHANNEL_0_STATUS_MASK_OFFSET +
				 (0x10U * Index);
			XDfeEqu_WriteReg(InstancePtr, Offset, 0U);
		}
	}
}

/****************************************************************************/
/**
*
* Enables an Equalizer status for channel ID.
*
* @param    InstancePtr is a pointer to the Equalizer instance.
* @param    ChannelField bits indicate which channel is enabled.
* @param    StatusMask is an Equalizer status mask container.
*
****************************************************************************/
void XDfeEqu_SetInterruptMask(const XDfeEqu *InstancePtr, u32 ChannelField,
			      const XDfeEqu_InterruptMask *StatusMask)
{
	u32 Index;
	u32 Offset;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ChannelField < ((u32)1U << XDFEEQU_CHANNEL_NUM));
	Xil_AssertVoid(StatusMask->Status <
		       (1U << XDFEEQU_CHANNEL_STATUS_MASK_ENABLE_WIDTH));

	/* Sets the Status Mask register for each channel selected in Channel_Field. */
	for (Index = 0; Index < XDFEEQU_CHANNEL_NUM; Index++) {
		if (0U != (ChannelField & ((u32)1U << Index))) {
			Offset = XDFEEQU_CHANNEL_0_STATUS_MASK_OFFSET +
				 (XDFEEQU_CHANNEL_STATUS_OFFSET * Index);
			XDfeEqu_WriteReg(InstancePtr, Offset,
					 StatusMask->Status);
		}
	}
}

/** @} */
