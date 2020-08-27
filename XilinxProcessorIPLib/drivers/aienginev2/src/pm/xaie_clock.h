/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_clock.h
* @{
*
* Header file for timer implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date        Changes
* ----- ------   --------    --------------------------------------------------
* 1.0   Dishita  06/26/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_CLOCK_H
#define XAIE_CLOCK_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"

/************************** Enum *********************************************/

/************************** Function Prototypes  *****************************/
void _XAie_PmSetPartitionClock(XAie_DevInst *DevInst, u8 Enable);
AieRC XAie_PmRequestTiles(XAie_DevInst *DevInst, XAie_LocType *Loc,
		u32 NumTiles);
AieRC XAie_PmReleaseTiles(XAie_DevInst *DevInst, XAie_LocType *Loc,
		u32 NumTiles);
u8 _XAie_PmIsTileRequested(XAie_DevInst *DevInst, XAie_LocType Loc);
#endif		/* end of protection macro */
