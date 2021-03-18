/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_rsc.c
* @{
*
* This file contains routines for AIE resource manager
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 01/11/2021  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdlib.h>

#include "xaie_rsc.h"
#include "xaie_rsc_internal.h"
#include "xaie_helper.h"
/*****************************************************************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API deallocates memory for all resource bitmaps.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RscMgrFinish(XAie_DevInst *DevInst)
{
	_XAie_PerfCntRscFinish(DevInst);
	_XAie_UserEventsRscFinish(DevInst);

	free(DevInst->RscMapping);
	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API initializes all resource bitmaps.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RscMgrInit(XAie_DevInst *DevInst)
{
	AieRC RC;

	DevInst->RscMapping = malloc(sizeof(*(DevInst->RscMapping)) *
		XAIEGBL_TILE_TYPE_MAX);
	if(DevInst->RscMapping == NULL) {
		XAIE_ERROR("Unable to allocate memory for bitmaps\n");
		return XAIE_ERR;
	}

	RC = _XAie_PerfCntRscInit(DevInst);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to allocate memory for perfcnt bitmaps\n");
		return XAIE_ERR;
	}

	RC = _XAie_UserEventsRscInit(DevInst);
	if(RC != XAIE_OK) {
		goto CleanPerfCntRsc;
	}

	return RC;

CleanPerfCntRsc:
	_XAie_PerfCntRscFinish(DevInst);

	return RC;
}

/*****************************************************************************/
/**
* This API checks validity for the given list of tiles.
*
* @param	DevInst: Device Instance
* @param	NumReq: Number of tiles to be validated
* @param	RscReq: Pointer to request containing locs to be validated
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_CheckLocsValidity(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq)
{
	for(u32 j = 0; j < NumReq; j++) {
		if(RscReq[j].Loc.Row >= DevInst->NumRows ||
			RscReq[j].Loc.Col >= DevInst->NumCols) {
			XAIE_ERROR("Invalid Loc Col:%d Row:%d\n",
					RscReq[j].Loc.Col, RscReq[j].Loc.Row);
			return XAIE_INVALID_ARGS;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API calculates index for the start bit in a resource bitmap.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of tile
* @param	MaxRscVal: Number of resource per tile
*
* @return	Start bit index
*
* @note		Internal only.
*
*******************************************************************************/
u32 _XAie_GetStartBit(XAie_DevInst *DevInst, XAie_LocType Loc, u32 MaxRscVal)
{
	u8 TileType;
	u32 StartRow, BitmapNumRows;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	StartRow = _XAie_GetStartRow(DevInst, TileType);
	BitmapNumRows = _XAie_GetNumRows(DevInst, TileType);

	return _XAie_GetTileRscStartBitPosFromLoc(BitmapNumRows, Loc,
		MaxRscVal, StartRow);
}

/** @} */
