/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_device_aie.c
* @{
*
* This file contains the apis for device specific operations of aie.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   05/03/2021  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the function used to get the tile type for a given device instance
* and tile location.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	TileType (AIETILE/MEMTILE/SHIMPL/SHIMNOC on success and MAX on
*		error)
*
* @note		Internal API only. This API returns tile type based on
*		SHIMPL-SHIMPL-SHIMNOC-SHIMNOC pattern
*
******************************************************************************/
u8 _XAie_GetTTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 ColType;

	if(Loc.Col >= DevInst->NumCols) {
		XAIE_ERROR("Invalid column: %d\n", Loc.Col);
		return XAIEGBL_TILE_TYPE_MAX;
	}

	if(Loc.Row == 0U) {
		ColType = Loc.Col % 4U;
		if((ColType == 0U) || (ColType == 1U)) {
			return XAIEGBL_TILE_TYPE_SHIMPL;
		}

		return XAIEGBL_TILE_TYPE_SHIMNOC;

	} else if(Loc.Row >= DevInst->ReservedRowStart &&
			(Loc.Row < (DevInst->ReservedRowStart +
				     DevInst->ReservedNumRows))) {
		return XAIEGBL_TILE_TYPE_RESERVED;
	} else if (Loc.Row >= DevInst->AieTileRowStart &&
			(Loc.Row < (DevInst->AieTileRowStart +
				     DevInst->AieTileNumRows))) {
		return XAIEGBL_TILE_TYPE_AIETILE;
	}

	XAIE_ERROR("Cannot find Tile Type\n");

	return XAIEGBL_TILE_TYPE_MAX;
}

/** @} */
