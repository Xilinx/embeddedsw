/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_reset.c
* @{
*
* This file contains routines for AI engine resets
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_clock.h"
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_npi.h"
#include "xaie_reset.h"
#include "xaiegbl.h"

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE

/*****************************************************************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API set the tile column reset
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE SHIM tile
* @param	RstEnable: XAIE_ENABLE to assert reset, XAIE_DISABLE to
*			   deassert reset.
*
* @return	none
*
* @note		It is not required to check the DevInst and the Loc tile type
*		as the caller function should provide the correct value.
*
******************************************************************************/
static void  _XAie_RstSetColumnReset(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 RstEnable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	RegAddr = PlIfMod->ColRstOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	FldVal = XAie_SetField(RstEnable,
			PlIfMod->ColRst.Lsb,
			PlIfMod->ColRst.Mask);

	XAie_Write32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API set the tile columns reset for every column in the partition
*
* @param	DevInst: Device Instance
* @param	RstEnable: XAIE_ENABLE to assert reset, XAIE_DISABLE to
*			   deassert reset.
*
* @return	none
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*
******************************************************************************/
static void  _XAie_RstSetAllColumnsReset(XAie_DevInst *DevInst, u8 RstEnable)
{
	for (u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);

		_XAie_RstSetColumnReset(DevInst, Loc, RstEnable);
	}
}

/*****************************************************************************/
/**
*
* This API to set if to block NSU AXI MM slave error and decode error. If NSU
* errors is blocked, it will only generate error events.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE SHIM tile
* @param	Enable: XAIE_ENABLE to block NSU AXI MM errors, or XAIE_DISABLE
*			to unblock NSU AXI MM errors.
*
* @return	none
*
* @note		It is not required to check the DevInst and the Loc tile type
*		as the caller function should provide the correct value.
*
******************************************************************************/
static void _XAie_RstSetBlockShimNocAxiMmNsuErr(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Enable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimNocAxiMMConfig *ShimNocAxiMM;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ShimNocAxiMM = PlIfMod->ShimNocAxiMM;
	RegAddr = ShimNocAxiMM->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	FldVal = XAie_SetField(Enable,
			ShimNocAxiMM->NsuSlvErr.Lsb,
			ShimNocAxiMM->NsuSlvErr.Mask);
	FldVal |= XAie_SetField(Enable,
			ShimNocAxiMM->NsuDecErr.Lsb,
			ShimNocAxiMM->NsuDecErr.Mask);

	XAie_Write32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API set if to block the NSU AXI MM slave error and decode error config
* for all the SHIM NOCs in the full partition.
*
* @param	DevInst: Device Instance
* @param	Enable: XAIE_ENABLE to enable NSU AXI MM errors, XAIE_DISABLE to
*			disable.
*
* @return	none
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*		This function will do the following steps:
*		 * enable protected registers
*		 * set AXI MM registers NSU errors fields in all SHIM NOC tiles
*		 * disable protected registers
*
******************************************************************************/
static void  _XAie_RstSetBlockAllShimsNocAxiMmNsuErr(XAie_DevInst *DevInst,
		u8 Enable)
{
	XAie_NpiProtRegReq ProtRegReq = {0};

	ProtRegReq.Enable = XAIE_ENABLE;
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_SET_PROTREG, (void *)&ProtRegReq);

	for (u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);
		u8 TileType;

		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		if (TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
			continue;
		}
		_XAie_RstSetBlockShimNocAxiMmNsuErr(DevInst, Loc, Enable);
	}

	ProtRegReq.Enable = XAIE_DISABLE;
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_SET_PROTREG, (void *)&ProtRegReq);
}

