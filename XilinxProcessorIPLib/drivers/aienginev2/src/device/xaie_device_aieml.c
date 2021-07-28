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
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_clock.h"
#include "xaie_tilectrl.h"

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
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
AieRC _XAieMl_SetPartIsolationAfterRst(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		u8 Dir = 0;

		if(C == 0) {
			Dir = XAIE_ISOLATE_WEST_MASK;
		} else if(C == (u8)(DevInst->NumCols - 1)) {
			Dir = XAIE_ISOLATE_EAST_MASK;
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
AieRC _XAieMl_PartMemZeroInit(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	const XAie_MemCtrlMod *MCtrlMod, *MCtrlModLast;
	u64 RegAddr;
	XAie_LocType Loc;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		for(u8 R = 1; R < DevInst->NumRows; R++) {
			u32 FldVal;
			u8 TileType, NumMods;

			Loc = XAie_TileLoc(C, R);
			TileType = DevInst->DevOps->GetTTypefromLoc(DevInst,
					Loc);
			NumMods = DevInst->DevProp.DevMod[TileType].NumModules;
			MCtrlMod = DevInst->DevProp.DevMod[TileType].MemCtrlMod;
			for (u8 M = 0; M < NumMods; M++) {
				RegAddr = MCtrlMod->MemCtrlRegOff +
					_XAie_GetTileAddr(DevInst, R, C);
				FldVal = XAie_SetField(XAIE_ENABLE,
					MCtrlMod->MemZeroisation.Lsb,
					MCtrlMod->MemZeroisation.Mask);
				RC = XAie_MaskWrite32(DevInst, RegAddr,
					MCtrlMod->MemZeroisation.Mask,
					FldVal);
				if(RC != XAIE_OK) {
					XAIE_ERROR("Failed to zeroize partition mems.\n");
					return RC;
				}
				MCtrlModLast = MCtrlMod;
				MCtrlMod++;
			}
		}
	}

	RegAddr = MCtrlModLast->MemCtrlRegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	return XAie_MaskPoll(DevInst, RegAddr,
			MCtrlModLast->MemZeroisation.Mask, 0, 0);

}

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
static AieRC _XAieMl_PmSetColumnClockBuffer(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Enable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	XAie_LocType ShimLoc = XAie_TileLoc(Loc.Col, 0U);
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimClkBufCntr *ClkBufCntr;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, ShimLoc);
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
/**
* This API enables clock for all the tiles passed as argument to this API.
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Args: Backend tile args
*
* @return       XAIE_OK on success, error code on failure
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAieMl_RequestTiles(XAie_DevInst *DevInst, XAie_BackendTilesArray *Args)
{
	AieRC RC;
	u32 SetTileStatus;


	if(Args->Locs == NULL) {
		u32 NumTiles;

		XAie_LocType TileLoc = XAie_TileLoc(0, 1);
		NumTiles = (DevInst->NumRows - 1) * (DevInst->NumCols);

		SetTileStatus = _XAie_GetTileBitPosFromLoc(DevInst, TileLoc);
		_XAie_SetBitInBitmap(DevInst->TilesInUse, SetTileStatus,
				NumTiles);

		return DevInst->DevOps->SetPartColClockAfterRst(DevInst,
				XAIE_ENABLE);
	}

	for(u32 i = 0; i < Args->NumTiles; i++) {
		u32 ColClockStatus;

		if(Args->Locs[i].Row == 0)
			continue;

		/*
		 * Check if column clock buffer is already enabled and continue
		 */
		ColClockStatus = _XAie_GetTileBitPosFromLoc(DevInst,
				Args->Locs[i]);
		if(CheckBit(DevInst->TilesInUse, ColClockStatus))
			continue;

		RC = _XAieMl_PmSetColumnClockBuffer(DevInst, Args->Locs[i],
				XAIE_ENABLE);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable clock for column: %d\n",
					Args->Locs[i].Col);
			return RC;
		}

		_XAie_SetBitInBitmap(DevInst->TilesInUse, ColClockStatus,
				DevInst->NumRows);
	}

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
/** @} */
