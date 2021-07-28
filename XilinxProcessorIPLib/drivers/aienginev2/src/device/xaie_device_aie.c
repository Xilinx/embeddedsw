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
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_clock.h"
#include "xaie_tilectrl.h"

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API sets the column clock control register. Its configuration affects
* (enable or disable) all tile's clock above the Shim tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE SHIM tile
* @param        Enable: XAIE_ENABLE to enable column global clock buffer,
*                       XAIE_DISABLE to disable.
*
* @return       XAIE_OK for success, and error code for failure.
*
* @note         It is not required to check the DevInst and the Loc tile type
*               as the caller function should provide the correct value.
*               It is internal function to this file
*
******************************************************************************/
static AieRC _XAie_PmSetColumnClockBuffer(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Enable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimClkBufCntr *ClkBufCntr;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ClkBufCntr = PlIfMod->ClkBufCntr;

	RegAddr = ClkBufCntr->RegOff +
			_XAie_GetTileAddr(DevInst, 0U, Loc.Col);
	FldVal = XAie_SetField(Enable, ClkBufCntr->ClkBufEnable.Lsb,
			ClkBufCntr->ClkBufEnable.Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, ClkBufCntr->ClkBufEnable.Mask,
			FldVal);
}

/*****************************************************************************/
/*
* This API enables clock for all tiles in the given device instance.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @param        Enable: XAIE_ENABLE to enable column global clock buffer,
*               XAIE_DISABLE to disable.
*
* @note         This is INTERNAL API.
*
*******************************************************************************/
AieRC _XAie_PmSetPartitionClock(XAie_DevInst *DevInst, u8 Enable)
{
	for(u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc;
		AieRC RC;

		Loc = XAie_TileLoc(C, 0);
		RC = _XAie_PmSetColumnClockBuffer(DevInst, Loc, Enable);
		if (RC != XAIE_OK) {
			XAIE_ERROR("Failed to set partition clock buffers.\n");
			return RC;
		}
	}

	return XAIE_OK;
}

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
AieRC _XAie_SetPartColClockAfterRst(XAie_DevInst *DevInst, u8 Enable)
{
	AieRC RC;

	if(Enable == XAIE_ENABLE) {
		/* Column clocks are enabled by default for aie device */
		return XAIE_OK;
	}

	RC = _XAie_PmSetPartitionClock(DevInst, XAIE_DISABLE);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to disable clock buffers.\n");
	}

	return RC;
}

/*****************************************************************************/
/**
*
* This API sets isolation boundry of an AI engine partition after reset
*
* @param	DevInst: Device Instance
*
* @return       XAIE_OK on success, error code on failure
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*		Internal API only.
*
******************************************************************************/
AieRC _XAie_SetPartIsolationAfterRst(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		u8 Dir = 0;

		if(C == 0) {
			Dir = XAIE_ISOLATE_WEST_MASK;
		} else if(C == (u8)(DevInst->NumCols - 1)) {
			Dir = XAIE_ISOLATE_EAST_MASK;
		} else {
			/* No isolation for tiles by default for AIE */
			continue;
		}

		for(u8 R = 0; R < DevInst->NumRows; R++) {
			RC = _XAie_TileCtrlSetIsolation(DevInst,
					XAie_TileLoc(C, R), Dir);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to set partition isolation.\n");
				return RC;
			}
		}
	}

	return RC;
}

/*****************************************************************************/
/**
*
* This API initialize the memories of the partition to zero.
*
* @param	DevInst: Device Instance
*
* @return       XAIE_OK on success, error code on failure
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*		Internal API only.
*
******************************************************************************/
AieRC _XAie_PartMemZeroInit(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	const XAie_CoreMod *CoreMod;
	const XAie_MemMod *MemMod;

	CoreMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;
	MemMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].MemMod;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		for(u8 R = 1; R < DevInst->NumRows; R++) {
			u64 RegAddr;

			/* Zeroize program memory */
			RegAddr = CoreMod->ProgMemHostOffset +
				_XAie_GetTileAddr(DevInst, R, C);
			RC = XAie_BlockSet32(DevInst, RegAddr, 0,
					CoreMod->ProgMemSize / sizeof(u32));
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to zeroize partition.\n");
				return RC;
			}

			/* Zeroize data memory */
			RegAddr = MemMod->MemAddr +
				_XAie_GetTileAddr(DevInst, R, C);
			RC = XAie_BlockSet32(DevInst, RegAddr, 0,
					MemMod->Size / sizeof(u32));
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to zeroize partition.\n");
				return RC;
			}
		}
	}

	return RC;
}

