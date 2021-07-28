/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_tilectrl.c
* @{
*
* This file contains routines for AIE tile controls.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   05/27/2021  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_tilectrl.h"
#include "xaiegbl_defs.h"

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE

/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API sets isolation of a tile
*
* @param	DevInst: Device Instance
* @param	Loc: tile location
* @param	Dir: directions to block, the direction input as:
*			XAIE_ISOLATE_EAST_MASK,
*			XAIE_ISOLATE_NORTH_MASK,
*			XAIE_ISOLATE_WEST_MASK,
*			XAIE_ISOLATE_SOUTH_MASK,
*			or "OR" operation of more than one of the above masks,
*			or XAIE_ISOLATE_ALL_MASK to block all directions.
*
* @return       XAIE_OK on success, error code on failure
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value. As this is internal function,
*		it will not verify the Dir input.
*		This is INTERNAL function.
*
******************************************************************************/
AieRC _XAie_TileCtrlSetIsolation(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 Dir)
{
	const XAie_TileCtrlMod *TCtrlMod;
	u32 FldVal, Mask;
	u64 RegAddr;
	u8 TileType;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Failed to set tile isolation, invalid tile type\n");
		return XAIE_ERR;
	}

	TCtrlMod = DevInst->DevProp.DevMod[TileType].TileCtrlMod;
	RegAddr = TCtrlMod->TileCtrlRegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	Mask = TCtrlMod->IsolateEast.Mask | TCtrlMod->IsolateNorth.Mask |
		TCtrlMod->IsolateWest.Mask | TCtrlMod->IsolateSouth.Mask;
	/*
	 * This is internal function, the Dir input masks matches the register
	 * isolation mask, there is no need to calculate each direction bit.
	 */
	FldVal = XAie_SetField(Dir,
			TCtrlMod->IsolateSouth.Lsb, Mask);

	return XAie_Write32(DevInst, RegAddr, FldVal);
}

#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
/** @} */
