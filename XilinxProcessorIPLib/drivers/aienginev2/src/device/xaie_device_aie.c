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
#include "xaie_clock.h"
#include "xaie_tilectrl.h"

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

/** @} */