/*****************************************************************************/
/*
* This is an API to gate clocks in tiles from the topmost row to the row above
* the Location passed as argument in that column. In AIE HW, the control of
* clock gating a tile is present in the tile below that tile in that column.
* Hence gating of clocks of unused tiles in a col happens from top.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE tile
* @return	XAIE_OK on success
*
* @note		None
*
*******************************************************************************/
static void _XAie_PmGateTiles(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	for (u8 R = DevInst->NumRows - 1; R > Loc.Row; R--) {
		u8 TileType;
		u64 RegAddr;
		const XAie_ClockMod *ClockMod;
		XAie_LocType TileLoc;

		TileLoc.Col = Loc.Col;
		TileLoc.Row = R - 1;
		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, TileLoc);
		ClockMod = DevInst->DevProp.DevMod[TileType].ClockMod;
		RegAddr = _XAie_GetTileAddr(DevInst, TileLoc.Row, TileLoc.Col) +
				ClockMod->ClockRegOff;
		XAie_MaskWrite32(DevInst, RegAddr,
				ClockMod->NextTileClockCntrl.Mask, 0U);
	}
}

/*****************************************************************************/
/*
* This is an API to enable clocks for tiles from FromLoc to ToLoc. In AIE HW,
* the control of clock gating a tile is present in the tile below that tile
* in that column. Hence ungating clocks in tiles happen bottom up.
*
* @param	DevInst: Device Instance
* @param	FromLoc: Location of tile to ungate from.
* @param	ToLoc: Location of tile to ungate to.
* @return	XAIE_OK on success
*
* @note		None
*
*******************************************************************************/
static void _XAie_PmUngateTiles(XAie_DevInst *DevInst, XAie_LocType FromLoc,
		XAie_LocType ToLoc)
{
	for (u8 R = FromLoc.Row; R < ToLoc.Row; R++) {
		XAie_LocType TileLoc;
		u8 TileType;
		u64 RegAddr;
		const XAie_ClockMod *ClockMod;

		TileLoc.Col = FromLoc.Col;
		TileLoc.Row = R;
		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, TileLoc);
		ClockMod = DevInst->DevProp.DevMod[TileType].ClockMod;
		RegAddr = _XAie_GetTileAddr(DevInst, TileLoc.Row, TileLoc.Col) +
				ClockMod->ClockRegOff;
		XAie_MaskWrite32(DevInst, RegAddr,
				ClockMod->NextTileClockCntrl.Mask,
				1 << ClockMod->NextTileClockCntrl.Lsb);
	}
}

/*****************************************************************************/
/**
* This API enables clock for all the tiles passed as argument to this API.
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Args: Backend tile args
*
* @return	XAIE_OK on success, error code on failure
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RequestTiles(XAie_DevInst *DevInst, XAie_BackendTilesArray *Args)
{
	u32 SetTileStatus;

	if(Args->Locs == NULL) {
		u32 NumTiles;

		XAie_LocType TileLoc = XAie_TileLoc(0, 1);
		NumTiles = (DevInst->NumRows - 1) * (DevInst->NumCols);

		SetTileStatus = _XAie_GetTileBitPosFromLoc(DevInst, TileLoc);
		_XAie_SetBitInBitmap(DevInst->TilesInUse, SetTileStatus,
				NumTiles);
		_XAie_PmSetPartitionClock(DevInst, XAIE_ENABLE);

		return XAIE_OK;
	}

	for(u32 i = 0; i < Args->NumTiles; i++) {
		u8 flag = 0;

		if(Args->Locs[i].Row == 0)
			continue;

		/* Calculate bit number in bit map for the tile requested */
		SetTileStatus = _XAie_GetTileBitPosFromLoc(DevInst,
				Args->Locs[i]);

		for(u32 row = DevInst->NumRows - 1U; row > 0U; row--) {
			u32 CheckTileStatus;
			/*
			 * Check for the upper most tile in use in the column
			 * of the tile requested.
			 */
			XAie_LocType TileLoc;

			TileLoc.Col = Args->Locs[i].Col;
			TileLoc.Row = row;
			CheckTileStatus = _XAie_GetTileBitPosFromLoc(DevInst,
					TileLoc);
			if(CheckBit(DevInst->TilesInUse, CheckTileStatus)) {
				flag = 1;
				if(SetTileStatus > CheckTileStatus) {
					XAie_LocType ToLoc, FromLoc;
					ToLoc.Col = Args->Locs[i].Col;
					ToLoc.Row = Args->Locs[i].Row;
					FromLoc.Col = Args->Locs[i].Col;
					FromLoc.Row = row;
					_XAie_PmUngateTiles(DevInst,
							FromLoc, ToLoc);
				}
				break;
			}
		}

		if(flag == 0) {
			XAie_LocType TileLoc;
			TileLoc.Col = Args->Locs[i].Col;
			TileLoc.Row = 0U;
			/* Ungate the shim tile of that column */
			_XAie_PmSetColumnClockBuffer(DevInst, TileLoc,
					XAIE_ENABLE);
			/* Gate unused tiles from top to uppermost tile inuse */
			TileLoc.Row = Args->Locs[i].Row;
			_XAie_PmGateTiles(DevInst, TileLoc);
		}
		/*
		 * Mark the tile and below are ungated.
		 * Assuming the row starts from 0.
		 */
		_XAie_SetBitInBitmap(DevInst->TilesInUse,
				SetTileStatus - Args->Locs[i].Row + 1,
				Args->Locs[i].Row);
	}

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
/** @} */
