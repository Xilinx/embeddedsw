/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_clock.c
* @{
*
* This file contains routines for AIE timers
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 06/26/2020  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_clock.h"
#include "xaie_helper.h"

/*****************************************************************************/
/***************************** Macro Definitions *****************************/
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
* @return       none
*
* @note         It is not required to check the DevInst and the Loc tile type
*               as the caller function should provide the correct value.
*
******************************************************************************/
static void  _XAie_PmSetColumnClockBuffer(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Enable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimClkBufCntr *ClkBufCntr;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ClkBufCntr = PlIfMod->ClkBufCntr;

	RegAddr = ClkBufCntr->RegOff +
			_XAie_GetTileAddr(DevInst, 0U, Loc.Col);
	FldVal = XAie_SetField(Enable, ClkBufCntr->ClkBufEnable.Lsb,
			ClkBufCntr->ClkBufEnable.Mask);

	XAie_MaskWrite32(DevInst, RegAddr, ClkBufCntr->ClkBufEnable.Mask,
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
* @note         None
*
*******************************************************************************/
void _XAie_PmSetPartitionClock(XAie_DevInst *DevInst, u8 Enable)
{
	for(u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc;

		Loc = XAie_TileLoc(C, 0);
		_XAie_PmSetColumnClockBuffer(DevInst, Loc, Enable);
	}
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
		TileType = _XAie_GetTileTypefromLoc(DevInst, TileLoc);
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
		TileType = _XAie_GetTileTypefromLoc(DevInst, TileLoc);
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
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE tile
* @param	NumTiles: Number of tiles requested for use.
*
* @return	XAIE_OK on success.
*
* @note		As all the tiles are gated when the system boots, this function
*		needs to be called after device instance is initialized and
*		before any other AI engine operations. Otherwise, the other
*		AI engine functions may access gated tiles.
*
*******************************************************************************/
AieRC XAie_PmRequestTiles(XAie_DevInst *DevInst, XAie_LocType *Loc,
		u32 NumTiles)
{
	u32 SetTileStatus, CheckTileStatus;
	XAie_BackendTilesArray TilesArray;
	AieRC RC;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE) {
		XAIE_ERROR("Clock gating not supported\n");
		return XAIE_ERR;
	}

	if(NumTiles > (DevInst->NumRows * DevInst->NumCols)) {
		XAIE_ERROR("Invalid NumTiles\n");
		return XAIE_INVALID_ARGS;
	}

	if (NumTiles != 0 && Loc == NULL) {
		XAIE_ERROR("NumTiles is not 0, but Location array is empty.\n");
		return XAIE_INVALID_ARGS;
	}

	/* Check validity of all tiles in the list passed to this API */
	for(u32 j = 0; j < NumTiles; j++) {
		if(Loc[j].Row >= DevInst->NumRows ||
			Loc[j].Col >= DevInst->NumCols) {
			XAIE_ERROR("Invalid Loc Col:%d Row:%d\n", Loc[j].Col, Loc[j].Row);
			return XAIE_INVALID_ARGS;
		}
	}

	TilesArray.NumTiles = NumTiles;
	TilesArray.Locs = Loc;
	/*
	 * Try IO backend request tiles operation first. If the operation is
	 * not supported, use the common implementation.
	 */
	RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_REQUEST_TILES,
			(void *)&TilesArray);
	if (RC != XAIE_FEATURE_NOT_SUPPORTED) {
		if (RC != XAIE_OK) {
			XAIE_ERROR("Failed to request tiles from backend.\n");
			return RC;
		}

		/* Setup the requested tiles bitmap locally */
		if (Loc == NULL) {
			u32 StartBit;

			NumTiles = DevInst->NumCols * (DevInst->NumRows - 1);
			/* Loc is NULL, it suggests all tiles are requested */
			StartBit = _XAie_GetTileBitPosFromLoc(DevInst,
						XAie_TileLoc(0, 1));
			_XAie_SetBitInBitmap(DevInst->TilesInUse, StartBit,
					NumTiles);
		} else {
			for(u32 i = 0; i < NumTiles; i++) {
				u32 Bit;

				if(Loc[i].Row == 0) {
					continue;
				}

				/*
				 * If a tile is ungated, the rows below it are
				 * ungated.
				 */
				Bit = _XAie_GetTileBitPosFromLoc(DevInst,
						XAie_TileLoc(Loc[i].Col, 1));
				_XAie_SetBitInBitmap(DevInst->TilesInUse,
						Bit, Loc[i].Row);
			}
		}

		return XAIE_OK;
	}

	/*
	 * Passing empty list to enable all tiles of device instance is
	 * temporary and will be removed soon.
	 */
	if(Loc == NULL) {
		u32 NumTiles;

		XAie_LocType TileLoc = XAie_TileLoc(0, 1);
		NumTiles = (DevInst->NumRows - 1) * (DevInst->NumCols);

		SetTileStatus = _XAie_GetTileBitPosFromLoc(DevInst, TileLoc);
		_XAie_SetBitInBitmap(DevInst->TilesInUse, SetTileStatus, NumTiles);
		_XAie_PmSetPartitionClock(DevInst, XAIE_ENABLE);

		return XAIE_OK;
	}

	for(u32 i = 0; i < NumTiles; i++) {
		u8 flag = 0;

		if(Loc[i].Row == 0)
			continue;

		/* Calculate bit number in bit map for the tile requested */
		SetTileStatus = _XAie_GetTileBitPosFromLoc(DevInst, Loc[i]);

		for(u32 row = DevInst->NumRows - 1U; row > 0U; row--) {
			/*
			 * Check for the upper most tile in use in the column
			 * of the tile requested.
			 */
			XAie_LocType TileLoc;

			TileLoc.Col = Loc[i].Col;
			TileLoc.Row = row;
			CheckTileStatus = _XAie_GetTileBitPosFromLoc(DevInst,
					TileLoc);
			if(CheckBit(DevInst->TilesInUse, CheckTileStatus)) {
				flag = 1;
				if(SetTileStatus > CheckTileStatus) {
					XAie_LocType ToLoc, FromLoc;
					ToLoc.Col = Loc[i].Col;
					ToLoc.Row = Loc[i].Row;
					FromLoc.Col = Loc[i].Col;
					FromLoc.Row = row;
					_XAie_PmUngateTiles(DevInst,
							FromLoc, ToLoc);
				}
				break;
			}
		}

		if(flag == 0) {
			XAie_LocType TileLoc;
			TileLoc.Col = Loc[i].Col;
			TileLoc.Row = 0U;
			/* Ungate the shim tile of that column */
			_XAie_PmSetColumnClockBuffer(DevInst, TileLoc,
					XAIE_ENABLE);
			/* Gate unused tiles from top to uppermost tile inuse */
			TileLoc.Row = Loc[i].Row;
			_XAie_PmGateTiles(DevInst, TileLoc);
		}
		/*
		 * Mark the tile and below are ungated.
		 * Assuming the row starts from 0.
		 */
		_XAie_SetBitInBitmap(DevInst->TilesInUse,
			SetTileStatus - Loc[i].Row + 1, Loc[i].Row);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API checks if an AI engine tile is in use.
*
* @param	DevInst: Device Instance
* @param	Loc: tile location
*
* @return	XAIE_ENABLE if a tile is in use, otherwise XAIE_DISABLE.
*
* @note		This API is supposed to be called internal only.
*******************************************************************************/
u8 _XAie_PmIsTileRequested(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType;
	u32 TileBit;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_DISABLE;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if (TileType == XAIEGBL_TILE_TYPE_MAX) {
		return XAIE_DISABLE;
	}

	if (TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
	    TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		return XAIE_ENABLE;
	}

	TileBit = Loc.Col * (DevInst->NumRows - 1) + Loc.Row - 1;
	if (CheckBit(DevInst->TilesInUse, TileBit)) {
		return XAIE_ENABLE;
	}

	return XAIE_DISABLE;
}

/** @} */
