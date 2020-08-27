/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_interrupt.h
* @{
*
* Header file for AIE interrupt module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad  07/21/2020  Initial creation
* 1.1   Nishad  07/23/2020  Add APIs to configure second level interrupt
*			    controller.
* 1.2   Nishad  07/23/2020  Add API to initialize error broadcast network.
* 1.3   Nishad  08/13/2020  Add macro for error broadcast mask.
* </pre>
*
******************************************************************************/
#ifndef XAIE_INTERRUPT_H
#define XAIE_INTERRUPT_H

/***************************** Include Files *********************************/
#include "xaie_events.h"

/**************************** Type Definitions *******************************/
#define XAIE_ERROR_BROADCAST_ID			0x0U
#define XAIE_ERROR_BROADCAST_MASK		0x1U
#define XAIE_ERROR_SHIM_INTR_ID			0x10U
#define XAIE_ERROR_NPI_INTR_ID			0x1U
#define XAIE_ERROR_L2_ENABLE			0x7FU

/************************** Function Prototypes  *****************************/
AieRC XAie_IntrCtrlL1Enable(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IntrId);
AieRC XAie_IntrCtrlL1Disable(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IntrId);
AieRC XAie_IntrCtrlL1IrqSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 BroadcastId);
AieRC XAie_IntrCtrlL1Event(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IrqEventId, XAie_Events Event);
AieRC XAie_IntrCtrlL1BroadcastBlock(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u32 ChannelBitMap);
AieRC XAie_IntrCtrlL1BroadcastUnblock(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u32 ChannelBitMap);
AieRC XAie_IntrCtrlL2Enable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 ChannelBitMap);
AieRC XAie_IntrCtrlL2Disable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 ChannelBitMap);
AieRC XAie_IntrCtrlL2IrqSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 NoCIrqId);
AieRC XAie_ErrorHandlingInit(XAie_DevInst *DevInst);

#endif		/* end of protection macro */
