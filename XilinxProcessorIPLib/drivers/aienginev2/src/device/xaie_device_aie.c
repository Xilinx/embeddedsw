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

/*****************************************************************************/
/**
*
* This API set the SHIM tile reset
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE SHIM tile
* @param	RstEnable: XAIE_ENABLE to enable reset, XAIE_DISABLE to
*			   disable reset.
*
* @return	XAIE_OK for success, and error code for failure
*
* @note		It is not required to check the DevInst and the Loc tile type
*		as the caller function should provide the correct value.
*
******************************************************************************/
static AieRC _XAie_SetShimReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 RstEnable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimRstMod *ShimTileRst;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ShimTileRst = PlIfMod->ShimTileRst;

	RegAddr = ShimTileRst->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	FldVal = XAie_SetField(RstEnable,
			ShimTileRst->RstCntr.Lsb,
			ShimTileRst->RstCntr.Mask);

	return XAie_Write32(DevInst, RegAddr, FldVal);
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
* @return	XAIE_OK for success, and error code for failure
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAie_SetPartColShimReset(XAie_DevInst *DevInst, u8 Enable)
{
	for(u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);
		AieRC RC;

		RC = _XAie_SetShimReset(DevInst, Loc, Enable);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to set SHIM resets.\n");
			return RC;
		}
	}

	return XAIE_OK;
}

/** @} */
