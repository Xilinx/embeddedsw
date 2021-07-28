/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_reset_aie.c
* @{
*
* This file contains routines for AI engine resets for AIE
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaiegbl.h"

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE

/*****************************************************************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
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
* @return	none
*
* @note		It is not required to check the DevInst and the Loc tile type
*		as the caller function should provide the correct value.
*
******************************************************************************/
static void _XAie_RstSetShimReset(XAie_DevInst *DevInst, XAie_LocType Loc,
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

	XAie_Write32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API reset the SHIM for the specified columns
*
* @param	DevInst: Device Instance
* @param	StartCol: Start column
* @param	NumCols: Number of columns
*
* @return	XAIE_OK
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value. Also it will not check the
*		@StartCol nor the @NumCols, the caller function should do the
*		validation and provide the correct values. This function does
*		the following steps:
*		 * Set reset bit for every SHIM
*		 * Assert SHIM reset
*		 * Deassert SHIM reset
*		 * Unset reset bit for every SHIM
*
******************************************************************************/
AieRC _XAie_RstShims(XAie_DevInst *DevInst, u32 StartCol, u32 NumCols)
{
	for (u32 C = StartCol; C < (StartCol + NumCols); C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);

		_XAie_RstSetShimReset(DevInst, Loc, XAIE_ENABLE);
	}

	XAie_RunOp(DevInst, XAIE_BACKEND_OP_ASSERT_SHIMRST,
			(void *)(uintptr_t)XAIE_ENABLE);

	XAie_RunOp(DevInst, XAIE_BACKEND_OP_ASSERT_SHIMRST,
			(void *)(uintptr_t)XAIE_DISABLE);

	for (u32 C = StartCol; C < (StartCol + NumCols); C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);

		_XAie_RstSetShimReset(DevInst, Loc, XAIE_DISABLE);
	}

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
/** @} */
