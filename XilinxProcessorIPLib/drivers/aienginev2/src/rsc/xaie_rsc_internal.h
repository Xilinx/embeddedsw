/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_rsc_internal.h
* @{
*
* Header file for resource manager implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date        Changes
* ----- ------   --------    --------------------------------------------------
* 1.0   Dishita  01/11/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_RSC_INTERNAL_H
#define XAIE_RSC_INTERNAL_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"

/************************** Enum *********************************************/

/************************** Function Prototypes  *****************************/
/* Global resource management APIs */
AieRC _XAie_RscMgrInit(XAie_DevInst *DevInst);
AieRC _XAie_RscMgrFinish(XAie_DevInst *DevInst);

/* Performance counter resource management internal APIs */
AieRC _XAie_PerfCntRscInit(XAie_DevInst *DevInst);
void _XAie_PerfCntRscFinish(XAie_DevInst *DevInst);

/* Common resource management helper APIs */
AieRC _XAie_CheckLocsValidity(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq);
u32 _XAie_GetStartBit(XAie_DevInst *DevInst, XAie_LocType Loc, u32 MaxRscVal);

/*****************************************************************************/
/**
*
* Rounds the BitmapSize up to the nearest multiple of n.
*
* @param        Val: value to be aligned
* @param        Aligned: multiple to align to
*
* @return       Aligned result
*
* @note         Internal API only.
*
******************************************************************************/
static inline u32 _XAie_NearestRoundUp(u32 Val, u32 Aligned)
{
	return (Aligned * ((Val + (Aligned - 1U)) / Aligned));
}

/*****************************************************************************/
/**
* This API calculates core module offset in bitmap.
*
* @param        DevInst: Device Instance
* @param        MaxRscVal: Number of resource per tile.
*
* @return       Index for start of core module bitmap.
*
* @note         Internal only.
*
*******************************************************************************/
static inline u32 _XAie_GetCoreBitmapOffset(XAie_DevInst *DevInst, u32 MaxRscVal)
{
	return MaxRscVal * DevInst->AieTileNumRows * DevInst->NumCols * 2U;
}

/*****************************************************************************/
/**
* This API calculates resource index in bitmap based on location and number of
* resource per tile.
*
* @param        BitmapNumRows: Number of rows based on tiletype
* @param        Loc: Location of AIE tile
* @param        MaxRscVal: Number of resource per tile.
* @param        StartRow: Start row for tiletype
*
* @return       Index for the resource start bit in bitmap
*
* @note         Internal only.
*
*******************************************************************************/
static inline u32 _XAie_GetTileRscStartBitPosFromLoc(u32 BitmapNumRows,
		XAie_LocType Loc, u32 MaxRscVal, u32 StartRow)
{
	return ((Loc.Col * BitmapNumRows + (Loc.Row - StartRow)) * MaxRscVal);
}

#endif		/* end of protection macro */
