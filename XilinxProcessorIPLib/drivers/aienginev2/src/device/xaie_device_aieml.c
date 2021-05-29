/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_device_aieml.c
* @{
*
* This file contains the apis for device specific operations of aieml.
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
#include "xaie_clock.h"

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
u8 _XAieMl_GetTTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
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

	} else if(Loc.Row >= DevInst->MemTileRowStart &&
			(Loc.Row < (DevInst->MemTileRowStart +
				     DevInst->MemTileNumRows))) {
		return XAIEGBL_TILE_TYPE_MEMTILE;
	} else if (Loc.Row >= DevInst->AieTileRowStart &&
			(Loc.Row < (DevInst->AieTileRowStart +
				     DevInst->AieTileNumRows))) {
		return XAIEGBL_TILE_TYPE_AIETILE;
	}

	XAIE_ERROR("Cannot find Tile Type\n");

	return XAIEGBL_TILE_TYPE_MAX;
}

/*****************************************************************************/
/**
*
* This API sets the reset bit of SHIM for the specified partition.
*
* @param	DevInst: Device Instance
* @param	Enable: Indicate if to enable SHIM reset or disable SHIM reset
*			XAIE_ENABLE to enable SHIM reset, XAIE_DISABLE to
*			disable SHIM reset.
*
* @return	XAIE_OK
*
* @note		Internal API only. Always returns XAIE_OK, as there is nothing
*		need to be done for aieml device.
*
******************************************************************************/
AieRC _XAieMl_SetPartColShimReset(XAie_DevInst *DevInst, u8 Enable)
{
	(void)DevInst;
	(void)Enable;
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API sets column clock buffers after SHIM is reset.
*
* @param	DevInst: Device Instance
* @param	Enable: Indicate if to enable clock buffers or disable them.
*			XAIE_ENABLE to enable clock buffers, XAIE_DISABLE to
*			disable.
*
* @return	XAIE_OK for success, and error code for failure
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAieMl_SetPartColClockAfterRst(XAie_DevInst *DevInst, u8 Enable)
{
	AieRC RC;

	if(Enable == XAIE_DISABLE) {
		/* Column  clocks are disable by default for aieml device */
		return XAIE_OK;
	}

	RC = _XAie_PmSetPartitionClock(DevInst, XAIE_ENABLE);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to enable clock buffers.\n");
	}

	return RC;
}

/** @} */