/*****************************************************************************/
/**
*
* This API reset all SHIMs in the AI engine partition
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		This function asserts reset, and then deassert it.
*		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*
******************************************************************************/
static AieRC _XAie_RstAllShims(XAie_DevInst *DevInst)
{
	u8 TileType;
	const XAie_ShimRstMod *ShimTileRst;
	XAie_LocType Loc = XAie_TileLoc(0, 0);

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	ShimTileRst = DevInst->DevProp.DevMod[TileType].PlIfMod->ShimTileRst;

	return ShimTileRst->RstShims(DevInst, 0, DevInst->NumCols);
}

/*****************************************************************************/
/**
*
* This API resets the AI engine partition pointed by the AI engine device
* instance. The AI engine partition is the column based groups of AI engine
* tiles represented by the AI engine device instance. It will reset the SHIMs
* and columns in the partition, gate all the columns, and it will configure the
* SHIMNOC to block the slave and decode errors.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success.
*		XAIE_INVALID_ARGS if any argument is invalid
*
* @note		The reset partition device should be in one single backend call
*		to avoid any other thread to change some registers which can
*		cause system to system to crash. For some backend, such as CDO
*		backend	and debug backend, there is no such issue as they don't
*		run on real hardware. This function defines the AI engine
*		partition reset sequence for the backends which allow the reset
*		partition device with multiple backend calls.
*		Here is the reset sequence:
*		* clock gate all columns
*		* reset columns
*		* reset shims
*		* setup AXI MM config to block NSU errors
*		* gate all the tiles
*******************************************************************************/
AieRC XAie_ResetPartition(XAie_DevInst *DevInst)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	_XAie_PmSetPartitionClock(DevInst, XAIE_DISABLE);

	_XAie_RstSetAllColumnsReset(DevInst, XAIE_ENABLE);

	_XAie_RstAllShims(DevInst);

	_XAie_RstSetBlockAllShimsNocAxiMmNsuErr(DevInst, XAIE_ENABLE);

	_XAie_PmSetPartitionClock(DevInst, XAIE_DISABLE);

	return XAIE_OK;
}


/*****************************************************************************/
/**
*
* This API clears an AI engine tile data memory
*
* @param	DevInst: Device Instance
*
* @return	None.
*
* @note		internal to this file.
*******************************************************************************/
static void _XAie_ClearDataMem(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	const XAie_MemMod *MemMod;
	u64 RegAddr;
	u8 TileType;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;
	RegAddr = MemMod->MemAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	XAie_BlockSet32(DevInst, RegAddr, 0, MemMod->Size / 4);
}

/*****************************************************************************/
/**
*
* This API clears an AI engine tile program memory
*
* @param	DevInst: Device Instance
*
* @return	None.
*
* @note		internal to this file.
*******************************************************************************/
static void _XAie_ClearProgMem(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	const XAie_CoreMod *CoreMod;
	u64 RegAddr;

	CoreMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;
	RegAddr = CoreMod->ProgMemHostOffset +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	XAie_BlockSet32(DevInst, RegAddr, 0, CoreMod->ProgMemSize / 4);
}

/*****************************************************************************/
/**
*
* This API clears AI engine partition pointed by the AI enigne device instance.
* It will zeroize both data and program memories of the requested tiles.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success.
*		XAIE_INVALID_ARGS if any argument is invalid
*
* @note		None.
*******************************************************************************/
AieRC XAie_ClearPartitionMems(XAie_DevInst *DevInst)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	for(u32 C = 0; C < DevInst->NumCols; C++) {
		for(u32 R = 0; R < DevInst->NumRows; R++) {
			XAie_LocType Loc = XAie_TileLoc(C, R);
			u8 TileType;

			TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
			if(TileType == XAIEGBL_TILE_TYPE_SHIMNOC ||
			   TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
				continue;
			}

			if(_XAie_PmIsTileRequested(DevInst, Loc) ==
			   XAIE_DISABLE) {
				continue;
			}

			_XAie_ClearDataMem(DevInst, Loc);
			if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
				_XAie_ClearProgMem(DevInst, Loc);
			}
		}
	}

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
/** @} */
