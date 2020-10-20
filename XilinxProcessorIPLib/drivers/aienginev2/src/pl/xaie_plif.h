/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_plif.h
* @{
*
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   10/28/2019  Initial creation
* 1.1   Tejus   03/16/2020  Implementation of apis for Mux/Demux configuration
* 1.2   Tejus   03/20/2020  Remove range apis
* </pre>
*
******************************************************************************/
#ifndef XAIEPLIF_H
#define XAIEPLIF_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
/**************************** Type Definitions *******************************/
/*
 * This enum captures the AIE-PL interface bit widths available in the hardware.
 */
typedef enum {
	PLIF_WIDTH_32 = 32,
	PLIF_WIDTH_64 = 64,
	PLIF_WIDTH_128 = 128
} XAie_PlIfWidth;

/************************** Function Prototypes  *****************************/
AieRC XAie_PlIfBliBypassEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_PlIfBliBypassDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_PlIfDownSzrEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_PlIfDownSzrDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_PlToAieIntfEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_PlToAieIntfDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_AieToPlIntfEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_AieToPlIntfDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_EnableShimDmaToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_EnableAieToShimDmaStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_EnableNoCToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_EnableAieToNoCStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_EnablePlToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_EnableAieToPlStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
#endif		/* end of protection macro */
